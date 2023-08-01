#pragma once

#include "noncopyable.h"
#include <functional>
#include <thread>
#include <string>
#include <atomic>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread(); //析构函数

    void start(); // 启动当前线程
    void join(); // 当前线程等待其他线程完了再运行下去

    bool started() const {return started_; };   
    pid_t tid() const {return tid_; };  //返回线程tid
    const std::string & name() const {return name_; }; //返回当前线程名字
    static int numCreated() { return numCreated_; }; //返回创建的线程数量

private:
    void setDefaultName(); //给线程设置默认的名字

    bool started_;  //启动当前线程
    bool joined_; // 当前线程等待其他线完了再运行下去
    std::shared_ptr<std::thread> thread_; // 自己掌控线程对象产生的时机，这里直接调用thread thread_它会立即创建一个新线程
    pid_t tid_;
    ThreadFunc func_;  // 存储线程函数
    std::string name_; // 调试的时候打印
    static std::atomic_int numCreated_; // 对线程数量计数
};