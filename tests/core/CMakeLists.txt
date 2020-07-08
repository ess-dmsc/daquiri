set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/TimeBaseTest.cpp
  ${dir}/TimeStampTest.cpp
  ${dir}/EventModelTest.cpp
  ${dir}/EventTest.cpp
  ${dir}/DetectorTest.cpp
  ${dir}/SpillTest.cpp
  ${dir}/SpillDequeTest.cpp
  ${dir}/DataspaceTest.cpp
  ${dir}/ConsumerMetadataTest.cpp
  ${dir}/ConsumerTest.cpp
  ${dir}/ConsumerFactoryTest.cpp
  ${dir}/ProducerTest.cpp
  ${dir}/ProducerFactoryTest.cpp
  ${dir}/ProjectTest.cpp
  ${dir}/EngineTest.cpp
  )

add_subdirectory(util)
add_subdirectory(calibration)
add_subdirectory(plugin)

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
