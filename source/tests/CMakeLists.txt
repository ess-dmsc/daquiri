include(${EXTRA_MODULES_DIR}/FindGTestFix.cmake)
if (NOT GTest_FOUND)
  message(WARNING "GTest not found. No tests will be built.")
  return()
endif ()

set(this_target unit_tests)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

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
set_target_properties(${this_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_custom_target(run_unit_tests
    COMMAND ${this_target} "--gtest_output=xml:${PROJECT_BINARY_DIR}/tests/test_results.xml"
    DEPENDS ${this_target})
add_test(NAME "RunGoogleTests" COMMAND run_unit_tests)

create_coverage_targets(
  coverage
  run_unit_tests
  ${PROJECT_BINARY_DIR}/tests
  ${CMAKE_SOURCE_DIR}/source/core
  ${PROJECT_BINARY_DIR}/tests/coverage
)

###############
# SYSTEM TEST #
###############

add_executable(
    system_test EXCLUDE_FROM_ALL
    system_test/main.cpp
)

target_link_libraries(
    system_test
    PRIVATE ${PROJECT_NAME}_core
    PRIVATE ${PROJECT_NAME}_producers
    PRIVATE ${PROJECT_NAME}_consumers
    PRIVATE ${Boost_LIBRARIES}
)

set_target_properties(system_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_custom_target(run_system_test
    COMMAND system_test
    DEPENDS system_test)
add_test(NAME "RunSystemTest" COMMAND run_system_test)


#############
# ALL TESTS #
#############

add_custom_target(all_tests
    DEPENDS unit_tests system_test)

add_custom_target(run_tests
    DEPENDS run_unit_tests run_system_test)
