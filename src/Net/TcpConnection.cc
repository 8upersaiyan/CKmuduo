#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

#include <functional>
#include <errno.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <string>

static EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d TcpConnection Loop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, //构造函数 
                const std::string &nameArg, 
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop))
    , name_(nameArg)
    , state_(kConnecting)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , highWaterMark_(64*1024*1024) //超过64M就到水位线了，要停止发送 
{
    //下面给channel设置相应的回调函数，poller给channel通知感兴趣的事件发生了，channel会回调相应的操作函数
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1)
    );
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);//启动Tcp Socket的保活机制
}


TcpConnection::~TcpConnection()//析构函数 
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n", 
        name_.c_str(), channel_->fd(), (int)state_);
}

// onmessage处理完业务结束 通过send给客户端返回处理结果数据
// 对外都提供string类型的  
void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)//连接成功状态
    {
        if (loop_->isInLoopThread())//当前loop是不是在对应的线程 
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            // 遇到重载函数的绑定，可以使用函数指针来指定确切的函数
            void(TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(
                //&TcpConnection::sendInLoop,
                fp,
                this,
                buf.c_str(),
                buf.size()
            ));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            // sendInLoop有多重重载，需要使用函数指针确定
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

/**
 * 发送数据  应用因为是非阻塞IO它写的快， 而内核发送数据慢，需要两者速度匹配
 * 需要把待发送数据写入缓冲区， 而且设置了水位回调
 */ 
void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0; //已发送数据
    size_t remaining = len; //还没发送的数据
    bool faultError = false;

    //之前调用过该connection的shutdown，不能再进行发送了
    if (state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }

    //表示channel_第一次开始写数据，而且缓冲区没有待发送数据 
    //(Channel向缓冲区写数据，给客户端应用响应)
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), data, len); //向发送缓冲区中 传入data
        if (nwrote >= 0) //发送成功
        {
            remaining = len - nwrote; //剩余还没有发送完的数据
            if (remaining == 0 && writeCompleteCallback_)
            {
                //既然在这里数据全部发送完成，就不用再给channel设置epollout事件了
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                );
            }
        }
        else//nwrote < 0发送失败
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE  RESET, 表示对端的socket的重置
                {
                    faultError = true;
                }
            }
        }
    }

    //说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
    //注册epollout事件，poller发现tcp的发送缓冲区有空间，会通知相应的sock-channel，调用writeCallback_回调方法
    //也就是调用TcpConnection::handleWrite方法，把发送缓冲区中的数据全部发送完成
    if (!faultError && remaining > 0) 
    {
        //目前发送缓冲区剩余的待发送数据的长度
        size_t oldLen = outputBuffer_.readableBytes();
        //发送缓冲区剩余数据 + 剩余还没发送的数据 >= 水位线  
        //且发送缓冲区剩余数据小于水位线 就原本是小于的加上这一些大于了
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(
                std::bind(highWaterMarkCallback_, shared_from_this(), oldLen+remaining)
            );
        }
        //把待发送数据发送到outputBuffer缓冲区上
        outputBuffer_.append((char*)data + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();//这里一定要注册channel的写事件，否则poller不会给channel通知epollout
        }
    }
}

//关闭连接
void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting); // 正在断开连接 不会正真的断开连接需要等待发送缓冲区为空
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

//回去底层调用handleClose方法
void TcpConnection::shutdownInLoop()
{
    //调用了shutdown并不是真正断开连接 只是把状态设置为 kDisconnecting 
    //等待数据发送完，标志就是！channel_->isWriting() channel没在写了 调用shutdownWrite()彻底关闭
    if (!channel_->isWriting())//说明outputBuffer中的数据已经全部发送完成
    {
        socket_->shutdownWrite();//关闭写端
    }
}

//连接建立 创建连接
void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();//向poller注册channel的epollin事件

    //新连接建立，执行回调
    connectionCallback_(shared_from_this());
}

//连接销毁 连接关闭
void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll(); // 把channel的所有感兴趣的事件，从poller中del掉
        connectionCallback_(shared_from_this());
    }
    channel_->remove();//把channel从poller中删除掉
}

//表示fd有数据可读
void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        //已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
        //shared_from_this:获取当前TcpConnection对象的一个智能指针
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)//客户端断开连接
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

//表示fd可写数据
void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) 
    {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);//发送数据
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) //发送完成
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    //唤醒loop_对应的thread线程，执行回调
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this())
                    );
                }
                if (state_ == kDisconnecting) //读完数据正在断开状态（发送缓冲区为空且是正在断开连接状态）
                {
                    shutdownInLoop(); //真正关闭连接
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else //调用handleWrite但是channel此时是不可写状态
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", channel_->fd());
    }
}

//poller => channel::closeCallback => TcpConnection::handleClose
//底层的poller通知Channel，socket要关闭了，channel调用的是CloseCallback这个回调方法
//CloseCallback这个回调方法在TcpServer中设置成removeConnection了
void TcpConnection::handleClose()
{
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);//执行连接关闭的的回调（connectionCallback_回调函数 连接成功和关闭都会响应）
    closeCallback_(connPtr);//关闭连接的回调  执行的是TcpServer::removeConnection回调方法
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}

