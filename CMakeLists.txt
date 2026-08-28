cmake_minimum_required(VERSION 3.15)
project(c-multiplatform-engine C)
if (MSVC AND CMAKE_HOST_UNIX)
    # Workaround for MSVC hanging under Wine on macOS/Linux due to mspdbsrv.exe and incremental linking
    # Replace flags in default debug variables
    string(REPLACE "/debug" "/DEBUG:NONE" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    string(REPLACE "/debug" "/DEBUG:NONE" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
    string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # Propagate to TryCompile
    list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
         CMAKE_EXE_LINKER_FLAGS_DEBUG
         CMAKE_SHARED_LINKER_FLAGS_DEBUG
         CMAKE_C_FLAGS_DEBUG
         CMAKE_CXX_FLAGS_DEBUG)

    # Forcefully apply to all downstream targets, overriding any individual target settings
    add_compile_options("/Z7")
    add_link_options("/DEBUG:NONE" "/INCREMENTAL:NO")
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
option(UI_SHARED_LIB "Build as Shared Library" OFF)
option(UI_MSVC_RUNTIME_CHECKS "Enable MSVC /RTC1, /RTCs, /RTCu" OFF)
option(BUILD_VISUAL_TESTS "Build and run visual fuzzy-match tests" ON)

option(UI_ENABLE_ASAN "Enable AddressSanitizer (ASAN)" OFF)

if(UI_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(UI_CRT_STATIC AND MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
    set(UI_CRT_STATIC OFF)
elseif(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded")
    set(UI_CRT_STATIC ON)
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

if(UI_MSVC_RUNTIME_CHECKS AND MSVC)
    add_compile_options(/RTC1 /RTCs /RTCu)
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
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Exported functions and methods
    set(UI_WASM_EXPORTED_FUNCS "_main,_malloc,_free,_ui_engine_tick,_ui_web_bridge_dispatch_event,_ui_web_bridge_dispatch_resize,_ui_web_bridge_dispatch_key")
    set(UI_WASM_RUNTIME_METHODS "ccall,cwrap,UTF8ToString,stringToUTF8,setValue,getValue")

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -gsource-map -Wno-c99-extensions -Wno-missing-prototypes")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O0 -gsource-map --source-map-base http://localhost:8080/")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -Wno-c99-extensions -Wno-missing-prototypes")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3 -flto")
    endif()

    if(NOT UI_SINGLE_THREADED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_PTHREADS=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4")
    endif()

    # Linker specific flags
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=4GB")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=${UI_WASM_EXPORTED_FUNCS} -s EXPORTED_RUNTIME_METHODS=${UI_WASM_RUNTIME_METHODS}")
endif()


if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gy /wd4701")



    # Target-specific warnings will be added later

else()
    # Target-specific warnings will be added later
endif()

if(UNIX AND NOT APPLE AND NOT CYGWIN AND NOT EMSCRIPTEN)
    if(UI_ENABLE_WAYLAND)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl egl wayland-cursor xkbcommon)

        find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
        execute_process(
            COMMAND ${WAYLAND_SCANNER} client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h
        )
        execute_process(
            COMMAND ${WAYLAND_SCANNER} private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c
        )
        include_directories(${CMAKE_CURRENT_BINARY_DIR})

        set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_wayland.c ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c)
        set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c PROPERTIES COMPILE_FLAGS "-Wno-pedantic -Wno-missing-prototypes -Wno-strict-prototypes -Wno-unused-parameter -Dinline=__inline__")
        set_source_files_properties(src/ui_window_backend_linux_wayland.c PROPERTIES COMPILE_FLAGS "-Wno-pedantic -Wno-missing-prototypes -Wno-strict-prototypes -Wno-unused-parameter -Dinline=__inline__")

    else()
        set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c)
    endif()
else()
    set(UI_WINDOW_BACKEND_LINUX_SRC src/ui_window_backend_linux_x11.c)
endif()

include(cmake/PlatformLinkages.cmake)

# Library Target
if(UI_SHARED_LIB)
    add_library(ui_engine SHARED src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_audio_alsa.c src/ui_audio_coreaudio.c src/ui_audio_opensles.c src/ui_audio_wasapi.c src/ui_audio_webaudio.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_jpeg.c src/ui_image_png.c src/ui_image_webp.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_video_avfoundation.c src/ui_video_ffmpeg.c src/ui_video_mediacodec.c src/ui_video_mf.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c)
else()
    add_library(ui_engine STATIC src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_audio_alsa.c src/ui_audio_coreaudio.c src/ui_audio_opensles.c src/ui_audio_wasapi.c src/ui_audio_webaudio.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_jpeg.c src/ui_image_png.c src/ui_image_webp.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_video_avfoundation.c src/ui_video_ffmpeg.c src/ui_video_mediacodec.c src/ui_video_mf.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c)
endif()

target_link_platform_libraries(ui_engine PUBLIC)

if(NOT UI_SINGLE_THREADED)
    target_link_libraries(ui_engine PRIVATE Threads::Threads)
endif()

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
    target_compile_options(ui_engine PRIVATE /W4 /WX /Gy /wd4702 /wd4716 /wd4033 /wd4133 /wd4024 /wd4047 /wd4715 /wd4113)

else()
    target_compile_options(ui_engine PRIVATE -pedantic -Wall -Werror  )
    if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
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
    set(BUILD_TESTS OFF CACHE BOOL "Disable testing for dependencies" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "Disable examples for dependencies" FORCE)

    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY "https://github.com/${GITHUB_REPO}.git"
        GIT_TAG "master"
    )
    FetchContent_MakeAvailable(${NAME})

    # Re-enable testing for the main project
    set(BUILD_TESTING ON CACHE BOOL "Enable testing for main project" FORCE)
endmacro()

