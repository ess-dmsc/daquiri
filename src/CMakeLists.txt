cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(daquiri CXX)
option(test "Build all tests." ON)
option(COV "Enable code coverage test for unit tests (if possible)." OFF)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

#===========================================================
# Internal compiler options
#===========================================================
# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(APPLE)
  add_definitions(-DUSE_BOOST_MUTEX=1)
endif()

#=========================================================
# Function for deducing include dirs
# retname - name of parent variable to fill
# file_list - list of dirs, pass to fn as "${list_var}"
#=========================================================
function(dirs_of retname file_list)
  set(dlist "")
  foreach (_file ${file_list})
      get_filename_component(_dir ${_file} PATH)
      list (APPEND dlist ${_dir})
  endforeach()
  list(REMOVE_DUPLICATES dlist)
  set (${retname} ${dlist} PARENT_SCOPE)
endfunction(dirs_of)

# deprecate
add_definitions(-DBOOST_LOG_DYN_LINK)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
endif()

# json++
list (APPEND json_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/external/json/src)

add_subdirectory(external/h5/source/h5cc)

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
