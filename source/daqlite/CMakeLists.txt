include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)


set(daqlite_src
  daqlite.cpp
  Configuration.cpp
  Custom2DPlot.cpp
  CustomAMOR2DTOFPlot.cpp
  CustomTofPlot.cpp
  ESSConsumer.cpp
  MainWindow.cpp
  WorkerThread.cpp
  )

set(daqlite_inc
  Configuration.h
  Custom2DPlot.h
  CustomAMOR2DTOFPlot.h
  CustomTofPlot.h
  ESSConsumer.h
  MainWindow.h
  WorkerThread.h
  )

set(daqlite_ui
  MainWindow.ui
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  daqlite
  ${daqlite_src}
  ${daqlite_inc}
  ${daqlite_ui}
)

target_link_libraries(
  daqlite
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt5::Widgets
)

target_link_libraries(daqlite
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(daqlite
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
