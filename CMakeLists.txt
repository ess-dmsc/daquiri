cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(daquiri
  LANGUAGES CXX C
  VERSION 0.0.1
  )

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

#=============================================================================
# ensure build type first
#=============================================================================

include(${EXTRA_MODULES_DIR}/EnsureBuildType.cmake)

#=============================================================================
# Conan
#=============================================================================

SET(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
SET(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path), MANUAL (expects conanbuildinfo.cmake in build directory) or DISABLE")
if(${CONAN} MATCHES "AUTO")
  include(${CMAKE_MODULE_PATH}/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.txt
          PROFILE ${CONAN_PROFILE}
          BASIC_SETUP NO_OUTPUT_DIRS SKIP_RPATH
          BUILD_TYPE "None"
          BUILD outdated)
elseif(${CONAN} MATCHES "MANUAL")
  if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS SKIP_RPATH)
  else()
    MESSAGE(FATAL_ERROR "CONAN set to MANUAL but no file named conanbuildinfo.cmake found in build directory")
  endif()
elseif(NOT ${CONAN} MATCHES "DISABLE")
  MESSAGE(FATAL_ERROR "Unrecognised option for CONAN (${CONAN}), use AUTO, MANUAL or DISABLE")
endif()

include(${EXTRA_MODULES_DIR}/OutputDirConfig.cmake)
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)

add_subdirectory(source)

include(${EXTRA_MODULES_DIR}/FindGTestFix.cmake)
if(GTest_FOUND)
  add_subdirectory(tests)
endif()

create_coverage_targets(
    coverage
    run_unit_tests
    ${PROJECT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/source/
    ${PROJECT_BINARY_DIR}/tests/coverage
)

