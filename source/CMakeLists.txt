include(InstallConfig)
include(BuildInfo)

include(BoostLibraryConfig)
find_package(h5cpp 0.0.7 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(core)

add_subdirectory(consumers)
add_subdirectory(producers)

add_subdirectory(cmd)
add_subdirectory(gui)
