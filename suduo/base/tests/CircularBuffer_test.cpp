#include "suduo/base/CircularBuffer.h"

#include <cstdio>

#include "suduo/base/CurrentThreadInfo.h"
using CircularBuffer = suduo::detail::CircularBuffer<int>;
const int test_size = 50;
void add_test_1(CircularBuffer& buffer) {
  for (int i = 0; i < test_size; i++) {
    buffer.push(i);
    printf("%d pushed\n", i);
  }
  printf("full:%d, empty:%d, size:%d, capacity:%d\n", buffer.full(),
         buffer.empty(), buffer.size(), buffer.capacity());
}

void pop_test_1(CircularBuffer& buffer) {
  for (int i = 0; i < test_size; i++) {
    auto val = buffer.pop();
    printf("%d pop\n", val);
  }
  printf("full:%d, empty:%d, size:%d, capacity:%d\n", buffer.full(),
         buffer.empty(), buffer.size(), buffer.capacity());
}

void test_2(CircularBuffer& buffer) {
  for (int i = 0; i < 50; i++) {
    buffer.push(i);
    printf("%d pushed\n", i);
  }
  printf("full:%d, empty:%d, size:%d, capacity:%d\n", buffer.full(),
         buffer.empty(), buffer.size(), buffer.capacity());
  for (int i = 0; i < 20; i++) {
    auto val = buffer.pop();
    printf("%d pop\n", val);
  }
  printf("full:%d, empty:%d, size:%d, capacity:%d\n", buffer.full(),
         buffer.empty(), buffer.size(), buffer.capacity());
  for (int i = 0; i < 20; i++) {
    buffer.push(i);
    printf("%d pushed\n", i);
  }
  printf("full:%d, empty:%d, size:%d, capacity:%d\n", buffer.full(),
         buffer.empty(), buffer.size(), buffer.capacity());
}

int main() {
  CircularBuffer buffer(test_size);
  add_test_1(buffer);
  pop_test_1(buffer);
  test_2(buffer);
}