
#include "serdes_headers.h"

#include <cstdio>

namespace serdes { namespace examples {

struct MyTest {
  int a = 29, b = 31;

  MyTest(int const) { }
  MyTest() = delete;

  static MyTest& reconstruct(void* buf) {
    printf("MyTest reconstruct\n");
    MyTest* a = new (buf) MyTest(100);
    return *a;
  }

  void print() {
    printf("MyTest: a=%d, b=%d\n", a, b);
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest serialize\n");
    s | a;
    s | b;
  }
};

}} // end namespace serdes::examples

int main(int, char**) {
  using namespace serdes::examples;

  MyTest my_test_inst{10};

  my_test_inst.print();

  auto serialized = serdes::serializeType<MyTest>(my_test_inst);

  auto const& buf = std::get<0>(serialized);
  auto const& buf_size = std::get<1>(serialized);

  printf("ptr=%p, size=%ld\n", buf->getBuffer(), buf_size);

  auto tptr = serdes::deserializeType<MyTest>(buf->getBuffer(), buf_size);
  auto& t = *tptr;

  t.print();

  delete tptr;

  return 0;
}
