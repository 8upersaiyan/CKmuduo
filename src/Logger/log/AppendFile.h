#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <stdio.h>
#include <string>

// not thread safe
class AppendFile : public noncopyable
{
public:
    explicit AppendFile(StringArg filename);
    ~AppendFile();
    void append(const char* logline, size_t len); // 添加log消息到文件末尾
    void flush(); // 冲刷文件到磁盘
    off_t writtenBytes() const { return writtenBytes_; } // 返回已写字节数

private:
    size_t write(const char* logline, size_t len); // 写数据到文件

    FILE* fp_;                                // 文件指针
    char buffer_[ReadSmallFile::kBufferSize]; // 文件操作的缓冲区
    off_t writtenBytes_;                      // 已写字节数
};

#endif // FILE_UTIL_H