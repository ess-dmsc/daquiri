set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/ContainerTest.cpp
  ${dir}/PatternTest.cpp
  ${dir}/SettingMetaTest.cpp
  ${dir}/SettingTest.cpp
  ${dir}/PluginTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
