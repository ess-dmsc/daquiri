set(this_target systest)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

add_executable(
  ${this_target}
  main.cpp
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${PROJECT_NAME}_producers
  PRIVATE ${PROJECT_NAME}_consumers
  PRIVATE ${Boost_LIBRARIES}
)

