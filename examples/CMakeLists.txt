cmake_minimum_required(VERSION 3.16)
project(CMPExamples VERSION 0.0.1 LANGUAGES C)

get_filename_component(SEA_PATH_SIBLING "${CMAKE_CURRENT_SOURCE_DIR}/.." REALPATH)
if(EXISTS "${SEA_PATH_SIBLING}/include/cmpc/cmp_core.h")
    set(PARENT_LIB_PATH "${SEA_PATH_SIBLING}")
else()
    message(FATAL_ERROR "Could not locate LibCMPC. Expected at: ${SEA_PATH_SIBLING}")
endif()

include(FetchContent)
FetchContent_Declare(cmpc SOURCE_DIR "${PARENT_LIB_PATH}")
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cmpc)

add_executable(node_js_clone node_js_clone.c)
target_link_libraries(node_js_clone PRIVATE cmp::cmpc)

add_executable(heavy_compute heavy_compute.c)
target_link_libraries(heavy_compute PRIVATE cmp::cmpc)

add_executable(resilient_actors resilient_actors.c)
target_link_libraries(resilient_actors PRIVATE cmp::cmpc)