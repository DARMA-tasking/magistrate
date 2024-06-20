cmake_minimum_required(VERSION 3.23)
project(magistrate VERSION 1.4.0)

include(cmake/turn_on_warnings.cmake)

include(cmake/check_system_functions.cmake)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

include (CTest)
enable_testing()

function(deprecated_option old_option new_option description default_value)
  option(${new_option} "${description}" ${default_value})
  if(DEFINED ${old_option})
    message(DEPRECATION "Warning: ${old_option} is deprecated and will be removed in a future version. Use ${new_option} instead.")
    set(${new_option} "${${old_option}}" CACHE BOOL "${description}" FORCE)
  endif()
endfunction()

deprecated_option(checkpoint_tests_enabled magistrate_tests_enabled "Enable magistrate tests" OFF)
deprecated_option(checkpoint_mpi_enabled magistrate_mpi_enabled "Enable magistrate tests with MPI" OFF)
deprecated_option(checkpoint_examples_enabled magistrate_examples_enabled "Enable magistrate examples" OFF)
deprecated_option(checkpoint_warnings_as_errors magistrate_warnings_as_errors "Enable warnings to generate errors" OFF)
deprecated_option(checkpoint_asan_enabled magistrate_asan_enabled "Enable address sanitizer in magistrate" OFF)
deprecated_option(checkpoint_ubsan_enabled magistrate_ubsan_enabled "Enable undefined behavior sanitizer in magistrate" OFF)

option(CODE_COVERAGE "Enable coverage reporting" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  set(error_check ON)
else()
  set(error_check OFF)
endif()
deprecated_option(checkpoint_serialization_error_checking_enabled
  magistrate_serialization_error_checking_enabled "Enable extensive serialization error checking" ${error_check})

if(magistrate_serialization_error_checking_enabled)
  add_definitions(-DSERIALIZATION_ERROR_CHECKING)
  message(STATUS "Building with serialization error checking enabled")
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
  # Try to find ccache to speed up compilation
  find_program(ccache_binary ccache)
  if (ccache_binary)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${ccache_binary})
  endif()
endif()

# MPI package
if(magistrate_mpi_enabled)
 include(cmake/load_mpi_package.cmake)
 set(
   MAGISTRATE_MPI_PROC 2 CACHE STRING
   "Set number of proc used by MPI for the tests. 2 is the default."
 )
endif()

message (STATUS "Magistrate build tests: ${magistrate_tests_enabled}")
message (STATUS "Magistrate build examples: ${magistrate_examples_enabled}")

include(cmake/load_package.cmake)

deprecated_option(checkpoint_doxygen_enabled magistrate_doxygen_enabled "Build doxygen documentation for magistrate" OFF)

# Doxygen library
if (${magistrate_doxygen_enabled})
  include(cmake/load_doxygen.cmake)
endif()

# Optionally enable address sanitizer library in build
if (magistrate_asan_enabled)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=address"
  )
  message(STATUS "Building with address sanitizer enabled")
endif()

if (magistrate_ubsan_enabled)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fsanitize=undefined"
  )
  message(STATUS "Building with undefined behavior sanitizer enabled")
endif()

set(CHECKPOINT_LIBRARY checkpoint CACHE INTERNAL "" FORCE )
set(MAGISTRATE_LIBRARY_NS vt::lib::checkpoint "" CACHE INTERNAL "" FORCE )

set(CMAKE_CXX_EXTENSIONS OFF)

option(kokkos_DISABLE "Disable Kokkos" OFF)
option(kokkos_kernels_DISABLE "Disable Kokkos Kernels" OFF)

if (NOT kokkos_DISABLE)
  # optional packages
  optional_pkg_directory(Kokkos "Kokkos" 1)

  if (${Kokkos_DIR_FOUND})
    find_package_local(Kokkos)
    # Used to properly setup transitive dependency in checkpointConfig.cmake.in
    set(CHECKPOINT_HAS_KOKKOS_LIBRARY 1)
  else()
    set(CHECKPOINT_HAS_KOKKOS_LIBRARY 0)
  endif()
endif()

if (NOT kokkos_kernels_DISABLE)
  # optional packages
  optional_pkg_directory(KokkosKernels "Kokkos kernels" 1)

  if (${KokkosKernels_DIR_FOUND})
    find_package_local(KokkosKernels)
    # Used to properly setup transitive dependency in checkpointConfig.cmake.in
    set(CHECKPOINT_HAS_KOKKOS_KERNELS_LIBRARY 1)
  else()
    set(CHECKPOINT_HAS_KOKKOS_KERNELS_LIBRARY 0)
  endif()
endif()

get_directory_property(hasParent PARENT_DIRECTORY)

if(hasParent)
  message(STATUS "Checkpoint: building as a sub-directory")
else()
  message(STATUS "Checkpoint: building as top-level")
endif()

# If checkpoint build tests require the GTest package
if (${magistrate_tests_enabled})
  if(NOT hasParent)
    find_package(GTest REQUIRED)
    set(CHECKPOINT_HAS_GTEST TRUE)
  else()
    if (NOT DISABLE_TPL_GTEST)
      set(CHECKPOINT_HAS_GTEST TRUE)
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
