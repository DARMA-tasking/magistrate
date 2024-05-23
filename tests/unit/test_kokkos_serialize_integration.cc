/*
//@HEADER
// *****************************************************************************
//
//                     test_kokkos_serialize_integration.cc
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
#if MAGISTRATE_KOKKOS_ENABLED

#include "test_harness.h"
#include "test_commons.h"

#include "test_kokkos_integration_commons.h"

struct KokkosIntegrateTest : KokkosBaseTest { };

TEST_F(KokkosIntegrateTest, test_integrate_1) {
  using namespace checkpoint;
  using DataType = Data;

  // Init test_data, check for golden status before and after serialization
  DataType test_data(DataConsTag);
  Data::checkIsGolden(test_data);

  auto ret = serialize<DataType>(test_data);
  auto out = deserialize<DataType>(std::move(ret));

  Data::checkIsGolden(*out);
  Data::checkIsGolden(test_data);
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with no-data (null), default constructor
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosNullTest : KokkosBaseTest { };

TEST_F(KokkosNullTest, test_null_1) {
  using namespace checkpoint;
  using ViewType = Kokkos::View<int*>;

  // Default construct
  ViewType test_data = {};

  auto ret = serialize<ViewType>(test_data);
  deserialize<ViewType>(std::move(ret));
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with view of view
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosViewOfVIewTest : KokkosBaseTest { };

TEST_F(KokkosViewOfVIewTest, test_view_of_view_init_1) {
  using namespace checkpoint;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data = Kokkos::View<Kokkos::View<double*>[3]>("test");
  test_data(0) = Kokkos::View<double*>();
  test_data(1) = Kokkos::View<double*>();
  test_data(2) = Kokkos::View<double*>();

  auto ret = serialize<ViewType>(test_data);
  deserialize<ViewType>(std::move(ret));
}

TEST_F(KokkosViewOfVIewTest, test_view_of_view_init_2) {
  using namespace checkpoint;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data = Kokkos::View<Kokkos::View<double*>[3]>("test");

  auto ret = serialize<ViewType>(test_data);
  deserialize<ViewType>(std::move(ret));
}

TEST_F(KokkosViewOfVIewTest, test_view_of_view_uninit_1) {
  using namespace checkpoint;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data;

  auto ret = serialize<ViewType>(test_data);
  deserialize<ViewType>(ret->getBuffer());
}

TEST_F(KokkosViewOfVIewTest, test_view_of_view_uninit_fix) {
  using namespace checkpoint;
  using ViewType = Kokkos::View<Kokkos::View<double*>[3]>;

  // Default construct
  ViewType test_data;
  bool canUseLabel = test_data.use_count() > 0;
  EXPECT_EQ(canUseLabel, false);

  ViewType test_data_initialized("");
  canUseLabel = test_data_initialized.use_count() > 0;
  EXPECT_EQ(canUseLabel, true);
  test_data_initialized(0).label();

  test_data_initialized(0) = Kokkos::View<double*>();
  canUseLabel = test_data_initialized.use_count() > 0;
  test_data_initialized(0).label();
  EXPECT_EQ(canUseLabel, true);
}

#endif
