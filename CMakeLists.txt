cmake_minimum_required(VERSION 3.16)

project(LibM3C C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(M3_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(M3_ENABLE_COVERAGE "Enable coverage instrumentation (GCC/Clang only)" OFF)
option(M3_REQUIRE_DOXYGEN "Require Doxygen for documentation coverage" OFF)
option(M3_ENABLE_SDL3 "Enable SDL3 debug backend" OFF)
option(M3_ENABLE_SDL3_TTF "Enable SDL3_ttf text rendering for the SDL3 backend" OFF)
option(M3_ENABLE_GTK4 "Enable GTK4 backend" OFF)
option(M3_ENABLE_COCOA "Enable Cocoa backend" OFF)
option(M3_ENABLE_WIN32 "Enable Win32 backend" ON)
option(M3_ENABLE_WEB "Enable Web (Emscripten) backend" OFF)
option(M3_ENABLE_WEBGPU "Enable WebGPU renderer for the Web backend" OFF)
option(M3_ENABLE_IOS "Enable iOS backend" OFF)
option(M3_APPLE_USE_CFNETWORK_C "Use CFNetwork C APIs for Apple network backends" ON)
option(M3_ENABLE_LIBCURL "Enable libcurl-backed network support" ON)
if(ANDROID)
    set(M3_ENABLE_ANDROID_DEFAULT ON)
else()
    set(M3_ENABLE_ANDROID_DEFAULT OFF)
endif()
option(M3_ENABLE_ANDROID "Enable Android backend" ${M3_ENABLE_ANDROID_DEFAULT})

add_library(m3_headers INTERFACE)
target_include_directories(m3_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(M3_WARNINGS_C
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<C_COMPILER_ID:MSVC>:$<$<BOOL:${M3_WARNINGS_AS_ERRORS}>:/WX>>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wextra>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wpedantic>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:$<$<BOOL:${M3_WARNINGS_AS_ERRORS}>:-Werror>>
)

function(m3_apply_c_settings target_name)
    target_compile_options(${target_name} PRIVATE ${M3_WARNINGS_C})

    if(M3_ENABLE_COVERAGE)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} PRIVATE -O0 -g --coverage)
            target_link_options(${target_name} PRIVATE --coverage)
        else()
            message(FATAL_ERROR "M3_ENABLE_COVERAGE is only supported with GCC or Clang.")
        endif()
    endif()
endfunction()

add_library(m3 STATIC
    src/core/m3_core.c
    src/core/m3_object.c
    src/core/m3_arena.c
    src/core/m3_utf8.c
    src/core/m3_log.c
    src/core/m3_store.c
    src/core/m3_storage.c
    src/core/m3_math.c
    src/core/m3_color.c
    src/core/m3_layout.c
    src/core/m3_anim.c
    src/core/m3_router.c
    src/core/m3_render.c
    src/core/m3_event.c
    src/core/m3_tasks.c
    src/core/m3_visuals.c
    src/core/m3_text.c
    src/core/m3_text_field.c
    src/core/m3_list.c
    src/core/m3_navigation.c
    src/core/m3_dialogs.c
    src/core/m3_button.c
    src/core/m3_selection.c
    src/core/m3_progress.c
    src/core/m3_camera.c
    src/core/m3_network.c
    src/backend/null/m3_backend_null.c
    src/backend/sdl3/m3_backend_sdl3.c
    src/backend/linux/m3_backend_gtk4.c
    src/backend/cocoa/m3_backend_cocoa.c
    src/backend/win32/m3_backend_win32.c
    src/backend/web/m3_backend_web.c
    src/backend/android/m3_backend_android.c
)
target_link_libraries(m3 PUBLIC m3_headers)
target_include_directories(m3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if (NOT WIN32)
    target_link_libraries(m3 PUBLIC m)
endif()
if(M3_ENABLE_SDL3)
    find_package(SDL3 CONFIG QUIET)
    if(SDL3_FOUND)
        target_link_libraries(m3 PUBLIC SDL3::SDL3)
        target_compile_definitions(m3 PUBLIC M3_SDL3_AVAILABLE)
    else()
        message(WARNING "SDL3 not found: SDL3 backend will be stubbed.")
    endif()
endif()
if(M3_ENABLE_SDL3_TTF)
    if(NOT M3_ENABLE_SDL3)
        message(WARNING "M3_ENABLE_SDL3_TTF is ON but M3_ENABLE_SDL3 is OFF; SDL3_ttf support disabled.")
    else()
        find_package(SDL3_ttf CONFIG QUIET)
        if(TARGET SDL3_ttf::SDL3_ttf)
            target_link_libraries(m3 PUBLIC SDL3_ttf::SDL3_ttf)
            target_compile_definitions(m3 PUBLIC M3_SDL3_TTF_AVAILABLE)
        elseif(TARGET SDL3_ttf::SDL3_ttf-static)
            target_link_libraries(m3 PUBLIC SDL3_ttf::SDL3_ttf-static)
            target_compile_definitions(m3 PUBLIC M3_SDL3_TTF_AVAILABLE)
        else()
            message(WARNING "SDL3_ttf not found: SDL3 text rendering disabled.")
        endif()
    endif()
endif()
if(M3_ENABLE_GTK4)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GTK4 QUIET IMPORTED_TARGET gtk4)
        if(GTK4_FOUND)
            target_link_libraries(m3 PUBLIC PkgConfig::GTK4)
            target_compile_definitions(m3 PUBLIC M3_GTK4_AVAILABLE)
        else()
            message(WARNING "GTK4 not found: GTK4 backend will be stubbed.")
        endif()
    else()
        message(WARNING "pkg-config not found: GTK4 backend will be stubbed.")
    endif()
endif()
if(M3_ENABLE_LIBCURL)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        if(TARGET CURL::libcurl)
            target_link_libraries(m3 PUBLIC CURL::libcurl)
        else()
            if(CURL_INCLUDE_DIRS)
                target_include_directories(m3 PUBLIC ${CURL_INCLUDE_DIRS})
            endif()
            if(CURL_LIBRARIES)
                target_link_libraries(m3 PUBLIC ${CURL_LIBRARIES})
            endif()
        endif()
        target_compile_definitions(m3 PUBLIC M3_LIBCURL_AVAILABLE)
    else()
        message(WARNING "libcurl not found: libcurl-backed network support disabled.")
    endif()
endif()
if(M3_ENABLE_COCOA)
    if(APPLE)
        include(CheckLanguage)
        check_language(OBJC)
        if(CMAKE_OBJC_COMPILER)
            enable_language(OBJC)
            set(CMAKE_OBJC_STANDARD 90)
            set(CMAKE_OBJC_STANDARD_REQUIRED ON)
            set(CMAKE_OBJC_EXTENSIONS OFF)
            find_library(COCOA_FRAMEWORK Cocoa)
            find_library(COREGRAPHICS_FRAMEWORK CoreGraphics)
            find_library(CORETEXT_FRAMEWORK CoreText)
            find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
            find_library(COREMEDIA_FRAMEWORK CoreMedia)
            find_library(COREVIDEO_FRAMEWORK CoreVideo)
            find_library(CFNETWORK_FRAMEWORK CFNetwork)
            if(COCOA_FRAMEWORK AND COREGRAPHICS_FRAMEWORK AND CORETEXT_FRAMEWORK AND AVFOUNDATION_FRAMEWORK AND COREMEDIA_FRAMEWORK AND COREVIDEO_FRAMEWORK AND CFNETWORK_FRAMEWORK)
                target_sources(m3 PRIVATE src/backend/cocoa/m3_backend_cocoa.m)
                target_link_libraries(m3 PUBLIC ${COCOA_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${CORETEXT_FRAMEWORK}
                    ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK} ${COREVIDEO_FRAMEWORK} ${CFNETWORK_FRAMEWORK})
                target_compile_definitions(m3 PUBLIC M3_COCOA_AVAILABLE)
            else()
                message(WARNING "Cocoa frameworks not found: Cocoa backend will be stubbed.")
            endif()
        else()
            message(WARNING "Objective-C compiler not found: Cocoa backend will be stubbed.")
        endif()
    else()
        message(WARNING "Cocoa backend requires Apple platform: Cocoa backend will be stubbed.")
    endif()
endif()
if(M3_ENABLE_WIN32 AND WIN32)
    target_compile_definitions(m3 PUBLIC
        M3_WIN32_AVAILABLE
        _WIN32_WINNT=0x0600
        WINVER=0x0600
    )
    target_link_libraries(m3 PUBLIC user32 gdi32 winhttp)
endif()
if(M3_ENABLE_WEB)
    if(EMSCRIPTEN)
        target_compile_definitions(m3 PUBLIC M3_WEB_AVAILABLE)
        if(M3_ENABLE_WEBGPU)
            target_compile_definitions(m3 PUBLIC M3_WEBGPU_AVAILABLE)
            target_link_options(m3 PUBLIC "-sUSE_WEBGPU=1")
        endif()
    else()
        message(WARNING "M3_ENABLE_WEB is ON but EMSCRIPTEN is not enabled; web backend will be stubbed.")
    endif()
endif()
if(M3_ENABLE_WEBGPU AND NOT M3_ENABLE_WEB)
    message(WARNING "M3_ENABLE_WEBGPU is ON but M3_ENABLE_WEB is OFF; WebGPU disabled.")
endif()
if(M3_ENABLE_ANDROID)
    if(ANDROID)
        target_compile_definitions(m3 PUBLIC M3_ANDROID_AVAILABLE)
        target_link_libraries(m3 PUBLIC camera2ndk mediandk)
    else()
        message(WARNING "M3_ENABLE_ANDROID is ON but not building for Android; Android backend will be stubbed.")
    endif()
endif()
if(M3_ENABLE_IOS AND APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
    include(CheckLanguage)
    check_language(OBJC)
    if(CMAKE_OBJC_COMPILER)
        enable_language(OBJC)
        set(CMAKE_OBJC_STANDARD 90)
        set(CMAKE_OBJC_STANDARD_REQUIRED ON)
        set(CMAKE_OBJC_EXTENSIONS OFF)
        find_library(UIKIT_FRAMEWORK UIKit)
        find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
        find_library(COREMEDIA_FRAMEWORK CoreMedia)
        find_library(COREVIDEO_FRAMEWORK CoreVideo)
        find_library(CFNETWORK_FRAMEWORK CFNetwork)
        if(UIKIT_FRAMEWORK AND AVFOUNDATION_FRAMEWORK AND COREMEDIA_FRAMEWORK AND COREVIDEO_FRAMEWORK AND CFNETWORK_FRAMEWORK)
            target_sources(m3 PRIVATE src/backend/ios/m3_backend_ios.m)
            target_link_libraries(m3 PUBLIC ${UIKIT_FRAMEWORK} ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK}
                ${COREVIDEO_FRAMEWORK} ${CFNETWORK_FRAMEWORK})
            target_compile_definitions(m3 PUBLIC M3_IOS_AVAILABLE)
        else()
            message(WARNING "iOS frameworks not found: iOS backend will be stubbed.")
            target_sources(m3 PRIVATE src/backend/ios/m3_backend_ios.c)
        endif()
    else()
        message(WARNING "Objective-C compiler not found: iOS backend will be stubbed.")
        target_sources(m3 PRIVATE src/backend/ios/m3_backend_ios.c)
    endif()
else()
    if(M3_ENABLE_IOS)
        message(WARNING "M3_ENABLE_IOS is ON but iOS toolchain is unavailable; iOS backend will be stubbed.")
    endif()
    target_sources(m3 PRIVATE src/backend/ios/m3_backend_ios.c)
endif()
if(APPLE AND M3_APPLE_USE_CFNETWORK_C)
    target_compile_definitions(m3 PUBLIC M3_APPLE_USE_CFNETWORK_C)
endif()
m3_apply_c_settings(m3)

include(CTest)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(m3_docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    if(BUILD_TESTING)
        add_test(NAME m3_docs COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT})
    endif()
else()
    if(M3_REQUIRE_DOXYGEN)
        message(FATAL_ERROR "Doxygen not found but M3_REQUIRE_DOXYGEN is ON.")
    else()
        message(STATUS "Doxygen not found: documentation target disabled.")
    endif()
endif()

find_program(GCOVR_EXECUTABLE gcovr)
if(GCOVR_EXECUTABLE)
    if(NOT M3_ENABLE_COVERAGE)
        message(STATUS "gcovr found but M3_ENABLE_COVERAGE is OFF; m3_coverage expects coverage-enabled builds.")
    endif()

    add_custom_target(m3_coverage
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${CMAKE_CTEST_COMMAND} -R m3_phase3_tasks --output-on-failure
        COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_CURRENT_SOURCE_DIR}
            --gcov-filter ${CMAKE_BINARY_DIR}/.*
            --object-directory ${CMAKE_BINARY_DIR}
            --filter ${CMAKE_CURRENT_SOURCE_DIR}/src/.*
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-cocoa$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-docs$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-coverage$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-coverage-clean$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-coverage-fresh$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-coverage-run$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-android$
            --exclude-directories=${CMAKE_CURRENT_SOURCE_DIR}/build-web$
            --print-summary
            --merge-mode-functions=merge-use-line-0
            --fail-under-line 100
            --fail-under-function 100
            --xml-pretty -o ${CMAKE_BINARY_DIR}/coverage.xml
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running coverage (Phase 1)"
        VERBATIM
    )
