/*
//@HEADER
// *****************************************************************************
//
//                              test_interface.cc
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

#include <gtest/gtest.h>

#include "test_harness.h"

#include <checkpoint/checkpoint.h>

#include <vector>
#include <cstdio>

#define TEST_INTERFACE_DEBUG_PRINT 0

namespace checkpoint { namespace tests { namespace unit {

struct TestInterface : TestHarness { };

TEST_F(TestInterface, test_serialize) {
  using namespace ::checkpoint;

  using TestType = std::vector<int>;
  TestType vec{1,2,3,100,200};

  #if TEST_INTERFACE_DEBUG_PRINT
    for (auto&& elm : vec) {
      printf("vec elm: %d\n", elm);
    }
  #endif

  auto ret = checkpoint::serialize<TestType>(vec);

  #if TEST_INTERFACE_DEBUG_PRINT
    printf("buffer=%p, size=%ld\n", ret->getBuffer(), ret->getSize());
  #endif

  auto vec_ptr = checkpoint::deserialize<TestType>(ret->getBuffer());
  auto& des_vec = *vec_ptr;

  #if TEST_INTERFACE_DEBUG_PRINT
    for (auto&& elm : des_vec) {
      printf("des_vec elm: %d\n", elm);
    }
  #endif

  EXPECT_EQ(des_vec.size(), vec.size());
  for (size_t i = 0; i < des_vec.size(); i++) {
    EXPECT_EQ(des_vec[i], vec[i]);
  }
}

}}} // end namespace checkpoint::tests::unit
