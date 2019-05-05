set(this_target ${PROJECT_NAME})
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

include(QtLibraryConfig)

find_package(qplot REQUIRED)
#OpenGL for plots
#add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)

set(${this_target}_sources
  ${dir}/daquiri.cpp
  ${dir}/main.cpp
  ${dir}/Profiles.cpp
  ${dir}/ProfilesForm.cpp
  ${dir}/Server.cpp
  ${dir}/SettingsForm.cpp
  ${dir}/SettingsTreeModel.cpp
  ${dir}/ThreadRunner.cpp
  ${dir}/UnitConverter.cpp
  )

set(${this_target}_headers
  ${dir}/daquiri.h
  ${dir}/Profiles.h
  ${dir}/ProfilesForm.h
  ${dir}/QStreamBuffer.h
  ${dir}/Server.h
  ${dir}/SettingsForm.h
  ${dir}/SettingsTreeModel.h
  ${dir}/ThreadRunner.h
  ${dir}/UnitConverter.h
  )

set(${this_target}_ui
  ${dir}/ProfilesForm.ui
  ${dir}/SettingsForm.ui
  )

add_subdirectory(widgets)
add_subdirectory(daq)

qt5_add_resources(${this_target}_resources
  ${dir}/resources/daquiri.qrc)

set(CMAKE_AUTOUIC ON)

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
)

target_link_libraries(
  ${this_target}
  ${PROJECT_NAME}_core
  ${PROJECT_NAME}_producers
  ${PROJECT_NAME}_consumers
  QPlot
  QtColorWidgets-Qt52
  Qt5::Widgets
  Qt5::PrintSupport
  Qt5::Network
)
