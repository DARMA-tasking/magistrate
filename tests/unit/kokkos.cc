#include "test_harness.h"
#include <Kokkos_View.hpp>
#include <Kokkos_Serial.hpp>
#include <container/view_serialize.h>
#include <container/string_serialize.h>
#include <container/vector_serialize.h>
#include <container/tuple_serialize.h>
#include <serializers/packer.h>
#include <serializers/unpacker.h>
#include <serdes_headers.h>

//

TEST(KOKKOS, stringTest)
{
  std::string str{"hello"};
  serdes::Packer s(1000);
  serdes::serialize(s, str);

  std::string outstr;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outstr);

  EXPECT_EQ(str, outstr);
}

TEST(KOKKOS, vectorOfStringTest)
{
  std::vector<std::string> vec;
  vec.emplace_back("hello");
  vec.emplace_back("world");

  serdes::Packer s(1000);
  serdes::serialize(s, vec);

  std::vector<std::string> outvec;
  serdes::Unpacker us(s.getBuffer(),1000);
  serdes::serialize(us, outvec);
  EXPECT_EQ(vec.size(), outvec.size());

  for(auto i = 0; i < vec.size(); ++i)
  {
    EXPECT_EQ(vec.at(i), outvec.at(i));
  }
}

TEST(KOKKOS, kokkosViewTest)
{
//    const size_t N = 5;
//    Kokkos::View<double*[3]> b ("another label", N);

//    serdes::Packer s(1000);
//    serdes::serialize(s, b);
}

