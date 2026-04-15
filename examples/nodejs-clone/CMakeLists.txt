cmake_minimum_required(VERSION 3.14)
project(cmp_example_nodejs_clone C)

add_executable(cmp_example_nodejs_clone main.c)
target_link_libraries(cmp_example_nodejs_clone PRIVATE cmp)
