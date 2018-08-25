include(InstallConfig)
include(BuildInfo)

find_package(h5cpp 0.0.7 REQUIRED)
find_package(Threads REQUIRED)

find_package(GraylogLogger REQUIRED)
add_definitions("-DGRAYLOG")

find_package(fmt REQUIRED)

add_subdirectory(core)

add_subdirectory(consumers)
add_subdirectory(producers)

add_subdirectory(cmd)
add_subdirectory(gui)
