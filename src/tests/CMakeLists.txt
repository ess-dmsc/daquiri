cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(daquiri_tests CXX)

enable_testing()

find_package(Threads)
find_package(GTest)

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
  PRIVATE ${GTEST_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${h5cc_LIBRARIES}
  h5cc
  core
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
