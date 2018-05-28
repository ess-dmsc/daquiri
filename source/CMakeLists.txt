if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config/CMakeLists.txt")
  add_subdirectory(config)
endif ()

if (NOT DAQuiri_config)
  set(DAQuiri_gui TRUE)
  list(APPEND DAQuiri_enabled_producers ESSStream MockProducer)
endif ()

include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/BuildInfo.cmake)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
find_package(h5cpp 0.0.7 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(core)
add_subdirectory(consumers)
add_subdirectory(producers)

add_subdirectory(cmd)
add_subdirectory(gui)

add_subdirectory(tests)
