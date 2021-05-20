set(this_target daqlite)

include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

set(${this_target}_sources
  daqlite.cpp
  Configuration.cpp
  Custom2DPlot.cpp
  CustomTofPlot.cpp
  ESSConsumer.cpp
  MainWindow.cpp
  WorkerThread.cpp
  )

set(${this_target}_headers
  Configuration.h
  Custom2DPlot.h
  CustomTofPlot.h
  ESSConsumer.h
  MainWindow.h
  WorkerThread.h
  )

set(${this_target}_ui
  MainWindow.ui
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  ${this_target}
  ${${this_target}_sources}
  ${${this_target}_headers}
  ${${this_target}_ui}
)

target_link_libraries(
  ${this_target}
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  QPlot
  QtColorWidgets-Qt52
  Qt5::Widgets
)
