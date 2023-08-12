#include "LogFile.h"

LogFile::LogFile(const std::string& basename,
        off_t rollSize,
        int flushInterval,
        int checkEveryN)
    : basename_(basename),  // 基础文件名, 用于新log文件命名
      rollSize_(rollSize),  // 滚动文件大小
      flushInterval_(flushInterval),    // 冲刷时间限值, 默认3 (秒)
      checkEveryN_(checkEveryN),    // 写数据次数限值, 默认1024
      count_(0),    // 写数据次数计数, 超过限值checkEveryN_时清除, 然后重新计数
      mutex_(new std::mutex),   // 互斥锁指针, 根据是否需要线程安全来初始化
      startOfPeriod_(0),    // 本次写log周期的起始时间(秒)
      lastRoll_(0),     // 上次roll日志文件时间(秒)
      lastFlush_(0)     // 上次flush日志文件时间(秒)
{
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* data, int len)
{
    std::lock_guard<std::mutex> lock(*mutex_);
    appendInLock(data, len);
}

void LogFile::appendInLock(const char* data, int len)
{
    file_->append(data, len);

    if (file_->writtenBytes() > rollSize_)
    {
        rollFile();
    }
    else
    {
        ++count_;
        if (count_ >= checkEveryN_)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod != startOfPeriod_)
            {
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

void LogFile::append_unlocked(const char *logline, int len)
{
    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_)
    { // written bytes to file_ > roll threshold (rollSize_)
        rollFile();
    }
    else
    {
        ++count_;
        if (count_ >= checkEveryN_)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;

            if (thisPeriod_ != startOfPeriod_)
            { // new period, roll file for log
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_)
            { // timeout ( flushInterval_ = 3 seconds)
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}



void LogFile::flush()
{
    if (mutex_)
    {
        MutexLockGuard lock(*mutex_);
        file_->flush();
    }
    // std::lock_guard<std::mutex> lock(*mutex_);
    file_->flush();
}

// 滚动日志
// basename + time + hostname + pid + ".log"
bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_, &now);
    // 计算现在是第几天 now/kRollPerSeconds求出现在是第几天，再乘以秒数相当于是当前天数0点对应的秒数
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_)
    {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        // 让file_指向一个名为filename的文件，相当于新建了一个文件
        file_.reset(new FileUtil(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm);
    // 写入时间
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;

    filename += ".log";

    return filename;
}