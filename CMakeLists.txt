cmake_minimum_required(VERSION 3.8)
project(serdes)

include (CTest)
enable_testing()

option(CHECKPOINT_BUILD_TESTS "Option for turning on checkpoint tests" OFF)
option(CHECKPOINT_BUILD_EXAMPLES "Option for turning on checkpoint examples" OFF)

message (STATUS "Checkpoint build tests: ${CHECKPOINT_BUILD_TESTS}")
message (STATUS "Checkpoint build examples: ${CHECKPOINT_BUILD_EXAMPLES}")

macro(require_pkg_directory pkg_name pkg_user_name)
  #message(STATUS "require_directory: name=${pkg_name}")
  option(${pkg_name}_DIR "Root folder for ${pkg_user_name} installation" OFF)
  if (NOT ${pkg_name}_DIR)
    message(FATAL_ERROR "Please specify ${pkg_user_name} library installation root with -D${pkg_name}_DIR=")
  endif()
endmacro(require_pkg_directory)

macro(find_package_local pkg_name pkg_directory pkg_other_name)
  message(STATUS "find_package_local: pkg name=\"${pkg_name}\", directory=\"${pkg_directory}\"")
  # search locally only for package
  find_package(
    ${pkg_name}
    PATHS ${pkg_directory}
    REQUIRED
    NAMES ${pkg_name} ${pkg_other_name}
    NO_CMAKE_PACKAGE_REGISTRY
    NO_CMAKE_BUILDS_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
  )
endmacro(find_package_local)

macro(optional_pkg_directory pkg_name pkg_user_name)
  #message(STATUS "optional_pkg_directory: name=${pkg_name}")
  option(${pkg_name}_DIR "Root folder for ${pkg_user_name} installation" OFF)
  if (NOT ${pkg_name}_DIR)
    message(
      STATUS
      "Path for ${pkg_user_name} library (optional) not specified "
      "with -D${pkg_name}_DIR="
    )
    message(
      STATUS
      "Building without ${pkg_user_name} library"
    )
    set(${pkg_name}_DIR_FOUND 0)
  else()
    message(
      STATUS
      "Path for ${pkg_user_name} library (optional) specified "
      "with -D${pkg_name}_DIR=${${pkg_name}_DIR}"
    )
    set(${pkg_name}_DIR_FOUND 1)
  endif()
endmacro(optional_pkg_directory)

set(SERDES_LIBRARY checkpoint)
set(SERDES_LIBRARY_NS vt::lib::checkpoint)

# require directories for these packages
require_pkg_directory(detector "VT detector library")
# find these required packages locally
find_package_local(detector "${detector_DIR}/cmake" detector)

# If checkpoint build tests require the GTest package
if (${CHECKPOINT_BUILD_TESTS})
  set(CHECKPOINT_HAS_GTEST TRUE)
  find_package(GTest REQUIRED)
else()
  set(CHECKPOINT_HAS_GTEST FALSE)
endif()

# unless kokkos or kokkoscore is found later
set(CHECKPOINT_HAS_KOKKOS_LIBRARY 0)

# optional packages
optional_pkg_directory(kokkos "Kokkos library")
# optional packages
optional_pkg_directory(kokkoscore "KokkosCore library")

if (${kokkos_DIR_FOUND})
  if (${kokkoscore_DIR_FOUND})
    message (FATAL_ERROR "Use either kokkos_DIR or kokkoscore_DIR, not both")
  endif()
endif()

if (${kokkos_DIR_FOUND})
  find_package_local(kokkos "${kokkos_DIR}/CMake/Kokkos/" Kokkos)
  # Used to properly setup transitive dependency in checkpointConfig.cmake.in
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 1)
endif()

if (${kokkoscore_DIR_FOUND})
  find_package_local(kokkoscore "${kokkoscore_DIR}/CMake/KokkosCore/" KokkosCore)
  # Used to properly setup transitive dependency in checkpointConfig.cmake.in
  set(CHECKPOINT_HAS_KOKKOS_LIBRARY 1)
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
