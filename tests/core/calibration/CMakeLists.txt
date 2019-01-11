set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/parameter.cpp
  ${dir}/coef_function.cpp
  ${dir}/coef_function_factory.cpp
  ${dir}/polynomial.cpp
  ${dir}/calibration.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
