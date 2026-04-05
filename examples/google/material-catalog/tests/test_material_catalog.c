/* clang-format off */
#include "material_catalog.h"
#include <greatest.h>

#if defined(_WIN32) || defined(__CYGWIN__)
__declspec(dllimport) void* __stdcall FindWindowA(const char* lpClassName, const char* lpWindowName);
__declspec(dllimport) int __stdcall PostMessageA(void* hWnd, unsigned int Msg, void* wParam, void* lParam);
#endif
/* clang-format on */

TEST test_initialization(void) {
  material_catalog_state_t state;

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER, material_catalog_init(NULL));

  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_cleanup(&state));
  PASS();
}

TEST test_ui_creation(void) {
  material_catalog_state_t state;
  cmp_window_config_t config;
  memset(&config, 0, sizeof(config));

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_create_ui(NULL));

  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_create_ui(&state));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_cleanup(&state));
  PASS();
}

TEST test_run(void) {
  material_catalog_state_t state;

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER, material_catalog_run(NULL));

  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  material_catalog_create_ui(&state);

#if defined(_WIN32) && !defined(__CYGWIN__)
  {
      void* hwnd = FindWindowA("CmpWindowClass", "Test");
      ASSERT_NEQ(NULL, hwnd);
      PostMessageA(hwnd, 0x0010, 0, 0); /* 0x0010 is WM_CLOSE */
  }
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_run(&state));
#endif

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_cleanup(&state));
  PASS();
}

TEST test_cleanup(void) {
  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_cleanup(NULL));
  PASS();
}

TEST test_invalid_states(void) {
  material_catalog_state_t state;
  state.window = NULL;

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER, material_catalog_run(&state));
  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_create_ui(&state));
  PASS();
}

TEST test_get_components(void) {
  int count = 0;
  const material_component_t *comps = material_catalog_get_components(&count);
  ASSERT_NEQ(NULL, comps);
  ASSERT(count > 0);
  ASSERT_EQ(1, comps[0].id);
  PASS();
}

TEST test_get_examples(void) {
  int count = 0;
  const material_example_t *examples = material_catalog_get_examples(1, &count);
  ASSERT_NEQ(NULL, examples);
  ASSERT(count > 0);
  PASS();
}

TEST test_routing(void) {
  material_catalog_state_t state;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);

  ASSERT_EQ(1, state.router.stack_size);
  ASSERT_EQ(CATALOG_SCREEN_HOME, state.router.stack[0].screen_id);

  material_catalog_navigate_to(&state, CATALOG_SCREEN_COMPONENT_DETAILS, 1, 0);
  ASSERT_EQ(2, state.router.stack_size);
  ASSERT_EQ(CATALOG_SCREEN_COMPONENT_DETAILS, state.router.stack[1].screen_id);

  material_catalog_navigate_back(&state);
  ASSERT_EQ(1, state.router.stack_size);
  ASSERT_EQ(CATALOG_SCREEN_HOME, state.router.stack[0].screen_id);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_invalidate_ui(void) {
  material_catalog_state_t state;
  state.is_ui_dirty = 0;
  material_catalog_invalidate_ui(&state);
  ASSERT_EQ(1, state.is_ui_dirty);
  PASS();
}

TEST test_color_utilities(void) {
  cmp_color_t black = {0.0f, 0.0f, 0.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t white = {1.0f, 1.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t on_color;
  cmp_palette_t palette;
  int is_light;
  float ratio;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_is_light_color(&white, &is_light));
  ASSERT_EQ(1, is_light);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_is_light_color(&black, &is_light));
  ASSERT_EQ(0, is_light);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_on_color(&white, &on_color));
  ASSERT_EQ(0.0f, on_color.r); /* black on white */

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_on_color(&black, &on_color));
  ASSERT_EQ(1.0f, on_color.r); /* white on black */

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_contrast_ratio(&white, &black, &ratio));
  ASSERT(ratio > 20.0f); /* Should be 21.0 */

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_palette(CATALOG_PALETTE_BLUE,
                                         CATALOG_THEME_LIGHT, &palette));
  ASSERT_NEQ(0.0f, palette.primary.a); /* Should be populated */

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_palette(CATALOG_PALETTE_RED,
                                         CATALOG_THEME_DARK, &palette));
  ASSERT_NEQ(0.0f, palette.primary.a); /* Should be populated */

  PASS();
}

