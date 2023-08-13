#pragma once

#include <iostream>
#include <string>

class Timestamp
{
    public:
        // 默认构造函数
        Timestamp();
        // 带参数的构造函数
        explicit Timestamp(int64_t microSecondsSinceRpoch_);
        //显示当前时间戳 并返回一个Timestamp对象
        static Timestamp now();
        //格式转化方法 将字符串转化成时间字符串
        std::string toString() const;
        //格式, "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
        std::string toFormattedString(bool showMicroseconds = false) const;
        //返回当前时间戳的秒数
        time_t secondsSinceEpoch() const
        { 
            return static_cast<time_t>(microSecondsSinceRpoch_ / kMicroSecondsPerSecond); 
        }

        //返回当前时间戳的微妙
        int64_t microSecondsSinceEpoch() const { return microSecondsSinceRpoch_; }
        static const int kMicroSecondsPerSecond = 1000 * 1000;

        // 失效的时间戳，返回一个值为0的Timestamp
        static Timestamp invalid()
        {
            return Timestamp();
        }
            
    private:
        int64_t microSecondsSinceRpoch_;
};

/**
 * 定时器需要比较时间戳，因此需要重载运算符
 */
inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// 如果是重复定时任务就会对此时间戳进行增加。
static inline Timestamp addTimec(Timestamp timestamp, double seconds)
{
    // 将延时的秒数转换为微妙
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    // 返回新增时后的时间戳
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}          

