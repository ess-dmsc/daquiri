set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Histogram1DTest.cpp
  ${dir}/Histogram2DTest.cpp
  ${dir}/Histogram3DTest.cpp
  ${dir}/Image2DTest.cpp
  ${dir}/Prebinned1DTest.cpp
  ${dir}/SpectrumTest.cpp
  ${dir}/TimeSpectrumTest.cpp
  ${dir}/StatsScalarTest.cpp
  ${dir}/TimeDelta1DTest.cpp
  ${dir}/TimeDomainTest.cpp
  ${dir}/TOF1DTest.cpp
  ${dir}/TOF1DCorrelateTest.cpp
  ${dir}/TOFVal2DTest.cpp
  ${dir}/TOFVal2DCorrelateTest.cpp
  )

add_subdirectory(dataspaces)
add_subdirectory(add_ons)

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
