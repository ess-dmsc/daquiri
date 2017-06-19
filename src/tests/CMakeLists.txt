cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(daquiri_tests CXX)

enable_testing()

find_package(Threads)
find_package(GTest)

file(GLOB ${PROJECT_NAME}_sources *.cpp)
file(GLOB ${PROJECT_NAME}_headers *.h)
dirs_of(${PROJECT_NAME}_include_dirs "${${PROJECT_NAME}_headers}")

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_sources}
  ${${PROJECT_NAME}_headers}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${h5cc_INCLUDE_DIRS}
  PRIVATE ${GTEST_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${GTEST_BOTH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${h5cc_LIBRARIES}
  h5cc
  core
)

set(DO_COV OFF)

mark_as_advanced(DO_COV)

if (${COV})
    if (${CMAKE_COMPILER_IS_GNUCXX})
        message(STATUS "Code coverage enabled.")
        set(DO_COV ON)
        set(CMAKE_BUILD_TYPE Debug)
    else()
        message(FATAL_ERROR "Unable to enable code coverage as this functionality only works with the g++ compiler.")
    endif()
endif()

if(DO_COV)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-coverage -fprofile-generate")
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-g -O0 -coverage -fprofile-generate")

    find_program(GCOV_PATH gcov)
    find_program(GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/tests)
    if (NOT GCOV_PATH OR NOT GCOVR_PATH)
        message(WARNING "Unable to enable coverage target as gcov and/or gocvr was not found.")
    else()
        message(STATUS "Generating coverage target.")
        add_custom_target(coverage COMMAND ${GCOVR_PATH} "-r" "${CMAKE_SOURCE_DIR}/" "-x" "-o" "${CMAKE_BINARY_DIR}/cov.xml"
          "--object-directory=${PROJECT_BINARY_DIR}/CMakeFiles/daquiri_tests.dir/")
    endif()
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
