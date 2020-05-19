set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/FilterBlock.cpp
  ${dir}/periodic_trigger.cpp
  ${dir}/recent_rate.cpp
  ${dir}/status.cpp
  ${dir}/value_filter.cpp
  ${dir}/value_latch.cpp
  )

set(HEADERS
  ${dir}/FilterBlock.h
  ${dir}/periodic_trigger.h
  ${dir}/recent_rate.h
  ${dir}/status.h
  ${dir}/value_filter.h
  ${dir}/value_latch.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
