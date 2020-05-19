set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/dense1d.cpp
  ${dir}/dense_matrix2d.cpp
  ${dir}/scalar.cpp
  ${dir}/sparse_map2d.cpp
  ${dir}/SparseMap3D.cpp
  ${dir}/SparseMatrix2D.cpp
  )

set(HEADERS
  ${dir}/dense1d.h
  ${dir}/dense_matrix2d.h
  ${dir}/scalar.h
  ${dir}/sparse_map2d.h
  ${dir}/SparseMap3D.h
  ${dir}/SparseMatrix2D.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
