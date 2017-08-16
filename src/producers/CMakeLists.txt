cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(producers CXX)

file(GLOB ${PROJECT_NAME}_sources *.cpp)
file(GLOB ${PROJECT_NAME}_headers *.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_headers}")

add_library(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_sources}
  ${${PROJECT_NAME}_headers}
)

include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
)

add_subdirectory(DummyDevice)
LIST(APPEND prod_LIBRARIES DummyDevice)
include_directories(${PROJECT_NAME} PUBLIC ${DummyDevice_INCLUDE_DIRS})

add_subdirectory(MockProducer)
LIST(APPEND prod_LIBRARIES MockProducer)
include_directories(${PROJECT_NAME} PUBLIC ${MockProducer_INCLUDE_DIRS})

add_subdirectory(ESSStream)
LIST(APPEND prod_LIBRARIES ESSStream)
include_directories(${PROJECT_NAME} PUBLIC ${ESSStream_INCLUDE_DIRS})


target_link_libraries(
  ${PROJECT_NAME}
  ${core_LIBRARIES}
  ${prod_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)
