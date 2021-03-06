#include "suduo/base/Timestamp.h"

#include <stdio.h>

#include <vector>

using suduo::Timestamp;

void passByConstReference(const Timestamp& x) {
  printf("%s\n", x.to_string().c_str());
}

void passByValue(Timestamp x) { printf("%s\n", x.to_string().c_str()); }

void benchmark() {
  const int kNumber = 1000 * 1000;

  std::vector<Timestamp> stamps;
  stamps.reserve(kNumber);
  for (int i = 0; i < kNumber; ++i) {
    stamps.push_back(Timestamp::now());
  }
  printf("%s\n", stamps.front().to_string().c_str());
  printf("%s\n", stamps.back().to_string().c_str());
  printf("%f\n", (stamps.back() - stamps.front()).get_seconds_in_double());

  int increments[100] = {0};
  int64_t start = stamps.front().get_microseconds_in_int64();
  for (int i = 1; i < kNumber; ++i) {
    int64_t next = stamps[i].get_microseconds_in_int64();
    int64_t inc = next - start;
    start = next;
    // printf("%ld\n", start);
    if (inc < 0) {
      printf("reverse!\n");
    } else if (inc < 100) {
      ++increments[inc];
    } else {
      printf("big gap %d\n", static_cast<int>(inc));
    }
  }

  for (int i = 0; i < 100; ++i) {
    printf("%2d: %d\n", i, increments[i]);
  }
}

int main() {
  Timestamp now(Timestamp::now());
  printf("%s\n", now.to_string().c_str());
  passByValue(now);
  passByConstReference(now);
  benchmark();
}