else()
    message(STATUS "gcovr not found: coverage target disabled.")
endif()

if(BUILD_TESTING)
    target_compile_definitions(m3 PRIVATE M3_TESTING)

    add_executable(m3_phase0_compile tests/phase0/compile.c)
    target_link_libraries(m3_phase0_compile PRIVATE m3_headers)
    m3_apply_c_settings(m3_phase0_compile)

    add_test(NAME m3_phase0_compile COMMAND m3_phase0_compile)

    add_executable(m3_phase1_allocator tests/phase1/allocator.c)
    target_link_libraries(m3_phase1_allocator PRIVATE m3)
    m3_apply_c_settings(m3_phase1_allocator)
    add_test(NAME m3_phase1_allocator COMMAND m3_phase1_allocator)

    add_executable(m3_phase1_arena tests/phase1/arena.c)
    target_link_libraries(m3_phase1_arena PRIVATE m3)
    target_compile_definitions(m3_phase1_arena PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_arena)
    add_test(NAME m3_phase1_arena COMMAND m3_phase1_arena)

    add_executable(m3_phase1_handle_system tests/phase1/handle_system.c)
    target_link_libraries(m3_phase1_handle_system PRIVATE m3)
    target_compile_definitions(m3_phase1_handle_system PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_handle_system)
    add_test(NAME m3_phase1_handle_system COMMAND m3_phase1_handle_system)

    add_executable(m3_phase1_utf8 tests/phase1/utf8.c)
    target_link_libraries(m3_phase1_utf8 PRIVATE m3)
    target_compile_definitions(m3_phase1_utf8 PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_utf8)
    add_test(NAME m3_phase1_utf8 COMMAND m3_phase1_utf8)

    add_executable(m3_phase1_log tests/phase1/log.c)
    target_link_libraries(m3_phase1_log PRIVATE m3)
    target_compile_definitions(m3_phase1_log PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_log)
    add_test(NAME m3_phase1_log COMMAND m3_phase1_log)

    add_executable(m3_phase1_store tests/phase1/store.c)
    target_link_libraries(m3_phase1_store PRIVATE m3)
    target_compile_definitions(m3_phase1_store PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_store)
    add_test(NAME m3_phase1_store COMMAND m3_phase1_store)

    add_executable(m3_phase2_math tests/phase2/math.c)
    target_link_libraries(m3_phase2_math PRIVATE m3)
    m3_apply_c_settings(m3_phase2_math)
    add_test(NAME m3_phase2_math COMMAND m3_phase2_math)

    add_executable(m3_phase2_color tests/phase2/color.c)
    target_link_libraries(m3_phase2_color PRIVATE m3)
    target_compile_definitions(m3_phase2_color PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase2_color)
    add_test(NAME m3_phase2_color COMMAND m3_phase2_color)

    add_executable(m3_phase2_layout tests/phase2/layout.c)
    target_link_libraries(m3_phase2_layout PRIVATE m3)
    target_compile_definitions(m3_phase2_layout PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase2_layout)
    add_test(NAME m3_phase2_layout COMMAND m3_phase2_layout)

    add_executable(m3_phase2_anim tests/phase2/anim.c)
    target_link_libraries(m3_phase2_anim PRIVATE m3)
    target_compile_definitions(m3_phase2_anim PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase2_anim)
    add_test(NAME m3_phase2_anim COMMAND m3_phase2_anim)

    add_executable(m3_phase2_router tests/phase2/router.c)
    target_link_libraries(m3_phase2_router PRIVATE m3)
    target_compile_definitions(m3_phase2_router PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase2_router)
    add_test(NAME m3_phase2_router COMMAND m3_phase2_router)

    add_executable(m3_phase3_render tests/phase3/render.c)
    target_link_libraries(m3_phase3_render PRIVATE m3)
    target_compile_definitions(m3_phase3_render PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_render)
    add_test(NAME m3_phase3_render COMMAND m3_phase3_render)

    add_executable(m3_phase3_event tests/phase3/event.c)
    target_link_libraries(m3_phase3_event PRIVATE m3)
    target_compile_definitions(m3_phase3_event PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_event)
    add_test(NAME m3_phase3_event COMMAND m3_phase3_event)

    add_executable(m3_phase3_tasks tests/phase3/tasks.c)
    target_link_libraries(m3_phase3_tasks PRIVATE m3)
    target_compile_definitions(m3_phase3_tasks PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_tasks)
    add_test(NAME m3_phase3_tasks COMMAND m3_phase3_tasks)

    add_executable(m3_phase4_null_backend tests/phase4/null_backend.c)
    target_link_libraries(m3_phase4_null_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_null_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_null_backend)
    add_test(NAME m3_phase4_null_backend COMMAND m3_phase4_null_backend)

    add_executable(m3_phase4_sdl3_backend tests/phase4/sdl3_backend.c)
    target_link_libraries(m3_phase4_sdl3_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_sdl3_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_sdl3_backend)
    add_test(NAME m3_phase4_sdl3_backend COMMAND m3_phase4_sdl3_backend)

    add_executable(m3_phase4_gtk4_backend tests/phase4/gtk4_backend.c)
    target_link_libraries(m3_phase4_gtk4_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_gtk4_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_gtk4_backend)
    add_test(NAME m3_phase4_gtk4_backend COMMAND m3_phase4_gtk4_backend)

    add_executable(m3_phase4_cocoa_backend tests/phase4/cocoa_backend.c)
    target_link_libraries(m3_phase4_cocoa_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_cocoa_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_cocoa_backend)
    add_test(NAME m3_phase4_cocoa_backend COMMAND m3_phase4_cocoa_backend)

    add_executable(m3_phase4_win32_backend tests/phase4/win32_backend.c)
    target_link_libraries(m3_phase4_win32_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_win32_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_win32_backend)
    add_test(NAME m3_phase4_win32_backend COMMAND m3_phase4_win32_backend)

    add_executable(m3_phase4_web_backend tests/phase4/web_backend.c)
    target_link_libraries(m3_phase4_web_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_web_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_web_backend)
    add_test(NAME m3_phase4_web_backend COMMAND m3_phase4_web_backend)

    add_executable(m3_phase4_ios_backend tests/phase4/ios_backend.c)
    target_link_libraries(m3_phase4_ios_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_ios_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_ios_backend)
    add_test(NAME m3_phase4_ios_backend COMMAND m3_phase4_ios_backend)

    add_executable(m3_phase4_android_backend tests/phase4/android_backend.c)
    target_link_libraries(m3_phase4_android_backend PRIVATE m3)
    target_compile_definitions(m3_phase4_android_backend PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase4_android_backend)
    add_test(NAME m3_phase4_android_backend COMMAND m3_phase4_android_backend)

    add_executable(m3_phase5_visuals tests/phase5/visuals.c)
    target_link_libraries(m3_phase5_visuals PRIVATE m3)
    target_compile_definitions(m3_phase5_visuals PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_visuals)
    add_test(NAME m3_phase5_visuals COMMAND m3_phase5_visuals)

    add_executable(m3_phase5_text tests/phase5/text.c)
    target_link_libraries(m3_phase5_text PRIVATE m3)
    target_compile_definitions(m3_phase5_text PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_text)
    add_test(NAME m3_phase5_text COMMAND m3_phase5_text)

    add_executable(m3_phase5_buttons tests/phase5/buttons.c)
    target_link_libraries(m3_phase5_buttons PRIVATE m3)
    target_compile_definitions(m3_phase5_buttons PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_buttons)
    add_test(NAME m3_phase5_buttons COMMAND m3_phase5_buttons)

    add_executable(m3_phase5_selection tests/phase5/selection.c)
    target_link_libraries(m3_phase5_selection PRIVATE m3)
    target_compile_definitions(m3_phase5_selection PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_selection)
    add_test(NAME m3_phase5_selection COMMAND m3_phase5_selection)

    add_executable(m3_phase5_text_field tests/phase5/text_field.c)
    target_link_libraries(m3_phase5_text_field PRIVATE m3)
    target_compile_definitions(m3_phase5_text_field PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_text_field)
    add_test(NAME m3_phase5_text_field COMMAND m3_phase5_text_field)

    add_executable(m3_phase5_list tests/phase5/list.c)
    target_link_libraries(m3_phase5_list PRIVATE m3)
    target_compile_definitions(m3_phase5_list PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_list)
    add_test(NAME m3_phase5_list COMMAND m3_phase5_list)

    add_executable(m3_phase5_navigation tests/phase5/navigation.c)
    target_link_libraries(m3_phase5_navigation PRIVATE m3)
    target_compile_definitions(m3_phase5_navigation PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_navigation)
    add_test(NAME m3_phase5_navigation COMMAND m3_phase5_navigation)

    add_executable(m3_phase5_dialogs tests/phase5/dialogs.c)
    target_link_libraries(m3_phase5_dialogs PRIVATE m3)
    target_compile_definitions(m3_phase5_dialogs PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_dialogs)
    add_test(NAME m3_phase5_dialogs COMMAND m3_phase5_dialogs)

add_executable(m3_phase5_progress tests/phase5/progress.c)
target_link_libraries(m3_phase5_progress PRIVATE m3)
target_compile_definitions(m3_phase5_progress PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase5_progress)
add_test(NAME m3_phase5_progress COMMAND m3_phase5_progress)

add_executable(m3_phase6_storage tests/phase6/storage.c)
target_link_libraries(m3_phase6_storage PRIVATE m3)
target_compile_definitions(m3_phase6_storage PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase6_storage)
add_test(NAME m3_phase6_storage COMMAND m3_phase6_storage)
add_executable(m3_phase6_camera tests/phase6/camera.c)
target_link_libraries(m3_phase6_camera PRIVATE m3)
target_compile_definitions(m3_phase6_camera PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase6_camera)
add_test(NAME m3_phase6_camera COMMAND m3_phase6_camera)
add_executable(m3_phase6_network tests/phase6/network.c)
target_link_libraries(m3_phase6_network PRIVATE m3)
target_compile_definitions(m3_phase6_network PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase6_network)
add_test(NAME m3_phase6_network COMMAND m3_phase6_network)
endif()
