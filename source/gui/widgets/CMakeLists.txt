set(this_target daquiri_widgets)

file(GLOB ${this_target}_ui *.ui)
file(GLOB ${this_target}_sources *.cpp AnalogWidgets/*.cpp)
file(GLOB ${this_target}_headers *.h
    ${PROJECT_BINARY_DIR}/*.h
    AnalogWidgets/*.h)

dirs_of(${this_target}_include_dirs "${${this_target}_headers}")

#OpenGL for plots

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)

add_subdirectory(qplot/source)
add_subdirectory(ColorWidgets)

add_library(
    ${this_target} STATIC
    ${${this_target}_sources}
    ${${this_target}_headers}
    ${${this_target}_ui}
)

target_include_directories(
    ${this_target}
    PUBLIC ${${this_target}_include_dirs}
)

target_link_libraries(
    ${this_target}
    ${PROJECT_NAME}_core
    ${Boost_LIBRARIES}
    qplot
    QtColorWidgets
    Qt5::Widgets
    Qt5::PrintSupport
)
