
set(PROJECT_TEST_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit)
set(PROJECT_TEST_MPI_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit/tests_mpi)

if(CHECKPOINT_BUILD_TESTS_WITH_MPI)
  message(STATUS "Building unit tests with MPI")
  file(
      GLOB
      PROJECT_TEST_LIST_MPI
      RELATIVE
      "${PROJECT_TEST_MPI_UNIT_DIR}"
      "${PROJECT_TEST_MPI_UNIT_DIR}/*.cc"
   )
endif()

file(
  GLOB
  PROJECT_TEST_LIST
  RELATIVE
  "${PROJECT_TEST_UNIT_DIR}"
  "${PROJECT_TEST_UNIT_DIR}/*.cc"
)

#
# Setup a potential target (a test, example, etc. that uses checkpoint) will all
# the libraries to link, include directories, definitions
#
macro(checkpoint_link_target target has_mpi)
  target_include_directories(${target} PUBLIC ${PROJECT_TEST_UNIT_DIR})
  target_link_libraries(${target} PRIVATE ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${target} PUBLIC ${SERDES_LIBRARY})
  target_link_libraries(${target} PUBLIC OpenMP::OpenMP_CXX)
  if (${kokkos_DIR_FOUND})
    target_include_directories(${target} SYSTEM PUBLIC ${Kokkos_INCLUDE_DIRS})
    target_link_libraries(${target} PUBLIC ${Kokkos_LIBRARIES})
  endif()

  if (${has_mpi})
    target_link_libraries(${target} PUBLIC ${MPI_LIBRARIES})
    target_compile_definitions(${target} PUBLIC HAVE_MPI=1)
    target_include_directories(${target} PUBLIC ${MPI_INCLUDE_PATH})
  endif()
endmacro()

if (${CHECKPOINT_BUILD_TESTS})
  if (NOT CHECKPOINT_HAS_GTEST)
    message(
      STATUS
      "Checkpoint: Unit test are disabled because GoogleTest is not setup:"
      "gtest_DIR=${gtest_DIR}"
    )
  else()
    message(
      STATUS
      "Checkpoint: GTEST package is found: unit tests are enabled "
      "gtest_DIR=${gtest_DIR}, GTEST_ROOT=${GTEST_ROOT}"
    )

    set(
      TEST_HEADER_FILES
      ${PROJECT_TEST_UNIT_DIR}/test_commons.h
      ${PROJECT_TEST_UNIT_DIR}/test_harness.h
    )

   if(CHECKPOINT_BUILD_TESTS_WITH_MPI)
      set(
        TEST_HEADER_FILES ${TEST_HEADER_FILES}
        ${PROJECT_TEST_MPI_UNIT_DIR}/mpi-init.h
        ${PROJECT_TEST_MPI_UNIT_DIR}/test_commons_mpi.h
      )
    endif()

    set(
      TEST_SOURCE_FILES
      ${PROJECT_TEST_UNIT_DIR}/main.cc
    )

    foreach(TEST_FULL ${PROJECT_TEST_LIST})
      GET_FILENAME_COMPONENT(
        TEST
        ${TEST_FULL}
        NAME_WE
      )

      add_executable(
        ${TEST}
        ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES}
        ${PROJECT_TEST_UNIT_DIR}/${TEST}.cc
      )

      checkpoint_link_target(${TEST} 0)

      add_test(
        ${TEST}
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST}
      )

      set_tests_properties(
        ${TEST}
        PROPERTIES TIMEOUT 60
        FAIL_REGULAR_EXPRESSION "FAILED;should be deleted but never is;WARNING"
        PASS_REGULAR_EXPRESSION "PASSED"
      )
    endforeach()

    if(CHECKPOINT_BUILD_TESTS_WITH_MPI)
      foreach(TEST_FULL ${PROJECT_TEST_LIST_MPI})
        GET_FILENAME_COMPONENT(
          TEST
          ${TEST_FULL}
          NAME_WE
        )

        add_executable(
          ${TEST}
          ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES}
          ${PROJECT_TEST_MPI_UNIT_DIR}/${TEST}.cc
        )

        checkpoint_link_target(${TEST} 1)

        set(
          CHECKPOINT_TEST_PARAM_MPI ${MPI_NUMPROC_FLAG}
          ${CHECK_POINT_MPI_PROC} "${CMAKE_CURRENT_BINARY_DIR}/${TEST}"
        )
        add_test(
          NAME ${TEST}
          COMMAND ${MPI_RUN_COMMAND} ${CHECKPOINT_TEST_PARAM_MPI}
        )

        set_tests_properties(
          ${TEST}
          PROPERTIES TIMEOUT 60
          FAIL_REGULAR_EXPRESSION "FAILED;should be deleted but never is;WARNING"
          PASS_REGULAR_EXPRESSION "PASSED"
        )

        endforeach()
    endif()
  endif()
endif()
