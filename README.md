# CKmuduo
基于 Reactor 反应堆模型的多线程 C++网络库
# 平台工具 
vs code 远程连接linux ubuntu20.04.5开发 + CMake构建
# 项目描述
在 Linux 环境下使用 C++11 + Reactor+one loop per thread 思想开发网络库模型。
# 主要工作
1. 学习陈硕老师的 C++ muduo 网络库优秀的代码设计及编程方法；
2. 重写 muduo 核心组件，去依赖 boost，用 C++11 重构代码，可移植性好；
3. 利用 Reactor 反应堆模型搭建多线程 C++11 网络库；
4. 基于事件驱动和事件回调的 epoll+线程池面向对象编程；
5. 实现 Channel、Poller、EventLoop、TcpServer、Buffer、TcpConnection 等重要部分。
# 具体实现细节博客
https://blog.csdn.net/super8ayan/category_12359543.html?spm=1001.2014.3001.5482
# 使用方法
编写shell脚本：
#!/bin/bash

set -e

#如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

#回到项目根目录
cd ..

#把头文件拷贝到 /usr/include/mymuduo  so库拷贝到 /usr/lib    PATH
if [ ! -d /usr/include/mymuduo ]; then 
    mkdir /usr/include/mymuduo
fi

#拷贝头文件 
for header in `ls *.h`
do
    cp $header /usr/include/mymuduo
done

#拷贝库文件
cp `pwd`/lib/libmymuduo.so /usr/lib

ldconfig
 
运行shell脚本：
chmod +x autobuild.sh //赋予可执行权限
./autobuild.sh //执行