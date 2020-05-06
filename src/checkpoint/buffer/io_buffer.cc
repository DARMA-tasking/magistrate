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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

namespace checkpoint { namespace buffer {

void IOBuffer::setupFile() {
  debug_checkpoint("IOBuffer: opening file: %s\n", file_.c_str());

  /*
   * Start by opening the file, create/read-write/truncate mode
   */
  fd_ = open(file_.c_str(), O_CREAT | O_RDWR | O_TRUNC, (mode_t)0600);
  assert(fd_ != -1 && "open must return a valid file descriptor");

  if (fd_ == -1) {
    throw new std::runtime_error("Failed to open file");
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
    throw new std::runtime_error("fallocate failed on file");
  }
# endif

  /*
   * Truncate the file to the appropriate size
   */
  debug_checkpoint("IOBuffer: truncating file\n");

  ret = ftruncate(fd_, size_);
  assert(ret == 0 && "ftruncate should not fail");

  if (ret != 0) {
    throw new std::runtime_error("ftruncate failed on file");
  }

  /*
   * Sync the file with the new size to the file system
   */
  debug_checkpoint("IOBuffer: syncing file\n");

  ret = fsync(fd_);
  assert(ret == 0 && "fsync should not fail");

  if (ret != 0) {
    throw new std::runtime_error("fsync failed on file");
  }

  debug_checkpoint("IOBuffer: mmap file: len=%lu\n", size_);

  /*
   * mmap or mmap64 the file descriptor in WRITE mode, PRIVATE to this process
   */
  void* addr = nullptr;

# if defined(checkpoint_has_mmap64)
  addr = mmap64(nullptr, size_, PROT_WRITE, MAP_PRIVATE, fd_, 0);
  assert(addr != MAP_FAILED && "mmap64 should not fail");

  if (addr == MAP_FAILED) {
    throw new std::runtime_error("mmap64 failed on file");
  }
# else
  addr = mmap(nullptr, size_, PROT_WRITE, MAP_PRIVATE, fd_, 0);
  assert(addr != MAP_FAILED && "mmap should not fail");

  if (addr == MAP_FAILED) {
    throw new std::runtime_error("mmap failed on file");
  }
# endif

  /*
   * Return out the mapped file as a char* pointer so the serializer writes to
   * this
   */
  buffer_ = static_cast<SerialByteType*>(addr);
}

/*virtual*/ IOBuffer::~IOBuffer() {
  auto addr = static_cast<void*>(buffer_);

  debug_checkpoint("~IOBuffer: msync: file=%s, len=%lu\n", file_.c_str(), size_);

  int ret = 0;

  /*
   * msync/msync64 the mapped file causing the file to be written out
   */
# if defined(checkpoint_has_msync64)
  ret = msync64(addr, size_, MS_SYNC);
  assert(ret == 0 && "msync64 should not fail");

  if (ret != 0) {
    throw new std::runtime_error("msync64 failed on file after write");
  }

# else
  ret = msync(addr, size_, MS_SYNC);
  assert(ret == 0 && "msync should not fail");

  if (ret != 0) {
    throw new std::runtime_error("msync failed on file after write");
  }
# endif

  /*
   * Unmap the file via munmap/munmap64
   */
  debug_checkpoint("~IOBuffer: munmap\n");

# if defined(checkpoint_has_munmap64)
  ret = munmap64(addr, size_);
  assert(ret == 0 && "munmap64 should not fail");

  if (ret != 0) {
    throw new std::runtime_error("munmap64 failed on file after write");
  }

# else
  ret = munmap(addr, size_);
  assert(ret == 0 && "munmap should not fail");

  if (ret != 0) {
    throw new std::runtime_error("munmap failed on file after write");
  }
# endif

  /*
   * Finally, close the file. We are done.
   */
  debug_checkpoint("~IOBuffer: closing file\n");

  ret = close(fd_);
  assert(ret == 0 && "close should not fail");

  if (ret != 0) {
    throw new std::runtime_error("close on file descriptor failed");
  }
}

}} /* end namespace checkpoint::buffer */

#endif /*INCLUDED_CHECKPOINT_BUFFER_IO_BUFFER_CC*/
