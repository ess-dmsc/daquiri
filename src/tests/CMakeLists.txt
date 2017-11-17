include(${EXTRA_MODULES_DIR}/FindGTestFix.cmake)

if (NOT GTest_FOUND)
  message(WARNING "GTest not found. No tests will be built.")
  return()
endif ()

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB ${PROJECT_NAME}_sources ${dir}/*.cpp)
file(GLOB ${PROJECT_NAME}_headers ${dir}/*.h)
dirs_of(${PROJECT_NAME}_include_dirs "${${PROJECT_NAME}_headers}")


add_executable(daquiri_unit_tests
  EXCLUDE_FROM_ALL
  ${${PROJECT_NAME}_sources}
  ${${PROJECT_NAME}_headers}
  )
add_dependencies(daquiri_unit_tests core)
target_include_directories(daquiri_unit_tests
  PRIVATE ${GTEST_INCLUDE_DIRS}
  PRIVATE core)
target_link_libraries(daquiri_unit_tests
  core
  ${GTEST_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${COVERAGE_LIBRARIES})
enable_coverage(daquiri_unit_tests)

add_custom_target(daquiri_run_tests
  COMMAND daquiri_unit_tests "--gtest_output=xml:daquiri_unit_tests_run.xml"
  DEPENDS daquiri_unit_tests)
add_test(NAME "RunGoogleTests" COMMAND daquiri_run_tests)

create_coverage_targets(
  daquiri_generate_coverage
  daquiri_run_tests
  ${PROJECT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/h5cpp
  ${PROJECT_BINARY_DIR}/coverage)

