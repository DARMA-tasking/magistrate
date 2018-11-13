
#include "test_harness.h"

#include <serdes_headers.h>
#include <serialization_library_headers.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdio>

namespace serdes { namespace tests { namespace unit {

template <typename T>
struct TestReconstruct : TestHarness { };

TYPED_TEST_CASE_P(TestReconstruct);

static constexpr int const x_val = 29;
static constexpr int const y_val = 31;
static constexpr int const z_val = 37;
static constexpr int const u_val = 43;
static constexpr int const vec_val = 41;

/*
 * Unit test with `UserObjectA` with a default constructor for deserialization
 * purposes
 */

struct UserObjectA {
  UserObjectA() = default;
  explicit UserObjectA(int in_u) : u_(in_u) { }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

/*
 * Unit test with `UserObjectB` with non-intrusive reconstruct for
 * deserialization purposes
 */

struct UserObjectB {
  explicit UserObjectB(int in_u) : u_(in_u) { }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

template <typename SerializerT>
void reconstruct(SerializerT& s, UserObjectB*& obj, void* buf) {
  obj = new (buf) UserObjectB(100);
}

/*
 * Unit test with `UserObjectC` with non-intrusive reconstruct for
 * deserialization purposes in the serdes namespace (ADL check)
 */

struct UserObjectC {
  explicit UserObjectC(int in_u) : u_(std::to_string(in_u)) { }

public:
  void check() {
    EXPECT_EQ(u_, std::to_string(u_val));
  }

  template <typename SerializerT>
  friend void serdes::serialize(SerializerT&, UserObjectC&);

  template <typename SerializerT>
  friend void serdes::reconstruct(SerializerT&, UserObjectC*&, void*);

private:
  std::string u_ = {};
};

}}} // end namespace serdes::tests::unit

namespace serdes {

// This using declaration is only used for convenience
using UserObjType = serdes::tests::unit::UserObjectC;

template <typename SerializerT>
void reconstruct(SerializerT& s, UserObjType*& obj, void* buf) {
  obj = new (buf) UserObjType(100);
}

template <typename SerializerT>
void serialize(SerializerT& s, UserObjType& x) {
  s | x.u_;
}

} /* end namespace serdes */

namespace serdes { namespace tests { namespace unit {

/*
 * Unit test with `UserObjectD` with intrusive reconstruct for deserialization
 * purposes
 */

struct UserObjectD {
  explicit UserObjectD(int in_u) : u_(in_u) { }

  static UserObjectD& reconstruct(void* buf) {
    auto t = new (buf) UserObjectD(100);
    return *t;
  }

  void check() {
    EXPECT_EQ(u_, u_val);
  }

  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | u_;
  }

  int u_;
};

/*
 * General test of serialization/deserialization for input object types
 */

TYPED_TEST_P(TestReconstruct, test_reconstruct_multi_type) {
  namespace ser = serialization::interface;

  using TestType = TypeParam;

  TestType in(u_val);
  in.check();

  auto ret = ser::serialize<TestType>(in);
  auto out = ser::deserialize<TestType>(std::move(ret));

  out->check();
}

using ConstructTypes = ::testing::Types<
  UserObjectA,
  UserObjectB,
  UserObjectC
>;

REGISTER_TYPED_TEST_CASE_P(TestReconstruct, test_reconstruct_multi_type);
INSTANTIATE_TYPED_TEST_CASE_P(Test_reconstruct, TestReconstruct, ConstructTypes);

}}} // end namespace serdes::tests::unit
