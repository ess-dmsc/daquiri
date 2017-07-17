cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(daquiri_gui CXX)

#For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)

file(GLOB ${PROJECT_NAME}_resources
  resources/*.qrc
  widgets/ColorWidgets/src/*.qrc)

file(GLOB ${PROJECT_NAME}_ui
  *.ui
  daq/*.ui
  widgets/*.ui
  widgets/ColorWidgets/src/*.ui)

file(GLOB ${PROJECT_NAME}_sources
  *.cpp
  daq/*.cpp
  widgets/*.cpp
  widgets/ColorWidgets/src/*.cpp)

file(GLOB ${PROJECT_NAME}_headers
  *.h
  daq/*.h
  widgets/*.h
  widgets/ColorWidgets/include/*.hpp
  ${PROJECT_BINARY_DIR}/*.h)

dirs_of(${PROJECT_NAME}_include_dirs "${${PROJECT_NAME}_headers}")

file(GLOB_RECURSE qplot_sources qplot/src/qplot/*.cpp)
file(GLOB_RECURSE qplot_headers qplot/src/qplot/*.h)
dirs_of(qplot_include_dirs "${qplot_headers}")


add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_resources}
  ${${PROJECT_NAME}_sources}
  ${${PROJECT_NAME}_headers}
  ${${PROJECT_NAME}_ui}
  ${qplot_sources}
  ${qplot_headers}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${${PROJECT_NAME}_include_dirs}
  PRIVATE ${PROJECT_BINARY_DIR}
  PRIVATE ${h5cc_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
  PRIVATE ${consumers_INCLUDE_DIRS}
  PRIVATE ${producers_INCLUDE_DIRS}
  PRIVATE ${qplot_include_dirs}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${CMAKE_THREAD_LIBS_INIT}
  ${h5cc_LIBRARIES}
  ${core_LIBRARIES}
  ${producers_LIBRARIES}
  ${consumers_LIBRARIES}
)

qt5_use_modules(${PROJECT_NAME} Widgets PrintSupport)
