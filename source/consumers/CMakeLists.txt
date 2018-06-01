set(this_target ${PROJECT_NAME}_consumers)

find_package(Eigen3 REQUIRED)

file(GLOB_RECURSE ${this_target}_SOURCES *.cpp)
file(GLOB_RECURSE ${this_target}_HEADERS *.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target} STATIC
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PUBLIC ${${this_target}_INCLUDE_DIRS}
  PRIVATE ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(
  ${this_target}
  PUBLIC ${PROJECT_NAME}_core
)

enable_coverage(${this_target})

