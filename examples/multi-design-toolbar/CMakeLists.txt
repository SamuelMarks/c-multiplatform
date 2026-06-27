cmake_minimum_required(VERSION 3.14)
project(cmp_example_multi_design_toolbar C)

set(CMAKE_C_STANDARD 90)

option(CMP_BACKEND_WIN32 "Use modern Win32/DXGI backend" ON)
option(CMP_BACKEND_LEGACY_WIN32 "Use Legacy Win97/GDI backend" OFF)
option(C_ABSTRACT_HTTP_USE_WINHTTP "Use WinHTTP backend for abstract-http" ON)
option(C_ABSTRACT_HTTP_USE_LIBCURL "Use libcurl backend for abstract-http" OFF)
option(C_ORM_USE_SQLITE "Use SQLite backend for c-orm" ON)

include(FetchContent)
if(NOT TARGET cmp)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/cmp.h")
        FetchContent_Declare(cmp SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
    else()
        FetchContent_Declare(cmp URL https://github.com/SamuelMarks/c-multiplatform/archive/master.zip )
    endif()
    FetchContent_MakeAvailable(cmp)
endif()

add_executable(cmp_example_multi_design_toolbar main.c app.c)
target_link_libraries(cmp_example_multi_design_toolbar PRIVATE cmp)
target_include_directories(cmp_example_multi_design_toolbar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
