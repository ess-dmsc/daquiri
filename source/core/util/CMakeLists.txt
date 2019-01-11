set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/custom_logger.cpp
  ${dir}/h5json.cpp
  ${dir}/timer.cpp
  ${dir}/time_extensions.cpp
  )

set(HEADERS
  ${dir}/ascii_tree.h
  ${dir}/bin_hex_print.h
  ${dir}/color_bash.h
  ${dir}/compare.h
  ${dir}/custom_logger.h
  ${dir}/h5json.h
  ${dir}/json_file.h
  ${dir}/lexical_extensions.h
  ${dir}/print_exception.h
  ${dir}/string_extensions.h
  ${dir}/sync_queue.h
  ${dir}/timer.h
  ${dir}/time_extensions.h
  ${dir}/unique_mangle.h
  ${dir}/UTF_extensions.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
