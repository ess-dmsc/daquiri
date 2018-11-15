set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/bin_hex_print.cpp
  ${dir}/h5json.cpp
  ${dir}/json_file.cpp
  ${dir}/lexical_extensions.cpp
  ${dir}/timer.cpp
  ${dir}/string_extensions.cpp
  ${dir}/time_extensions.cpp
  ${dir}/UTF_extensions.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
