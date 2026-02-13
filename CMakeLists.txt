cmake_minimum_required(VERSION 3.16)

project(LibCMPC VERSION 0.1.0 LANGUAGES C)

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

set(CMP_WARNINGS_C
    $<$<C_COMPILER_ID:MSVC>:/W4>
    $<$<C_COMPILER_ID:MSVC>:$<$<BOOL:${CMP_WARNINGS_AS_ERRORS}>:/WX>>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wextra>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wpedantic>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:$<$<BOOL:${CMP_WARNINGS_AS_ERRORS}>:-Werror>>
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
    src/m3/m3_navigation.c
    src/m3/m3_app_bar.c
    src/m3/m3_tabs.c
    src/m3/m3_dialogs.c
    src/m3/m3_sheet.c
    src/m3/m3_scaffold.c
    src/m3/m3_menu.c
    src/m3/m3_button.c
    src/m3/m3_card.c
    src/m3/m3_chip.c
    src/m3/m3_selection.c
    src/m3/m3_progress.c
    src/m3/m3_date_picker.c
    src/m3/m3_time_picker.c
)
add_library(m3::m3 ALIAS m3)
target_include_directories(m3 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(m3 PUBLIC cmpc)

if(CMP_ENABLE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(cmpc PRIVATE -O0 -g --coverage)
        target_link_options(cmpc PRIVATE --coverage)
        target_compile_options(m3 PRIVATE -O0 -g --coverage)
        target_link_options(m3 PRIVATE --coverage)
    else()
        message(FATAL_ERROR "CMP_ENABLE_COVERAGE is only supported with GCC or Clang.")
    endif()
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
if(CMP_ENABLE_SDL3)
    find_package(SDL3 CONFIG QUIET)
    if(SDL3_FOUND)
        set(CMP_HAS_SDL3 ON)
        target_link_libraries(m3 PUBLIC SDL3::SDL3)
        target_compile_definitions(m3 PUBLIC CMP_SDL3_AVAILABLE)
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
            target_link_libraries(m3 PUBLIC SDL3_ttf::SDL3_ttf)
            target_compile_definitions(m3 PUBLIC CMP_SDL3_TTF_AVAILABLE)
        elseif(TARGET SDL3_ttf::SDL3_ttf-static)
            set(CMP_HAS_SDL3_TTF ON)
            target_link_libraries(m3 PUBLIC SDL3_ttf::SDL3_ttf-static)
            target_compile_definitions(m3 PUBLIC CMP_SDL3_TTF_AVAILABLE)
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
            target_link_libraries(m3 PUBLIC PkgConfig::GTK4)
            target_compile_definitions(m3 PUBLIC CMP_GTK4_AVAILABLE)
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
            target_link_libraries(m3 PUBLIC CURL::libcurl)
        else()
            if(CURL_INCLUDE_DIRS)
                target_include_directories(m3 PUBLIC ${CURL_INCLUDE_DIRS})
            endif()
            if(CURL_LIBRARIES)
                target_link_libraries(m3 PUBLIC ${CURL_LIBRARIES})
            endif()
        endif()
        target_compile_definitions(m3 PUBLIC CMP_LIBCURL_AVAILABLE)
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
                target_sources(m3 PRIVATE src/backend/cocoa/cmp_backend_cocoa.m)
                target_link_libraries(m3 PUBLIC ${COCOA_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${CORETEXT_FRAMEWORK}
                    ${AVFOUNDATION_FRAMEWORK} ${COREMEDIA_FRAMEWORK} ${COREVIDEO_FRAMEWORK} ${CFNETWORK_FRAMEWORK})
                target_compile_definitions(m3 PUBLIC CMP_COCOA_AVAILABLE)
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
if(CMP_ENABLE_WIN32 AND WIN32)
    target_compile_definitions(m3 PUBLIC
        CMP_WIN32_AVAILABLE
        _WIN32_WINNT=0x0600
        WINVER=0x0600
    )
    target_link_libraries(m3 PUBLIC user32 gdi32 winhttp)
endif()
if(CMP_ENABLE_WEB)
    if(EMSCRIPTEN)
        target_compile_definitions(m3 PUBLIC CMP_WEB_AVAILABLE)
        if(CMP_ENABLE_WEBGPU)
            target_compile_definitions(m3 PUBLIC CMP_WEBGPU_AVAILABLE)
            target_link_options(m3 PUBLIC "-sUSE_WEBGPU=1")
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
        target_compile_definitions(m3 PUBLIC CMP_ANDROID_AVAILABLE)
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
                target_link_libraries(m3 PUBLIC ${CMP_ANDROID_CAMERA2_LIB} ${CMP_ANDROID_MEDIA_LIB})
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

if(BUILD_TESTING)
    target_compile_definitions(cmpc PRIVATE CMP_TESTING)
    target_compile_definitions(m3 PRIVATE CMP_TESTING)

    add_executable(cmp_phase0_compile tests/phase0/compile.c)
    target_link_libraries(cmp_phase0_compile PRIVATE cmp_headers)
    cmp_apply_c_settings(cmp_phase0_compile)

    add_test(NAME cmp_phase0_compile COMMAND cmp_phase0_compile)

    add_executable(cmp_phase1_allocator tests/phase1/allocator.c)
    target_link_libraries(cmp_phase1_allocator PRIVATE cmpc)
    cmp_apply_c_settings(cmp_phase1_allocator)
    add_test(NAME cmp_phase1_allocator COMMAND cmp_phase1_allocator)

    add_executable(cmp_phase1_arena tests/phase1/arena.c)
    target_link_libraries(cmp_phase1_arena PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_arena PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_arena)
    add_test(NAME cmp_phase1_arena COMMAND cmp_phase1_arena)

    add_executable(cmp_phase1_handle_system tests/phase1/handle_system.c)
    target_link_libraries(cmp_phase1_handle_system PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_handle_system PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_handle_system)
    add_test(NAME cmp_phase1_handle_system COMMAND cmp_phase1_handle_system)

    add_executable(cmp_phase1_utf8 tests/phase1/utf8.c)
    target_link_libraries(cmp_phase1_utf8 PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_utf8 PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_utf8)
    add_test(NAME cmp_phase1_utf8 COMMAND cmp_phase1_utf8)

    add_executable(cmp_phase1_log tests/phase1/log.c)
    target_link_libraries(cmp_phase1_log PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_log PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_log)
    add_test(NAME cmp_phase1_log COMMAND cmp_phase1_log)

    add_executable(cmp_phase1_store tests/phase1/store.c)
    target_link_libraries(cmp_phase1_store PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_store PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_store)
    add_test(NAME cmp_phase1_store COMMAND cmp_phase1_store)

    add_executable(cmp_phase1_i18n tests/phase1/i18n.c)
    target_link_libraries(cmp_phase1_i18n PRIVATE cmpc)
    target_compile_definitions(cmp_phase1_i18n PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase1_i18n)
    add_test(NAME cmp_phase1_i18n COMMAND cmp_phase1_i18n)

    add_executable(cmp_phase2_math tests/phase2/math.c)
    target_link_libraries(cmp_phase2_math PRIVATE cmpc)
    cmp_apply_c_settings(cmp_phase2_math)
    add_test(NAME cmp_phase2_math COMMAND cmp_phase2_math)

    add_executable(cmp_phase2_color tests/phase2/color.c)
    target_link_libraries(cmp_phase2_color PRIVATE m3)
    target_compile_definitions(cmp_phase2_color PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase2_color)
    add_test(NAME cmp_phase2_color COMMAND cmp_phase2_color)

    add_executable(cmp_phase2_layout tests/phase2/layout.c)
    target_link_libraries(cmp_phase2_layout PRIVATE cmpc)
    target_compile_definitions(cmp_phase2_layout PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase2_layout)
    add_test(NAME cmp_phase2_layout COMMAND cmp_phase2_layout)

    add_executable(cmp_phase2_layout_rtl tests/phase2/layout_rtl.c)
    target_link_libraries(cmp_phase2_layout_rtl PRIVATE cmpc)
    target_compile_definitions(cmp_phase2_layout_rtl PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase2_layout_rtl)
    add_test(NAME cmp_phase2_layout_rtl COMMAND cmp_phase2_layout_rtl)

    add_executable(cmp_phase2_anim tests/phase2/anim.c)
    target_link_libraries(cmp_phase2_anim PRIVATE cmpc)
    target_compile_definitions(cmp_phase2_anim PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase2_anim)
    add_test(NAME cmp_phase2_anim COMMAND cmp_phase2_anim)

    add_executable(cmp_phase2_router tests/phase2/router.c)
    target_link_libraries(cmp_phase2_router PRIVATE cmpc)
    target_compile_definitions(cmp_phase2_router PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase2_router)
    add_test(NAME cmp_phase2_router COMMAND cmp_phase2_router)

    add_executable(cmp_phase3_render tests/phase3/render.c)
    target_link_libraries(cmp_phase3_render PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_render PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_render)
    add_test(NAME cmp_phase3_render COMMAND cmp_phase3_render)

    add_executable(cmp_phase3_path tests/phase3/path.c)
    target_link_libraries(cmp_phase3_path PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_path PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_path)
    add_test(NAME cmp_phase3_path COMMAND cmp_phase3_path)

    add_executable(cmp_phase3_event tests/phase3/event.c)
    target_link_libraries(cmp_phase3_event PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_event PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_event)
    add_test(NAME cmp_phase3_event COMMAND cmp_phase3_event)

    add_executable(cmp_phase3_gesture tests/phase3/gesture.c)
    target_link_libraries(cmp_phase3_gesture PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_gesture PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_gesture)
    add_test(NAME cmp_phase3_gesture COMMAND cmp_phase3_gesture)

    add_executable(cmp_phase3_scroll tests/phase3/scroll.c)
    target_link_libraries(cmp_phase3_scroll PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_scroll PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_scroll)
    add_test(NAME cmp_phase3_scroll COMMAND cmp_phase3_scroll)

    add_executable(cmp_phase3_tasks tests/phase3/tasks.c)
    target_link_libraries(cmp_phase3_tasks PRIVATE cmpc)
    target_compile_definitions(cmp_phase3_tasks PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase3_tasks)
    add_test(NAME cmp_phase3_tasks COMMAND cmp_phase3_tasks)

    add_executable(cmp_phase4_null_backend tests/phase4/null_backend.c)
    target_link_libraries(cmp_phase4_null_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_null_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_null_backend)
    add_test(NAME cmp_phase4_null_backend COMMAND cmp_phase4_null_backend)

    add_executable(cmp_phase4_sdl3_backend tests/phase4/sdl3_backend.c)
    target_link_libraries(cmp_phase4_sdl3_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_sdl3_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_sdl3_backend)
    add_test(NAME cmp_phase4_sdl3_backend COMMAND cmp_phase4_sdl3_backend)

    add_executable(cmp_phase4_gtk4_backend tests/phase4/gtk4_backend.c)
    target_link_libraries(cmp_phase4_gtk4_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_gtk4_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_gtk4_backend)
    add_test(NAME cmp_phase4_gtk4_backend COMMAND cmp_phase4_gtk4_backend)

    add_executable(cmp_phase4_cocoa_backend tests/phase4/cocoa_backend.c)
    target_link_libraries(cmp_phase4_cocoa_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_cocoa_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_cocoa_backend)
    add_test(NAME cmp_phase4_cocoa_backend COMMAND cmp_phase4_cocoa_backend)

    add_executable(cmp_phase4_win32_backend tests/phase4/win32_backend.c)
    target_link_libraries(cmp_phase4_win32_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_win32_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_win32_backend)
    add_test(NAME cmp_phase4_win32_backend COMMAND cmp_phase4_win32_backend)

    add_executable(cmp_phase4_web_backend tests/phase4/web_backend.c)
    target_link_libraries(cmp_phase4_web_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_web_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_web_backend)
    add_test(NAME cmp_phase4_web_backend COMMAND cmp_phase4_web_backend)

    add_executable(cmp_phase4_ios_backend tests/phase4/ios_backend.c)
    target_link_libraries(cmp_phase4_ios_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_ios_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_ios_backend)
    add_test(NAME cmp_phase4_ios_backend COMMAND cmp_phase4_ios_backend)

    add_executable(cmp_phase4_android_backend tests/phase4/android_backend.c)
    target_link_libraries(cmp_phase4_android_backend PRIVATE cmpc)
    target_compile_definitions(cmp_phase4_android_backend PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase4_android_backend)
    add_test(NAME cmp_phase4_android_backend COMMAND cmp_phase4_android_backend)

    add_executable(cmp_phase5_visuals tests/phase5/visuals.c)
    target_link_libraries(cmp_phase5_visuals PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_visuals PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_visuals)
    add_test(NAME cmp_phase5_visuals COMMAND cmp_phase5_visuals)

    add_executable(cmp_phase5_text tests/phase5/text.c)
    target_link_libraries(cmp_phase5_text PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_text PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_text)
    add_test(NAME cmp_phase5_text COMMAND cmp_phase5_text)

    add_executable(cmp_phase5_icons tests/phase5/icons.c)
    target_link_libraries(cmp_phase5_icons PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_icons PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_icons)
    add_test(NAME cmp_phase5_icons COMMAND cmp_phase5_icons)

    add_executable(cmp_phase5_extras tests/phase5/extras.c)
    target_link_libraries(cmp_phase5_extras PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_extras PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_extras)
    add_test(NAME cmp_phase5_extras COMMAND cmp_phase5_extras)

    add_executable(cmp_phase5_buttons tests/phase5/buttons.c)
    target_link_libraries(cmp_phase5_buttons PRIVATE m3)
    target_compile_definitions(cmp_phase5_buttons PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_buttons)
    add_test(NAME cmp_phase5_buttons COMMAND cmp_phase5_buttons)

    add_executable(cmp_phase5_cards tests/phase5/cards.c)
    target_link_libraries(cmp_phase5_cards PRIVATE m3)
    target_compile_definitions(cmp_phase5_cards PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_cards)
    add_test(NAME cmp_phase5_cards COMMAND cmp_phase5_cards)

    add_executable(cmp_phase5_chips tests/phase5/chips.c)
    target_link_libraries(cmp_phase5_chips PRIVATE m3)
    target_compile_definitions(cmp_phase5_chips PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_chips)
    add_test(NAME cmp_phase5_chips COMMAND cmp_phase5_chips)

    add_executable(cmp_phase5_selection tests/phase5/selection.c)
    target_link_libraries(cmp_phase5_selection PRIVATE m3)
    target_compile_definitions(cmp_phase5_selection PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_selection)
    add_test(NAME cmp_phase5_selection COMMAND cmp_phase5_selection)

    add_executable(cmp_phase5_text_field tests/phase5/text_field.c)
    target_link_libraries(cmp_phase5_text_field PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_text_field PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_text_field)
    add_test(NAME cmp_phase5_text_field COMMAND cmp_phase5_text_field)

    add_executable(cmp_phase5_list tests/phase5/list.c)
    target_link_libraries(cmp_phase5_list PRIVATE cmpc)
    target_compile_definitions(cmp_phase5_list PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_list)
    add_test(NAME cmp_phase5_list COMMAND cmp_phase5_list)

    add_executable(cmp_phase5_navigation tests/phase5/navigation.c)
    target_link_libraries(cmp_phase5_navigation PRIVATE m3)
    target_compile_definitions(cmp_phase5_navigation PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_navigation)
    add_test(NAME cmp_phase5_navigation COMMAND cmp_phase5_navigation)

    add_executable(cmp_phase5_app_bar tests/phase5/app_bar.c)
    target_link_libraries(cmp_phase5_app_bar PRIVATE m3)
    target_compile_definitions(cmp_phase5_app_bar PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_app_bar)
    add_test(NAME cmp_phase5_app_bar COMMAND cmp_phase5_app_bar)

    add_executable(cmp_phase5_tabs tests/phase5/tabs.c)
    target_link_libraries(cmp_phase5_tabs PRIVATE m3)
    target_compile_definitions(cmp_phase5_tabs PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_tabs)
    add_test(NAME cmp_phase5_tabs COMMAND cmp_phase5_tabs)

    add_executable(cmp_phase5_menus tests/phase5/menus.c)
    target_link_libraries(cmp_phase5_menus PRIVATE m3)
    target_compile_definitions(cmp_phase5_menus PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_menus)
    add_test(NAME cmp_phase5_menus COMMAND cmp_phase5_menus)

    add_executable(cmp_phase5_dialogs tests/phase5/dialogs.c)
    target_link_libraries(cmp_phase5_dialogs PRIVATE m3)
    target_compile_definitions(cmp_phase5_dialogs PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_dialogs)
    add_test(NAME cmp_phase5_dialogs COMMAND cmp_phase5_dialogs)

    add_executable(cmp_phase5_sheet tests/phase5/sheet.c)
    target_link_libraries(cmp_phase5_sheet PRIVATE m3)
    target_compile_definitions(cmp_phase5_sheet PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_sheet)
    add_test(NAME cmp_phase5_sheet COMMAND cmp_phase5_sheet)

    add_executable(cmp_phase5_scaffold tests/phase5/scaffold.c)
    target_link_libraries(cmp_phase5_scaffold PRIVATE m3)
    target_compile_definitions(cmp_phase5_scaffold PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase5_scaffold)
    add_test(NAME cmp_phase5_scaffold COMMAND cmp_phase5_scaffold)

add_executable(cmp_phase5_progress tests/phase5/progress.c)
target_link_libraries(cmp_phase5_progress PRIVATE m3)
target_compile_definitions(cmp_phase5_progress PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase5_progress)
add_test(NAME cmp_phase5_progress COMMAND cmp_phase5_progress)

add_executable(cmp_phase5_date_picker tests/phase5/date_picker.c)
target_link_libraries(cmp_phase5_date_picker PRIVATE m3)
target_compile_definitions(cmp_phase5_date_picker PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase5_date_picker)
add_test(NAME cmp_phase5_date_picker COMMAND cmp_phase5_date_picker)

add_executable(cmp_phase5_time_picker tests/phase5/time_picker.c)
target_link_libraries(cmp_phase5_time_picker PRIVATE m3)
target_compile_definitions(cmp_phase5_time_picker PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase5_time_picker)
add_test(NAME cmp_phase5_time_picker COMMAND cmp_phase5_time_picker)

add_executable(cmp_phase6_storage tests/phase6/storage.c)
target_link_libraries(cmp_phase6_storage PRIVATE cmpc)
target_compile_definitions(cmp_phase6_storage PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_storage)
add_test(NAME cmp_phase6_storage COMMAND cmp_phase6_storage)
add_executable(cmp_phase6_camera tests/phase6/camera.c)
target_link_libraries(cmp_phase6_camera PRIVATE cmpc)
target_compile_definitions(cmp_phase6_camera PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_camera)
add_test(NAME cmp_phase6_camera COMMAND cmp_phase6_camera)
add_executable(cmp_phase6_network tests/phase6/network.c)
target_link_libraries(cmp_phase6_network PRIVATE cmpc)
target_compile_definitions(cmp_phase6_network PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_network)
add_test(NAME cmp_phase6_network COMMAND cmp_phase6_network)
add_executable(cmp_phase6_image tests/phase6/image.c)
target_link_libraries(cmp_phase6_image PRIVATE cmpc)
target_compile_definitions(cmp_phase6_image PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_image)
add_test(NAME cmp_phase6_image COMMAND cmp_phase6_image)
add_executable(cmp_phase6_audio tests/phase6/audio.c)
target_link_libraries(cmp_phase6_audio PRIVATE cmpc)
target_compile_definitions(cmp_phase6_audio PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_audio)
add_test(NAME cmp_phase6_audio COMMAND cmp_phase6_audio)
add_executable(cmp_phase6_video tests/phase6/video.c)
target_link_libraries(cmp_phase6_video PRIVATE cmpc)
target_compile_definitions(cmp_phase6_video PRIVATE CMP_TESTING)
cmp_apply_c_settings(cmp_phase6_video)
add_test(NAME cmp_phase6_video COMMAND cmp_phase6_video)
    add_executable(cmp_phase6_a11y tests/phase6/a11y.c)
    target_link_libraries(cmp_phase6_a11y PRIVATE cmpc)
    target_compile_definitions(cmp_phase6_a11y PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase6_a11y)
    add_test(NAME cmp_phase6_a11y COMMAND cmp_phase6_a11y)

    add_executable(cmp_phase6_predictive tests/phase6/predictive.c)
    target_link_libraries(cmp_phase6_predictive PRIVATE cmpc)
    target_compile_definitions(cmp_phase6_predictive PRIVATE CMP_TESTING)
    cmp_apply_c_settings(cmp_phase6_predictive)
    add_test(NAME cmp_phase6_predictive COMMAND cmp_phase6_predictive)
endif()
