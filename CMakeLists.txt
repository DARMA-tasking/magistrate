cmake_minimum_required(VERSION 3.8)
project(serdes)

set(CMAKE_CXX_STANDARD 14)

enable_language(CXX)

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

macro(find_package_local pkg_name pkg_directory)
  message(STATUS "find_package_local: pkg name=\"${pkg_name}\", directory=\"${pkg_directory}\"")
  # search locally only for package
  find_package(
    ${pkg_name}
    PATHS ${pkg_directory}
    REQUIRED
    NO_CMAKE_PACKAGE_REGISTRY
    NO_CMAKE_BUILDS_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
  )
endmacro(find_package_local)

set(SERDES_LIBRARY darma_checkpoint)

# require directories for these packages
require_pkg_directory(darma_detector "DARMA detector")
# find these required packages locally
find_package_local(darma_detector "${darma_detector_DIR}/cmake")

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_subdirectory(src)

add_custom_target(checkpoint_examples)
add_subdirectory(examples)

add_custom_target(checkpoint_tests)
add_subdirectory(tests)

configure_file(
  cmake/darma_checkpointConfig.cmake.in
  "${PROJECT_BINARY_DIR}/darma_checkpointConfig.cmake" @ONLY
)

install(
  FILES
  "${PROJECT_BINARY_DIR}/darma_checkpointConfig.cmake"
  DESTINATION cmake
  COMPONENT extCfg
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
