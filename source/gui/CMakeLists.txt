set(this_target ${PROJECT_NAME})
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

include(BoostLibraryConfig)
include(QtLibraryConfig)

find_package(qplot REQUIRED)
#OpenGL for plots
#add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)


#find package for QtColorWidgets

set(${this_target}_sources
  ${dir}/daquiri.cpp
  ${dir}/main.cpp
  ${dir}/Profiles.cpp
  ${dir}/ProfilesForm.cpp
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
  ${dir}/SettingsForm.h
  ${dir}/SettingsTreeModel.h
  ${dir}/ThreadRunner.h
  ${dir}/UnitConverter.h
  )

set(CMAKE_AUTOUIC ON)
set(${this_target}_ui
  ${dir}/ProfilesForm.ui
  ${dir}/SettingsForm.ui
  )

qt5_add_resources(${this_target}_resources
  ${dir}/resources/daquiri.qrc)

add_subdirectory(widgets)
add_subdirectory(daq)

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
