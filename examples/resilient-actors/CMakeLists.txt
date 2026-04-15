cmake_minimum_required(VERSION 3.14)
project(cmp_example_resilient_actors C)

add_executable(cmp_example_resilient_actors main.c)
target_link_libraries(cmp_example_resilient_actors PRIVATE cmp)
