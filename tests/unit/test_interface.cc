
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_harness.h"

#include "serdes_headers.h"
#include "serialization_library_headers.h"

#include <vector>
#include <cstdio>

#define TEST_INTERFACE_DEBUG_PRINT 0

namespace serdes { namespace tests { namespace unit {

struct TestInterface : TestHarness { };

TEST_F(TestInterface, test_serialize) {
  using namespace ::serialization::interface;

  using TestType = std::vector<int>;
  TestType vec{1,2,3,100,200};

  #if TEST_INTERFACE_DEBUG_PRINT
    for (auto&& elm : vec) {
      printf("vec elm: %d\n", elm);
    }
  #endif

  auto ret = serialization::interface::serialize<TestType>(vec);

  #if TEST_INTERFACE_DEBUG_PRINT
    printf("buffer=%p, size=%ld\n", ret->getBuffer(), ret->getSize());
  #endif

  auto vec_ptr = serialization::interface::deserialize<TestType>(
    ret->getBuffer(), ret->getSize()
  );
  auto& des_vec = *vec_ptr;

  #if TEST_INTERFACE_DEBUG_PRINT
    for (auto&& elm : des_vec) {
      printf("des_vec elm: %d\n", elm);
    }
  #endif

  EXPECT_EQ(des_vec.size(), vec.size());
  for (auto i = 0; i < des_vec.size(); i++) {
    EXPECT_EQ(des_vec[i], vec[i]);
  }
}

}}} // end namespace serdes::tests::unit
