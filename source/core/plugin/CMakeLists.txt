set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Plugin.cpp
  ${dir}/Setting.cpp
  ${dir}/setting_metadata.cpp
  )

set(HEADERS
  ${dir}/Container.h
  ${dir}/Pattern.h
  ${dir}/Plugin.h
  ${dir}/PreciseFloat.h
  ${dir}/Setting.h
  ${dir}/SettingMeta.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
