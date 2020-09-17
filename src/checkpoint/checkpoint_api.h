/*
//@HEADER
// *****************************************************************************
//
//                               checkpoint_api.h
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

#if !defined INCLUDED_CHECKPOINT_CHECKPOINT_API_H
#define INCLUDED_CHECKPOINT_CHECKPOINT_API_H

#include <cstdlib>
#include <functional>
#include <memory>

namespace checkpoint {

/// Callback for user to allocate bytes during serialization
using BufferCallbackType = std::function<char*(std::size_t size)>;

/// Return of serialize that contains the buffer and size serialized
struct SerializedInfo {
  virtual std::size_t getSize() const = 0;
  virtual char* getBuffer() const = 0;
  virtual ~SerializedInfo() { }
};

/// Convenience typedef for \c std::unique_ptr<SerializedInfo>
using SerializedReturnType = std::unique_ptr<SerializedInfo>;

/**
 * \brief Serialize \c T into a byte buffer
 *
 * Serializes an object recursively by invoking the \c serialize
 * functions/methods recursively.
 *
 * \param[in] target the \c T to serialize
 * \param[in] fn (optional) callback to supply buffer for to allow user
 * allocation of the produced byte buffer. The callback will be passed the
 * number of bytes required and return a char* to a buffer of at least that
 * many bytes.
 *
 * \return a \c std::unique_ptr to a \c SerializedInfo containing the buffer
 * with serialized data and the size of the buffer
 */
template <typename T>
SerializedReturnType serialize(T& target, BufferCallbackType fn = nullptr);

/**
 * \brief De-serialize and reify \c T from a byte buffer and corresponding \c
 * size
 *
 * De-serializes an object recursively by first invoking the reconstruction
 * strategy and then \c serialize functions/methods recursively to rebuild the
 * state of the object as serialized. During reconstruction, based on trait
 * detection, \c T will either be default constructed on \c user_buf (or a
 * system allocated buffer) or reconstructed based on a user-defined reconstruct
 * method. If \c user_buf is not passed, the returned object point must be
 * deallocated with \c delete
 *
 * \param[in] buf the buffer containing the bytes to reify \c T
 * \param[in] object_buf (optional) buffer containing bytes allocated with
 * sufficient size for \c T. If this buffer != null, the caller is responsible
 * for deallocating the buffer. If it is not passed, the system will allocate a
 * buffer that must be de-allocated with \c delete
 *
 * \return a pointer to the newly reified \c T based on bytes in \c buf
 */
template <typename T>
T* deserialize(char* buf, char* object_buf);

/**
 * \brief De-serialize and reify \c T from a byte buffer and corresponding \c
 * size
 *
 * De-serializes an object recursively by first invoking the reconstruction
 * strategy and then \c serialize functions/methods recursively to rebuild the
 * state of the object as serialized. During reconstruction, based on trait
 * detection, \c T will either be default constructed or reconstructed based on
 * a user-defined reconstruct method.
 *
 * \param[in] buf the buffer containing the bytes to reify \c T
 *
 * \return a unique pointer to the newly reified \c T based on bytes in \c buf
 */
template <typename T>
std::unique_ptr<T> deserialize(char* buf);

/**
 * \brief De-serialize and reify \c T from a byte buffer and corresponding \c
 * size in-place on the user-provided \c t
 *
 * Note: the other form of \c deserialize will either reconstruct to default
 * construct \c T in-place. This overload will not allocate or construct \c T
 *
 * \param[in] buf the buffer containing the bytes to reify \c T
 * \param[in] t a valid pointer to a \c T that has been user-allocated and
 * constructed
 */
template <typename T>
void deserializeInPlace(char* buf, T* t);

/**
 * \brief Convenience function for de-serializing and reify \c T directly from \c
 * in the return value from \c serialize
 *
 * \param[in] in the buffer and size combo returned from \c serialize
 *
 * \return a unique pointer to \c T that must be deallocated
 */
template <typename T>
std::unique_ptr<T> deserialize(SerializedReturnType&& in);

/**
 * \brief Get the number of bytes that \c target requires for serialization.
 *
 * \param[in] target reference to \c T to size
 *
 * \return number of bytes for the \c target
 */
template <typename T>
std::size_t getSize(T& target);

/**
 * \brief Get memory footprint of \c target
 *
 * \param[in] target reference to \c T to measure footprint
 *
 * \return memory footprint of the \c target
 */
template <typename T>
std::size_t getMemoryFootprint(T& target);

/**
 * \brief Serialize \c T to file with filename \c file
 *
 * Byte-serializes \c T to file, truncating \c file if it already exists. If any
 * error occurs while opening/closing/mapping the file, \c std::runtime_error
 * will be thrown with an appropriate error message containing the corresponding
 * errno.
 *
 * \param[in] target the \c T to serialize
 * \param[in] file name of the file to create
 */
template <typename T>
void serializeToFile(T& target, std::string const& file);

/**
 * \brief De-serialize and reify \c T from a file
 *
 * De-serializes an object recursively by first invoking the reconstruction
 * strategy and then \c serialize functions/methods recursively to rebuild the
 * state of the object as serialized. During reconstruction, based on trait
 * detection, \c T will either be default constructed or reconstructed based on
 * a user-defined reconstruct method.
 *
 * \param[in] file the filename to read with bytes for \c T
 *
 * \return unique pointer to the new object \c T
 */
template <typename T>
std::unique_ptr<T> deserializeFromFile(std::string const& file);

/**
 * \brief De-serialize and reify \c T from a file in place on an existing
 * pointer to \c T
 *
 * De-serializes an object recursively by invoking the \c serialize
 * functions/methods recursively to rebuild the state of the object as
 * serialized.
 *
 * \param[in] file the filename to read with bytes for \c T
 * \param[in] t a valid, constructed \c T to deserialize into
 */
template <typename T>
void deserializeInPlaceFromFile(std::string const& file, T* buf);

} /* end namespace checkpoint */

#endif /*INCLUDED_CHECKPOINT_CHECKPOINT_API_H*/
