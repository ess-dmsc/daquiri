set(this_target ${PROJECT_NAME}_producers)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

find_package(RdKafka REQUIRED)

set(SOURCES
  ${dir}/producers_autoreg.cpp
  )

set(HEADERS
  ${dir}/producers_autoreg.h
  )

set(${this_target}_headers ${HEADERS})
set(${this_target}_sources ${SOURCES})
add_subdirectory(DetectorIndex)
add_subdirectory(DummyDevice)
add_subdirectory(MockProducer)
add_subdirectory(ESSStream)

add_library(
  ${this_target} STATIC
  ${${this_target}_headers}
  ${${this_target}_sources}
)

target_include_directories(
  ${this_target}
  PUBLIC ${PROJECT_SOURCE_DIR}/source
  PRIVATE ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(
  ${this_target}
  PUBLIC ${PROJECT_NAME}_core
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
)

#enable_coverage(${this_target})
