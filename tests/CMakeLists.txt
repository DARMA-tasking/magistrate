
set(PROJECT_TEST_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit)

file(
  GLOB
  PROJECT_TEST_LIST
  RELATIVE
  "${PROJECT_TEST_UNIT_DIR}"
  "${PROJECT_TEST_UNIT_DIR}/*.cc"
)

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
      ${PROJECT_TEST_UNIT_DIR}/test_harness.h
    )

    set(
      TEST_SOURCE_FILES
      ${PROJECT_TEST_UNIT_DIR}/main.cc
    )

    include_directories(${PROJECT_TEST_UNIT_DIR})

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

      target_link_libraries(${TEST} PUBLIC ${SERDES_LIBRARY})
      if (${kokkos_DIR_FOUND})
        target_link_libraries(${TEST} PUBLIC kokkos)
      endif()
      target_link_libraries(${TEST} PRIVATE GTest::GTest)
      target_link_libraries(${TEST} PRIVATE GTest::Main)

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
  endif()
endif()
