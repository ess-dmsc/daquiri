if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
endif ()

if (NOT DAQuiri_config)
  set(DAQuiri_gui TRUE)
  list(APPEND DAQuiri_enabled_producers ESSStream)
endif ()

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/BuildInfo.cmake)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
find_package(h5cpp 0.0.5 REQUIRED)
find_package(Threads REQUIRED)

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
