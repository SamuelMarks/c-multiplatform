cmake_minimum_required(VERSION 3.15)

# Disable vcpkg applocal post-build dependency copying to prevent file lock race conditions
# across concurrent test executables (error MSB3073 / exit code 32).
set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "" FORCE)
set(X_VCPKG_APPLOCAL_DEPS_SERIALIZED ON CACHE BOOL "" FORCE)

if (CMAKE_HOST_UNIX AND ((DEFINED CMAKE_C_COMPILER AND CMAKE_C_COMPILER MATCHES "(^|[/\\])cl(\\.exe)?$") OR (DEFINED CMAKE_CXX_COMPILER AND CMAKE_CXX_COMPILER MATCHES "(^|[/\\])cl(\\.exe)?$")))
    # Workaround for MSVC hanging under Wine on macOS/Linux due to mspdbsrv.exe and incremental linking
option(C_MULTIPLATFORM_BUILD_EXAMPLES "Build the example catalog and applications" ON)

    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded" CACHE STRING "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG:NONE /INCREMENTAL:NO" CACHE STRING "")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG:NONE /INCREMENTAL:NO" CACHE STRING "")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/DEBUG:NONE /INCREMENTAL:NO" CACHE STRING "")
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
         CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
         CMAKE_EXE_LINKER_FLAGS_DEBUG
         CMAKE_SHARED_LINKER_FLAGS_DEBUG
         CMAKE_MODULE_LINKER_FLAGS_DEBUG)
endif()

project(c-multiplatform-engine C)

# iOS Bundle and Signing Defaults
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(CMAKE_MACOSX_BUNDLE ON)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
    set(CMAKE_MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in")
    set(CMAKE_MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.uiengine")
endif()

if (MSVC AND CMAKE_HOST_UNIX)
    string(REGEX REPLACE "/[dD][eE][bB][uU][gG](:[a-zA-Z]+)?" "/DEBUG:NONE" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/[dD][eE][bB][uU][gG](:[a-zA-Z]+)?" "/DEBUG:NONE" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/[dD][eE][bB][uU][gG](:[a-zA-Z]+)?" "/DEBUG:NONE" CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/[iI][nN][cC][rR][eE][mM][eE][nN][tT][aA][lL](:[a-zA-Z]+)?" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/[iI][nN][cC][rR][eE][mM][eE][nN][tT][aA][lL](:[a-zA-Z]+)?" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/[iI][nN][cC][rR][eE][mM][eE][nN][tT][aA][lL](:[a-zA-Z]+)?" "/INCREMENTAL:NO" CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REGEX REPLACE "/Z[iI]" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    string(REGEX REPLACE "/[fF][sS]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "/[fF][sS]" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

    # Propagate to TryCompile
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
         CMAKE_EXE_LINKER_FLAGS_DEBUG
         CMAKE_SHARED_LINKER_FLAGS_DEBUG
         CMAKE_MODULE_LINKER_FLAGS_DEBUG
         CMAKE_C_FLAGS_DEBUG
         CMAKE_CXX_FLAGS_DEBUG
         CMAKE_C_FLAGS_RELWITHDEBINFO
         CMAKE_CXX_FLAGS_RELWITHDEBINFO)

    # Forcefully apply to all downstream targets, overriding any individual target settings
    add_compile_options("/Z7")
    add_link_options("/DEBUG:NONE" "/INCREMENTAL:NO" "/MANIFEST:NO")
endif()

# Set C89 standard strictly
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Toggles for Support Matrix
option(UI_CRT_STATIC "Use Static CRT linkage (/MT, /MTd)" OFF)
option(UI_ENABLE_UNICODE "Enable UNICODE charset" ON)
option(UI_SINGLE_THREADED "Disable Multi-threading" OFF)
option(UI_ENABLE_LTO "Enable Link-Time Optimization" OFF)
option(UI_ENABLE_WAYLAND "Enable Wayland Support on Linux" ON)
option(UI_USE_HARFBUZZ "Enable HarfBuzz integration for complex text shaping" OFF)
option(UI_SHARED_LIB "Build as Shared Library" OFF)
option(UI_MSVC_RUNTIME_CHECKS "Enable MSVC /RTC1, /RTCs, /RTCu" OFF)
option(UI_ENABLE_DESIGN_SYSTEM_M3 "Enable Material Design 3 system" ON)
option(UI_ENABLE_DESIGN_SYSTEM_M3_EXPRESSIVE "Enable Material 3 Expressive system" ON)
option(UI_ENABLE_DESIGN_SYSTEM_M2 "Enable Material Design 2 system" ON)
option(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO "Enable Cupertino (Apple HIG) design system" ON)
option(BUILD_VISUAL_TESTS "Build and run visual fuzzy-match tests" ON)
option(UI_ENABLE_ASAN "Enable AddressSanitizer (ASAN)" OFF)

# Synchronize with CDD / external matrix variables
if(DEFINED BUILD_SHARED_LIBS)
    set(UI_SHARED_LIB ${BUILD_SHARED_LIBS})
else()
    set(BUILD_SHARED_LIBS ${UI_SHARED_LIB})
endif()

if(DEFINED CDD_CHARSET)
    if(CDD_CHARSET STREQUAL "UNICODE")
        set(UI_ENABLE_UNICODE ON)
    elseif(CDD_CHARSET STREQUAL "ANSI")
        set(UI_ENABLE_UNICODE OFF)
    endif()
endif()

if(DEFINED CDD_THREADING)
    if(CDD_THREADING)
        set(UI_SINGLE_THREADED OFF)
    else()
        set(UI_SINGLE_THREADED ON)
    endif()
endif()

if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    set(UI_ENABLE_LTO ON)
endif()

if(DEFINED CDD_CRT_LINKAGE)
    if(CDD_CRT_LINKAGE STREQUAL "Static")
        set(UI_CRT_STATIC ON)
    elseif(CDD_CRT_LINKAGE STREQUAL "Shared")
        set(UI_CRT_STATIC OFF)
    endif()
endif()

if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
        set(UI_CRT_STATIC OFF)
    elseif(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded")
        set(UI_CRT_STATIC ON)
    endif()
endif()

if(DEFINED CDD_MSVC_RTC)
    if(CDD_MSVC_RTC AND NOT CDD_MSVC_RTC STREQUAL "OFF")
        set(UI_MSVC_RUNTIME_CHECKS ON)
        set(CFS_MSVC_RTC "${CDD_MSVC_RTC}" CACHE STRING "" FORCE)
        set(CDD_MSVC_RTC "${CDD_MSVC_RTC}" CACHE STRING "" FORCE)
    else()
        set(UI_MSVC_RUNTIME_CHECKS OFF)
        set(CFS_MSVC_RTC "" CACHE STRING "" FORCE)
        set(CDD_MSVC_RTC "" CACHE STRING "" FORCE)
    endif()
endif()

if(UI_USE_HARFBUZZ)
    message(STATUS "Fetching harfbuzz via FetchContent")
    FetchContent_Declare(
      harfbuzz
      GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
      GIT_TAG        8.3.0
    )
    set(HB_HAVE_FREETYPE OFF CACHE BOOL "Disable FreeType dependency in HarfBuzz for now" FORCE)
    set(HB_BUILD_TESTS OFF CACHE BOOL "Disable HarfBuzz tests" FORCE)
    set(HB_BUILD_SUBDIR_ALREADY_CONFIGURED ON)
    FetchContent_MakeAvailable(harfbuzz)
endif()

if(UI_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(UI_CRT_STATIC AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
elseif(MSVC AND NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()

# Pass down CRT static options to dependencies
set(C_CDD_STATIC_CRT ${UI_CRT_STATIC} CACHE BOOL "" FORCE)
set(CDD_USE_STATIC_CRT ${UI_CRT_STATIC} CACHE BOOL "" FORCE)
set(C_ABSTRACT_HTTP_STATIC_CRT ${UI_CRT_STATIC} CACHE BOOL "" FORCE)
set(CFS_MSVC_CRT_STATIC ${UI_CRT_STATIC} CACHE BOOL "" FORCE)
set(C_ORM_STATIC_CRT ${UI_CRT_STATIC} CACHE BOOL "" FORCE)
set(GREATEST_USE_STATIC_CRT ${UI_CRT_STATIC} CACHE BOOL "" FORCE)

if(UI_CRT_STATIC)
    set(C_ABSTRACT_HTTP_SHARED_CRT OFF CACHE BOOL "" FORCE)
    set(CDD_CRT_LINKAGE "Static" CACHE STRING "" FORCE)
else()
    set(C_ABSTRACT_HTTP_SHARED_CRT ON CACHE BOOL "" FORCE)
    set(CDD_CRT_LINKAGE "Shared" CACHE STRING "" FORCE)
endif()

if(UI_SINGLE_THREADED)
    add_compile_definitions(UI_SINGLE_THREADED)
else()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

if(UI_ENABLE_UNICODE)
    add_compile_definitions(UNICODE _UNICODE)
endif()

if(MSVC)
    if(CDD_MSVC_RTC AND NOT CDD_MSVC_RTC STREQUAL "OFF")
        add_compile_options("/${CDD_MSVC_RTC}")
    elseif(UI_MSVC_RUNTIME_CHECKS)
        if(UI_MSVC_RUNTIME_CHECKS STREQUAL "RTCs")
            add_compile_options(/RTCs)
        elseif(UI_MSVC_RUNTIME_CHECKS STREQUAL "RTCu")
            add_compile_options(/RTCu)
        else()
            add_compile_options(/RTC1)
        endif()
    endif()
endif()

if(UI_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported)
    if(lto_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb")
if(EMSCRIPTEN)
    if(BUILD_TESTING)
        set(CMAKE_EXECUTABLE_SUFFIX ".js")
    else()
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
    endif()

    # Exported functions and methods
    set(UI_WASM_EXPORTED_FUNCS "_main,_malloc,_free,_c-multiplatform-cdk_tick,_ui_web_bridge_dispatch_event,_ui_web_bridge_dispatch_resize,_ui_web_bridge_dispatch_key")
    set(UI_WASM_RUNTIME_METHODS "ccall,cwrap,UTF8ToString,stringToUTF8,setValue,getValue")

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -gsource-map")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0 -gsource-map --source-map-base http://localhost:8080/")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -flto")
    endif()

    if(NOT UI_SINGLE_THREADED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_PTHREADS=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4")
    endif()

    # Linker specific flags
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s STACK_SIZE=1048576 -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB -s ERROR_ON_UNDEFINED_SYMBOLS=0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=${UI_WASM_EXPORTED_FUNCS} -s EXPORTED_RUNTIME_METHODS=${UI_WASM_RUNTIME_METHODS}")
endif()


if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gy")



    # Target-specific warnings will be added later

else()
    # Target-specific warnings will be added later
endif()

if(UNIX AND NOT APPLE AND NOT CYGWIN AND NOT EMSCRIPTEN)
    if(UI_ENABLE_WAYLAND)
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(WAYLAND wayland-client wayland-egl egl wayland-cursor xkbcommon)
            find_program(WAYLAND_SCANNER wayland-scanner)
        endif()

        if(WAYLAND_FOUND AND WAYLAND_SCANNER AND EXISTS "/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml")
            add_definitions(-DUI_ENABLE_WAYLAND=1)
            execute_process(
                COMMAND ${WAYLAND_SCANNER} client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h
            )
            execute_process(
                COMMAND ${WAYLAND_SCANNER} private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c
            )
            if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")
                file(READ "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h" _xdg_hdr)
                string(REGEX REPLACE ",([ \t\r\n]*})" "\\1" _xdg_hdr "${_xdg_hdr}")
                file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h" "${_xdg_hdr}")
            endif()
            include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR})

            set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c src/ui_window_backend_linux_wayland.c ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c)
            set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c PROPERTIES COMPILE_FLAGS " -Dinline=__inline__")
            set_source_files_properties(src/ui_window_backend_linux_wayland.c PROPERTIES COMPILE_FLAGS " -Dinline=__inline__")
        else()
            message(STATUS "Wayland dependencies not found; falling back to X11")
            set(UI_ENABLE_WAYLAND OFF)
            find_package(X11 QUIET)
            if(X11_FOUND)
                add_definitions(-DHAVE_X11)
                include_directories(${X11_INCLUDE_DIR})
                set(HAVE_X11 ON)
            endif()
            set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c src/ui_window_backend_linux_wayland.c)
        endif()
    else()
        find_package(X11 QUIET)
        if(X11_FOUND)
            add_definitions(-DHAVE_X11)
            include_directories(${X11_INCLUDE_DIR})
            set(HAVE_X11 ON)
        endif()
        set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c src/ui_window_backend_linux_wayland.c)
    endif()
else()
    set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c src/ui_window_backend_linux_wayland.c)
endif()

include(cmake/PlatformLinkages.cmake)

set(UI_DESIGN_SYSTEM_SRC src/ui_design_system.c)
set(UI_DESIGN_SYSTEM_M3_SRC "")
if(UI_ENABLE_DESIGN_SYSTEM_M3)
    list(APPEND UI_DESIGN_SYSTEM_M3_SRC
        src/material3/md3_init.c
        src/material3/md3_color.c
        src/material3/md3_elevation.c
        src/material3/md3_state_layer.c
        src/material3/md3_typography.c
        src/material3/md3_shape.c
        src/material3/md3_shape_morph.c
        src/material3/md3_spring.c
        src/material3/md3_motion.c
        src/material3/md3_button.c
        src/material3/md3_split_button.c
        src/material3/md3_segmented_button.c
        src/material3/md3_icon_button.c
        src/material3/md3_fab.c
        src/material3/md3_checkbox.c
        src/material3/md3_radio_button.c
        src/material3/md3_switch.c
        src/material3/md3_text_field.c
        src/material3/md3_search.c
        src/material3/md3_slider.c
        src/material3/md3_progress.c
        src/material3/md3_card.c
        src/material3/md3_divider.c
        src/material3/md3_list.c
        src/material3/md3_carousel.c
        src/material3/md3_chip.c
        src/material3/md3_navigation.c
        src/material3/md3_overlay.c
        src/material3/md3_a11y.c
        src/material3/md3_bidi.c
        src/material3/md3_action_widgets.c
        src/material3/md3_selection_controls.c
        src/material3/md3_containment.c
        src/material3/md3_shell_layout.c
        src/material3/md3_overlay_menus.c
        src/material3/md3_data_hierarchy.c
        src/material3/md3_media_workspace.c
        src/material3/md3_top_app_bar.c
        src/material3/md3_floating_toolbar.c
        src/material3/md3_fab_menu.c
        src/material3/md3_date_time_pickers.c
        src/material3/md3_adaptive.c)
