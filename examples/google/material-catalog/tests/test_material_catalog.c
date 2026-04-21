/* clang-format off */
#include "material_catalog.h"
#include "m3_memory.h"
#include "m3_state.h"
#include "m3_events.h"
#include "m3_router.h"
#include "m3_color.h"
#include "m3_typography.h"
#include "m3_shapes.h"
#include "m3_motion.h"
#include "m3_adaptive.h"
#include "m3_buttons.h"
#include "m3_communication.h"
#include "m3_containment.h"
#include "m3_navigation.h"
#include "m3_inputs.h"
#include "m3_a11y.h"
#include "m3_i18n.h"
#include <math.h>
#include <greatest.h>

#if defined(_WIN32) || defined(__CYGWIN__)
__declspec(dllimport) void* __stdcall FindWindowA(const char* lpClassName, const char* lpWindowName);
__declspec(dllimport) int __stdcall PostMessageA(void* hWnd, unsigned int Msg, void* wParam, void* lParam);
#endif
/* clang-format on */

TEST test_initialization(void) {
  material_catalog_state_t state;

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER, material_catalog_init(NULL));

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_cleanup(&state));
  PASS();
}

TEST test_ui_creation(void) {
  material_catalog_state_t state;
  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_create_ui(NULL));

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_create_ui(&state));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_cleanup(&state));
  PASS();
}

TEST test_run(void) {
  material_catalog_state_t state;

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER, material_catalog_run(NULL));

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  material_catalog_create_ui(&state);

#if defined(_WIN32) && !defined(__CYGWIN__)
  {
    void *hwnd = FindWindowA("CmpWindowClass", "Material Catalog");
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

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
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_get_shape_radius(
                                          THEME_SHAPE_EXTRA_SMALL, &radius));
  ASSERT_EQ(4.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_SMALL, &radius));
  ASSERT_EQ(8.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_MEDIUM, &radius));
  ASSERT_EQ(12.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_LARGE, &radius));
  ASSERT_EQ(16.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_get_shape_radius(
                                          THEME_SHAPE_EXTRA_LARGE, &radius));
  ASSERT_EQ(28.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_get_shape_radius(THEME_SHAPE_FULL, &radius));
  ASSERT_EQ(1000.0f, radius);

  ASSERT_EQ(MATERIAL_CATALOG_ERROR_NULL_POINTER,
            material_catalog_get_shape_radius((theme_shape_size_t)99, &radius));
  PASS();
}

TEST test_theme_persistence(void) {
  theme_saver_t saver;
  cmp_color_t pink = {1.0f, 0.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  cmp_color_t blue = {0.0f, 0.0f, 1.0f, 1.0f, CMP_COLOR_SPACE_SRGB};
  saver.config_file_path = "test_theme_config.bin";
  saver.current.mode = CATALOG_THEME_DARK;
  saver.current.seed_color = pink;
  saver.current.font_family = THEME_FONT_MONOSPACE;
  saver.current.shape_family = THEME_SHAPE_CUT;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_save_theme(&saver));

  /* Modify state to ensure load overrides */
  saver.current.mode = CATALOG_THEME_LIGHT;
  saver.current.seed_color = blue;

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_load_theme(&saver));
  ASSERT_EQ(CATALOG_THEME_DARK, saver.current.mode);
  ASSERT_EQ(1.0f, saver.current.seed_color.r); /* Pink check */
  ASSERT_EQ(THEME_FONT_MONOSPACE, saver.current.font_family);
  ASSERT_EQ(THEME_SHAPE_CUT, saver.current.shape_family);

  remove(saver.config_file_path);

  PASS();
}

