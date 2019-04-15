set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
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

add_subdirectory(dataspaces)
add_subdirectory(add_ons)

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