endif()

set(UI_DESIGN_SYSTEM_M2_SRC "")
if(UI_ENABLE_DESIGN_SYSTEM_M2)
    list(APPEND UI_DESIGN_SYSTEM_M2_SRC
        src/material2/md2_theme.c
        src/material2/md2_elevation.c
        src/material2/md2_shape.c
        src/material2/md2_button.c
        src/material2/md2_card.c
        src/material2/md2_fab.c
        src/material2/md2_top_app_bar.c
        src/material2/md2_bottom_navigation.c
        src/material2/md2_navigation_drawer.c
        src/material2/md2_selection_controls.c
        src/material2/md2_text_field.c
        src/material2/md2_overlays.c)
endif()

set(UI_DESIGN_SYSTEM_CUPERTINO_SRC "")
if(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO)
    list(APPEND UI_DESIGN_SYSTEM_CUPERTINO_SRC
        src/cupertino/cupertino_init.c
        src/cupertino/cupertino_tokens.c
        src/cupertino/cupertino_squircle.c
        src/cupertino/cupertino_spring.c
        src/cupertino/cupertino_material.c
        src/cupertino/cupertino_button.c
        src/cupertino/cupertino_switch.c
        src/cupertino/cupertino_slider.c
        src/cupertino/cupertino_checkbox.c
        src/cupertino/cupertino_typography.c
        src/cupertino/cupertino_radio.c
        src/cupertino/cupertino_segmented_control.c
        src/cupertino/cupertino_stepper.c
        src/cupertino/cupertino_form.c
        src/cupertino/cupertino_text_field.c
        src/cupertino/cupertino_nav_bar.c
        src/cupertino/cupertino_tab_bar.c
        src/cupertino/cupertino_toolbar.c
        src/cupertino/cupertino_alert.c
        src/cupertino/cupertino_sheet.c
        src/cupertino/cupertino_list_tile.c
        src/cupertino/cupertino_list_section.c
        src/cupertino/cupertino_scrollbar.c
        src/cupertino/cupertino_search_bar.c
        src/cupertino/cupertino_refresh.c
        src/cupertino/cupertino_indicator.c
        src/cupertino/cupertino_badge.c
        src/cupertino/cupertino_page_control.c
        src/cupertino/cupertino_haptics.c
        src/cupertino/cupertino_activity_rings.c
        src/cupertino/cupertino_magnifier.c
        src/cupertino/cupertino_wheel.c
        src/cupertino/cupertino_apple_pay.c
        src/cupertino/cupertino_dynamic_island.c
        src/cupertino/cupertino_menu_bar.c
        src/cupertino/cupertino_photo_picker.c
        src/cupertino/cupertino_date_picker.c
        src/cupertino/cupertino_color_well.c
        src/cupertino/cupertino_text_selection_toolbar.c
        src/cupertino/cupertino_biometrics.c
        src/cupertino/cupertino_popover.c
        src/cupertino/cupertino_shareplay.c
        src/cupertino/cupertino_sf_symbols.c
        src/cupertino/cupertino_content_unavailable.c
        src/cupertino/cupertino_popup_surface.c
        src/cupertino/cupertino_split_view.c
        src/cupertino/cupertino_page_scaffold.c
        src/cupertino/cupertino_context_menu.c
        src/cupertino/cupertino_tab_scaffold.c
        src/cupertino/cupertino_tab_view.c
        src/cupertino/cupertino_activity_view.c
        src/cupertino/cupertino_quick_look.c
        src/cupertino/cupertino_fullscreen_dialog.c
        src/cupertino/cupertino_live_activity.c
        src/cupertino/cupertino_menu_button.c
        src/cupertino/cupertino_table_view.c
        src/cupertino/cupertino_collection_view.c
        src/cupertino/cupertino_bidi.c
        src/cupertino/cupertino_a11y.c
        src/cupertino/cupertino_gesture.c
        src/cupertino/cupertino_control_center.c
        src/cupertino/cupertino_menu_bar_extra.c
        src/cupertino/cupertino_dock_tile.c
        src/cupertino/cupertino_desktop_text_selection_toolbar.c
        src/cupertino/cupertino_spell_check_toolbar.c
        src/cupertino/cupertino_spatial_ornament.c
        src/cupertino/cupertino_status_bar.c)
endif()

# Library Target
if(UI_SHARED_LIB)
    add_library(c-multiplatform-cdk SHARED src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_design_system.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_floating_toolbar_base.c src/ui_adaptive_pane_scaffold_base.c src/ui_scroll_field_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_form_validators.c src/ui_runtime_schema.c src/ui_component_registry.c src/ui_app_state_registry.c src/ui_dynamic_context.c src/ui_workflow_engine.c src/ui_runtime_router.c src/ui_runtime_builder.c src/ui_runtime_eject.c src/ui_app_load.c src/ui_ffi_runtime.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_font_provider_macos.c src/ui_font_provider_win32.c src/ui_font_provider_linux.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_audio_alsa.c src/ui_audio_coreaudio.c src/ui_audio_opensles.c src/ui_audio_wasapi.c src/ui_audio_webaudio.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_jpeg.c src/ui_image_png.c src/ui_image_webp.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_video_avfoundation.c src/ui_video_ffmpeg.c src/ui_video_mediacodec.c src/ui_video_mf.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c )
else()
    add_library(c-multiplatform-cdk STATIC src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_design_system.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_floating_toolbar_base.c src/ui_adaptive_pane_scaffold_base.c src/ui_scroll_field_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_form_validators.c src/ui_runtime_schema.c src/ui_component_registry.c src/ui_app_state_registry.c src/ui_dynamic_context.c src/ui_workflow_engine.c src/ui_runtime_router.c src/ui_runtime_builder.c src/ui_runtime_eject.c src/ui_app_load.c src/ui_ffi_runtime.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_font_provider_macos.c src/ui_font_provider_win32.c src/ui_font_provider_linux.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_audio_alsa.c src/ui_audio_coreaudio.c src/ui_audio_opensles.c src/ui_audio_wasapi.c src/ui_audio_webaudio.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_jpeg.c src/ui_image_png.c src/ui_image_webp.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_video_avfoundation.c src/ui_video_ffmpeg.c src/ui_video_mediacodec.c src/ui_video_mf.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c )
endif()

target_link_platform_libraries(c-multiplatform-cdk PUBLIC)
if(UI_SHARED_LIB)
    add_library(c-multiplatform-material2 SHARED ${UI_DESIGN_SYSTEM_M2_SRC})
    add_library(c-multiplatform-material3 SHARED ${UI_DESIGN_SYSTEM_M3_SRC})
    add_library(c-multiplatform-cupertino SHARED ${UI_DESIGN_SYSTEM_CUPERTINO_SRC})
else()
    add_library(c-multiplatform-material2 STATIC ${UI_DESIGN_SYSTEM_M2_SRC})
    add_library(c-multiplatform-material3 STATIC ${UI_DESIGN_SYSTEM_M3_SRC})
    add_library(c-multiplatform-cupertino STATIC ${UI_DESIGN_SYSTEM_CUPERTINO_SRC})
endif()

target_link_libraries(c-multiplatform-material2 PUBLIC c-multiplatform-cdk)
target_link_libraries(c-multiplatform-material3 PUBLIC c-multiplatform-cdk)
target_link_libraries(c-multiplatform-cupertino PUBLIC c-multiplatform-cdk)

