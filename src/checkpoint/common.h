/*
//@HEADER
// *****************************************************************************
//
//                                   common.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_COMMON_H
#define INCLUDED_SRC_CHECKPOINT_COMMON_H

#include <checkpoint/cmake_config.h>

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <utility>
#include <cassert>

#define DEBUG_CHECKPOINT 0

#if DEBUG_CHECKPOINT
#define debug_checkpoint(...)        \
  do {                               \
    printf(__VA_ARGS__);             \
  } while (0);
#else
#define debug_checkpoint(...)
#endif

#define CHECKPOINT_ASSERT_ENABLED 1

namespace checkpoint { namespace debug {

template <typename T>
inline int useVarsDummy(T t) {
  return (void)(t), 0;
}

template <typename... Args>
inline void useVars(Args&&... args) {
  using expander = int[];
  (void)(expander {
    0,
    useVarsDummy(std::forward<Args>(args)) ...
  });
}

inline void assertOut(
  char const* cond, char const* str,
  char const* file, int const line, char const* func
) {
  auto msg = "Assertion failed in Checkpoint library:";
  fprintf(
    stderr,
    "-----------------------------------------------------------------------\n"
    "%s\n"
    "-----------------------------------------------------------------------\n"
    "   Reason: %s\n"
    "Condition: %s\n"
    "     File: %s\n"
    "     Line: %d\n"
    "     Func: %s\n"
    "-----------------------------------------------------------------------\n",
    msg, str, cond, file, line, func
  );
  assert(false && "Checkpoint assertion failure");
}

}} /* end namespace checkpoint::debug */

// Macro for to force use of variables in assert statement to avoid warnings
#define checkpoint_force_use(...) ::checkpoint::debug::useVars(__VA_ARGS__);

// Macro for assertion with info about where and why it triggered
#if CHECKPOINT_ASSERT_ENABLED
#define checkpointAssert(cond,str)                                    \
  do {                                                                \
    if (!(cond)) {                                                    \
      ::checkpoint::debug::assertOut(                                 \
        #cond, str, __FILE__, __LINE__, __func__                      \
      );                                                              \
    }                                                                 \
  } while (false)
#else
#define checkpointAssert(cond,str) checkpoint_force_use(cond)
#endif

namespace checkpoint {

using SerialSizeType = size_t;
using SerialByteType = char;

} /* end namespace checkpoint */

#endif /*INCLUDED_SRC_CHECKPOINT_COMMON_H*/
