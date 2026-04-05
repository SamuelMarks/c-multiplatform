cmake_minimum_required(VERSION 3.14)
project(cmp_example_apple_cupertino_ios C)

set(CMAKE_C_STANDARD 90)

option(CMP_BACKEND_WIN32 "Use modern Win32/DXGI backend" ON)
option(CMP_BACKEND_LEGACY_WIN32 "Use Legacy Win97/GDI backend" OFF)
option(C_ABSTRACT_HTTP_USE_WINHTTP "Use WinHTTP backend for abstract-http" ON)
option(C_ABSTRACT_HTTP_USE_LIBCURL "Use libcurl backend for abstract-http" OFF)
option(C_ORM_USE_SQLITE "Use SQLite backend for c-orm" ON)

include(FetchContent)
if(NOT TARGET cmp)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../include/cmp.h")
        FetchContent_Declare(cmp SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../")
    else()
        FetchContent_Declare(cmp GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git
    GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    endif()
    FetchContent_MakeAvailable(cmp)
endif()

add_custom_target(apple_cupertino_ios_example
    COMMAND ${CMAKE_COMMAND} -E echo "Apple Cupertino iOS Example Stub"
)