set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/FilterBlock.cpp
  ${dir}/PeriodicTrigger.cpp
  ${dir}/RecentRate.cpp
  ${dir}/Status.cpp
  ${dir}/ValueFilter.cpp
  ${dir}/ValueLatch.cpp
  )

set(HEADERS
  ${dir}/FilterBlock.h
  ${dir}/PeriodicTrigger.cpp
  ${dir}/RecentRate.h
  ${dir}/Status.h
  ${dir}/ValueFilter.h
  ${dir}/ValueLatch.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
