set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/plugin.cpp
  ${dir}/setting.cpp
  ${dir}/setting_metadata.cpp
  )

set(HEADERS
  ${dir}/container.h
  ${dir}/pattern.h
  ${dir}/plugin.h
  ${dir}/precise_float.h
  ${dir}/setting.h
  ${dir}/setting_metadata.h
  )

set(daquiri_headers ${daquiri_headers} ${HEADERS} PARENT_SCOPE)
set(daquiri_sources ${daquiri_sources} ${SOURCES} PARENT_SCOPE)
