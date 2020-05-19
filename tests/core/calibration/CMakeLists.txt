set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/parameter.cpp
  ${dir}/coef_function.cpp
  ${dir}/CoefFunctionFactoryTest.cpp
  ${dir}/PolynomialTest.cpp
  ${dir}/CalibrationTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
