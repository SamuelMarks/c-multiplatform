cmake_minimum_required(VERSION 3.16)

# Force C99 globally
set(CMAKE_C_STANDARD 99)
set(CMAKE_OBJC_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(CMPDemo VERSION 0.0.1 LANGUAGES C CXX)

get_filename_component(SEA_PATH_SIBLING "${CMAKE_CURRENT_SOURCE_DIR}/.." REALPATH)

if(EXISTS "${SEA_PATH_SIBLING}/include/cmpc/cmp_core.h")
    set(PARENT_LIB_PATH "${SEA_PATH_SIBLING}")
else()
    message(FATAL_ERROR "Could not locate LibCMPC. Expected at: ${SEA_PATH_SIBLING}")
endif()

# --- Dependencies and Backend Selection ---

option(DEMO_BACKEND_SDL3 "Build SDL3" OFF)
option(DEMO_BACKEND_NATIVE "Build Native" ON)

# 1. Detect Linux/GTK4 environment
if(UNIX AND NOT APPLE AND DEMO_BACKEND_NATIVE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 QUIET IMPORTED_TARGET gtk4)
    if(GTK4_FOUND)
        message(STATUS "GTK4 found: Enabling CMP_ENABLE_GTK4")
        set(CMP_ENABLE_GTK4 ON CACHE BOOL "" FORCE)
    else()
        message(WARNING "GTK4 not found. Install libgtk-4-dev to build the Linux demo.")
    endif()
endif()

# 2. Detect Windows environment
if(WIN32 AND DEMO_BACKEND_NATIVE)
    set(CMP_ENABLE_WIN32 ON CACHE BOOL "" FORCE)
endif()

# 3. Detect SDL3 environment
if(DEMO_BACKEND_SDL3)
    set(CMP_ENABLE_SDL3 ON CACHE BOOL "" FORCE)
    # SDL3_ttf is widely used in the SDL backend for text
    set(CMP_ENABLE_SDL3_TTF ON CACHE BOOL "" FORCE) 
endif()

include(FetchContent)
FetchContent_Declare(cmpc SOURCE_DIR "${PARENT_LIB_PATH}")

# --- Test/Build toggles for Core ---
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(CMP_ENABLE_COVERAGE OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(cmpc)

# --- Shared Demo Logic ---
add_library(demo_logic STATIC src/app/demo_app.c)
target_include_directories(demo_logic PUBLIC src)
target_link_libraries(demo_logic PUBLIC m3::m3 cmp::cmpc)

# --- Executable Definitions ---

# Target: Linux (GTK4)
if(GTK4_FOUND AND DEMO_BACKEND_NATIVE)
    add_executable(cmp_demo_gtk4 src/main/main_linux.c)
    target_link_libraries(cmp_demo_gtk4 PRIVATE demo_logic PkgConfig::GTK4)
    # Ensure math library is linked on Linux
    target_link_libraries(cmp_demo_gtk4 PRIVATE m) 
endif()

# Target: Windows (Win32)
if(WIN32 AND DEMO_BACKEND_NATIVE)
    add_executable(cmp_demo_win32 WIN32 src/main/main_win32.c)
    target_link_libraries(cmp_demo_win32 PRIVATE demo_logic)
endif()

# Target: SDL3
if(DEMO_BACKEND_SDL3)
    find_package(SDL3 CONFIG REQUIRED)
    add_executable(cmp_demo_sdl3 src/main/main_sdl3.c)
    target_link_libraries(cmp_demo_sdl3 PRIVATE demo_logic SDL3::SDL3)
endif()