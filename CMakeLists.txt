cmake_minimum_required(VERSION 3.8)
project(checkpoint)

include(cmake/turn_on_warnings.cmake)

include(cmake/check_system_functions.cmake)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

include (CTest)
enable_testing()

option(checkpoint_tests_enabled "Enable checkpoint tests" OFF)
option(checkpoint_mpi_enabled "Enable checkpoint tests with MPI" OFF)
option(checkpoint_examples_enabled "Enable checkpoint examples" OFF)
option(checkpoint_warnings_as_errors "Enable warnings to generate errors" OFF)
option(checkpoint_asan_enabled "Enable address sanitizer in Checkpoint" OFF)
option(
  checkpoint_ubsan_enabled
  "Enable undefined behavior sanitizer in Checkpoint" OFF
)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  option(checkpoint_serialization_error_checking "Enable extensive serialization error checking" ON)
else()
  option(checkpoint_serialization_error_checking "Enable extensive serialization error checking" OFF)
endif()

if(checkpoint_serialization_error_checking)
  add_definitions(-DSERIALIZATION_ERROR_CHECKING)
  message(STATUS "Building with serialization error checking enabled")
endif()

# Try to find ccache to speed up compilation
find_program(ccache_binary ccache)
if (ccache_binary)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${ccache_binary}")
endif()

# MPI package
if(checkpoint_mpi_enabled)
 include(cmake/load_mpi_package.cmake)
 set(
   CHECK_POINT_MPI_PROC 2 CACHE STRING
   "Set number of proc used by MPI for the tests. 2 is the default."
 )
endif()

message (STATUS "Checkpoint build tests: ${checkpoint_tests_enabled}")
message (STATUS "Checkpoint build examples: ${checkpoint_examples_enabled}")

include(cmake/load_package.cmake)

option(
  checkpoint_doxygen_enabled "Build doxygen documentation for checkpoint" OFF
)

# Doxygen library
if (${checkpoint_doxygen_enabled})
  include(cmake/load_doxygen.cmake)
endif()

# Optionally enable address sanitizer library in build
if (checkpoint_asan_enabled)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
     CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
     (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
       AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.8"))
    set(
      CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fsanitize=address"
    )
    message(STATUS "Building with address sanitizer enabled")
  else()
    message(SEND_ERROR "Cannot use ASan without clang or gcc >= 4.8")
  endif()
endif()

if (checkpoint_ubsan_enabled)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR
     CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
     (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
       AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "4.8"))
    set(
      CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fsanitize=undefined"
    )
    message(STATUS "Building with undefined behavior sanitizer enabled")
  else()
    message(SEND_ERROR "Cannot use UBSan without clang or gcc >= 4.8")
  endif()
endif()

set(CHECKPOINT_LIBRARY checkpoint CACHE INTERNAL "" FORCE )
set(CHECKPOINT_LIBRARY_NS vt::lib::checkpoint "" CACHE INTERNAL "" FORCE )

# require directories for these packages
require_pkg_directory(detector "VT detector library")
# find these required packages locally
find_package_local(detector "${detector_DIR}" detector)

# optional packages
optional_pkg_directory(kokkos "Kokkos" 1)

if (${kokkos_DIR_FOUND})
  find_package_local(
    kokkos "${kokkos_DIR}/" Kokkos "CMake/Kokkos/" "cmake/Kokkos/"
  )
  # Used to properly setup transitive dependency in checkpointConfig.cmake.in
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 1)
else()
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 0)
endif()

# optional packages
optional_pkg_directory(KokkosKernels "Kokkos kernels" 1)

if (KokkosKernels_DIR_FOUND)
  find_package_local(
    KokkosKernels "${KokkosKernels_DIR}/" KokkosKernels
    "CMake/KokkosKernels/" "cmake/KokkosKernels/"
  )
  # Used to properly setup transitive dependency in checkpointConfig.cmake.in
  set(CHECKPOINT_HAS_KOKKOS_KERNELS_LIBRARY 1)
else()
  set(CHECKPOINT_HAS_KOKKOS_KERNELS_LIBRARY 0)
endif()


# If checkpoint build tests require the GTest package
if (${checkpoint_tests_enabled})
  get_directory_property(hasParent PARENT_DIRECTORY)
  if(NOT hasParent)
    find_package(GTest REQUIRED)
    set(CHECKPOINT_HAS_GTEST TRUE)
  else()
    if (NOT DISABLE_TPL_GTEST)
      set(CHECKPOINT_HAS_GTEST TRUE)
      set(GTEST_BOTH_LIBRARIES gtest gtest_main)
    else()
      set(CHECKPOINT_HAS_GTEST FALSE)
    endif()
  endif()
else()
  set(CHECKPOINT_HAS_GTEST FALSE)
endif()

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

include (CTest)
enable_testing()

add_subdirectory(src)

add_custom_target(checkpoint_examples)
add_subdirectory(examples)

add_custom_target(checkpoint_tests)
add_subdirectory(tests)

configure_file(
  cmake/checkpointConfig.cmake.in
  "${PROJECT_BINARY_DIR}/checkpointConfig.cmake" @ONLY
)

install(
  FILES "${PROJECT_BINARY_DIR}/checkpointConfig.cmake"
  DESTINATION cmake
  COMPONENT extCfg
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