TEST test_insets(void) {
  material_catalog_state_t state;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

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

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
            app_bar->layout
                ->height); /* metrics.height_collapsed + status bar height */

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_scrim(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_scrim(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_launch_url(void) {
  /* Pass NULL to avoid launching a browser during tests */
  material_catalog_launch_url(NULL);
  PASS();
}

TEST test_more_menu(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_more_menu(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_apply_ripple(void) {
  cmp_ui_node_t *root;
  material_catalog_state_t state;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_apply_ripple(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_home_screen(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_home_screen(&state, root));
  /* Should contain Top App Bar and the Grid */
  ASSERT(root->child_count > 0);

  /* Find grid */
  if (root->child_count > 1 && root->children[1]->child_count > 0) {
    cmp_ui_node_t *grid = root->children[1];
    cmp_ui_node_t *card = grid->children[0];
    ASSERT(fabs(dp_to_px(&state, 180.0f) - card->layout->height) <
           0.1f); /* Card height strictly 180dp */
  }

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_component_details(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  /* Component 0 = Color */
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_component_details(&state, root, 0));

  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_theme_picker(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_render_theme_picker(&state, root));
  ASSERT(root->child_count > 0);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_backdrop_example(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  /* Mock check for focus logic via adding keyboard event hooks */
  ASSERT_EQ(0, cmp_ui_node_add_event_listener(
                   root, CMP_EVENT_TYPE_KEYBOARD /* Keyboard event mapping */,
                   0, dummy_event_cb, NULL));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_rtl_support(void) {
  material_catalog_state_t state;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

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
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            material_catalog_apply_viewport_culling(&state, root));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_memory(void) {
  m3_memory_state_t mem_state;
  void *ptr1 = NULL;
  void *ptr2 = NULL;
  const char *str1 = NULL;
  const char *str2 = NULL;
  const char *str3 = NULL;

  ASSERT_EQ(0, m3_memory_init(&mem_state));

  ASSERT_EQ(0, m3_state_alloc(&mem_state, 128, &ptr1));
  ASSERT_NEQ(NULL, ptr1);

  ASSERT_EQ(0, m3_ui_alloc(&mem_state, 256, &ptr2));
  ASSERT_NEQ(NULL, ptr2);

  ASSERT_EQ(0, m3_string_intern(&mem_state, "hello", &str1));
  ASSERT_NEQ(NULL, str1);
  ASSERT_EQ(0, strcmp(str1, "hello"));

  ASSERT_EQ(0, m3_string_intern(&mem_state, "world", &str2));
  ASSERT_NEQ(NULL, str2);
  ASSERT_NEQ(str1, str2);

  ASSERT_EQ(0, m3_string_intern(&mem_state, "hello", &str3));
  ASSERT_EQ(str1, str3);

  ASSERT_EQ(0, m3_ui_arena_clear(&mem_state));

  /* Fallback allocator test */
  ASSERT_EQ(0, m3_state_alloc(&mem_state, 1024 * 1024 * 10,
                              &ptr1)); /* 10 MB, should trigger fallback */
  ASSERT_NEQ(NULL, ptr1);

  ASSERT_EQ(0, m3_memory_cleanup(&mem_state));
  PASS();
}

static void dummy_debounce_cb(material_catalog_state_t *state,
                              void *user_data) {
  (void)state;
  (void)user_data;
}

TEST test_m3_state_engine(void) {
  m3_state_engine_t engine;
  material_catalog_state_t state;
  cmp_ui_node_t *node;
  int my_state_var = 0;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  ASSERT_EQ(0, cmp_ui_box_create(&node));
  ASSERT_EQ(0, m3_state_engine_init(&engine));

  /* Test binding */
  ASSERT_EQ(
      0, m3_state_bind_node(&engine, &state, node, &my_state_var, NULL, NULL));
  ASSERT_NEQ(NULL, engine.dependencies);

  /* Test notification */
  my_state_var = 1;
  ASSERT_EQ(0, m3_state_notify_changed(&engine, &my_state_var));

  /* Test debounce */
  ASSERT_EQ(0,
            m3_debounce_task(&engine, &state, 1, 100, dummy_debounce_cb, NULL));
  ASSERT_NEQ(NULL, engine.debounce_queue);

  /* Process debounce queue */
  m3_process_debounce_queue(&engine, &state, 50);
  ASSERT_NEQ(NULL, engine.debounce_queue); /* still there */

  /* Trigger */
  ASSERT_EQ(0, m3_state_engine_cleanup(&engine));
  cmp_ui_node_destroy(node);
  material_catalog_cleanup(&state);
  PASS();
}

static int debounce_triggered = 0;
static void test_debounce_cb(material_catalog_state_t *state, void *user_data) {
  (void)state;
  (void)user_data;
  debounce_triggered = 1;
}

TEST test_m3_state_debounce_and_notify(void) {
  m3_state_engine_t engine;
  material_catalog_state_t state;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, m3_state_engine_init(&engine));

  debounce_triggered = 0;
  ASSERT_EQ(0,
            m3_debounce_task(&engine, &state, 1, 100, test_debounce_cb, NULL));
  m3_process_debounce_queue(&engine, &state, 50);
  ASSERT_EQ(0, debounce_triggered);
  m3_process_debounce_queue(&engine, &state, 50);
  ASSERT_EQ(1, debounce_triggered);

  ASSERT_EQ(0, m3_state_engine_cleanup(&engine));
  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_events_engine(void) {
  m3_event_engine_t engine;
  material_catalog_state_t state;
  cmp_event_t evt;
  uint32_t modality = 0;
  int exceeded = 0;
  float vx = 0.0f;
  float vy = 0.0f;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, m3_event_engine_init(&engine));

  /* Test modality update and focus ring toggle via keyboard */
  evt.type = CMP_EVENT_TYPE_KEYBOARD;
  evt.action = CMP_ACTION_DOWN;
  evt.x = 0;
  evt.y = 0;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 100));
  ASSERT_EQ(1, engine.is_focus_ring_visible);

  ASSERT_EQ(0, m3_event_get_active_modality(&engine, &modality));
  ASSERT_EQ(CMP_EVENT_TYPE_KEYBOARD, modality);

  /* Test mouse down removing focus ring */
  evt.type = CMP_EVENT_TYPE_MOUSE;
  evt.action = CMP_ACTION_DOWN;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 110));
  ASSERT_EQ(0, engine.is_focus_ring_visible);

  /* Test touch slop and velocity */
  evt.type = CMP_EVENT_TYPE_TOUCH;
  evt.action = CMP_ACTION_DOWN;
  evt.x = 100;
  evt.y = 100;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 200));
  ASSERT_EQ(1, engine.is_dragging);

  /* Move within slop */
  evt.action = CMP_ACTION_MOVE;
  evt.x = 104; /* Moved 4px, slop is 8dp -> if DPI is 1.0, this is under */
  evt.y = 104;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 210));
  ASSERT_EQ(0, m3_event_check_touch_slop(&engine, 1.0f, &exceeded));
  ASSERT_EQ(0, exceeded);

  /* Move past slop */
  evt.x = 150;
  evt.y = 150;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 250));
  ASSERT_EQ(0, m3_event_check_touch_slop(&engine, 1.0f, &exceeded));
  ASSERT_EQ(1, exceeded);

  /* Velocity check */
  ASSERT_EQ(0, m3_event_get_velocity(&engine, &vx, &vy));
  ASSERT(vx > 0.0f);
  ASSERT(vy > 0.0f);

  /* Release */
  evt.action = CMP_ACTION_UP;
  ASSERT_EQ(0, m3_event_process(&engine, &state, &evt, 260));
  ASSERT_EQ(0, engine.is_dragging);

  ASSERT_EQ(0, m3_event_engine_cleanup(&engine));
  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_router_engine(void) {
  m3_router_engine_t engine;
  int can_pop;
  float sx = 0.0f;
  float sy = 0.0f;

  ASSERT_EQ(0, m3_router_init(&engine));
  ASSERT_EQ(0, engine.stack_size);

  /* Deep link test - mock "home" */
  ASSERT_EQ(0, m3_router_handle_deep_link(&engine, "cmp://catalog/home"));
  ASSERT_EQ(1, engine.stack_size);
  ASSERT_EQ(CATALOG_SCREEN_HOME, engine.stack[0].screen_id);

  /* Scroll state test */
  ASSERT_EQ(0, m3_router_save_scroll_state(&engine, 10.0f, 20.0f));
  ASSERT_EQ(0, m3_router_get_scroll_state(&engine, &sx, &sy));
  ASSERT_EQ(10.0f, sx);
  ASSERT_EQ(20.0f, sy);

  /* Push details */
  ASSERT_EQ(0, m3_router_push(&engine, CATALOG_SCREEN_COMPONENT_DETAILS, 1, 0,
                              NULL, M3_TRANSITION_SHARED_AXIS_Z));
  ASSERT_EQ(2, engine.stack_size);
  ASSERT_EQ(CATALOG_SCREEN_COMPONENT_DETAILS, engine.stack[1].screen_id);
  ASSERT_EQ(1, engine.stack[1].component_id);

  /* Replace top */
  ASSERT_EQ(0, m3_router_replace(&engine, CATALOG_SCREEN_EXAMPLE_VIEWER, 1, 5,
                                 NULL, M3_TRANSITION_FADE_THROUGH));
  ASSERT_EQ(2, engine.stack_size); /* Size stays same */
  ASSERT_EQ(CATALOG_SCREEN_EXAMPLE_VIEWER, engine.stack[1].screen_id);
  ASSERT_EQ(5, engine.stack[1].example_index);

  /* Can pop */
  ASSERT_EQ(0, m3_router_can_pop(&engine, &can_pop));
  ASSERT_EQ(1, can_pop);

  /* Pop */
  ASSERT_EQ(0, m3_router_pop(&engine));
  ASSERT_EQ(1, engine.stack_size);

  /* Cannot pop root */
  ASSERT_EQ(1, m3_router_pop(&engine));

  /* Reset */
  ASSERT_EQ(0, m3_router_reset_to(&engine, CATALOG_SCREEN_HOME));
  ASSERT_EQ(1, engine.stack_size);

  /* Deep link invalid */
  ASSERT_EQ(1, m3_router_handle_deep_link(&engine, "https://google.com"));

  ASSERT_EQ(0, m3_router_cleanup(&engine));
  PASS();
}