foreach(tgt c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    target_include_directories(${tgt} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>")
    target_include_directories(${tgt} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
endforeach()

if(UI_ENABLE_DESIGN_SYSTEM_M3)
    target_compile_definitions(c-multiplatform-cdk PUBLIC UI_HAS_DESIGN_SYSTEM_M3)
endif()
if(UI_ENABLE_DESIGN_SYSTEM_M3_EXPRESSIVE)
    target_compile_definitions(c-multiplatform-cdk PUBLIC UI_HAS_DESIGN_SYSTEM_M3_EXPRESSIVE)
endif()
if(UI_ENABLE_DESIGN_SYSTEM_M2)
    target_compile_definitions(c-multiplatform-cdk PUBLIC UI_HAS_DESIGN_SYSTEM_M2)
endif()
if(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO)
    target_compile_definitions(c-multiplatform-cdk PUBLIC UI_HAS_DESIGN_SYSTEM_CUPERTINO)
endif()

if(UI_SHARED_LIB)
    target_compile_definitions(c-multiplatform-cdk PUBLIC C_MULTIPLATFORM_BUILD_SHARED)
    target_compile_definitions(c-multiplatform-cdk PRIVATE c-multiplatform-cdk_EXPORTS)
endif()

if(NOT UI_SINGLE_THREADED)
    target_link_libraries(c-multiplatform-cdk PRIVATE Threads::Threads)
endif()
if(MSVC)
    target_compile_options(c-multiplatform-cdk PRIVATE /W4 /WX /Gy)
else()
    target_compile_options(c-multiplatform-cdk PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    if(ENABLE_COVERAGE)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
        endif()
    endif()
endif()


include(FetchContent)

macro(UI_FetchDependencyLocalOrArchive NAME LOCAL_DIR GITHUB_REPO)
    # Disable tests for dependencies to speed up builds
    set(BUILD_TESTING OFF CACHE BOOL "Disable testing for dependencies" FORCE)
    set(BUILD_TESTING_${NAME} OFF CACHE BOOL "Disable testing for dependencies" FORCE)
    set(C_CDD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(C_CDD_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(C_ORM_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(C_ABSTRACT_HTTP_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(C_ORM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(C_ORM_BUILD_EXAMPLES OFF CACHE BOOL "Disable c-orm examples" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Disable testing for dependencies" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples for dependencies" FORCE)

    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY "https://github.com/${GITHUB_REPO}.git"
        GIT_TAG "master"
    )
    set(_PREV_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(${NAME})
    set(BUILD_SHARED_LIBS ${_PREV_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)

    if(NAME STREQUAL "c_cdd" AND EXISTS "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h")
        file(READ "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h" _cdd_diff_h)
        string(REPLACE "#ifdef CDD_BUILD_TESTS\n/**\n * @brief DiffLine struct" "/**\n * @brief DiffLine struct" _cdd_diff_h "${_cdd_diff_h}")
        string(REPLACE "};\n\n/**\n * @brief Test helper" "};\n\n#ifdef CDD_BUILD_TESTS\n/**\n * @brief Test helper" _cdd_diff_h "${_cdd_diff_h}")
        file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h" "${_cdd_diff_h}")
    endif()

    if(NAME STREQUAL "c_cdd" AND EXISTS "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c")
        file(READ "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c" _cdd_url_c)
        string(REPLACE "#else\nstatic void *url_segment_malloc_impl(size_t sz) { return malloc(sz); }\nstatic void *url_segment_realloc_impl(void *ptr, size_t sz) {\n  return realloc(ptr, sz);\n}"
                       "#elif defined(CDD_BUILD_TESTS)\nstatic void *url_segment_malloc_impl(size_t sz) { return malloc(sz); }\nstatic void *url_segment_realloc_impl(void *ptr, size_t sz) {\n  return realloc(ptr, sz);\n}"
                       _cdd_url_c "${_cdd_url_c}")
        file(WRITE "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c" "${_cdd_url_c}")
    endif()
    if(NAME STREQUAL "c_cdd" AND EXISTS "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c")
        file(READ "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c" _cdd_pp_dir_c)
        string(REPLACE "static cdd_c_error_t " "cdd_c_error_t " _cdd_pp_dir_c "${_cdd_pp_dir_c}")
        file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c" "${_cdd_pp_dir_c}")
    endif()
    if(NAME STREQUAL "c_cdd" AND EXISTS "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c")
        file(READ "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c" _cdd_pp_eval_c)
        string(REPLACE "static cdd_c_error_t " "cdd_c_error_t " _cdd_pp_eval_c "${_cdd_pp_eval_c}")
        file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c" "${_cdd_pp_eval_c}")
    endif()

    # Re-enable testing for the main project
    set(BUILD_TESTING ON CACHE BOOL "Enable testing for main project" FORCE)
endmacro()

if(NOT MSVC)
    set(OLD_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

if(NOT TARGET cfs)
    UI_FetchDependencyLocalOrArchive(cfs "c-fs" "SamuelMarks/c-fs")
endif()

UI_FetchDependencyLocalOrArchive(parson "parson" "SamuelMarks/parson")
UI_FetchDependencyLocalOrArchive(c_cdd "cdd-c" "SamuelMarks/cdd-c")
if(EXISTS "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h")
    file(READ "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h" _cdd_diff_h)
    string(REPLACE "#ifdef CDD_BUILD_TESTS\n/**\n * @brief DiffLine struct" "/**\n * @brief DiffLine struct" _cdd_diff_h "${_cdd_diff_h}")
    string(REPLACE "};\n\n/**\n * @brief Test helper" "};\n\n#ifdef CDD_BUILD_TESTS\n/**\n * @brief Test helper" _cdd_diff_h "${_cdd_diff_h}")
    file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/emit/diff.h" "${_cdd_diff_h}")
endif()
if(EXISTS "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c")
    file(READ "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c" _cdd_url_c)
    string(REPLACE "#else\nstatic void *url_segment_malloc_impl(size_t sz) { return malloc(sz); }\nstatic void *url_segment_realloc_impl(void *ptr, size_t sz) {\n  return realloc(ptr, sz);\n}"
                   "#elif defined(CDD_BUILD_TESTS)\nstatic void *url_segment_malloc_impl(size_t sz) { return malloc(sz); }\nstatic void *url_segment_realloc_impl(void *ptr, size_t sz) {\n  return realloc(ptr, sz);\n}"
                   _cdd_url_c "${_cdd_url_c}")
    file(WRITE "${c_cdd_SOURCE_DIR}/src/routes/emit/url.c" "${_cdd_url_c}")
endif()
if(EXISTS "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c")
    file(READ "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c" _cdd_pp_dir_c)
    string(REPLACE "static cdd_c_error_t " "cdd_c_error_t " _cdd_pp_dir_c "${_cdd_pp_dir_c}")
    file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_directives.c" "${_cdd_pp_dir_c}")
endif()
if(EXISTS "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c")
    file(READ "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c" _cdd_pp_eval_c)
    string(REPLACE "static cdd_c_error_t " "cdd_c_error_t " _cdd_pp_eval_c "${_cdd_pp_eval_c}")
    file(WRITE "${c_cdd_SOURCE_DIR}/src/functions/parse/preprocessor_eval.c" "${_cdd_pp_eval_c}")
endif()
UI_FetchDependencyLocalOrArchive(c-abstract-http "c-abstract-http" "SamuelMarks/c-abstract-http")
UI_FetchDependencyLocalOrArchive(c_orm "c-orm" "SamuelMarks/c-orm")

foreach(dep c_cdd c_cdd_compiler_flags c_str_span c_str_span_compiler_flags c-abstract-http c-orm c-orm-async c89stringutils c89stringutils_compiler_flags parson cfs)
    if(TARGET ${dep})
        set_property(TARGET ${dep} PROPERTY COMPILE_OPTIONS "")
        set_property(TARGET ${dep} PROPERTY INTERFACE_COMPILE_OPTIONS "")
        set_property(TARGET ${dep} PROPERTY LINK_OPTIONS "")
        set_property(TARGET ${dep} PROPERTY INTERFACE_LINK_OPTIONS "")
    endif()
endforeach()
if(TARGET c_cdd_compiler_flags)
    set_property(TARGET c_cdd_compiler_flags PROPERTY COMPILE_OPTIONS "")
    set_property(TARGET c_cdd_compiler_flags PROPERTY INTERFACE_COMPILE_OPTIONS "")
    set_property(TARGET c_cdd_compiler_flags PROPERTY LINK_OPTIONS "")
    set_property(TARGET c_cdd_compiler_flags PROPERTY INTERFACE_LINK_OPTIONS "")
endif()
if(TARGET c_str_span_compiler_flags)
    set_property(TARGET c_str_span_compiler_flags PROPERTY COMPILE_OPTIONS "")
    set_property(TARGET c_str_span_compiler_flags PROPERTY INTERFACE_COMPILE_OPTIONS "")
    set_property(TARGET c_str_span_compiler_flags PROPERTY LINK_OPTIONS "")
    set_property(TARGET c_str_span_compiler_flags PROPERTY INTERFACE_LINK_OPTIONS "")
endif()
if(TARGET c89stringutils_compiler_flags)
    set_property(TARGET c89stringutils_compiler_flags PROPERTY COMPILE_OPTIONS "")
    set_property(TARGET c89stringutils_compiler_flags PROPERTY INTERFACE_COMPILE_OPTIONS "")
    set_property(TARGET c89stringutils_compiler_flags PROPERTY LINK_OPTIONS "")
    set_property(TARGET c89stringutils_compiler_flags PROPERTY INTERFACE_LINK_OPTIONS "")
endif()

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${OLD_CMAKE_C_FLAGS}")
endif()
if(TARGET c-orm AND TARGET c_str_span)
    target_include_directories(c-orm PUBLIC "${c_cdd_SOURCE_DIR}/src")
    target_link_libraries(c-orm PUBLIC c_str_span c_cdd)
endif()

if(TARGET c_orm_standalone_fixtures)
    set_target_properties(c_orm_standalone_fixtures PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
if(TARGET c_orm_legacy_cdd_db)
    set_target_properties(c_orm_legacy_cdd_db PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

if(TARGET c_cdd)
    if(NOT WIN32)
        find_package(OpenSSL QUIET)
        if(TARGET OpenSSL::Crypto)
            target_link_libraries(c_cdd PRIVATE OpenSSL::Crypto)
        endif()
    endif()
endif()


# Strip coverage flags that bleed from dependencies in Debug mode
foreach(DEP_TARGET c_cdd c_cdd_compiler_flags c-abstract-http cfs c-orm c-orm-async c_str_span c_str_span_compiler_flags c89stringutils c89stringutils_compiler_flags parson)
    if(TARGET ${DEP_TARGET})
        set_target_properties(${DEP_TARGET} PROPERTIES INTERFACE_COMPILE_OPTIONS "")
        set_target_properties(${DEP_TARGET} PROPERTIES INTERFACE_LINK_OPTIONS "")
    endif()
endforeach()

if(TARGET cfs AND MSVC)
    get_target_property(_cfs_opts cfs COMPILE_OPTIONS)
    if(_cfs_opts)
        list(REMOVE_ITEM _cfs_opts "/OFF")
        set_target_properties(cfs PROPERTIES COMPILE_OPTIONS "${_cfs_opts}")
    endif()
endif()

target_link_libraries(c-multiplatform-cdk PRIVATE c-abstract-http cfs c-orm)
if(WIN32 OR MSVC)
    target_link_libraries(c-multiplatform-cdk PRIVATE mfplat mfuuid ole32 user32 gdi32 gdiplus opengl32 windowscodecs)
endif()
target_include_directories(c-multiplatform-cdk PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" "$<INSTALL_INTERFACE:include>")
target_include_directories(c-multiplatform-cdk PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Tests
include(CTest)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    enable_testing()
    add_compile_options(-UNDEBUG)
    remove_definitions(-DNDEBUG)

    include(FetchContent)
    FetchContent_Declare(
      greatest
      GIT_REPOSITORY https://github.com/SamuelMarks/greatest.git
      GIT_TAG        master
    )
    FetchContent_MakeAvailable(greatest)

    # Expose greatest to test targets
    include_directories(${greatest_SOURCE_DIR})

    # Recompile library with mock allocator injected for OOM tests
    add_library(c-multiplatform-cdk_test STATIC src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_design_system.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_floating_toolbar_base.c src/ui_adaptive_pane_scaffold_base.c src/ui_scroll_field_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_form_validators.c src/ui_runtime_schema.c src/ui_component_registry.c src/ui_app_state_registry.c src/ui_dynamic_context.c src/ui_workflow_engine.c src/ui_runtime_router.c src/ui_runtime_builder.c src/ui_runtime_eject.c src/ui_app_load.c src/ui_ffi_runtime.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_audio_wasapi.c src/ui_audio_coreaudio.c src/ui_audio_alsa.c src/ui_audio_opensles.c src/ui_audio_webaudio.c src/ui_video_mf.c src/ui_video_avfoundation.c src/ui_video_mediacodec.c src/ui_video_ffmpeg.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_png.c src/ui_image_jpeg.c src/ui_image_webp.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_font_provider_macos.c src/ui_font_provider_win32.c src/ui_font_provider_linux.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c )
    target_include_directories(c-multiplatform-cdk_test PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
    target_include_directories(c-multiplatform-cdk_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_compile_definitions(c-multiplatform-cdk_test PUBLIC C_MULTIPLATFORM_MALLOC=ui_mock_malloc C_MULTIPLATFORM_FREE=ui_mock_free C_MULTIPLATFORM_REALLOC=ui_mock_realloc C_MULTIPLATFORM_STRDUP=ui_mock_strdup UI_TEST_MOCK_ALLOC=1)
    if(UI_ENABLE_DESIGN_SYSTEM_M3)
        target_compile_definitions(c-multiplatform-cdk_test PUBLIC UI_HAS_DESIGN_SYSTEM_M3)
    endif()
    if(UI_ENABLE_DESIGN_SYSTEM_M3_EXPRESSIVE)
        target_compile_definitions(c-multiplatform-cdk_test PUBLIC UI_HAS_DESIGN_SYSTEM_M3_EXPRESSIVE)
    endif()
    if(UI_ENABLE_DESIGN_SYSTEM_M2)
        target_compile_definitions(c-multiplatform-cdk_test PUBLIC UI_HAS_DESIGN_SYSTEM_M2)
    endif()
    if(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO)
        target_compile_definitions(c-multiplatform-cdk_test PUBLIC UI_HAS_DESIGN_SYSTEM_CUPERTINO)
    endif()
    add_dependencies(c-multiplatform-cdk_test c-multiplatform-cdk)
    target_link_platform_libraries(c-multiplatform-cdk_test PUBLIC)
    add_library(c-multiplatform-material2_test STATIC ${UI_DESIGN_SYSTEM_M2_SRC})
    add_library(c-multiplatform-material3_test STATIC ${UI_DESIGN_SYSTEM_M3_SRC})
    add_library(c-multiplatform-cupertino_test STATIC ${UI_DESIGN_SYSTEM_CUPERTINO_SRC})

    target_link_libraries(c-multiplatform-material2_test PUBLIC c-multiplatform-cdk_test)
    target_link_libraries(c-multiplatform-material3_test PUBLIC c-multiplatform-cdk_test)
    target_link_libraries(c-multiplatform-cupertino_test PUBLIC c-multiplatform-cdk_test)

    foreach(tgt c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        target_include_directories(${tgt} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
        target_include_directories(${tgt} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_compile_definitions(${tgt} PUBLIC C_MULTIPLATFORM_MALLOC=ui_mock_malloc C_MULTIPLATFORM_FREE=ui_mock_free C_MULTIPLATFORM_REALLOC=ui_mock_realloc C_MULTIPLATFORM_STRDUP=ui_mock_strdup UI_TEST_MOCK_ALLOC=1)
        if(UI_ENABLE_DESIGN_SYSTEM_M3)
            target_compile_definitions(${tgt} PUBLIC UI_HAS_DESIGN_SYSTEM_M3)
        endif()
        if(UI_ENABLE_DESIGN_SYSTEM_M3_EXPRESSIVE)
            target_compile_definitions(${tgt} PUBLIC UI_HAS_DESIGN_SYSTEM_M3_EXPRESSIVE)
        endif()
        if(UI_ENABLE_DESIGN_SYSTEM_M2)
            target_compile_definitions(${tgt} PUBLIC UI_HAS_DESIGN_SYSTEM_M2)
        endif()
        if(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO)
            target_compile_definitions(${tgt} PUBLIC UI_HAS_DESIGN_SYSTEM_CUPERTINO)
        endif()
        add_dependencies(${tgt} c-multiplatform-cdk)
        target_link_platform_libraries(${tgt} PUBLIC)
    endforeach()
    if(NOT UI_SINGLE_THREADED)
        target_link_libraries(c-multiplatform-cdk_test PRIVATE Threads::Threads)
    endif()

    target_link_libraries(c-multiplatform-cdk_test PRIVATE c-abstract-http cfs c-orm)
    if(WIN32 OR MSVC)
        target_link_libraries(c-multiplatform-cdk_test PRIVATE mfplat mfuuid ole32 user32 gdi32 gdiplus opengl32 windowscodecs)
    endif()

    if(MSVC)
        target_compile_options(c-multiplatform-cdk_test PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(c-multiplatform-cdk_test PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
        if(ENABLE_COVERAGE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        endif()
    endif()

    add_executable(test_ui_execution_context tests/test_ui_execution_context.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_execution_context c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_execution_context COMMAND test_ui_execution_context)

    add_executable(test_ui_tick_engine tests/test_ui_tick_engine.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tick_engine c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_tick_engine COMMAND test_ui_tick_engine)

    add_executable(test_ui_thread_pool tests/test_ui_thread_pool.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_thread_pool c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_thread_pool COMMAND test_ui_thread_pool)

    add_executable(test_ui_reactor tests/test_ui_reactor.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_reactor c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_reactor COMMAND test_ui_reactor)

    add_executable(test_ui_reactor_real tests/test_ui_reactor_real.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_reactor_real c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    if(ENABLE_COVERAGE)
        target_link_options(test_ui_reactor_real PRIVATE --coverage)
        target_compile_options(test_ui_reactor_real PRIVATE --coverage)
    endif()
    add_test(NAME test_ui_reactor_real COMMAND test_ui_reactor_real)

    add_executable(test_ui_promise tests/test_ui_promise.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_promise c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_promise COMMAND test_ui_promise)

    add_executable(test_ui_atomic tests/test_ui_atomic.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_atomic c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_atomic COMMAND test_ui_atomic)

    add_executable(test_ui_ring_buffer tests/test_ui_ring_buffer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ring_buffer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_ring_buffer COMMAND test_ui_ring_buffer)

    add_executable(test_ui_timer tests/test_ui_timer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_timer COMMAND test_ui_timer)

    add_executable(test_ui_types tests/test_ui_types.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_types c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_types COMMAND test_ui_types)

    add_executable(test_ui_arena tests/test_ui_arena.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_arena c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_arena COMMAND test_ui_arena)

    add_executable(test_ui_pool tests/test_ui_pool.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pool c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_pool COMMAND test_ui_pool)

    add_executable(test_ui_engine_core tests/test_ui_engine_core.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_engine_core c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_engine_core COMMAND test_ui_engine_core)

    add_executable(test_ui_handle_manager tests/test_ui_handle_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_handle_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_handle_manager COMMAND test_ui_handle_manager)

    add_executable(test_ui_window_backend_win32 tests/test_ui_window_backend_win32.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_win32 c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_win32 COMMAND test_ui_window_backend_win32)

    add_executable(test_ui_window_backend tests/test_ui_window_backend.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend COMMAND test_ui_window_backend)

    add_executable(test_ui_window_backend_android tests/test_ui_window_backend_android.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_android c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_android COMMAND test_ui_window_backend_android)

    add_executable(test_ui_window_backend_ios tests/test_ui_window_backend_ios.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_ios c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_ios COMMAND test_ui_window_backend_ios)

    add_executable(test_ui_window_backend_linux tests/test_ui_window_backend_linux.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_linux c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_linux COMMAND test_ui_window_backend_linux)

    add_executable(test_ui_window_backend_macos tests/test_ui_window_backend_macos.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_macos c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_macos COMMAND test_ui_window_backend_macos)

    add_executable(test_ui_dpi_scaling tests/test_ui_dpi_scaling.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dpi_scaling c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_dpi_scaling COMMAND test_ui_dpi_scaling)

    add_executable(test_ui_font_loading tests/test_ui_font_loading.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_font_loading c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_font_loading COMMAND test_ui_font_loading)

    add_executable(test_ui_kerning tests/test_ui_kerning.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_kerning c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_kerning COMMAND test_ui_kerning)

    add_executable(test_ui_window_backend_web tests/test_ui_window_backend_web.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_web c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_web COMMAND test_ui_window_backend_web)

    add_executable(test_ui_window_creation_screenshots tests/test_ui_window_creation_screenshots.c)
    target_link_libraries(test_ui_window_creation_screenshots c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    add_test(NAME test_ui_window_creation_screenshots COMMAND test_ui_window_creation_screenshots)

    add_executable(test_ui_window_handle_hook tests/test_ui_window_handle_hook.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_handle_hook c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_handle_hook COMMAND test_ui_window_handle_hook)

    add_executable(test_ui_data_source tests/test_ui_data_source.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_data_source c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_data_source COMMAND test_ui_data_source)

    add_executable(test_ui_input_mask tests/test_ui_input_mask.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_input_mask c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_input_mask COMMAND test_ui_input_mask)

    add_executable(test_ui_i18n tests/test_ui_i18n.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_i18n c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_i18n COMMAND test_ui_i18n)

    add_executable(test_ui_video_ffmpeg tests/test_ui_video_ffmpeg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_ffmpeg c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_ffmpeg COMMAND test_ui_video_ffmpeg)

    add_executable(test_ui_tree_grid_base tests/test_ui_tree_grid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tree_grid_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_tree_grid_base COMMAND test_ui_tree_grid_base)

    add_executable(test_ui_side_sheet_base tests/test_ui_side_sheet_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_side_sheet_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_side_sheet_base COMMAND test_ui_side_sheet_base)

    add_executable(test_ui_dockable_layout_base tests/test_ui_dockable_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dockable_layout_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_dockable_layout_base COMMAND test_ui_dockable_layout_base)

    add_executable(test_ui_hotkey_registry tests/test_ui_hotkey_registry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_hotkey_registry c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_hotkey_registry COMMAND test_ui_hotkey_registry)

    add_executable(test_ui_scroll_spy tests/test_ui_scroll_spy.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_spy c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_scroll_spy COMMAND test_ui_scroll_spy)

    add_executable(test_ui_resizable_behavior tests/test_ui_resizable_behavior.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_resizable_behavior c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_resizable_behavior COMMAND test_ui_resizable_behavior)

    add_executable(test_ui_virtual_keyboard tests/test_ui_virtual_keyboard.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_virtual_keyboard c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_virtual_keyboard COMMAND test_ui_virtual_keyboard)

    add_executable(test_ui_preferences tests/test_ui_preferences.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_preferences c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_preferences COMMAND test_ui_preferences)
    add_executable(test_ui_predictive_back tests/test_ui_predictive_back.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_predictive_back c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_predictive_back COMMAND test_ui_predictive_back)

    add_executable(test_ui_dom_node tests/test_ui_dom_node.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_audio_alsa tests/test_ui_audio_alsa.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_alsa c-multiplatform-cdk_test cfs parson c_cdd c-orm)
    add_test(NAME test_ui_audio_alsa COMMAND test_ui_audio_alsa)

    add_executable(test_ui_event tests/test_ui_event.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_event c-multiplatform-cdk_test cfs parson c_cdd c-orm)
    add_test(NAME test_ui_event COMMAND test_ui_event)
    target_link_libraries(test_ui_dom_node c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_dom_node COMMAND test_ui_dom_node)

    add_executable(test_ui_cssom tests/test_ui_cssom.c tests/test_ui_cssom_selectors.c tests/test_ui_cssom_rules.c tests/test_ui_cssom_declarations.c tests/test_ui_cssom_stylesheets.c tests/test_ui_cssom_serialization.c tests/test_ui_cssom_memory.c tests/test_ui_cssom_vars.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_cssom COMMAND test_ui_cssom)


    add_executable(test_ui_fs tests/test_ui_fs.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_fs c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_fs COMMAND test_ui_fs)

    add_executable(test_ui_layout tests/test_ui_layout.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_layout c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_layout COMMAND test_ui_layout)

    add_executable(test_ui_css_parser tests/test_ui_css_parser.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_parser c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_parser COMMAND test_ui_css_parser)

    add_executable(test_ui_css_values tests/test_ui_css_values.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_values c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_values COMMAND test_ui_css_values)

    add_executable(test_ui_css_env tests/test_ui_css_env.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_env c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_env COMMAND test_ui_css_env)

    add_executable(test_ui_css_contain tests/test_ui_css_contain.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_contain c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_contain COMMAND test_ui_css_contain)

    add_executable(test_ui_css_overflow tests/test_ui_css_overflow.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_overflow c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_overflow COMMAND test_ui_css_overflow)

    add_executable(test_ui_css_lists tests/test_ui_css_lists.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_lists c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_lists COMMAND test_ui_css_lists)

    add_executable(test_ui_css_content tests/test_ui_css_content.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_content c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_content COMMAND test_ui_css_content)

    add_executable(test_ui_css_logical tests/test_ui_css_logical.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_logical c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_logical COMMAND test_ui_css_logical)

    add_executable(test_ui_css_scrollbars tests/test_ui_css_scrollbars.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scrollbars c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_scrollbars COMMAND test_ui_css_scrollbars)

    add_executable(test_ui_css_round_display tests/test_ui_css_round_display.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_round_display c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_round_display COMMAND test_ui_css_round_display)

    add_executable(test_ui_css_ui tests/test_ui_css_ui.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_ui c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_ui COMMAND test_ui_css_ui)

    add_executable(test_ui_css_forms tests/test_ui_css_forms.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_forms c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_forms COMMAND test_ui_css_forms)

    add_executable(test_ui_css_motion tests/test_ui_css_motion.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_motion c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_motion COMMAND test_ui_css_motion)

    add_executable(test_ui_css_view_transition tests/test_ui_css_view_transition.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_view_transition c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_view_transition COMMAND test_ui_css_view_transition)

    add_executable(test_ui_component tests/test_ui_component.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_component c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_component COMMAND test_ui_component)

    add_executable(test_ui_media tests/test_ui_media.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_media c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_media COMMAND test_ui_media)

    add_executable(test_ui_audio_sink tests/test_ui_audio_sink.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_sink c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_audio_sink COMMAND test_ui_audio_sink)

    add_executable(test_ui_video_decoder tests/test_ui_video_decoder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_decoder c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_decoder COMMAND test_ui_video_decoder)

    add_executable(test_ui_av_sync tests/test_ui_av_sync.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_av_sync c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_av_sync COMMAND test_ui_av_sync)

    add_executable(test_ui_image_decoder tests/test_ui_image_decoder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_decoder c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_image_decoder COMMAND test_ui_image_decoder)

    add_executable(test_ui_image_png tests/test_ui_image_png.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_png c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_image_png COMMAND test_ui_image_png)

    add_executable(test_ui_image_jpeg tests/test_ui_image_jpeg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_jpeg c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_image_jpeg COMMAND test_ui_image_jpeg)

    add_executable(test_ui_image_webp tests/test_ui_image_webp.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_webp c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_image_webp COMMAND test_ui_image_webp)

    add_executable(test_ui_renderer_gles_fallback tests/test_ui_renderer_gles_fallback.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gles_fallback c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer_gles_fallback COMMAND test_ui_renderer_gles_fallback)

    add_executable(test_ui_svg tests/test_ui_svg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_svg c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_svg COMMAND test_ui_svg)

    add_executable(test_ui_asset_streamer tests/test_ui_asset_streamer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_asset_streamer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_asset_streamer COMMAND test_ui_asset_streamer)

    add_executable(test_ui_shader_manager tests/test_ui_shader_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_shader_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_shader_manager COMMAND test_ui_shader_manager)

    add_executable(test_ui_renderer tests/test_ui_renderer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer COMMAND test_ui_renderer)

    add_executable(test_ui_renderer_gles2 tests/test_ui_renderer_gles2.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gles2 c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer_gles2 COMMAND test_ui_renderer_gles2)

    add_executable(test_ui_font_manager tests/test_ui_font_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_font_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_font_manager COMMAND test_ui_font_manager)

    add_executable(test_ui_text_layout tests/test_ui_text_layout.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_layout c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_text_layout COMMAND test_ui_text_layout)

    add_executable(test_ui_video_shaders tests/test_ui_video_shaders.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_shaders c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_shaders COMMAND test_ui_video_shaders)

    add_executable(test_ui_effects_shaders tests/test_ui_effects_shaders.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_effects_shaders c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_effects_shaders COMMAND test_ui_effects_shaders)

    add_executable(test_ui_vibrancy tests/test_ui_vibrancy.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_vibrancy c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_vibrancy COMMAND test_ui_vibrancy)


    add_executable(test_ui_transfer_list_base tests/test_ui_transfer_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_transfer_list_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_transfer_list_base COMMAND test_ui_transfer_list_base)

    add_executable(test_ui_search_bar_base tests/test_ui_search_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_search_bar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_search_bar_base COMMAND test_ui_search_bar_base)

    add_executable(test_ui_swipe_action_base tests/test_ui_swipe_action_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_swipe_action_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_swipe_action_base COMMAND test_ui_swipe_action_base)

    add_executable(test_ui_speed_dial_base tests/test_ui_speed_dial_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_speed_dial_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_speed_dial_base COMMAND test_ui_speed_dial_base)

    add_executable(test_ui_video_player_base tests/test_ui_video_player_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_player_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_player_base COMMAND test_ui_video_player_base)

    add_executable(test_ui_command_palette_base tests/test_ui_command_palette_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_command_palette_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_command_palette_base COMMAND test_ui_command_palette_base)

    add_executable(test_ui_focus_manager tests/test_ui_focus_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_focus_manager COMMAND test_ui_focus_manager)

    add_executable(test_ui_keyboard_responder tests/test_ui_keyboard_responder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_keyboard_responder c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_keyboard_responder COMMAND test_ui_keyboard_responder)

    add_executable(test_ui_gesture tests/test_ui_gesture.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_gesture c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_gesture COMMAND test_ui_gesture)

    add_executable(test_ui_aria tests/test_ui_aria.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_aria c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_aria COMMAND test_ui_aria)


    add_executable(test_ui_overlay_director tests/test_ui_overlay_director.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_overlay_director c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_overlay_director COMMAND test_ui_overlay_director)

    add_executable(test_ui_geometry tests/test_ui_geometry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_geometry c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_geometry COMMAND test_ui_geometry)

    add_executable(test_ui_geometry_anchor tests/test_ui_geometry_anchor.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_geometry_anchor c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_geometry_anchor COMMAND test_ui_geometry_anchor)

    add_executable(test_ui_backdrop tests/test_ui_backdrop.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_backdrop c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_backdrop COMMAND test_ui_backdrop)

    add_executable(test_ui_alert_base tests/test_ui_alert_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_alert_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_alert_base COMMAND test_ui_alert_base)

    add_executable(test_ui_button_base tests/test_ui_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_button_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_button_base COMMAND test_ui_button_base)

    add_executable(test_ui_split_button_base tests/test_ui_split_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_split_button_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_split_button_base COMMAND test_ui_split_button_base)

    add_executable(test_ui_bottom_app_bar_base tests/test_ui_bottom_app_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bottom_app_bar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_bottom_app_bar_base COMMAND test_ui_bottom_app_bar_base)

    add_executable(test_ui_section_index_base tests/test_ui_section_index_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_section_index_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_section_index_base COMMAND test_ui_section_index_base)

    add_executable(test_ui_haptics tests/test_ui_haptics.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_haptics c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_haptics COMMAND test_ui_haptics)

    add_executable(test_ui_toolbar_base tests/test_ui_toolbar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_toolbar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_toolbar_base COMMAND test_ui_toolbar_base)

    add_executable(test_ui_floating_toolbar_base tests/test_ui_floating_toolbar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_floating_toolbar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_floating_toolbar_base COMMAND test_ui_floating_toolbar_base)

    add_executable(test_ui_adaptive_pane_scaffold_base tests/test_ui_adaptive_pane_scaffold_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_adaptive_pane_scaffold_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_adaptive_pane_scaffold_base COMMAND test_ui_adaptive_pane_scaffold_base)

    add_executable(test_ui_scroll_field_base tests/test_ui_scroll_field_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_field_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_scroll_field_base COMMAND test_ui_scroll_field_base)

    add_executable(test_ui_split_pane_base tests/test_ui_split_pane_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_split_pane_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_split_pane_base COMMAND test_ui_split_pane_base)

    add_executable(test_ui_grid_list_base tests/test_ui_grid_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_grid_list_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_grid_list_base COMMAND test_ui_grid_list_base)

    add_executable(test_ui_fab_base tests/test_ui_fab_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_fab_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_fab_base COMMAND test_ui_fab_base)

    add_executable(test_ui_icon_base tests/test_ui_icon_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_icon_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_icon_base COMMAND test_ui_icon_base)

    add_executable(test_ui_skeleton_base tests/test_ui_skeleton_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_skeleton_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_skeleton_base COMMAND test_ui_skeleton_base)

    add_executable(test_ui_avatar_base tests/test_ui_avatar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_avatar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_avatar_base COMMAND test_ui_avatar_base)

    add_executable(test_ui_avatar_group_base tests/test_ui_avatar_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_avatar_group_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_avatar_group_base COMMAND test_ui_avatar_group_base)

    add_executable(test_ui_sort_header_base tests/test_ui_sort_header_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sort_header_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_sort_header_base COMMAND test_ui_sort_header_base)
    add_executable(test_ui_divider_base tests/test_ui_divider_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_divider_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_divider_base COMMAND test_ui_divider_base)
    add_executable(test_ui_selection_model tests/test_ui_selection_model.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_selection_model PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_selection_model COMMAND test_ui_selection_model)
    add_executable(test_ui_chips_base tests/test_ui_chips_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chips_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_chips_base COMMAND test_ui_chips_base)
    add_executable(test_ui_calendar_base tests/test_ui_calendar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_calendar_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_calendar_base COMMAND test_ui_calendar_base)
    add_executable(test_ui_card_base tests/test_ui_card_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_card_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_card_base COMMAND test_ui_card_base)
    add_executable(test_ui_pagination_base tests/test_ui_pagination_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pagination_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_pagination_base COMMAND test_ui_pagination_base)
    add_executable(test_ui_accordion_base tests/test_ui_accordion_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_accordion_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_accordion_base COMMAND test_ui_accordion_base)
    add_executable(test_ui_surface_base tests/test_ui_surface_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_surface_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_surface_base COMMAND test_ui_surface_base)
    add_executable(test_ui_link_base tests/test_ui_link_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_link_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_link_base COMMAND test_ui_link_base)
    add_executable(test_ui_meter_base tests/test_ui_meter_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_meter_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_meter_base COMMAND test_ui_meter_base)
    add_executable(test_ui_page_control_base tests/test_ui_page_control_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_page_control_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_page_control_base COMMAND test_ui_page_control_base)
    add_executable(test_ui_banner_base tests/test_ui_banner_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_banner_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_banner_base COMMAND test_ui_banner_base)
    add_executable(test_ui_button_group_base tests/test_ui_button_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_button_group_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_button_group_base COMMAND test_ui_button_group_base)
    add_executable(test_ui_empty_state_base tests/test_ui_empty_state_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_empty_state_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_empty_state_base COMMAND test_ui_empty_state_base)
    add_executable(test_ui_scaffold_base tests/test_ui_scaffold_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scaffold_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_scaffold_base COMMAND test_ui_scaffold_base)
    add_executable(test_ui_menubar_base tests/test_ui_menubar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_menubar_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_menubar_base COMMAND test_ui_menubar_base)
    add_executable(test_ui_radio_group_base tests/test_ui_radio_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_radio_group_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_radio_group_base COMMAND test_ui_radio_group_base)
    add_executable(test_ui_color_picker_base tests/test_ui_color_picker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_picker_base PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_color_picker_base COMMAND test_ui_color_picker_base)
    add_executable(test_ui_color_space tests/test_ui_color_space.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_space PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_color_space COMMAND test_ui_color_space)

    add_executable(test_ui_color_quantization tests/test_ui_color_quantization.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_quantization PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_color_quantization COMMAND test_ui_color_quantization)
    add_executable(test_ui_design_tokens tests/test_ui_design_tokens.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_design_tokens PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_design_tokens COMMAND test_ui_design_tokens)
    add_executable(test_ui_tonal_palette tests/test_ui_tonal_palette.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tonal_palette PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_tonal_palette COMMAND test_ui_tonal_palette)
    add_executable(test_ui_theme_provider tests/test_ui_theme_provider.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_theme_provider PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_theme_provider COMMAND test_ui_theme_provider)
    add_executable(test_ui_shape_family tests/test_ui_shape_family.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_shape_family PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_shape_family COMMAND test_ui_shape_family)
    add_executable(test_ui_typography_scale tests/test_ui_typography_scale.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_typography_scale PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_typography_scale COMMAND test_ui_typography_scale)
    add_executable(test_ui_state_layer tests/test_ui_state_layer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_state_layer PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_state_layer COMMAND test_ui_state_layer)
    add_executable(test_ui_elevation tests/test_ui_elevation.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_elevation PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_elevation COMMAND test_ui_elevation)
    add_executable(test_ui_focus_ring tests/test_ui_focus_ring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_ring PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_focus_ring COMMAND test_ui_focus_ring)

    add_executable(test_ui_timeline_base tests/test_ui_timeline_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timeline_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_timeline_base COMMAND test_ui_timeline_base)

    add_executable(test_ui_slide_toggle_base tests/test_ui_slide_toggle_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_slide_toggle_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_slide_toggle_base COMMAND test_ui_slide_toggle_base)

    add_executable(test_ui_timepicker_base tests/test_ui_timepicker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timepicker_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_timepicker_base COMMAND test_ui_timepicker_base)

    add_executable(test_ui_drag_drop tests/test_ui_drag_drop.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_drag_drop c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_drag_drop COMMAND test_ui_drag_drop)

    add_executable(test_ui_nav_rail_base tests/test_ui_nav_rail_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_nav_rail_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_nav_rail_base COMMAND test_ui_nav_rail_base)

    add_executable(test_ui_bottom_nav_base tests/test_ui_bottom_nav_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bottom_nav_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_bottom_nav_base COMMAND test_ui_bottom_nav_base)

    add_executable(test_ui_segmented_control_base tests/test_ui_segmented_control_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_segmented_control_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_segmented_control_base COMMAND test_ui_segmented_control_base)

    add_executable(test_ui_list_base tests/test_ui_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_list_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_list_base COMMAND test_ui_list_base)

    add_executable(test_ui_listbox_base tests/test_ui_listbox_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_listbox_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_listbox_base COMMAND test_ui_listbox_base)

    add_executable(test_ui_autocomplete_base tests/test_ui_autocomplete_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_autocomplete_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_autocomplete_base COMMAND test_ui_autocomplete_base)

    add_executable(test_ui_clipboard tests/test_ui_clipboard.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_clipboard c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_clipboard COMMAND test_ui_clipboard)

    add_executable(test_ui_layout_observer tests/test_ui_layout_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_layout_observer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_layout_observer COMMAND test_ui_layout_observer)

    add_executable(test_ui_toggle_base tests/test_ui_toggle_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_toggle_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_toggle_base COMMAND test_ui_toggle_base)

    add_executable(test_ui_checkbox_base tests/test_ui_checkbox_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_checkbox_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_checkbox_base COMMAND test_ui_checkbox_base)

    add_executable(test_ui_file_uploader_base tests/test_ui_file_uploader_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_file_uploader_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_file_uploader_base COMMAND test_ui_file_uploader_base)

    add_executable(test_ui_intersection_observer tests/test_ui_intersection_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_intersection_observer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_intersection_observer COMMAND test_ui_intersection_observer)

    add_executable(test_ui_mutation_observer tests/test_ui_mutation_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_mutation_observer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_mutation_observer COMMAND test_ui_mutation_observer)

    add_executable(test_ui_platform_detector tests/test_ui_platform_detector.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_platform_detector c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_platform_detector COMMAND test_ui_platform_detector)

    add_executable(test_ui_focus_trap tests/test_ui_focus_trap.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_trap c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_focus_trap COMMAND test_ui_focus_trap)

    add_executable(test_ui_coercion_utils tests/test_ui_coercion_utils.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_coercion_utils c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_coercion_utils COMMAND test_ui_coercion_utils)

    add_executable(test_ui_form_field_base tests/test_ui_form_field_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_field_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_field_base COMMAND test_ui_form_field_base)

    add_executable(test_ui_sidenav_base tests/test_ui_sidenav_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sidenav_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_sidenav_base COMMAND test_ui_sidenav_base)

    add_executable(test_ui_bottom_sheet_base tests/test_ui_bottom_sheet_base.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_action_sheet_base tests/test_ui_action_sheet_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_action_sheet_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_action_sheet_base COMMAND test_ui_action_sheet_base)
    target_link_libraries(test_ui_bottom_sheet_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_bottom_sheet_base COMMAND test_ui_bottom_sheet_base)

    add_executable(test_ui_snackbar_base tests/test_ui_snackbar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_snackbar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_snackbar_base COMMAND test_ui_snackbar_base)

    add_executable(test_ui_input_base tests/test_ui_input_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_input_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_input_base COMMAND test_ui_input_base)

    add_executable(test_ui_scroll_base tests/test_ui_scroll_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_scroll_base COMMAND test_ui_scroll_base)

    add_executable(test_ui_slider_base tests/test_ui_slider_base.c tests/ui_test_mock_mem.c)
add_executable(test_ui_range_slider_base tests/test_ui_range_slider_base.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_range_slider_base PRIVATE c-multiplatform-cdk_test)
    add_executable(test_ui_wheel_picker_base tests/test_ui_wheel_picker_base.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_pull_to_refresh_base tests/test_ui_pull_to_refresh_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pull_to_refresh_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_pull_to_refresh_base COMMAND test_ui_pull_to_refresh_base)
    target_link_libraries(test_ui_wheel_picker_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_wheel_picker_base COMMAND test_ui_wheel_picker_base)
    add_executable(test_ui_text_node_base tests/test_ui_text_node_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_node_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_text_node_base COMMAND test_ui_text_node_base)
    add_executable(test_ui_spin_button_base tests/test_ui_spin_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_spin_button_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_spin_button_base COMMAND test_ui_spin_button_base)
    target_link_libraries(test_ui_slider_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_slider_base COMMAND test_ui_slider_base)
add_test(NAME test_ui_range_slider_base COMMAND test_ui_range_slider_base)

    add_executable(test_ui_select_base tests/test_ui_select_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_select_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_select_base COMMAND test_ui_select_base)
    add_executable(test_ui_dialog_base tests/test_ui_dialog_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dialog_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_dialog_base COMMAND test_ui_dialog_base)

    add_executable(test_ui_disclosure_base tests/test_ui_disclosure_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_disclosure_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_disclosure_base COMMAND test_ui_disclosure_base)

    add_executable(test_ui_modifier tests/test_ui_modifier.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_modifier c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_modifier COMMAND test_ui_modifier)

    add_executable(test_ui_router tests/test_ui_router.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_router c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_router COMMAND test_ui_router)

    add_executable(test_ui_e2e_headless tests/test_ui_e2e_headless.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_e2e_headless c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_e2e_headless COMMAND test_ui_e2e_headless)

    add_executable(test_ui_e2e_headful tests/test_ui_e2e_headful.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_e2e_headful c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_e2e_headful COMMAND test_ui_e2e_headful)

    add_executable(test_ui_web_animation tests/test_ui_web_animation.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_spring tests/test_ui_spring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_spring c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_spring COMMAND test_ui_spring)
    target_link_libraries(test_ui_web_animation c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_web_animation COMMAND test_ui_web_animation)

    add_executable(test_ui_css_speech tests/test_ui_css_speech.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_speech c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_speech COMMAND test_ui_css_speech)

    add_executable(test_ui_css_scroll_snap tests/test_ui_css_scroll_snap.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scroll_snap c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_scroll_snap COMMAND test_ui_css_scroll_snap)

    add_executable(test_ui_css_scroll_anchoring tests/test_ui_css_scroll_anchoring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scroll_anchoring c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_scroll_anchoring COMMAND test_ui_css_scroll_anchoring)

    add_executable(test_ui_css_spatial_nav tests/test_ui_css_spatial_nav.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_spatial_nav c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_spatial_nav COMMAND test_ui_css_spatial_nav)

    add_executable(test_ui_css_highlight_api tests/test_ui_css_highlight_api.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_highlight_api c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_highlight_api COMMAND test_ui_css_highlight_api)

    add_executable(test_ui_css_page tests/test_ui_css_page.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_page c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_page COMMAND test_ui_css_page)

    add_executable(test_ui_css_gcpm tests/test_ui_css_gcpm.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_gcpm c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_gcpm COMMAND test_ui_css_gcpm)

    add_executable(test_ui_css_page_floats tests/test_ui_css_page_floats.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_page_floats c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_css_page_floats COMMAND test_ui_css_page_floats)

    add_executable(test_ui_cssom_api tests/test_ui_cssom_api.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom_api c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_cssom_api COMMAND test_ui_cssom_api)

    add_executable(test_ui_cssom_view tests/test_ui_cssom_view.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_cssom_view COMMAND test_ui_cssom_view)

    add_executable(test_ui_menu_base tests/test_ui_menu_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_menu_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_menu_base COMMAND test_ui_menu_base)

    add_executable(test_ui_popover_base tests/test_ui_popover_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_popover_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_popover_base COMMAND test_ui_popover_base)

    add_executable(test_ui_stepper_base tests/test_ui_stepper_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_stepper_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_stepper_base COMMAND test_ui_stepper_base)

    add_executable(test_ui_table_base tests/test_ui_table_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_table_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_table_base COMMAND test_ui_table_base)

    add_executable(test_ui_tabs_base tests/test_ui_tabs_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tabs_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_tabs_base COMMAND test_ui_tabs_base)

    add_executable(test_ui_toast_manager_base tests/test_ui_toast_manager_base.c tests/ui_test_mock_mem.c)
add_executable(test_ui_breakpoint_observer tests/test_ui_breakpoint_observer.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_breakpoint_observer PRIVATE c-multiplatform-cdk_test)
    target_link_libraries(test_ui_toast_manager_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_toast_manager_base COMMAND test_ui_toast_manager_base)
add_test(NAME test_ui_breakpoint_observer COMMAND test_ui_breakpoint_observer)

    add_executable(test_ui_tooltip_base tests/test_ui_tooltip_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tooltip_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_tooltip_base COMMAND test_ui_tooltip_base)

    add_executable(test_ui_tree_base tests/test_ui_tree_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tree_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_tree_base COMMAND test_ui_tree_base)

    add_executable(test_ui_virtual_scroll_base tests/test_ui_virtual_scroll_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_virtual_scroll_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_virtual_scroll_base COMMAND test_ui_virtual_scroll_base)

    add_executable(test_ui_carousel_base tests/test_ui_carousel_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_carousel_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_carousel_base COMMAND test_ui_carousel_base)

    add_executable(test_ui_breadcrumbs_base tests/test_ui_breadcrumbs_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_breadcrumbs_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_breadcrumbs_base COMMAND test_ui_breadcrumbs_base)

    add_executable(test_ui_portal tests/test_ui_portal.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_portal c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_portal COMMAND test_ui_portal)

    add_executable(test_ui_context_menu_base tests/test_ui_context_menu_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_context_menu_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_context_menu_base COMMAND test_ui_context_menu_base)

    add_executable(test_ui_live_announcer tests/test_ui_live_announcer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_live_announcer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_live_announcer COMMAND test_ui_live_announcer)

    add_executable(test_ui_bidi_manager tests/test_ui_bidi_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bidi_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_bidi_manager COMMAND test_ui_bidi_manager)

    add_executable(test_ui_progress_base tests/test_ui_progress_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_progress_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_progress_base COMMAND test_ui_progress_base)

    add_executable(test_ui_camera_base tests/test_ui_camera_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_camera_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_camera_base COMMAND test_ui_camera_base)

    add_executable(test_ui_webview_base tests/test_ui_webview_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_webview_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_webview_base COMMAND test_ui_webview_base)

    add_executable(test_ui_ink_base tests/test_ui_ink_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ink_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_ink_base COMMAND test_ui_ink_base)

    add_executable(test_ui_coachmark_base tests/test_ui_coachmark_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_coachmark_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_coachmark_base COMMAND test_ui_coachmark_base)

    add_executable(test_ui_signal tests/test_ui_signal.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_signal c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_signal COMMAND test_ui_signal)

    add_executable(test_ui_concurrency tests/test_ui_concurrency.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_concurrency c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_concurrency COMMAND test_ui_concurrency)

    add_executable(test_ui_computed tests/test_ui_computed.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_computed c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_computed COMMAND test_ui_computed)

    add_executable(test_ui_effect tests/test_ui_effect.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_effect c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_effect COMMAND test_ui_effect)

    add_executable(test_ui_rich_text_base tests/test_ui_rich_text_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rich_text_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_rich_text_base COMMAND test_ui_rich_text_base)

    add_executable(test_ui_renderer_gl1 tests/test_ui_renderer_gl1.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gl1 c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer_gl1 COMMAND test_ui_renderer_gl1)

    add_executable(test_ui_web_bridge tests/test_ui_web_bridge.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_web_bridge c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_web_bridge COMMAND test_ui_web_bridge)

    add_executable(test_ui_reactive_graph tests/test_ui_reactive_graph.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_reactive_graph c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_reactive_graph COMMAND test_ui_reactive_graph)

    add_executable(test_ui_form_node tests/test_ui_form_node.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_node c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_node COMMAND test_ui_form_node)

    add_executable(test_ui_form_builder tests/test_ui_form_builder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_builder c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_builder COMMAND test_ui_form_builder)

    add_executable(test_ui_datepicker_base tests/test_ui_datepicker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_datepicker_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_datepicker_base COMMAND test_ui_datepicker_base)

    add_executable(test_ui_os_dialogs tests/test_ui_os_dialogs.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_os_dialogs c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_os_dialogs COMMAND test_ui_os_dialogs)



    add_executable(test_ui_window_backend_linux_wayland tests/test_ui_window_backend_linux_wayland.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_linux_wayland c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_backend_linux_wayland COMMAND test_ui_window_backend_linux_wayland)

    add_executable(test_ui_badge_base tests/test_ui_badge_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_badge_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_badge_base COMMAND test_ui_badge_base)

    add_executable(test_ui_rating_base tests/test_ui_rating_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rating_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_rating_base COMMAND test_ui_rating_base)

    add_executable(test_ui_ripple_base tests/test_ui_ripple_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ripple_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_ripple_base COMMAND test_ui_ripple_base)

    add_executable(test_ui_textarea_autosize tests/test_ui_textarea_autosize.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_textarea_autosize c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_textarea_autosize COMMAND test_ui_textarea_autosize)

    add_executable(test_ui_key_manager tests/test_ui_key_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_key_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_key_manager COMMAND test_ui_key_manager)

    add_executable(test_ui_scroll_dispatcher tests/test_ui_scroll_dispatcher.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_dispatcher c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_scroll_dispatcher COMMAND test_ui_scroll_dispatcher)

    add_executable(test_ui_label_base tests/test_ui_label_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_label_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_label_base COMMAND test_ui_label_base)

    add_executable(test_ui_aspect_ratio_base tests/test_ui_aspect_ratio_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_aspect_ratio_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_aspect_ratio_base COMMAND test_ui_aspect_ratio_base)

    add_executable(test_ui_masonry_layout_base tests/test_ui_masonry_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_masonry_layout_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_masonry_layout_base COMMAND test_ui_masonry_layout_base)

    add_executable(test_ui_pin_input_base tests/test_ui_pin_input_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pin_input_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_pin_input_base COMMAND test_ui_pin_input_base)

    add_executable(test_ui_hover_card_base tests/test_ui_hover_card_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_hover_card_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_hover_card_base COMMAND test_ui_hover_card_base)

    add_executable(test_ui_window_manager_base tests/test_ui_window_manager_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_manager_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_manager_base COMMAND test_ui_window_manager_base)

    add_executable(test_ui_rich_text_editor_base tests/test_ui_rich_text_editor_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rich_text_editor_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_rich_text_editor_base COMMAND test_ui_rich_text_editor_base)

    add_executable(test_ui_datagrid_base tests/test_ui_datagrid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_datagrid_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_datagrid_base COMMAND test_ui_datagrid_base)

    add_executable(test_ui_form_group tests/test_ui_form_group.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_group c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_group COMMAND test_ui_form_group)

    add_executable(test_ui_form_e2e tests/test_ui_form_e2e.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_e2e c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_e2e COMMAND test_ui_form_e2e)


    add_executable(test_ui_form_array tests/test_ui_form_array.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_array c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_array COMMAND test_ui_form_array)

    add_executable(test_ui_canonical_layout_base tests/test_ui_canonical_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_canonical_layout_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_canonical_layout_base COMMAND test_ui_canonical_layout_base)

    add_executable(test_ui_window_controls_base tests/test_ui_window_controls_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_controls_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_window_controls_base COMMAND test_ui_window_controls_base)

    add_executable(test_ui_compositor_material_base tests/test_ui_compositor_material_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_compositor_material_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_compositor_material_base COMMAND test_ui_compositor_material_base)

    add_executable(test_ui_loupe_base tests/test_ui_loupe_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_loupe_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_loupe_base COMMAND test_ui_loupe_base)

    add_executable(test_ui_ribbon_base tests/test_ui_ribbon_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ribbon_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_ribbon_base COMMAND test_ui_ribbon_base)

    add_executable(test_ui_miller_columns_base tests/test_ui_miller_columns_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_miller_columns_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_miller_columns_base COMMAND test_ui_miller_columns_base)

    add_executable(test_ui_property_grid_base tests/test_ui_property_grid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_property_grid_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_property_grid_base COMMAND test_ui_property_grid_base)

    add_executable(test_ui_syntax_surface_base tests/test_ui_syntax_surface_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_syntax_surface_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_syntax_surface_base COMMAND test_ui_syntax_surface_base)

    add_executable(test_ui_node_graph_base tests/test_ui_node_graph_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_node_graph_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_node_graph_base COMMAND test_ui_node_graph_base)

    add_executable(test_ui_chart_base tests/test_ui_chart_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chart_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_chart_base COMMAND test_ui_chart_base)

    add_executable(test_ui_chat_bubble_base tests/test_ui_chat_bubble_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chat_bubble_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_chat_bubble_base COMMAND test_ui_chat_bubble_base)

    add_executable(test_ui_form_control tests/test_ui_form_control.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_control c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_control COMMAND test_ui_form_control)

    add_executable(test_ui_form_oom tests/test_ui_form_oom.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_oom c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_oom COMMAND test_ui_form_oom)

    add_executable(test_ui_form_validators tests/test_ui_form_validators.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_validators c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_form_validators COMMAND test_ui_form_validators)

    add_executable(test_ui_top_app_bar_base tests/test_ui_top_app_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_top_app_bar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_top_app_bar_base COMMAND test_ui_top_app_bar_base)

    add_executable(test_ui_titlebar_base tests/test_ui_titlebar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_titlebar_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_titlebar_base COMMAND test_ui_titlebar_base)

    add_executable(test_ui_theme_manager tests/test_ui_theme_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_theme_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_theme_manager COMMAND test_ui_theme_manager)
    add_executable(test_ui_safe_area_manager tests/test_ui_safe_area_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_safe_area_manager c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_safe_area_manager COMMAND test_ui_safe_area_manager)

    if(BUILD_VISUAL_TESTS)
        add_executable(test_ui_visual tests/test_ui_visual.c tests/ui_test_mock_mem.c src/ui_test_visual.c)
        target_link_libraries(test_ui_visual c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    endif()
    add_executable(test_ui_test_visual tests/test_ui_test_visual.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_test_visual c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_test_visual COMMAND test_ui_test_visual)
    add_executable(test_ui_date_range_picker_base tests/test_ui_date_range_picker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_date_range_picker_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_date_range_picker_base COMMAND test_ui_date_range_picker_base)
    add_executable(test_ui_map_view_base tests/test_ui_map_view_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_map_view_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_map_view_base COMMAND test_ui_map_view_base)
    add_executable(test_ui_tray_manager tests/test_ui_tray_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tray_manager PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_tray_manager COMMAND test_ui_tray_manager)
    add_executable(test_ui_auth tests/test_ui_auth.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_auth PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_auth COMMAND test_ui_auth)
    add_executable(test_ui_share tests/test_ui_share.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_share PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_share COMMAND test_ui_share)
    add_executable(test_ui_sensor_manager tests/test_ui_sensor_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sensor_manager PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_sensor_manager COMMAND test_ui_sensor_manager)
    add_executable(test_ui_text_layout_hb tests/test_ui_text_layout_hb.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_layout_hb PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_ui_text_layout_hb COMMAND test_ui_text_layout_hb)
        add_test(NAME test_ui_visual COMMAND test_ui_visual)
    add_executable(test_ui_audio_coreaudio tests/test_ui_audio_coreaudio.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_coreaudio c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    if(APPLE)
        target_link_libraries(test_ui_audio_coreaudio "-framework AudioToolbox" "-framework CoreFoundation")
    endif()
    add_test(NAME test_ui_audio_coreaudio COMMAND test_ui_audio_coreaudio)

    add_executable(test_ui_audio_opensles tests/test_ui_audio_opensles.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_opensles c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_audio_opensles COMMAND test_ui_audio_opensles)

    add_executable(test_ui_audio_wasapi tests/test_ui_audio_wasapi.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_wasapi c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_audio_wasapi COMMAND test_ui_audio_wasapi)

    add_executable(test_ui_audio_webaudio tests/test_ui_audio_webaudio.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_webaudio c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_audio_webaudio COMMAND test_ui_audio_webaudio)

    add_executable(test_ui_video_avfoundation tests/test_ui_video_avfoundation.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_avfoundation c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    if(APPLE)
        target_link_libraries(test_ui_video_avfoundation "-framework VideoToolbox" "-framework CoreMedia" "-framework CoreFoundation")
    endif()
    add_test(NAME test_ui_video_avfoundation COMMAND test_ui_video_avfoundation)

    add_executable(test_ui_video_mediacodec tests/test_ui_video_mediacodec.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_mediacodec c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_mediacodec COMMAND test_ui_video_mediacodec)

    add_executable(test_ui_video_mf tests/test_ui_video_mf.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_mf c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_video_mf COMMAND test_ui_video_mf)

    add_executable(test_ui_renderer_coregraphics tests/test_ui_renderer_coregraphics.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_coregraphics c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer_coregraphics COMMAND test_ui_renderer_coregraphics)

    add_executable(test_ui_image_base tests/test_ui_image_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_base c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_image_base COMMAND test_ui_image_base)


    add_executable(test_ui_renderer_gdiplus tests/test_ui_renderer_gdiplus.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gdiplus c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_renderer_gdiplus COMMAND test_ui_renderer_gdiplus)

    add_executable(test_ui_runtime_schema tests/test_ui_runtime_schema.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_schema c-multiplatform-cdk_test parson)
    add_test(NAME test_ui_runtime_schema COMMAND test_ui_runtime_schema)

    add_executable(test_ui_component_registry tests/test_ui_component_registry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_component_registry c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_component_registry COMMAND test_ui_component_registry)

    add_executable(test_ui_app_state_registry tests/test_ui_app_state_registry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_app_state_registry c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_app_state_registry COMMAND test_ui_app_state_registry)

    add_executable(test_ui_dynamic_context tests/test_ui_dynamic_context.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dynamic_context c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_dynamic_context COMMAND test_ui_dynamic_context)

    add_executable(test_ui_workflow_engine tests/test_ui_workflow_engine.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_workflow_engine c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_workflow_engine COMMAND test_ui_workflow_engine)

    add_executable(test_ui_runtime_router tests/test_ui_runtime_router.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_router c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_runtime_router COMMAND test_ui_runtime_router)

    add_executable(test_ui_runtime_builder tests/test_ui_runtime_builder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_builder c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_runtime_builder COMMAND test_ui_runtime_builder)

    add_executable(test_ui_runtime_eject tests/test_ui_runtime_eject.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_eject c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_runtime_eject COMMAND test_ui_runtime_eject)

    add_executable(test_ui_app_load tests/test_ui_app_load.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_app_load c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_app_load COMMAND test_ui_app_load)

    add_executable(test_ui_ffi_runtime tests/test_ui_ffi_runtime.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ffi_runtime c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_ffi_runtime COMMAND test_ui_ffi_runtime)

    add_executable(test_ui_runtime_equivalence tests/test_ui_runtime_equivalence.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_equivalence c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_runtime_equivalence COMMAND test_ui_runtime_equivalence)

    add_executable(test_ui_runtime_fuzz tests/test_ui_runtime_fuzz.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_runtime_fuzz c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_runtime_fuzz COMMAND test_ui_runtime_fuzz)

    add_executable(test_ui_design_system tests/test_ui_design_system.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_design_system c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
    add_test(NAME test_ui_design_system COMMAND test_ui_design_system)

    if(UI_ENABLE_DESIGN_SYSTEM_M3)
        add_executable(test_material3_init tests/test_material3_init.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_init c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_init COMMAND test_material3_init)

        add_executable(test_material3_tokens tests/test_material3_tokens.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_tokens c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_tokens COMMAND test_material3_tokens)

        add_executable(test_material3_components tests/test_material3_components.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_components c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_components COMMAND test_material3_components)

        add_executable(test_material3_cva tests/test_material3_cva.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_cva c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_cva COMMAND test_material3_cva)

        add_executable(test_material3_state_delegation tests/test_material3_state_delegation.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_state_delegation c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_state_delegation COMMAND test_material3_state_delegation)

        add_executable(test_material3_color_hct tests/test_material3_color_hct.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_color_hct c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_color_hct COMMAND test_material3_color_hct)

        add_executable(test_material3_spring_physics tests/test_material3_spring_physics.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_spring_physics c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_spring_physics COMMAND test_material3_spring_physics)

        add_executable(test_material3_oom_mock tests/test_material3_oom_mock.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_oom_mock c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_oom_mock COMMAND test_material3_oom_mock)

        add_executable(test_material3_a11y tests/test_material3_a11y.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_a11y c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_a11y COMMAND test_material3_a11y)

        add_executable(test_material3_bidi tests/test_material3_bidi.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_bidi c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_bidi COMMAND test_material3_bidi)

        add_executable(test_material3_action_widgets tests/test_material3_action_widgets.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_action_widgets c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_action_widgets COMMAND test_material3_action_widgets)

        add_executable(test_material3_selection_controls tests/test_material3_selection_controls.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_selection_controls c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_selection_controls COMMAND test_material3_selection_controls)

        add_executable(test_material3_containment tests/test_material3_containment.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_containment c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_containment COMMAND test_material3_containment)

        add_executable(test_material3_shell_layout tests/test_material3_shell_layout.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_shell_layout c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_shell_layout COMMAND test_material3_shell_layout)

        add_executable(test_material3_overlay_menus tests/test_material3_overlay_menus.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_overlay_menus c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_overlay_menus COMMAND test_material3_overlay_menus)

        add_executable(test_material3_data_hierarchy tests/test_material3_data_hierarchy.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_data_hierarchy c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_data_hierarchy COMMAND test_material3_data_hierarchy)

        add_executable(test_material3_media_workspace tests/test_material3_media_workspace.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_media_workspace c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_media_workspace COMMAND test_material3_media_workspace)

        add_executable(test_material3_visual_catalog tests/test_material3_visual_catalog.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_visual_catalog c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_visual_catalog COMMAND test_material3_visual_catalog)

        add_executable(test_material3_top_app_bar tests/test_material3_top_app_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_top_app_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_top_app_bar COMMAND test_material3_top_app_bar)

        add_executable(test_material3_floating_toolbar tests/test_material3_floating_toolbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_floating_toolbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_floating_toolbar COMMAND test_material3_floating_toolbar)

        add_executable(test_material3_fab_menu tests/test_material3_fab_menu.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_fab_menu c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_fab_menu COMMAND test_material3_fab_menu)

        add_executable(test_material3_date_time_pickers tests/test_material3_date_time_pickers.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_date_time_pickers c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_date_time_pickers COMMAND test_material3_date_time_pickers)

        add_executable(test_material3_adaptive tests/test_material3_adaptive.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material3_adaptive c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material3_adaptive COMMAND test_material3_adaptive)
    endif()

    if(UI_ENABLE_DESIGN_SYSTEM_M2)
        add_executable(test_material2_elevation tests/test_material2_elevation.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_elevation c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_elevation COMMAND test_material2_elevation)

        add_executable(test_material2_shape tests/test_material2_shape.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_shape c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_shape COMMAND test_material2_shape)

        add_executable(test_material2_navigation_drawer tests/test_material2_navigation_drawer.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_navigation_drawer c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_navigation_drawer COMMAND test_material2_navigation_drawer)

        add_executable(test_material2_selection_controls tests/test_material2_selection_controls.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_selection_controls c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_selection_controls COMMAND test_material2_selection_controls)

        add_executable(test_material2_text_field tests/test_material2_text_field.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_text_field c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_text_field COMMAND test_material2_text_field)

        add_executable(test_material2_overlays tests/test_material2_overlays.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_overlays c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_overlays COMMAND test_material2_overlays)


        add_executable(test_material2_theme tests/test_material2_theme.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_theme c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_theme COMMAND test_material2_theme)
        add_executable(test_material2_theme_card tests/test_material2_theme_card.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_theme_card c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_theme_card COMMAND test_material2_theme_card)

        add_executable(test_material2_button tests/test_material2_button.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_button c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_button COMMAND test_material2_button)

        add_executable(test_material2_fab tests/test_material2_fab.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_fab c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_fab COMMAND test_material2_fab)

        add_executable(test_material2_card tests/test_material2_card.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_card c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_card COMMAND test_material2_card)

        add_executable(test_material2_top_app_bar tests/test_material2_top_app_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_top_app_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_top_app_bar COMMAND test_material2_top_app_bar)

        add_executable(test_material2_bottom_navigation tests/test_material2_bottom_navigation.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_material2_bottom_navigation c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_material2_bottom_navigation COMMAND test_material2_bottom_navigation)
    endif()

    if(UI_ENABLE_DESIGN_SYSTEM_CUPERTINO)
        add_executable(test_cupertino_init tests/test_cupertino_init.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_init c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_init COMMAND test_cupertino_init)

        add_executable(test_cupertino_tokens tests/test_cupertino_tokens.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_tokens c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_tokens COMMAND test_cupertino_tokens)

        add_executable(test_cupertino_squircle tests/test_cupertino_squircle.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_squircle c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_squircle COMMAND test_cupertino_squircle)

        add_executable(test_cupertino_spring tests/test_cupertino_spring.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_spring c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_spring COMMAND test_cupertino_spring)

        add_executable(test_cupertino_material tests/test_cupertino_material.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_material c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_material COMMAND test_cupertino_material)

        add_executable(test_cupertino_button tests/test_cupertino_button.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_button c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_button COMMAND test_cupertino_button)

        add_executable(test_cupertino_switch tests/test_cupertino_switch.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_switch c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_switch COMMAND test_cupertino_switch)

        add_executable(test_cupertino_slider tests/test_cupertino_slider.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_slider c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_slider COMMAND test_cupertino_slider)

        add_executable(test_cupertino_checkbox tests/test_cupertino_checkbox.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_checkbox c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_checkbox COMMAND test_cupertino_checkbox)

        add_executable(test_cupertino_typography tests/test_cupertino_typography.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_typography c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_typography COMMAND test_cupertino_typography)

        add_executable(test_cupertino_radio tests/test_cupertino_radio.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_radio c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_radio COMMAND test_cupertino_radio)

        add_executable(test_cupertino_segmented_control tests/test_cupertino_segmented_control.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_segmented_control c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_segmented_control COMMAND test_cupertino_segmented_control)

        add_executable(test_cupertino_stepper tests/test_cupertino_stepper.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_stepper c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_stepper COMMAND test_cupertino_stepper)

        add_executable(test_cupertino_form tests/test_cupertino_form.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_form c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_form COMMAND test_cupertino_form)

        add_executable(test_cupertino_text_field tests/test_cupertino_text_field.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_text_field c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_text_field COMMAND test_cupertino_text_field)

        add_executable(test_cupertino_nav_bar tests/test_cupertino_nav_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_nav_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_nav_bar COMMAND test_cupertino_nav_bar)

        add_executable(test_cupertino_tab_bar tests/test_cupertino_tab_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_tab_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_tab_bar COMMAND test_cupertino_tab_bar)

        add_executable(test_cupertino_toolbar tests/test_cupertino_toolbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_toolbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_toolbar COMMAND test_cupertino_toolbar)

        add_executable(test_cupertino_alert tests/test_cupertino_alert.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_alert c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_alert COMMAND test_cupertino_alert)

        add_executable(test_cupertino_sheet tests/test_cupertino_sheet.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_sheet c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_sheet COMMAND test_cupertino_sheet)

        add_executable(test_cupertino_list_tile tests/test_cupertino_list_tile.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_list_tile c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_list_tile COMMAND test_cupertino_list_tile)

        add_executable(test_cupertino_list_section tests/test_cupertino_list_section.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_list_section c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_list_section COMMAND test_cupertino_list_section)

        add_executable(test_cupertino_scrollbar tests/test_cupertino_scrollbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_scrollbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_scrollbar COMMAND test_cupertino_scrollbar)

        add_executable(test_cupertino_search_bar tests/test_cupertino_search_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_search_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_search_bar COMMAND test_cupertino_search_bar)

        add_executable(test_cupertino_refresh tests/test_cupertino_refresh.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_refresh c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_refresh COMMAND test_cupertino_refresh)

        add_executable(test_cupertino_indicator tests/test_cupertino_indicator.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_indicator c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_indicator COMMAND test_cupertino_indicator)

        add_executable(test_cupertino_badge tests/test_cupertino_badge.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_badge c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_badge COMMAND test_cupertino_badge)

        add_executable(test_cupertino_page_control tests/test_cupertino_page_control.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_page_control c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_page_control COMMAND test_cupertino_page_control)

        add_executable(test_cupertino_haptics tests/test_cupertino_haptics.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_haptics c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_haptics COMMAND test_cupertino_haptics)

        add_executable(test_cupertino_activity_rings tests/test_cupertino_activity_rings.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_activity_rings c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_activity_rings COMMAND test_cupertino_activity_rings)

        add_executable(test_cupertino_magnifier tests/test_cupertino_magnifier.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_magnifier c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_magnifier COMMAND test_cupertino_magnifier)

        add_executable(test_cupertino_wheel tests/test_cupertino_wheel.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_wheel c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_wheel COMMAND test_cupertino_wheel)

        add_executable(test_cupertino_apple_pay tests/test_cupertino_apple_pay.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_apple_pay c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_apple_pay COMMAND test_cupertino_apple_pay)

        add_executable(test_cupertino_dynamic_island tests/test_cupertino_dynamic_island.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_dynamic_island c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_dynamic_island COMMAND test_cupertino_dynamic_island)

        add_executable(test_cupertino_menu_bar tests/test_cupertino_menu_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_menu_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_menu_bar COMMAND test_cupertino_menu_bar)

        add_executable(test_cupertino_photo_picker tests/test_cupertino_photo_picker.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_photo_picker c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_photo_picker COMMAND test_cupertino_photo_picker)

        add_executable(test_cupertino_date_picker tests/test_cupertino_date_picker.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_date_picker c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_date_picker COMMAND test_cupertino_date_picker)

        add_executable(test_cupertino_color_well tests/test_cupertino_color_well.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_color_well c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_color_well COMMAND test_cupertino_color_well)

        add_executable(test_cupertino_text_selection_toolbar tests/test_cupertino_text_selection_toolbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_text_selection_toolbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_text_selection_toolbar COMMAND test_cupertino_text_selection_toolbar)

        add_executable(test_cupertino_biometrics tests/test_cupertino_biometrics.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_biometrics c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_biometrics COMMAND test_cupertino_biometrics)

        add_executable(test_cupertino_popover tests/test_cupertino_popover.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_popover c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_popover COMMAND test_cupertino_popover)

        add_executable(test_cupertino_shareplay tests/test_cupertino_shareplay.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_shareplay c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_shareplay COMMAND test_cupertino_shareplay)

        add_executable(test_cupertino_sf_symbols tests/test_cupertino_sf_symbols.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_sf_symbols c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_sf_symbols COMMAND test_cupertino_sf_symbols)

        add_executable(test_cupertino_content_unavailable tests/test_cupertino_content_unavailable.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_content_unavailable c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_content_unavailable COMMAND test_cupertino_content_unavailable)

        add_executable(test_cupertino_popup_surface tests/test_cupertino_popup_surface.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_popup_surface c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_popup_surface COMMAND test_cupertino_popup_surface)

        add_executable(test_cupertino_split_view tests/test_cupertino_split_view.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_split_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_split_view COMMAND test_cupertino_split_view)

        add_executable(test_cupertino_page_scaffold tests/test_cupertino_page_scaffold.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_page_scaffold c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_page_scaffold COMMAND test_cupertino_page_scaffold)

        add_executable(test_cupertino_context_menu tests/test_cupertino_context_menu.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_context_menu c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_context_menu COMMAND test_cupertino_context_menu)

        add_executable(test_cupertino_tab_scaffold tests/test_cupertino_tab_scaffold.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_tab_scaffold c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_tab_scaffold COMMAND test_cupertino_tab_scaffold)

        add_executable(test_cupertino_tab_view tests/test_cupertino_tab_view.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_tab_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_tab_view COMMAND test_cupertino_tab_view)

        add_executable(test_cupertino_activity_view tests/test_cupertino_activity_view.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_activity_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_activity_view COMMAND test_cupertino_activity_view)

        add_executable(test_cupertino_quick_look tests/test_cupertino_quick_look.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_quick_look c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_quick_look COMMAND test_cupertino_quick_look)

        add_executable(test_cupertino_fullscreen_dialog tests/test_cupertino_fullscreen_dialog.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_fullscreen_dialog c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_fullscreen_dialog COMMAND test_cupertino_fullscreen_dialog)

        add_executable(test_cupertino_live_activity tests/test_cupertino_live_activity.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_live_activity c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_live_activity COMMAND test_cupertino_live_activity)

        add_executable(test_cupertino_menu_button tests/test_cupertino_menu_button.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_menu_button c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_menu_button COMMAND test_cupertino_menu_button)

        add_executable(test_cupertino_table_view tests/test_cupertino_table_view.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_table_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_table_view COMMAND test_cupertino_table_view)

        add_executable(test_cupertino_collection_view tests/test_cupertino_collection_view.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_collection_view c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_collection_view COMMAND test_cupertino_collection_view)

        add_executable(test_cupertino_bidi tests/test_cupertino_bidi.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_bidi c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_bidi COMMAND test_cupertino_bidi)

        add_executable(test_cupertino_a11y tests/test_cupertino_a11y.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_a11y c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_a11y COMMAND test_cupertino_a11y)

        add_executable(test_cupertino_gesture tests/test_cupertino_gesture.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_gesture c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_gesture COMMAND test_cupertino_gesture)

        add_executable(test_cupertino_control_center tests/test_cupertino_control_center.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_control_center c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_control_center COMMAND test_cupertino_control_center)

        add_executable(test_cupertino_menu_bar_extra tests/test_cupertino_menu_bar_extra.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_menu_bar_extra c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_menu_bar_extra COMMAND test_cupertino_menu_bar_extra)

        add_executable(test_cupertino_dock_tile tests/test_cupertino_dock_tile.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_dock_tile c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_dock_tile COMMAND test_cupertino_dock_tile)

        add_executable(test_cupertino_desktop_text_selection_toolbar tests/test_cupertino_desktop_text_selection_toolbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_desktop_text_selection_toolbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_desktop_text_selection_toolbar COMMAND test_cupertino_desktop_text_selection_toolbar)

        add_executable(test_cupertino_spell_check_toolbar tests/test_cupertino_spell_check_toolbar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_spell_check_toolbar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_spell_check_toolbar COMMAND test_cupertino_spell_check_toolbar)

        add_executable(test_cupertino_spatial_ornament tests/test_cupertino_spatial_ornament.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_spatial_ornament c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_spatial_ornament COMMAND test_cupertino_spatial_ornament)

        add_executable(test_cupertino_status_bar tests/test_cupertino_status_bar.c tests/ui_test_mock_mem.c)
        target_link_libraries(test_cupertino_status_bar c-multiplatform-cdk_test c-multiplatform-material2_test c-multiplatform-material3_test c-multiplatform-cupertino_test)
        add_test(NAME test_cupertino_status_bar COMMAND test_cupertino_status_bar)
    endif()
endif()


if(C_MULTIPLATFORM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

add_executable(screenshot_tool
    tools/screenshot_tool/main.c
    tools/screenshot_tool/rasterizer.c
    tools/screenshot_tool/design_system_registry.c
    tools/screenshot_tool/ds_material3.c
    tools/screenshot_tool/ds_cupertino.c
    tools/screenshot_tool/ds_fluent2.c
    tools/screenshot_material3/components_render.c
)
target_include_directories(screenshot_tool PRIVATE include vendor tools/screenshot_tool tools/screenshot_material3)
target_link_libraries(screenshot_tool PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
if(NOT MSVC)
    target_link_libraries(screenshot_tool PRIVATE m)
endif()

add_executable(screenshot_material3
    tools/screenshot_material3/main.c
    tools/screenshot_material3/rasterizer.c
    tools/screenshot_material3/components_render.c
)
target_include_directories(screenshot_material3 PRIVATE include vendor tools/screenshot_material3)
target_link_libraries(screenshot_material3 PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
if(NOT MSVC)
    target_link_libraries(screenshot_material3 PRIVATE m)
endif()

if(BUILD_TESTING)
    add_executable(test_screenshot_tool
        tests/test_screenshot_tool.c
        tests/ui_test_mock_mem.c
        tools/screenshot_tool/rasterizer.c
        tools/screenshot_tool/design_system_registry.c
    )
    target_include_directories(test_screenshot_tool PRIVATE include vendor tools/screenshot_tool tests)
    target_compile_definitions(test_screenshot_tool PRIVATE C_MULTIPLATFORM_MALLOC=ui_mock_malloc C_MULTIPLATFORM_FREE=ui_mock_free C_MULTIPLATFORM_REALLOC=ui_mock_realloc C_MULTIPLATFORM_STRDUP=ui_mock_strdup UI_TEST_MOCK_ALLOC=1)
    target_link_libraries(test_screenshot_tool PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    if(NOT MSVC)
        target_link_libraries(test_screenshot_tool PRIVATE m)
    endif()
    add_test(NAME test_screenshot_tool COMMAND test_screenshot_tool)

    add_executable(test_screenshot_rasterizer
        tests/test_screenshot_rasterizer.c
        tests/ui_test_mock_mem.c
        tools/screenshot_tool/rasterizer.c
    )
    target_include_directories(test_screenshot_rasterizer PRIVATE include vendor tools/screenshot_tool tests)
    target_compile_definitions(test_screenshot_rasterizer PRIVATE C_MULTIPLATFORM_MALLOC=ui_mock_malloc C_MULTIPLATFORM_FREE=ui_mock_free C_MULTIPLATFORM_REALLOC=ui_mock_realloc C_MULTIPLATFORM_STRDUP=ui_mock_strdup UI_TEST_MOCK_ALLOC=1)
    target_link_libraries(test_screenshot_rasterizer PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    if(NOT MSVC)
        target_link_libraries(test_screenshot_rasterizer PRIVATE m)
    endif()
    add_test(NAME test_screenshot_rasterizer COMMAND test_screenshot_rasterizer)

    add_test(NAME test_cli_screenshot_tool COMMAND screenshot_tool --help)
    add_test(NAME test_cli_screenshot_material3 COMMAND screenshot_material3 --help)
endif()
add_executable(test_ui_layout_internal tests/test_ui_layout_internal.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_layout_internal PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
if(ENABLE_COVERAGE)
    target_link_options(test_ui_layout_internal PRIVATE --coverage)
    target_compile_options(test_ui_layout_internal PRIVATE --coverage)
endif()
add_test(NAME test_ui_layout_internal COMMAND test_ui_layout_internal)
add_executable(test_ui_cssom_internal tests/test_ui_cssom_internal.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_cssom_internal c-multiplatform-cdk c_cdd c89stringutils c-abstract-http c-orm c-orm-async cfs parson c_str_span)
add_test(NAME test_ui_cssom_internal COMMAND test_ui_cssom_internal)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(src/ui_stb_impl.c PROPERTIES COMPILE_FLAGS "-fno-profile-instr-generate -fno-coverage-mapping")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set_source_files_properties(src/ui_stb_impl.c PROPERTIES COMPILE_FLAGS "-fno-profile-arcs -fno-test-coverage")
endif()


if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING) OR C_MULTIPLATFORM_BUILD_TESTS)

    add_executable(test_example_basic tests/test_example_basic.c tests/ui_test_mock_mem.c)
    target_include_directories(test_example_basic PRIVATE include examples/basic)
    target_compile_definitions(test_example_basic PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_basic PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_basic PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_basic PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_example_basic COMMAND test_example_basic)

    add_executable(test_example_window_basic tests/test_example_window_basic.c tests/ui_test_mock_mem.c)
    target_include_directories(test_example_window_basic PRIVATE include examples/window_basic)
    target_compile_definitions(test_example_window_basic PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_window_basic PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_window_basic PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_window_basic PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_example_window_basic COMMAND test_example_window_basic)

    add_executable(test_example_responsive_mosaic tests/test_example_responsive_mosaic.c tests/ui_test_mock_mem.c)
    target_include_directories(test_example_responsive_mosaic PRIVATE include examples/responsive_mosaic)
    target_compile_definitions(test_example_responsive_mosaic PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_responsive_mosaic PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_responsive_mosaic PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_responsive_mosaic PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_example_responsive_mosaic COMMAND test_example_responsive_mosaic)

    add_executable(test_example_unstyled_components tests/test_example_unstyled_components.c tests/ui_test_mock_mem.c)
    target_include_directories(test_example_unstyled_components PRIVATE include examples/unstyled_components)
    target_compile_definitions(test_example_unstyled_components PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_unstyled_components PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_unstyled_components PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_unstyled_components PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_example_unstyled_components COMMAND test_example_unstyled_components)

    add_executable(test_example_auth_flow tests/test_example_auth_flow.c tests/ui_test_mock_mem.c examples/auth_flow/views.c examples/auth_flow/i18n_dict.c examples/auth_flow/mock_backend.c)
    target_include_directories(test_example_auth_flow PRIVATE include examples/auth_flow)
    target_compile_definitions(test_example_auth_flow PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_auth_flow PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_auth_flow PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_auth_flow PRIVATE c-multiplatform-cdk_test)
    add_test(NAME test_example_auth_flow COMMAND test_example_auth_flow)

    add_executable(test_example_oauth2_sync_flow tests/test_example_oauth2_sync_flow.c tests/ui_test_mock_mem.c examples/oauth2_sync_flow/oauth2_types.c examples/oauth2_sync_flow/offline_store.c examples/oauth2_sync_flow/oauth2_client.c examples/oauth2_sync_flow/app_state.c examples/oauth2_sync_flow/ui_dl_list.c examples/oauth2_sync_flow/views.c)
    target_include_directories(test_example_oauth2_sync_flow PRIVATE include examples/oauth2_sync_flow)
    target_compile_definitions(test_example_oauth2_sync_flow PRIVATE CI_TEST_RUN=1)
    if(MSVC)
        target_compile_options(test_example_oauth2_sync_flow PRIVATE /W4 /WX /Gy)
    else()
        target_compile_options(test_example_oauth2_sync_flow PRIVATE -pedantic -Wall -Wextra -Werror -Wdeclaration-after-statement)
    endif()
    target_link_libraries(test_example_oauth2_sync_flow PRIVATE c-multiplatform-cdk_test parson)
    add_test(NAME test_example_oauth2_sync_flow COMMAND test_example_oauth2_sync_flow)

    add_executable(test_compose_material_catalog
        tests/test_compose_material_catalog.c
        tests/test_sampler_home.c
        tests/ui_test_mock_mem.c
        examples/sampler/src/sampler/sampler_error.c
        examples/sampler/src/sampler/sampler_models.c
        examples/sampler/src/sampler/sampler_theme.c
        examples/sampler/src/sampler/sampler_preferences.c
        examples/sampler/src/sampler/sampler_nav.c
        examples/sampler/src/sampler/sampler_top_app_bar.c
        examples/sampler/src/sampler/sampler_specification_screen.c
        examples/sampler/src/sampler/sampler_home.c
        examples/sampler/src/sampler/sampler_component_screen.c
        examples/sampler/src/sampler/sampler_example_screen.c
        examples/sampler/src/sampler/sampler_theme_picker.c
        examples/sampler/src/sampler/m2/m2_home.c
        examples/sampler/src/sampler/samples/sample_buttons.c
        examples/sampler/src/sampler/samples/sample_stub.c
    )
    target_include_directories(test_compose_material_catalog PRIVATE include vendor tests src examples/sampler/include)
    target_compile_definitions(test_compose_material_catalog PRIVATE
        C_MULTIPLATFORM_MALLOC=ui_mock_malloc
        C_MULTIPLATFORM_FREE=ui_mock_free
        C_MULTIPLATFORM_REALLOC=ui_mock_realloc
        C_MULTIPLATFORM_STRDUP=ui_mock_strdup
        UI_TEST_MOCK_ALLOC=1
    )
    if(MSVC)
        target_compile_options(test_compose_material_catalog PRIVATE /W4 /WX /Za)
    else()
        target_compile_options(test_compose_material_catalog PRIVATE -Wall -Wextra -Wpedantic -Werror -Wdeclaration-after-statement -std=c89)
    endif()
    target_link_libraries(test_compose_material_catalog PRIVATE c-multiplatform-cdk c-multiplatform-material2 c-multiplatform-material3 c-multiplatform-cupertino)
    if(NOT MSVC)
        target_link_libraries(test_compose_material_catalog PRIVATE m)
    endif()
    add_test(NAME test_compose_material_catalog COMMAND test_compose_material_catalog)
endif()


if(APPLE)


endif()



if(APPLE)
    target_link_libraries(c-multiplatform-cdk PRIVATE "-framework AudioToolbox" "-framework VideoToolbox" "-framework CoreMedia" "-framework CoreFoundation")
    target_link_libraries(c-multiplatform-cdk_test PRIVATE "-framework AudioToolbox" "-framework VideoToolbox" "-framework CoreMedia" "-framework CoreFoundation")
endif()
