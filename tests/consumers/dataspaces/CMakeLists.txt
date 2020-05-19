set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Dense1DTest.cpp
  ${dir}/ScalarTest.cpp
  ${dir}/SparseMap2DTest.cpp
  ${dir}/SparseMap3DTest.cpp
  ${dir}/SparseMatrix2DTest.cpp
  )

set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
