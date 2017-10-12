
#include "serdes_headers.h"

#include <cstdio>

namespace serdes { namespace examples {

struct TestReconstruct {
  int a = 29;

  TestReconstruct(int const) { }
  TestReconstruct() = delete;

  static TestReconstruct& reconstruct(void* buf) {
    auto a = new (buf) TestReconstruct(100);
    return *a;
  }

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

struct TestShouldFailReconstruct {
  int a = 29;

  TestShouldFailReconstruct(int const) { }
  TestShouldFailReconstruct() = delete;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

struct TestDefaultCons {
  int a = 29;

  TestDefaultCons() = default;

  template <typename Serializer>
  void serialize(Serializer& s) {
    s | a;
  }
};

struct TestNoSerialize {
  int a = 29;
};

}} // end namespace serdes::examples

#if HAS_DETECTION_COMPONENT
  #include "traits/serializable_traits.h"

  namespace serdes {

  using namespace examples;

  static_assert(
    SerializableTraits<TestReconstruct>::is_serializable,
    "Should be serializable"
  );
  static_assert(
    ! SerializableTraits<TestShouldFailReconstruct>::is_serializable,
    "Should not be serializable"
  );
  static_assert(
    SerializableTraits<TestDefaultCons>::is_serializable,
    "Should be serializable"
  );
  static_assert(
    ! SerializableTraits<TestNoSerialize>::is_serializable,
    "Should not be serializable"
  );

  } // end namespace serdes
#endif

int main(int, char**) {
  // Example is a compile-time test of serializability traits
  return 0;
}
