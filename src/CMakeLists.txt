cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(daquiri
  LANGUAGES CXX C
#  VERSION 0.0.1
  )

option(test "Build all tests." ON)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

#set(EXTRA_MODULES_DIR ${PROJECT_SOURCE_DIR}/cmake)
set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)

if(APPLE)
  add_definitions(-DUSE_BOOST_MUTEX=1)
endif()

# deprecate
add_definitions(-DBOOST_LOG_DYN_LINK)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
endif()

find_package(h5cpp REQUIRED)

add_subdirectory(core)
add_subdirectory(consumers)
add_subdirectory(producers)

if (DAQuiri_cmd)
  add_subdirectory(cmd)
endif()

if (DAQuiri_gui)
  add_subdirectory(gui)
endif()

if (test)
  add_subdirectory(tests)
endif()
