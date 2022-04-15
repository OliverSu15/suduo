#include "suduo/net/Buffer.h"

//#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using std::string;
using suduo::net::Buffer;

BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve) {
  Buffer buf;
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);

  const string str(200, 'x');
  buf.append(str);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), str.size());
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - str.size());
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);

  const string str2 = buf.retrieve_as_string(50);
  BOOST_CHECK_EQUAL(str2.size(), 50);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - str.size());
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(),
                    Buffer::pre_append_size + str2.size());
  BOOST_CHECK_EQUAL(str2, string(50, 'x'));

  buf.append(str);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 2 * str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 2 * str.size());
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(),
                    Buffer::pre_append_size + str2.size());

  const string str3 = buf.retrieve_all_as_string();
  BOOST_CHECK_EQUAL(str3.size(), 350);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);
  BOOST_CHECK_EQUAL(str3, string(350, 'x'));
}

BOOST_AUTO_TEST_CASE(testBufferGrow) {
  Buffer buf;
  buf.append(string(400, 'y'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 400);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 400);

  buf.retrieve(50);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 350);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 400);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size + 50);

  buf.append(string(1000, 'z'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 1350);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(),
                    Buffer::pre_append_size + 50);  // FIXME

  buf.retrieve_all();
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), 1400);  // FIXME
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);
}

BOOST_AUTO_TEST_CASE(testBufferInsideGrow) {
  Buffer buf;
  buf.append(string(800, 'y'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 800);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 800);

  buf.retrieve(500);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 300);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 800);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size + 500);

  buf.append(string(300, 'z'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 600);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 600);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);
}

BOOST_AUTO_TEST_CASE(testBufferShrink) {
  Buffer buf;
  buf.append(string(2000, 'y'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 2000);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);

  buf.retrieve(1500);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 500);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size + 1500);

  buf.shrink();
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 500);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 500);
  BOOST_CHECK_EQUAL(buf.retrieve_all_as_string(), string(500, 'y'));
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);
}

BOOST_AUTO_TEST_CASE(testBufferPrepend) {
  Buffer buf;
  buf.append(string(200, 'y'));
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 200);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 200);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size);

  int x = 0;
  buf.prepend(&x, sizeof x);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 204);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size - 200);
  BOOST_CHECK_EQUAL(buf.pre_append_bytes(), Buffer::pre_append_size - 4);
}

BOOST_AUTO_TEST_CASE(testBufferReadInt) {
  Buffer buf;
  buf.append("HTTP");

  BOOST_CHECK_EQUAL(buf.readable_bytes(), 4);
  BOOST_CHECK_EQUAL(buf.peek_int_8(), 'H');
  int top16 = buf.peek_int_16();
  BOOST_CHECK_EQUAL(top16, 'H' * 256 + 'T');
  BOOST_CHECK_EQUAL(buf.peek_int_32(), top16 * 65536 + 'T' * 256 + 'P');

  BOOST_CHECK_EQUAL(buf.peek_int_8(), 'H');
  BOOST_CHECK_EQUAL(buf.peek_int_16(), 'T' * 256 + 'T');
  BOOST_CHECK_EQUAL(buf.peek_int_8(), 'P');
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.writeadble_bytes(), Buffer::init_size);

  buf.append_int_8(-1);
  buf.append_int_16(-2);
  buf.append_int_32(-3);
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 7);
  BOOST_CHECK_EQUAL(buf.peek_int_8(), -1);
  BOOST_CHECK_EQUAL(buf.peek_int_16(), -2);
  BOOST_CHECK_EQUAL(buf.peek_int_32(), -3);
}

BOOST_AUTO_TEST_CASE(testBufferFindEOL) {
  Buffer buf;
  buf.append(string(100000, 'x'));
  const char* null = NULL;
  BOOST_CHECK_EQUAL(buf.find_EOL(), null);
  BOOST_CHECK_EQUAL(buf.find_EOL(buf.peek() + 90000), null);
}

void output(Buffer&& buf, const void* inner) {
  Buffer newbuf(std::move(buf));
  // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.peek());
  BOOST_CHECK_EQUAL(inner, newbuf.peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
BOOST_AUTO_TEST_CASE(testMove) {
  Buffer buf;
  buf.append("muduo", 5);
  const void* inner = buf.peek();
  // printf("Buffer at %p, inner %p\n", &buf, inner);
  output(std::move(buf), inner);
}
