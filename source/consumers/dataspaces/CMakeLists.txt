set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/dense1d.cpp
  ${dir}/dense_matrix2d.cpp
  ${dir}/scalar.cpp
  ${dir}/sparse_map2d.cpp
  ${dir}/sparse_map3d.cpp
  ${dir}/sparse_matrix2d.cpp
  )

set(HEADERS
  ${dir}/dense1d.h
  ${dir}/dense_matrix2d.h
  ${dir}/scalar.h
  ${dir}/sparse_map2d.h
  ${dir}/sparse_map3d.h
  ${dir}/sparse_matrix2d.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_headers ${${this_target}_headers} ${SOURCES} PARENT_SCOPE)
