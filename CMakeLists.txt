cmake_minimum_required(VERSION 3.16)

project(LibM3C VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

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
option(M3_ENABLE_PACKAGING "Enable packaging targets" OFF)
option(M3_BUILD_WEB_APP "Build minimal web app target" OFF)
option(M3_BUILD_PACKAGING_STUB "Build packaging stub executable" OFF)

add_library(m3_headers INTERFACE)
target_include_directories(m3_headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
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

add_library(m3
    src/core/m3_core.c
    src/core/m3_object.c
    src/core/m3_arena.c
    src/core/m3_utf8.c
    src/core/m3_log.c
    src/core/m3_store.c
    src/core/m3_storage.c
    src/core/m3_i18n.c
    src/core/m3_math.c
    src/core/m3_color.c
    src/core/m3_layout.c
    src/core/m3_a11y.c
    src/core/m3_anim.c
    src/core/m3_router.c
    src/core/m3_predictive.c
    src/core/m3_path.c
    src/core/m3_render.c
    src/core/m3_event.c
    src/core/m3_gesture.c
    src/core/m3_scroll.c
    src/core/m3_tasks.c
    src/core/m3_visuals.c
    src/core/m3_text.c
    src/core/m3_icon.c
    src/core/m3_extras.c
    src/core/m3_text_field.c
    src/core/m3_list.c
    src/core/m3_navigation.c
    src/core/m3_app_bar.c
    src/core/m3_tabs.c
    src/core/m3_dialogs.c
    src/core/m3_sheet.c
    src/core/m3_scaffold.c
    src/core/m3_menu.c
    src/core/m3_button.c
    src/core/m3_card.c
    src/core/m3_chip.c
    src/core/m3_selection.c
    src/core/m3_progress.c
    src/core/m3_date_picker.c
    src/core/m3_time_picker.c
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
add_library(m3::m3 ALIAS m3)
target_include_directories(m3 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if (NOT WIN32)
    target_link_libraries(m3 PUBLIC m)
endif()
if(BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(m3 PRIVATE M3_BUILD_DLL)
    target_compile_definitions(m3 INTERFACE M3_USE_DLL)
endif()

set(M3_HAS_SDL3 OFF)
set(M3_HAS_SDL3_TTF OFF)
set(M3_HAS_GTK4 OFF)
set(M3_HAS_LIBCURL OFF)
if(M3_ENABLE_SDL3)
    find_package(SDL3 CONFIG QUIET)
    if(SDL3_FOUND)
        set(M3_HAS_SDL3 ON)
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
            set(M3_HAS_SDL3_TTF ON)
            target_link_libraries(m3 PUBLIC SDL3_ttf::SDL3_ttf)
            target_compile_definitions(m3 PUBLIC M3_SDL3_TTF_AVAILABLE)
        elseif(TARGET SDL3_ttf::SDL3_ttf-static)
            set(M3_HAS_SDL3_TTF ON)
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
            set(M3_HAS_GTK4 ON)
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
        set(M3_HAS_LIBCURL ON)
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
        set(M3_ANDROID_API_LEVEL 0)
        if(DEFINED ANDROID_NATIVE_API_LEVEL)
            set(M3_ANDROID_API_LEVEL ${ANDROID_NATIVE_API_LEVEL})
        elseif(DEFINED ANDROID_PLATFORM)
            string(REGEX REPLACE "android-" "" M3_ANDROID_API_LEVEL "${ANDROID_PLATFORM}")
        elseif(CMAKE_SYSTEM_VERSION)
            set(M3_ANDROID_API_LEVEL ${CMAKE_SYSTEM_VERSION})
        endif()
        if(M3_ANDROID_API_LEVEL GREATER_EQUAL 24)
            find_library(M3_ANDROID_CAMERA2_LIB camera2ndk)
            find_library(M3_ANDROID_MEDIA_LIB mediandk)
            if(M3_ANDROID_CAMERA2_LIB AND M3_ANDROID_MEDIA_LIB)
                target_link_libraries(m3 PUBLIC ${M3_ANDROID_CAMERA2_LIB} ${M3_ANDROID_MEDIA_LIB})
            else()
                message(WARNING "Android camera2ndk/mediandk not found; camera support disabled.")
            endif()
        else()
            message(WARNING "Android API level < 24; camera2ndk/mediandk not linked.")
        endif()
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

set_target_properties(m3 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(M3_BUILD_WEB_APP)
    add_executable(m3_web_app packaging/web/app.c)
    target_link_libraries(m3_web_app PRIVATE m3)
    m3_apply_c_settings(m3_web_app)
endif()

if(M3_BUILD_PACKAGING_STUB)
    add_executable(m3_packaging_stub packaging/desktop/stub.c)
    target_link_libraries(m3_packaging_stub PRIVATE m3)
    set_target_properties(m3_packaging_stub PROPERTIES OUTPUT_NAME "libm3c-stub")
    m3_apply_c_settings(m3_packaging_stub)
    install(TARGETS m3_packaging_stub
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

include(CMakePackageConfigHelpers)

set(LIBM3C_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/LibM3C")

install(TARGETS m3
    EXPORT LibM3CTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibM3CConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LibM3CConfig.cmake
    INSTALL_DESTINATION ${LIBM3C_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LibM3CConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LibM3CConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LibM3CConfigVersion.cmake
    DESTINATION ${LIBM3C_INSTALL_CMAKEDIR}
)

install(EXPORT LibM3CTargets
    NAMESPACE m3::
    DESTINATION ${LIBM3C_INSTALL_CMAKEDIR}
)

export(EXPORT LibM3CTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/LibM3CTargets.cmake
    NAMESPACE m3::
)

if(M3_ENABLE_PACKAGING)
    include(cmake/M3Packaging.cmake)
endif()

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
            --exclude-noncode-lines
            --merge-lines
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

    add_executable(m3_phase1_i18n tests/phase1/i18n.c)
    target_link_libraries(m3_phase1_i18n PRIVATE m3)
    target_compile_definitions(m3_phase1_i18n PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase1_i18n)
    add_test(NAME m3_phase1_i18n COMMAND m3_phase1_i18n)

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

    add_executable(m3_phase2_layout_rtl tests/phase2/layout_rtl.c)
    target_link_libraries(m3_phase2_layout_rtl PRIVATE m3)
    target_compile_definitions(m3_phase2_layout_rtl PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase2_layout_rtl)
    add_test(NAME m3_phase2_layout_rtl COMMAND m3_phase2_layout_rtl)

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

    add_executable(m3_phase3_path tests/phase3/path.c)
    target_link_libraries(m3_phase3_path PRIVATE m3)
    target_compile_definitions(m3_phase3_path PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_path)
    add_test(NAME m3_phase3_path COMMAND m3_phase3_path)

    add_executable(m3_phase3_event tests/phase3/event.c)
    target_link_libraries(m3_phase3_event PRIVATE m3)
    target_compile_definitions(m3_phase3_event PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_event)
    add_test(NAME m3_phase3_event COMMAND m3_phase3_event)

    add_executable(m3_phase3_gesture tests/phase3/gesture.c)
    target_link_libraries(m3_phase3_gesture PRIVATE m3)
    target_compile_definitions(m3_phase3_gesture PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_gesture)
    add_test(NAME m3_phase3_gesture COMMAND m3_phase3_gesture)

    add_executable(m3_phase3_scroll tests/phase3/scroll.c)
    target_link_libraries(m3_phase3_scroll PRIVATE m3)
    target_compile_definitions(m3_phase3_scroll PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase3_scroll)
    add_test(NAME m3_phase3_scroll COMMAND m3_phase3_scroll)

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

    add_executable(m3_phase5_icons tests/phase5/icons.c)
    target_link_libraries(m3_phase5_icons PRIVATE m3)
    target_compile_definitions(m3_phase5_icons PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_icons)
    add_test(NAME m3_phase5_icons COMMAND m3_phase5_icons)

    add_executable(m3_phase5_extras tests/phase5/extras.c)
    target_link_libraries(m3_phase5_extras PRIVATE m3)
    target_compile_definitions(m3_phase5_extras PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_extras)
    add_test(NAME m3_phase5_extras COMMAND m3_phase5_extras)

    add_executable(m3_phase5_buttons tests/phase5/buttons.c)
    target_link_libraries(m3_phase5_buttons PRIVATE m3)
    target_compile_definitions(m3_phase5_buttons PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_buttons)
    add_test(NAME m3_phase5_buttons COMMAND m3_phase5_buttons)

    add_executable(m3_phase5_cards tests/phase5/cards.c)
    target_link_libraries(m3_phase5_cards PRIVATE m3)
    target_compile_definitions(m3_phase5_cards PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_cards)
    add_test(NAME m3_phase5_cards COMMAND m3_phase5_cards)

    add_executable(m3_phase5_chips tests/phase5/chips.c)
    target_link_libraries(m3_phase5_chips PRIVATE m3)
    target_compile_definitions(m3_phase5_chips PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_chips)
    add_test(NAME m3_phase5_chips COMMAND m3_phase5_chips)

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

    add_executable(m3_phase5_app_bar tests/phase5/app_bar.c)
    target_link_libraries(m3_phase5_app_bar PRIVATE m3)
    target_compile_definitions(m3_phase5_app_bar PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_app_bar)
    add_test(NAME m3_phase5_app_bar COMMAND m3_phase5_app_bar)

    add_executable(m3_phase5_tabs tests/phase5/tabs.c)
    target_link_libraries(m3_phase5_tabs PRIVATE m3)
    target_compile_definitions(m3_phase5_tabs PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_tabs)
    add_test(NAME m3_phase5_tabs COMMAND m3_phase5_tabs)

    add_executable(m3_phase5_menus tests/phase5/menus.c)
    target_link_libraries(m3_phase5_menus PRIVATE m3)
    target_compile_definitions(m3_phase5_menus PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_menus)
    add_test(NAME m3_phase5_menus COMMAND m3_phase5_menus)

    add_executable(m3_phase5_dialogs tests/phase5/dialogs.c)
    target_link_libraries(m3_phase5_dialogs PRIVATE m3)
    target_compile_definitions(m3_phase5_dialogs PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_dialogs)
    add_test(NAME m3_phase5_dialogs COMMAND m3_phase5_dialogs)

    add_executable(m3_phase5_sheet tests/phase5/sheet.c)
    target_link_libraries(m3_phase5_sheet PRIVATE m3)
    target_compile_definitions(m3_phase5_sheet PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_sheet)
    add_test(NAME m3_phase5_sheet COMMAND m3_phase5_sheet)

    add_executable(m3_phase5_scaffold tests/phase5/scaffold.c)
    target_link_libraries(m3_phase5_scaffold PRIVATE m3)
    target_compile_definitions(m3_phase5_scaffold PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase5_scaffold)
    add_test(NAME m3_phase5_scaffold COMMAND m3_phase5_scaffold)

add_executable(m3_phase5_progress tests/phase5/progress.c)
target_link_libraries(m3_phase5_progress PRIVATE m3)
target_compile_definitions(m3_phase5_progress PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase5_progress)
add_test(NAME m3_phase5_progress COMMAND m3_phase5_progress)

add_executable(m3_phase5_date_picker tests/phase5/date_picker.c)
target_link_libraries(m3_phase5_date_picker PRIVATE m3)
target_compile_definitions(m3_phase5_date_picker PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase5_date_picker)
add_test(NAME m3_phase5_date_picker COMMAND m3_phase5_date_picker)

add_executable(m3_phase5_time_picker tests/phase5/time_picker.c)
target_link_libraries(m3_phase5_time_picker PRIVATE m3)
target_compile_definitions(m3_phase5_time_picker PRIVATE M3_TESTING)
m3_apply_c_settings(m3_phase5_time_picker)
add_test(NAME m3_phase5_time_picker COMMAND m3_phase5_time_picker)

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
    add_executable(m3_phase6_a11y tests/phase6/a11y.c)
    target_link_libraries(m3_phase6_a11y PRIVATE m3)
    target_compile_definitions(m3_phase6_a11y PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase6_a11y)
    add_test(NAME m3_phase6_a11y COMMAND m3_phase6_a11y)

    add_executable(m3_phase6_predictive tests/phase6/predictive.c)
    target_link_libraries(m3_phase6_predictive PRIVATE m3)
    target_compile_definitions(m3_phase6_predictive PRIVATE M3_TESTING)
    m3_apply_c_settings(m3_phase6_predictive)
    add_test(NAME m3_phase6_predictive COMMAND m3_phase6_predictive)
endif()
