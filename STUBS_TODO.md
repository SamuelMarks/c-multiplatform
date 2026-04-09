# C-Multiplatform Exhaustive Stubs & TODOs

This file contains an exhaustively compiled list of all `TODO`s, `FIXME`s, stubs, missing implementations, and blank vtable methods across the codebase.

## `include/cmp.h`
- [✓] `cmp_event_pop`: * @brief Execute a hit test mapping coordinates to a UI tree node (stub)

## `src/cmp_bezier_ease.c`
- [✓] `cmp_bezier_ease_evaluate`: stub, we'll approximate u = t for common curves, or we would implement a

## `src/cmp_borders.c`
- [✓] `cmp_shadow_9patch_generate`: out_shadow->base_texture = NULL; Stub

## `src/cmp_dnd.c`
- [✓] `cmp_dnd_add_item_to_stack`: payloads. Since our stub only tracks a single string right now, we

## `src/cmp_dpi.c`
- [✓] `cmp_dpi_update_window_scale`: For now, just a stub that successfully "applied" it

## `src/cmp_embedded_pty.c`
- [✓] `Global/Macro`: This is a portable stub.

## `src/cmp_i18n.c`
- [✓] `cmp_i18n_translate_plural`: Empty implementation / void cast stub

## `src/cmp_image_preview.c`
- [✓] `cmp_image_preview_load_base64`: Stub: Decode base64 to RGBA logic. For now, generate a tiny red 4x4 square

## `src/cmp_input_mask.c`
- [✓] `cmp_input_mask_apply`: Assume 'X' means any char in this simple stub

## `src/cmp_math_renderer.c`
- [✓] `cmp_math_renderer_parse`: Stub implementation of KaTeX equivalent parser tree.

## `src/cmp_native_dialog.c`
- [✓] `cmp_native_dialog_show`: on modality. For this stub, we just simulate showing it.

## `src/cmp_orm.c`
- [✓] `cmp_orm_migrate`: Stub: In a real implementation this would scan the directory
- [✓] `cmp_orm_set_encryption_key`: Stub: In a real implementation this would bridge to PRAGMA key

## `src/cmp_passive_event.c`
- [✓] `cmp_passive_event_init`: for passive listeners. For now, it's just a lifecycle stub.

## `src/cmp_position.c`
- [✓] `cmp_pos_sticky`: Basic sticky evaluation stub

## `src/cmp_secure_network.c`
- [✓] `cmp_secure_network_retrieve_credential`: Fallback stub if not found / not Windows

## `src/cmp_svg.c`
- [✓] `cmp_svg_smil_tick`: break; Simple single animation stub
- [✓] `cmp_svg_filter_evaluate`: Stub passthrough

## `src/cmp_syntax_highlight.c`
- [✓] `cmp_syntax_highlighter_parse`: Naive lexer stub for basic string testing.

## `src/cmp_undo_redo.c`
- [✓] `cmp_undo_redo_push`: Simple drop oldest for stub

## `src/cmp_visuals.c`
- [✓] `cmp_color_parse_p3`: Simplified stub for parsing "color(display-p3 r g b)"
- [✓] `cmp_color_oklch_to_srgb`: Matrix transformation stub
- [✓] `cmp_color_oklch_to_srgb`: out_color->r = in_color->r; stub mapped values
- [✓] `cmp_color_mix`: Linear interpolation stub based on target space
- [✓] `cmp_color_luminance`: WCAG relative luminance formula stub

## `src/cmp_win32_input.c`
- [✓] `cmp_win32_init_touch_ink`: * \brief Stub for non-Windows platforms.

