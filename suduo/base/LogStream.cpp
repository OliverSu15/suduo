#include "LogStream.h"

#include <system_error>
namespace suduo {
template class suduo::StreamBuffer<MIN_BUFFER_SIZE>;
template class suduo::StreamBuffer<MAX_BUFFER_SIZE>;
}  // namespace suduo

const std::errc suduo::LogStream::err = std::errc();