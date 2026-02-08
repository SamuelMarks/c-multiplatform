cmake_minimum_required(VERSION 3.16)

project(LibM3C C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(M3_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(M3_ENABLE_COVERAGE "Enable coverage instrumentation (GCC/Clang only)" OFF)
option(M3_REQUIRE_DOXYGEN "Require Doxygen for documentation coverage" ON)
option(M3_ENABLE_SDL3 "Enable SDL3 debug backend" OFF)
option(M3_ENABLE_SDL3_TTF "Enable SDL3_ttf text rendering for the SDL3 backend" OFF)

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
    src/core/m3_math.c
    src/core/m3_color.c
    src/core/m3_layout.c
    src/core/m3_anim.c
    src/core/m3_router.c
    src/core/m3_render.c
    src/core/m3_event.c
    src/core/m3_tasks.c
    src/backend/null/m3_backend_null.c
    src/backend/sdl3/m3_backend_sdl3.c
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
        COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_CURRENT_SOURCE_DIR}
            --object-directory ${CMAKE_BINARY_DIR}
            --filter ${CMAKE_CURRENT_SOURCE_DIR}/src/.*
            --print-summary
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
    m3_apply_c_settings(m3_phase1_arena)
    add_test(NAME m3_phase1_arena COMMAND m3_phase1_arena)

    add_executable(m3_phase1_handle_system tests/phase1/handle_system.c)
    target_link_libraries(m3_phase1_handle_system PRIVATE m3)
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
endif()
