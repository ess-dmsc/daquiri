set(this_target unit_tests)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ${this_target}_sources ${dir}/core/*.cpp ${dir}/consumers/*.cpp)
file(GLOB_RECURSE ${this_target}_headers ${dir}/core/*.h ${dir}/consumers/*.h)

add_executable(
    ${this_target} EXCLUDE_FROM_ALL
    ${${this_target}_sources}
    ${${this_target}_headers}
    ${dir}/gtest_color_print.h
    ${dir}/main.cpp
)

target_include_directories(
    ${this_target}
    PRIVATE ${GTEST_INCLUDE_DIRS}
    PRIVATE ${dir}
)

target_link_libraries(
    ${this_target}
    PRIVATE ${PROJECT_NAME}_core
    PRIVATE ${PROJECT_NAME}_consumers
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

###############
# SYSTEM TEST #
###############

add_subdirectory(system_tests)

#############
# ALL TESTS #
#############

add_custom_target(all_tests
    DEPENDS unit_tests system_test)

add_custom_target(run_tests
    DEPENDS run_unit_tests run_system_test)
