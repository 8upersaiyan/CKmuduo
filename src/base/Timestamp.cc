#include "Timestamp.h"
#include <time.h>

Timestamp::Timestamp()
    :microSecondsSinceRpoch_(0)
    {}

// 带参数的构造函数
Timestamp::Timestamp(int64_t microSecondsSinceRpoch)
    : microSecondsSinceRpoch_(microSecondsSinceRpoch)
    {}

//显示当前时间 
Timestamp Timestamp::now()
{
    // time_t timenow = time(NULL);
    // return Timestamp(timenow);
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t time = tv.tv_sec * 1000000 + tv.tv_usec; // 转换为微秒
    return Timestamp(time);
    
}

// 2022/08/26 16:29:10
// 20220826 16:29:10.773804
std::string Timestamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceRpoch_ / kMicroSecondsPerSecond);
    // 使用localtime函数将秒数格式化成日历时间
    tm *tm_time = localtime(&seconds);
    if (showMicroseconds)
    {
        int microseconds = static_cast<int>(microSecondsSinceRpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
                tm_time->tm_year + 1900,
                tm_time->tm_mon + 1,
                tm_time->tm_mday,
                tm_time->tm_hour,
                tm_time->tm_min,
                tm_time->tm_sec,
                microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
                tm_time->tm_year + 1900,
                tm_time->tm_mon + 1,
                tm_time->tm_mday,
                tm_time->tm_hour,
                tm_time->tm_min,
                tm_time->tm_sec);
    }
    return buf;
}

//格式转化方法 将字符串转化成时间字符串
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    tm * tm_time = localtime(&microSecondsSinceRpoch_);
    snprintf(buf,128,"%4d/%02d/%02d %02d:%02d:%02d",
    tm_time->tm_year + 1900,
    tm_time->tm_mon + 1,
    tm_time->tm_mday,
    tm_time->tm_hour,
    tm_time->tm_min,
    tm_time->tm_sec
    );
    return buf;
}
