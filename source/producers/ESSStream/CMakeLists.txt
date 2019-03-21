set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/ESSGeometryPlugin.cpp
  ${dir}/ESSStream.cpp
  ${dir}/ev42_parser.cpp
  ${dir}/f142_parser.cpp
  ${dir}/fb_parser.cpp
  ${dir}/KafkaPlugin.cpp
  ${dir}/mo01_parser.cpp
  ${dir}/senv_data_parser.cpp
  ${dir}/senv_data_parser_wrong.cpp
  )

set(HEADERS
  ${dir}/ESSGeometryPlugin.h
  ${dir}/ESSStream.h
  ${dir}/ev42_parser.h
  ${dir}/f142_parser.h
  ${dir}/fb_parser.h
  ${dir}/KafkaPlugin.h
  ${dir}/mo01_parser.h
  ${dir}/senv_data_parser.h
  ${dir}/senv_data_wrong.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
