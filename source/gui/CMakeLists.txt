set(this_target ${PROJECT_NAME})
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

#For Qt
if(APPLE AND EXISTS /usr/local/opt/qt)
  # Homebrew installs Qt5 (up to at least 5.9.1) in
  # /usr/local/opt/qt, ensure it can be found by CMake since
  # it is not in the default /usr/local prefix.
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt")
endif()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

find_package(qplot REQUIRED)

set(SOURCES
  ${dir}/daquiri.cpp
  ${dir}/main.cpp
  ${dir}/Profiles.cpp
  ${dir}/ProfilesForm.cpp
  ${dir}/SettingsForm.cpp
  ${dir}/SettingsTreeModel.cpp
  ${dir}/ThreadRunner.cpp
  ${dir}/UnitConverter.cpp
  )

set(HEADERS
  ${dir}/daquiri.h
  ${dir}/Profiles.h
  ${dir}/ProfilesForm.h
  ${dir}/SettingsForm.h
  ${dir}/SettingsTreeModel.h
  ${dir}/ThreadRunner.h
  ${dir}/UnitConverter.h
  ${dir}/qt_boost_logger.h
  )

set(UI
  ${dir}/ProfilesForm.ui
  ${dir}/SettingsForm.ui
  )

set(RC
  ${dir}/resources/daquiri.qrc
  )

set(${this_target}_headers ${HEADERS})
set(${this_target}_sources ${SOURCES})
set(${this_target}_ui ${UI})

add_subdirectory(widgets)
add_subdirectory(daq)

#OpenGL for plots
add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)

add_executable(
  ${this_target}
  ${${this_target}_resources}
  ${${this_target}_sources}
  ${${this_target}_headers}
  ${${this_target}_ui}
)

target_include_directories(
  ${this_target}
  PRIVATE ${PROJECT_SOURCE_DIR}/source
  PRIVATE ${${this_target}_include_dirs}
)

target_link_libraries(
  ${this_target}
  ${PROJECT_NAME}_core
  ${PROJECT_NAME}_producers
  ${PROJECT_NAME}_consumers
  ${Boost_LIBRARIES}
  QPlot
  QtColorWidgets-Qt52
  Qt5::Widgets
  Qt5::PrintSupport
)