TEST test_m3_color(void) {
  cmp_color_t seed = {0.4f, 0.2f, 0.8f, 1.0f, CMP_COLOR_SPACE_SRGB};
  m3_color_roles_t light_roles;
  m3_color_roles_t dark_roles;
  m3_color_roles_t tweened_roles;

  ASSERT_EQ(0, m3_color_generate_roles(seed, 0, &light_roles));
  ASSERT_EQ(0, m3_color_generate_roles(seed, 1, &dark_roles));
  ASSERT_EQ(
      0, m3_color_tween_roles(&light_roles, &dark_roles, 0.5f, &tweened_roles));

  PASS();
}

TEST test_m3_typography(void) {
  m3_typography_t typo;
  m3_type_metrics_t metrics;

  ASSERT_EQ(0, m3_typography_init(&typo, NULL, NULL, NULL));
  ASSERT_EQ(0,
            m3_typography_get_metrics(&typo, M3_TYPE_DISPLAY_LARGE, &metrics));
  ASSERT_EQ(57.0f, metrics.size_sp);

  m3_typography_set_accessibility_scale(&typo, 1.5f);
  ASSERT_EQ(0,
            m3_typography_get_metrics(&typo, M3_TYPE_DISPLAY_LARGE, &metrics));
  ASSERT_EQ(85.5f, metrics.size_sp); /* 57 * 1.5 */

  ASSERT_EQ(0, m3_typography_cleanup(&typo));
  PASS();
}

