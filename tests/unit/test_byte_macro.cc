
#include <gtest/gtest.h>

#include "test_harness.h"

#include "serdes_headers.h"
#include "serialization_library_headers.h"

#include <vector>
#include <cstdio>

namespace userTest {

static constexpr int const my_val_init = 29;

struct UserObject1 {
  int my_val;

  void init() {
    my_val = my_val_init;
  }

  void check() {
    EXPECT_EQ(my_val, my_val_init);
  }
};

} // end namespace userTest

SERDES_BYTES(::userTest::UserObject1);

namespace serdes { namespace tests { namespace unit {

struct TestByteMacro : TestHarness { };

TEST_F(TestByteMacro, test_bytecopy_trait) {
  using namespace ::serialization::interface;

  using TestType = ::userTest::UserObject1;
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
