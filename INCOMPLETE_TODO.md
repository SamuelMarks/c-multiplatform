# Incomplete Features and Stubs

| completed | filename | symbol name | notes |
|---|---|---|---|
| [x] | src/cmp_audio_capture.c | cmp_audio_capture_start | Mock 1 second of 16-bit mono audio |
| [x] | src/cmp_auth_sec.c | cmp_siwa_request | cmp_siwa_request: SIWA request mock fulfilled |
| [x] | src/cmp_auth_sec.c | cmp_local_auth_request | cmp_local_auth_request: Local auth request mocked success |
| [x] | src/cmp_auth_sec.c | cmp_keychain_save | cmp_keychain_save: Keychain save mocked successfully |
| [x] | src/cmp_borders.c | cmp_backdrop_kawase_blur | STUB |
| [x] | src/cmp_borders.c | cmp_blend_mode_resolve | STUB |
| [x] | src/cmp_borders.c | cmp_blend_mode_resolve | cmp_blend_mode_resolve: Mocked blend state mapping |
| [x] | src/cmp_borders.c | cmp_svg_filter_fe_color_matrix | STUB |
| [x] | src/cmp_collections.c | cmp_system_map_view_mount | cmp_system_map_view_mount: Mock mounted map view |
| [x] | src/cmp_collections.c | cmp_system_web_view_mount | cmp_system_web_view_mount: Mock mounted web view for %s |
| [x] | src/cmp_complex_gesture.c | cmp_complex_gesture_process_event | Simplified state machine mock handling for test validation |
| [x] | src/cmp_compositor_anim.c | cmp_compositor_capture_framebuffer | cmp_compositor_capture_framebuffer: Successfully created mock |
| [x] | src/cmp_compositor_anim.c | cmp_compositor_start_crossfade | cmp_compositor_start_crossfade: Mock scheduled crossfade |
| [x] | src/cmp_compositor_thread.c | cmp_compositor_thread_push_tree | cmp_compositor_thread_push_tree: Mock pushed layout tree onto |
| [x] | src/cmp_context_menu.c | cmp_context_menu_process_event | Modifiers bitmask mock: bit 1 = right click |
| [x] | src/cmp_credential_manager.c | cmp_credential_manager_destroy | Clean up mock static state for tests |
| [x] | src/cmp_credential_manager.c | cmp_credential_manager_set_secret | Simple mock for tests |
| [x] | src/cmp_credential_manager.c | cmp_credential_manager_set_secret | cmp_credential_manager_set_secret: Successfully cached mocked |
| [x] | src/cmp_credential_manager.c | cmp_credential_manager_delete_secret | cmp_credential_manager_delete_secret: Deleted mocked secret |
| [x] | src/cmp_cssom.c | cmp_cssom_decl_init | Parsing properties not implemented yet |
| [x] | src/cmp_cssom.c | cmp_cssom_mutator_insert_rule | Not implemented |
| [x] | src/cmp_cssom.c | cmp_cssom_mutator_insert_rule | Stub implementation |
| [x] | src/cmp_cssom.c | cmp_cssom_mutator_delete_rule | Not implemented |
| [x] | src/cmp_cssom.c | cmp_cssom_mutator_delete_rule | Stub implementation |
| [x] | src/cmp_embedded_pty.c | cmp_embedded_pty_write | Mock echoing the input back |
| [x] | src/cmp_gesture.c | cmp_gesture_process_event | Mock deterministic state progression based on standard action flow |
| [x] | src/cmp_gpu.c | dummy_begin_frame | dummy_begin_frame: Executed mock begin_frame |
| [x] | src/cmp_gpu.c | dummy_end_frame | dummy_end_frame: Executed mock end_frame |
| [x] | src/cmp_gpu.c | dummy_destroy | dummy_destroy: Executed mock destroy |
| [x] | src/cmp_gpu.c | cmp_shader_compile_spirv | cmp_shader_compile_spirv: Mapped mocked SPIR-V generation |
| [x] | src/cmp_hit_test.c | cmp_hit_test_query | Simulate finding a hit using the mock bypass if set |
| [x] | src/cmp_http.c | cmp_http_ws_init | Mock WS configuration onto the request struct |
| [x] | src/cmp_http.c | cmp_http_ws_init | cmp_http_ws_init: Mocked Websockets initialization |
| [x] | src/cmp_http.c | cmp_http_ws_send | Mocking the sending mechanics |
| [x] | src/cmp_http.c | cmp_http_ws_send | cmp_http_ws_send: Mocked WS transmission |
| [x] | src/cmp_http.c | cmp_http_ws_close | Mocking the connection closure |
| [x] | src/cmp_http.c | cmp_http_ws_close | cmp_http_ws_close: Mocked WS close |
| [x] | src/cmp_http.c | cmp_http_ws_run | cmp_http_ws_run: Mocked WS run execution |
| [x] | src/cmp_http.c | cmp_http_sse_init | cmp_http_sse_init: Mocked SSE init |
| [x] | src/cmp_http.c | cmp_http_sse_run | cmp_http_sse_run: Mocked SSE run execution |
| [x] | src/cmp_i18n.c | cmp_i18n_format | Hardcoded mock logic for tests |
| [x] | src/cmp_inputs.c | cmp_segmented_control_get_visuals | Assuming standard 100px segments for mock |
| [x] | src/cmp_interop_swift.c | cmp_interop_allocate_retained_object | Set mock retain count to 1 |
| [x] | src/cmp_interop_swift.c | cmp_interop_release_object | Verify mock ARC ref count |
| [x] | src/cmp_ipc.c | cmp_process_send | Mock implementation for IPC pipes: Serialize and deserialize to create a deep copy |
| [x] | src/cmp_ipc.c | cmp_process_send | cmp_process_send: Appended message to mock queue |
| [x] | src/cmp_ipc.c | cmp_process_recv | cmp_process_recv: Received message from mock queue |
| [x] | src/cmp_lottie.c | cmp_lottie_create | Mock duration |
| [x] | src/cmp_mermaid_renderer.c | cmp_mermaid_renderer_generate_svg | Simple mocked syntax validation for stubs |
| [x] | src/cmp_mipmap.c | cmp_mipmap_generator_generate | Mock implementation |
| [x] | src/cmp_mmap.c | cmp_mmap_open | Mock filling the mapped file with a recognizable byte |
| [x] | src/cmp_msaa.c | cmp_msaa_create | Mock allocation for the internal renderbuffer based on samples |
| [x] | src/cmp_msaa.c | cmp_msaa_create | Initialize the mock buffer to zero |
| [x] | src/cmp_msaa.c | cmp_msaa_resolve | Here we simply mock a successful resolve by validating the inputs |
| [x] | src/cmp_plugin_loader.c | cmp_plugin_loader_load | Mock load |
| [x] | src/cmp_plugin_loader.c | cmp_plugin_loader_unload | Mock unload |
| [x] | src/cmp_pointer_events.c | cmp_ui_node_set_pointer_events | Mock implementation using properties just to pass logic tests |
| [x] | src/cmp_pointer_pressure.c | cmp_pointer_pressure_process_event | Mock mapping from raw event payload into the tracking struct |
| [x] | src/cmp_pointer_pressure.c | cmp_pointer_pressure_process_event | Mock mapping tilt |
| [x] | src/cmp_pty.c | (global/file scope) | Cross-platform mock implementation for tests |
| [x] | src/cmp_pty.c | cmp_pty_create | Mock initial output |
| [x] | src/cmp_pty.c | cmp_pty_write | Mock echo behavior |
| [x] | src/cmp_pty.c | cmp_pty_read | Consume mock buffer |
| [x] | src/cmp_screen_capture.c | cmp_screen_capture_active_window | Mock capture |
| [x] | src/cmp_screen_capture.c | cmp_screen_capture_active_window | White background mock |
| [x] | src/cmp_screen_capture.c | cmp_screen_capture_region | Mock capture |
| [x] | src/cmp_screen_capture.c | cmp_screen_capture_region | Gray background mock |
| [x] | src/cmp_svg.c | cmp_svg_smil_tick | Mock applying the value to target |
| [x] | src/cmp_swapchain.c | cmp_swapchain_create | Mock internal handle |
| [x] | src/cmp_swapchain.c | cmp_swapchain_set_msaa | Mock implementation |
| [x] | src/cmp_system_theme_android_web.c | cmp_system_theme_is_high_contrast | Requires Android 14+ UI_MODE_NIGHT_MASK logic via JNI, mock to 0 |
| [x] | src/cmp_tex_compression.c | cmp_tex_compression_mount | We might allocate an internal handle to mock a bound texture on the GPU |
| [x] | src/cmp_tex_compression.c | cmp_tex_compression_mount | Test mock |
| [x] | src/cmp_touch_action.c | cmp_ui_node_set_touch_action | Mock storing the bitfield value in the node's properties ptr |
| [x] | src/cmp_tree_sitter.c | cmp_tree_sitter_parse | Mock root node representing a translation_unit |
| [x] | src/cmp_typography.c | cmp_harfbuzz_text_shape | Mocked text shaping length logic |
| [x] | src/cmp_typography.c | cmp_harfbuzz_text_shape | cmp_harfbuzz_text_shape: Mocked HarfBuzz text shaping |
| [x] | src/cmp_typography.c | cmp_arabic_indic_shape | Mocked complex shape counting logic |
| [x] | src/cmp_typography.c | cmp_bidi_run_split | Mocked single LTR run fallback |
| [x] | src/cmp_typography.c | cmp_bidi_run_split | cmp_bidi_run_split: Mocked BiDi run splitting fallback |
| [x] | src/cmp_typography.c | cmp_font_render_sdf | cmp_font_render_sdf: Mocked SDF font rendering |
| [x] | src/cmp_typography.c | cmp_font_render_msdf | cmp_font_render_msdf: Mocked MSDF font rendering |
| [x] | src/cmp_typography.c | cmp_font_render_color_emoji | cmp_font_render_color_emoji: Mocked Color Emoji rendering |
| [x] | src/cmp_typography.c | cmp_variable_font_axis_interpolate | cmp_variable_font_axis_interpolate: Mocked variable font axis |
| [x] | src/cmp_ui.c | cmp_ui_layout_measure_dispatch | TODO: Theme text measurement routing with max_width |
| [x] | src/cmp_visuals.c | cmp_icc_profile_parse | Mocked Display P3 to sRGB or similar matrix |
| [x] | src/cmp_wayland_protocols.c | cmp_wayland_protocols_bind | Mock: In real application this uses wayland-client and parses globals. |
| [x] | src/cmp_win32_input.c | cmp_win32_init_touch_ink | cmp_win32_init_touch_ink: Mocked Win32 Touch/Ink initialization |
| [x] | src/cmp_win32_window.c | cmp_win32_request_windows_material | cmp_win32_request_windows_material: Mocked Windows material request |
| [x] | src/themes/cmp_f2_button.c | cmp_f2_compound_button_create | MOCK: create a secondary text node representing descriptive text |
| [x] | src/themes/cmp_f2_button.c | cmp_f2_menu_button_create | MOCK: create trailing chevron icon |
| [x] | src/themes/cmp_f2_button.c | cmp_f2_split_button_create | MOCK: create divider and chevron |
| [x] | src/themes/cmp_f2_dropdowns.c | cmp_f2_dropdown_create | Mock children |
| [x] | src/themes/cmp_f2_overlays.c | cmp_f2_progress_ring_create | Shares data structure with progress bar internally for this mock |
| [x] | src/themes/cmp_f2_text_inputs.c | cmp_f2_field_create | Mock label addition |
| [x] | src/themes/cmp_f2_text_inputs.c | cmp_f2_field_set_help_text | If changing text, map it natively. Here we mock it logicially |
