cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(consumers CXX)

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)
file(GLOB ${PROJECT_NAME}_HEADERS *.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

add_library(
  ${PROJECT_NAME}
  ${consumers_SOURCES}
  ${consumers_HEADERS}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${core_INCLUDE_DIRS}
  PRIVATE ${consumers_INCLUDE_DIRS}
  PRIVATE ${h5cc_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${h5cc_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
    ${h5cc_INCLUDE_DIRS}
    ${json_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    -Wl,--whole-archive ${PROJECT_NAME} -Wl,--no-whole-archive
    PARENT_SCOPE)
