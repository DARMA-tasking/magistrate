
#include "serdes_headers.h"

#include <cstdio>

namespace serdes { namespace examples {

struct MyTest3 {
  int a = 1, b = 2 , c = 3;

  void print() {
    printf("MyTest3: a=%d, b=%d, c=%d\n", a, b, c);
  }
};

template <typename Serializer>
void serialize(Serializer& s, MyTest3& my_test3) {
  s | my_test3.a;
  s | my_test3.b;
  s | my_test3.c;
}

}} // end namespace serdes::examples

int main(int, char**) {
  using namespace serdes::examples;

  MyTest3 my_test3;
  my_test3.print();

  auto serialized = serdes::serializeType<MyTest3>(my_test3);

  auto const& buf = std::get<0>(serialized);
  auto const& buf_size = std::get<1>(serialized);

  printf("ptr=%p, size=%ld\n", buf->getBuffer(), buf_size);

  auto& t = serdes::deserializeType<MyTest3>(buf->getBuffer(), buf_size);

  t.print();

  return 0;
}
