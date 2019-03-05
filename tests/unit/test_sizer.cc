
#include <gtest/gtest.h>

#include "test_harness.h"

#include "serdes_headers.h"

namespace serdes { namespace tests { namespace unit {

struct TestSizer : TestHarness { };

struct Test1 {
  int a;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

struct Test2 {
  int a, b;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
    s | b;
  }
};

struct Test3 {
  int a, b, c;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
    s | b;
    s | c;
  }
};

struct Test4 {
  int a, b, c;
};

template <typename Serializer>
void serialize(Serializer& s, Test4 t) {
  s | t.a;
  s | t.b;
  s | t.c;
}

TEST_F(TestSizer, test_sizer_1) {
  using namespace serdes;

  Test1 t;
  auto const& size = Dispatch<Test1>::sizeType(t);
  EXPECT_EQ(size, sizeof(int));
}

TEST_F(TestSizer, test_sizer_2) {
  using namespace serdes;

  Test2 t;
  auto const& size = Dispatch<Test2>::sizeType(t);
  EXPECT_EQ(size, sizeof(int)*2);
}

TEST_F(TestSizer, test_sizer_3) {
  using namespace serdes;

  Test3 t;
  auto const& size = Dispatch<Test3>::sizeType(t);
  EXPECT_EQ(size, sizeof(int)*3);
}

TEST_F(TestSizer, test_sizer_4) {
  using namespace serdes;

  Test4 t;
  auto const& size = Dispatch<Test4>::sizeType(t);
  EXPECT_EQ(size, sizeof(int)*3);
}

}}} // end namespace serdes::tests::unit
