set(this_target gui_tests)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

include(QtLibraryConfig)

add_executable(
  ${this_target} EXCLUDE_FROM_ALL
  QTimeExtensions.cpp
  ../../source/gui/widgets/QTimeExtensions.cpp
)

target_include_directories(
  ${this_target}
  PRIVATE ${dir}/../../source/gui
)

target_link_libraries(
  ${this_target}
  PRIVATE Qt5::Test
  PRIVATE ${PROJECT_NAME}_core
)

add_custom_target(run_${this_target}
  COMMAND ${this_target}
  DEPENDS ${this_target})

add_test(NAME "RunGuiTests" COMMAND run_${this_target})
