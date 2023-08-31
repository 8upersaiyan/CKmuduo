#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Channel;//只用到指针类型 
class EventLoop;//只用到指针类型 

//muduo库中多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;//虚析构函数 

    //需要交给派生类实现的接口 给所有IO复用保留统一的接口 纯虚函数
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;//相当于启动了epoll_wait 
    virtual void updateChannel(Channel *channel) = 0;//相当于启动了epoll_ctrl 
    virtual void removeChannel(Channel *channel) = 0;//fd所感兴趣的事件delete掉 
    
    //判断参数channel是否在当前Poller当中
    bool hasChannel(Channel *channel) const;

    // EventLoop可以通过该接口获取默认的IO复用实现方式(默认epoll)
    /** 
     * 它的实现并不在 Poller.cc 文件中
     * 如果要实现则可以预料会其会包含EPollPoller PollPoller
     * 那么外面就会在基类引用派生类的头文件，这个抽象的设计就不好
     * 所以外面会单独创建一个 DefaultPoller.cc 的文件去实现
     */
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    //map的key：sockfd  value：sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;//定义Poller所属的事件循环EventLoop
};

