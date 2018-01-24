set(this_target ${PROJECT_NAME}_core)

if(APPLE)
  add_definitions(-DUSE_BOOST_MUTEX=1)
endif()

file(GLOB ${this_target}_SOURCES *.cpp util/*.cpp)
file(GLOB ${this_target}_HEADERS *.h util/*.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target}
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PUBLIC ${${this_target}_INCLUDE_DIRS}
  PUBLIC ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
  ${this_target}
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC ${COVERAGE_LIBRARIES}
  PUBLIC h5cpp
)

enable_coverage(${this_target})

#ADD_DEPENDENCIES(${this_target} buildinfo)
