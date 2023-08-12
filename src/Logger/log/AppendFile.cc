#include "FileUtil.h"
#include "Logging.h"

FileUtil::AppendFile::AppendFile(StringArg filename)
  : fp_(::fopen(filename.c_str(), "ae")),  // 'e' for O_CLOEXEC
    writtenBytes_(0)
{
  assert(fp_);
  ::setbuffer(fp_, buffer_, sizeof buffer_);
  // posix_fadvise POSIX_FADV_DONTNEED ?
}

FileUtil::AppendFile::~AppendFile()
{
  ::fclose(fp_);
}

void FileUtil::AppendFile::append(const char* logline, const size_t len)
{
  size_t written = 0; // 记录已经写入的数据大小

  while (written != len)
  {
    size_t remain = len - written; // 还需写入的数据大小
    size_t n = write(logline + written, remain);
    if (n != remain)
    {
      int err = ferror(fp_);
      if (err)
      {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
        break;
      }
    }
    written += n; // 更新写入的数据大小
  }

  writtenBytes_ += written; // 记录目前为止写入的数据大小，超过限制会滚动日志
}

size_t FileUtil::AppendFile::write(const char* logline, size_t len)
{
  // #undef fwrite_unlocked
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

void FileUtil::AppendFile::flush()
{
    ::fflush(fp_);
}
