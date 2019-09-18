/*
//@HEADER
// *****************************************************************************
//
//                         test_vector_bool_serialize.cc
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
#include "serialization_library_headers.h"

namespace serdes { namespace tests { namespace unit {

struct BoolVectorTest : TestHarness { };

TEST_F(BoolVectorTest, test_bool_vector) {
  std::vector<bool> boolVector;
  boolVector.push_back(false);
  boolVector.push_back(true);
  boolVector.push_back(false);
  boolVector.push_back(false);
  boolVector.push_back(true);

  auto ret = serialization::interface::serialize<std::vector<bool>>(boolVector);

  #if TEST_BYTE_DEBUG_PRINT
    printf("buffer=%p, size=%ld\n", ret->getBuffer(), ret->getSize());
  #endif

  auto tptr = serialization::interface::deserialize<std::vector<bool>>(
   ret->getBuffer(), ret->getSize()
  );

   auto& dest = *tptr;
   for (auto it = dest.cbegin(); it != dest.cend(); ++it) {
    std::cout << *it << std::endl;
   }

#if TEST_BYTE_DEBUG_PRINT
  auto& dest = *tptr;
  printf("ByteCopyStruct {%d,%d}\n", dest.x, dest.y);
#endif
}

}}} // end namespace serdes::tests::unit
