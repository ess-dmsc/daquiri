set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/plugin.cpp
  ${dir}/Setting.cpp
  ${dir}/setting_metadata.cpp
  )

set(HEADERS
  ${dir}/container.h
  ${dir}/pattern.h
  ${dir}/plugin.h
  ${dir}/precise_float.h
  ${dir}/Setting.h
  ${dir}/setting_metadata.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
