set(this_target DummyDevice)

file(GLOB_RECURSE ${this_target}_SOURCES *.cpp)
file(GLOB_RECURSE ${this_target}_HEADERS *.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target}
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PRIVATE ${${this_target}_INCLUDE_DIRS}
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
)
