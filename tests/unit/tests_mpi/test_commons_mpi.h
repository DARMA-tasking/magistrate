/*
//@HEADER
// *****************************************************************************
//
//                              test_commons_mpi.h
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
#ifndef TEST_COMMONS_MPI_H
#define TEST_COMMONS_MPI_H

#include "test_commons.h"
#include "test_kokkos_0d_commons.h"
#include "tests_mpi/mpi-init.h"

#include <functional>

#include <mpi.h>

namespace  {
template <typename T>
void serializeAnyMPI(T& view, std::function<void(T const&,T const&)> compare) {
  using namespace checkpoint;

  // Test the respect of the max rank needed for the test'
  EXPECT_EQ(MPIEnvironment::isRankValid(1), true);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (world_rank == 0) {
    auto ret = serialize<T>(view);
    int viewSize = ret->getSize();
    MPI_Send( &viewSize, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    char * viewBuffer = ret->getBuffer();
    MPI_Send(viewBuffer, viewSize, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
  }
  else  {
    int viewSize;
    MPI_Recv( & viewSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * recv = new char[viewSize];

    MPI_Recv(recv, viewSize, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    auto out_view = deserialize<T>(recv);
    auto const& out_view_ref = *out_view;

#if CHECKPOINT_USE_ND_COMPARE
    compareND(view, out_view_ref);
#else
    compare(view, out_view_ref);
#endif
  }
}
} // end namespace

#endif // TEST_COMMONS_MPI_H
