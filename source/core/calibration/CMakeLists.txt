set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Parameter.cpp
  ${dir}/CoefFunction.cpp
  ${dir}/CoefFunctionFactory.cpp
  ${dir}/Polynomial.cpp
  ${dir}/Calibration.cpp
  )

set(HEADERS
  ${dir}/Parameter.h
  ${dir}/CoefFunction.h
  ${dir}/CoefFunctionFactory.h
  ${dir}/Polynomial.h
  ${dir}/Calibration.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
