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
  printf("opening file: %s\n", file_.c_str());

  fd_ = open(file_.c_str(), O_CREAT | O_RDWR | O_TRUNC, (mode_t)0600);
  assert(fd_ != -1 && "open must be valid");

  //printf("truncating file: %s\n", file_.c_str());

  int ret = ftruncate(fd_, size_);
  assert(ret == 0 && "ftruncate should not fail");

  //printf("syncing file: %s\n", file_.c_str());

  ret = fsync(fd_);
  assert(ret == 0 && "fsync should not fail");

  //printf("mmap file: %s, len=%lu\n", file_.c_str(), size_);

  void* addr = mmap(nullptr, size_, PROT_WRITE, MAP_PRIVATE, fd_, 0);
  assert(addr != MAP_FAILED && "mmap should not fail");

  // fallocate does not exist on darwin

  //posix_fallocate(fd, 0, size_);

  buffer_ = static_cast<SerialByteType*>(addr);
}

/*virtual*/ IOBuffer::~IOBuffer() {
  auto addr = static_cast<void*>(buffer_);

  //printf("msync file: %s, len=%lu\n", file_.c_str(), size_);

  msync(addr, size_, MS_SYNC);

  //printf("munmap file: %s, len=%lu\n", file_.c_str(), size_);

  munmap(addr, size_);

  //printf("close file: %s, len=%lu\n", file_.c_str(), size_);

  close(fd_);
}

}} /* end namespace checkpoint::buffer */

#endif /*INCLUDED_CHECKPOINT_BUFFER_IO_BUFFER_CC*/
