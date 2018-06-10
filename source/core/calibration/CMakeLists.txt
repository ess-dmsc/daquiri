set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/calibration.cpp
  ${dir}/coef_function.cpp
  ${dir}/parameter.cpp
  ${dir}/uncertain.cpp
  )

set(HEADERS
  ${dir}/calibration.h
  ${dir}/coef_function.h
  ${dir}/parameter.h
  ${dir}/uncertain.h
  )

set(daquiri_headers ${daquiri_headers} ${HEADERS} PARENT_SCOPE)
set(daquiri_sources ${daquiri_sources} ${SOURCES} PARENT_SCOPE)
