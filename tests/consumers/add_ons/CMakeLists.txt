set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/FilterBlockTest.cpp
  ${dir}/PeriodicTriggerTest.cpp
  ${dir}/RecentRateTest.cpp
  ${dir}/StatusTest.cpp
  ${dir}/ValueFilterTest.cpp
  ${dir}/ValueLatchTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
