#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Logger.h"
#include "suduo/base/Timestamp.h"
using namespace suduo;
const size_t N = 1000000;

void default_output(const char* val, size_t len) {
  // FIXME error handle
}

void benchLogStream_string() {
  const int kBatch = 1000;
  // LogStream os;
  Logger::set_output_function(default_output);
  Timestamp start(Timestamp::now());
  for (int t = 0; t < 30; ++t) {
    for (size_t i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz ";
      // os.reset_buffer();
    }
  }
  Timestamp end(Timestamp::now());

  printf("benchLogStream_string %f\n", (end - start).get_seconds_in_double());
}

int main() { benchLogStream_string(); }