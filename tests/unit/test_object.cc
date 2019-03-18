
#include <gtest/gtest.h>

#include "test_harness.h"

#include "serdes_headers.h"
#include "serialization_library_headers.h"

#include <vector>
#include <cstdio>

namespace serdes { namespace tests { namespace unit {

struct TestObject : TestHarness { };

static constexpr int const x_val = 29;
static constexpr int const y_val = 31;
static constexpr int const z_val = 37;
static constexpr int const u_val = 43;
static constexpr int const vec_val = 41;

struct UserObject3 {
  using isByteCopyable = std::true_type;

  int u;

  void init() {
    u = u_val;
  }

  void check() {
    EXPECT_EQ(u, u_val);
  }
};

struct UserObject2 {
  using isByteCopyable = std::false_type;

  int x, y;
  std::vector<int> vec;
  UserObject3 obj;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | x | y | vec | obj;
  }

  void init() {
    x = x_val;
    y = y_val;
    vec.push_back(vec_val);
    obj.init();
  }

  void check() {
    EXPECT_EQ(x, x_val);
    EXPECT_EQ(y, y_val);
    EXPECT_EQ(vec.size(), 1UL);
    EXPECT_EQ(vec[0], vec_val);
    obj.init();
  }
};

struct UserObject1 {
  using isByteCopyable = std::false_type;

  int z;
  UserObject2 obj;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | z | obj;
  }

  void init() {
    z = z_val;
    obj.init();
  }

  void check() {
    EXPECT_EQ(z, z_val);
    obj.check();
  }
};

TEST_F(TestObject, test_bytecopy_trait) {
  using namespace ::serialization::interface;

  using TestType = UserObject1;
  TestType t;
  t.init();
  t.check();

  auto ret = serialization::interface::serialize<TestType>(t);

  auto tptr = serialization::interface::deserialize<TestType>(
    ret->getBuffer(), ret->getSize()
  );
  auto& t_final = *tptr;

  t_final.check();
}

}}} // end namespace serdes::tests::unit