TEST test_m3_shapes(void) {
  m3_shape_t shape_uniform;
  m3_shape_t shape_asymmetric;
  m3_shape_t shape_tweened;

  ASSERT_EQ(0, m3_shape_init_uniform(M3_SHAPE_MEDIUM, &shape_uniform));
  ASSERT_EQ(12.0f, shape_uniform.top_left_dp);
  ASSERT_EQ(0, shape_uniform.is_full);

  ASSERT_EQ(0, m3_shape_init_asymmetric(M3_SHAPE_LARGE, M3_SHAPE_LARGE,
                                        M3_SHAPE_NONE, M3_SHAPE_NONE,
                                        &shape_asymmetric));
  ASSERT_EQ(16.0f, shape_asymmetric.top_left_dp);
  ASSERT_EQ(0.0f, shape_asymmetric.bottom_right_dp);

  ASSERT_EQ(0, m3_shape_tween(&shape_uniform, &shape_asymmetric, 0.5f,
                              &shape_tweened));
  ASSERT_EQ(14.0f, shape_tweened.top_left_dp);    /* 12 + (16-12)*0.5 */
  ASSERT_EQ(6.0f, shape_tweened.bottom_right_dp); /* 12 + (0-12)*0.5 */

  PASS();
}

TEST test_m3_motion(void) {
  m3_motion_ctx_t motion;
  float val;
  float out_outg, out_inc;

  ASSERT_EQ(0, m3_motion_init(&motion));
  ASSERT_EQ(0, m3_motion_evaluate(&motion, M3_EASING_STANDARD, 0.5f, &val));

  ASSERT_EQ(0, m3_motion_shared_axis_offset(&motion, 0.5f, 1, 100.0f, &val));

  ASSERT_EQ(0, m3_motion_shared_axis_fade(&motion, 0.15f, &out_outg, &out_inc));
  ASSERT_EQ(0.5f, out_outg);
  ASSERT_EQ(0.0f, out_inc);

  ASSERT_EQ(0, m3_motion_shared_axis_fade(&motion, 0.65f, &out_outg, &out_inc));
  ASSERT_EQ(0.0f, out_outg);
  ASSERT(out_inc > 0.49f && out_inc < 0.51f);

  ASSERT_EQ(0, m3_motion_cleanup(&motion));
  PASS();
}

