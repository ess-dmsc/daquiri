set(this_target ${PROJECT_NAME}_core)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/consumer.cpp
  ${dir}/consumer_factory.cpp
  ${dir}/consumer_metadata.cpp
  ${dir}/dataspace.cpp
  ${dir}/detector.cpp
  ${dir}/engine.cpp
  ${dir}/producer.cpp
  ${dir}/producer_factory.cpp
  ${dir}/project.cpp
  ${dir}/spill.cpp
  )

set(HEADERS
  ${dir}/consumer.h
  ${dir}/consumer_factory.h
  ${dir}/consumer_metadata.h
  ${dir}/dataspace.h
  ${dir}/detector.h
  ${dir}/engine.h
  ${dir}/producer.h
  ${dir}/producer_factory.h
  ${dir}/project.h
  ${dir}/spill.h

  ${dir}/event.h
  ${dir}/event_model.h
  ${dir}/spill_dequeue.h
  ${dir}/thread_wrappers.h
  ${dir}/time_base.h
  ${dir}/time_stamp.h
  )

set(${this_target}_headers ${HEADERS})
set(${this_target}_sources ${SOURCES})
add_subdirectory(calibration)
add_subdirectory(plugin)
add_subdirectory(util)

add_library(
  ${this_target} STATIC
  ${${this_target}_headers}
  ${${this_target}_sources}
)

target_include_directories(
  ${this_target}
  PUBLIC ${PROJECT_SOURCE_DIR}/source
  PUBLIC ${CMAKE_BINARY_DIR}/build_info
  INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
  ${this_target}
  PUBLIC ${COVERAGE_LIBRARIES}
  PUBLIC ${GraylogLogger_STATIC_LIBRARIES}
  PUBLIC h5cpp
  PUBLIC fmt::fmt
  PUBLIC ${DATE_LIBRARIES}
  PRIVATE Threads::Threads
)

enable_coverage(${this_target})

ADD_DEPENDENCIES(${this_target} build_time)
