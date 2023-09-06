#pragma once

#include <vector>
#include <string>
#include <algorithm>

//网络库底层的缓冲器类型定义
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;//头部字节大小 记录数据包的长度 
    static const size_t kInitialSize = 1024;//缓冲区 的大小 

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize)//开辟的大小 
        , readerIndex_(kCheapPrepend)
        , writerIndex_(kCheapPrepend)
    {}

    //可读的数据长度 
    size_t readableBytes() const 
    {
        return writerIndex_ - readerIndex_;
    }

    //可写的缓冲区长度 
    size_t writableBytes() const 
    {
        return buffer_.size() - writerIndex_;
    }

    //返回头部的空间的大小 
    size_t prependableBytes() const 
    {
        return readerIndex_;
    }

    //返回缓冲区中可读数据的起始地址
    const char* peek() const
    {
        return begin() + readerIndex_;
    }

    void retrieveUntil(const char *end)
    {
        retrieve(end - peek());
    }
    
    // 判断数据有没有一次性读完
    void retrieve(size_t len)
    {
        //没有一次性将数据读完
        if (len < readableBytes())
        {
            readerIndex_ += len;//应用只读取了刻度缓冲区数据的一部分，就是len，还剩下数据readerIndex_ += len -> writerIndex_
        }
        else//len == readableBytes()，一次性将数据读完了
        {
            retrieveAll();  //将readerIndex_和writerIndex_复位
        }
    }

    //将readerIndex_和writerIndex_复位
    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());//应用可读取数据的长度
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len);//上面一句把缓冲区中可读的数据，已经读取出来，这里肯定要对缓冲区进行复位操作
        return result;
    }

    //buffer_.size() - writerIndex_    写len长的数据 
    void ensureWriteableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);//扩容函数
        }
    }

    // string::data() 转换成字符数组，但是没有 '\0'
    void append(const std::string &str)
    {
        append(str.data(),str.size());
    }
    //把[data, data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data+len, beginWrite());//data复制到目标位置beginWrite()
        writerIndex_ += len;
    }

    const char* findCRLF() const
    {
        // FIXME: replace with memmem()?
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    char* beginWrite()//返回写入起始地址
    {
        return begin() + writerIndex_;
    }
    const char* beginWrite() const//返回常数地址
    {
        return begin() + writerIndex_;
    }

    //从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);
    //通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);

private:
    char* begin()
    {
        //it.operator*() 获取迭代器指向的内容，然后取个地址 
        return &*buffer_.begin();//vector底层数组首元素的地址，也就是数组的起始地址
    }
    const char* begin() const
    {
        return &*buffer_.begin();
    }

    // 扩容函数
    void makeSpace(size_t len)
    {
        /*
        kCheapPrepend |    reader    |    writer  |
        kCheapPrepend |            len            |
        */
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else//writableBytes() + prependableBytes()这些空间足够写len长度的数据！
        {
            size_t readalbe = readableBytes();
            std::copy(begin() + readerIndex_, //未读取的数据往begin() + kCheapPrepend处搬
                    begin() + writerIndex_,
                    begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;//回到原位置  8下标位置 
            writerIndex_ = readerIndex_ + readalbe;
        }
    }

    std::vector<char> buffer_;//vector数组 扩容方便 
    size_t readerIndex_;//可读数据的下标位置 
    size_t writerIndex_;//写数据的下标位置 
    static const char kCRLF[];
};  

