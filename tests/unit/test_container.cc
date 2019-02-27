
#include <gtest/gtest.h>

#include "test_harness.h"

#include "serdes_headers.h"

namespace serdes { namespace tests { namespace unit {

template <typename ContainerT>
struct TestContainer : TestHarness { };

template <typename ContainerT>
struct TestContainerUnordered : TestHarness { };

TYPED_TEST_CASE_P(TestContainer);
TYPED_TEST_CASE_P(TestContainerUnordered);

template <typename ContainerT>
static void testEqualityContainerOrdered(ContainerT& c1, ContainerT& t1) {
  for (auto i = c1.begin(), j = t1.begin(), i_end = c1.end(), j_end = t1.end();
       i != i_end && j != j_end; i++, j++) {
    EXPECT_EQ(*i, *j);
  }
}

template <typename ContainerT>
static void testEqualityContainerUnordered(ContainerT& c1, ContainerT& t1) {
  for (auto&& elm1 : c1) {
    bool found = false;
    for (auto&& elm2 : t1) {
      if (elm1 == elm2) {
        found = true;
        break;
      }
    }
    EXPECT_EQ(found, true);
  }
}

template <typename ContainerT, typename T>
static void testContainer(bool is_ordered, std::initializer_list<T> lst) {
  ContainerT c1{lst};
  auto s1 = serdes::serializeType(c1);
  auto const& buf1 = std::get<0>(s1);

  auto tptr = serdes::deserializeType<ContainerT>(
    buf1->getBuffer(), std::get<1>(s1)
  );
  auto& t1 = *tptr;

  EXPECT_EQ(c1.size(), t1.size());

  if (is_ordered) {
    testEqualityContainerOrdered(c1, t1);
  } else {
    testEqualityContainerUnordered(c1, t1);
  }

  delete tptr;
}

TYPED_TEST_P(TestContainer, test_single_ordered_container) {
  using namespace serdes;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  testContainer<ContainerT, ValueT>(
    true,
    {(ValueT)1, (ValueT)2, (ValueT)3, (ValueT)4, (ValueT)5}
  );
}

TYPED_TEST_P(TestContainerUnordered, test_single_unordered_container) {
  using namespace serdes;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  testContainer<ContainerT, ValueT>(
    false,
    {(ValueT)1, (ValueT)2, (ValueT)3, (ValueT)4, (ValueT)5}
  );
}

template <typename T>
using ContOrderedTypes = ::testing::Types<
  std::vector<T>,
  std::list<T>,
  std::deque<T>,
  std::set<T>,
  std::multiset<T>
>;

template <typename T>
using ContUnorderedTypes = ::testing::Types<
  std::unordered_set<T>,
  std::unordered_multiset<T>
>;

REGISTER_TYPED_TEST_CASE_P(TestContainer, test_single_ordered_container);

INSTANTIATE_TYPED_TEST_CASE_P(Test_int, TestContainer, ContOrderedTypes<int>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int16_t, TestContainer, ContOrderedTypes<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int32_t, TestContainer, ContOrderedTypes<int32_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int64_t, TestContainer, ContOrderedTypes<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_double, TestContainer, ContOrderedTypes<double>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_float, TestContainer, ContOrderedTypes<float>);

REGISTER_TYPED_TEST_CASE_P(TestContainerUnordered, test_single_unordered_container);

INSTANTIATE_TYPED_TEST_CASE_P(Test_int, TestContainerUnordered, ContUnorderedTypes<int>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int16_t, TestContainerUnordered, ContUnorderedTypes<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int32_t, TestContainerUnordered, ContUnorderedTypes<int32_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int64_t, TestContainerUnordered, ContUnorderedTypes<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_double, TestContainerUnordered, ContUnorderedTypes<double>);
INSTANTIATE_TYPED_TEST_CASE_P(Test_float, TestContainerUnordered, ContUnorderedTypes<float>);

template <typename ContainerT>
struct TestMultiContainer : TestHarness { };

template <typename ContainerT>
struct TestMultiContainerUnordered : TestHarness { };

TYPED_TEST_CASE_P(TestMultiContainer);
TYPED_TEST_CASE_P(TestMultiContainerUnordered);

template <typename ContainerT, typename Pair>
static void testMultiContainer(bool is_ordered, std::initializer_list<Pair> lst) {
  ContainerT c1{lst};
  auto s1 = serdes::serializeType(c1);
  auto const& buf1 = std::get<0>(s1);

  auto tptr = serdes::deserializeType<ContainerT>(
    buf1->getBuffer(), std::get<1>(s1)
  );
  auto& t1 = *tptr;

  EXPECT_EQ(c1.size(), t1.size());

  if (is_ordered) {
    testEqualityContainerOrdered(c1, t1);
  } else {
    testEqualityContainerUnordered(c1, t1);
  }

  delete tptr;
}

TYPED_TEST_P(TestMultiContainer, test_multi_container) {
  using namespace serdes;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  using FstValueT = typename ContainerT::value_type::first_type;
  using SndValueT = typename ContainerT::value_type::second_type;

  testMultiContainer<ContainerT, ValueT>(true, {
    {(FstValueT)1, (SndValueT)2},
    {(FstValueT)3, (SndValueT)4},
    {(FstValueT)5, (SndValueT)6},
    {(FstValueT)7, (SndValueT)8},
    {(FstValueT)9, (SndValueT)10},
  });
}

TYPED_TEST_P(TestMultiContainerUnordered, test_multi_container_unordered) {
  using namespace serdes;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  using FstValueT = typename ContainerT::value_type::first_type;
  using SndValueT = typename ContainerT::value_type::second_type;

  testMultiContainer<ContainerT, ValueT>(false, {
    {(FstValueT)1, (SndValueT)2},
    {(FstValueT)3, (SndValueT)4},
    {(FstValueT)5, (SndValueT)6},
    {(FstValueT)7, (SndValueT)8},
    {(FstValueT)9, (SndValueT)10},
  });
}

template <typename T, typename U = T>
using ContainerMultiTypes = ::testing::Types<
  std::map<T, U>,
  std::multimap<T, U>
>;

template <typename T>
using ContainerMultiTypesDouble = ContainerMultiTypes<T, double>;

template <typename T, typename U = T>
using ContainerMultiTypesUnordered = ::testing::Types<
  std::unordered_map<T, U>,
  std::unordered_multimap<T, U>
>;

template <typename T>
using ContainerMultiTypesUnorderedDouble = ContainerMultiTypesUnordered<T, double>;

REGISTER_TYPED_TEST_CASE_P(TestMultiContainer, test_multi_container);
REGISTER_TYPED_TEST_CASE_P(TestMultiContainerUnordered, test_multi_container_unordered);

INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int, TestMultiContainer, ContainerMultiTypes<int>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_double, TestMultiContainer, ContainerMultiTypes<double>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int64_t, TestMultiContainer, ContainerMultiTypes<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int16_t, TestMultiContainer, ContainerMultiTypes<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_float, TestMultiContainer, ContainerMultiTypes<float>);

INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int, TestMultiContainer, ContainerMultiTypesDouble<int>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int64_t, TestMultiContainer, ContainerMultiTypesDouble<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int16_t, TestMultiContainer, ContainerMultiTypesDouble<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_float, TestMultiContainer, ContainerMultiTypesDouble<float>);


INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_double, TestMultiContainerUnordered, ContainerMultiTypesUnordered<double>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int64_t, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int16_t, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_float, TestMultiContainerUnordered, ContainerMultiTypesUnordered<float>);

INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int64_t, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int64_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int16_t, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int16_t>);
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_float, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<float>);

}}} // end namespace serdes::tests::unit
