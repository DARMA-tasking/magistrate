/*
//@HEADER
// *****************************************************************************
//
//                         test_kokkos_serialize_0d.cc
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

#include "test_harness.h"
#include "test_commons.h"
#include "test_kokkos_0d_commons.h"

template <typename ParamT> struct KokkosViewTest0D : KokkosViewTest<ParamT> { };

TYPED_TEST_CASE_P(KokkosViewTest0D);

TYPED_TEST_P(KokkosViewTest0D, test_0d_any) {
  using namespace checkpoint;

  using DataType          = TypeParam;
  using ViewType          = Kokkos::View<DataType>;
  using NonConstT         = typename ViewType::traits::non_const_data_type;
  using NonConstViewType  = Kokkos::View<NonConstT>;
  using ConstT            = typename ViewType::traits::const_data_type;
  using ConstViewType     = Kokkos::View<ConstT>;

  NonConstViewType in_view("test");

  init0d(in_view);

  if (std::is_same<NonConstViewType, ViewType>::value) {
    serializeAny<NonConstViewType>(in_view, &compare0d<NonConstViewType>);
  } else {
    ConstViewType const_in_view = in_view;
    serializeAny<ConstViewType>(const_in_view, &compare0d<ConstViewType>);
  }
}

REGISTER_TYPED_TEST_CASE_P(KokkosViewTest0D, test_0d_any);

#if DO_UNIT_TESTS_FOR_VIEW

INSTANTIATE_TYPED_TEST_CASE_P(test_0d, KokkosViewTest0D, Test0DTypes, );

#endif

#endif
