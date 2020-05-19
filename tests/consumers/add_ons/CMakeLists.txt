set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/FilterBlockTest.cpp
  ${dir}/periodic_trigger.cpp
  ${dir}/recent_rate.cpp
  ${dir}/status.cpp
  ${dir}/ValueFilterTest.cpp
  ${dir}/ValueLatchTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
