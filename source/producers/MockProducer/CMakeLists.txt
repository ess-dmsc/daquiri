set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/MockProducer.cpp
  ${dir}/ValueDefinition.cpp
  )

set(HEADERS
  ${dir}/MockProducer.h
  ${dir}/ValueDefinition.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_headers ${${this_target}_headers} ${SOURCES} PARENT_SCOPE)
