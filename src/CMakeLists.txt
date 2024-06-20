set(
  TOP_LEVEL_SUBDIRS
  traits container serializers traits buffer dispatch
)

set(MAGISTRATE_INSTALL_DESTINATION "include/checkpoint")
set(MAGISTRATE_EXTERNAL_DESTINATION "include")

foreach(DIR ${TOP_LEVEL_SUBDIRS})
  install(
    DIRECTORY "checkpoint/${DIR}"
    DESTINATION ${MAGISTRATE_INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

file(GLOB TOP_HEADERS "checkpoint/*.h")

install(
  FILES ${TOP_HEADERS}
  DESTINATION ${MAGISTRATE_INSTALL_DESTINATION}
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
  ${MAGISTRATE_LIBRARY}
  ${HEADER_FILES} ${SOURCE_FILES}
)

add_library(${MAGISTRATE_LIBRARY_NS} ALIAS ${MAGISTRATE_LIBRARY})

target_compile_features(${MAGISTRATE_LIBRARY} PUBLIC cxx_std_17)

include(CMakePrintHelpers)

if (${Kokkos_DIR_FOUND})
  if (${KokkosKernels_DIR_FOUND})
    message(STATUS "Magistrate: Kokkos kernels enabled")
    set(KERNELS 1)
    # Kokkos Kernels historically installed its EXPORT targets in the Kokkos:: namespace, and several
    # downstream packages import it as such, so it's not readily changed.
    # Meanwhile, Trilinos installs it under the more consistent KokkosKernels:: namespace.
    # Account for both possible routes of acquiring Kokkos Kernels here
    if(TARGET KokkosKernels::kokkoskernels)
      target_link_libraries(${MAGISTRATE_LIBRARY} PUBLIC KokkosKernels::kokkoskernels)
    else()
      target_link_libraries(${MAGISTRATE_LIBRARY} PUBLIC Kokkos::kokkoskernels)
    endif()
  else()
    message(STATUS "Magistrate: Kokkos kernels disabled")
    set(KERNELS 0)
  endif()
  target_compile_definitions(
    ${MAGISTRATE_LIBRARY} PUBLIC
    MAGISTRATE_KOKKOS_ENABLED=1 MAGISTRATE_KOKKOS_KERNELS_ENABLED=${KERNELS}
  )
  message(STATUS "Magistrate: Kokkos enabled")
  target_link_libraries(${MAGISTRATE_LIBRARY} PUBLIC Kokkos::kokkos)
endif()

target_link_libraries(${MAGISTRATE_LIBRARY} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  message(STATUS "Magistrate: enabling code coverage generation")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(
    ${MAGISTRATE_LIBRARY} PUBLIC
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )

  target_link_options(${MAGISTRATE_LIBRARY} INTERFACE --coverage)
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
  ${MAGISTRATE_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(
  ${MAGISTRATE_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS                   ${MAGISTRATE_LIBRARY}
  EXPORT                    ${MAGISTRATE_LIBRARY}
  LIBRARY DESTINATION       lib
  ARCHIVE DESTINATION       lib
  RUNTIME DESTINATION       bin
  INCLUDES DESTINATION      ${MAGISTRATE_EXTERNAL_DESTINATION}
)

install(
  EXPORT                    ${MAGISTRATE_LIBRARY}
  DESTINATION               cmake
  FILE                      "checkpointTargets.cmake"
  NAMESPACE                 vt::lib::
  COMPONENT                 runtime
)

export(
  TARGETS                   ${MAGISTRATE_LIBRARY}
  FILE                      "checkpointTargets.cmake"
  NAMESPACE                 vt::lib::
)
