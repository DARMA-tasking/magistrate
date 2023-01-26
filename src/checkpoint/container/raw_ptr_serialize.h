/*
//@HEADER
// *****************************************************************************
//
//                             raw_ptr_serialize.h
//                 DARMA/checkpoint => Serialization Library
//
// Copyright 2020 National Technology & Engineering Solutions of Sandia, LLC
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_RAW_PTR_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_RAW_PTR_SERIALIZE_H

#include "checkpoint/common.h"

namespace checkpoint {

/**
 * \brief Serialize raw pointer \c ptr
 *
 * Only footprinting mode is supported at the moment. Counts the pointer size
 * and follows it (note that it doesn't work correctly for C-style arrays!).
 *
 * \param[in] serializer serializer to use
 * \param[in] ptr pointer to serialize
 */
template <
  typename SerializerT,
  typename T,
  typename = std::enable_if_t<
    std::is_same<
      checkpoint::Footprinter<>,
      SerializerT
    >::value
  >
>
void serialize(SerializerT& s, T* ptr) {
  serializeRawPtr(s, ptr);
}

template <
  typename SerializerT,
  typename T
>
void serializeRawPtr(SerializerT& s, T* ptr) {
  s.countBytes(ptr);
  if (ptr != nullptr) {
    s | *ptr;
  }
}

/**
 * Note: do not follow void pointer.
 */
template <
  typename SerializerT
>
void serializeRawPtr(SerializerT& s, void* ptr) {
  s.countBytes(ptr);
}

#define CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(PIMPL_TYPE) \
  template < \
    typename SerializerT, \
    typename = std::enable_if_t< checkpoint::is_footprinter<SerializerT>::value > \
  > \
  void serialize(SerializerT &s, PIMPL_TYPE *t) { \
    s.countBytes(t); \
  }

} /* end namespace checkpoint */

/**
 * Note: do not follow pointers to these types, as they might be
 * incomplete on some platforms.
 */
CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(FILE)

struct ompi_communicator_t;
CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(ompi_communicator_t)
struct ompi_group_t;
CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(ompi_group_t)
struct ompi_request_t;
CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(ompi_request_t)
struct ompi_win_t;
CHECKPOINT_FOOTPRINT_PIMPL_WITH_SIZEOF_PTR(ompi_win_t)

#endif /*INCLUDED_CHECKPOINT_CONTAINER_RAW_PTR_SERIALIZE_H*/
