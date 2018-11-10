set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/uncertain.cpp
  ${dir}/parameter.cpp
  ${dir}/coef_function.cpp
  ${dir}/calibration.cpp
  ${dir}/udouble.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
