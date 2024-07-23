/*
//@HEADER
// *****************************************************************************
//
//                                  dispatch.h
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

#if !defined INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_H
#define INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_H

#include "checkpoint/common.h"
#include "checkpoint/buffer/buffer.h"
#include "checkpoint/dispatch/clean_type.h"
#include "checkpoint/dispatch/dispatch_serializer.h"
#include "checkpoint/dispatch/dispatch_byte_macro.h"
#include "checkpoint/dispatch/reconstructor.h"

#include <functional>

#include <tuple>

namespace checkpoint {

using BufferObtainFnType = std::function<SerialByteType*(SerialSizeType size)>;

template <typename Serializer, typename T>
inline Serializer& operator|(Serializer& s, T& target);

} /* end namespace checkpoint */

namespace checkpoint { namespace dispatch {

struct InPlaceTag { };

/**
 * \struct Traverse
 *
 * \brief Traverse a target recursively\c T with a traverser object (typically
 * inheriting from \c Serializer).
 */
struct Traverse {

  /**
   * \brief Traverse a \c target of type \c T recursively with a general \c
   * TraverserT that gets applied to each element
   *
   * \param[in,out] target the target to traverse
   * \param[in,out] t a reference to the traverser
   * \param[in] len the len of the target. If > 1, \c target is an array
   *
   * \return the traverser after traversal is complete
   */
  template <typename T, typename TraverserT>
  static TraverserT& with(T& target, TraverserT& t, SerialSizeType len = 1);

  /**
   * \brief Traverse a \c target of type \c T recursively after constructing a
   * \c TraverserT from \c args
   *
   * \param[in,out] target the target to traverse
   * \param[in] args the args to pass to the traverser for construction
   *
   * \return the constructed traverser after traversal is complete
   */
  template <typename T, typename TraverserT, typename... Args>
  static TraverserT with(T& target, Args&&... args);

  /**
   * \brief Reconstruct in place a \c T on allocated memory \c mem. The buffer
   * \c mem must contain enough memory to hold \c sizeof(T). This calls the
   * reconstruction strategy based on trait detection which will detect the
   * user's method of reconstructing the class.
   *
   * \param[in] mem The memory to in-place construct on
   *
   * \return a pointer to \c T
   */
  template <typename T>
  static T* reconstruct(SerialByteType* mem);

};

/**
 * \struct Standard
 *
 * \brief Standard traversals for sizing, packing and unpacking a class
 */
struct Standard {

  /**
   * \brief Recursively get the number of bytes to serialize \c T
   *
   * \param[in] target the target to size
   * \param[in] args arguments to the sizer's constructor
   *
   * \return the number of bytes
   */
  template <typename T, typename SizerT, typename...Args>
  static SerialSizeType size(T& target, Args&&... args);

  /**
   * \brief Recursively get the memory footprint of \c T
   *
   * \param[in] target the target to measure
   * \param[in] args arguments to the footprinter's constructor
   *
   * \return memory footprint of \c T
   */
  template <typename T, typename FootprinterT, typename...Args>
  static SerialSizeType footprint(T& target, Args&&... args);

  /**
   * \brief Pack \c target that requires \c size number of bytes.
   *
   * \param[in] target the target to pack
   * \param[in] size the number of bytes for \c target
   * \param[in] args arguments to the packer's constructor
   *
   * \return the packer after packing
   */
  template <typename T, typename PackerT, typename... Args>
  static PackerT pack(T& target, SerialSizeType const& size, Args&&... args);

  /**
   * \brief Unpack \c T from packed byte-buffer \c mem
   *
   * \param[in] mem bytes holding a serialized \c T
   * \param[in] constructed whether mem is constructed \c T or not
   * \param[in] args arguments to the unpacker's constructor
   *
   * \return a pointer to an unpacked \c T
   */
  template <typename T, typename UnpackerT, typename... Args>
  static T* unpack(T* mem, Args&&... args);

  /**
   * \brief Construct \c T on allocate memory \c mem
   *
   * \param[in] mem allocated buffer
   *
   * \return constructed pointer to \c T
   */
  template <typename T>
  static T* construct(SerialByteType* mem);

  /**
   * \brief Allocate memory for \c T
   *
   * \return allocated buffer
   */
  template <typename T>
  static SerialByteType* allocate();
};

/**
 * \struct Prefixed
 *
 * \brief Traversal for polymorphic types prefixed with the vrt::TypeIdx
 */
struct Prefixed {
  /**
   * \brief Traverse a \c target of type \c T recursively with a general \c
   * TraverserT that gets applied to each element.
   * Allows to traverse only part of the data.
   *
   * \param[in,out] target the target to traverse
   * \param[in] len the len of the target. If > 1, \c target is an array
   * \param[in] check_type the flag to control type validation
   * \param[in] check_mem the flag to control memory validation
   * \param[in] args the args to pass to the traverser for construction
   *
   * \return the traverser after traversal is complete
   */
  template <typename T, typename TraverserT, typename... Args>
  static TraverserT traverse(T& target, SerialSizeType len, bool check_type, bool check_mem, Args&&... args);

  /**
   * \brief Unpack \c T from packed byte-buffer \c mem
   *
   * \param[in] mem bytes holding a serialized \c T
   * \param[in] check_type the flag to control type validation
   * \param[in] check_mem the flag to control memory validation
   * \param[in] args arguments to the unpacker's constructor
   *
   * \return a pointer to an unpacked \c T
   */
  template <typename T, typename UnpackerT, typename... Args>
  static T* unpack(T* mem, bool check_type = true, bool check_mem = true, Args&&... args);
};

template <typename T>
buffer::ImplReturnType packBuffer(
  T& target, SerialSizeType size, BufferObtainFnType fn
);

template <typename Serializer, typename T>
inline void serializeArray(Serializer& s, T* array, SerialSizeType const len);

template <typename T>
typename std::enable_if<std::is_class<T>::value && vrt::VirtualSerializeTraits<T>::has_virtual_serialize, buffer::ImplReturnType>::type
serializeType(T& target, BufferObtainFnType fn = nullptr);

template <typename T>
typename std::enable_if<!std::is_class<T>::value || !vrt::VirtualSerializeTraits<T>::has_virtual_serialize, buffer::ImplReturnType>::type
serializeType(T& target, BufferObtainFnType fn = nullptr);

template <typename T>
typename std::enable_if<std::is_class<T>::value && vrt::VirtualSerializeTraits<T>::has_virtual_serialize, T*>::type
deserializeType(SerialByteType* data, SerialByteType* allocBuf = nullptr);

template <typename T>
typename std::enable_if<!std::is_class<T>::value || !vrt::VirtualSerializeTraits<T>::has_virtual_serialize, T*>::type
deserializeType(SerialByteType* data, SerialByteType* allocBuf = nullptr);

template <typename T>
void deserializeType(InPlaceTag, SerialByteType* data, T* t);

template <typename T>
std::size_t sizeType(T& t);

}} /* end namespace checkpoint::dispatch */

#include "checkpoint/dispatch/dispatch.impl.h"

#endif /*INCLUDED_SRC_CHECKPOINT_DISPATCH_DISPATCH_H*/
