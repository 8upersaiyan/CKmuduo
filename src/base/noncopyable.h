#pragma once

/*
noncopyable 被继承后 派生类对象可以正常调用构造函数和析构函数
但是派生类对象无法使用拷贝构造函数  和 赋值操作
*/
class noncopyable
{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
 
