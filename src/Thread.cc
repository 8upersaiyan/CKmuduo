#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

//Thread构造函数
Thread::Thread(ThreadFunc func,const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , func_(std::move(func))
    , name_(name)
{
    setDefaultName();//给线程设置默认的名字
}

//Thread析构函数
Thread::~Thread() 
{
    if(started_ && !joined_) //线程运行起来了且不是
    {
        // thread 类提供的设置分离线程的方法，成了1个守护线程，主线程结束，守护线程自动结束
        thread_->detach();
    }
}

//启动当前线程
void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程的tid
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        //开启一个新线程 专门执行该线程函数
        func_(); //包含一个eventLoop
    }));//只能指针指向线程对象

    //调用start什么时候结束？ 必须等待新线程创建完成 返回其tid后才能结束 通过信号量
    sem_wait(&sem);
}

//当前线程阻塞，直到被调用的线程执行完毕并退出
//也可以理解为主线程等待其他线程完成，确保主线程结束之前所有的子线程都已经完成
void Thread::join()
{
    joined_ = true;
    thread_ -> join();
}

//给线程设置默认的名字
void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32] = {0};
        snprintf(buf,sizeof(buf),"Thread&d",num);
        name_ = buf;
    }

}