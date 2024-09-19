/*
//@HEADER
// *****************************************************************************
//
//                         test_kokkos_serialize_nd.cc
//                 DARMA/magistrate => Serialization Library
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

// N-D dimension comparison
template <typename ViewT>
static void compareND(ViewT const& k1, ViewT const& k2) {
  checkpoint::ViewEquality<ViewT>::template compare<GTestEquality>(k1,k2);
}

template <typename LayoutT>
inline LayoutT layout4d(lsType d1, lsType d2, lsType d3, lsType d4) {
  return LayoutT{d1,d2,d3,d4};
}

// Be very careful here: Kokkos strides must be laid out properly for this to be
// correct. This computes a simple strided layout. Strides are absolute for each
// dimension and shall calculated as such.
template <>
inline Kokkos::LayoutStride layout4d(lsType d1,lsType d2,lsType d3,lsType d4) {
  return Kokkos::LayoutStride{d1,1,d2,d1,d3,d1*d2,d4,d1*d2*d3};
}

#endif
