cmake_minimum_required(VERSION 3.16)

# Force C99 globally
set(CMAKE_C_STANDARD 99)
set(CMAKE_OBJC_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(CMPDemo VERSION 0.0.1 LANGUAGES C CXX)

# --- Path Detection ---
# Assume library is reachable at ../c_multiplatform or nested
get_filename_component(SEA_PATH_SIBLING "${CMAKE_CURRENT_SOURCE_DIR}/.." REALPATH)

if(EXISTS "${SEA_PATH_SIBLING}/include/cmpc/cmp_core.h")
    set(PARENT_LIB_PATH "${SEA_PATH_SIBLING}")
else()
    message(FATAL_ERROR "Could not locate LibCMPC. Expected at: ${SEA_PATH_SIBLING}")
endif()

# --- Backends ---
include(FetchContent)
FetchContent_Declare(cmpc SOURCE_DIR "${PARENT_LIB_PATH}")

option(DEMO_BACKEND_SDL3 "Build SDL3" OFF)
option(DEMO_BACKEND_NATIVE "Build Native" ON)

# Configure Backend Flags based on target platform
if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(CMP_ENABLE_IOS ON CACHE BOOL "" FORCE)
elseif(APPLE)
    set(CMP_ENABLE_COCOA ON CACHE BOOL "" FORCE)
elseif(WIN32)
    set(CMP_ENABLE_WIN32 ON CACHE BOOL "" FORCE)
elseif(EMSCRIPTEN)
    set(CMP_ENABLE_WEB ON CACHE BOOL "" FORCE)
elseif(UNIX AND NOT ANDROID)
    set(CMP_ENABLE_GTK4 ON CACHE BOOL "" FORCE)
endif()

if(DEMO_BACKEND_SDL3)
    set(CMP_ENABLE_SDL3 ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(cmpc)

# --- Demo Logic ---
add_library(demo_logic STATIC src/app/demo_app.c)
target_include_directories(demo_logic PUBLIC src) # Allows <app/demo_app.h>
target_link_libraries(demo_logic PUBLIC m3::m3 cmp::cmpc)

# --- Platform Targets ---

if(DEMO_BACKEND_SDL3)
    add_executable(cmp_demo_sdl3 src/main/main_sdl3.c)
    target_link_libraries(cmp_demo_sdl3 PRIVATE demo_logic)
    if(TARGET SDL3::SDL3)
        target_link_libraries(cmp_demo_sdl3 PRIVATE SDL3::SDL3)
    endif()
endif()

if(WIN32 AND DEMO_BACKEND_NATIVE)
    add_executable(cmp_demo_win32 WIN32 src/main/main_win32.c)
    target_link_libraries(cmp_demo_win32 PRIVATE demo_logic)
endif()

if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND DEMO_BACKEND_NATIVE) # macOS
    enable_language(OBJC)
    add_executable(cmp_demo_cocoa MACOSX_BUNDLE src/main/main_cocoa.m)
    target_link_libraries(cmp_demo_cocoa PRIVATE demo_logic "-framework Cocoa" "-framework QuartzCore" "-framework AVFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CFNetwork")
    set_target_properties(cmp_demo_cocoa PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "CMPDemo")
endif()

if(IOS AND DEMO_BACKEND_NATIVE)
    enable_language(OBJC)
    add_executable(cmp_demo_ios MACOSX_BUNDLE src/main/main_ios.m)
    target_link_libraries(cmp_demo_ios PRIVATE demo_logic "-framework UIKit" "-framework QuartzCore" "-framework AVFoundation" "-framework CoreMedia" "-framework CoreVideo" "-framework CFNetwork")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN AND DEMO_BACKEND_NATIVE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 REQUIRED gtk4)
    add_executable(cmp_demo_gtk4 src/main/main_linux.c)
    target_link_libraries(cmp_demo_gtk4 PRIVATE demo_logic ${GTK4_LIBRARIES})
    target_include_directories(cmp_demo_gtk4 PRIVATE ${GTK4_INCLUDE_DIRS})
    target_compile_options(cmp_demo_gtk4 PRIVATE ${GTK4_CFLAGS_OTHER})
endif()

if(EMSCRIPTEN)
    add_executable(cmp_demo_web src/main/main_web.c)
    target_link_libraries(cmp_demo_web PRIVATE demo_logic)
    target_link_options(cmp_demo_web PRIVATE "-sUSE_WEBGL2=1" "-sALLOW_MEMORY_GROWTH=1" "-sNO_EXIT_RUNTIME=1")
endif()

if(ANDROID AND DEMO_BACKEND_NATIVE)
    add_library(cmp_demo_android SHARED src/main/main_android.c)
    target_link_libraries(cmp_demo_android PRIVATE demo_logic android log)
endif()

# --- Tests ---
enable_testing()
add_executable(test_app_logic tests/test_app_logic.c)
target_link_libraries(test_app_logic PRIVATE demo_logic)
add_test(NAME app_logic_test COMMAND test_app_logic)