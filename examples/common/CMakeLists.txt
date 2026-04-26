cmake_minimum_required(VERSION 3.14)
project(cmp_example_common C)

add_library(cmp_example_common STATIC
    src/cmp_example_app.c
)

target_include_directories(cmp_example_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(cmp_example_common PUBLIC cmp)
