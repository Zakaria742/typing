cmake_minimum_required( VERSION 3.21 )
project(typing)

set(SOURCES src/main.c)
set(HEADERS libs/utils.h libs/info.h)
add_executable(typing ${SOURCES} ${HEADERS})
target_compile_options(typing PRIVATE -g )
target_link_libraries(typing PRIVATE m)
