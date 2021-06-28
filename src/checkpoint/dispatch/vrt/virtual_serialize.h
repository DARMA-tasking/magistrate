/*
//@HEADER
// *****************************************************************************
//
//                             virtual_serialize.h
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

#if !defined INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H
#define INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H

#include "checkpoint/dispatch/vrt/base.h"
#include "checkpoint/dispatch/vrt/derived.h"

namespace checkpoint { namespace dispatch { namespace vrt {

/**
 * \brief A function to handle serialization of objects of a mix of
 * types in a virtual inheritance hierarchy
 *
 * This will automatically record the exact derived type at
 * serialization, and reconstruct objects accordingly at
 * deserialization.
 */
template <typename T, typename SerializerT>
void virtualSerialize(T*& base, SerializerT& s) {
  // Get the real base in case this is called on a derived type
  using BaseT = ::checkpoint::dispatch::vrt::checkpoint_base_type_t<T>;
  auto serializer_idx = serializer_registry::makeObjIdx<BaseT, SerializerT>();
  base->_checkpointDynamicSerialize(&s, serializer_idx, no_type_idx);
}

}}} /* end namespace checkpoint::dispatch::vrt */

namespace checkpoint {

/**
 * \struct SerializeVirtualTypeIfNeeded
 *
 * \brief Do a static trait test on type to check for virtual serializability.
 *
 * If virtually serializable, we need to perform some extra
 * work to register the type, allocate, and construct the proper
 * type. Otherwise, we go through the normal path for allocating memory for T
 * and serializing what the pointer points to.
 */
template <typename T, typename SerializerT, typename _enabled = void>
struct SerializeVirtualTypeIfNeeded;

template <typename T, typename SerializerT>
struct SerializeVirtualTypeIfNeeded<
  T,
  SerializerT,
  typename std::enable_if_t<
    dispatch::vrt::VirtualSerializeTraits<T>::has_not_virtual_serialize
  >
>
{
  static dispatch::vrt::TypeIdx apply(SerializerT& s, T* target) {
    // no type idx needed in this case
    return dispatch::vrt::no_type_idx;
  }
};

template <typename T, typename SerializerT>
struct SerializeVirtualTypeIfNeeded<
  T,
  SerializerT,
  typename std::enable_if_t<
    dispatch::vrt::VirtualSerializeTraits<T>::has_virtual_serialize
  >
>
{
  static typename dispatch::vrt::TypeIdx apply(SerializerT& s, T* target) {
    dispatch::vrt::TypeIdx entry = dispatch::vrt::no_type_idx;

    if (not s.isUnpacking()) {
      entry = target->_checkpointDynamicTypeIndex();
    }

    // entry doesn't count as part of a footprint
    if (not s.isFootprinting()) {
      s | entry;
    }

    if (target != nullptr) {
      // Support deserialization in place, and make sure it's safe
      checkpointAssert(entry == target->_checkpointDynamicTypeIndex(),
                       "Trying to deserialize in place over a mismatched type");
    }

    return entry;
  }
};

/**
 * \struct ReconstructAsVirtualIfNeeded
 *
 * \brief Do a static trait test on type to check for virtual
 * serializability. If virtually serializable, we need to perform some extra
 * work to register the type, allocate, and construct the proper
 * type. Otherwise, we go through the normal path for allocating memory for T
 * and serializing what the pointer points to.
 */
template <typename T, typename SerializerT, typename _enabled = void>
struct ReconstructAsVirtualIfNeeded;

template <typename T, typename SerializerT>
struct ReconstructAsVirtualIfNeeded<
  T,
  SerializerT,
  typename std::enable_if_t<
    dispatch::vrt::VirtualSerializeTraits<T>::has_not_virtual_serialize and
    not std::is_same<SerializerT, checkpoint::Footprinter>::value
  >
> {
  static T* apply(SerializerT& s, dispatch::vrt::TypeIdx entry) {
    // no type idx needed in this case, static construction in default case
    auto t = std::allocator<T>{}.allocate(1);
    return dispatch::Reconstructor<T>::construct(t);
  }
};

template <typename T, typename SerializerT>
struct ReconstructAsVirtualIfNeeded<
  T,
  SerializerT,
  typename std::enable_if_t<
    dispatch::vrt::VirtualSerializeTraits<T>::has_not_virtual_serialize and
    std::is_same<SerializerT, checkpoint::Footprinter>::value
  >
> {
  static T* apply(SerializerT& s, dispatch::vrt::TypeIdx entry) { return nullptr; }
};

template <typename T, typename SerializerT>
struct ReconstructAsVirtualIfNeeded<
  T,
  SerializerT,
  typename std::enable_if_t<
    dispatch::vrt::VirtualSerializeTraits<T>::has_virtual_serialize
  >
> {
  static T* apply(SerializerT& s, dispatch::vrt::TypeIdx entry) {
    using BaseT = ::checkpoint::dispatch::vrt::checkpoint_base_type_t<T>;

    // use type idx here, registration needed for proper type re-construction
    auto t = dispatch::vrt::objregistry::allocateConcreteType<BaseT>(entry);
    return static_cast<T*>(
                           dispatch::vrt::objregistry::constructConcreteType<BaseT>(entry, t)
                           );
  }
};

/**
 * \brief Allocate and construct memory for a pointer with type \c T
 *
 * This function automatically handles allocating and constructing the right
 * type for virtually serialized pointers or non-virtual static allocation and
 * construction.
 *
 * An example of how to use this to properly serialize a std::shared_ptr<T>:
 *
 *   template <typename T>
 *   struct X {
 *     std::shared_ptr<T> a;
 *
 *     template <typename SerializerT>
 *     void serialize(SerializerT& s) {
 *       T* raw = elm.get();
 *       checkpoint::reconstructPointedToObjectIfNeeded(s, raw);
 *       if (s.isUnpacking()) {
 *         a = std::shared_ptr<T>(raw);
 *       }
 *       s | *a;
 *     }
 *   };
 *
 * \param[in] s the serializer
 * \param[in] target a reference to a pointer to the target object
 */
template <typename SerializerT, typename T>
void reconstructPointedToObjectIfNeeded(SerializerT& s, T*& target) {
  auto entry = SerializeVirtualTypeIfNeeded<T, SerializerT>::apply(s, target);

  if (target != nullptr) {
    // Support deserialization in place; assumes matching of virtual
    // types was checked in serializeDynamicTypeIndex()
    return;
  }

  if (s.isUnpacking()) {
    target = ReconstructAsVirtualIfNeeded<T, SerializerT>::apply(s, entry);
  }
}

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_DISPATCH_VRT_VIRTUAL_SERIALIZE_H*/
