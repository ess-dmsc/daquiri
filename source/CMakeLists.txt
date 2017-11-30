if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
else ()
  set(DAQuiri_gui TRUE)
  list(APPEND DAQuiri_enabled_producers ESSStream)
endif ()

set(EXTRA_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/OutputDirConfig.cmake)
include(${EXTRA_MODULES_DIR}/CoverageReports.cmake)
include(${EXTRA_MODULES_DIR}/BuildInfo.cmake)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
find_package(h5cpp REQUIRED)

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
