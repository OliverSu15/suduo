#ifndef FILE_UTIL_H
#define FILE_UTIL_H
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
namespace suduo {
namespace FileUtil {
class FileAppender {
 public:
  explicit FileAppender(const std::string& filename);
  explicit FileAppender(const char* filename);
  ~FileAppender();
  void append(const char* log, size_t len);
  void flush();
  int64_t written_bytes() const { return _written_bytes; }

 private:
  size_t write(const char* log, size_t len);
  std::FILE* _fp;
  std::array<char, 64 * 1024> _buffer;
  int64_t _written_bytes;
};
}  // namespace FileUtil
}  // namespace suduo
#endif