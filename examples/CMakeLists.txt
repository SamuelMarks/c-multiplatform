cmake_minimum_required(VERSION 3.14)
project(cmp_examples C)

set(CMAKE_C_STANDARD 90)

# Example global options for the backend (mapped down to CMP)
option(CMP_BACKEND_WIN32 "Use modern Win32/DXGI backend" ON)
option(CMP_BACKEND_LEGACY_WIN32 "Use Legacy Win97/GDI backend" OFF)
option(C_ABSTRACT_HTTP_USE_WINHTTP "Use WinHTTP backend for abstract-http" ON)
option(C_ABSTRACT_HTTP_USE_LIBCURL "Use libcurl backend for abstract-http" OFF)
option(C_ORM_USE_SQLITE "Use SQLite backend for c-orm" ON)

include(FetchContent)
if(NOT TARGET cmp)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../include/cmp.h")
        FetchContent_Declare(cmp SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
    else()
        FetchContent_Declare(cmp GIT_REPOSITORY https://github.com/SamuelMarks/c-multiplatform.git GIT_TAG master)
    endif()
    FetchContent_MakeAvailable(cmp)
endif()

add_subdirectory(kotlin-multiplatform)
add_subdirectory(google-material-android)
add_subdirectory(apple-cupertino-ios)
add_subdirectory(microsoft-fluent-2)
add_subdirectory(theme_switcher_demo)

add_subdirectory(compile_enum_demo)
