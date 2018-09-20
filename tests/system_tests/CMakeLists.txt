set(this_target system_test)

set(dir ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
    ${this_target} EXCLUDE_FROM_ALL
    ${dir}/main.cpp
)

target_link_libraries(
    ${this_target}
    PRIVATE ${PROJECT_NAME}_core
    PRIVATE ${PROJECT_NAME}_producers
    PRIVATE ${PROJECT_NAME}_consumers
)

set_target_properties(${this_target} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

add_custom_target(run_${this_target}
    COMMAND ${this_target}
    DEPENDS ${this_target})

add_test(NAME "RunSystemTest" COMMAND run_${this_target})
