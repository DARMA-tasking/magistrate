
#include "serdes_headers.h"

#include <cstdio>

namespace serdes { namespace examples {

struct MyTest2 {
  int c = 41;

  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest2 serialize\n");
    s | c;
  }

  void print() {
    printf("\t MyTest2: c=%d\n", c);
  }
};

struct MyTest {
  int a = 29, b = 31;
  MyTest2 my_test_2;

  MyTest() = default;

  void print() {
    printf("MyTest: a=%d, b=%d\n", a, b);
    my_test_2.print();
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    printf("MyTest serialize\n");
    s | a;
    s | b;
    s | my_test_2;
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

  auto& t = serdes::deserializeType<MyTest>(buf->getBuffer(), buf_size);

  t.print();

  return 0;
}