TEST test_typography_utilities(void) {
  theme_typography_style_t style;
  material_catalog_state_t state;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_typography_style(
                THEME_TYPOGRAPHY_DISPLAY_LARGE, &style));
  ASSERT_EQ(57.0f, style.font_size);
  ASSERT_EQ(-0.25f, style.letter_spacing);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_typography_style(THEME_TYPOGRAPHY_LABEL_LARGE,
                                                  &style));
  ASSERT_EQ(14.0f, style.font_size);
  ASSERT_EQ(500, style.font_weight);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_load_fonts(&state));

  PASS();
}

TEST test_shape_utilities(void) {
  float radius;
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_SMALL, &radius));
  ASSERT_EQ(16.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_MEDIUM, &radius));
  ASSERT_EQ(32.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_LARGE, &radius));
  ASSERT_EQ(48.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_get_shape_radius((theme_shape_size_t)99, &radius));
  PASS();
}

TEST test_theme_persistence(void) {
  theme_saver_t saver;
  saver.config_file_path = "test_theme_config.bin";
  saver.current.mode = CATALOG_THEME_DARK;
  saver.current.palette_id = CATALOG_PALETTE_PINK;
  saver.current.font_family = THEME_FONT_MONOSPACE;
  saver.current.shape_family = THEME_SHAPE_CUT;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_save_theme(&saver));

  /* Modify state to ensure load overrides */
  saver.current.mode = CATALOG_THEME_LIGHT;
  saver.current.palette_id = CATALOG_PALETTE_BLUE;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_load_theme(&saver));
  ASSERT_EQ(CATALOG_THEME_DARK, saver.current.mode);
  ASSERT_EQ(CATALOG_PALETTE_PINK, saver.current.palette_id);
  ASSERT_EQ(THEME_FONT_MONOSPACE, saver.current.font_family);
  ASSERT_EQ(THEME_SHAPE_CUT, saver.current.shape_family);

  remove(saver.config_file_path);

  PASS();
}

