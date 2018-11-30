cmake_minimum_required(VERSION 3.8)
project(serdes)

include (CTest)
enable_testing()

option(
  CHECKPOINT_BUILD_TESTS "Option for turning on checkpoint tests" OFF
)
option(
  CHECKPOINT_BUILD_TESTS_WITH_MPI
  "Option for turning on checkpoint examples using MPI" OFF
)
option(
  CHECKPOINT_BUILD_EXAMPLES "Option for turning on checkpoint examples" OFF
)

# MPI package
if(CHECKPOINT_BUILD_TESTS_WITH_MPI)
 include(cmake/load_mpi_package.cmake)
 set(
   CHECK_POINT_MPI_PROC 2 CACHE STRING
   "Set number of proc used by MPI for the tests. 2 is the default."
 )
endif()

message (STATUS "Checkpoint build tests: ${CHECKPOINT_BUILD_TESTS}")
message (STATUS "Checkpoint build examples: ${CHECKPOINT_BUILD_EXAMPLES}")

include(cmake/load_package.cmake)

set(SERDES_LIBRARY checkpoint)
set(SERDES_LIBRARY_NS vt::lib::checkpoint)

# require directories for these packages
require_pkg_directory(detector "VT detector library")
# find these required packages locally
find_package_local(detector "${detector_DIR}" detector)

# optional packages
optional_pkg_directory(kokkos "Kokkos library")

if (${kokkos_DIR_FOUND})
  find_package_local(
    kokkos "${kokkos_DIR}/" Kokkos "CMake/Kokkos/" "cmake/Kokkos/"
  )
  # Used to properly setup transitive dependency in checkpointConfig.cmake.in
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 1)
else()
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 0)
endif()

# If checkpoint build tests require the GTest package
if (${CHECKPOINT_BUILD_TESTS})
  set(CHECKPOINT_HAS_GTEST TRUE)
  find_package(GTest REQUIRED)
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
