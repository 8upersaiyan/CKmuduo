#pragma once
//Reactor模型中的事件循环 相当与事件分发器 epoll
//事件循环类   主要包含了两个大模块 Channel Poller（epoll的抽象）  
#include <functional>
#include <vector>
#include <atomic>//C++ 11
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "TimerQueue.h"
#include "CurrentThread.h"

class Channel;
class Poller;

//事件循环类  主要包含了两个大模块 Channel   Poller（epoll的抽象）
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;//定义一个回调的类型 
    //using代替typedef，进行类型的重命名 

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    //返回当前时间
    Timestamp pollReturnTime() const { return pollReturnTime_; }
    
    //在当前loop中执行cb，cb是回调操作
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    //用来唤醒loop所在的线程的(mainReactor用来唤醒subReactor)
    void wakeup();
 
    //EventLoop的方法,其中调用的是Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    //判断EventLoop对象是否在自己的线程里面
    //在自己线程里执行runInLoop 不在自己线程里执行queueInLoop
    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }

    //定时任务相关函数
    void runAt(Timestamp timestamp, Functor&& cb) {
        timerQueue_->addTimer(std::move(cb), timestamp, 0.0);
    }

    void runAfter(double waitTime, Functor&& cb) {
        Timestamp time(addTime(Timestamp::now(), waitTime)); 
        runAt(time, std::move(cb));
    }

    void runEvery(double interval, Functor&& cb) {
        Timestamp timestamp(addTime(Timestamp::now(), interval)); 
        timerQueue_->addTimer(std::move(cb), timestamp, interval);
    }

private:
    void handleRead();//唤醒wake up
    void doPendingFunctors();//执行回调

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;//事件循环是正常运行还是退出循环 (atomic，我们使用C++11CAS实现的)
    std::atomic_bool quit_;//客户端全部断开连接以后，服务器也需要退出事件循环，用来标识退出loop循环
    
    const pid_t threadId_;//记录当前loop所在线程的id
    Timestamp pollReturnTime_;//poller返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;//eventloop所管理的poller 
    std::unique_ptr<TimerQueue> timerQueue_;

    //mainReactor如何将发生事件的channel给到subReactor
    int wakeupFd_;//linux内核的eventfd创建出来的 
	//主要作用，当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;//包括wakeupFd和感兴趣的事件的channel指针 
    
    ChannelList activeChannels_;//eventloop管理的所有channel 
    Channel* currentActtiveChannel_; //当前处理的活跃channel

    std::atomic_bool callingPendingFunctors_;//标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;//存储loop需要执行的所有的回调操作
    std::mutex mutex_;//互斥锁，用来保护上面vector容器的线程安全操作
};


