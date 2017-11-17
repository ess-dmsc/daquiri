set(this_target unit_tests)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
include(${EXTRA_MODULES_DIR}/FindGTestFix.cmake)

if (NOT GTest_FOUND)
  message(WARNING "GTest not found. No tests will be built.")
  return()
endif ()

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB ${this_target}_sources ${dir}/*.cpp)
file(GLOB ${this_target}_headers ${dir}/*.h)
dirs_of(${this_target}_include_dirs "${${this_target}_headers}")

add_executable(
  ${this_target} EXCLUDE_FROM_ALL
  ${${this_target}_sources}
  ${${this_target}_headers}
)

target_include_directories(
  ${this_target}
  PRIVATE ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${GTEST_LIBRARIES}
  PRIVATE ${CMAKE_THREAD_LIBS_INIT}
  PRIVATE ${Boost_LIBRARIES}
  PRIVATE ${COVERAGE_LIBRARIES}
)

enable_coverage(${this_target})

add_custom_target(run_tests
  COMMAND ${this_target} "--gtest_output=xml:${PROJECT_BINARY_DIR}/tests/test_results.xml"
  DEPENDS ${this_target})
add_test(NAME "RunGoogleTests" COMMAND run_tests)

create_coverage_targets(
  coverage
  run_tests
  ${PROJECT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/source/core
  ${PROJECT_BINARY_DIR}/tests/coverage
)

