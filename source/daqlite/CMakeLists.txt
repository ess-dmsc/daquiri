set(this_target daqlite)
set(dir ${CMAKE_CURRENT_SOURCE_DIR})

include(QtLibraryConfig)
find_package(LibRDKafka REQUIRED)

find_package(qplot REQUIRED)
#OpenGL for plots
#add_definitions(-DQCUSTOMPLOT_USE_OPENGL=1)

set(${this_target}_sources
  ${dir}/daqlite.cpp
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

target_include_directories(
  ${this_target}
  PRIVATE ${PROJECT_SOURCE_DIR}/source
)

target_link_libraries(
  ${this_target}
  PUBLIC fmt::fmt
  PRIVATE ${LibRDKafka_LIBRARIES}
  PRIVATE ${LibRDKafka_C_LIBRARIES}
  QPlot
)
