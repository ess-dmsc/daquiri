set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/calibration.cpp
  ${dir}/coef_function.cpp
  ${dir}/parameter.cpp
  ${dir}/uncertain.cpp
  ${dir}/udouble.cpp
  )

set(HEADERS
  ${dir}/calibration.h
  ${dir}/coef_function.h
  ${dir}/parameter.h
  ${dir}/uncertain.h
  ${dir}/udouble.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
