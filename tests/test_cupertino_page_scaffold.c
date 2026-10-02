/**
 * @file test_cupertino_page_scaffold.c
 * @brief Unit tests for Cupertino Page Scaffold (CupertinoPageScaffold).
 */

/* clang-format off */
#include "greatest.h"
#include "cupertino/cupertino_page_scaffold.h"
#include "ui_test_mock_mem.h"
#include <string.h>
/* clang-format on */

SUITE(cupertino_page_scaffold_suite);

TEST test_page_scaffold_invalid_arguments(void) {
  struct cupertino_page_scaffold_descriptor desc;
  struct cupertino_page_scaffold *scaffold = NULL;
  struct ui_scaffold_base *base = NULL;
  struct ui_component *comp = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_scaffold_background bg_style;
  struct cupertino_safe_area_insets insets;
  float val1, val2;
  int flag;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  memset(&insets, 0, sizeof(insets));

  /* Creation invalid */
  rc = cupertino_page_scaffold_create(NULL, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_create(dummy_engine, NULL, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Out of range background_style */
  desc.background_style = (enum cupertino_scaffold_background)99;
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.background_style = (enum cupertino_scaffold_background) - 1;
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  desc.background_style = CUPERTINO_SCAFFOLD_BG_SYSTEM;

  /* Destruction invalid */
  rc = cupertino_page_scaffold_destroy(NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Setters / getters invalid */
  rc = cupertino_page_scaffold_set_nav_bar(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_nav_bar(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_nav_bar(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_body(NULL, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_body(NULL, &comp);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_body(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_background_style(
      NULL, CUPERTINO_SCAFFOLD_BG_SYSTEM);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_background_style(NULL, &bg_style);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_background_style(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_resize_to_avoid_bottom_inset(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_resize_to_avoid_bottom_inset(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_resize_to_avoid_bottom_inset(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_safe_area_insets(NULL, &insets);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_set_safe_area_insets(
      (struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_safe_area_insets(NULL, &insets);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_safe_area_insets(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_bottom_inset(NULL, 100.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_bottom_inset(NULL, &val1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_bottom_inset(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_get_effective_padding(NULL, &val1, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_effective_padding(
      (const struct cupertino_page_scaffold *)0x123, NULL, &val2);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_effective_padding(
      (const struct cupertino_page_scaffold *)0x123, &val1, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_set_dark_mode(NULL, 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_dark_mode(NULL, &flag);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_dark_mode(
      (const struct cupertino_page_scaffold *)0x123, NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  rc = cupertino_page_scaffold_get_base(NULL, &base);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_get_base((struct cupertino_page_scaffold *)0x123,
                                        NULL);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

TEST test_page_scaffold_lifecycle_and_insets(void) {
  struct cupertino_page_scaffold_descriptor desc;
  struct cupertino_page_scaffold *scaffold = NULL;
  struct ui_scaffold_base *base = NULL;
  struct ui_component *nav_bar = NULL;
  struct ui_component *body = NULL;
  struct ui_component *ret_comp = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  enum cupertino_scaffold_background bg_style;
  struct cupertino_safe_area_insets insets;
  float top_pad, bottom_pad;
  float b_inset;
  int is_dark, avoid;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.background_style = CUPERTINO_SCAFFOLD_BG_SYSTEM;
  desc.resize_to_avoid_bottom_inset = 1;
  desc.safe_area_insets.top = 47.0f;
  desc.safe_area_insets.bottom = 34.0f;
  desc.safe_area_insets.left = 0.0f;
  desc.safe_area_insets.right = 0.0f;
  desc.bottom_inset = 0.0f;
  desc.is_dark = 1;

  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  rc = cupertino_page_scaffold_get_base(scaffold, &base);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(base != NULL);

  rc = cupertino_page_scaffold_get_background_style(scaffold, &bg_style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCAFFOLD_BG_SYSTEM, bg_style);

  rc = cupertino_page_scaffold_get_resize_to_avoid_bottom_inset(scaffold,
                                                                &avoid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, avoid);

  rc = cupertino_page_scaffold_get_dark_mode(scaffold, &is_dark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_dark);

  rc = cupertino_page_scaffold_get_safe_area_insets(scaffold, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(47.0f, insets.top, "%f");
  ASSERT_EQ_FMT(34.0f, insets.bottom, "%f");

  /* Without nav_bar, top padding = 47.0f, bottom = 34.0f */
  rc = cupertino_page_scaffold_get_effective_padding(scaffold, &top_pad,
                                                     &bottom_pad);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(47.0f, top_pad, "%f");
  ASSERT_EQ_FMT(34.0f, bottom_pad, "%f");

  /* Attach dummy nav bar and body components */
  rc = ui_component_create(&nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nav_bar->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_create(&body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &body->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = cupertino_page_scaffold_set_nav_bar(scaffold, nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_nav_bar(scaffold, &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(nav_bar, ret_comp);

  rc = cupertino_page_scaffold_set_body(scaffold, body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_body(scaffold, &ret_comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(body, ret_comp);

  /* With nav_bar (+44.0f), effective top padding = 47.0f + 44.0f = 91.0f */
  rc = cupertino_page_scaffold_get_effective_padding(scaffold, &top_pad,
                                                     &bottom_pad);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(91.0f, top_pad, "%f");
  ASSERT_EQ_FMT(34.0f, bottom_pad, "%f");

  /* Simulate active keyboard bottom inset (e.g. 291.0f) */
  rc = cupertino_page_scaffold_set_bottom_inset(scaffold, 291.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_bottom_inset(scaffold, &b_inset);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(291.0f, b_inset, "%f");

  rc = cupertino_page_scaffold_get_effective_padding(scaffold, &top_pad,
                                                     &bottom_pad);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(291.0f, bottom_pad, "%f");

  /* Test when resize_to_avoid_bottom_inset is disabled */
  rc = cupertino_page_scaffold_set_resize_to_avoid_bottom_inset(scaffold, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_effective_padding(scaffold, &top_pad,
                                                     &bottom_pad);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(34.0f, bottom_pad, "%f");

  /* Test setting safe_area_insets successfully */
  insets.top = 20.0f;
  insets.bottom = 15.0f;
  rc = cupertino_page_scaffold_set_safe_area_insets(scaffold, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_safe_area_insets(scaffold, &insets);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(20.0f, insets.top, "%f");

  /* Test setting NULL nav_bar and body (or component with NULL shadow_root) */
  rc = cupertino_page_scaffold_set_nav_bar(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_set_body(scaffold, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  nav_bar->shadow_root = NULL;
  rc = cupertino_page_scaffold_set_nav_bar(scaffold, nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  body->shadow_root = NULL;
  rc = cupertino_page_scaffold_set_body(scaffold, body);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test scaffold->base == NULL paths */
  scaffold->base = NULL;
  rc = cupertino_page_scaffold_set_nav_bar(scaffold, nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_set_body(scaffold, body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Test create with negative bottom_inset */
  memset(&desc, 0, sizeof(desc));
  desc.bottom_inset = -5.0f;
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ_FMT(0.0f, scaffold->bottom_inset, "%f");

  /* Toggle background style with negative and valid */
  rc = cupertino_page_scaffold_set_background_style(
      scaffold, (enum cupertino_scaffold_background) - 1);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_set_background_style(
      scaffold, CUPERTINO_SCAFFOLD_BG_GROUPED);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_background_style(scaffold, &bg_style);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(CUPERTINO_SCAFFOLD_BG_GROUPED, bg_style);

  /* Toggle dark mode */
  rc = cupertino_page_scaffold_set_dark_mode(scaffold, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = cupertino_page_scaffold_get_dark_mode(scaffold, &is_dark);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_dark);

  /* Test invalid bounds */
  rc = cupertino_page_scaffold_set_bottom_inset(scaffold, -10.0f);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  rc = cupertino_page_scaffold_set_background_style(
      scaffold, (enum cupertino_scaffold_background)99);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);

  /* Clean up components and scaffold */
  rc = cupertino_page_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  nav_bar->shadow_root = NULL;
  rc = ui_component_destroy(nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  body->shadow_root = NULL;
  rc = ui_component_destroy(body);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_page_scaffold_oom_simulation(void) {
#ifdef UI_TEST_MOCK_ALLOC
  extern int g_cupertino_page_scaffold_mock_create_fail;
  extern int g_cupertino_page_scaffold_mock_set_top_bar_fail;
  extern int g_cupertino_page_scaffold_mock_set_main_fail;
  extern int g_cupertino_page_scaffold_mock_destroy_fail;
#endif
  struct cupertino_page_scaffold_descriptor desc;
  struct cupertino_page_scaffold *scaffold = NULL;
  struct ui_component *nav_bar = NULL;
  struct ui_component *body = NULL;
  struct ui_engine *dummy_engine = (struct ui_engine *)0x1234;
  ui_error_t rc;

  memset(&desc, 0, sizeof(desc));
  desc.background_style = CUPERTINO_SCAFFOLD_BG_SYSTEM;

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail malloc for scaffold */
  g_malloc_fail_countdown = 0;
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, scaffold);
  g_malloc_fail_countdown = -1;

  /* Fail ui_scaffold_base_create */
  g_cupertino_page_scaffold_mock_create_fail = 1;
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  ASSERT_EQ(NULL, scaffold);
  g_cupertino_page_scaffold_mock_create_fail = 0;

  /* Create valid scaffold to test setter errors and destroy failure */
  rc = cupertino_page_scaffold_create(dummy_engine, &desc, &scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(scaffold != NULL);

  rc = ui_component_create(&nav_bar);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &nav_bar->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_component_create(&body);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &body->shadow_root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Fail set_top_bar */
  g_cupertino_page_scaffold_mock_set_top_bar_fail = 1;
  rc = cupertino_page_scaffold_set_nav_bar(scaffold, nav_bar);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_page_scaffold_mock_set_top_bar_fail = 0;

  /* Fail set_main_content */
  g_cupertino_page_scaffold_mock_set_main_fail = 1;
  rc = cupertino_page_scaffold_set_body(scaffold, body);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
  g_cupertino_page_scaffold_mock_set_main_fail = 0;

  /* Fail destroy */
  g_cupertino_page_scaffold_mock_destroy_fail = 1;
  rc = cupertino_page_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_cupertino_page_scaffold_mock_destroy_fail = 0;

  rc = cupertino_page_scaffold_destroy(scaffold);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  nav_bar->shadow_root = NULL;
  ui_component_destroy(nav_bar);
  body->shadow_root = NULL;
  ui_component_destroy(body);
#else
  rc = cupertino_page_scaffold_create(dummy_engine, NULL, &scaffold);
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, rc);
#endif

  PASS();
}

SUITE(cupertino_page_scaffold_suite) {
  RUN_TEST(test_page_scaffold_invalid_arguments);
  RUN_TEST(test_page_scaffold_lifecycle_and_insets);
  RUN_TEST(test_page_scaffold_oom_simulation);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cupertino_page_scaffold_suite);
  GREATEST_MAIN_END();
}
