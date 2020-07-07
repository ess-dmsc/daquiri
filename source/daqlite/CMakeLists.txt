set(this_target daqlite)

include(QtLibraryConfig)
find_package(LibRDKafka REQUIRED)
find_package(qplot REQUIRED)

set(${this_target}_sources
  daqlite.cpp
  Configuration.cpp
  Custom2DPlot.cpp
  ESSConsumer.cpp
  MainWindow.cpp
  WorkerThread.cpp
  )

set(${this_target}_headers
  Configuration.h
  Custom2DPlot.h
  ESSConsumer.h
  MainWindow.h
  WorkerThread.h
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  ${this_target}
  ${${this_target}_sources}
  ${${this_target}_headers}
)

target_link_libraries(
  ${this_target}
  PUBLIC fmt::fmt
  PRIVATE ${LibRDKafka_LIBRARIES}
  PRIVATE ${LibRDKafka_C_LIBRARIES}
  QPlot
  QtColorWidgets-Qt52
  Qt5::Widgets
)
