/*
//@HEADER
// *****************************************************************************
//
//                              vector_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_CONTAINER_VECTOR_SERIALIZE_H
#define INCLUDED_CHECKPOINT_CONTAINER_VECTOR_SERIALIZE_H

#include "checkpoint/common.h"
#include "checkpoint/dispatch/dispatch.h"
#include "checkpoint/dispatch/reconstructor.h"
#include "checkpoint/serializers/serializers_headers.h"

#include <vector>

namespace checkpoint {

template <typename SerializerT, typename T, typename VectorAllocator>
typename std::enable_if_t<
  not std::is_same<SerializerT, checkpoint::Footprinter>::value, SerialSizeType
>
serializeVectorMeta(SerializerT& s, std::vector<T, VectorAllocator>& vec) {
  SerialSizeType vec_capacity = vec.capacity();
  s | vec_capacity;
  vec.reserve(vec_capacity);

  SerialSizeType vec_size = vec.size();
  s | vec_size;
  return vec_size;
}

template <typename SerializerT, typename T, typename VectorAllocator>
typename std::enable_if_t<
  std::is_same<SerializerT, checkpoint::Footprinter>::value, SerialSizeType
>
serializeVectorMeta(SerializerT& s, std::vector<T, VectorAllocator>& vec) {
  s.countBytes(vec);
  return vec.size();
}

template <typename T, typename VectorAllocator>
void constructVectorDataWithResize(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isDefaultConsType<T>* = nullptr
) {
  vec.resize(vec_size);
}

template <typename T, typename VectorAllocator>
void constructVectorDataWithResize(
  SerialSizeType const, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isNotDefaultConsType<T>* = nullptr
) {
  static_assert(
    SerializableTraits<T, void>::is_tagged_constructible or
      SerializableTraits<T, void>::is_reconstructible or
      std::is_default_constructible<T>::value,
    "Either a default constructor, reconstruct() function, or tagged "
    "constructor are required for std::vector de-serialization"
  );
}

template <typename T>
struct Allocated {
  Allocated() : buf{dispatch::Standard::template allocate<T>()} { }
  ~Allocated() { std::allocator<T>{}.deallocate(reinterpret_cast<T*>(buf), 1); }

  SerialByteType* buf;
};

template <typename T, typename VectorAllocator>
void constructVectorDataReconstruct(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isReconstructibleType<T>* = nullptr
) {
  Allocated<T> const allocated;
  for (SerialSizeType i = 0; i < vec_size; ++i) {
    auto& t = T::reconstruct(allocated.buf);
    vec.emplace_back(std::move(t));
  }
}

template <typename T, typename VectorAllocator>
void constructVectorDataReconstruct(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isNonIntReconstructibleType<T>* = nullptr
) {
  Allocated<T> const allocated;
  for (SerialSizeType i = 0; i < vec_size; ++i) {
    T* t = nullptr;
    reconstruct(t, allocated.buf);
    vec.emplace_back(std::move(*t));
  }
}

template <typename T, typename VectorAllocator>
void constructVectorDataReconstruct(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isNotReconstructibleType<T>* = nullptr
) {
  constructVectorDataWithResize(vec_size, vec);
}

template <typename T, typename VectorAllocator>
void constructVectorData(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isTaggedConstructibleType<T>* = nullptr
) {
  vec.resize(vec_size, T{SERIALIZE_CONSTRUCT_TAG{}});
}

template <typename T, typename VectorAllocator>
void constructVectorData(
  SerialSizeType const vec_size, std::vector<T, VectorAllocator>& vec,
  typename ReconstructorTraits<T>::template isNotTaggedConstructibleType<T>* = nullptr
) {
  constructVectorDataReconstruct(vec_size, vec);
}

template <typename Serializer, typename T, typename VectorAllocator>
void serialize(Serializer& s, std::vector<T, VectorAllocator>& vec) {
  auto const vec_size = serializeVectorMeta(s, vec);

  if (s.isUnpacking()) {
    constructVectorData(vec_size, vec);
  }

  dispatch::serializeArray(s, vec.data(), vec.size());

  // make sure to account for reserved space when footprinting
  s.addBytes(sizeof(T) * (vec.capacity() - vec.size()));
}

template <typename Serializer, typename VectorAllocator>
void serialize(Serializer& s, std::vector<bool, VectorAllocator>& vec) {
  if (s.isFootprinting()) {
    s.countBytes(vec);
    return;
  }

  auto const vec_size = serializeVectorMeta(s, vec);

  if (s.isUnpacking()) {
    constructVectorData(vec_size, vec);
  }

  if (!s.isUnpacking()) {
    for (bool elt : vec) {
      s | elt;
    }
  } else {
    for (size_t i = 0; i < vec.size(); ++i) {
      bool elt = false;
      s | elt;
      vec[i] = elt;
    }
  }
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CONTAINER_VECTOR_SERIALIZE_H*/
