
macro(add_test_for_example_checkpoint test_name test_exec)
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

if (${CHECKPOINT_BUILD_EXAMPLES})
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
    target_include_directories(${EXAMPLE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

    add_test_for_example_checkpoint(
      checkpoint:${EXAMPLE}
      ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}
    )

    target_link_libraries(
      ${EXAMPLE}
      PUBLIC
      ${SERDES_LIBRARY_NS}
    )

  if (${kokkos_DIR_FOUND})
    target_include_directories(${EXAMPLE} SYSTEM PUBLIC ${Kokkos_INCLUDE_DIRS})
    target_link_libraries(${EXAMPLE} PUBLIC ${Kokkos_LIBRARIES})

      find_package(OpenMP)
      if(OpenMP_CXX_FOUND)
        target_link_libraries(${EXAMPLE} PUBLIC OpenMP::OpenMP_CXX)
      endif()
  endif()

  endforeach()
endif()
