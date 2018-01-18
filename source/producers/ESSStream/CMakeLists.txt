set(this_target ESSStream)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake_modules)

find_package(LibRDKafka REQUIRED)

file(GLOB ${this_target}_SOURCES *.cpp)
file(GLOB ${this_target}_HEADERS *.h ${CMAKE_CURRENT_LIST_DIR}/logical-geometry/cpp/src/*.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target}
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PRIVATE ${${this_target}_INCLUDE_DIRS}
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/logical-geometry/cpp/src
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${LibRDKafka_LIBRARIES}
  PRIVATE ${LibRDKafka_C_LIBRARIES}
)
