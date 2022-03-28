#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include <sched.h>

#include <functional>
#include <string>
namespace suduo {
using string = std::string;
namespace _detail {
void* run(void* it);
}

class Thread {
 public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(ThreadFunc func_, const string&& name = "suduo");
  ~Thread();

  void start();

  void join();

  bool started() const { return _started; }
  pid_t tid() const { return _tid; }
  const string& name() const { return _name; }

 private:
  friend void* suduo::_detail::run(void* it);
  bool _started = false;
  bool _joined = false;
  pthread_t _thread_id;
  pid_t _tid;
  string _name;
  ThreadFunc _func;
};
}  // namespace suduo
#endif