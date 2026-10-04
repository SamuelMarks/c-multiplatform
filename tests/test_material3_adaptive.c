/**
 * @file test_material3_adaptive.c
 * @brief Unit tests for Material 3 Adaptive Scaffolds.
 */

/* clang-format off */
#include "material3/md3_adaptive.h"
#include "ui_engine.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

SUITE(md3_adaptive_suite);

TEST test_md3_list_detail_pane_scaffold(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_list_detail_pane_scaffold *s = NULL;
  struct ui_adaptive_pane_scaffold_base *base = NULL;
  struct ui_component *p1 = NULL;
  struct ui_component *p2 = NULL;
  struct ui_component *p3 = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_list_detail_pane_scaffold_create(NULL, &s);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_detail_pane_scaffold_create(engine, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_detail_pane_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_detail_pane_scaffold_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_detail_pane_scaffold_set_list_pane(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_detail_pane_scaffold_set_detail_pane(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_list_detail_pane_scaffold_set_extra_pane(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_list_detail_pane_scaffold_create(engine, &s);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(s != NULL);

  rc = md3_list_detail_pane_scaffold_get_base(s, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  /* Set panes */
  rc = ui_component_create(&p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&p3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_list_detail_pane_scaffold_set_list_pane(s, p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_detail_pane_scaffold_set_detail_pane(s, p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_list_detail_pane_scaffold_set_extra_pane(s, p3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Cleanup */
  rc = ui_component_destroy(p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(p3);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_list_detail_pane_scaffold_destroy(s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_supporting_pane_scaffold(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_supporting_pane_scaffold *s = NULL;
  struct ui_adaptive_pane_scaffold_base *base = NULL;
  struct ui_component *p1 = NULL;
  struct ui_component *p2 = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_supporting_pane_scaffold_create(NULL, &s);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_supporting_pane_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_supporting_pane_scaffold_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_supporting_pane_scaffold_set_main_pane(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_supporting_pane_scaffold_set_supporting_pane(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = md3_supporting_pane_scaffold_create(engine, &s);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(s != NULL);

  rc = md3_supporting_pane_scaffold_get_base(s, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = ui_component_create(&p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_create(&p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_supporting_pane_scaffold_set_main_pane(s, p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_supporting_pane_scaffold_set_supporting_pane(s, p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_destroy(p1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(p2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = md3_supporting_pane_scaffold_destroy(s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_md3_navigation_suite_scaffold(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_navigation_suite_scaffold *s = NULL;
  struct ui_component *comp = NULL;
  const char *nav_layout = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Null checks */
  rc = md3_navigation_suite_scaffold_create(NULL, &s);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = md3_navigation_suite_scaffold_set_window_width_class(
      NULL, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_scaffold_get_component(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = md3_navigation_suite_scaffold_get_component(s, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Valid creation */
  rc = md3_navigation_suite_scaffold_create(engine, &s);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(s != NULL);

  rc = md3_navigation_suite_scaffold_get_component(s, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);
  ASSERT(comp->shadow_root != NULL);

  /* Test adaptive chrome adaptation */
  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_get_attribute(comp->shadow_root, "data-nav-layout",
                                 &nav_layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("navigation-bar", nav_layout);

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_MEDIUM);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_get_attribute(comp->shadow_root, "data-nav-layout",
                                 &nav_layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("navigation-rail", nav_layout);

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_EXPANDED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_get_attribute(comp->shadow_root, "data-nav-layout",
                                 &nav_layout);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_STR_EQ("navigation-drawer", nav_layout);

  rc = md3_navigation_suite_scaffold_destroy(s);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

#ifdef UI_TEST_MOCK_ALLOC
TEST test_md3_adaptive_oom(void) {
  struct ui_engine_config engine_cfg;
  struct ui_engine *engine = NULL;
  struct md3_list_detail_pane_scaffold *s1 = NULL;
  struct md3_supporting_pane_scaffold *s2 = NULL;
  struct md3_navigation_suite_scaffold *s3 = NULL;
  ui_error_t rc;

  memset(&engine_cfg, 0, sizeof(engine_cfg));
  engine_cfg.num_threads = 1;
  rc = ui_engine_create(&engine_cfg, &engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_malloc_fail_countdown = 0;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(s1 == NULL);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(s2 == NULL);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = md3_navigation_suite_scaffold_create(engine, &s3);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT(s3 == NULL);
  g_malloc_fail_countdown = -1;

  rc = ui_engine_destroy(engine);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}
#endif

SUITE(md3_adaptive_suite) {
  RUN_TEST(test_md3_list_detail_pane_scaffold);
  RUN_TEST(test_md3_supporting_pane_scaffold);
  RUN_TEST(test_md3_navigation_suite_scaffold);
#ifdef UI_TEST_MOCK_ALLOC
  RUN_TEST(test_md3_adaptive_oom);
#endif
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_adaptive_suite);
  GREATEST_MAIN_END();
}
