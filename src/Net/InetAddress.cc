#include "InetAddress.h"
#include <strings.h>
#include <string.h>


//初始化InetAddress地址类对象
InetAddress::InetAddress(uint16_t port, std::string ip)
{
    memset(&addr_, 0, sizeof addr_); //将 addr_ 的内存空间清零
    addr_.sin_family = AF_INET; //表示使用 IPv4 地址族
    addr_.sin_port = htons(port); //将主机字节序转换为网络字节序，设置端口号
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); //转换为网络字节序的 IPv4 地址
}

//将存储在 addr_ 成员变量中的 IPv4 地址转换为字符串表示
std::string InetAddress::toIp() const
{
    // addr_ 
    char buf[64] = {0};
    //::inet_ntop() 使用了全局命名空间中的 inet_ntop() 函数
    //保证调用的是全局作用域之中的函数
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    //组合成 ip ：port    
    char buf[64] = {0};
    //::inet_ntop() 使用了全局命名空间中的 inet_ntop() 函数
    //保证调用的是全局作用域之中的函数
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}

// #include <iostream>
// int main()
// {
//     InetAddress addr(8080);
//     std::cout<<addr.toIpPort()<<std::endl;
//     return 0;
// }
