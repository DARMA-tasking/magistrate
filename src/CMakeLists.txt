set(
  TOP_LEVEL_SUBDIRS
  traits container serializers traits buffer dispatch
)

set(CHECKPOINT_INSTALL_DESTINATION "include/checkpoint")
set(CHECKPOINT_EXTERNAL_DESTINATION "include")

foreach(DIR ${TOP_LEVEL_SUBDIRS})
  install(
    DIRECTORY "checkpoint/${DIR}"
    DESTINATION ${CHECKPOINT_INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

file(GLOB TOP_HEADERS "checkpoint/*.h")

install(
  FILES ${TOP_HEADERS}
  DESTINATION ${CHECKPOINT_INSTALL_DESTINATION}
)

file(
  GLOB
  HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/checkpoint/*.h
)

file(
  GLOB
  SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/checkpoint/*.cc
)

foreach(SUB_DIR ${TOP_LEVEL_SUBDIRS})
  file(
    GLOB
    "${SUB_DIR}_HEADER_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/checkpoint/${SUB_DIR}/*.h"
  )

  file(
    GLOB
    "${SUB_DIR}_SOURCE_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/checkpoint/${SUB_DIR}/*.cc"
  )

  #message("headers=${${SUB_DIR}_HEADER_FILES}")
  #message("source=${${SUB_DIR}_SOURCE_FILES}")

  list(
    APPEND
    HEADER_FILES
    "${${SUB_DIR}_HEADER_FILES}"
  )

  list(
    APPEND
    SOURCE_FILES
    "${${SUB_DIR}_SOURCE_FILES}"
  )
endforeach()

add_library(
  ${CHECKPOINT_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

add_library(${CHECKPOINT_LIBRARY_NS} ALIAS ${CHECKPOINT_LIBRARY})

target_compile_options(${CHECKPOINT_LIBRARY} PUBLIC "-fPIC")
target_compile_options(${CHECKPOINT_LIBRARY} PUBLIC ${CXX_STANDARD_FLAGS})

target_compile_definitions(${CHECKPOINT_LIBRARY} PUBLIC HAS_DETECTION_COMPONENT=1)

include(CMakePrintHelpers)

if (${kokkos_DIR_FOUND})
  if (KokkosKernels_DIR_FOUND)
    message(STATUS "Checkpoint: Kokkos kernels enabled")
    set(KERNELS 1)
    target_link_libraries(${CHECKPOINT_LIBRARY} PUBLIC Kokkos::kokkoskernels)
  else()
    message(STATUS "Checkpoint: Kokkos kernels disabled")
    set(KERNELS 0)
  endif()
  target_compile_definitions(
    ${CHECKPOINT_LIBRARY} PUBLIC
    KOKKOS_ENABLED_CHECKPOINT=1 KOKKOS_KERNELS_ENABLED=${KERNELS}
  )
  message(STATUS "Checkpoint: Kokkos enabled")
  # This should not have to be done, missing include configuration from kokkos
  target_link_libraries(${CHECKPOINT_LIBRARY} PUBLIC Kokkos::kokkos)
endif()

target_link_libraries(${CHECKPOINT_LIBRARY} PUBLIC vt::lib::detector)
target_link_libraries(${CHECKPOINT_LIBRARY} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  message(STATUS "Checkpoint: enabling code coverage generation")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(
    ${CHECKPOINT_LIBRARY} PUBLIC
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(${CHECKPOINT_LIBRARY} INTERFACE --coverage)
  else()
    target_link_libraries(${CHECKPOINT_LIBRARY} INTERFACE --coverage)
  endif()
endif()

configure_file(
  ${PROJECT_SOURCE_DIR}/checkpoint/cmake_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/checkpoint/cmake_config.h @ONLY
)

install(
  FILES            "${CMAKE_CURRENT_BINARY_DIR}/checkpoint/cmake_config.h"
  DESTINATION      include/checkpoint
)

target_include_directories(
  ${CHECKPOINT_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(
  ${CHECKPOINT_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

if("${CMAKE_SOURCE_DIR}/src" STREQUAL PROJECT_SOURCE_DIR)
  message(STATUS "Checkpoint: building as top-level")
else()
  message(STATUS "Checkpoint: building as a sub-directory")

  if(CMAKE_VERSION VERSION_LESS "3.13.0")
    message(
      WARNING
      "Checkpoint: Install targets skipped because CMake version ${CMAKE_VERSION}<3.13; "
      "affects packages built with a Checkpoint install. "
      "CMake <3.13 does not support exporting targets from subdirectories"
    )
  else()
    install(TARGETS detector EXPORT ${CHECKPOINT_LIBRARY})
    set(DETECTOR_EXPORT detector)
  endif()

endif()

install(
  TARGETS                   ${CHECKPOINT_LIBRARY}
  EXPORT                    ${CHECKPOINT_LIBRARY}
  LIBRARY DESTINATION       lib
  ARCHIVE DESTINATION       lib
  RUNTIME DESTINATION       bin
  INCLUDES DESTINATION      ${CHECKPOINT_EXTERNAL_DESTINATION}
)

install(
  EXPORT                    ${CHECKPOINT_LIBRARY}
  DESTINATION               cmake
  FILE                      "checkpointTargets.cmake"
  NAMESPACE                 vt::lib::
  COMPONENT                 runtime
)

export(
  TARGETS                   ${CHECKPOINT_LIBRARY}
                            ${DETECTOR_EXPORT}
  FILE                      "checkpointTargets.cmake"
  NAMESPACE                 vt::lib::
)
