#include "suduo/base/Exception.h"

#include <stdio.h>

#include <functional>
#include <vector>

#include "suduo/base/CurrentThreadInfo.h"

class Bar {
 public:
  void test(std::vector<std::string> names = {}) {
    printf("Stack:\n%s\n",
           suduo::Current_thread_info::stack_trace(true).c_str());
    [] {
      printf("Stack inside lambda:\n%s\n",
             suduo::Current_thread_info::stack_trace(true).c_str());
    }();
    std::function<void()> func([] {
      printf("Stack inside std::function:\n%s\n",
             suduo::Current_thread_info::stack_trace(true).c_str());
    });
    func();

    func = std::bind(&Bar::callback, this);
    func();

    throw suduo::Exception("oops");
  }

 private:
  void callback() {
    printf("Stack inside std::bind:\n%s\n",
           suduo::Current_thread_info::stack_trace(true).c_str());
  }
};

void foo() {
  Bar b;
  b.test();
}

int main() {
  try {
    foo();
  } catch (const suduo::Exception& ex) {
    printf("reason: %s\n", ex.what());
    printf("stack trace:\n%s\n", ex.stackTrace());
  }
}