if(NOT MSVC)
    set(OLD_CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error -Wno-error=format-nonliteral -Wno-error=sign-conversion -Wno-long-long -Wno-error=overlength-strings -Wno-error=shorten-64-to-32 -Wno-error=float-conversion -Wno-error=implicit-int-float-conversion -Wno-error=macro-redefined -Wno-macro-redefined")
endif()

if(NOT TARGET cfs)
    UI_FetchDependencyLocalOrArchive(cfs "c-fs" "SamuelMarks/c-fs")
endif()

UI_FetchDependencyLocalOrArchive(parson "parson" "SamuelMarks/parson")
UI_FetchDependencyLocalOrArchive(c_cdd "cdd-c" "SamuelMarks/cdd-c")
UI_FetchDependencyLocalOrArchive(c-abstract-http "c-abstract-http" "SamuelMarks/c-abstract-http")
UI_FetchDependencyLocalOrArchive(c_orm "c-orm" "SamuelMarks/c-orm")

foreach(dep c_cdd c_str_span c-abstract-http c-orm c89stringutils parson cfs)
    if(TARGET ${dep})
        if(NOT MSVC)
            set_property(TARGET ${dep} PROPERTY COMPILE_OPTIONS "")
            set_property(TARGET ${dep} PROPERTY INTERFACE_COMPILE_OPTIONS "")
        endif()
        set_property(TARGET ${dep} PROPERTY LINK_OPTIONS "")
        set_property(TARGET ${dep} PROPERTY INTERFACE_LINK_OPTIONS "")
    endif()
endforeach()
if(TARGET c_cdd_compiler_flags)
    set_property(TARGET c_cdd_compiler_flags PROPERTY LINK_OPTIONS "")
    set_property(TARGET c_cdd_compiler_flags PROPERTY INTERFACE_LINK_OPTIONS "")
endif()

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${OLD_CMAKE_C_FLAGS}")
endif()
if(TARGET c-orm AND TARGET c_str_span)
    target_include_directories(c-orm PUBLIC build/_deps/c_cdd-src/src)
    target_link_libraries(c-orm PUBLIC c_str_span c_cdd)
endif()


# Strip coverage flags that bleed from dependencies in Debug mode
foreach(DEP_TARGET c_cdd c-abstract-http cfs c-orm c-orm-async c_str_span c89stringutils parson)
    if(TARGET ${DEP_TARGET})
        set_target_properties(${DEP_TARGET} PROPERTIES INTERFACE_COMPILE_OPTIONS "")
        set_target_properties(${DEP_TARGET} PROPERTIES INTERFACE_LINK_OPTIONS "")
    endif()
endforeach()

if(TARGET c_cdd AND MSVC)
    target_compile_options(c_cdd PRIVATE "/wd4005")
endif()
if(TARGET c_cdd)
    target_compile_definitions(c_cdd PUBLIC CDD_BUILD_TESTS=1)
endif()

target_link_libraries(ui_engine PRIVATE c-abstract-http cfs c-orm)
target_include_directories(ui_engine PUBLIC include)

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
    add_library(ui_engine_test STATIC src/ui_execution_context.c src/ui_tick_engine.c src/ui_thread_pool.c src/ui_reactor.c src/ui_promise.c src/ui_atomic.c src/ui_ring_buffer.c src/ui_timer.c src/ui_arena.c src/ui_pool.c src/ui_engine_core.c src/ui_event.c src/ui_signal.c src/ui_reactive_graph.c src/ui_computed.c src/ui_effect.c src/ui_color_space.c src/ui_color_quantization.c src/ui_design_tokens.c src/ui_tonal_palette.c src/ui_theme_provider.c src/ui_theme_manager.c src/ui_safe_area_manager.c src/ui_shape_family.c src/ui_typography_scale.c src/ui_state_layer.c src/ui_elevation.c src/ui_focus_ring.c src/ui_handle_manager.c src/ui_window_backend_win32.c src/ui_window_backend_macos.c src/ui_window_backend_ios.c src/ui_window_backend_android.c ${UI_WINDOW_BACKEND_LINUX_SRC} src/ui_window_backend_web.c src/ui_dom_node.c
    src/ui_fs.c src/ui_web_bridge.c src/ui_cssom.c src/ui_layout.c src/ui_css_parser.c src/ui_css_values.c src/ui_css_env.c src/ui_css_contain.c src/ui_css_overflow.c src/ui_css_lists.c src/ui_css_content.c src/ui_css_logical.c src/ui_css_scrollbars.c src/ui_css_round_display.c src/ui_css_ui.c src/ui_css_forms.c src/ui_css_motion.c src/ui_css_view_transition.c src/ui_css_speech.c src/ui_css_scroll_snap.c src/ui_css_scroll_anchoring.c src/ui_css_spatial_nav.c src/ui_css_highlight_api.c src/ui_css_page.c src/ui_css_gcpm.c src/ui_css_page_floats.c src/ui_cssom_api.c src/ui_cssom_view.c src/ui_component.c src/ui_modifier.c src/ui_drag_drop.c src/ui_list_base.c src/ui_data_source.c src/ui_input_mask.c src/ui_i18n.c src/ui_tree_grid_base.c src/ui_side_sheet_base.c src/ui_dockable_layout_base.c src/ui_hotkey_registry.c src/ui_scroll_spy.c src/ui_resizable_behavior.c src/ui_virtual_keyboard.c src/ui_predictive_back.c src/ui_nav_rail_base.c src/ui_bottom_nav_base.c src/ui_segmented_control_base.c src/ui_listbox_base.c src/ui_autocomplete_base.c src/ui_popover_base.c src/ui_clipboard.c src/ui_layout_observer.c src/ui_scroll_dispatcher.c src/ui_key_manager.c src/ui_textarea_autosize.c src/ui_ripple_base.c src/ui_alert_base.c src/ui_button_base.c src/ui_split_button_base.c src/ui_top_app_bar_base.c src/ui_bottom_app_bar_base.c src/ui_section_index_base.c src/ui_toolbar_base.c src/ui_split_pane_base.c src/ui_grid_list_base.c src/ui_fab_base.c src/ui_icon_base.c src/ui_skeleton_base.c src/ui_avatar_group_base.c src/ui_avatar_base.c src/ui_timeline_base.c src/ui_slide_toggle_base.c src/ui_timepicker_base.c src/ui_toggle_base.c src/ui_checkbox_base.c src/ui_file_uploader_base.c src/ui_os_dialogs.c src/ui_tray_manager.c src/ui_auth.c src/ui_share.c src/ui_sensor_manager.c src/ui_intersection_observer.c src/ui_mutation_observer.c src/ui_coercion_utils.c src/ui_focus_trap.c src/ui_platform_detector.c src/ui_card_base.c src/ui_divider_base.c src/ui_input_base.c src/ui_form_field_base.c src/ui_sidenav_base.c src/ui_bottom_sheet_base.c src/ui_action_sheet_base.c src/ui_snackbar_base.c src/ui_scroll_base.c src/ui_slider_base.c src/ui_range_slider_base.c src/ui_wheel_picker_base.c src/ui_pull_to_refresh_base.c src/ui_text_node_base.c src/ui_spin_button_base.c src/ui_select_base.c src/ui_dialog_base.c src/ui_disclosure_base.c src/ui_audio_wasapi.c src/ui_audio_coreaudio.c src/ui_audio_alsa.c src/ui_audio_opensles.c src/ui_audio_webaudio.c src/ui_video_mf.c src/ui_video_avfoundation.c src/ui_video_mediacodec.c src/ui_video_ffmpeg.c src/ui_av_sync.c src/ui_image_decoder.c src/ui_image_base.c src/ui_transfer_list_base.c src/ui_search_bar_base.c src/ui_swipe_action_base.c src/ui_speed_dial_base.c src/ui_video_player_base.c src/ui_command_palette_base.c src/ui_image_png.c src/ui_image_jpeg.c src/ui_image_webp.c src/ui_svg.c src/ui_asset_streamer.c src/ui_shader_manager.c src/ui_renderer.c src/ui_renderer_gdiplus.c src/ui_renderer_coregraphics.c src/ui_renderer_gles_fallback.c src/ui_renderer_gl1.c src/ui_renderer_gles2.c src/ui_font_manager.c src/ui_stb_impl.c src/ui_text_layout.c src/ui_text_layout_hb.c src/ui_video_shaders.c src/ui_effects_shaders.c src/ui_vibrancy.c src/ui_focus_manager.c src/ui_keyboard_responder.c src/ui_aria.c src/ui_gesture.c src/ui_haptics.c src/ui_overlay_director.c src/ui_geometry.c src/ui_geometry_anchor.c src/ui_backdrop.c src/ui_router.c src/ui_e2e_headless.c src/ui_e2e_headful.c src/ui_web_animation.c src/ui_spring.c src/ui_test_visual.c src/ui_audio_alsa.c src/ui_menu_base.c src/ui_stepper_base.c src/ui_table_base.c src/ui_tabs_base.c src/ui_toast_manager_base.c src/ui_tooltip_base.c src/ui_tree_base.c src/ui_selection_model.c src/ui_pagination_base.c src/ui_sort_header_base.c src/ui_calendar_base.c src/ui_datepicker_base.c src/ui_date_range_picker_base.c src/ui_radio_group_base.c src/ui_chips_base.c src/ui_color_picker_base.c src/ui_virtual_scroll_base.c src/ui_breakpoint_observer.c src/ui_carousel_base.c src/ui_breadcrumbs_base.c src/ui_portal.c src/ui_context_menu_base.c src/ui_live_announcer.c src/ui_bidi_manager.c src/ui_progress_base.c src/ui_accordion_base.c src/ui_surface_base.c src/ui_link_base.c src/ui_meter_base.c src/ui_page_control_base.c src/ui_banner_base.c src/ui_button_group_base.c src/ui_empty_state_base.c src/ui_scaffold_base.c src/ui_menubar_base.c src/ui_badge_base.c src/ui_ink_base.c src/ui_webview_base.c src/ui_camera_base.c src/ui_map_view_base.c src/ui_rating_base.c src/ui_coachmark_base.c src/ui_rich_text_base.c src/ui_label_base.c src/ui_aspect_ratio_base.c src/ui_masonry_layout_base.c src/ui_preferences.c src/ui_pin_input_base.c src/ui_hover_card_base.c src/ui_titlebar_base.c src/ui_window_manager_base.c src/ui_rich_text_editor_base.c src/ui_datagrid_base.c src/ui_canonical_layout_base.c src/ui_window_controls_base.c src/ui_compositor_material_base.c src/ui_loupe_base.c src/ui_ribbon_base.c src/ui_miller_columns_base.c src/ui_property_grid_base.c src/ui_syntax_surface_base.c src/ui_node_graph_base.c src/ui_chart_base.c src/ui_chat_bubble_base.c src/ui_form_control.c src/ui_form_node.c src/ui_form_group.c src/ui_form_array.c src/ui_form_builder.c)
    target_include_directories(ui_engine_test PUBLIC include)
    target_compile_definitions(ui_engine_test PUBLIC C_MULTIPLATFORM_MALLOC=ui_mock_malloc C_MULTIPLATFORM_FREE=ui_mock_free C_MULTIPLATFORM_REALLOC=ui_mock_realloc C_MULTIPLATFORM_STRDUP=ui_mock_strdup UI_TEST_MOCK_ALLOC=1)
    add_dependencies(ui_engine_test ui_engine)
    target_link_platform_libraries(ui_engine_test PUBLIC)
    if(NOT UI_SINGLE_THREADED)
        target_link_libraries(ui_engine_test PRIVATE Threads::Threads)
    endif()

    target_link_libraries(ui_engine_test PRIVATE c-abstract-http cfs c-orm)

    if(MSVC)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
        target_compile_options(ui_engine_test PRIVATE /W4 /WX /Gy /wd4702 /wd4716 /wd4033 /wd4133 /wd4024 /wd4047 /wd4715 /wd4113)

    else()
        target_compile_options(ui_engine_test PRIVATE -pedantic -Wall -Werror  )
        if(ENABLE_COVERAGE)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
        endif()
    endif()

    add_executable(test_ui_execution_context tests/test_ui_execution_context.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_execution_context ui_engine_test)
    add_test(NAME test_ui_execution_context COMMAND test_ui_execution_context)

    add_executable(test_ui_tick_engine tests/test_ui_tick_engine.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tick_engine ui_engine_test)
    add_test(NAME test_ui_tick_engine COMMAND test_ui_tick_engine)

    add_executable(test_ui_thread_pool tests/test_ui_thread_pool.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_thread_pool ui_engine_test)
    add_test(NAME test_ui_thread_pool COMMAND test_ui_thread_pool)

    add_executable(test_ui_reactor tests/test_ui_reactor.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_reactor ui_engine_test)
    add_test(NAME test_ui_reactor COMMAND test_ui_reactor)

    add_executable(test_ui_reactor_real tests/test_ui_reactor_real.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_reactor_real ui_engine)
    if(ENABLE_COVERAGE)
        target_link_options(test_ui_reactor_real PRIVATE --coverage)
        target_compile_options(test_ui_reactor_real PRIVATE --coverage)
    endif()
    add_test(NAME test_ui_reactor_real COMMAND test_ui_reactor_real)

    add_executable(test_ui_promise tests/test_ui_promise.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_promise ui_engine_test)
    add_test(NAME test_ui_promise COMMAND test_ui_promise)

    add_executable(test_ui_atomic tests/test_ui_atomic.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_atomic ui_engine_test)
    add_test(NAME test_ui_atomic COMMAND test_ui_atomic)

    add_executable(test_ui_ring_buffer tests/test_ui_ring_buffer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ring_buffer ui_engine_test)
    add_test(NAME test_ui_ring_buffer COMMAND test_ui_ring_buffer)

    add_executable(test_ui_timer tests/test_ui_timer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timer ui_engine_test)
    add_test(NAME test_ui_timer COMMAND test_ui_timer)

    add_executable(test_ui_types tests/test_ui_types.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_types ui_engine_test)
    add_test(NAME test_ui_types COMMAND test_ui_types)

    add_executable(test_ui_arena tests/test_ui_arena.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_arena ui_engine_test)
    add_test(NAME test_ui_arena COMMAND test_ui_arena)

    add_executable(test_ui_pool tests/test_ui_pool.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pool ui_engine_test)
    add_test(NAME test_ui_pool COMMAND test_ui_pool)

    add_executable(test_ui_engine_core tests/test_ui_engine_core.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_engine_core ui_engine_test)
    add_test(NAME test_ui_engine_core COMMAND test_ui_engine_core)

    add_executable(test_ui_handle_manager tests/test_ui_handle_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_handle_manager ui_engine_test)
    add_test(NAME test_ui_handle_manager COMMAND test_ui_handle_manager)

    add_executable(test_ui_window_backend_win32 tests/test_ui_window_backend_win32.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_win32 ui_engine_test)
    add_test(NAME test_ui_window_backend_win32 COMMAND test_ui_window_backend_win32)

    add_executable(test_ui_window_backend tests/test_ui_window_backend.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend ui_engine_test)
    add_test(NAME test_ui_window_backend COMMAND test_ui_window_backend)

    add_executable(test_ui_window_backend_android tests/test_ui_window_backend_android.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_android ui_engine_test)
    add_test(NAME test_ui_window_backend_android COMMAND test_ui_window_backend_android)

    add_executable(test_ui_window_backend_ios tests/test_ui_window_backend_ios.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_ios ui_engine_test)
    add_test(NAME test_ui_window_backend_ios COMMAND test_ui_window_backend_ios)

    add_executable(test_ui_window_backend_linux tests/test_ui_window_backend_linux.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_linux ui_engine_test)
    add_test(NAME test_ui_window_backend_linux COMMAND test_ui_window_backend_linux)

    add_executable(test_ui_window_backend_macos tests/test_ui_window_backend_macos.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_macos ui_engine_test)
    add_test(NAME test_ui_window_backend_macos COMMAND test_ui_window_backend_macos)

    add_executable(test_ui_window_backend_web tests/test_ui_window_backend_web.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_web ui_engine_test)
    add_test(NAME test_ui_window_backend_web COMMAND test_ui_window_backend_web)

    add_executable(test_ui_data_source tests/test_ui_data_source.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_data_source ui_engine_test)
    add_test(NAME test_ui_data_source COMMAND test_ui_data_source)

    add_executable(test_ui_input_mask tests/test_ui_input_mask.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_input_mask ui_engine_test)
    add_test(NAME test_ui_input_mask COMMAND test_ui_input_mask)

    add_executable(test_ui_i18n tests/test_ui_i18n.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_i18n ui_engine_test)
    add_test(NAME test_ui_i18n COMMAND test_ui_i18n)

    add_executable(test_ui_video_ffmpeg tests/test_ui_video_ffmpeg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_ffmpeg ui_engine_test)
    add_test(NAME test_ui_video_ffmpeg COMMAND test_ui_video_ffmpeg)

    add_executable(test_ui_tree_grid_base tests/test_ui_tree_grid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tree_grid_base ui_engine_test)
    add_test(NAME test_ui_tree_grid_base COMMAND test_ui_tree_grid_base)

    add_executable(test_ui_side_sheet_base tests/test_ui_side_sheet_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_side_sheet_base ui_engine_test)
    add_test(NAME test_ui_side_sheet_base COMMAND test_ui_side_sheet_base)

    add_executable(test_ui_dockable_layout_base tests/test_ui_dockable_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dockable_layout_base ui_engine_test)
    add_test(NAME test_ui_dockable_layout_base COMMAND test_ui_dockable_layout_base)

    add_executable(test_ui_hotkey_registry tests/test_ui_hotkey_registry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_hotkey_registry ui_engine_test)
    add_test(NAME test_ui_hotkey_registry COMMAND test_ui_hotkey_registry)

    add_executable(test_ui_scroll_spy tests/test_ui_scroll_spy.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_spy ui_engine_test)
    add_test(NAME test_ui_scroll_spy COMMAND test_ui_scroll_spy)

    add_executable(test_ui_resizable_behavior tests/test_ui_resizable_behavior.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_resizable_behavior ui_engine_test)
    add_test(NAME test_ui_resizable_behavior COMMAND test_ui_resizable_behavior)

    add_executable(test_ui_virtual_keyboard tests/test_ui_virtual_keyboard.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_virtual_keyboard ui_engine_test)
    add_test(NAME test_ui_virtual_keyboard COMMAND test_ui_virtual_keyboard)

    add_executable(test_ui_preferences tests/test_ui_preferences.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_preferences ui_engine_test)
    add_test(NAME test_ui_preferences COMMAND test_ui_preferences)
    add_executable(test_ui_predictive_back tests/test_ui_predictive_back.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_predictive_back ui_engine_test)
    add_test(NAME test_ui_predictive_back COMMAND test_ui_predictive_back)

    add_executable(test_ui_dom_node tests/test_ui_dom_node.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_audio_alsa tests/test_ui_audio_alsa.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_alsa ui_engine_test cfs parson c_cdd c-orm)
    add_test(NAME test_ui_audio_alsa COMMAND test_ui_audio_alsa)

    add_executable(test_ui_event tests/test_ui_event.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_event ui_engine_test cfs parson c_cdd c-orm)
    add_test(NAME test_ui_event COMMAND test_ui_event)
    target_link_libraries(test_ui_dom_node ui_engine_test)
    add_test(NAME test_ui_dom_node COMMAND test_ui_dom_node)

    add_executable(test_ui_cssom tests/test_ui_cssom.c tests/test_ui_cssom_selectors.c tests/test_ui_cssom_rules.c tests/test_ui_cssom_declarations.c tests/test_ui_cssom_stylesheets.c tests/test_ui_cssom_serialization.c tests/test_ui_cssom_memory.c tests/test_ui_cssom_vars.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom ui_engine_test)
    add_test(NAME test_ui_cssom COMMAND test_ui_cssom)


    add_executable(test_ui_fs tests/test_ui_fs.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_fs ui_engine_test)
    add_test(NAME test_ui_fs COMMAND test_ui_fs)

    add_executable(test_ui_layout tests/test_ui_layout.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_layout ui_engine_test)
    add_test(NAME test_ui_layout COMMAND test_ui_layout)

    add_executable(test_ui_css_parser tests/test_ui_css_parser.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_parser ui_engine_test)
    add_test(NAME test_ui_css_parser COMMAND test_ui_css_parser)

    add_executable(test_ui_css_values tests/test_ui_css_values.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_values ui_engine_test)
    add_test(NAME test_ui_css_values COMMAND test_ui_css_values)

    add_executable(test_ui_css_env tests/test_ui_css_env.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_env ui_engine_test)
    add_test(NAME test_ui_css_env COMMAND test_ui_css_env)

    add_executable(test_ui_css_contain tests/test_ui_css_contain.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_contain ui_engine_test)
    add_test(NAME test_ui_css_contain COMMAND test_ui_css_contain)

    add_executable(test_ui_css_overflow tests/test_ui_css_overflow.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_overflow ui_engine_test)
    add_test(NAME test_ui_css_overflow COMMAND test_ui_css_overflow)

    add_executable(test_ui_css_lists tests/test_ui_css_lists.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_lists ui_engine_test)
    add_test(NAME test_ui_css_lists COMMAND test_ui_css_lists)

    add_executable(test_ui_css_content tests/test_ui_css_content.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_content ui_engine_test)
    add_test(NAME test_ui_css_content COMMAND test_ui_css_content)

    add_executable(test_ui_css_logical tests/test_ui_css_logical.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_logical ui_engine_test)
    add_test(NAME test_ui_css_logical COMMAND test_ui_css_logical)

    add_executable(test_ui_css_scrollbars tests/test_ui_css_scrollbars.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scrollbars ui_engine_test)
    add_test(NAME test_ui_css_scrollbars COMMAND test_ui_css_scrollbars)

    add_executable(test_ui_css_round_display tests/test_ui_css_round_display.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_round_display ui_engine_test)
    add_test(NAME test_ui_css_round_display COMMAND test_ui_css_round_display)

    add_executable(test_ui_css_ui tests/test_ui_css_ui.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_ui ui_engine_test)
    add_test(NAME test_ui_css_ui COMMAND test_ui_css_ui)

    add_executable(test_ui_css_forms tests/test_ui_css_forms.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_forms ui_engine_test)
    add_test(NAME test_ui_css_forms COMMAND test_ui_css_forms)

    add_executable(test_ui_css_motion tests/test_ui_css_motion.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_motion ui_engine_test)
    add_test(NAME test_ui_css_motion COMMAND test_ui_css_motion)

    add_executable(test_ui_css_view_transition tests/test_ui_css_view_transition.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_view_transition ui_engine_test)
    add_test(NAME test_ui_css_view_transition COMMAND test_ui_css_view_transition)

    add_executable(test_ui_component tests/test_ui_component.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_component ui_engine_test)
    add_test(NAME test_ui_component COMMAND test_ui_component)

    add_executable(test_ui_media tests/test_ui_media.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_media ui_engine_test)
    add_test(NAME test_ui_media COMMAND test_ui_media)

    add_executable(test_ui_audio_sink tests/test_ui_audio_sink.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_sink ui_engine_test)
    add_test(NAME test_ui_audio_sink COMMAND test_ui_audio_sink)

    add_executable(test_ui_video_decoder tests/test_ui_video_decoder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_decoder ui_engine_test)
    add_test(NAME test_ui_video_decoder COMMAND test_ui_video_decoder)

    add_executable(test_ui_av_sync tests/test_ui_av_sync.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_av_sync ui_engine_test)
    add_test(NAME test_ui_av_sync COMMAND test_ui_av_sync)

    add_executable(test_ui_image_decoder tests/test_ui_image_decoder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_decoder ui_engine_test)
    add_test(NAME test_ui_image_decoder COMMAND test_ui_image_decoder)

    add_executable(test_ui_image_png tests/test_ui_image_png.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_png ui_engine_test)
    add_test(NAME test_ui_image_png COMMAND test_ui_image_png)

    add_executable(test_ui_image_jpeg tests/test_ui_image_jpeg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_jpeg ui_engine_test)
    add_test(NAME test_ui_image_jpeg COMMAND test_ui_image_jpeg)

    add_executable(test_ui_image_webp tests/test_ui_image_webp.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_webp ui_engine_test)
    add_test(NAME test_ui_image_webp COMMAND test_ui_image_webp)

    add_executable(test_ui_renderer_gles_fallback tests/test_ui_renderer_gles_fallback.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gles_fallback ui_engine_test)
    add_test(NAME test_ui_renderer_gles_fallback COMMAND test_ui_renderer_gles_fallback)

    add_executable(test_ui_svg tests/test_ui_svg.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_svg ui_engine_test)
    add_test(NAME test_ui_svg COMMAND test_ui_svg)

    add_executable(test_ui_asset_streamer tests/test_ui_asset_streamer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_asset_streamer ui_engine_test)
    add_test(NAME test_ui_asset_streamer COMMAND test_ui_asset_streamer)

    add_executable(test_ui_shader_manager tests/test_ui_shader_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_shader_manager ui_engine_test)
    add_test(NAME test_ui_shader_manager COMMAND test_ui_shader_manager)

    add_executable(test_ui_renderer tests/test_ui_renderer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer ui_engine_test)
    add_test(NAME test_ui_renderer COMMAND test_ui_renderer)

    add_executable(test_ui_renderer_gles2 tests/test_ui_renderer_gles2.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gles2 ui_engine_test)
    add_test(NAME test_ui_renderer_gles2 COMMAND test_ui_renderer_gles2)

    add_executable(test_ui_font_manager tests/test_ui_font_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_font_manager ui_engine_test)
    add_test(NAME test_ui_font_manager COMMAND test_ui_font_manager)

    add_executable(test_ui_text_layout tests/test_ui_text_layout.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_layout ui_engine_test)
    add_test(NAME test_ui_text_layout COMMAND test_ui_text_layout)

    add_executable(test_ui_video_shaders tests/test_ui_video_shaders.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_shaders ui_engine_test)
    add_test(NAME test_ui_video_shaders COMMAND test_ui_video_shaders)

    add_executable(test_ui_effects_shaders tests/test_ui_effects_shaders.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_effects_shaders ui_engine_test)
    add_test(NAME test_ui_effects_shaders COMMAND test_ui_effects_shaders)

    add_executable(test_ui_vibrancy tests/test_ui_vibrancy.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_vibrancy ui_engine_test)
    add_test(NAME test_ui_vibrancy COMMAND test_ui_vibrancy)


    add_executable(test_ui_transfer_list_base tests/test_ui_transfer_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_transfer_list_base ui_engine_test)
    add_test(NAME test_ui_transfer_list_base COMMAND test_ui_transfer_list_base)

    add_executable(test_ui_search_bar_base tests/test_ui_search_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_search_bar_base ui_engine_test)
    add_test(NAME test_ui_search_bar_base COMMAND test_ui_search_bar_base)

    add_executable(test_ui_swipe_action_base tests/test_ui_swipe_action_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_swipe_action_base ui_engine_test)
    add_test(NAME test_ui_swipe_action_base COMMAND test_ui_swipe_action_base)

    add_executable(test_ui_speed_dial_base tests/test_ui_speed_dial_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_speed_dial_base ui_engine_test)
    add_test(NAME test_ui_speed_dial_base COMMAND test_ui_speed_dial_base)

    add_executable(test_ui_video_player_base tests/test_ui_video_player_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_player_base ui_engine_test)
    add_test(NAME test_ui_video_player_base COMMAND test_ui_video_player_base)

    add_executable(test_ui_command_palette_base tests/test_ui_command_palette_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_command_palette_base ui_engine_test)
    add_test(NAME test_ui_command_palette_base COMMAND test_ui_command_palette_base)

    add_executable(test_ui_focus_manager tests/test_ui_focus_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_manager ui_engine_test)
    add_test(NAME test_ui_focus_manager COMMAND test_ui_focus_manager)

    add_executable(test_ui_keyboard_responder tests/test_ui_keyboard_responder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_keyboard_responder ui_engine_test)
    add_test(NAME test_ui_keyboard_responder COMMAND test_ui_keyboard_responder)

    add_executable(test_ui_gesture tests/test_ui_gesture.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_gesture ui_engine_test)
    add_test(NAME test_ui_gesture COMMAND test_ui_gesture)

    add_executable(test_ui_aria tests/test_ui_aria.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_aria ui_engine_test)
    add_test(NAME test_ui_aria COMMAND test_ui_aria)


    add_executable(test_ui_overlay_director tests/test_ui_overlay_director.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_overlay_director ui_engine_test)
    add_test(NAME test_ui_overlay_director COMMAND test_ui_overlay_director)

    add_executable(test_ui_geometry tests/test_ui_geometry.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_geometry ui_engine_test)
    add_test(NAME test_ui_geometry COMMAND test_ui_geometry)

    add_executable(test_ui_geometry_anchor tests/test_ui_geometry_anchor.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_geometry_anchor ui_engine_test)
    add_test(NAME test_ui_geometry_anchor COMMAND test_ui_geometry_anchor)

    add_executable(test_ui_backdrop tests/test_ui_backdrop.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_backdrop ui_engine_test)
    add_test(NAME test_ui_backdrop COMMAND test_ui_backdrop)

    add_executable(test_ui_alert_base tests/test_ui_alert_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_alert_base ui_engine_test)
    add_test(NAME test_ui_alert_base COMMAND test_ui_alert_base)

    add_executable(test_ui_button_base tests/test_ui_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_button_base ui_engine_test)
    add_test(NAME test_ui_button_base COMMAND test_ui_button_base)

    add_executable(test_ui_split_button_base tests/test_ui_split_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_split_button_base ui_engine_test)
    add_test(NAME test_ui_split_button_base COMMAND test_ui_split_button_base)

    add_executable(test_ui_bottom_app_bar_base tests/test_ui_bottom_app_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bottom_app_bar_base ui_engine_test)
    add_test(NAME test_ui_bottom_app_bar_base COMMAND test_ui_bottom_app_bar_base)

    add_executable(test_ui_section_index_base tests/test_ui_section_index_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_section_index_base ui_engine_test)
    add_test(NAME test_ui_section_index_base COMMAND test_ui_section_index_base)

    add_executable(test_ui_haptics tests/test_ui_haptics.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_haptics ui_engine_test)
    add_test(NAME test_ui_haptics COMMAND test_ui_haptics)

    add_executable(test_ui_toolbar_base tests/test_ui_toolbar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_toolbar_base ui_engine_test)
    add_test(NAME test_ui_toolbar_base COMMAND test_ui_toolbar_base)

    add_executable(test_ui_split_pane_base tests/test_ui_split_pane_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_split_pane_base ui_engine_test)
    add_test(NAME test_ui_split_pane_base COMMAND test_ui_split_pane_base)

    add_executable(test_ui_grid_list_base tests/test_ui_grid_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_grid_list_base ui_engine_test)
    add_test(NAME test_ui_grid_list_base COMMAND test_ui_grid_list_base)

    add_executable(test_ui_fab_base tests/test_ui_fab_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_fab_base ui_engine_test)
    add_test(NAME test_ui_fab_base COMMAND test_ui_fab_base)

    add_executable(test_ui_icon_base tests/test_ui_icon_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_icon_base ui_engine_test)
    add_test(NAME test_ui_icon_base COMMAND test_ui_icon_base)

    add_executable(test_ui_skeleton_base tests/test_ui_skeleton_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_skeleton_base ui_engine_test)
    add_test(NAME test_ui_skeleton_base COMMAND test_ui_skeleton_base)

    add_executable(test_ui_avatar_base tests/test_ui_avatar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_avatar_base ui_engine_test)
    add_test(NAME test_ui_avatar_base COMMAND test_ui_avatar_base)

    add_executable(test_ui_avatar_group_base tests/test_ui_avatar_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_avatar_group_base ui_engine_test)
    add_test(NAME test_ui_avatar_group_base COMMAND test_ui_avatar_group_base)

    add_executable(test_ui_sort_header_base tests/test_ui_sort_header_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sort_header_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_sort_header_base COMMAND test_ui_sort_header_base)
    add_executable(test_ui_divider_base tests/test_ui_divider_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_divider_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_divider_base COMMAND test_ui_divider_base)
    add_executable(test_ui_selection_model tests/test_ui_selection_model.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_selection_model PRIVATE ui_engine_test)
    add_test(NAME test_ui_selection_model COMMAND test_ui_selection_model)
    add_executable(test_ui_chips_base tests/test_ui_chips_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chips_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_chips_base COMMAND test_ui_chips_base)
    add_executable(test_ui_calendar_base tests/test_ui_calendar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_calendar_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_calendar_base COMMAND test_ui_calendar_base)
    add_executable(test_ui_card_base tests/test_ui_card_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_card_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_card_base COMMAND test_ui_card_base)
    add_executable(test_ui_pagination_base tests/test_ui_pagination_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pagination_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_pagination_base COMMAND test_ui_pagination_base)
    add_executable(test_ui_accordion_base tests/test_ui_accordion_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_accordion_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_accordion_base COMMAND test_ui_accordion_base)
    add_executable(test_ui_surface_base tests/test_ui_surface_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_surface_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_surface_base COMMAND test_ui_surface_base)
    add_executable(test_ui_link_base tests/test_ui_link_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_link_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_link_base COMMAND test_ui_link_base)
    add_executable(test_ui_meter_base tests/test_ui_meter_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_meter_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_meter_base COMMAND test_ui_meter_base)
    add_executable(test_ui_page_control_base tests/test_ui_page_control_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_page_control_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_page_control_base COMMAND test_ui_page_control_base)
    add_executable(test_ui_banner_base tests/test_ui_banner_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_banner_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_banner_base COMMAND test_ui_banner_base)
    add_executable(test_ui_button_group_base tests/test_ui_button_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_button_group_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_button_group_base COMMAND test_ui_button_group_base)
    add_executable(test_ui_empty_state_base tests/test_ui_empty_state_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_empty_state_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_empty_state_base COMMAND test_ui_empty_state_base)
    add_executable(test_ui_scaffold_base tests/test_ui_scaffold_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scaffold_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_scaffold_base COMMAND test_ui_scaffold_base)
    add_executable(test_ui_menubar_base tests/test_ui_menubar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_menubar_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_menubar_base COMMAND test_ui_menubar_base)
    add_executable(test_ui_radio_group_base tests/test_ui_radio_group_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_radio_group_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_radio_group_base COMMAND test_ui_radio_group_base)
    add_executable(test_ui_color_picker_base tests/test_ui_color_picker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_picker_base PRIVATE ui_engine_test)
    add_test(NAME test_ui_color_picker_base COMMAND test_ui_color_picker_base)
    add_executable(test_ui_color_space tests/test_ui_color_space.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_space PRIVATE ui_engine_test)
    add_test(NAME test_ui_color_space COMMAND test_ui_color_space)

    add_executable(test_ui_color_quantization tests/test_ui_color_quantization.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_color_quantization PRIVATE ui_engine_test)
    add_test(NAME test_ui_color_quantization COMMAND test_ui_color_quantization)
    add_executable(test_ui_design_tokens tests/test_ui_design_tokens.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_design_tokens PRIVATE ui_engine_test)
    add_test(NAME test_ui_design_tokens COMMAND test_ui_design_tokens)
    add_executable(test_ui_tonal_palette tests/test_ui_tonal_palette.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tonal_palette PRIVATE ui_engine_test)
    add_test(NAME test_ui_tonal_palette COMMAND test_ui_tonal_palette)
    add_executable(test_ui_theme_provider tests/test_ui_theme_provider.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_theme_provider PRIVATE ui_engine_test)
    add_test(NAME test_ui_theme_provider COMMAND test_ui_theme_provider)
    add_executable(test_ui_shape_family tests/test_ui_shape_family.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_shape_family PRIVATE ui_engine_test)
    add_test(NAME test_ui_shape_family COMMAND test_ui_shape_family)
    add_executable(test_ui_typography_scale tests/test_ui_typography_scale.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_typography_scale PRIVATE ui_engine_test)
    add_test(NAME test_ui_typography_scale COMMAND test_ui_typography_scale)
    add_executable(test_ui_state_layer tests/test_ui_state_layer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_state_layer PRIVATE ui_engine_test)
    add_test(NAME test_ui_state_layer COMMAND test_ui_state_layer)
    add_executable(test_ui_elevation tests/test_ui_elevation.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_elevation PRIVATE ui_engine_test)
    add_test(NAME test_ui_elevation COMMAND test_ui_elevation)
    add_executable(test_ui_focus_ring tests/test_ui_focus_ring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_ring PRIVATE ui_engine_test)
    add_test(NAME test_ui_focus_ring COMMAND test_ui_focus_ring)

    add_executable(test_ui_timeline_base tests/test_ui_timeline_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timeline_base ui_engine_test)
    add_test(NAME test_ui_timeline_base COMMAND test_ui_timeline_base)

    add_executable(test_ui_slide_toggle_base tests/test_ui_slide_toggle_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_slide_toggle_base ui_engine_test)
    add_test(NAME test_ui_slide_toggle_base COMMAND test_ui_slide_toggle_base)

    add_executable(test_ui_timepicker_base tests/test_ui_timepicker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_timepicker_base ui_engine_test)
    add_test(NAME test_ui_timepicker_base COMMAND test_ui_timepicker_base)

    add_executable(test_ui_drag_drop tests/test_ui_drag_drop.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_drag_drop ui_engine_test)
    add_test(NAME test_ui_drag_drop COMMAND test_ui_drag_drop)

    add_executable(test_ui_nav_rail_base tests/test_ui_nav_rail_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_nav_rail_base ui_engine_test)
    add_test(NAME test_ui_nav_rail_base COMMAND test_ui_nav_rail_base)

    add_executable(test_ui_bottom_nav_base tests/test_ui_bottom_nav_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bottom_nav_base ui_engine_test)
    add_test(NAME test_ui_bottom_nav_base COMMAND test_ui_bottom_nav_base)

    add_executable(test_ui_segmented_control_base tests/test_ui_segmented_control_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_segmented_control_base ui_engine_test)
    add_test(NAME test_ui_segmented_control_base COMMAND test_ui_segmented_control_base)

    add_executable(test_ui_list_base tests/test_ui_list_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_list_base ui_engine_test)
    add_test(NAME test_ui_list_base COMMAND test_ui_list_base)

    add_executable(test_ui_listbox_base tests/test_ui_listbox_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_listbox_base ui_engine_test)
    add_test(NAME test_ui_listbox_base COMMAND test_ui_listbox_base)

    add_executable(test_ui_autocomplete_base tests/test_ui_autocomplete_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_autocomplete_base ui_engine_test)
    add_test(NAME test_ui_autocomplete_base COMMAND test_ui_autocomplete_base)

    add_executable(test_ui_clipboard tests/test_ui_clipboard.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_clipboard ui_engine_test)
    add_test(NAME test_ui_clipboard COMMAND test_ui_clipboard)

    add_executable(test_ui_layout_observer tests/test_ui_layout_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_layout_observer ui_engine_test)
    add_test(NAME test_ui_layout_observer COMMAND test_ui_layout_observer)

    add_executable(test_ui_toggle_base tests/test_ui_toggle_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_toggle_base ui_engine_test)
    add_test(NAME test_ui_toggle_base COMMAND test_ui_toggle_base)

    add_executable(test_ui_checkbox_base tests/test_ui_checkbox_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_checkbox_base ui_engine_test)
    add_test(NAME test_ui_checkbox_base COMMAND test_ui_checkbox_base)

    add_executable(test_ui_file_uploader_base tests/test_ui_file_uploader_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_file_uploader_base ui_engine_test)
    add_test(NAME test_ui_file_uploader_base COMMAND test_ui_file_uploader_base)

    add_executable(test_ui_intersection_observer tests/test_ui_intersection_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_intersection_observer ui_engine_test)
    add_test(NAME test_ui_intersection_observer COMMAND test_ui_intersection_observer)

    add_executable(test_ui_mutation_observer tests/test_ui_mutation_observer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_mutation_observer ui_engine_test)
    add_test(NAME test_ui_mutation_observer COMMAND test_ui_mutation_observer)

    add_executable(test_ui_platform_detector tests/test_ui_platform_detector.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_platform_detector ui_engine_test)
    add_test(NAME test_ui_platform_detector COMMAND test_ui_platform_detector)

    add_executable(test_ui_focus_trap tests/test_ui_focus_trap.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_focus_trap ui_engine_test)
    add_test(NAME test_ui_focus_trap COMMAND test_ui_focus_trap)

    add_executable(test_ui_coercion_utils tests/test_ui_coercion_utils.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_coercion_utils ui_engine_test)
    add_test(NAME test_ui_coercion_utils COMMAND test_ui_coercion_utils)

    add_executable(test_ui_form_field_base tests/test_ui_form_field_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_field_base ui_engine_test)
    add_test(NAME test_ui_form_field_base COMMAND test_ui_form_field_base)

    add_executable(test_ui_sidenav_base tests/test_ui_sidenav_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sidenav_base ui_engine_test)
    add_test(NAME test_ui_sidenav_base COMMAND test_ui_sidenav_base)

    add_executable(test_ui_bottom_sheet_base tests/test_ui_bottom_sheet_base.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_action_sheet_base tests/test_ui_action_sheet_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_action_sheet_base ui_engine_test)
    add_test(NAME test_ui_action_sheet_base COMMAND test_ui_action_sheet_base)
    target_link_libraries(test_ui_bottom_sheet_base ui_engine_test)
    add_test(NAME test_ui_bottom_sheet_base COMMAND test_ui_bottom_sheet_base)

    add_executable(test_ui_snackbar_base tests/test_ui_snackbar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_snackbar_base ui_engine_test)
    add_test(NAME test_ui_snackbar_base COMMAND test_ui_snackbar_base)

    add_executable(test_ui_input_base tests/test_ui_input_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_input_base ui_engine_test)
    add_test(NAME test_ui_input_base COMMAND test_ui_input_base)

    add_executable(test_ui_scroll_base tests/test_ui_scroll_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_base ui_engine_test)
    add_test(NAME test_ui_scroll_base COMMAND test_ui_scroll_base)

    add_executable(test_ui_slider_base tests/test_ui_slider_base.c tests/ui_test_mock_mem.c)
add_executable(test_ui_range_slider_base tests/test_ui_range_slider_base.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_range_slider_base PRIVATE ui_engine_test)
    add_executable(test_ui_wheel_picker_base tests/test_ui_wheel_picker_base.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_pull_to_refresh_base tests/test_ui_pull_to_refresh_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pull_to_refresh_base ui_engine_test)
    add_test(NAME test_ui_pull_to_refresh_base COMMAND test_ui_pull_to_refresh_base)
    target_link_libraries(test_ui_wheel_picker_base ui_engine_test)
    add_test(NAME test_ui_wheel_picker_base COMMAND test_ui_wheel_picker_base)
    add_executable(test_ui_text_node_base tests/test_ui_text_node_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_node_base ui_engine_test)
    add_test(NAME test_ui_text_node_base COMMAND test_ui_text_node_base)
    add_executable(test_ui_spin_button_base tests/test_ui_spin_button_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_spin_button_base ui_engine_test)
    add_test(NAME test_ui_spin_button_base COMMAND test_ui_spin_button_base)
    target_link_libraries(test_ui_slider_base ui_engine_test)
    add_test(NAME test_ui_slider_base COMMAND test_ui_slider_base)
add_test(NAME test_ui_range_slider_base COMMAND test_ui_range_slider_base)

    add_executable(test_ui_select_base tests/test_ui_select_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_select_base ui_engine_test)
    add_test(NAME test_ui_select_base COMMAND test_ui_select_base)
    add_executable(test_ui_dialog_base tests/test_ui_dialog_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_dialog_base ui_engine_test)
    add_test(NAME test_ui_dialog_base COMMAND test_ui_dialog_base)

    add_executable(test_ui_disclosure_base tests/test_ui_disclosure_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_disclosure_base ui_engine_test)
    add_test(NAME test_ui_disclosure_base COMMAND test_ui_disclosure_base)

    add_executable(test_ui_modifier tests/test_ui_modifier.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_modifier ui_engine_test)
    add_test(NAME test_ui_modifier COMMAND test_ui_modifier)

    add_executable(test_ui_router tests/test_ui_router.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_router ui_engine_test)
    add_test(NAME test_ui_router COMMAND test_ui_router)

    add_executable(test_ui_e2e_headless tests/test_ui_e2e_headless.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_e2e_headless ui_engine_test)
    add_test(NAME test_ui_e2e_headless COMMAND test_ui_e2e_headless)

    add_executable(test_ui_e2e_headful tests/test_ui_e2e_headful.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_e2e_headful ui_engine_test)
    add_test(NAME test_ui_e2e_headful COMMAND test_ui_e2e_headful)

    add_executable(test_ui_web_animation tests/test_ui_web_animation.c tests/ui_test_mock_mem.c)
    add_executable(test_ui_spring tests/test_ui_spring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_spring ui_engine_test)
    add_test(NAME test_ui_spring COMMAND test_ui_spring)
    target_link_libraries(test_ui_web_animation ui_engine_test)
    add_test(NAME test_ui_web_animation COMMAND test_ui_web_animation)

    add_executable(test_ui_css_speech tests/test_ui_css_speech.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_speech ui_engine_test)
    add_test(NAME test_ui_css_speech COMMAND test_ui_css_speech)

    add_executable(test_ui_css_scroll_snap tests/test_ui_css_scroll_snap.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scroll_snap ui_engine_test)
    add_test(NAME test_ui_css_scroll_snap COMMAND test_ui_css_scroll_snap)

    add_executable(test_ui_css_scroll_anchoring tests/test_ui_css_scroll_anchoring.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_scroll_anchoring ui_engine_test)
    add_test(NAME test_ui_css_scroll_anchoring COMMAND test_ui_css_scroll_anchoring)

    add_executable(test_ui_css_spatial_nav tests/test_ui_css_spatial_nav.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_spatial_nav ui_engine_test)
    add_test(NAME test_ui_css_spatial_nav COMMAND test_ui_css_spatial_nav)

    add_executable(test_ui_css_highlight_api tests/test_ui_css_highlight_api.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_highlight_api ui_engine_test)
    add_test(NAME test_ui_css_highlight_api COMMAND test_ui_css_highlight_api)

    add_executable(test_ui_css_page tests/test_ui_css_page.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_page ui_engine_test)
    add_test(NAME test_ui_css_page COMMAND test_ui_css_page)

    add_executable(test_ui_css_gcpm tests/test_ui_css_gcpm.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_gcpm ui_engine_test)
    add_test(NAME test_ui_css_gcpm COMMAND test_ui_css_gcpm)

    add_executable(test_ui_css_page_floats tests/test_ui_css_page_floats.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_css_page_floats ui_engine_test)
    add_test(NAME test_ui_css_page_floats COMMAND test_ui_css_page_floats)

    add_executable(test_ui_cssom_api tests/test_ui_cssom_api.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom_api ui_engine_test)
    add_test(NAME test_ui_cssom_api COMMAND test_ui_cssom_api)

    add_executable(test_ui_cssom_view tests/test_ui_cssom_view.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_cssom_view ui_engine_test)
    add_test(NAME test_ui_cssom_view COMMAND test_ui_cssom_view)

    add_executable(test_ui_menu_base tests/test_ui_menu_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_menu_base ui_engine_test)
    add_test(NAME test_ui_menu_base COMMAND test_ui_menu_base)

    add_executable(test_ui_popover_base tests/test_ui_popover_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_popover_base ui_engine_test)
    add_test(NAME test_ui_popover_base COMMAND test_ui_popover_base)

    add_executable(test_ui_stepper_base tests/test_ui_stepper_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_stepper_base ui_engine_test)
    add_test(NAME test_ui_stepper_base COMMAND test_ui_stepper_base)

    add_executable(test_ui_table_base tests/test_ui_table_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_table_base ui_engine_test)
    add_test(NAME test_ui_table_base COMMAND test_ui_table_base)

    add_executable(test_ui_tabs_base tests/test_ui_tabs_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tabs_base ui_engine_test)
    add_test(NAME test_ui_tabs_base COMMAND test_ui_tabs_base)

    add_executable(test_ui_toast_manager_base tests/test_ui_toast_manager_base.c tests/ui_test_mock_mem.c)
add_executable(test_ui_breakpoint_observer tests/test_ui_breakpoint_observer.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_breakpoint_observer PRIVATE ui_engine_test)
    target_link_libraries(test_ui_toast_manager_base ui_engine_test)
    add_test(NAME test_ui_toast_manager_base COMMAND test_ui_toast_manager_base)
add_test(NAME test_ui_breakpoint_observer COMMAND test_ui_breakpoint_observer)

    add_executable(test_ui_tooltip_base tests/test_ui_tooltip_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tooltip_base ui_engine_test)
    add_test(NAME test_ui_tooltip_base COMMAND test_ui_tooltip_base)

    add_executable(test_ui_tree_base tests/test_ui_tree_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tree_base ui_engine_test)
    add_test(NAME test_ui_tree_base COMMAND test_ui_tree_base)

    add_executable(test_ui_virtual_scroll_base tests/test_ui_virtual_scroll_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_virtual_scroll_base ui_engine_test)
    add_test(NAME test_ui_virtual_scroll_base COMMAND test_ui_virtual_scroll_base)

    add_executable(test_ui_carousel_base tests/test_ui_carousel_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_carousel_base ui_engine_test)
    add_test(NAME test_ui_carousel_base COMMAND test_ui_carousel_base)

    add_executable(test_ui_breadcrumbs_base tests/test_ui_breadcrumbs_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_breadcrumbs_base ui_engine_test)
    add_test(NAME test_ui_breadcrumbs_base COMMAND test_ui_breadcrumbs_base)

    add_executable(test_ui_portal tests/test_ui_portal.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_portal ui_engine_test)
    add_test(NAME test_ui_portal COMMAND test_ui_portal)

    add_executable(test_ui_context_menu_base tests/test_ui_context_menu_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_context_menu_base ui_engine_test)
    add_test(NAME test_ui_context_menu_base COMMAND test_ui_context_menu_base)

    add_executable(test_ui_live_announcer tests/test_ui_live_announcer.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_live_announcer ui_engine_test)
    add_test(NAME test_ui_live_announcer COMMAND test_ui_live_announcer)

    add_executable(test_ui_bidi_manager tests/test_ui_bidi_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_bidi_manager ui_engine_test)
    add_test(NAME test_ui_bidi_manager COMMAND test_ui_bidi_manager)

    add_executable(test_ui_progress_base tests/test_ui_progress_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_progress_base ui_engine_test)
    add_test(NAME test_ui_progress_base COMMAND test_ui_progress_base)

    add_executable(test_ui_camera_base tests/test_ui_camera_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_camera_base ui_engine_test)
    add_test(NAME test_ui_camera_base COMMAND test_ui_camera_base)

    add_executable(test_ui_webview_base tests/test_ui_webview_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_webview_base ui_engine_test)
    add_test(NAME test_ui_webview_base COMMAND test_ui_webview_base)

    add_executable(test_ui_ink_base tests/test_ui_ink_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ink_base ui_engine_test)
    add_test(NAME test_ui_ink_base COMMAND test_ui_ink_base)

    add_executable(test_ui_coachmark_base tests/test_ui_coachmark_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_coachmark_base ui_engine_test)
    add_test(NAME test_ui_coachmark_base COMMAND test_ui_coachmark_base)

    add_executable(test_ui_signal tests/test_ui_signal.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_signal ui_engine_test)
    add_test(NAME test_ui_signal COMMAND test_ui_signal)

    add_executable(test_ui_computed tests/test_ui_computed.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_computed ui_engine_test)
    add_test(NAME test_ui_computed COMMAND test_ui_computed)

    add_executable(test_ui_effect tests/test_ui_effect.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_effect ui_engine_test)
    add_test(NAME test_ui_effect COMMAND test_ui_effect)

    add_executable(test_ui_rich_text_base tests/test_ui_rich_text_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rich_text_base ui_engine_test)
    add_test(NAME test_ui_rich_text_base COMMAND test_ui_rich_text_base)

    add_executable(test_ui_renderer_gl1 tests/test_ui_renderer_gl1.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gl1 ui_engine_test)
    add_test(NAME test_ui_renderer_gl1 COMMAND test_ui_renderer_gl1)

    add_executable(test_ui_web_bridge tests/test_ui_web_bridge.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_web_bridge ui_engine_test)
    add_test(NAME test_ui_web_bridge COMMAND test_ui_web_bridge)

    add_executable(test_ui_reactive_graph tests/test_ui_reactive_graph.c)
    target_link_libraries(test_ui_reactive_graph ui_engine_test)
    add_test(NAME test_ui_reactive_graph COMMAND test_ui_reactive_graph)

    add_executable(test_ui_form_node tests/test_ui_form_node.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_node ui_engine_test)
    add_test(NAME test_ui_form_node COMMAND test_ui_form_node)

    add_executable(test_ui_form_builder tests/test_ui_form_builder.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_builder ui_engine_test)
    add_test(NAME test_ui_form_builder COMMAND test_ui_form_builder)

    add_executable(test_ui_datepicker_base tests/test_ui_datepicker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_datepicker_base ui_engine_test)
    add_test(NAME test_ui_datepicker_base COMMAND test_ui_datepicker_base)

    add_executable(test_ui_os_dialogs tests/test_ui_os_dialogs.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_os_dialogs ui_engine_test)
    add_test(NAME test_ui_os_dialogs COMMAND test_ui_os_dialogs)



    add_executable(test_ui_window_backend_linux_wayland tests/test_ui_window_backend_linux_wayland.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_backend_linux_wayland ui_engine_test)
    add_test(NAME test_ui_window_backend_linux_wayland COMMAND test_ui_window_backend_linux_wayland)

    add_executable(test_ui_badge_base tests/test_ui_badge_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_badge_base ui_engine_test)
    add_test(NAME test_ui_badge_base COMMAND test_ui_badge_base)

    add_executable(test_ui_rating_base tests/test_ui_rating_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rating_base ui_engine_test)
    add_test(NAME test_ui_rating_base COMMAND test_ui_rating_base)

    add_executable(test_ui_ripple_base tests/test_ui_ripple_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ripple_base ui_engine_test)
    add_test(NAME test_ui_ripple_base COMMAND test_ui_ripple_base)

    add_executable(test_ui_textarea_autosize tests/test_ui_textarea_autosize.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_textarea_autosize ui_engine_test)
    add_test(NAME test_ui_textarea_autosize COMMAND test_ui_textarea_autosize)

    add_executable(test_ui_key_manager tests/test_ui_key_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_key_manager ui_engine_test)
    add_test(NAME test_ui_key_manager COMMAND test_ui_key_manager)

    add_executable(test_ui_scroll_dispatcher tests/test_ui_scroll_dispatcher.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_scroll_dispatcher ui_engine_test)
    add_test(NAME test_ui_scroll_dispatcher COMMAND test_ui_scroll_dispatcher)

    add_executable(test_ui_label_base tests/test_ui_label_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_label_base ui_engine_test)
    add_test(NAME test_ui_label_base COMMAND test_ui_label_base)

    add_executable(test_ui_aspect_ratio_base tests/test_ui_aspect_ratio_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_aspect_ratio_base ui_engine_test)
    add_test(NAME test_ui_aspect_ratio_base COMMAND test_ui_aspect_ratio_base)

    add_executable(test_ui_masonry_layout_base tests/test_ui_masonry_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_masonry_layout_base ui_engine_test)
    add_test(NAME test_ui_masonry_layout_base COMMAND test_ui_masonry_layout_base)

    add_executable(test_ui_pin_input_base tests/test_ui_pin_input_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_pin_input_base ui_engine_test)
    add_test(NAME test_ui_pin_input_base COMMAND test_ui_pin_input_base)

    add_executable(test_ui_hover_card_base tests/test_ui_hover_card_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_hover_card_base ui_engine_test)
    add_test(NAME test_ui_hover_card_base COMMAND test_ui_hover_card_base)

    add_executable(test_ui_window_manager_base tests/test_ui_window_manager_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_manager_base ui_engine_test)
    add_test(NAME test_ui_window_manager_base COMMAND test_ui_window_manager_base)

    add_executable(test_ui_rich_text_editor_base tests/test_ui_rich_text_editor_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_rich_text_editor_base ui_engine_test)
    add_test(NAME test_ui_rich_text_editor_base COMMAND test_ui_rich_text_editor_base)

    add_executable(test_ui_datagrid_base tests/test_ui_datagrid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_datagrid_base ui_engine_test)
    add_test(NAME test_ui_datagrid_base COMMAND test_ui_datagrid_base)

    add_executable(test_ui_form_group tests/test_ui_form_group.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_group ui_engine_test)
    add_test(NAME test_ui_form_group COMMAND test_ui_form_group)

    add_executable(test_ui_form_e2e tests/test_ui_form_e2e.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_e2e ui_engine_test)
    add_test(NAME test_ui_form_e2e COMMAND test_ui_form_e2e)


    add_executable(test_ui_form_array tests/test_ui_form_array.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_array ui_engine_test)
    add_test(NAME test_ui_form_array COMMAND test_ui_form_array)

    add_executable(test_ui_canonical_layout_base tests/test_ui_canonical_layout_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_canonical_layout_base ui_engine_test)
    add_test(NAME test_ui_canonical_layout_base COMMAND test_ui_canonical_layout_base)

    add_executable(test_ui_window_controls_base tests/test_ui_window_controls_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_window_controls_base ui_engine_test)
    add_test(NAME test_ui_window_controls_base COMMAND test_ui_window_controls_base)

    add_executable(test_ui_compositor_material_base tests/test_ui_compositor_material_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_compositor_material_base ui_engine_test)
    add_test(NAME test_ui_compositor_material_base COMMAND test_ui_compositor_material_base)

    add_executable(test_ui_loupe_base tests/test_ui_loupe_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_loupe_base ui_engine_test)
    add_test(NAME test_ui_loupe_base COMMAND test_ui_loupe_base)

    add_executable(test_ui_ribbon_base tests/test_ui_ribbon_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_ribbon_base ui_engine_test)
    add_test(NAME test_ui_ribbon_base COMMAND test_ui_ribbon_base)

    add_executable(test_ui_miller_columns_base tests/test_ui_miller_columns_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_miller_columns_base ui_engine_test)
    add_test(NAME test_ui_miller_columns_base COMMAND test_ui_miller_columns_base)

    add_executable(test_ui_property_grid_base tests/test_ui_property_grid_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_property_grid_base ui_engine_test)
    add_test(NAME test_ui_property_grid_base COMMAND test_ui_property_grid_base)

    add_executable(test_ui_syntax_surface_base tests/test_ui_syntax_surface_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_syntax_surface_base ui_engine_test)
    add_test(NAME test_ui_syntax_surface_base COMMAND test_ui_syntax_surface_base)

    add_executable(test_ui_node_graph_base tests/test_ui_node_graph_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_node_graph_base ui_engine_test)
    add_test(NAME test_ui_node_graph_base COMMAND test_ui_node_graph_base)

    add_executable(test_ui_chart_base tests/test_ui_chart_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chart_base ui_engine_test)
    add_test(NAME test_ui_chart_base COMMAND test_ui_chart_base)

    add_executable(test_ui_chat_bubble_base tests/test_ui_chat_bubble_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_chat_bubble_base ui_engine_test)
    add_test(NAME test_ui_chat_bubble_base COMMAND test_ui_chat_bubble_base)

    add_executable(test_ui_form_control tests/test_ui_form_control.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_form_control ui_engine_test)
    add_test(NAME test_ui_form_control COMMAND test_ui_form_control)

    add_executable(test_ui_top_app_bar_base tests/test_ui_top_app_bar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_top_app_bar_base ui_engine_test)
    add_test(NAME test_ui_top_app_bar_base COMMAND test_ui_top_app_bar_base)

    add_executable(test_ui_titlebar_base tests/test_ui_titlebar_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_titlebar_base ui_engine_test)
    add_test(NAME test_ui_titlebar_base COMMAND test_ui_titlebar_base)

    add_executable(test_ui_theme_manager tests/test_ui_theme_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_theme_manager ui_engine_test)
    add_test(NAME test_ui_theme_manager COMMAND test_ui_theme_manager)
    add_executable(test_ui_safe_area_manager tests/test_ui_safe_area_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_safe_area_manager ui_engine_test)
    add_test(NAME test_ui_safe_area_manager COMMAND test_ui_safe_area_manager)

    if(BUILD_VISUAL_TESTS)
        add_executable(test_ui_visual tests/test_ui_visual.c tests/ui_test_mock_mem.c src/ui_test_visual.c)
        target_link_libraries(test_ui_visual ui_engine_test)
    endif()
    add_executable(test_ui_date_range_picker_base tests/test_ui_date_range_picker_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_date_range_picker_base ui_engine_test)
    add_test(NAME test_ui_date_range_picker_base COMMAND test_ui_date_range_picker_base)
    add_executable(test_ui_map_view_base tests/test_ui_map_view_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_map_view_base ui_engine_test)
    add_test(NAME test_ui_map_view_base COMMAND test_ui_map_view_base)
    add_executable(test_ui_tray_manager tests/test_ui_tray_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_tray_manager PRIVATE ui_engine_test)
    add_test(NAME test_ui_tray_manager COMMAND test_ui_tray_manager)
    add_executable(test_ui_auth tests/test_ui_auth.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_auth PRIVATE ui_engine_test)
    add_test(NAME test_ui_auth COMMAND test_ui_auth)
    add_executable(test_ui_share tests/test_ui_share.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_share PRIVATE ui_engine_test)
    add_test(NAME test_ui_share COMMAND test_ui_share)
    add_executable(test_ui_sensor_manager tests/test_ui_sensor_manager.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_sensor_manager PRIVATE ui_engine_test)
    add_test(NAME test_ui_sensor_manager COMMAND test_ui_sensor_manager)
    add_executable(test_ui_text_layout_hb tests/test_ui_text_layout_hb.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_text_layout_hb PRIVATE ui_engine_test)
    add_test(NAME test_ui_text_layout_hb COMMAND test_ui_text_layout_hb)
        add_test(NAME test_ui_visual COMMAND test_ui_visual)
    add_executable(test_ui_audio_coreaudio tests/test_ui_audio_coreaudio.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_coreaudio ui_engine_test)
    add_test(NAME test_ui_audio_coreaudio COMMAND test_ui_audio_coreaudio)

    add_executable(test_ui_audio_opensles tests/test_ui_audio_opensles.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_opensles ui_engine_test)
    add_test(NAME test_ui_audio_opensles COMMAND test_ui_audio_opensles)

    add_executable(test_ui_audio_wasapi tests/test_ui_audio_wasapi.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_wasapi ui_engine_test)
    add_test(NAME test_ui_audio_wasapi COMMAND test_ui_audio_wasapi)

    add_executable(test_ui_audio_webaudio tests/test_ui_audio_webaudio.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_audio_webaudio ui_engine_test)
    add_test(NAME test_ui_audio_webaudio COMMAND test_ui_audio_webaudio)

    add_executable(test_ui_video_avfoundation tests/test_ui_video_avfoundation.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_avfoundation ui_engine_test)
    add_test(NAME test_ui_video_avfoundation COMMAND test_ui_video_avfoundation)

    add_executable(test_ui_video_mediacodec tests/test_ui_video_mediacodec.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_mediacodec ui_engine_test)
    add_test(NAME test_ui_video_mediacodec COMMAND test_ui_video_mediacodec)

    add_executable(test_ui_video_mf tests/test_ui_video_mf.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_video_mf ui_engine_test)
    add_test(NAME test_ui_video_mf COMMAND test_ui_video_mf)

    add_executable(test_ui_renderer_coregraphics tests/test_ui_renderer_coregraphics.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_coregraphics ui_engine_test)
    add_test(NAME test_ui_renderer_coregraphics COMMAND test_ui_renderer_coregraphics)

    add_executable(test_ui_image_base tests/test_ui_image_base.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_image_base ui_engine_test)
    add_test(NAME test_ui_image_base COMMAND test_ui_image_base)


    add_executable(test_ui_renderer_gdiplus tests/test_ui_renderer_gdiplus.c tests/ui_test_mock_mem.c)
    target_link_libraries(test_ui_renderer_gdiplus ui_engine_test)
    add_test(NAME test_ui_renderer_gdiplus COMMAND test_ui_renderer_gdiplus)
endif()


add_subdirectory(examples)
add_executable(test_ui_layout_internal tests/test_ui_layout_internal.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_layout_internal PRIVATE ui_engine)
add_executable(test_ui_cssom_internal tests/test_ui_cssom_internal.c tests/ui_test_mock_mem.c)
target_link_libraries(test_ui_cssom_internal ui_engine c_cdd c89stringutils c-abstract-http c-orm c-orm-async cfs parson c_str_span)
if(ENABLE_COVERAGE)
    target_link_options(test_ui_cssom_internal PRIVATE --coverage)
    target_compile_options(test_ui_cssom_internal PRIVATE --coverage)
endif()
add_test(NAME test_ui_cssom_internal COMMAND test_ui_cssom_internal)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(src/ui_stb_impl.c PROPERTIES COMPILE_FLAGS "-fno-profile-instr-generate -fno-coverage-mapping")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    set_source_files_properties(src/ui_stb_impl.c PROPERTIES COMPILE_FLAGS "-fno-profile-arcs -fno-test-coverage")
endif()


if (BUILD_TESTING OR CMAKE_BUILD_TYPE STREQUAL "Debug")

    add_executable(test_example_basic tests/test_example_basic.c tests/ui_test_mock_mem.c)
    target_compile_definitions(test_example_basic PRIVATE CI_TEST_RUN=1)
    target_compile_options(test_example_basic PRIVATE    )
    target_link_libraries(test_example_basic PRIVATE ui_engine_test)
    add_test(NAME test_example_basic COMMAND test_example_basic)

    add_executable(test_example_window_basic tests/test_example_window_basic.c tests/ui_test_mock_mem.c)
    target_compile_definitions(test_example_window_basic PRIVATE CI_TEST_RUN=1)
    target_compile_options(test_example_window_basic PRIVATE    )
    target_link_libraries(test_example_window_basic PRIVATE ui_engine_test)
    add_test(NAME test_example_window_basic COMMAND test_example_window_basic)

    add_executable(test_example_responsive_mosaic tests/test_example_responsive_mosaic.c tests/ui_test_mock_mem.c)
    target_compile_definitions(test_example_responsive_mosaic PRIVATE CI_TEST_RUN=1)
    target_compile_options(test_example_responsive_mosaic PRIVATE    )
    target_link_libraries(test_example_responsive_mosaic PRIVATE ui_engine_test)
    add_test(NAME test_example_responsive_mosaic COMMAND test_example_responsive_mosaic)

    add_executable(test_example_unstyled_components tests/test_example_unstyled_components.c tests/ui_test_mock_mem.c)
    target_compile_definitions(test_example_unstyled_components PRIVATE CI_TEST_RUN=1)
    target_compile_options(test_example_unstyled_components PRIVATE    )
    target_link_libraries(test_example_unstyled_components PRIVATE ui_engine_test)
    add_test(NAME test_example_unstyled_components COMMAND test_example_unstyled_components)

    add_executable(test_example_auth_flow tests/test_example_auth_flow.c tests/ui_test_mock_mem.c examples/auth_flow/views.c examples/auth_flow/i18n_dict.c examples/auth_flow/mock_backend.c)
    target_compile_definitions(test_example_auth_flow PRIVATE CI_TEST_RUN=1)
    target_compile_options(test_example_auth_flow PRIVATE    )
    target_link_libraries(test_example_auth_flow PRIVATE ui_engine_test)
    add_test(NAME test_example_auth_flow COMMAND test_example_auth_flow)
endif()
