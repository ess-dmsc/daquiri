set(this_target unit_tests)
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(${this_target}_headers
  gtest_color_print.h)
set(${this_target}_sources
  main.cpp)

add_subdirectory(core)
add_subdirectory(consumers)

add_executable(
    ${this_target} EXCLUDE_FROM_ALL
    ${${this_target}_sources}
    ${${this_target}_headers}
)

target_include_directories(
    ${this_target}
    PRIVATE ${GTest_INCLUDE_DIRS}
    PRIVATE ${dir}
)

target_link_libraries(
    ${this_target}
    PRIVATE ${PROJECT_NAME}_core
    PRIVATE ${PROJECT_NAME}_consumers
    PRIVATE ${GTest_LIBRARIES}
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    PRIVATE ${COVERAGE_LIBRARIES}
)

enable_coverage(${this_target})
set_target_properties(${this_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_custom_target(run_unit_tests
    COMMAND ${this_target} "--gtest_output=xml:${PROJECT_BINARY_DIR}/tests/test_results.xml"
    DEPENDS ${this_target})
add_test(NAME "RunGoogleTests" COMMAND run_unit_tests)

add_subdirectory(system_tests)
add_subdirectory(gui)

#############
# ALL TESTS #
#############

add_custom_target(all_tests DEPENDS
  unit_tests
  system_test
  gui_tests
  )

add_custom_target(run_tests DEPENDS
  run_unit_tests
  run_system_test
  run_gui_tests
  )
