set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
#  ${dir}/time_base.cpp
#  ${dir}/time_stamp.cpp
#  ${dir}/event_model.cpp
#  ${dir}/event.cpp
#  ${dir}/detector.cpp
#  ${dir}/spill.cpp
#  ${dir}/spill_deque.cpp
#  ${dir}/dataspace.cpp
#  ${dir}/consumer_metadata.cpp
#  ${dir}/consumer.cpp
#  ${dir}/consumer_factory.cpp
#  ${dir}/producer.cpp
#  ${dir}/producer_factory.cpp
#  ${dir}/project.cpp
#  ${dir}/engine.cpp
  )

add_subdirectory(util)
add_subdirectory(calibration)
#add_subdirectory(plugin)

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
