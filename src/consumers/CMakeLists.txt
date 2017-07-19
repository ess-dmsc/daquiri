cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(consumers CXX)

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp xylib/xylib/*.cpp)
file(GLOB ${PROJECT_NAME}_HEADERS *.h xylib/xylib/*.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

add_library(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
  PUBLIC ${core_INCLUDE_DIRS}
  PUBLIC ${h5cc_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${h5cc_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)

#set(${PROJECT_NAME}_LIBRARIES
#    -Wl,--whole-archive ${PROJECT_NAME} -Wl,--no-whole-archive
#    PARENT_SCOPE)