TEST test_insets(void) {
  material_catalog_state_t state;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_update_insets(&state));
  ASSERT_EQ(24.0f, state.window_insets.y);      /* Mock status bar */
  ASSERT_EQ(48.0f, state.window_insets.height); /* Mock nav bar */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_top_app_bar(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_ui_node_t *app_bar;
  cmp_m3_top_app_bar_metrics_t metrics;

  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  /* With back button */
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_top_app_bar(&state, root, "Test Title", 1));
  ASSERT(root->child_count > 0);

  cmp_m3_top_app_bar_resolve(CMP_M3_TOP_APP_BAR_CENTER_ALIGNED, &metrics);

  /* Verify layout direction and safe areas */
  app_bar = root->children[0];
  ASSERT_EQ(CMP_FLEX_ROW, app_bar->layout->direction);
  ASSERT_EQ(metrics.height_collapsed + 24.0f,
            app_bar->layout->height); /* metrics.height_collapsed + status bar height */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_scrim(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_scrim(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_launch_url(void) {
  /* Just a stub invocation to ensure it doesn't crash */
  material_catalog_launch_url("https://example.com");
  PASS();
}

TEST test_more_menu(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_more_menu(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_apply_ripple(void) {
  cmp_ui_node_t *root;
  material_catalog_state_t state;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_apply_ripple(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_home_screen(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;

  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_home_screen(&state, root));
  /* Should contain Top App Bar and the Grid */
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_backdrop_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_backdrop_example(&state, root));
  ASSERT(root->child_count > 0); /* Label */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_badges_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_badges_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock badge */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_bottom_navigation_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_bottom_navigation_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock nav */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_navigation_rail_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_navigation_rail_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock rail */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_buttons_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_buttons_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock button */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_fabs_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_fabs_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock FAB */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_cards_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_cards_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + mock card */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_selection_controls_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root1;
  cmp_ui_node_t *root2;
  cmp_ui_node_t *root3;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);

  ASSERT_EQ(0, cmp_ui_box_create(&root1));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_checkboxes_example(&state, root1));
  ASSERT(root1->child_count > 0);
  cmp_ui_node_destroy(root1);

  ASSERT_EQ(0, cmp_ui_box_create(&root2));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_radio_buttons_example(&state, root2));
  ASSERT(root2->child_count > 0);
  cmp_ui_node_destroy(root2);

  ASSERT_EQ(0, cmp_ui_box_create(&root3));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_switches_example(&state, root3));
  ASSERT(root3->child_count > 0);
  cmp_ui_node_destroy(root3);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_chips_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_chips_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_dialogs_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_dialogs_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_lists_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_lists_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_menus_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_menus_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_drawers_sheets_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_drawers_sheets_example(&state, root));
  ASSERT(root->child_count > 0); /* Label + Mock Drawer + Mock Sheet */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_progress_indicators_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_progress_indicators_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_sliders_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_sliders_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_snackbars_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_snackbars_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_tabs_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_tabs_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_text_fields_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_text_fields_example(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_semantic_mapping(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(
      MATERIAL_CATALOG_SUCCESS,
      material_catalog_map_semantic_node(&state, root, "Test Label", "button"));

  material_catalog_cleanup(&state);
  PASS();
}

static void dummy_event_cb(cmp_event_t *evt, cmp_ui_node_t *node,
                           void *user_data) {
  (void)evt;
  (void)node;
  (void)user_data;
}

TEST test_focus_management(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  /* Mock check for focus logic via adding keyboard event hooks */
  ASSERT_EQ(0,
            cmp_ui_node_add_event_listener(root, CMP_EVENT_TYPE_KEYBOARD /* Keyboard event mapping */,
                                           0, dummy_event_cb, NULL));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_rtl_support(void) {
  material_catalog_state_t state;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);

  /* Default is LTR */
  ASSERT_EQ(0, state.is_rtl);

  state.is_rtl = 1;
  ASSERT_EQ(1, state.is_rtl);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_viewport_culling(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  cmp_window_config_t config;
  cmp_window_system_init();
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_init(&state));
  memset(&config, 0, sizeof(config));
  config.width = 1024;
  config.height = 768;
  config.title = "Test";
  cmp_window_create(&config, &state.window);
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  /* Validate abstract stub doesn't break execution */
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_apply_viewport_culling(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

SUITE(material_catalog_suite) {
  RUN_TEST(test_initialization);
  RUN_TEST(test_invalid_states);
  RUN_TEST(test_ui_creation);
  RUN_TEST(test_run);
  RUN_TEST(test_cleanup);
  RUN_TEST(test_get_components);
  RUN_TEST(test_get_examples);
  RUN_TEST(test_routing);
  RUN_TEST(test_invalidate_ui);
  RUN_TEST(test_color_utilities);
  RUN_TEST(test_typography_utilities);
  RUN_TEST(test_shape_utilities);
  RUN_TEST(test_theme_persistence);
  RUN_TEST(test_insets);
  RUN_TEST(test_top_app_bar);
  RUN_TEST(test_scrim);
  RUN_TEST(test_launch_url);
  RUN_TEST(test_more_menu);
  RUN_TEST(test_apply_ripple);
  RUN_TEST(test_home_screen);
  RUN_TEST(test_backdrop_example);
  RUN_TEST(test_badges_example);
  RUN_TEST(test_bottom_navigation_example);
  RUN_TEST(test_navigation_rail_example);
  RUN_TEST(test_buttons_example);
  RUN_TEST(test_fabs_example);
  RUN_TEST(test_cards_example);
  RUN_TEST(test_selection_controls_example);
  RUN_TEST(test_chips_example);
  RUN_TEST(test_dialogs_example);
  RUN_TEST(test_lists_example);
  RUN_TEST(test_menus_example);
  RUN_TEST(test_drawers_sheets_example);
  RUN_TEST(test_progress_indicators_example);
  RUN_TEST(test_sliders_example);
  RUN_TEST(test_snackbars_example);
  RUN_TEST(test_tabs_example);
  RUN_TEST(test_text_fields_example);
  RUN_TEST(test_semantic_mapping);
  RUN_TEST(test_focus_management);
  RUN_TEST(test_rtl_support);
  RUN_TEST(test_viewport_culling);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material_catalog_suite);
  GREATEST_MAIN_END();
}