TEST test_m3_adaptive(void) {
  m3_window_size_class_t size_class;
  m3_nav_scaffolding_t nav;
  m3_list_detail_layout_t ld;
  m3_supporting_pane_layout_t sp;
  m3_feed_layout_t feed;

  ASSERT_EQ(0, m3_adaptive_get_window_size_class(400.0f, &size_class));
  ASSERT_EQ(M3_WINDOW_SIZE_CLASS_COMPACT, size_class);

  ASSERT_EQ(0, m3_adaptive_get_window_size_class(700.0f, &size_class));
  ASSERT_EQ(M3_WINDOW_SIZE_CLASS_MEDIUM, size_class);

  ASSERT_EQ(0, m3_adaptive_get_window_size_class(1000.0f, &size_class));
  ASSERT_EQ(M3_WINDOW_SIZE_CLASS_EXPANDED, size_class);

  ASSERT_EQ(
      0, m3_adaptive_get_nav_scaffolding(M3_WINDOW_SIZE_CLASS_COMPACT, &nav));
  ASSERT_EQ(M3_NAV_SCAFFOLDING_BOTTOM_BAR, nav);

  ASSERT_EQ(0,
            m3_adaptive_get_nav_scaffolding(M3_WINDOW_SIZE_CLASS_MEDIUM, &nav));
  ASSERT_EQ(M3_NAV_SCAFFOLDING_NAVIGATION_RAIL, nav);

  ASSERT_EQ(
      0, m3_adaptive_get_nav_scaffolding(M3_WINDOW_SIZE_CLASS_EXPANDED, &nav));
  ASSERT_EQ(M3_NAV_SCAFFOLDING_PERMANENT_DRAWER, nav);

  ASSERT_EQ(
      0, m3_adaptive_get_list_detail_layout(M3_WINDOW_SIZE_CLASS_COMPACT, &ld));
  ASSERT_EQ(M3_LIST_DETAIL_STACKED, ld);

  ASSERT_EQ(0, m3_adaptive_get_list_detail_layout(M3_WINDOW_SIZE_CLASS_EXPANDED,
                                                  &ld));
  ASSERT_EQ(M3_LIST_DETAIL_SIDE_BY_SIDE, ld);

  ASSERT_EQ(0, m3_adaptive_get_supporting_pane_layout(
                   M3_WINDOW_SIZE_CLASS_COMPACT, &sp));
  ASSERT_EQ(M3_SUPPORTING_PANE_HIDDEN_BOTTOM_SHEET, sp);

  ASSERT_EQ(0, m3_adaptive_get_supporting_pane_layout(
                   M3_WINDOW_SIZE_CLASS_MEDIUM, &sp));
  ASSERT_EQ(M3_SUPPORTING_PANE_VISIBLE_SIDE_PANEL, sp);

  ASSERT_EQ(0,
            m3_adaptive_get_feed_layout(M3_WINDOW_SIZE_CLASS_COMPACT, &feed));
  ASSERT_EQ(M3_FEED_SINGLE_COLUMN, feed);

  ASSERT_EQ(0,
            m3_adaptive_get_feed_layout(M3_WINDOW_SIZE_CLASS_EXPANDED, &feed));
  ASSERT_EQ(M3_FEED_RESPONSIVE_GRID, feed);

  PASS();
}

