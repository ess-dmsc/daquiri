set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/Consumer1D.cpp
  ${dir}/Consumer2D.cpp
  ${dir}/ConsumerDialog.cpp
  ${dir}/ConsumerScalar.cpp
  ${dir}/ConsumerTemplatesForm.cpp
  ${dir}/ListModeForm.cpp
  ${dir}/ProjectForm.cpp
  ${dir}/ProjectView.cpp
  )

set(UI
  ${dir}/ConsumerDialog.ui
  ${dir}/ConsumerTemplatesForm.ui
  ${dir}/ListModeForm.ui
  ${dir}/ProjectForm.ui
  ${dir}/ProjectView.ui
  )

set(HEADERS
  ${dir}/AbstractConsumerWidget.h
  ${dir}/Consumer1D.h
  ${dir}/Consumer2D.h
  ${dir}/ConsumerDialog.h
  ${dir}/ConsumerScalar.h
  ${dir}/ConsumerTemplatesForm.h
  ${dir}/ListModeForm.h
  ${dir}/ProjectForm.h
  ${dir}/ProjectView.h
  ${dir}/ThreadPlotSignal.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
set(${this_target}_ui ${${this_target}_ui} ${UI} PARENT_SCOPE)
