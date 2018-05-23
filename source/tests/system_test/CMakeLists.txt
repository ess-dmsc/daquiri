set(this_target system_test)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

add_executable(
  ${this_target} EXCLUDE_FROM_ALL
  main.cpp
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${PROJECT_NAME}_producers
  PRIVATE ${PROJECT_NAME}_consumers
  PRIVATE ${Boost_LIBRARIES}
)

set_target_properties(${this_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_custom_target(run_system_test
    COMMAND ${this_target}
    DEPENDS ${this_target})
add_test(NAME "RunSystemTest" COMMAND run_system_test)
