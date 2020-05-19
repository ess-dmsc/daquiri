set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/dense1d.cpp
  ${dir}/scalar.cpp
  ${dir}/sparse_map2d.cpp
  ${dir}/sparse_map3d.cpp
  ${dir}/SparseMatrix2DTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
