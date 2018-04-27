set(this_target ${PROJECT_NAME})

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

#For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

file(GLOB ${this_target}_resources resources/*.qrc)
file(GLOB ${this_target}_ui *.ui daq/*.ui)
file(GLOB ${this_target}_sources *.cpp daq/*.cpp)
file(GLOB ${this_target}_headers *.h daq/*.h ${PROJECT_BINARY_DIR}/*.h)
dirs_of(${this_target}_include_dirs "${${this_target}_headers}")

#OpenGL for plots
add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)

add_subdirectory(widgets)

add_executable(
    ${this_target}
    ${${this_target}_resources}
    ${${this_target}_sources}
    ${${this_target}_headers}
    ${${this_target}_ui}
)

target_include_directories(
    ${this_target}
    PRIVATE ${${this_target}_include_dirs}
)

target_link_libraries(
    ${this_target}
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_producers
    ${PROJECT_NAME}_consumers
    ${Boost_LIBRARIES}
    daquiri_widgets
    Qt5::Widgets
    Qt5::PrintSupport
)
