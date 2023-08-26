# CKmuduo
本项目是参考 muduo 实现的基于 Reactor 模型的多线程网络库。使用 C++ 11 编写去除 muduo 对 boost 的依赖，内部实现了一个小型的 HTTP 服务器。
# 开发环境
* 操作系统：`Ubuntu 18.04.6 LTS`
* 编译器：`VsCode`
* 编译环境：`g++ (GCC) 10.2.1`
* 版本控制：`git`
* 项目构建：`cmake version 3.20.2`
# 项目描述
* 学习陈硕老师的 C++ muduo 网络库优秀的代码设计及编程方法。
* 去掉了Muduo库中的Boost依赖，完全使用C++标准，如使用std::function<>。
* 底层使用 Epoll + LT 模式的 I/O 复用模型，并且结合非阻塞 I/O 实现主从 Reactor 模型。
* 采用「one loop per thread」线程模型，并向上封装线程池避免线程创建和销毁带来的性能开销。
* 使用 Epoll + LT 模式的 I/O 复用模型，并且结合非阻塞 I/O 实现主从 Reactor 模型。
* 基于自实现的双缓冲区实现异步日志，由后端线程负责定时向磁盘写入前端日志信息，支持日志滚动功能。
* 基于红黑树实现定时器管理结构，内部使用 Linux 的 timerfd 通知到期任务，高效管理定时任务。
* 实现 Channel、Poller、EventLoop、TcpServer、Buffer、TcpConnection 等重要部分。
* 提供面向用户的TcpServer类，用户只需要通过bind绑定自定义的回调方法即可开始使用。
* 实现了echo服务器和一个小型的HTTP服务器，可支持GET请求和静态资源访问，使用了Apache Benchmark做了压测。


# 具体实现细节博客
https://blog.csdn.net/super8ayan/category_12359543.html?spm=1001.2014.3001.5482
# 使用方法
执行`autobuild.sh`脚本执行`CMakeList`生成`libmymuduo.so`库
并将库文件拷贝到 /usr/lib路径下

    #!/bin/bash
    set -e
    # 如果没有build目录，创建该目录
    if [ ! -d `pwd`/build ]; then
        mkdir `pwd`/build
    fi
    rm -rf `pwd`/build/*
    cd `pwd`/build &&
        cmake .. &&
        make
    # 回到项目根目录
    cd ..
    # 把头文件拷贝到 /usr/include/mymuduo  so库拷贝到 /usr/lib    PATH
    if [ ! -d /usr/include/mymuduo ]; then 
        mkdir /usr/include/mymuduo
    fi
    # 拷贝头文件 
    for header in $(find src -type f -name "*.h"); 
    do
        cp $header /usr/include/mymuduo
    done
    #拷贝库文件
    cp `pwd`/lib/libmymuduo.so /usr/lib
    ldconfig
# 使用运行案例
## EchoServer 回响服务器
这里以一个简单的回声服务器作为案例，EchoServer默认监听端口为8080。

    cd ./example
    ./EchoServer

执行情况：
![image](https://github.com/8upersaiyan/CKmuduo/assets/102213169/4fa91142-34a6-4ad7-a546-3eebb984db75)

## 简单Http 回响服务器
可支持 GET 请求和静态资源的访问

    cd example/test2-HttpServer/bin
    ./HttpServer

执行情况：
![image](https://github.com/8upersaiyan/TinyNetworkLibrary/assets/102213169/cf4c369a-080c-4c5e-be5e-06a10d385534)

使用Apache Benchmark压测：
在使用VMware虚拟机 Ubuntu 18.04.6 LTS CPU核数：4 系统运行内存4GB （setThreadNums = 4）

    ab -n 1000000 -c 1000 -k http://127.0.0.1:8000/hello

模拟1000个客户端同时访问 ” http://127.0.0.1:8000/hello “ 总计发出1,000,000个请求，所有的请求都会尝试重用TCP连接。

数据为：

    Server Software:        Muduo
    Server Hostname:        192.168.11.129
    Server Port:            8080
    
    Concurrency Level:      1000              
    Time taken for tests:   62.436 seconds
    Complete requests:      1000000
    Failed requests:        0
    Keep-Alive requests:    1000000
    Total transferred:      112000000 bytes
    HTML transferred:       14000000 bytes
    Requests per second:    20171.60 [#/sec] (mean)
    Time per request:       50.376 [ms] (mean)
    Time per request:       0.050 [ms] (mean, across all concurrent requests)
    Transfer rate:          2182.08 [Kbytes/sec] received

解释：
    服务器监听在192.168.11.129的8080端口上；
    服务器能够很好地处理高并发请求，在1000并发连接下，成功完成了1000000次请求，且没有任何请求失败；
    服务器的吞吐率为20171.60请求/秒，这意味着它在每秒内能处理超过2万个请求。
    服务器的响应时间相当快，每个请求的平均响应时间只有50.376毫秒，而在高并发情况下，这一数字降至0.050毫秒。
    服务器在测试期间传输了大量的数据，平均传输速率为2182.08KB/秒。
    ![image](https://github.com/8upersaiyan/TinyNetworkLibrary/assets/102213169/09f42a05-c3e3-4983-8f1d-0f0451de077d)


