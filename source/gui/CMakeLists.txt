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
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${PROJECT_NAME}_producers
  PRIVATE ${PROJECT_NAME}_consumers
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt5::Widgets
  PRIVATE Qt5::PrintSupport
  PRIVATE Qt5::Network
)

# Link stdc++fs or c++experimental to get std::experimental::filesystem when necessary
target_link_libraries(${this_target}
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(${this_target}
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
