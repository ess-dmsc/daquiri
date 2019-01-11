set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/parameter.cpp
  ${dir}/coef_function.cpp
  ${dir}/coef_function_factory.cpp
  ${dir}/polynomial.cpp
  ${dir}/calibration.cpp
  )

set(HEADERS
  ${dir}/parameter.h
  ${dir}/coef_function.h
  ${dir}/coef_function_factory.h
  ${dir}/polynomial.h
  ${dir}/calibration.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
