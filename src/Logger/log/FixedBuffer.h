#ifndef FIXED_BUFFER_H
#define FIXED_BUFFER_H

#include "noncopyable.h"
#include <assert.h>
#include <string.h> // memcpy
#include <strings.h>
#include <string>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000; 

template<int SIZE>
class FixedBuffer : noncopyable
{
 public:
  FixedBuffer()
    : cur_(data_)
  {
    setCookie(cookieStart);
  }

  ~FixedBuffer()
  {
    setCookie(cookieEnd);
  }

  // 向缓冲区添加数据
  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
	  // 将buf地址，长为len的数据拷贝到cur_地址处
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }

  // write to data_ directly
  char* current() { return cur_; }
  int avail() const { return static_cast<int>(end() - cur_); }
  void add(size_t len) { cur_ += len; }

  void reset() { cur_ = data_; }
  void bzero() { memZero(data_, sizeof data_); }

  // for used by GDB
  const char* debugString();
  void setCookie(void (*cookie)()) { cookie_ = cookie; }
  // for used by unit test
  string toString() const { return string(data_, length()); }
  StringPiece toStringPiece() const { return StringPiece(data_, length()); }

 private:
  const char* end() const { return data_ + sizeof data_; }
  // Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();

  void (*cookie_)();
  char data_[SIZE];
  char* cur_;
};

#endif // FIXED_BUFFER_H