/*
//@HEADER
// *****************************************************************************
//
//                      test_kokkos_serialize_dynrankview.cc
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
#if KOKKOS_ENABLED_CHECKPOINT

#include "test_commons.h"
#include "test_kokkos_1d_commons.h"
#include "test_kokkos_2d_commons.h"
#include "test_kokkos_3d_commons.h"

#include <Kokkos_DynRankView.hpp>

template <typename ParamT>
struct KokkosDynRankViewTest1D : KokkosViewTest<ParamT> { };

template <typename ParamT>
struct KokkosDynRankViewTest2D : KokkosViewTest<ParamT> { };

template <typename ParamT>
struct KokkosDynRankViewTest3D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosDynRankViewTest1D);
TYPED_TEST_CASE_P(KokkosDynRankViewTest2D);
TYPED_TEST_CASE_P(KokkosDynRankViewTest3D);

TYPED_TEST_P(KokkosDynRankViewTest1D, test_1d_any) {
  using namespace checkpoint;

  using DataType = TypeParam;
  using ViewType = Kokkos::DynRankView<DataType>;

  static constexpr size_t const N = 241;

  ViewType in_view("test", N);

  init1d(in_view);
  serializeAny<ViewType>(in_view, &compare1d<ViewType>);
}

TYPED_TEST_P(KokkosDynRankViewTest2D, test_2d_any) {
  using namespace checkpoint;

  using DataType = TypeParam;
  using ViewType = Kokkos::DynRankView<DataType>;

  static constexpr size_t const N1 = 241;
  static constexpr size_t const N2 = 54;

  ViewType in_view("test2", N1, N2);

  init2d(in_view);
  serializeAny<ViewType>(in_view, &compare2d<ViewType>);
}

TYPED_TEST_P(KokkosDynRankViewTest3D, test_3d_any) {
  using namespace checkpoint;

  using DataType = TypeParam;
  using ViewType = Kokkos::DynRankView<DataType>;

  static constexpr size_t const N1 = 10;
  static constexpr size_t const N2 = 54;
  static constexpr size_t const N3 = 58;

  ViewType in_view("test2", N1, N2, N3);

  init3d(in_view);
  serializeAny<ViewType>(in_view, &compare3d<ViewType>);
}

REGISTER_TYPED_TEST_CASE_P(KokkosDynRankViewTest1D, test_1d_any);
REGISTER_TYPED_TEST_CASE_P(KokkosDynRankViewTest2D, test_2d_any);
REGISTER_TYPED_TEST_CASE_P(KokkosDynRankViewTest3D, test_3d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_dynrank_1, KokkosDynRankViewTest1D, DynRankViewTestTypes);
INSTANTIATE_TYPED_TEST_CASE_P(test_dynrank_2, KokkosDynRankViewTest2D, DynRankViewTestTypes);
INSTANTIATE_TYPED_TEST_CASE_P(test_dynrank_3, KokkosDynRankViewTest3D, DynRankViewTestTypes);

#endif

#endif
