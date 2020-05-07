/*
//@HEADER
// *****************************************************************************
//
//                                 io_buffer.cc
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

#if !defined INCLUDED_CHECKPOINT_BUFFER_IO_BUFFER_CC
#define INCLUDED_CHECKPOINT_BUFFER_IO_BUFFER_CC

#include "checkpoint/common.h"
#include "checkpoint/buffer/io_buffer.h"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

namespace checkpoint { namespace buffer {

void IOBuffer::setupForRead() {
  debug_checkpoint("IOBuffer: opening file for read: %s\n", file_.c_str());

  int ret = 0;

  /*
   * Start by opening the file, read-only mode
   */
  fd_ = open(file_.c_str(), O_RDONLY, (mode_t)0600);
  if (fd_ == -1) {
    auto err = std::string("Failed to open file=") + file_ + ": errno=" +
      std::to_string(errno);
    throw std::runtime_error(err);
  }

  debug_checkpoint("IOBuffer: fstat for file size\n");

  /*
   * Obtain the file size with fstat to do the proper mmap
   */
  struct stat sb;
  ret = fstat(fd_, &sb);

  if (ret != 0) {
    auto err = std::string("fstat failed for reading file size: errno=") +
      std::to_string(errno);
    throw std::runtime_error(err);
  }

  size_ = sb.st_size;

  debug_checkpoint("IOBuffer: got file size=%lu\n", size_);

  /*
   * mmap or mmap64 the file descriptor in READ mode
   */
  void* addr = nullptr;

#   if defined(checkpoint_has_mmap64)
  addr = mmap64(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0);

  if (addr == MAP_FAILED) {
    auto err = std::string("mmap64 failed for writing file: errno=") +
      std::to_string(errno);
    throw std::runtime_error(err);
  }
#   else
  addr = mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0);

  if (addr == MAP_FAILED) {
    auto err = std::string("mmap failed for writing file: errno=") +
      std::to_string(errno);
    throw std::runtime_error(err);
  }
# endif

  /*
   * Return out the mapped file as a char* pointer so the serializer writes to
   * this
   */
  buffer_ = static_cast<SerialByteType*>(addr);

  return;
}

void IOBuffer::setupForWrite() {
  debug_checkpoint("IOBuffer: opening file for write: %s\n", file_.c_str());

  /*
   * Start by opening the file, create/read-write/truncate mode
   */
  fd_ = open(file_.c_str(), O_CREAT | O_RDWR | O_TRUNC, (mode_t)0600);
  if (fd_ == -1) {
    auto err = std::string("Failed to open file=") + file_ + ": errno=" +
               std::to_string(errno);
    throw std::runtime_error(err);
  }

  int ret = 0;

  /*
   * If CMake detected fallocate, invoke on size_. We might want to change this
   * to posix_fallocate, but from what I read it might be slower.
   */
# if defined(checkpoint_has_fallocate)
  debug_checkpoint("IOBuffer: fallocate file\n");

  ret = fallocate(fd_, 0, 0, size_);

  if (ret != 0) {
    throw std::runtime_error("fallocate failed on file");
  }
# endif

  /*
   * Truncate the file to the appropriate size
   */
  debug_checkpoint("IOBuffer: truncating file: size=%lu\n", size_);

  ret = ftruncate(fd_, size_);

  if (ret != 0) {
    auto err = std::string("ftruncate failed on file: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }

  /*
   * Sync the file with the new size to the file system
   */
  debug_checkpoint("IOBuffer: syncing file\n");

  ret = fsync(fd_);

  if (ret != 0) {
    auto err = std::string("fsync failed on file: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }

  debug_checkpoint("IOBuffer: mmap file: len=%lu\n", size_);

  /*
   * mmap or mmap64 the file descriptor in WRITE mode
   */
  void* addr = nullptr;

# if defined(checkpoint_has_mmap64)
  addr = mmap64(nullptr, size_, PROT_WRITE, MAP_SHARED, fd_, 0);

  if (addr == MAP_FAILED) {
    auto err = std::string("mmap64 failed for writing file: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }
# else
  addr = mmap(nullptr, size_, PROT_WRITE, MAP_SHARED, fd_, 0);

  if (addr == MAP_FAILED) {
    auto err = std::string("mmap failed for writing file: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }
# endif

  /*
   * Return out the mapped file as a char* pointer so the serializer writes to
   * this
   */
  buffer_ = static_cast<SerialByteType*>(addr);
}


void IOBuffer::setupFile() {
  debug_checkpoint("IOBuffer: opening file: %s\n", file_.c_str());

  if (mode_ == ModeEnum::ReadFromFile) {
    setupForRead();
  } else if (mode_ == ModeEnum::WriteToFile) {
    setupForWrite();
  } else {
    throw std::runtime_error("Invalid IOBuffer mode");
  }
}

void IOBuffer::closeFile() {
  /*
   * If fd_ is not set, then we should not have anything open or mapped
   */
  if (fd_ == -1) {
    return;
  }

  auto addr = static_cast<void*>(buffer_);

  int ret = 0;

  /*
   * msync/msync64 the mapped file causing the file to be written out
   */
  if (mode_ == ModeEnum::WriteToFile) {
    debug_checkpoint("~IOBuffer: msync: file=%s, len=%lu\n", file_.c_str(), size_);

#   if defined(checkpoint_has_msync64)
    ret = msync64(addr, size_, MS_SYNC);

    if (ret != 0) {
      auto err = std::string("msync64 failed on file after write: errno=") +
                 std::to_string(errno);
      throw std::runtime_error(err);
    }

#   else
    ret = msync(addr, size_, MS_SYNC);

    if (ret != 0) {
      auto err = std::string("msync failed on file after write: errno=") +
                 std::to_string(errno);
      throw std::runtime_error(err);
    }
#   endif
  }

  /*
   * Unmap the file via munmap/munmap64
   */
  debug_checkpoint("~IOBuffer: munmap\n");

# if defined(checkpoint_has_munmap64)
  ret = munmap64(addr, size_);

  if (ret != 0) {
    auto err = std::string("munmap64 failed on after write: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }

# else
  ret = munmap(addr, size_);

  if (ret != 0) {
    auto err = std::string("munmap failed on file after write: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }
# endif

  /*
   * Finally, close the file. We are done.
   */
  debug_checkpoint("~IOBuffer: closing file\n");

  ret = close(fd_);

  if (ret != 0) {
    auto err = std::string("close on file descriptor failed: errno=") +
               std::to_string(errno);
    throw std::runtime_error(err);
  }

  fd_ = -1;
}

}} /* end namespace checkpoint::buffer */

#endif /*INCLUDED_CHECKPOINT_BUFFER_IO_BUFFER_CC*/
