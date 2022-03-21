set(this_target ${PROJECT_NAME}_core)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Consumer.cpp
  ${dir}/ConsumerFactory.cpp
  ${dir}/ConsumerMetadata.cpp
  ${dir}/Dataspace.cpp
  ${dir}/Detector.cpp
  ${dir}/Engine.cpp
  ${dir}/Producer.cpp
  ${dir}/ProducerFactory.cpp
  ${dir}/Project.cpp
  ${dir}/Spill.cpp
  )

set(HEADERS
  ${dir}/Consumer.h
  ${dir}/ConsumerFactory.h
  ${dir}/ConsumerMetadata.h
  ${dir}/Dataspace.h
  ${dir}/Detector.h
  ${dir}/Engine.h
  ${dir}/Producer.h
  ${dir}/ProducerFactory.h
  ${dir}/Project.h
  ${dir}/Spill.h

  ${dir}/Event.h
  ${dir}/EventModel.h
  ${dir}/SpillDequeue.h
  ${dir}/thread_wrappers.h
  ${dir}/TimeBase.h
  ${dir}/TimeStamp.h
  )

set(${this_target}_headers ${HEADERS})
set(${this_target}_sources ${SOURCES})

add_subdirectory(util)
add_subdirectory(calibration)
add_subdirectory(plugin)

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
  PUBLIC h5cpp::h5cpp
  PUBLIC fmt::fmt
  PUBLIC spdlog::spdlog
  PUBLIC ${DATE_LIBRARIES}
  PRIVATE Threads::Threads
)

enable_coverage(${this_target})

# this will affect what version metadata gets saved to file in Project.cpp
option(BI "Update build info at compile time" OFF)
if(${BI})
  add_dependencies(${this_target} build_time)
  add_definitions(-DBUILD_TIME="TRUE")
endif()
