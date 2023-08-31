//公共源文件
#include "Poller.h"
#include "EPollPoller.h"
#include <stdlib.h>


Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;  //生成一个poll实例
    }
    /*
    if(::getenv("MUDUO_USE_POLL"))
    {
        return new PollPoller(loop);  //生成一个poll实例 没实现
    }
    */
    else{
        return new EPollPoller(loop);  //生成一个epoll实例
    }
}

