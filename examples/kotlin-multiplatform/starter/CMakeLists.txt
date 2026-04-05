cmake_minimum_required(VERSION 3.14)
project(cmp_example_kmp_starter C)

set(CMAKE_C_STANDARD 90)

# Options specifying backend, c-abstract-http flags, and c-orm flags
option(CMP_BACKEND_WIN32 "Use modern Win32/DXGI backend" ON)
option(CMP_BACKEND_LEGACY_WIN32 "Use Legacy Win97/GDI backend" OFF)
option(C_ABSTRACT_HTTP_USE_WINHTTP "Use WinHTTP backend for abstract-http" ON)
option(C_ABSTRACT_HTTP_USE_LIBCURL "Use libcurl backend for abstract-http" OFF)
option(C_ORM_USE_SQLITE "Use SQLite backend for c-orm" ON)

include(FetchContent)
if(NOT TARGET cmp)
    set(CMP_BUILD_EXAMPLES OFF CACHE BOOL "Disable transitive example building" FORCE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/cmp.h")
        FetchContent_Declare(cmp SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../")
    else()
        FetchContent_Declare(cmp GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git
    GIT_TAG master UPDATE_COMMAND git fetch origin master COMMAND git reset --hard origin/master)
    endif()
    FetchContent_MakeAvailable(cmp)
endif()

add_executable(cmp_example_kmp_starter main.c app.c)
target_link_libraries(cmp_example_kmp_starter PRIVATE cmp)