cmake_minimum_required(VERSION 3.14)
project(cmp_example_multi_design_buttons C)

set(CMAKE_C_STANDARD 90)

if(NOT TARGET cmp)
    include(FetchContent)
    FetchContent_Declare(cmp
        URL https://github.com/SamuelMarks/c-multiplatform/archive/master.zip
    )
    FetchContent_MakeAvailable(cmp)
endif()

if(NOT TARGET cmp_example_common)
    add_subdirectory(../common common)
endif()

add_executable(cmp_example_multi_design_buttons main.c)
target_link_libraries(cmp_example_multi_design_buttons PRIVATE cmp_example_common cmp)
