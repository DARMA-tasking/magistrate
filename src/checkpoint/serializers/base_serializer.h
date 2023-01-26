/*
//@HEADER
// *****************************************************************************
//
//                              base_serializer.h
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

#if !defined INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H
#define INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H

#include "checkpoint/common.h"

#include <type_traits>
#include <cstdlib>

namespace checkpoint {

enum struct eSerializationMode : int8_t {
  None = 0,
  Unpacking = 1,
  Packing = 2,
  Sizing = 3,
  Footprinting = 4,
  Invalid = -1
};

namespace dispatch {

template <typename SerializerT, typename T>
struct BasicDispatcher;

} /* end namespace dispatch */

namespace {

    template<typename, typename, typename...>
    struct checkTraitImpl : std::false_type {};

    template<typename Type, typename UserTrait, typename... UserTraits>
    struct checkTraitImpl<std::enable_if_t<std::is_same<Type, UserTrait>::value>*, Type, UserTrait, UserTraits...> : 
            std::true_type {};

    template<typename Type, typename UserTrait, typename... UserTraits>
    struct checkTraitImpl<std::enable_if_t<not std::is_same<Type, UserTrait>::value>*, Type, UserTrait, UserTraits...> : 
            checkTraitImpl<void*, Type, UserTraits...> {};

    template<typename Type, typename... UserTraits>
    struct checkTrait : checkTraitImpl<void*, Type, UserTraits...> {};
}

/**
 * \struct Serializer
 *
 * \brief General base class for serialiers
 */
template<typename... UserTraits>
struct Serializer {
  using ModeType = eSerializationMode;

  template <typename SerializerT, typename T>
  using DispatcherType = dispatch::BasicDispatcher<SerializerT, T>;

  /**
   * \brief Construct a base serializer
   *
   * \param[in] in_mode The serializer mode
   */
  explicit Serializer(ModeType const& in_mode) : cur_mode_(in_mode) {}

  /**
   * \brief Get the serializer mode
   *
   * \return the serializer mode
   */
  ModeType getMode() const { return cur_mode_; }

  /**
   * \brief Check if the serializer is sizing
   *
   * \return if it is sizing
   */
  bool isSizing() const { return cur_mode_ == ModeType::Sizing; }

  /**
   * \brief Check if the serializer is packing
   *
   * \return if it is packing
   */
  bool isPacking() const { return cur_mode_ == ModeType::Packing; }

  /**
   * \brief Check if the serializer is unpacking
   *
   * \return if it is unpacking
   */
  bool isUnpacking() const { return cur_mode_ == ModeType::Unpacking; }

  /**
   * \brief Check if the serializer is footprinting
   *
   * \return if it is footprinting
   */
  bool isFootprinting() const { return cur_mode_ == ModeType::Footprinting; }

  /**
   * \brief Count bytes for footprinting---default empty implementation
   *
   * \param[in] t an element
   */
  template<typename T>
  void countBytes(const T& t) {}

  /**
   * \brief Add bytes for footprinting---default empty implementation
   *
   * \param[in] s the amount of bytes to add
   */
  void addBytes(std::size_t s) {}

  /**
   * \brief Add contiguous bytes to the sizer
   *
   * \param[in] ptr the element being serialized (not used)
   * \param[in] size the number of bytes for each element
   * \param[in] num_elms the number of elements
   */
  void contiguousBytes(void* ptr, SerialSizeType size, SerialSizeType num_elms) {}

  /**
   * \brief Returns size of buffer (in bytes) used during given serialization
   * step. By default it returns 0.
   *
   * \param[in] t an element
   */
  SerialSizeType usedBufferSize() { return 0; }

  /**
   * \brief Serialize a contiguous set of typed bytes
   *
   * \param[in] serdes the serializer
   * \param[in] ptr pointer to a typed element \c T
   * \param[in] num_elms the number of elements
   */
  template <typename SerializerT, typename T>
  void contiguousTyped(SerializerT& serdes, T* ptr, SerialSizeType num_elms) {
    serdes.contiguousBytes(static_cast<void*>(ptr), sizeof(T), num_elms);
  }

  /**
   * \brief Tell the serializer that some data is skipped in the traversal.
   *
   * \note Used/implemented in serialization sanitizer.
   */
  template <typename... Args>
  void skip(Args&&... args) { }

  /**
   * \brief Get a buffer if it is associated with the serializer
   *
   * \return pointer to the \c char* buffer
   */
  SerialByteType* getBuffer() const { return nullptr; }

  /**
   * \brief Get the current spot in the buffer and then increment by some number
   * of bytes
   *
   * \param[in] inc the number of bytes to incremenet
   *
   * \return the current spot
   */
  SerialByteType* getSpotIncrement(SerialSizeType const inc) { return nullptr; }

  /**
   * \brief Check if virtual serialization is disabled
   *
   * \return whether virtual serialization is disabled
   */
  bool isVirtualDisabled() const { return virtual_disabled_; }

  /**
   * \brief Set whether virtual serialization is enabled/disabled
   *
   * \param[in] val whether virtual serialization is disabled
   */
  void setVirtualDisabled(bool val) { virtual_disabled_ = val; }

  /*template<typename Type>
  static constexpr bool hasTrait(void) {
      return checkTrait<Type, UserTraits...>::value;
  }*/
  template<typename Type>
  using hasTrait = checkTrait<Type, UserTraits...>;

protected:
  ModeType cur_mode_ = ModeType::Invalid; /**< The current mode */
  bool virtual_disabled_ = false;         /**< Virtual serialization disabled */
};


} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_SERIALIZERS_BASE_SERIALIZER_H*/
