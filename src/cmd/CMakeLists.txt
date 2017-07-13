cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(daquiri_cmd CXX)

#add_definitions(-DBOOST_LOG_DYN_LINK)

file(GLOB ${PROJECT_NAME}_sources *.cpp)
file(GLOB ${PROJECT_NAME}_headers *.h)
dirs_of(${PROJECT_NAME}_include_dirs "${${PROJECT_NAME}_headers}")

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_sources}
  ${${PROJECT_NAME}_headers}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${h5cc_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
  PRIVATE ${consumers_INCLUDE_DIRS}
  PRIVATE ${producers_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_THREAD_LIBS_INIT}
  ${h5cc_LIBRARIES}
  ${core_LIBRARIES}
  ${producers_LIBRARIES}
  ${consumers_LIBRARIES}
)
