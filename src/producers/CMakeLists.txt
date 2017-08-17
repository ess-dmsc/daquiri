cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(producers CXX)

set(autoreg_source ${PROJECT_BINARY_DIR}/producers_autoreg.cpp)

set(${PROJECT_NAME}_headers producers_autoreg.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_headers}")

add_library(
  ${PROJECT_NAME}
  ${autoreg_source}
  ${${PROJECT_NAME}_headers}
)

include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
  PRIVATE ${core_INCLUDE_DIRS}
)

foreach(prod ${DAQuiri_enabled_producers})
  add_subdirectory(${prod})
  include_directories(${PROJECT_NAME} PUBLIC ${${prod}_INCLUDE_DIRS})
  set(prod_reg_includes "${prod_reg_includes} #include \"${prod}.h\"\n")
  set(prod_reg_cmds "${prod_reg_cmds}  DAQUIRI_REGISTER_PRODUCER(${prod})\n")
endforeach(prod)

file(WRITE ${autoreg_source} "#include \"producers_autoreg.h\"\n")
file(APPEND ${autoreg_source} "#include \"producer_factory.h\"\n\n")
file(APPEND ${autoreg_source} ${prod_reg_includes})
file(APPEND ${autoreg_source} "\nvoid producers_autoreg()\n{\n")
file(APPEND ${autoreg_source} ${prod_reg_cmds})
file(APPEND ${autoreg_source} "}\n")

target_link_libraries(
  ${PROJECT_NAME}
  ${core_LIBRARIES}
  ${DAQuiri_enabled_producers}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${${PROJECT_NAME}_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)
