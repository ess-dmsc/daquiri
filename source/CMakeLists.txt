include(InstallConfig)
include(BuildInfo)

include(BoostLibraryConfig)
find_package(h5cpp 0.0.9 REQUIRED)
find_package(Threads REQUIRED)

find_package(GraylogLogger REQUIRED)
add_definitions("-DGRAYLOG")

find_package(fmt REQUIRED)
find_package(date REQUIRED)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(core)

add_subdirectory(consumers)
add_subdirectory(producers)

add_subdirectory(cmd)
add_subdirectory(gui)