## `src/cmp_window.c`
- [✓] `Global/Macro`: POSIX fallback / unsupported stub for Phase 6
- [✓] `cmp_hardware_poll_gamepad`: out_gamepad->is_connected = 0; Stub
- [✓] `cmp_hardware_trigger_haptic`: return CMP_SUCCESS; Stub
- [✓] `cmp_renderer_create`: Stub for Phase 18 layout logic
- [✓] `cmp_font_load`: font->internal_handle = NULL; Stub for stb_truetype / FreeType handle
- [✓] `cmp_font_add_fallback`: Stub for linking fallback chains
- [✓] `cmp_text_shape`: Basic stub calculation
- [✓] `cmp_theme_generate_palette`: Stub for color space math (e.g. HCT space generation for Material 3)

## `src/core/cmp_event.c`
- [✓] `cmp_event_hit_test`: Stub: Mapping coordinates to UI tree nodes is Phase 13 logic handled
- [✓] `cmp_event_handle_tab_targeting`: Stub: In a fully implemented UI tree, this would traverse a11y nodes or DOM

## `src/themes/cmp_cupertino.c`
- [✓] `cmp_cupertino_measure_button`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_text_input`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_slider`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_toggle`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_radio`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_progress`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_measure_dropdown`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_layout_nav_bar`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_layout_tab_bar`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_layout_dialog_content`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_layout_sidebar`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_text_input`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_slider`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_toggle`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_radio`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_progress`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_card`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_tooltip`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_menu`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_draw_focus_ring`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_get_ripple_config`: Empty implementation / void cast stub
- [✓] `cmp_cupertino_get_state_layer_opacity`: Empty implementation / void cast stub

## `src/themes/cmp_fluent2.c`
- [✓] `cmp_fluent2_measure_button`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_text_input`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_slider`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_toggle`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_radio`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_progress`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_measure_dropdown`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_layout_nav_bar`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_layout_tab_bar`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_layout_dialog_content`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_layout_sidebar`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_button`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_text_input`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_slider`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_toggle`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_radio`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_progress`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_card`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_tooltip`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_draw_menu`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_get_spring_config`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_get_ripple_config`: Empty implementation / void cast stub
- [✓] `cmp_fluent2_get_state_layer_opacity`: Empty implementation / void cast stub

## `src/themes/cmp_material3.c`
- [✓] `cmp_material3_draw_button`: TODO: Inject cmp_gpu_t draw calls or display list nodes
- [✓] `cmp_material3_draw_card`: TODO: Inject cmp_gpu_t draw calls or display list nodes
- [✓] `cmp_material3_measure_button`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_text_input`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_slider`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_toggle`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_radio`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_progress`: Empty implementation / void cast stub
- [✓] `cmp_material3_measure_dropdown`: Empty implementation / void cast stub
- [✓] `cmp_material3_layout_nav_bar`: Empty implementation / void cast stub
- [✓] `cmp_material3_layout_tab_bar`: Empty implementation / void cast stub
- [✓] `cmp_material3_layout_dialog_content`: Empty implementation / void cast stub
- [✓] `cmp_material3_layout_sidebar`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_text_input`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_slider`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_toggle`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_radio`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_progress`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_tooltip`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_menu`: Empty implementation / void cast stub
- [✓] `cmp_material3_draw_focus_ring`: Empty implementation / void cast stub
- [✓] `cmp_material3_get_spring_config`: Empty implementation / void cast stub

## `src/themes/cmp_theme_dispatch.c`
- [✓] `Global/Macro`: TODO: We need a global context or window context to provide the default.

## `src/themes/cmp_unstyled.c`
- [✓] `cmp_unstyled_measure_button`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_text_input`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_slider`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_toggle`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_radio`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_progress`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_measure_dropdown`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_layout_nav_bar`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_layout_tab_bar`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_layout_dialog_content`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_layout_sidebar`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_slider`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_toggle`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_checkbox`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_radio`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_progress`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_card`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_tooltip`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_menu`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_draw_focus_ring`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_get_spring_config`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_get_ripple_config`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_get_state_layer_opacity`: Empty implementation / void cast stub
- [✓] `cmp_unstyled_get_transition_duration`: Empty implementation / void cast stub