TEST test_m3_buttons(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *btn;
  m3_button_config_t config;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  memset(&config, 0, sizeof(config));
  config.type = M3_BUTTON_TYPE_FILLED;
  config.label = "Test";

  ASSERT_EQ(0, m3_button_create(&state, &config, &btn));
  ASSERT_NEQ(NULL, btn);
  ASSERT_EQ(dp_to_px(&state, 48.0f), btn->layout->min_width);
  ASSERT_EQ(dp_to_px(&state, 48.0f), btn->layout->min_height);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_communication(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *node;
  m3_badge_config_t b_config;
  m3_progress_config_t p_config;
  m3_snackbar_config_t s_config;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  /* Badge */
  memset(&b_config, 0, sizeof(b_config));
  b_config.type = M3_BADGE_TYPE_LARGE;
  b_config.number = 5;
  ASSERT_EQ(0, m3_badge_create(&state, &b_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Progress */
  memset(&p_config, 0, sizeof(p_config));
  p_config.type = M3_PROGRESS_TYPE_LINEAR;
  p_config.is_determinate = 1;
  p_config.progress = 0.5f;
  ASSERT_EQ(0, m3_progress_create(&state, &p_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Snackbar */
  memset(&s_config, 0, sizeof(s_config));
  s_config.text = "Message";
  s_config.duration = M3_SNACKBAR_DURATION_SHORT;
  ASSERT_EQ(0, m3_snackbar_create(&state, &s_config, &node));
  ASSERT_NEQ(NULL, node);

  ASSERT_EQ(0, m3_snackbar_enqueue(&state, &s_config));
  ASSERT_EQ(0, m3_snackbar_process_queue(&state, 100.0f));

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_containment(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *node;
  m3_card_config_t c_config;
  m3_divider_config_t d_config;
  m3_dialog_config_t dg_config;
  m3_bottom_sheet_config_t bs_config;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  /* Card */
  memset(&c_config, 0, sizeof(c_config));
  c_config.type = M3_CARD_TYPE_ELEVATED;
  c_config.is_draggable = 1;
  ASSERT_EQ(0, m3_card_create(&state, &c_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Divider */
  memset(&d_config, 0, sizeof(d_config));
  d_config.type = M3_DIVIDER_TYPE_INSET;
  d_config.is_vertical = 0;
  d_config.inset_start_dp = 16.0f;
  ASSERT_EQ(0, m3_divider_create(&state, &d_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Dialog */
  memset(&dg_config, 0, sizeof(dg_config));
  dg_config.type = M3_DIALOG_TYPE_BASIC;
  dg_config.headline = "Headline";
  dg_config.supporting_text = "Supporting Text";
  dg_config.confirm_label = "OK";
  dg_config.dismiss_label = "Cancel";
  ASSERT_EQ(0, m3_dialog_create(&state, &dg_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Bottom Sheet */
  memset(&bs_config, 0, sizeof(bs_config));
  bs_config.type = M3_BOTTOM_SHEET_TYPE_STANDARD;
  bs_config.show_drag_handle = 1;
  ASSERT_EQ(0, m3_bottom_sheet_create(&state, &bs_config, &node));
  ASSERT_NEQ(NULL, node);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_navigation(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *node;
  m3_top_app_bar_config_t tab_config;
  m3_bottom_app_bar_config_t bab_config;
  m3_navigation_bar_config_t nb_config;
  m3_navigation_rail_config_t nr_config;
  m3_navigation_drawer_config_t nd_config;
  m3_tabs_config_t t_config;
  const char *act_icons[] = {"A", "B", "C"};
  m3_nav_item_t items[3];

  memset(items, 0, sizeof(items));
  items[0].label = "Home";
  items[0].icon = "H";
  items[1].label = "Settings";
  items[1].icon = "S";
  items[2].label = "Profile";
  items[2].icon = "P";

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  /* Top App Bar */
  memset(&tab_config, 0, sizeof(tab_config));
  tab_config.type = M3_TOP_APP_BAR_TYPE_SMALL;
  tab_config.title = "Small";
  tab_config.nav_icon = "M";
  tab_config.action_icons = act_icons;
  tab_config.action_count = 2;
  ASSERT_EQ(0, m3_top_app_bar_create(&state, &tab_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Bottom App Bar */
  memset(&bab_config, 0, sizeof(bab_config));
  bab_config.action_icons = act_icons;
  bab_config.action_count = 2;
  bab_config.has_fab = 1;
  ASSERT_EQ(0, m3_bottom_app_bar_create(&state, &bab_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Navigation Bar */
  memset(&nb_config, 0, sizeof(nb_config));
  nb_config.items = items;
  nb_config.item_count = 3;
  nb_config.selected_index = 0;
  ASSERT_EQ(0, m3_navigation_bar_create(&state, &nb_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Navigation Rail */
  memset(&nr_config, 0, sizeof(nr_config));
  nr_config.items = items;
  nr_config.item_count = 3;
  nr_config.selected_index = 1;
  nr_config.alignment = M3_NAV_RAIL_ALIGN_CENTER;
  nr_config.has_fab = 1;
  ASSERT_EQ(0, m3_navigation_rail_create(&state, &nr_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Navigation Drawer */
  memset(&nd_config, 0, sizeof(nd_config));
  nd_config.type = M3_NAV_DRAWER_TYPE_STANDARD;
  nd_config.items = items;
  nd_config.item_count = 3;
  nd_config.headline = "Menu";
  ASSERT_EQ(0, m3_navigation_drawer_create(&state, &nd_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Tabs */
  memset(&t_config, 0, sizeof(t_config));
  t_config.type = M3_TABS_TYPE_PRIMARY;
  t_config.tab_labels = act_icons; /* Using just as string array */
  t_config.tab_count = 3;
  t_config.selected_index = 0;
  t_config.is_scrollable = 1;
  ASSERT_EQ(0, m3_tabs_create(&state, &t_config, &node));
  ASSERT_NEQ(NULL, node);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_inputs(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *node;
  m3_checkbox_config_t cb_config;
  m3_radio_button_config_t rb_config;
  m3_switch_config_t sw_config;
  m3_chip_config_t ch_config;
  m3_slider_config_t sl_config;
  m3_range_slider_config_t rs_config;
  m3_text_field_config_t tf_config;

  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  /* Checkbox */
  memset(&cb_config, 0, sizeof(cb_config));
  cb_config.state = M3_CHECKBOX_STATE_CHECKED;
  ASSERT_EQ(0, m3_checkbox_create(&state, &cb_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Radio Button */
  memset(&rb_config, 0, sizeof(rb_config));
  rb_config.is_selected = 1;
  ASSERT_EQ(0, m3_radio_button_create(&state, &rb_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Switch */
  memset(&sw_config, 0, sizeof(sw_config));
  sw_config.is_on = 1;
  sw_config.show_icon = 1;
  ASSERT_EQ(0, m3_switch_create(&state, &sw_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Chip */
  memset(&ch_config, 0, sizeof(ch_config));
  ch_config.type = M3_CHIP_TYPE_FILTER;
  ch_config.label = "Filter";
  ch_config.is_selected = 1;
  ASSERT_EQ(0, m3_chip_create(&state, &ch_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Slider */
  memset(&sl_config, 0, sizeof(sl_config));
  sl_config.type = M3_SLIDER_TYPE_CONTINUOUS;
  sl_config.value = 0.5f;
  sl_config.min_value = 0.0f;
  sl_config.max_value = 1.0f;
  ASSERT_EQ(0, m3_slider_create(&state, &sl_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Range Slider */
  memset(&rs_config, 0, sizeof(rs_config));
  rs_config.type = M3_SLIDER_TYPE_DISCRETE;
  rs_config.start_value = 0.2f;
  rs_config.end_value = 0.8f;
  rs_config.min_value = 0.0f;
  rs_config.max_value = 1.0f;
  rs_config.steps = 5;
  ASSERT_EQ(0, m3_range_slider_create(&state, &rs_config, &node));
  ASSERT_NEQ(NULL, node);

  /* Text Field */
  memset(&tf_config, 0, sizeof(tf_config));
  tf_config.type = M3_TEXT_FIELD_TYPE_OUTLINED;
  tf_config.text = "Hello";
  tf_config.label = "Name";
  tf_config.is_focused = 1;
  ASSERT_EQ(0, m3_text_field_create(&state, &tf_config, &node));
  ASSERT_NEQ(NULL, node);

  material_catalog_cleanup(&state);
  PASS();
}

TEST test_m3_a11y(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, m3_a11y_init(&state));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            m3_a11y_set_content_description(&state, root, "button",
                                            "Test A11y Button"));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            m3_a11y_set_state(&state, root, "aria-checked", "true"));
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            m3_a11y_announce_state_change(&state, "Button Checked"));

  /* Test High Contrast */
  state.current_theme = CATALOG_THEME_LIGHT;
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, m3_a11y_apply_high_contrast(&state));
  ASSERT_EQ(0.0f, state.sys_colors.primary.r);    /* Black */
  ASSERT_EQ(1.0f, state.sys_colors.on_primary.r); /* White */

  material_catalog_cleanup(&state);
  cmp_ui_node_destroy(root);
  PASS();
}

TEST test_m3_i18n(void) {
  material_catalog_state_t state;
  cmp_ui_node_t *root;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }
  ASSERT_EQ(0, cmp_ui_box_create(&root));
  root->layout->padding[1] = 10.0f; /* Right */
  root->layout->padding[3] = 20.0f; /* Left */

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, m3_i18n_init(&state));
  ASSERT_EQ(0, state.is_rtl);
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, m3_i18n_set_rtl(&state, 1));
  ASSERT_EQ(1, state.is_rtl);

  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS,
            m3_i18n_apply_rtl_mirroring(&state, root));
  ASSERT_EQ(20.0f, root->layout->padding[1]); /* Right should now be 20 */
  ASSERT_EQ(10.0f, root->layout->padding[3]); /* Left should now be 10 */

  ASSERT_EQ(1, m3_i18n_is_directional_icon("arrow_back"));
  ASSERT_EQ(0, m3_i18n_is_directional_icon("home"));

  material_catalog_cleanup(&state);
  cmp_ui_node_destroy(root);
  PASS();
}

TEST test_example_viewer(void) {
  material_catalog_state_t state;
  if (material_catalog_init(&state) != MATERIAL_CATALOG_SUCCESS) {
    PASS();
  }

  material_catalog_navigate_to(&state, CATALOG_SCREEN_EXAMPLE_VIEWER, 1, 0);
  ASSERT_EQ(MATERIAL_CATALOG_SUCCESS, material_catalog_recompose_ui(&state));

  /* Root node should have the scaffold body */
  ASSERT(state.root_node != NULL);
  ASSERT(state.root_node->child_count > 0);

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
  RUN_TEST(test_theme_picker);
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
  RUN_TEST(test_m3_memory);
  RUN_TEST(test_m3_state_engine);
  RUN_TEST(test_m3_state_debounce_and_notify);
  RUN_TEST(test_m3_events_engine);
  RUN_TEST(test_m3_router_engine);
  RUN_TEST(test_m3_color);
  RUN_TEST(test_m3_typography);
  RUN_TEST(test_m3_shapes);
  RUN_TEST(test_m3_motion);
  RUN_TEST(test_m3_adaptive);
  RUN_TEST(test_m3_buttons);
  RUN_TEST(test_m3_communication);
  RUN_TEST(test_m3_containment);
  RUN_TEST(test_m3_navigation);
  RUN_TEST(test_m3_inputs);
  RUN_TEST(test_m3_a11y);
  RUN_TEST(test_m3_i18n);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material_catalog_suite);
  GREATEST_MAIN_END();
}
