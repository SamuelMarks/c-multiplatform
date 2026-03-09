cmake_minimum_required(VERSION 3.16)

project(LibCMPC VERSION 0.0.1 LANGUAGES C)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(CMP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(CMP_ENABLE_COVERAGE "Enable coverage instrumentation (GCC/Clang only)" OFF)
option(CMP_REQUIRE_DOXYGEN "Require Doxygen for documentation coverage" OFF)
set(CMP_COVERAGE_LINE_THRESHOLD 100 CACHE STRING "Minimum line coverage percentage for cmp_coverage.")
set(CMP_COVERAGE_FUNCTION_THRESHOLD 100 CACHE STRING "Minimum function coverage percentage for cmp_coverage.")
set(CMP_COVERAGE_BRANCH_THRESHOLD 100 CACHE STRING "Minimum branch coverage percentage for cmp_coverage.")
option(CMP_ENABLE_LCOV "Generate lcov coverage output when lcov is available" ON)
option(CMP_ENABLE_SDL3 "Enable SDL3 debug backend" OFF)
option(CMP_ENABLE_SDL3_TTF "Enable SDL3_ttf text rendering for the SDL3 backend" OFF)
option(CMP_ENABLE_GTK4 "Enable GTK4 backend" OFF)
option(CMP_ENABLE_COCOA "Enable Cocoa backend" OFF)
option(CMP_ENABLE_WIN32 "Enable Win32 backend" ON)
option(CMP_ENABLE_WEB "Enable Web (Emscripten) backend" OFF)
option(CMP_ENABLE_WEBGPU "Enable WebGPU renderer for the Web backend" OFF)
option(CMP_ENABLE_IOS "Enable iOS backend" OFF)
option(CMP_APPLE_USE_CFNETWORK_C "Use CFNetwork C APIs for Apple network backends" ON)
option(CMP_ENABLE_LIBCURL "Enable libcurl-backed network support" ON)
if(ANDROID)
    set(CMP_ENABLE_ANDROID_DEFAULT ON)
else()
    set(CMP_ENABLE_ANDROID_DEFAULT OFF)
endif()
option(CMP_ENABLE_ANDROID "Enable Android backend" ${CMP_ENABLE_ANDROID_DEFAULT})
option(CMP_ENABLE_PACKAGING "Enable packaging targets" OFF)
option(CMP_BUILD_WEB_APP "Build minimal web app target" OFF)
option(CMP_BUILD_PACKAGING_STUB "Build packaging stub executable" OFF)

add_library(cmp_headers INTERFACE)
target_include_directories(cmp_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(CMP_ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(CMP_ENABLE_VALGRIND "Enable Valgrind target for testing" OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(CMP_MSVC_STATIC_CRT "Use static CRT (/MT, /MTd) on MSVC" OFF)
option(CMP_UNICODE "Build with UNICODE and _UNICODE defined" ON)
option(CMP_SINGLE_THREADED "Build as single threaded (disables multithreading support)" OFF)
option(CMP_ENABLE_LTO "Enable Link-Time Optimization (LTO)" OFF)
set(CMP_MSVC_RUNTIME_CHECKS "RTC1" CACHE STRING "MSVC Runtime checks: RTC1, RTCs, RTCu, OFF")

if(MSVC)
    if(CMP_MSVC_STATIC_CRT)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endif()

if(CMP_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO is not supported: ${ipo_error}")
    endif()
endif()

set(CMP_WARNINGS_C
    $<$<C_COMPILER_ID:MSVC>:/W4>
    # $<$<C_COMPILER_ID:MSVC>:/Za> # Removed because it breaks <windows.h> and <winnt.h> headers
    $<$<C_COMPILER_ID:MSVC>:/wd4127>
    $<$<AND:$<C_COMPILER_ID:MSVC>,$<VERSION_GREATER_EQUAL:${MSVC_VERSION},1700>>:/sdl>
    $<$<AND:$<C_COMPILER_ID:MSVC>,$<CONFIG:Debug>,$<NOT:$<STREQUAL:${CMP_MSVC_RUNTIME_CHECKS},OFF>>>:/${CMP_MSVC_RUNTIME_CHECKS}>
    $<$<C_COMPILER_ID:MSVC>:$<$<BOOL:${CMP_WARNINGS_AS_ERRORS}>:/WX>>
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wextra>
    $<$<C_COMPILER_ID:GNU>:-pedantic>
    $<$<C_COMPILER_ID:GNU>:-ansi>
    $<$<C_COMPILER_ID:GNU>:-Wconversion>
    $<$<C_COMPILER_ID:GNU>:-Wsign-conversion>
    $<$<C_COMPILER_ID:GNU>:-Wcast-qual>
    $<$<C_COMPILER_ID:GNU>:-Wformat=2>
    $<$<C_COMPILER_ID:GNU>:-Wundef>
    $<$<C_COMPILER_ID:GNU>:-Wshadow>
    $<$<C_COMPILER_ID:GNU>:-Wcast-align>
    $<$<C_COMPILER_ID:GNU>:-Wstrict-prototypes>
    $<$<C_COMPILER_ID:GNU>:-Wmissing-prototypes>
    $<$<C_COMPILER_ID:GNU>:-Wmissing-declarations>
    $<$<C_COMPILER_ID:GNU>:-Wredundant-decls>
    $<$<C_COMPILER_ID:GNU>:-Wnested-externs>
    $<$<C_COMPILER_ID:GNU>:-Winline>
    $<$<C_COMPILER_ID:GNU>:-Wvla>
    $<$<C_COMPILER_ID:GNU>:$<$<BOOL:${CMP_WARNINGS_AS_ERRORS}>:>>
    $<$<C_COMPILER_ID:Clang>:-Wall>
    $<$<C_COMPILER_ID:Clang>:-Wextra>
    $<$<C_COMPILER_ID:Clang>:-pedantic>
    $<$<C_COMPILER_ID:Clang>:-ansi>
    $<$<C_COMPILER_ID:Clang>:-Wconversion>
    $<$<C_COMPILER_ID:Clang>:-Wsign-conversion>
    $<$<C_COMPILER_ID:Clang>:-Wcast-qual>
    $<$<C_COMPILER_ID:Clang>:-Wformat=2>
    $<$<C_COMPILER_ID:Clang>:-Wundef>
    $<$<C_COMPILER_ID:Clang>:-Wshadow>
    $<$<C_COMPILER_ID:Clang>:-Wcast-align>
    $<$<C_COMPILER_ID:Clang>:-Wstrict-prototypes>
    $<$<C_COMPILER_ID:Clang>:-Wmissing-prototypes>
    $<$<C_COMPILER_ID:Clang>:-Wmissing-declarations>
    $<$<C_COMPILER_ID:Clang>:-Wredundant-decls>
    $<$<C_COMPILER_ID:Clang>:-Wnested-externs>
    $<$<C_COMPILER_ID:Clang>:-Winline>
    $<$<C_COMPILER_ID:Clang>:-Wvla>
    $<$<C_COMPILER_ID:Clang>:$<$<BOOL:${CMP_WARNINGS_AS_ERRORS}>:>>
)

function(cmp_apply_c_settings target_name)
    target_compile_options(${target_name} PRIVATE ${CMP_WARNINGS_C})

    if(CMP_ENABLE_COVERAGE)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} PRIVATE -O0 -g --coverage)
            target_link_options(${target_name} PRIVATE --coverage)
        else()
            message(FATAL_ERROR "CMP_ENABLE_COVERAGE is only supported with GCC or Clang.")
        endif()
    endif()

    if(CMP_ENABLE_ASAN)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
        else()
            message(WARNING "AddressSanitizer is primarily supported with GCC/Clang.")
        endif()
    endif()
endfunction()

add_library(cmpc
    src/core/cmp_core.c
    src/core/cmp_object.c
    src/core/cmp_arena.c
    src/core/cmp_utf8.c
    src/core/cmp_log.c
    src/core/cmp_store.c
    src/core/cmp_storage.c
    src/core/cmp_i18n.c
    src/core/cmp_math.c
    src/core/cmp_layout.c
    src/core/cmp_a11y.c
    src/core/cmp_anim.c
    src/core/cmp_router.c
    src/core/cmp_predictive.c
    src/core/cmp_path.c
    src/core/cmp_render.c
    src/core/cmp_event.c
    src/core/cmp_gesture.c
    src/core/cmp_scroll.c
    src/core/cmp_tasks.c
    src/core/cmp_visuals.c
    src/core/cmp_text.c
    src/core/cmp_icon.c
    src/core/cmp_extras.c
    src/core/cmp_text_field.c
    src/core/cmp_list.c
    src/core/cmp_camera.c
    src/core/cmp_image.c
    src/core/cmp_video.c
    src/core/cmp_audio.c
    src/core/cmp_network.c
    src/backend/null/cmp_backend_null.c
    src/backend/sdl3/cmp_backend_sdl3.c
    src/backend/linux/cmp_backend_gtk4.c
    src/backend/cocoa/cmp_backend_cocoa.c
    src/backend/win32/cmp_backend_win32.c
    src/backend/web/cmp_backend_web.c
    src/backend/android/cmp_backend_android.c
)
add_library(cmp::cmpc ALIAS cmpc)
target_include_directories(cmpc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if (NOT WIN32)
    target_link_libraries(cmpc PUBLIC m)
endif()

add_library(m3
    src/m3/m3_color.c
    src/m3/m3_adaptive.c
    src/m3/m3_typography.c
    src/m3/m3_motion.c
    src/m3/m3_text_field.c
    src/m3/m3_search.c
    src/m3/m3_carousel.c

    src/m3/m3_list.c
        src/m3/m3_navigation.c
    src/m3/m3_app_bar.c
    src/m3/m3_tabs.c
    src/m3/m3_dialogs.c
    src/m3/m3_tooltip.c
    src/m3/m3_sheet.c
    src/m3/m3_scaffold.c
    src/m3/m3_menu.c
    src/m3/m3_button.c
    src/m3/m3_card.c
    src/m3/m3_divider.c
    src/m3/m3_data_table.c
    src/m3/m3_stepper.c
    src/m3/m3_badge.c
    src/m3/m3_chip.c
    src/m3/m3_selection.c
    src/m3/m3_progress.c
    src/m3/m3_date_picker.c
    src/m3/m3_time_picker.c
    src/m3/m3_pull_to_refresh.c
    src/m3/m3_shape.c
)
add_library(m3::m3 ALIAS m3)
target_include_directories(m3 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(m3 PUBLIC cmpc)

add_library(f2
    src/f2/f2_color.c
    src/f2/f2_typography.c
    src/f2/f2_button.c
    src/f2/f2_selection.c
    src/f2/f2_text_field.c
    src/f2/f2_slider.c
    src/f2/f2_progress.c
    src/f2/f2_badge.c
    src/f2/f2_link.c
    src/f2/f2_card.c
    src/f2/f2_flyout.c
    src/f2/f2_dialog.c
    src/f2/f2_tooltip.c
    src/f2/f2_spin_button.c
    src/f2/f2_searchbox.c
    src/f2/f2_navigation.c
    src/f2/f2_command_bar.c
    src/f2/f2_data_components.c
    src/f2/f2_message_bar.c
    src/f2/f2_avatar.c
)
add_library(f2::f2 ALIAS f2)
target_include_directories(f2 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(f2 PUBLIC cmpc)

add_library(cupertino
    src/cupertino/cupertino_color.c
    src/cupertino/cupertino_shape.c
    src/cupertino/cupertino_typography.c
    src/cupertino/cupertino_button.c
    src/cupertino/cupertino_blur.c
    src/cupertino/cupertino_haptics.c
    src/cupertino/cupertino_navigation_bar.c
    src/cupertino/cupertino_tab_bar.c
    src/cupertino/cupertino_action_sheet.c
    src/cupertino/cupertino_pull_to_refresh.c
    src/cupertino/cupertino_menu.c
    src/cupertino/cupertino_switch.c
    src/cupertino/cupertino_slider.c
    src/cupertino/cupertino_activity_indicator.c
    src/cupertino/cupertino_alert.c
    src/cupertino/cupertino_segmented_control.c
    src/cupertino/cupertino_progress_bar.c
    src/cupertino/cupertino_stepper.c
    src/cupertino/cupertino_text_field.c
    src/cupertino/cupertino_mac_traffic_lights.c
    src/cupertino/cupertino_list_section.c
    src/cupertino/cupertino_picker.c
    src/cupertino/cupertino_detent_sheet.c
    src/cupertino/cupertino_mac_sidebar.c
    src/cupertino/cupertino_tv_card.c
    src/cupertino/cupertino_watch_button.c
    src/cupertino/cupertino_date_picker.c
    src/cupertino/cupertino_mac_toolbar.c
    src/cupertino/cupertino_focus_ring.c
)
add_library(cupertino::cupertino ALIAS cupertino)
target_include_directories(cupertino PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cupertino PUBLIC cmpc)

if(CMP_ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cmpc PUBLIC -O0 -g --coverage)
        target_link_options(cmpc PUBLIC --coverage)
        target_compile_options(m3 PUBLIC -O0 -g --coverage)
        target_link_options(m3 PUBLIC --coverage)
    else()
        message(FATAL_ERROR "CMP_ENABLE_COVERAGE is only supported with GCC or Clang.")
    endif()
endif()

if(CMP_ENABLE_ASAN)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cmpc PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(cmpc PUBLIC -fsanitize=address,undefined)
        target_compile_options(m3 PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(m3 PUBLIC -fsanitize=address,undefined)
    endif()
endif()

if(CMP_ENABLE_VALGRIND)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1")
    find_program(MEMORYCHECK_COMMAND valgrind)
endif()

if(BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(cmpc PRIVATE CMP_BUILD_DLL)
    target_compile_definitions(cmpc INTERFACE CMP_USE_DLL)
    target_compile_definitions(m3 PRIVATE CMP_BUILD_DLL)
    target_compile_definitions(m3 INTERFACE CMP_USE_DLL)
endif()

set(CMP_HAS_SDL3 OFF)
set(CMP_HAS_SDL3_TTF OFF)
set(CMP_HAS_GTK4 OFF)
set(CMP_HAS_LIBCURL OFF)

# FIX: Applied definitions to 'cmpc' instead of 'm3' so backend sources see them
if(CMP_ENABLE_SDL3)
    find_package(SDL3 CONFIG QUIET)
    if(SDL3_FOUND)
        set(CMP_HAS_SDL3 ON)
        target_link_libraries(cmpc PUBLIC SDL3::SDL3)
        target_compile_definitions(cmpc PUBLIC CMP_SDL3_AVAILABLE)
    else()
        message(WARNING "SDL3 not found: SDL3 backend will be stubbed.")
    endif()
endif()

if(CMP_ENABLE_SDL3_TTF)
    if(NOT CMP_ENABLE_SDL3)
        message(WARNING "CMP_ENABLE_SDL3_TTF is ON but CMP_ENABLE_SDL3 is OFF; SDL3_ttf support disabled.")
    else()
        find_package(SDL3_ttf CONFIG QUIET)
        if(TARGET SDL3_ttf::SDL3_ttf)
            set(CMP_HAS_SDL3_TTF ON)
            target_link_libraries(cmpc PUBLIC SDL3_ttf::SDL3_ttf)
            target_compile_definitions(cmpc PUBLIC CMP_SDL3_TTF_AVAILABLE)
        elseif(TARGET SDL3_ttf::SDL3_ttf-static)
            set(CMP_HAS_SDL3_TTF ON)
            target_link_libraries(cmpc PUBLIC SDL3_ttf::SDL3_ttf-static)
            target_compile_definitions(cmpc PUBLIC CMP_SDL3_TTF_AVAILABLE)
        else()
            message(WARNING "SDL3_ttf not found: SDL3 text rendering disabled.")
        endif()
    endif()
endif()

if(CMP_ENABLE_GTK4)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GTK4 QUIET IMPORTED_TARGET gtk4)
        if(GTK4_FOUND)
            set(CMP_HAS_GTK4 ON)
            target_link_libraries(cmpc PUBLIC PkgConfig::GTK4)
            target_compile_definitions(cmpc PUBLIC CMP_GTK4_AVAILABLE)
        else()
            message(WARNING "GTK4 not found: GTK4 backend will be stubbed.")
        endif()
    else()
        message(WARNING "pkg-config not found: GTK4 backend will be stubbed.")
    endif()
endif()

if(CMP_ENABLE_LIBCURL)
    find_package(CURL QUIET)
    if(CURL_FOUND)
        set(CMP_HAS_LIBCURL ON)
        if(TARGET CURL::libcurl)
            target_link_libraries(cmpc PUBLIC CURL::libcurl)
        else()
            if(CURL_INCLUDE_DIRS)
                target_include_directories(cmpc PUBLIC ${CURL_INCLUDE_DIRS})
            endif()
            if(CURL_LIBRARIES)
                target_link_libraries(cmpc PUBLIC ${CURL_LIBRARIES})
            endif()
        endif()
        target_compile_definitions(cmpc PUBLIC CMP_LIBCURL_AVAILABLE)
    else()
        message(WARNING "libcurl not found: libcurl-backed network support disabled.")
    endif()
endif()

if(CMP_ENABLE_COCOA)
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
                target_sources(cmpc PRIVATE src/backend/cocoa/cmp_backend_cocoa.m)
                target_link_libraries(cmpc PUBLIC ${COCOA_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${CORETEXT_FRAMEWORK}
                    ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK} ${COREVIDEO_FRAMEWORK} ${CFNETWORK_FRAMEWORK})
                target_compile_definitions(cmpc PUBLIC CMP_COCOA_AVAILABLE)
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

# FIX: Applied definitions to 'cmpc' for Win32
if(CMP_ENABLE_WIN32 AND WIN32)
    target_compile_definitions(cmpc PUBLIC
        CMP_WIN32_AVAILABLE
        _WIN32_WINNT=0x0600
        WINVER=0x0600
    )
    target_link_libraries(cmpc PUBLIC user32 gdi32)
    if(NOT MSVC OR MSVC_VERSION GREATER_EQUAL 1500)
        target_link_libraries(cmpc PUBLIC winhttp)
    endif()
endif()

if(CMP_ENABLE_WEB)
    if(EMSCRIPTEN)
        target_compile_definitions(cmpc PUBLIC CMP_WEB_AVAILABLE)
        if(CMP_ENABLE_WEBGPU)
            target_compile_definitions(cmpc PUBLIC CMP_WEBGPU_AVAILABLE)
            target_link_options(cmpc PUBLIC "-sUSE_WEBGPU=1")
        endif()
    else()
        message(WARNING "CMP_ENABLE_WEB is ON but EMSCRIPTEN is not enabled; web backend will be stubbed.")
    endif()
endif()

if(CMP_ENABLE_WEBGPU AND NOT CMP_ENABLE_WEB)
    message(WARNING "CMP_ENABLE_WEBGPU is ON but CMP_ENABLE_WEB is OFF; WebGPU disabled.")
endif()

if(CMP_ENABLE_ANDROID)
    if(ANDROID)
        target_compile_definitions(cmpc PUBLIC CMP_ANDROID_AVAILABLE)
        set(CMP_ANDROID_API_LEVEL 0)
        if(DEFINED ANDROID_NATIVE_API_LEVEL)
            set(CMP_ANDROID_API_LEVEL ${ANDROID_NATIVE_API_LEVEL})
        elseif(DEFINED ANDROID_PLATFORM)
            string(REGEX REPLACE "android-" "" CMP_ANDROID_API_LEVEL "${ANDROID_PLATFORM}")
        elseif(CMAKE_SYSTEM_VERSION)
            set(CMP_ANDROID_API_LEVEL ${CMAKE_SYSTEM_VERSION})
        endif()
        if(CMP_ANDROID_API_LEVEL GREATER_EQUAL 24)
            find_library(CMP_ANDROID_CAMERA2_LIB camera2ndk)
            find_library(CMP_ANDROID_MEDIA_LIB mediandk)
            if(CMP_ANDROID_CAMERA2_LIB AND CMP_ANDROID_MEDIA_LIB)
                target_link_libraries(cmpc PUBLIC ${CMP_ANDROID_CAMERA2_LIB} ${CMP_ANDROID_MEDIA_LIB})
            else()
                message(WARNING "Android camera2ndk/mediandk not found; camera support disabled.")
            endif()
        else()
            message(WARNING "Android API level < 24; camera2ndk/mediandk not linked.")
        endif()
    else()
        message(WARNING "CMP_ENABLE_ANDROID is ON but not building for Android; Android backend will be stubbed.")
    endif()
endif()

if(CMP_ENABLE_IOS AND APPLE AND CMAKE_SYSTEM_NAME STREQUAL "iOS")
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
            target_sources(cmpc PRIVATE src/backend/ios/cmp_backend_ios.m)
            target_link_libraries(cmpc PUBLIC ${UIKIT_FRAMEWORK} ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK}
                ${COREVIDEO_FRAMEWORK} ${CFNETWORK_FRAMEWORK})
            target_compile_definitions(cmpc PUBLIC CMP_IOS_AVAILABLE)
        else()
            message(WARNING "iOS frameworks not found: iOS backend will be stubbed.")
            target_sources(cmpc PRIVATE src/backend/ios/cmp_backend_ios.c)
        endif()
    else()
        message(WARNING "Objective-C compiler not found: iOS backend will be stubbed.")
        target_sources(cmpc PRIVATE src/backend/ios/cmp_backend_ios.c)
    endif()
else()
    if(CMP_ENABLE_IOS)
        message(WARNING "CMP_ENABLE_IOS is ON but iOS toolchain is unavailable; iOS backend will be stubbed.")
    endif()
    target_sources(cmpc PRIVATE src/backend/ios/cmp_backend_ios.c)
endif()

if(APPLE AND CMP_APPLE_USE_CFNETWORK_C)
    target_compile_definitions(cmpc PUBLIC CMP_APPLE_USE_CFNETWORK_C)
endif()

cmp_apply_c_settings(cmpc)
cmp_apply_c_settings(m3)

set_target_properties(m3 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(CMP_BUILD_WEB_APP)
    add_executable(cmp_web_app packaging/web/app.c)
    target_link_libraries(cmp_web_app PRIVATE cmpc)
    cmp_apply_c_settings(cmp_web_app)
endif()

if(CMP_BUILD_PACKAGING_STUB)
    add_executable(cmp_packaging_stub packaging/desktop/stub.c)
    target_link_libraries(cmp_packaging_stub PRIVATE cmpc)
    set_target_properties(cmp_packaging_stub PROPERTIES OUTPUT_NAME "libcmpc-stub")
    cmp_apply_c_settings(cmp_packaging_stub)
    install(TARGETS cmp_packaging_stub
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

include(CMakePackageConfigHelpers)

set(LIBCMPC_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/LibCMPC")

install(TARGETS cmpc
    EXPORT LibCMPCTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS m3
    EXPORT LibM3Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibCMPCConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LibCMPCConfig.cmake
    INSTALL_DESTINATION ${LIBCMPC_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LibCMPCConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LibCMPCConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LibCMPCConfigVersion.cmake
    DESTINATION ${LIBCMPC_INSTALL_CMAKEDIR}
)

install(EXPORT LibCMPCTargets
    NAMESPACE cmp::
    DESTINATION ${LIBCMPC_INSTALL_CMAKEDIR}
)
install(EXPORT LibM3Targets
    NAMESPACE m3::
    DESTINATION ${LIBCMPC_INSTALL_CMAKEDIR}
)

export(EXPORT LibCMPCTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/LibCMPCTargets.cmake
    NAMESPACE cmp::
)
export(EXPORT LibM3Targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/LibM3Targets.cmake
    NAMESPACE m3::
)

if(CMP_ENABLE_PACKAGING)
    include(cmake/CMPPackaging.cmake)
endif()

include(CTest)

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(cmp_docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )

    if(BUILD_TESTING)
        add_test(NAME cmp_docs COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT})
    endif()
else()
    if(CMP_REQUIRE_DOXYGEN)
        message(FATAL_ERROR "Doxygen not found but CMP_REQUIRE_DOXYGEN is ON.")
    else()
        message(STATUS "Doxygen not found: documentation target disabled.")
    endif()
endif()

find_program(GCOVR_EXECUTABLE gcovr)
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)
find_program(LLVM_COV_EXECUTABLE llvm-cov)

set(CMP_GCOVR_GCOV_ARGS)
if(CMAKE_C_COMPILER_ID MATCHES "Clang" AND LLVM_COV_EXECUTABLE)
    set(CMP_GCOVR_GCOV_ARGS --gcov-executable "${LLVM_COV_EXECUTABLE} gcov")
endif()

if(GCOVR_EXECUTABLE)
    if(CMP_ENABLE_COVERAGE)
        add_custom_target(cmp_coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${CMAKE_CTEST_COMMAND} -R cmp_phase3_tasks --output-on-failure
            COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMP_GCOVR_GCOV_ARGS}
                --object-directory ${CMAKE_BINARY_DIR}
                --filter .*src/.*
                --exclude .*src/backend/.*
                --exclude .*src/cupertino/.*
                --exclude .*src/m3/.*
                --exclude .*src/f2/.*
                --exclude .*src/core/cmp_tasks.c                --gcov-ignore-errors=source_not_found
                --gcov-ignore-errors=no_working_dir_found
                --exclude-noncode-lines
                
                --print-summary
                --merge-mode-functions=merge-use-line-0
                --fail-under-line ${CMP_COVERAGE_LINE_THRESHOLD}
                --fail-under-function ${CMP_COVERAGE_FUNCTION_THRESHOLD}
                --fail-under-branch ${CMP_COVERAGE_BRANCH_THRESHOLD}
                --xml-pretty -o ${CMAKE_BINARY_DIR}/coverage.xml
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running coverage (gcovr)"
            VERBATIM
        )
    else()
        add_custom_target(cmp_coverage
            COMMAND ${CMAKE_COMMAND} -E echo
                "cmp_coverage requires CMP_ENABLE_COVERAGE=ON."
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "Coverage disabled"
            VERBATIM
        )
    endif()
else()
    message(STATUS "gcovr not found: cmp_coverage target disabled.")
endif()

if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE AND CMP_ENABLE_LCOV)
    if(CMP_ENABLE_COVERAGE)
        add_custom_target(cmp_coverage_lcov
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_EXECUTABLE} --capture
                --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage.info
                --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_EXECUTABLE} ${CMAKE_BINARY_DIR}/coverage.info
                --branch-coverage
                --output-directory ${CMAKE_BINARY_DIR}/coverage-lcov
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running coverage (lcov)"
            VERBATIM
        )
    else()
        add_custom_target(cmp_coverage_lcov
            COMMAND ${CMAKE_COMMAND} -E echo
                "cmp_coverage_lcov requires CMP_ENABLE_COVERAGE=ON."
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "Coverage disabled"
            VERBATIM
        )
    endif()
else()
    message(STATUS "lcov/genhtml not found or disabled: cmp_coverage_lcov target disabled.")
endif()

option(CMP_BUILD_DOCGEN "Build the documentation generator tool" ON)
if(CMP_BUILD_DOCGEN)
    add_executable(cmp_docgen tools/docgen/docgen.c)
    target_compile_options(cmp_docgen PRIVATE -g)
    target_link_libraries(cmp_docgen PRIVATE m3 cmpc)
    cmp_apply_c_settings(cmp_docgen)
endif()

if(BUILD_TESTING)
    target_compile_definitions(cmpc PUBLIC CMP_TESTING)
    target_compile_definitions(m3 PUBLIC CMP_TESTING)

    add_executable(cmp_phase0_arena tests/phase0/arena.c)
    target_link_libraries(cmp_phase0_arena PRIVATE cmpc)
    add_test(NAME cmp_phase0_arena COMMAND cmp_phase0_arena)

    add_executable(cmp_phase0_compile tests/phase0/compile.c)
    target_link_libraries(cmp_phase0_compile PRIVATE cmpc)
    add_test(NAME cmp_phase0_compile COMMAND cmp_phase0_compile)

    add_executable(cmp_phase0_math tests/phase0/math.c)
    target_link_libraries(cmp_phase0_math PRIVATE cmpc)
    add_test(NAME cmp_phase0_math COMMAND cmp_phase0_math)

    add_executable(cmp_phase1_allocator tests/phase1/allocator.c)
    target_link_libraries(cmp_phase1_allocator PRIVATE cmpc)
    add_test(NAME cmp_phase1_allocator COMMAND cmp_phase1_allocator)

    add_executable(cmp_phase1_arena tests/phase1/arena.c)
    target_link_libraries(cmp_phase1_arena PRIVATE cmpc)
    add_test(NAME cmp_phase1_arena COMMAND cmp_phase1_arena)

    add_executable(cmp_phase1_handle_system tests/phase1/handle_system.c)
    target_link_libraries(cmp_phase1_handle_system PRIVATE cmpc)
    add_test(NAME cmp_phase1_handle_system COMMAND cmp_phase1_handle_system)

    add_executable(cmp_phase1_i18n tests/phase1/i18n.c)
    target_link_libraries(cmp_phase1_i18n PRIVATE cmpc)
    add_test(NAME cmp_phase1_i18n COMMAND cmp_phase1_i18n)

    add_executable(cmp_phase1_log tests/phase1/log.c)
    target_link_libraries(cmp_phase1_log PRIVATE cmpc)
    add_test(NAME cmp_phase1_log COMMAND cmp_phase1_log)

    add_executable(cmp_phase1_store tests/phase1/store.c)
    target_link_libraries(cmp_phase1_store PRIVATE cmpc)
    add_test(NAME cmp_phase1_store COMMAND cmp_phase1_store)

    add_executable(cmp_phase1_utf8 tests/phase1/utf8.c)
    target_link_libraries(cmp_phase1_utf8 PRIVATE cmpc)
    add_test(NAME cmp_phase1_utf8 COMMAND cmp_phase1_utf8)

    add_executable(cmp_phase2_anim tests/phase2/anim.c)
    target_link_libraries(cmp_phase2_anim PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_anim COMMAND cmp_phase2_anim)

    add_executable(cmp_phase2_color tests/phase2/color.c)
    target_link_libraries(cmp_phase2_color PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_color COMMAND cmp_phase2_color)

    add_executable(cmp_phase2_cupertino_color tests/phase2/cupertino_color.c)
    target_link_libraries(cmp_phase2_cupertino_color PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase2_cupertino_color COMMAND cmp_phase2_cupertino_color)

    add_executable(cmp_phase2_layout tests/phase2/layout.c)
    target_link_libraries(cmp_phase2_layout PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_layout COMMAND cmp_phase2_layout)

    add_executable(cmp_phase2_layout_rtl tests/phase2/layout_rtl.c)
    target_link_libraries(cmp_phase2_layout_rtl PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_layout_rtl COMMAND cmp_phase2_layout_rtl)

    add_executable(cmp_phase2_math tests/phase2/math.c)
    target_link_libraries(cmp_phase2_math PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_math COMMAND cmp_phase2_math)

    add_executable(cmp_phase2_router tests/phase2/router.c)
    target_link_libraries(cmp_phase2_router PRIVATE m3 cmpc)
    add_test(NAME cmp_phase2_router COMMAND cmp_phase2_router)

    add_executable(cmp_phase3_event tests/phase3/event.c)
    target_link_libraries(cmp_phase3_event PRIVATE cmpc)
    add_test(NAME cmp_phase3_event COMMAND cmp_phase3_event)

    add_executable(cmp_phase3_gesture tests/phase3/gesture.c)
    target_link_libraries(cmp_phase3_gesture PRIVATE cmpc)
    add_test(NAME cmp_phase3_gesture COMMAND cmp_phase3_gesture)

    # MSVC 2005 does not support the C99 mixed declarations extensively used in Phase 3+ tests.
    # The library itself compiles strictly as C89 and works, but testing is restricted to Phase 0-2.
    if(NOT MSVC OR MSVC_VERSION GREATER_EQUAL 1500)
        add_executable(cmp_phase3_list tests/phase3/list.c)
        target_link_libraries(cmp_phase3_list PRIVATE cmpc)
        add_test(NAME cmp_phase3_list COMMAND cmp_phase3_list)

    add_executable(cmp_phase3_list_2 tests/phase3/list_2.c)
    target_link_libraries(cmp_phase3_list_2 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_2 COMMAND cmp_phase3_list_2)

    add_executable(cmp_phase3_list_3 tests/phase3/list_3.c)
    target_link_libraries(cmp_phase3_list_3 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_3 COMMAND cmp_phase3_list_3)

    add_executable(cmp_phase3_list_4 tests/phase3/list_4.c)
    target_link_libraries(cmp_phase3_list_4 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_4 COMMAND cmp_phase3_list_4)

    add_executable(cmp_phase3_list_5 tests/phase3/list_5.c)
    target_link_libraries(cmp_phase3_list_5 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_5 COMMAND cmp_phase3_list_5)

    add_executable(cmp_phase3_list_6 tests/phase3/list_6.c)
    target_link_libraries(cmp_phase3_list_6 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_6 COMMAND cmp_phase3_list_6)

    add_executable(cmp_phase3_list_7 tests/phase3/list_7.c)
    target_link_libraries(cmp_phase3_list_7 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_7 COMMAND cmp_phase3_list_7)

    add_executable(cmp_phase3_list_8 tests/phase3/list_8.c)
    target_link_libraries(cmp_phase3_list_8 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_8 COMMAND cmp_phase3_list_8)

    add_executable(cmp_phase3_list_9 tests/phase3/list_9.c)
    target_link_libraries(cmp_phase3_list_9 PRIVATE cmpc)
    add_test(NAME cmp_phase3_list_9 COMMAND cmp_phase3_list_9)

    add_executable(cmp_phase3_path tests/phase3/path.c)
    target_link_libraries(cmp_phase3_path PRIVATE cmpc)
    add_test(NAME cmp_phase3_path COMMAND cmp_phase3_path)

    add_executable(cmp_phase3_cupertino_shape tests/phase3/cupertino_shape.c)
    target_link_libraries(cmp_phase3_cupertino_shape PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase3_cupertino_shape COMMAND cmp_phase3_cupertino_shape)

    add_executable(cmp_phase3_render tests/phase3/render.c)
    target_link_libraries(cmp_phase3_render PRIVATE cmpc)
    add_test(NAME cmp_phase3_render COMMAND cmp_phase3_render)

    add_executable(cmp_phase3_scroll tests/phase3/scroll.c)
    target_link_libraries(cmp_phase3_scroll PRIVATE cmpc)
    add_test(NAME cmp_phase3_scroll COMMAND cmp_phase3_scroll)

    add_executable(cmp_phase3_tasks tests/phase3/tasks.c)
    target_link_libraries(cmp_phase3_tasks PRIVATE cmpc)
    add_test(NAME cmp_phase3_tasks COMMAND cmp_phase3_tasks)

    add_executable(cmp_phase3_text_field tests/phase3/text_field.c)
    target_link_libraries(cmp_phase3_text_field PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field COMMAND cmp_phase3_text_field)

    add_executable(cmp_phase3_text_field_10 tests/phase3/text_field_10.c)
    target_link_libraries(cmp_phase3_text_field_10 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_10 COMMAND cmp_phase3_text_field_10)

    add_executable(cmp_phase3_text_field_11 tests/phase3/text_field_11.c)
    target_link_libraries(cmp_phase3_text_field_11 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_11 COMMAND cmp_phase3_text_field_11)

    add_executable(cmp_phase3_text_field_12 tests/phase3/text_field_12.c)
    target_link_libraries(cmp_phase3_text_field_12 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_12 COMMAND cmp_phase3_text_field_12)

    add_executable(cmp_phase3_text_field_13 tests/phase3/text_field_13.c)
    target_link_libraries(cmp_phase3_text_field_13 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_13 COMMAND cmp_phase3_text_field_13)

    add_executable(cmp_phase3_text_field_14 tests/phase3/text_field_14.c)
    target_link_libraries(cmp_phase3_text_field_14 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_14 COMMAND cmp_phase3_text_field_14)

    add_executable(cmp_phase3_text_field_15 tests/phase3/text_field_15.c)
    target_link_libraries(cmp_phase3_text_field_15 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_15 COMMAND cmp_phase3_text_field_15)

    add_executable(cmp_phase3_text_field_16 tests/phase3/text_field_16.c)
    target_link_libraries(cmp_phase3_text_field_16 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_16 COMMAND cmp_phase3_text_field_16)

    add_executable(cmp_phase3_text_field_17 tests/phase3/text_field_17.c)
    target_link_libraries(cmp_phase3_text_field_17 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_17 COMMAND cmp_phase3_text_field_17)

    add_executable(cmp_phase3_text_field_18 tests/phase3/text_field_18.c)
    target_link_libraries(cmp_phase3_text_field_18 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_18 COMMAND cmp_phase3_text_field_18)

    add_executable(cmp_phase3_text_field_19 tests/phase3/text_field_19.c)
    target_link_libraries(cmp_phase3_text_field_19 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_19 COMMAND cmp_phase3_text_field_19)

    add_executable(cmp_phase3_text_field_2 tests/phase3/text_field_2.c)
    target_link_libraries(cmp_phase3_text_field_2 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_2 COMMAND cmp_phase3_text_field_2)

    add_executable(cmp_phase3_text_field_20 tests/phase3/text_field_20.c)
    target_link_libraries(cmp_phase3_text_field_20 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_20 COMMAND cmp_phase3_text_field_20)

    add_executable(cmp_phase3_text_field_21 tests/phase3/text_field_21.c)
    target_link_libraries(cmp_phase3_text_field_21 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_21 COMMAND cmp_phase3_text_field_21)

    add_executable(cmp_phase3_text_field_22 tests/phase3/text_field_22.c)
    target_link_libraries(cmp_phase3_text_field_22 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_22 COMMAND cmp_phase3_text_field_22)

    add_executable(cmp_phase3_text_field_23 tests/phase3/text_field_23.c)
    target_link_libraries(cmp_phase3_text_field_23 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_23 COMMAND cmp_phase3_text_field_23)

    add_executable(cmp_phase3_text_field_24 tests/phase3/text_field_24.c)
    target_link_libraries(cmp_phase3_text_field_24 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_24 COMMAND cmp_phase3_text_field_24)

    add_executable(cmp_phase3_text_field_25 tests/phase3/text_field_25.c)
    target_link_libraries(cmp_phase3_text_field_25 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_25 COMMAND cmp_phase3_text_field_25)

    add_executable(cmp_phase3_text_field_26 tests/phase3/text_field_26.c)
    target_link_libraries(cmp_phase3_text_field_26 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_26 COMMAND cmp_phase3_text_field_26)

    add_executable(cmp_phase3_text_field_27 tests/phase3/text_field_27.c)
    target_link_libraries(cmp_phase3_text_field_27 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_27 COMMAND cmp_phase3_text_field_27)

    add_executable(cmp_phase3_text_field_28 tests/phase3/text_field_28.c)
    target_link_libraries(cmp_phase3_text_field_28 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_28 COMMAND cmp_phase3_text_field_28)

    add_executable(cmp_phase3_text_field_30 tests/phase3/text_field_30.c)
    target_link_libraries(cmp_phase3_text_field_30 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_30 COMMAND cmp_phase3_text_field_30)

    add_executable(cmp_phase3_text_field_31 tests/phase3/text_field_31.c)
    target_link_libraries(cmp_phase3_text_field_31 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_31 COMMAND cmp_phase3_text_field_31)
    add_executable(cmp_phase3_text_field_29 tests/phase3/text_field_29.c)
    target_link_libraries(cmp_phase3_text_field_29 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_29 COMMAND cmp_phase3_text_field_29)

    add_executable(cmp_phase3_text_field_3 tests/phase3/text_field_3.c)
    target_link_libraries(cmp_phase3_text_field_3 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_3 COMMAND cmp_phase3_text_field_3)

    add_executable(cmp_phase3_text_field_4 tests/phase3/text_field_4.c)
    target_link_libraries(cmp_phase3_text_field_4 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_4 COMMAND cmp_phase3_text_field_4)

    add_executable(cmp_phase3_text_field_5 tests/phase3/text_field_5.c)
    target_link_libraries(cmp_phase3_text_field_5 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_5 COMMAND cmp_phase3_text_field_5)

    add_executable(cmp_phase3_text_field_6 tests/phase3/text_field_6.c)
    target_link_libraries(cmp_phase3_text_field_6 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_6 COMMAND cmp_phase3_text_field_6)

    add_executable(cmp_phase3_text_field_7 tests/phase3/text_field_7.c)
    target_link_libraries(cmp_phase3_text_field_7 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_7 COMMAND cmp_phase3_text_field_7)

    add_executable(cmp_phase3_text_field_8 tests/phase3/text_field_8.c)
    target_link_libraries(cmp_phase3_text_field_8 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_8 COMMAND cmp_phase3_text_field_8)

    add_executable(cmp_phase3_text_field_9 tests/phase3/text_field_9.c)
    target_link_libraries(cmp_phase3_text_field_9 PRIVATE cmpc)
    add_test(NAME cmp_phase3_text_field_9 COMMAND cmp_phase3_text_field_9)

    add_executable(cmp_phase4_null_backend tests/phase4/null_backend.c)
    target_link_libraries(cmp_phase4_null_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_null_backend COMMAND cmp_phase4_null_backend)

    add_executable(cmp_phase4_android_backend tests/phase4/android_backend.c)
    target_link_libraries(cmp_phase4_android_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_android_backend COMMAND cmp_phase4_android_backend)

    add_executable(cmp_phase4_cocoa_backend tests/phase4/cocoa_backend.c)
    target_link_libraries(cmp_phase4_cocoa_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_cocoa_backend COMMAND cmp_phase4_cocoa_backend)

    add_executable(cmp_phase4_gtk4_backend tests/phase4/gtk4_backend.c)
    target_link_libraries(cmp_phase4_gtk4_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_gtk4_backend COMMAND cmp_phase4_gtk4_backend)

    add_executable(cmp_phase4_ios_backend tests/phase4/ios_backend.c)
    target_link_libraries(cmp_phase4_ios_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_ios_backend COMMAND cmp_phase4_ios_backend)

    add_executable(cmp_phase4_sdl3_backend tests/phase4/sdl3_backend.c)
    target_link_libraries(cmp_phase4_sdl3_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_sdl3_backend COMMAND cmp_phase4_sdl3_backend)

    add_executable(cmp_phase4_web_backend tests/phase4/web_backend.c)
    target_link_libraries(cmp_phase4_web_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_web_backend COMMAND cmp_phase4_web_backend)

    add_executable(cmp_phase4_win32_backend tests/phase4/win32_backend.c)
    target_link_libraries(cmp_phase4_win32_backend PRIVATE cmpc)
    add_test(NAME cmp_phase4_win32_backend COMMAND cmp_phase4_win32_backend)

    add_executable(cmp_phase5_f2_tests tests/phase5/f2_tests.c)
    target_link_libraries(cmp_phase5_f2_tests PRIVATE f2 cmpc)
    add_test(NAME cmp_phase5_f2_tests COMMAND cmp_phase5_f2_tests)

    add_executable(cmp_phase5_adaptive tests/phase5/adaptive.c)
    target_link_libraries(cmp_phase5_adaptive PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_adaptive COMMAND cmp_phase5_adaptive)

    add_executable(cmp_phase5_app_bar tests/phase5/app_bar.c)
    target_link_libraries(cmp_phase5_app_bar PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_app_bar COMMAND cmp_phase5_app_bar)

    add_executable(cmp_phase5_badge tests/phase5/badge.c)
    target_link_libraries(cmp_phase5_badge PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_badge COMMAND cmp_phase5_badge)

    add_executable(cmp_phase5_buttons tests/phase5/buttons.c)
    target_link_libraries(cmp_phase5_buttons PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_buttons COMMAND cmp_phase5_buttons)

    add_executable(cmp_phase5_cards tests/phase5/cards.c)
    target_link_libraries(cmp_phase5_cards PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_cards COMMAND cmp_phase5_cards)

    add_executable(cmp_phase5_carousel tests/phase5/carousel.c)
    target_link_libraries(cmp_phase5_carousel PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel COMMAND cmp_phase5_carousel)

    add_executable(cmp_phase5_carousel_2 tests/phase5/carousel_2.c)
    target_link_libraries(cmp_phase5_carousel_2 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_2 COMMAND cmp_phase5_carousel_2)

    add_executable(cmp_phase5_carousel_3 tests/phase5/carousel_3.c)
    target_link_libraries(cmp_phase5_carousel_3 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_3 COMMAND cmp_phase5_carousel_3)

    add_executable(cmp_phase5_carousel_4 tests/phase5/carousel_4.c)
    target_link_libraries(cmp_phase5_carousel_4 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_4 COMMAND cmp_phase5_carousel_4)

    add_executable(cmp_phase5_carousel_5 tests/phase5/carousel_5.c)
    target_link_libraries(cmp_phase5_carousel_5 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_5 COMMAND cmp_phase5_carousel_5)

    add_executable(cmp_phase5_carousel_6 tests/phase5/carousel_6.c)
    target_link_libraries(cmp_phase5_carousel_6 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_6 COMMAND cmp_phase5_carousel_6)

    add_executable(cmp_phase5_carousel_7 tests/phase5/carousel_7.c)
    target_link_libraries(cmp_phase5_carousel_7 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_carousel_7 COMMAND cmp_phase5_carousel_7)

    add_executable(cmp_phase5_chips tests/phase5/chips.c)
    target_link_libraries(cmp_phase5_chips PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_chips COMMAND cmp_phase5_chips)

    add_executable(cmp_phase5_date_picker tests/phase5/date_picker.c)
    target_link_libraries(cmp_phase5_date_picker PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_date_picker COMMAND cmp_phase5_date_picker)

    add_executable(cmp_phase5_dialogs tests/phase5/dialogs.c)
    target_link_libraries(cmp_phase5_dialogs PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_dialogs COMMAND cmp_phase5_dialogs)

    add_executable(cmp_phase5_stepper tests/phase5/stepper.c)
    target_link_libraries(cmp_phase5_stepper PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_stepper COMMAND cmp_phase5_stepper)

    add_executable(cmp_phase5_data_table tests/phase5/data_table.c)
    target_link_libraries(cmp_phase5_data_table PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_data_table COMMAND cmp_phase5_data_table)

    add_executable(cmp_phase5_divider tests/phase5/divider.c)
    target_link_libraries(cmp_phase5_divider PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_divider COMMAND cmp_phase5_divider)

    add_executable(cmp_phase5_extras tests/phase5/extras.c)
    target_link_libraries(cmp_phase5_extras PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_extras COMMAND cmp_phase5_extras)

    add_executable(cmp_phase5_icons tests/phase5/icons.c)
    target_link_libraries(cmp_phase5_icons PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_icons COMMAND cmp_phase5_icons)

    add_executable(cmp_phase5_list tests/phase5/list.c)
    target_link_libraries(cmp_phase5_list PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_list COMMAND cmp_phase5_list)

    add_executable(cmp_phase5_list_2 tests/phase5/list_2.c)
    target_link_libraries(cmp_phase5_list_2 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_list_2 COMMAND cmp_phase5_list_2)

    add_executable(cmp_phase5_list_3 tests/phase5/list_3.c)
    target_link_libraries(cmp_phase5_list_3 PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_list_3 COMMAND cmp_phase5_list_3)

    add_executable(cmp_phase5_menus tests/phase5/menus.c)
    target_link_libraries(cmp_phase5_menus PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_menus COMMAND cmp_phase5_menus)

    add_executable(cmp_phase5_motion tests/phase5/motion.c)
    target_link_libraries(cmp_phase5_motion PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_motion COMMAND cmp_phase5_motion)

    add_executable(cmp_phase5_navigation tests/phase5/navigation.c)
    target_link_libraries(cmp_phase5_navigation PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_navigation COMMAND cmp_phase5_navigation)

    add_executable(cmp_phase5_progress tests/phase5/progress.c)
    target_link_libraries(cmp_phase5_progress PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_progress COMMAND cmp_phase5_progress)

    add_executable(cmp_phase5_pull_to_refresh tests/phase5/pull_to_refresh.c)
    target_link_libraries(cmp_phase5_pull_to_refresh PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_pull_to_refresh COMMAND cmp_phase5_pull_to_refresh)

    add_executable(cmp_phase5_scaffold tests/phase5/scaffold.c)
    target_link_libraries(cmp_phase5_scaffold PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_scaffold COMMAND cmp_phase5_scaffold)

    add_executable(cmp_phase5_search tests/phase5/search.c)
    target_link_libraries(cmp_phase5_search PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_search COMMAND cmp_phase5_search)

    add_executable(cmp_phase5_selection tests/phase5/selection.c)
    target_link_libraries(cmp_phase5_selection PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_selection COMMAND cmp_phase5_selection)

    add_executable(cmp_phase5_shape tests/phase5/shape.c)
    target_link_libraries(cmp_phase5_shape PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_shape COMMAND cmp_phase5_shape)

    add_executable(cmp_phase5_sheet tests/phase5/sheet.c)
    target_link_libraries(cmp_phase5_sheet PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_sheet COMMAND cmp_phase5_sheet)

    add_executable(cmp_phase5_side_sheet tests/phase5/side_sheet.c)
    target_link_libraries(cmp_phase5_side_sheet PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_side_sheet COMMAND cmp_phase5_side_sheet)

    add_executable(cmp_phase5_tabs tests/phase5/tabs.c)
    target_link_libraries(cmp_phase5_tabs PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_tabs COMMAND cmp_phase5_tabs)

    add_executable(cmp_phase5_text tests/phase5/text.c)
    target_link_libraries(cmp_phase5_text PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_text COMMAND cmp_phase5_text)

    add_executable(cmp_phase5_text_field tests/phase5/text_field.c)
    target_link_libraries(cmp_phase5_text_field PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_text_field COMMAND cmp_phase5_text_field)

    add_executable(cmp_phase5_time_picker tests/phase5/time_picker.c)
    target_link_libraries(cmp_phase5_time_picker PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_time_picker COMMAND cmp_phase5_time_picker)

    add_executable(cmp_phase5_tooltip tests/phase5/tooltip.c)
    target_link_libraries(cmp_phase5_tooltip PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_tooltip COMMAND cmp_phase5_tooltip)

    add_executable(cmp_phase5_typography tests/phase5/typography.c)
    target_link_libraries(cmp_phase5_typography PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_typography COMMAND cmp_phase5_typography)

    add_executable(cmp_phase5_cupertino_typography tests/phase5/cupertino_typography.c)
    target_link_libraries(cmp_phase5_cupertino_typography PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_typography COMMAND cmp_phase5_cupertino_typography)

    
    add_executable(cmp_phase5_cupertino_switch tests/phase5/cupertino_switch.c)
    target_link_libraries(cmp_phase5_cupertino_switch PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_switch COMMAND cmp_phase5_cupertino_switch)

    add_executable(cmp_phase5_cupertino_slider tests/phase5/cupertino_slider.c)
    target_link_libraries(cmp_phase5_cupertino_slider PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_slider COMMAND cmp_phase5_cupertino_slider)

    add_executable(cmp_phase5_cupertino_activity_indicator tests/phase5/cupertino_activity_indicator.c)
    target_link_libraries(cmp_phase5_cupertino_activity_indicator PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_activity_indicator COMMAND cmp_phase5_cupertino_activity_indicator)

    
    add_executable(cmp_phase5_cupertino_segmented_control tests/phase5/cupertino_segmented_control.c)
    target_link_libraries(cmp_phase5_cupertino_segmented_control PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_segmented_control COMMAND cmp_phase5_cupertino_segmented_control)

    add_executable(cmp_phase5_cupertino_progress_bar tests/phase5/cupertino_progress_bar.c)
    target_link_libraries(cmp_phase5_cupertino_progress_bar PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_progress_bar COMMAND cmp_phase5_cupertino_progress_bar)

    add_executable(cmp_phase5_cupertino_stepper tests/phase5/cupertino_stepper.c)
    target_link_libraries(cmp_phase5_cupertino_stepper PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_stepper COMMAND cmp_phase5_cupertino_stepper)

    
    add_executable(cmp_phase5_cupertino_mac_traffic_lights tests/phase5/cupertino_mac_traffic_lights.c)
    target_link_libraries(cmp_phase5_cupertino_mac_traffic_lights PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_mac_traffic_lights COMMAND cmp_phase5_cupertino_mac_traffic_lights)

    add_executable(cmp_phase5_cupertino_list_section tests/phase5/cupertino_list_section.c)
    target_link_libraries(cmp_phase5_cupertino_list_section PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_list_section COMMAND cmp_phase5_cupertino_list_section)

    add_executable(cmp_phase5_cupertino_picker tests/phase5/cupertino_picker.c)
    target_link_libraries(cmp_phase5_cupertino_picker PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_picker COMMAND cmp_phase5_cupertino_picker)

    
    add_executable(cmp_phase5_cupertino_mac_sidebar tests/phase5/cupertino_mac_sidebar.c)
    target_link_libraries(cmp_phase5_cupertino_mac_sidebar PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_mac_sidebar COMMAND cmp_phase5_cupertino_mac_sidebar)

    add_executable(cmp_phase5_cupertino_tv_card tests/phase5/cupertino_tv_card.c)
    target_link_libraries(cmp_phase5_cupertino_tv_card PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_tv_card COMMAND cmp_phase5_cupertino_tv_card)

    add_executable(cmp_phase5_cupertino_watch_button tests/phase5/cupertino_watch_button.c)
    target_link_libraries(cmp_phase5_cupertino_watch_button PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_watch_button COMMAND cmp_phase5_cupertino_watch_button)

    
    add_executable(cmp_phase5_cupertino_mac_toolbar tests/phase5/cupertino_mac_toolbar.c)
    target_link_libraries(cmp_phase5_cupertino_mac_toolbar PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_mac_toolbar COMMAND cmp_phase5_cupertino_mac_toolbar)

    add_executable(cmp_phase5_cupertino_focus_ring tests/phase5/cupertino_focus_ring.c)
    target_link_libraries(cmp_phase5_cupertino_focus_ring PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_focus_ring COMMAND cmp_phase5_cupertino_focus_ring)

    add_executable(cmp_phase5_cupertino_date_picker tests/phase5/cupertino_date_picker.c)
    target_link_libraries(cmp_phase5_cupertino_date_picker PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_date_picker COMMAND cmp_phase5_cupertino_date_picker)

    add_executable(cmp_phase5_cupertino_detent_sheet tests/phase5/cupertino_detent_sheet.c)
    target_link_libraries(cmp_phase5_cupertino_detent_sheet PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_detent_sheet COMMAND cmp_phase5_cupertino_detent_sheet)

    add_executable(cmp_phase5_cupertino_text_field tests/phase5/cupertino_text_field.c)
    target_link_libraries(cmp_phase5_cupertino_text_field PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_text_field COMMAND cmp_phase5_cupertino_text_field)

    add_executable(cmp_phase5_cupertino_alert tests/phase5/cupertino_alert.c)
    target_link_libraries(cmp_phase5_cupertino_alert PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_alert COMMAND cmp_phase5_cupertino_alert)

    add_executable(cmp_phase5_cupertino_button tests/phase5/cupertino_button.c)
    target_link_libraries(cmp_phase5_cupertino_button PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_button COMMAND cmp_phase5_cupertino_button)

    add_executable(cmp_phase5_cupertino_blur tests/phase5/cupertino_blur.c)
    target_link_libraries(cmp_phase5_cupertino_blur PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_blur COMMAND cmp_phase5_cupertino_blur)

    add_executable(cmp_phase5_cupertino_haptics tests/phase5/cupertino_haptics.c)
    target_link_libraries(cmp_phase5_cupertino_haptics PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_haptics COMMAND cmp_phase5_cupertino_haptics)

    add_executable(cmp_phase5_cupertino_navigation_bar tests/phase5/cupertino_navigation_bar.c)
    target_link_libraries(cmp_phase5_cupertino_navigation_bar PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_navigation_bar COMMAND cmp_phase5_cupertino_navigation_bar)

    add_executable(cmp_phase5_cupertino_tab_bar tests/phase5/cupertino_tab_bar.c)
    target_link_libraries(cmp_phase5_cupertino_tab_bar PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_tab_bar COMMAND cmp_phase5_cupertino_tab_bar)

    add_executable(cmp_phase5_cupertino_action_sheet tests/phase5/cupertino_action_sheet.c)
    target_link_libraries(cmp_phase5_cupertino_action_sheet PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_action_sheet COMMAND cmp_phase5_cupertino_action_sheet)

    add_executable(cmp_phase5_cupertino_pull_to_refresh tests/phase5/cupertino_pull_to_refresh.c)
    target_link_libraries(cmp_phase5_cupertino_pull_to_refresh PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_pull_to_refresh COMMAND cmp_phase5_cupertino_pull_to_refresh)

    add_executable(cmp_phase5_cupertino_menu tests/phase5/cupertino_menu.c)
    target_link_libraries(cmp_phase5_cupertino_menu PRIVATE cupertino cmpc)
    add_test(NAME cmp_phase5_cupertino_menu COMMAND cmp_phase5_cupertino_menu)

    add_executable(cmp_phase5_visuals tests/phase5/visuals.c)
    target_link_libraries(cmp_phase5_visuals PRIVATE m3 cmpc)
    add_test(NAME cmp_phase5_visuals COMMAND cmp_phase5_visuals)

    add_executable(cmp_phase6_a11y tests/phase6/a11y.c)
    target_link_libraries(cmp_phase6_a11y PRIVATE cmpc)
    add_test(NAME cmp_phase6_a11y COMMAND cmp_phase6_a11y)

    add_executable(cmp_phase6_audio tests/phase6/audio.c)
    target_link_libraries(cmp_phase6_audio PRIVATE cmpc)
    add_test(NAME cmp_phase6_audio COMMAND cmp_phase6_audio)

    add_executable(cmp_phase6_camera tests/phase6/camera.c)
    target_link_libraries(cmp_phase6_camera PRIVATE cmpc)
    add_test(NAME cmp_phase6_camera COMMAND cmp_phase6_camera)

    add_executable(cmp_phase6_image tests/phase6/image.c)
    target_link_libraries(cmp_phase6_image PRIVATE cmpc)
    add_test(NAME cmp_phase6_image COMMAND cmp_phase6_image)

    add_executable(cmp_phase6_network tests/phase6/network.c)
    target_link_libraries(cmp_phase6_network PRIVATE cmpc)
    add_test(NAME cmp_phase6_network COMMAND cmp_phase6_network)

    add_executable(cmp_phase6_predictive tests/phase6/predictive.c)
    target_link_libraries(cmp_phase6_predictive PRIVATE cmpc)
    add_test(NAME cmp_phase6_predictive COMMAND cmp_phase6_predictive)

    add_executable(cmp_phase6_storage tests/phase6/storage.c)
    target_link_libraries(cmp_phase6_storage PRIVATE cmpc)
    add_test(NAME cmp_phase6_storage COMMAND cmp_phase6_storage)

    add_executable(cmp_phase6_video tests/phase6/video.c)
    target_link_libraries(cmp_phase6_video PRIVATE cmpc)
    add_test(NAME cmp_phase6_video COMMAND cmp_phase6_video)

    endif()

endif()

if(CMP_ENABLE_GTK4)
    add_executable(screenshot_tool tools/screenshot_tool.c)
    target_link_libraries(screenshot_tool PRIVATE m3 cmpc)
endif()
