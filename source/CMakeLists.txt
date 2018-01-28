if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
endif ()

if (NOT DAQuiri_config)
  set(DAQuiri_gui TRUE)
  list(APPEND DAQuiri_enabled_producers ESSStream)
endif ()

if (EXISTS ${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  message(STATUS "Using existing conanbuildinfo.cmake file")
  include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup(KEEP_RPATHS)
endif ()

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/OutputDirConfig.cmake)
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)
include(${EXTRA_MODULES_DIR}/BuildInfo.cmake)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
find_package(h5cpp)
find_package(Threads)

if (h5cpp_FOUND)
  add_definitions(-DDAQUIRI_USE_H5=1)
endif()

add_subdirectory(core)
add_subdirectory(consumers)
add_subdirectory(producers)

if (DAQuiri_cmd)
  add_subdirectory(cmd)
endif ()

if (DAQuiri_gui)
  add_subdirectory(gui)
endif ()

#option(test "Build all tests." ON)
#if (test)
  add_subdirectory(tests)
#endif ()
