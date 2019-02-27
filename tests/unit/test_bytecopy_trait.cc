
#include <gtest/gtest.h>

#include "test_harness.h"

#include "serdes_headers.h"
#include "serialization_library_headers.h"

#include <vector>
#include <cstdio>

#define TEST_BYTE_DEBUG_PRINT 0

namespace serdes { namespace tests { namespace unit {

struct TestByteCopyTrait : TestHarness { };

struct ByteCopyStruct {
  using isByteCopyable = std::true_type;

  int x = 10, y = 20;
};

TEST_F(TestByteCopyTrait, test_bytecopy_trait) {
  using namespace ::serialization::interface;

  using TestType = ByteCopyStruct;
  TestType t{100,200};

  auto ret = serialization::interface::serialize<TestType>(t);

  #if TEST_BYTE_DEBUG_PRINT
    printf("buffer=%p, size=%ld\n", ret->getBuffer(), ret->getSize());
  #endif

  auto tptr = serialization::interface::deserialize<TestType>(
    ret->getBuffer(), ret->getSize()
  );
  auto& dest = *tptr;

  #if TEST_BYTE_DEBUG_PRINT
    printf("ByteCopyStruct {%d,%d}\n", dest.x, dest.y);
  #endif
}

}}} // end namespace serdes::tests::unit
