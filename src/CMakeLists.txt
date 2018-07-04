set(
  TOP_LEVEL_SUBDIRS
  traits container serializers traits buffer dispatch
)

foreach(DIR ${TOP_LEVEL_SUBDIRS})
  install(
    DIRECTORY ${DIR} DESTINATION include
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

file(GLOB TOP_HEADERS *.h)
install(FILES ${TOP_HEADERS} DESTINATION include)

file(
  GLOB
  HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(
  GLOB
  SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

foreach(SUB_DIR ${TOP_LEVEL_SUBDIRS})
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
endforeach()

add_library(
  ${SERDES_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

add_library(${SERDES_LIBRARY_NS} ALIAS ${SERDES_LIBRARY})

target_compile_options(${SERDES_LIBRARY} PUBLIC "-fPIC")
target_compile_options(${SERDES_LIBRARY} PUBLIC ${CXX_STANDARD_FLAGS})

target_compile_definitions(${SERDES_LIBRARY} PUBLIC HAS_DETECTION_COMPONENT=1)

target_link_libraries(${SERDES_LIBRARY} vt::lib::detector)
target_link_libraries(${SERDES_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(
  ${SERDES_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(
  ${SERDES_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS                   ${SERDES_LIBRARY}
  EXPORT                    ${SERDES_LIBRARY}
  LIBRARY DESTINATION       lib
  ARCHIVE DESTINATION       lib
  RUNTIME DESTINATION       bin
  INCLUDES DESTINATION      include
)

install(
  EXPORT                    ${SERDES_LIBRARY}
  DESTINATION               cmake
  FILE                      "checkpointTargets.cmake"
  NAMESPACE                 vt::lib::
  COMPONENT                 runtime
)

export(
  TARGETS                   ${SERDES_LIBRARY}
  FILE                      "darma_checkpointTargets.cmake"
  NAMESPACE                 darma::backend::
)
