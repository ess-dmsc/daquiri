set(this_target ${PROJECT_NAME})

include(QtLibraryConfig)

find_package(qplot REQUIRED)

set(${this_target}_sources
  daquiri.cpp
  main.cpp
  Profiles.cpp
  ProfilesForm.cpp
  Server.cpp
  SettingsForm.cpp
  SettingsTreeModel.cpp
  ThreadRunner.cpp
  UnitConverter.cpp
  )

set(${this_target}_headers
  daquiri.h
  Profiles.h
  ProfilesForm.h
  QStreamBuffer.h
  Server.h
  SettingsForm.h
  SettingsTreeModel.h
  ThreadRunner.h
  UnitConverter.h
  )

set(${this_target}_ui
  ProfilesForm.ui
  SettingsForm.ui
  )

add_subdirectory(widgets)
add_subdirectory(daq)

qt5_add_resources(${this_target}_resources
  resources/daquiri.qrc)

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
