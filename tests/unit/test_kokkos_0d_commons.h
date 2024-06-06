/*
//@HEADER
// *****************************************************************************
//
//                           test_kokkos_0d_commons.h
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

#if !defined INCLUDED_UNIT_TEST_KOKKOS_0D_COMMONS_H
#define INCLUDED_UNIT_TEST_KOKKOS_0D_COMMONS_H

#include "test_commons.h"

template <typename ViewT>
static void compare0d(ViewT const& k1, ViewT const& k2) {
  compareBasic(k1,k2);
  EXPECT_EQ(k1.operator()(), k2.operator()());
}

// 0-D initialization
template <typename T, typename... Args>
static inline void init0d(Kokkos::View<T,Args...> const& v) {
  v.operator()() = 29;
}

#if DO_UNIT_TESTS_FOR_VIEW

///////////////////////////////////////////////////////////////////////////////
// 0-D Kokkos::View Tests (a single value)
///////////////////////////////////////////////////////////////////////////////

using Test0DTypes = testing::Types<
  int      , const int      ,
  double   , const double   ,
  float    , const float    ,
  int32_t  , const int32_t  ,
  int64_t  , const int64_t  ,
  unsigned , const unsigned ,
  long     , const long     ,
  long long, const long long
>;

#endif

#endif /*INCLUDED_UNIT_TEST_KOKKOS_0D_COMMONS_H*/
