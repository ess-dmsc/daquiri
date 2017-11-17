cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(ESSStream CXX)

add_subdirectory(streaming-data-types)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

find_package(LibRDKafka)
if(NOT LIBRDKAFKA_FOUND)
    message(FATAL_ERROR "Unable to proceed as librdkafka could not be found.")
endif()

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
  PUBLIC ${core_INCLUDE_DIRS}
  PRIVATE ${PROJECT_BINARY_DIR}/streaming-data-types/schemas
)

target_link_libraries(
  ${PROJECT_NAME}
  ${core_LIBRARIES}
  ${LibRDKafka_LIBRARIES}
)

add_dependencies(${PROJECT_NAME} flatbuffers_generate)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)
