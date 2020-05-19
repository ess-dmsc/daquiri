set(this_target ${PROJECT_NAME}_consumers)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Eigen3 REQUIRED)

set(SOURCES
  ${dir}/consumers_autoreg.cpp
  ${dir}/histogram_1d.cpp
  ${dir}/histogram_2d.cpp
  ${dir}/histogram_3d.cpp
  ${dir}/Image2D.cpp
  ${dir}/Prebinned1D.cpp
  ${dir}/Spectrum.cpp
  ${dir}/TimeSpectrum.cpp
  ${dir}/StatsScalar.cpp
  ${dir}/TimeDelta1D.cpp
  ${dir}/TimeDomain.cpp
  ${dir}/TOF1D.cpp
  ${dir}/TOF1DCorrelate.cpp
  ${dir}/TOFVal2D.cpp
  ${dir}/TOFVal2DCorrelate.cpp
  )

set(HEADERS
  ${dir}/consumers_autoreg.h
  ${dir}/histogram_1d.h
  ${dir}/histogram_2d.h
  ${dir}/histogram_3d.h
  ${dir}/Image2D.h
  ${dir}/Prebinned1D.h
  ${dir}/Spectrum.h
  ${dir}/TimeSpectrum.h
  ${dir}/StatsScalar.h
  ${dir}/TimeDelta1D.h
  ${dir}/TimeDomain.h
  ${dir}/TOF1D.h
  ${dir}/TOF1DCorrelate.h
  ${dir}/TOFVal2D.h
  ${dir}/TOFVal2DCorrelate.h
  )

set(${this_target}_headers ${HEADERS})
set(${this_target}_sources ${SOURCES})
add_subdirectory(add_ons)
add_subdirectory(dataspaces)

add_library(
  ${this_target} STATIC
  ${${this_target}_headers}
  ${${this_target}_sources}
)

target_include_directories(
  ${this_target}
  PUBLIC ${PROJECT_SOURCE_DIR}/source
  PRIVATE ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(
  ${this_target}
  PUBLIC ${PROJECT_NAME}_core
)

enable_coverage(${this_target})
