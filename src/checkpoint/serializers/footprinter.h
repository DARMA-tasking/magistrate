/*
//@HEADER
// *****************************************************************************
//
//                                footprinter.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_SERIALIZERS_FOOTPRINTER_H
#define INCLUDED_SRC_CHECKPOINT_SERIALIZERS_FOOTPRINTER_H

#include "checkpoint/common.h"
#include "checkpoint/serializers/base_serializer.h"

namespace checkpoint {

struct Footprinter : BaseSerializer {
  Footprinter() : BaseSerializer(ModeType::Footprinting) { }

  SerialSizeType getMemoryFootprint() const {
    return num_bytes_;
  }
  void contiguousBytes(void*, SerialSizeType size, SerialSizeType num_elms) {
    num_bytes_ += size * num_elms;
  }

  template<typename T>
  void countBytes(const T& t) {
    num_bytes_ += sizeof(t);
  }

  void addBytes(std::size_t s) {
    num_bytes_ += s;
  }

private:
  SerialSizeType num_bytes_ = 0;
};

namespace {
    template <typename>
    struct is_footprinter_impl : public std::false_type {};

    template <>
    struct is_footprinter_impl<Footprinter> : public std::true_type {};
}

template<typename U>
using is_footprinter = is_footprinter_impl<std::decay_t<U>>;

template<typename U>
static constexpr const bool is_footprinter_v = is_footprinter<U>::value;

} /* end namespace checkpoint */

#endif /*INCLUDED_SRC_CHECKPOINT_SERIALIZERS_FOOTPRINTER_H*/
