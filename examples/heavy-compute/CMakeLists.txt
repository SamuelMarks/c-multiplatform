cmake_minimum_required(VERSION 3.14)
project(cmp_example_heavy_compute C)

add_executable(cmp_example_heavy_compute main.c)
target_link_libraries(cmp_example_heavy_compute PRIVATE cmp)
