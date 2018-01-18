set(this_target ${PROJECT_NAME})

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

#For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)

file(GLOB ${this_target}_resources
  resources/*.qrc
  qplot/src/qplot/*.qrc
  widgets/ColorWidgets/src/*.qrc)

file(GLOB ${this_target}_ui
  *.ui
  daq/*.ui
  widgets/*.ui
  widgets/ColorWidgets/src/*.ui)

file(GLOB ${this_target}_sources
  *.cpp
  daq/*.cpp
  widgets/*.cpp
  widgets/ColorWidgets/src/*.cpp)

file(GLOB ${this_target}_headers
  *.h
  daq/*.h
  widgets/*.h
  widgets/ColorWidgets/include/*.hpp
  ${PROJECT_BINARY_DIR}/*.h)

dirs_of(${this_target}_include_dirs "${${this_target}_headers}")

file(GLOB_RECURSE qplot_sources qplot/src/qplot/*.cpp)
file(GLOB_RECURSE qplot_headers qplot/src/qplot/*.h)
dirs_of(qplot_include_dirs "${qplot_headers}")

#OpenGL for plots
find_package(OpenGL REQUIRED)
add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)

add_executable(
  ${this_target}
  ${${this_target}_resources}
  ${${this_target}_sources}
  ${${this_target}_headers}
  ${${this_target}_ui}
  ${qplot_sources}
  ${qplot_headers}
)

target_include_directories(
  ${this_target}
  PRIVATE ${${this_target}_include_dirs}
  PRIVATE ${qplot_include_dirs}
  PRIVATE ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(
  ${this_target}
  ${PROJECT_NAME}_core
  ${PROJECT_NAME}_producers
  ${PROJECT_NAME}_consumers
  ${OPENGL_LIBRARIES}
  ${Boost_LIBRARIES}
)

qt5_use_modules(${this_target} Widgets PrintSupport)
