cmake_minimum_required(VERSION 3.8)
project(serdes)

set(CMAKE_CXX_STANDARD 14)

enable_language(CXX)

include (CTest)
enable_testing()

if(NOT CMAKE_DETECTOR_PATH)
  message("Detection library not found")
else()
  add_compile_options(-DHAS_DETECTION_COMPONENT=1)
  include_directories(${CMAKE_DETECTOR_PATH}/src/)
  message("Detection library found: ${CMAKE_DETECTOR_PATH}")
endif()

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

set(
  PROJECT_SUBDIRS_LIST
  traits container serializers traits buffer dispatch
)

file(
  GLOB
  HEADER_FILES
  ${PROJECT_SOURCE_DIR}/*.h
)

file(
  GLOB
  SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/*.cc
)

install(FILES ${HEADER_FILES} DESTINATION include)

#message("headers=${HEADER_FILES}")
#message("source=${SOURCE_FILES}")

foreach(SUB_DIR ${PROJECT_SUBDIRS_LIST})

  file(
    GLOB
    "${SUB_DIR}_HEADER_FILES"
    ${PROJECT_SOURCE_DIR}/${SUB_DIR}/*.h
  )

  file(
    GLOB
    "${SUB_DIR}_SOURCE_FILES"
    ${PROJECT_SOURCE_DIR}/${SUB_DIR}/*.cc
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
  
  install(FILES ${${SUB_DIR}_HEADER_FILES} DESTINATION include/${SUB_DIR})

endforeach()

set(SERDES_LIBRARY serialization)

add_library(
  ${SERDES_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

install(TARGETS ${SERDES_LIBRARY} EXPORT ${SERDES_LIBRARY} DESTINATION lib)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_EXAMPLE_DIR})

macro(add_test_for_example test_name test_exec)
  add_test(
    ${test_name} ${test_exec} ${ARGN}
  )

  set_tests_properties(
    ${test_name}
    PROPERTIES TIMEOUT 300 FAIL_REGULAR_EXPRESSION "FAILED;WARNING"
  )
endmacro()

file(
  GLOB
  PROJECT_EXAMPLES
  RELATIVE
  "${PROJECT_EXAMPLE_DIR}"
  "${PROJECT_EXAMPLE_DIR}/*.cc"
)

if (${CHECKPOINT_BUILD_SAMPLES})
  foreach(EXAMPLE_FULL ${PROJECT_EXAMPLES})
    GET_FILENAME_COMPONENT(
      EXAMPLE
      ${EXAMPLE_FULL}
      NAME_WE
    )

    add_executable(
      ${EXAMPLE}
      ${PROJECT_EXAMPLE_DIR}/${EXAMPLE}.cc
    )

    add_test_for_example(
      ${EXAMPLE}
      ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}
    )

    target_link_libraries(
      ${EXAMPLE}
      ${SERDES_LIBRARY}
    )
  endforeach()
endif()

set(PROJECT_TEST_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit)
set(PROJECT_TESTS ${PROJECT_TEST_UNIT_DIR}/tests)

file(
  GLOB
  PROJECT_TEST_LIST
  RELATIVE
  "${PROJECT_TESTS}"
  "${PROJECT_TESTS}/*.cc"
)

if (${CHECKPOINT_BUILD_TESTS})
  if (NOT GTEST_DIR)
    message("Unit test are disabled because GoogleTest is not setup via -DGTEST_DIR=")
  else()
    message("GTEST_DIR is set: unit tests are enabled GTEST_DIR=${GTEST_DIR}")

    set(
      TEST_HEADER_FILES
      ${PROJECT_TEST_UNIT_DIR}/test_harness.h
    )

    set(
      TEST_SOURCE_FILES
      ${PROJECT_TEST_UNIT_DIR}/main.cc
    )

    include_directories(${GTEST_DIR}/include)
    include_directories(${PROJECT_TEST_UNIT_DIR})

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${GTEST_DIR}/lib")

    foreach(TEST_FULL ${PROJECT_TEST_LIST})
      GET_FILENAME_COMPONENT(
        TEST
        ${TEST_FULL}
        NAME_WE
      )

      add_executable(
        ${TEST}
        ${TEST_SOURCE_FILES} ${TEST_HEADER_FILES}
        ${PROJECT_TESTS}/${TEST}.cc
      )

      target_link_libraries(
        ${TEST}
        ${SERDES_LIBRARY}
        gtest gmock
      )

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
