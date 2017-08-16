cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(DummyDevice CXX)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES *.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS *.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

add_library(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
)

include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${core_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)
