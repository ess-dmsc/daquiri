set(dir ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
  ${dir}/BinaryWidget.cpp
  ${dir}/PatternWidget.cpp
  ${dir}/QColorExtensions.cpp
  ${dir}/QFileExtensions.cpp
  ${dir}/qt_util.cpp
  ${dir}/QTimeExtensions.cpp
  ${dir}/SelectorWidget.cpp
  ${dir}/SettingDelegate.cpp
  ${dir}/TimeDurationWidget.cpp
  )

set(UI
  ${dir}/TimeDurationWidget.ui
  )

set(HEADERS
  ${dir}/BinaryWidget.h
  ${dir}/PatternWidget.h
  ${dir}/QColorExtensions.h
  ${dir}/QFileExtensions.h
  ${dir}/qt_util.h
  ${dir}/QTimeExtensions.h
  ${dir}/SelectorWidget.h
  ${dir}/SettingDelegate.h
  ${dir}/TabCloseButton.h
  ${dir}/TimeDurationWidget.h
  )

set(${this_target}_headers ${${this_target}_headers} ${HEADERS} PARENT_SCOPE)
set(${this_target}_sources ${${this_target}_sources} ${SOURCES} PARENT_SCOPE)
set(${this_target}_ui ${${this_target}_ui} ${UI} PARENT_SCOPE)

