include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

#include_directories(.)

set(sniffer_src
  sniffer.cpp
  ESSConsumer.cpp
  )

set(sniffer_inc
  ESSConsumer.h
  )


add_executable(
  sniffer
  ${sniffer_src}
  ${sniffer_inc}
)

target_link_libraries(
  sniffer
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt5::Widgets
)

target_link_libraries(sniffer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(sniffer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
