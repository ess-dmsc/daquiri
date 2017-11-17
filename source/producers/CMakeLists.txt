set(this_target ${PROJECT_NAME}_producers)

set(autoreg_source ${PROJECT_BINARY_DIR}/producers_autoreg.cpp)

list(APPEND ${this_target}_HEADERS
  ${CMAKE_CURRENT_LIST_DIR}/producers_autoreg.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target} STATIC
  ${autoreg_source}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PUBLIC ${${this_target}_INCLUDE_DIRS}
)

foreach(prod ${DAQuiri_enabled_producers})
  add_subdirectory(${prod})
  target_include_directories(${this_target} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/${prod})
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
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${DAQuiri_enabled_producers}
)
