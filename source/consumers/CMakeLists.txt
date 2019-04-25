set(this_target ${PROJECT_NAME}_consumers)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Eigen3 REQUIRED)

set(SOURCES
  ${dir}/consumers_autoreg.cpp
  ${dir}/histogram_1d.cpp
  ${dir}/histogram_2d.cpp
  ${dir}/histogram_3d.cpp
  ${dir}/image_2d.cpp
  ${dir}/prebinned_1d.cpp
  ${dir}/spectrum.cpp
  ${dir}/spectrum_time.cpp
  ${dir}/stats_scalar.cpp
  ${dir}/time_delta_1d.cpp
  ${dir}/time_domain.cpp
  ${dir}/tof_1d.cpp
  ${dir}/tof_1d_correlate.cpp
  ${dir}/tof_val_2d.cpp
  ${dir}/tof_val_2d_correlate.cpp
  )

set(HEADERS
  ${dir}/consumers_autoreg.h
  ${dir}/histogram_1d.h
  ${dir}/histogram_2d.h
  ${dir}/histogram_3d.h
  ${dir}/image_2d.h
  ${dir}/prebinned_1d.h
  ${dir}/spectrum.h
  ${dir}/spectrum_time.h
  ${dir}/stats_scalar.h
  ${dir}/time_delta_1d.h
  ${dir}/time_domain.h
  ${dir}/tof_1d.h
  ${dir}/tof_1d_correlate.h
  ${dir}/tof_val_2d.h
  ${dir}/tof_val_2d_correlate.h
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

