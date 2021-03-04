/*
//@HEADER
// *****************************************************************************
//
//                              test_container.cc
//                           DARMA Toolkit v. 1.0.0
//                 DARMA/checkpoint => Serialization Library
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/

#include <gtest/gtest.h>

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

namespace checkpoint { namespace tests { namespace unit {

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
static void testContainerOrdered(std::initializer_list<T> lst) {
  ContainerT c1{lst};
  auto ret = checkpoint::serialize(c1);

  auto t1 = checkpoint::deserialize<ContainerT>(ret->getBuffer());

  EXPECT_EQ(c1.size(), t1->size());

  testEqualityContainerOrdered(c1, *t1);
}

template <>
void testContainerOrdered<std::vector<int>, int>(
  std::initializer_list<int> lst
) {
  std::vector<int> c1{lst};
  std::size_t const reserved_capacity = 1000;
  c1.reserve(reserved_capacity);
  auto ret = checkpoint::serialize(c1);

  auto t1 = checkpoint::deserialize<std::vector<int>>(ret->getBuffer());

  EXPECT_EQ(c1.size(), t1->size());
  // capacity: reserve() can overallocate, so check for greater or equal
  EXPECT_GE(t1->capacity(), reserved_capacity);
  EXPECT_GE(t1->capacity(), c1.capacity());

  testEqualityContainerOrdered(c1, *t1);
}

template <typename ContainerT, typename T>
static void testContainerUnordered(std::initializer_list<T> lst) {
  ContainerT c1{lst};
  float const custom_max_load_factor = .75;
  c1.max_load_factor(custom_max_load_factor);
  std::size_t const custom_bucket_count = 1000;
  c1.rehash(custom_bucket_count);

  auto ret = checkpoint::serialize(c1);
  auto t1 = checkpoint::deserialize<ContainerT>(ret->getBuffer());

  EXPECT_EQ(c1.size(), t1->size());
  EXPECT_EQ(t1->max_load_factor(), custom_max_load_factor);
  EXPECT_EQ(t1->max_load_factor(), c1.max_load_factor());
  EXPECT_GE(t1->bucket_count(), custom_bucket_count);
  EXPECT_GE(t1->bucket_count(), c1.bucket_count());

  testEqualityContainerUnordered(c1, *t1);
}

TYPED_TEST_P(TestContainer, test_single_ordered_container) {
  using namespace checkpoint;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;

  testContainerOrdered<ContainerT, ValueT>({});

  testContainerOrdered<ContainerT, ValueT>(
    {(ValueT)1, (ValueT)2, (ValueT)3, (ValueT)4, (ValueT)5}
  );
}

TYPED_TEST_P(TestContainerUnordered, test_single_unordered_container) {
  using namespace checkpoint;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;

  testContainerUnordered<ContainerT, ValueT>({});

  testContainerUnordered<ContainerT, ValueT>(
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

INSTANTIATE_TYPED_TEST_CASE_P(Test_int, TestContainer, ContOrderedTypes<int>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int16_t, TestContainer, ContOrderedTypes<int16_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int32_t, TestContainer, ContOrderedTypes<int32_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int64_t, TestContainer, ContOrderedTypes<int64_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_double, TestContainer, ContOrderedTypes<double>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_float, TestContainer, ContOrderedTypes<float>,);

REGISTER_TYPED_TEST_CASE_P(TestContainerUnordered, test_single_unordered_container);

INSTANTIATE_TYPED_TEST_CASE_P(Test_int, TestContainerUnordered, ContUnorderedTypes<int>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int16_t, TestContainerUnordered, ContUnorderedTypes<int16_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int32_t, TestContainerUnordered, ContUnorderedTypes<int32_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_int64_t, TestContainerUnordered, ContUnorderedTypes<int64_t>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_double, TestContainerUnordered, ContUnorderedTypes<double>,);
INSTANTIATE_TYPED_TEST_CASE_P(Test_float, TestContainerUnordered, ContUnorderedTypes<float>,);

template <typename ContainerT>
struct TestMultiContainer : TestHarness { };

template <typename ContainerT>
struct TestMultiContainerUnordered : TestHarness { };

TYPED_TEST_CASE_P(TestMultiContainer);
TYPED_TEST_CASE_P(TestMultiContainerUnordered);

template <typename ContainerT, typename Pair>
static void testMultiContainerOrdered(std::initializer_list<Pair> lst) {
  ContainerT c1{lst};
  auto ret = checkpoint::serialize(c1);

  auto t1 = checkpoint::deserialize<ContainerT>(ret->getBuffer());

  EXPECT_EQ(c1.size(), t1->size());

  testEqualityContainerOrdered(c1, *t1);
}

template <typename ContainerT, typename Pair>
static void testMultiContainerUnordered(std::initializer_list<Pair> lst) {
  ContainerT c1{lst};
  float const custom_max_load_factor = .75;
  c1.max_load_factor(custom_max_load_factor);
  std::size_t const custom_bucket_count = 1000;
  c1.rehash(custom_bucket_count);

  auto ret = checkpoint::serialize(c1);
  auto t1 = checkpoint::deserialize<ContainerT>(ret->getBuffer());

  EXPECT_EQ(c1.size(), t1->size());
  EXPECT_EQ(t1->max_load_factor(), custom_max_load_factor);
  EXPECT_EQ(t1->max_load_factor(), c1.max_load_factor());
  EXPECT_GE(t1->bucket_count(), custom_bucket_count);
  EXPECT_GE(t1->bucket_count(), c1.bucket_count());

  testEqualityContainerUnordered(c1, *t1);
}

TYPED_TEST_P(TestMultiContainer, test_multi_container) {
  using namespace checkpoint;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  using FstValueT = typename ContainerT::value_type::first_type;
  using SndValueT = typename ContainerT::value_type::second_type;

  testMultiContainerOrdered<ContainerT, ValueT>({});

  testMultiContainerOrdered<ContainerT, ValueT>({
    {(FstValueT)1, (SndValueT)2},
    {(FstValueT)3, (SndValueT)4},
    {(FstValueT)5, (SndValueT)6},
    {(FstValueT)7, (SndValueT)8},
    {(FstValueT)9, (SndValueT)10},
  });
}

TYPED_TEST_P(TestMultiContainerUnordered, test_multi_container_unordered) {
  using namespace checkpoint;

  using ContainerT = TypeParam;
  using ValueT = typename ContainerT::value_type;
  using FstValueT = typename ContainerT::value_type::first_type;
  using SndValueT = typename ContainerT::value_type::second_type;

  testMultiContainerUnordered<ContainerT, ValueT>({});

  testMultiContainerUnordered<ContainerT, ValueT>({
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

INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int, TestMultiContainer, ContainerMultiTypes<int>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_double, TestMultiContainer, ContainerMultiTypes<double>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int64_t, TestMultiContainer, ContainerMultiTypes<int64_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int16_t, TestMultiContainer, ContainerMultiTypes<int16_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_float, TestMultiContainer, ContainerMultiTypes<float>, );

INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int, TestMultiContainer, ContainerMultiTypesDouble<int>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int64_t, TestMultiContainer, ContainerMultiTypesDouble<int64_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int16_t, TestMultiContainer, ContainerMultiTypesDouble<int16_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_float, TestMultiContainer, ContainerMultiTypesDouble<float>, );


INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_double, TestMultiContainerUnordered, ContainerMultiTypesUnordered<double>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int64_t, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int64_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_int16_t, TestMultiContainerUnordered, ContainerMultiTypesUnordered<int16_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMulti_float, TestMultiContainerUnordered, ContainerMultiTypesUnordered<float>, );

INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int64_t, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int64_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_int16_t, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<int16_t>, );
INSTANTIATE_TYPED_TEST_CASE_P(TestMultiDouble_float, TestMultiContainerUnordered, ContainerMultiTypesUnorderedDouble<float>, );

}}} // end namespace checkpoint::tests::unit
