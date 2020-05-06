
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckLibraryExists)

check_include_files(sys/types.h checkpoint_has_sys_types)
check_include_files(sys/stat.h  checkpoint_has_sys_stat)
check_include_files(fcntl.h     checkpoint_has_fcntl)
check_include_files(sys/mman.h  checkpoint_has_sys_mman)
check_include_files(unistd.h    checkpoint_has_unistd)

if (NOT checkpoint_has_sys_types)
  message(FATAL_ERROR "Could not find sys/types.h, required for IO")
endif()

if (NOT checkpoint_has_sys_stat)
  message(FATAL_ERROR "Could not find sys/stat.h, required for IO")
endif()

if (NOT checkpoint_has_fcntl)
  message(FATAL_ERROR "Could not find fcntl.h, required for IO")
endif()

if (NOT checkpoint_has_sys_mman)
  message(FATAL_ERROR "Could not find sys/mman.h, required for IO")
endif()

if (NOT checkpoint_has_unistd)
  message(FATAL_ERROR "Could not find unistd.h, required for IO")
endif()

set(CMAKE_REQUIRED_INCLUDES "sys/types.h;sys/stat.h;fcntl.h")
check_function_exists(open checkpoint_has_open)

if (NOT checkpoint_has_open)
  message(FATAL_ERROR "Expected function open(..), required for IO, was not found")
endif()

set(CMAKE_REQUIRED_INCLUDES "sys/types.h;unistd.h")
check_function_exists(ftruncate checkpoint_has_ftruncate)

if (NOT checkpoint_has_ftruncate)
  message(FATAL_ERROR "Expected function ftruncate(..), required for IO, was not found")
endif()

set(CMAKE_REQUIRED_INCLUDES "unistd.h")
check_function_exists(fsync checkpoint_has_fsync)

if (NOT checkpoint_has_fsync)
  message(FATAL_ERROR "Expected function fsync(..), required for IO, was not found")
endif()

set(CMAKE_REQUIRED_INCLUDES "sys/mman.h")
check_function_exists(mmap checkpoint_has_mmap)
check_function_exists(msync checkpoint_has_msync)
check_function_exists(mmap64 checkpoint_has_mmap64)
check_function_exists(msync64 checkpoint_has_msync64)
check_function_exists(munmap64 checkpoint_has_munmap64)

if (NOT checkpoint_has_mmap)
  message(FATAL_ERROR "Expected function mmap(..), required for IO, was not found")
endif()

if (NOT checkpoint_has_msync)
  message(FATAL_ERROR "Expected function msync(..), required for IO, was not found")
endif()

if (NOT checkpoint_has_mmap64)
  message(STATUS "Could not find mmap64(..), optional for IO (limited to 2GB IO)")
else()
  if (NOT checkpoint_has_msync64)
    message(STATUS "Could not find msync64(..), required when mmap64 exists")
  endif()
  if (NOT checkpoint_has_munmap64)
    message(STATUS "Could not find munmap64(..), required when mmap64 exists")
  endif()
endif()

set(CMAKE_REQUIRED_INCLUDES "fcntl.h")
check_function_exists(fallocate checkpoint_has_fallocate)

if (NOT checkpoint_has_fallocate)
  message(STATUS "Could not find fallocate(..), optional for IO")
endif()
