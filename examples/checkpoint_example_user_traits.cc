/*
//@HEADER
// *****************************************************************************
//
//                      checkpoint_example_user_traits.cc
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
#include "checkpoint/checkpoint.h"

#include "checkpoint_example_user_traits.hpp"

int main(int, char**){
  test::TestObj obj;

  //Each invocation will be handled based on the traits attached
  auto s_info_a = checkpoint::serialize(obj);
  auto s_info_b = checkpoint::serialize<test::TestObj, checkpoint_trait>(obj);
  auto s_info_c = checkpoint::serialize<test::TestObj, checkpoint_trait, checkpoint_trait>(obj);
  auto s_info_d = checkpoint::serialize<test::TestObj, test::random_trait, checkpoint_trait>(obj);
  auto s_info_e = checkpoint::serialize<test::TestObj, checkpoint_trait, test::random_trait>(obj);
  auto s_info_f = checkpoint::serialize<test::TestObj, test::random_trait, test::random_trait>(obj);
  auto s_info_g = checkpoint::serialize<test::TestObj, shallow_trait>(obj);
  auto s_info_h = checkpoint::serialize<test::TestObj, misc::namespace_trait>(obj);
  auto s_info_i = checkpoint::serialize<test::TestObj, misc::hook_all_trait>(obj);
}