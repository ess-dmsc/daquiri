cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(daquiri
  LANGUAGES CXX C
#  VERSION 0.0.1
  )

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

#=============================================================================
# ensure build type first
#=============================================================================

include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)
include(${EXTRA_MODULES_DIR}/EnsureBuildType.cmake)

#=============================================================================
# setting up conan for use from within cmake
#=============================================================================
set(WITH_CONAN OFF CACHE BOOL "Enable dependency build with conan")

if (NOT EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  set (WITH_CONAN ON)
endif()

if (WITH_CONAN)
  include(${EXTRA_MODULES_DIR}/ConanSetup.cmake)
  conan_cmake_run(CONANFILE "conanfile.txt"
    BASIC_SETUP UPDATE
    BUILD outdated)
endif ()

if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(KEEP_RPATHS)
endif ()

include(${EXTRA_MODULES_DIR}/OutputDirConfig.cmake)

add_subdirectory(source)


