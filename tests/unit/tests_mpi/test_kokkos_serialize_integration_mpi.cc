/*
//@HEADER
// *****************************************************************************
//
//                   test_kokkos_serialize_integration_mpi.cc
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

#include "test_kokkos_integration_commons.h"
#include "test_kokkos_1d_commons.h"
#include "tests_mpi/test_commons_mpi.h"

struct KokkosIntegrateTestMPI : KokkosBaseTest { };

TEST_F(KokkosIntegrateTestMPI, test_integrate_1) {
  using namespace serialization::interface;
  using DataType = Data;

  // Init test_data, check for golden status before and after serialization
  DataType test_data(DataConsTag);
  Data::checkIsGolden(test_data);

  // Test the respect of the max rank needed for the test'
  EXPECT_EQ(MPIEnvironment::isRankValid(1), true);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    auto ret = serialize<DataType>(test_data);
    int dataSize = ret->getSize();
    MPI_Send( &dataSize, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    char * viewBuffer = ret->getBuffer();
    MPI_Send(viewBuffer, dataSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
  }
  else  {
    int dataSize;
    MPI_Recv( & dataSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * recv = (char *) malloc(dataSize);

    MPI_Recv(recv, dataSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto out_data = deserialize<DataType>(recv, dataSize);
    auto const& out_data_ref = *out_data;

    Data::checkIsGolden(out_data_ref);
    Data::checkIsGolden(test_data);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Kokkos::View unit test with no-data (null), default constructor
/////////////////////// ////////////////////////////////////////////////////////

struct KokkosNullTestMPI : KokkosBaseTest { };

TEST_F(KokkosNullTestMPI, test_null_1) {
  using namespace serialization::interface;
  using ViewType = Kokkos::View<int*>;

  // Default construct
  ViewType test_data = {};

  serializeAnyMPI<ViewType>(test_data, &compare1d<ViewType>);
}

#endif
