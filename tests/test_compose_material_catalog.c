/**
 * @file test_compose_material_catalog.c
 * @brief Comprehensive unit test suite for Compose Material Catalog
 * implementation.
 */

/* clang-format off */
#include "sampler/sampler_error.h"
#include "sampler/sampler_models.h"
#include "sampler/sampler_nav.h"
#include "sampler/sampler_preferences.h"
#include "sampler/sampler_theme.h"
#include "sampler/sampler_top_app_bar.h"
#include "sampler/sampler_specification_screen.h"
#include "sampler/sampler_home.h"
#include "ui_dom_node.h"
#include "ui_engine.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

enum greatest_test_res test_sampler_home_create(void);

SUITE(sampler_catalog_suite);

TEST test_sampler_errors(void) {
  const char *desc = NULL;
  sampler_error_t rc;

  /* Null pointer check */
  rc = sampler_error_to_string(SAMPLER_SUCCESS, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* All valid error codes */
  rc = sampler_error_to_string(SAMPLER_SUCCESS, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_NULL_POINTER, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_OUT_OF_MEMORY, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_INVALID_ARGUMENT, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_OUT_OF_BOUNDS, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_ROUTE_NOT_FOUND, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_STORAGE_FAILURE, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_PARSE_FAILURE, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_DOM_ATTACH_FAILED, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_STYLE_PROP_FAILED, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_LAYOUT_FAILED, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_EVENT_DISPATCH_FAILED, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  rc = sampler_error_to_string(SAMPLER_ERROR_ANIMATION_FAILED, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(desc != NULL);

  /* Unknown code fallback */
  rc = sampler_error_to_string((sampler_error_t)999, &desc);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ("Unknown error", desc);

  PASS();
}

TEST test_sampler_models_catalog(void) {
  const struct sampler_component *const *comps = NULL;
  const struct sampler_component *comp = NULL;
  size_t count = 0;
  sampler_error_t rc;

  /* Null checks */
  rc = sampler_catalog_get_components(NULL, &count);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_catalog_get_components(&comps, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Retrieve all components */
  rc = sampler_catalog_get_components(&comps, &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(SAMPLER_TOTAL_COMPONENTS, (int)count);
  ASSERT_EQ(41, (int)count);
  ASSERT(comps != NULL);
  ASSERT_STR_EQ("Adaptive", comps[0]->name);
  ASSERT_STR_EQ("Typography", comps[40]->name);

  /* Find by ID */
  rc = sampler_catalog_find_component_by_id(1, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_catalog_find_component_by_id(5, &comp);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ("Buttons", comp->name);
  ASSERT_EQ(7, (int)comp->examples_count);

  /* Invalid ID */
  rc = sampler_catalog_find_component_by_id(999, &comp);
  ASSERT_EQ(SAMPLER_ERROR_ROUTE_NOT_FOUND, rc);

  /* Find by Name */
  rc = sampler_catalog_find_component_by_name(NULL, &comp);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_catalog_find_component_by_name("Card", NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  rc = sampler_catalog_find_component_by_name("card", &comp);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ("Card", comp->name);

  rc = sampler_catalog_find_component_by_name("CHECKBOXES", &comp);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ("Checkboxes", comp->name);

  rc = sampler_catalog_find_component_by_name("NonExistentComponent", &comp);
  ASSERT_EQ(SAMPLER_ERROR_ROUTE_NOT_FOUND, rc);

  PASS();
}

TEST test_sampler_models_filter(void) {
  const struct sampler_component *results[SAMPLER_TOTAL_COMPONENTS];
  size_t count = 0;
  sampler_error_t rc;

  /* Null checks */
  rc = sampler_catalog_filter(NULL, 0, NULL, SAMPLER_TOTAL_COMPONENTS, &count);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_catalog_filter(NULL, 0, results, SAMPLER_TOTAL_COMPONENTS, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Empty query returns all components */
  rc = sampler_catalog_filter(NULL, 0, results, SAMPLER_TOTAL_COMPONENTS,
                              &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(41, (int)count);

  rc = sampler_catalog_filter("", 0, results, SAMPLER_TOTAL_COMPONENTS, &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(41, (int)count);

  /* Filter by "Button" */
  rc = sampler_catalog_filter("Button", 0, results, SAMPLER_TOTAL_COMPONENTS,
                              &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(count >=
         5); /* Buttons, Button Groups, Floating Action Buttons, Icon Buttons,
                Segmented Buttons, Split Buttons, Toggle Buttons */

  /* Filter by expressive only */
  rc = sampler_catalog_filter(NULL, 1, results, SAMPLER_TOTAL_COMPONENTS,
                              &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(count > 0 && count < 41);

  /* Non-existent query returns 0 */
  rc = sampler_catalog_filter("ZzzUnknownNonExistentSearchTerm", 0, results,
                              SAMPLER_TOTAL_COMPONENTS, &count);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, (int)count);

  PASS();
}

TEST test_sampler_theme_lifecycle(void) {
  struct sampler_theme theme_default;
  struct sampler_theme theme_custom;
  struct sampler_theme theme_deserialized;
  char buf[512];
  size_t written = 0;
  int equals = 0;
  sampler_error_t rc;

  memset(&theme_default, 0, sizeof(theme_default));
  memset(&theme_custom, 0, sizeof(theme_custom));
  memset(&theme_deserialized, 0, sizeof(theme_deserialized));

  /* Null checks */
  rc = sampler_theme_init_default(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_theme_copy(NULL, &theme_custom);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_theme_copy(&theme_default, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_theme_equals(NULL, &theme_custom, &equals);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_theme_serialize(NULL, buf, sizeof(buf), &written);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_theme_deserialize(NULL, &theme_deserialized);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Default init */
  rc = sampler_theme_init_default(&theme_default);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(SAMPLER_THEME_COLOR_SYSTEM, theme_default.theme_color_mode);
  ASSERT_EQ(1.0f, theme_default.font_scale);
  ASSERT_EQ(1, theme_default.mark_expressive_components);

  /* Copy */
  rc = sampler_theme_copy(&theme_default, &theme_custom);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_theme_equals(&theme_default, &theme_custom, &equals);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, equals);

  /* Modify custom */
  theme_custom.theme_color_mode = SAMPLER_THEME_COLOR_DARK;
  theme_custom.font_scale = 1.5f;
  theme_custom.text_direction = SAMPLER_TEXT_DIRECTION_RTL;
  rc = sampler_theme_equals(&theme_default, &theme_custom, &equals);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, equals);

  /* Serialization buffer too small */
  rc = sampler_theme_serialize(&theme_custom, buf, 10, &written);
  ASSERT_EQ(SAMPLER_ERROR_OUT_OF_BOUNDS, rc);

  /* Serialization & Deserialization round-trip */
  rc = sampler_theme_serialize(&theme_custom, buf, sizeof(buf), &written);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(written > 0);

  rc = sampler_theme_deserialize(buf, &theme_deserialized);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  rc = sampler_theme_equals(&theme_custom, &theme_deserialized, &equals);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, equals);

  PASS();
}

TEST test_sampler_preferences_in_memory(void) {
  struct sampler_preferences *prefs = NULL;
  struct sampler_theme theme;
  struct sampler_theme loaded_theme;
  char route[128];
  int has_val = 0;
  sampler_error_t rc;

  /* Null checks */
  rc = sampler_preferences_create_in_memory(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_preferences_destroy(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Create in-memory */
  rc = sampler_preferences_create_in_memory(&prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(prefs != NULL);

  /* Initially no favorite route */
  rc = sampler_preferences_get_favorite_route(prefs, route, sizeof(route),
                                              &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, has_val);
  ASSERT_STR_EQ("", route);

  /* Save favorite route */
  rc = sampler_preferences_save_favorite_route(prefs, "material3/component/5");
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  rc = sampler_preferences_get_favorite_route(prefs, route, sizeof(route),
                                              &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_STR_EQ("material3/component/5", route);

  /* Clear favorite route */
  rc = sampler_preferences_clear_favorite_route(prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_preferences_get_favorite_route(prefs, route, sizeof(route),
                                              &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, has_val);

  /* Save theme */
  rc = sampler_theme_init_default(&theme);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  theme.theme_color_mode = SAMPLER_THEME_COLOR_LIGHT;
  theme.font_scale = 1.25f;

  rc = sampler_preferences_save_theme(prefs, &theme);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  rc = sampler_preferences_get_theme(prefs, &loaded_theme, &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(SAMPLER_THEME_COLOR_LIGHT, loaded_theme.theme_color_mode);

  /* Destroy */
  rc = sampler_preferences_destroy(&prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(prefs == NULL);

  PASS();
}

TEST test_sampler_preferences_file(void) {
  const char *tmp_path = "test_sampler_prefs.tmp";
  struct sampler_preferences *prefs = NULL;
  struct sampler_theme theme;
  struct sampler_theme loaded_theme;
  char route[128];
  int has_val = 0;
  sampler_error_t rc;

  /* Remove any existing temp file */
  remove(tmp_path);

  /* Create file-backed prefs */
  rc = sampler_preferences_create_file(tmp_path, &prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(prefs != NULL);

  /* Save data */
  rc = sampler_preferences_save_favorite_route(prefs, "material3/component/7");
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  rc = sampler_theme_init_default(&theme);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  theme.theme_color_mode = SAMPLER_THEME_COLOR_DARK;
  theme.font_scale = 1.75f;
  rc = sampler_preferences_save_theme(prefs, &theme);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  /* Close file */
  rc = sampler_preferences_destroy(&prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(prefs == NULL);

  /* Re-open from disk and verify persistence */
  rc = sampler_preferences_create_file(tmp_path, &prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(prefs != NULL);

  rc = sampler_preferences_get_favorite_route(prefs, route, sizeof(route),
                                              &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_STR_EQ("material3/component/7", route);

  rc = sampler_preferences_get_theme(prefs, &loaded_theme, &has_val);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(SAMPLER_THEME_COLOR_DARK, loaded_theme.theme_color_mode);

  /* Clean up */
  rc = sampler_preferences_destroy(&prefs);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  remove(tmp_path);

  PASS();
}

TEST test_sampler_nav_routing(void) {
  struct sampler_nav *nav = NULL;
  const char *route = NULL;
  size_t depth = 0;
  int popped = 0;
  sampler_error_t rc;

  /* Null checks */
  rc = sampler_nav_create(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_nav_destroy(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Create router */
  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(nav != NULL);

  /* Initial state: empty */
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, (int)depth);

  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_ERROR_ROUTE_NOT_FOUND, rc);

  /* Push "specification" */
  rc = sampler_nav_navigate(nav, SAMPLER_ROUTE_SPECIFICATION);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, (int)depth);
  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ(SAMPLER_ROUTE_SPECIFICATION, route);

  /* Push "material3" */
  rc = sampler_nav_navigate(nav, SAMPLER_ROUTE_MATERIAL3);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(2, (int)depth);
  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ(SAMPLER_ROUTE_MATERIAL3, route);

  /* Push component route */
  rc = sampler_nav_navigate(nav, "material3/component/5");
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(3, (int)depth);
  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ("material3/component/5", route);

  /* Pop to material3 */
  rc = sampler_nav_pop(nav, &popped);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, popped);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(2, (int)depth);
  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ(SAMPLER_ROUTE_MATERIAL3, route);

  /* Pop to specification */
  rc = sampler_nav_pop(nav, &popped);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, popped);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, (int)depth);
  rc = sampler_nav_get_current_route(nav, &route);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_STR_EQ(SAMPLER_ROUTE_SPECIFICATION, route);

  /* Pop at root cannot pop further */
  rc = sampler_nav_pop(nav, &popped);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, popped);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(1, (int)depth);

  /* Clear */
  rc = sampler_nav_clear(nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  rc = sampler_nav_get_stack_depth(nav, &depth);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT_EQ(0, (int)depth);

  /* Destroy */
  rc = sampler_nav_destroy(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(nav == NULL);

  PASS();
}

TEST test_sampler_top_app_bar(void) {
  struct sampler_top_app_bar_config cfg;
  struct ui_dom_node *top_bar = NULL;
  struct ui_engine *dummy_engine = NULL;
  struct ui_engine_config engine_cfg;
  sampler_error_t rc;
  ui_error_t u_rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  u_rc = ui_engine_create(&engine_cfg, &dummy_engine);
  ASSERT_EQ(UI_ERROR_NONE, u_rc);

  memset(&cfg, 0, sizeof(cfg));

  /* Null checks */
  rc = sampler_top_app_bar_create(dummy_engine, NULL, &top_bar);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_top_app_bar_create(dummy_engine, &cfg, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_top_app_bar_create(NULL, &cfg, &top_bar);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_top_app_bar_set_pinned(NULL, 1);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Valid creation with back button, favorite pin, theme, more menu */
  memset(&cfg, 0, sizeof(cfg));
  cfg.title = "Compose Material 3";
  cfg.show_back_button = 1;
  cfg.show_search_field = 1;
  cfg.show_favorite_pin = 1;
  cfg.is_favorite_pinned = 0;
  cfg.show_theme_button = 1;
  cfg.show_more_menu = 1;

  rc = sampler_top_app_bar_create(dummy_engine, &cfg, &top_bar);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(top_bar != NULL);

  /* We can't trivially destroy top_bar by destroying root anymore. We'd have to
   * retain the m3_bar in the out params. Since this is just a test of the
   * create function, and the DOM node handles don't automatically free C
   * components unless wired via a component system, we acknowledge a memory
   * leak in this basic test for now, or we'd need to mock the component system.
   * For now just test it doesn't crash on create. */
  /* u_rc = ui_dom_node_destroy(top_bar); */
  /* ASSERT_EQ(UI_ERROR_NONE, u_rc); */

  ui_engine_destroy(dummy_engine);
  PASS();
}

TEST test_sampler_specification_screen(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct sampler_nav *nav = NULL;
  struct sampler_specification_screen *screen = NULL;
  struct ui_dom_node *root_node = NULL;
  sampler_error_t rc;
  ui_error_t u_rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  u_rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, u_rc);
  ASSERT(engine != NULL);

  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  /* Null checks */
  rc = sampler_specification_screen_create(NULL, nav, &screen);
  ASSERT_EQ(SAMPLER_ERROR_INVALID_ARGUMENT, rc);
  rc = sampler_specification_screen_create(engine, NULL, &screen);
  ASSERT_EQ(SAMPLER_ERROR_INVALID_ARGUMENT, rc);
  rc = sampler_specification_screen_create(engine, nav, NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_specification_screen_get_root(NULL, &root_node);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);
  rc = sampler_specification_screen_destroy(NULL);
  ASSERT_EQ(SAMPLER_ERROR_NULL_POINTER, rc);

  /* Valid creation strictly composing M3 widgets */
  rc = sampler_specification_screen_create(engine, nav, &screen);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(screen != NULL);

  rc = sampler_specification_screen_get_root(screen, &root_node);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(root_node != NULL);
  ASSERT_EQ(UI_DOM_NODE_TYPE_ELEMENT, root_node->type);
  ASSERT_STR_EQ("div", root_node->tag_name);

  /* Cleanup */
  rc = sampler_specification_screen_destroy(&screen);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  ASSERT(screen == NULL);

  rc = sampler_nav_destroy(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  u_rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, u_rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_sampler_oom_mocking(void) {
  struct sampler_preferences *prefs = NULL;
  struct sampler_nav *nav = NULL;
  sampler_error_t rc;

  /* Mock OOM on preferences creation (fails on 1st malloc) */
  g_malloc_fail_countdown = 0;
  rc = sampler_preferences_create_in_memory(&prefs);
  ASSERT_EQ(SAMPLER_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(prefs == NULL);
  g_malloc_fail_countdown = -1;

  /* Mock OOM on nav creation (fails on 1st malloc) */
  g_malloc_fail_countdown = 0;
  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(nav == NULL);
  g_malloc_fail_countdown = -1;

  /* Mock OOM on nav creation (fails on 2nd malloc for entries) */
  g_malloc_fail_countdown = 1;
  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(nav == NULL);
  g_malloc_fail_countdown = -1;

  /* Mock OOM on nav navigate (fails on route_copy malloc) */
  rc = sampler_nav_create(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);
  g_malloc_fail_countdown = 0;
  rc = sampler_nav_navigate(nav, "material3");
  ASSERT_EQ(SAMPLER_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  rc = sampler_nav_destroy(&nav);
  ASSERT_EQ(SAMPLER_SUCCESS, rc);

  PASS();
}
#endif

SUITE(sampler_catalog_suite) {
  RUN_TEST(test_sampler_errors);
  RUN_TEST(test_sampler_models_catalog);
  RUN_TEST(test_sampler_models_filter);
  RUN_TEST(test_sampler_theme_lifecycle);
  RUN_TEST(test_sampler_preferences_in_memory);
  RUN_TEST(test_sampler_preferences_file);
  RUN_TEST(test_sampler_nav_routing);
  RUN_TEST(test_sampler_top_app_bar);
  RUN_TEST(test_sampler_specification_screen);
  RUN_TEST(test_sampler_home_create);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_sampler_oom_mocking);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(sampler_catalog_suite);
  GREATEST_MAIN_END();
}
