/**
 * @file test_material3_adaptive.c
 * @brief Unit tests for Material Design 3 Adaptive Scaffolds.
 */

/* clang-format off */
#include "material3/md3_adaptive.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_engine.h"

#include "ui_test_mock_mem.h"
#include "ui_dom_node.h"

struct md3_navigation_suite_scaffold {
  struct ui_component *component;
  struct ui_dom_node *root_node;
  enum ui_adaptive_window_width_class width_class;
};

#include "ui_dom_node.h"
#include <greatest.h>
/* clang-format on */

TEST test_md3_list_detail_pane_scaffold(void) {
  struct md3_list_detail_pane_scaffold *s = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_adaptive_pane_scaffold_base *base = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Invalid args */
  rc = md3_list_detail_pane_scaffold_create(NULL, &s);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_create(engine, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_list_detail_pane_scaffold_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_get_base(s, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_list_pane(NULL, comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_detail_pane(NULL, comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_extra_pane(NULL, comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md3_list_detail_pane_scaffold_create(engine, &s);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, s);

  rc = md3_list_detail_pane_scaffold_get_base(s, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_list_detail_pane_scaffold_get_base(s, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = ui_component_create(&comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_list_pane(s, comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_detail_pane(s, comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_list_detail_pane_scaffold_set_extra_pane(s, comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md3_list_detail_pane_scaffold_destroy(s);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md3_supporting_pane_scaffold(void) {
  struct md3_supporting_pane_scaffold *s = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_adaptive_pane_scaffold_base *base = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Invalid args */
  rc = md3_supporting_pane_scaffold_create(NULL, &s);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_supporting_pane_scaffold_create(engine, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_supporting_pane_scaffold_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_supporting_pane_scaffold_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_supporting_pane_scaffold_set_main_pane(NULL, comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_supporting_pane_scaffold_set_supporting_pane(NULL, comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md3_supporting_pane_scaffold_create(engine, &s);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, s);

  rc = md3_supporting_pane_scaffold_get_base(s, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_supporting_pane_scaffold_get_base(s, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = ui_component_create(&comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_supporting_pane_scaffold_set_main_pane(s, comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_supporting_pane_scaffold_set_supporting_pane(s, comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md3_supporting_pane_scaffold_destroy(s);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md3_navigation_suite_scaffold(void) {
  struct md3_navigation_suite_scaffold *s = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Invalid args */
  rc = md3_navigation_suite_scaffold_create(NULL, &s);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_navigation_suite_scaffold_create(engine, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_navigation_suite_scaffold_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_navigation_suite_scaffold_get_component(NULL, &comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      NULL, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md3_navigation_suite_scaffold_create(engine, &s);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, s);

  rc = md3_navigation_suite_scaffold_get_component(s, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md3_navigation_suite_scaffold_get_component(s, &comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, comp);

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_COMPACT);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_MEDIUM);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, UI_WINDOW_WIDTH_EXPANDED);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md3_navigation_suite_scaffold_set_window_width_class(
      s, (enum ui_adaptive_window_width_class)999);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md3_navigation_suite_scaffold_destroy(s);
  ui_engine_destroy(engine);

  PASS();
}

TEST test_md3_adaptive_oom(void) {
  struct md3_list_detail_pane_scaffold *s1 = NULL;
  struct md3_supporting_pane_scaffold *s2 = NULL;
  struct md3_navigation_suite_scaffold *s3 = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;
  int i;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md3_list_detail_pane_scaffold_create(engine, &s1);
    if (rc == UI_ERROR_NONE) {
      md3_list_detail_pane_scaffold_destroy(s1);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md3_supporting_pane_scaffold_create(engine, &s2);
    if (rc == UI_ERROR_NONE) {
      md3_supporting_pane_scaffold_destroy(s2);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md3_navigation_suite_scaffold_create(engine, &s3);
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 50; ++i) {
    g_malloc_fail_countdown = i;
    rc = md3_navigation_suite_scaffold_set_window_width_class(
        s3, UI_WINDOW_WIDTH_COMPACT);
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }
  g_malloc_fail_countdown = -1;
  md3_navigation_suite_scaffold_destroy(s3);

  ui_engine_destroy(engine);
  PASS();
}

int g_md3_adaptive_mock_comp_null = 0;
TEST test_md3_adaptive_mock_fail_branches(void) {
  struct md3_list_detail_pane_scaffold *s1 = NULL;
  struct md3_supporting_pane_scaffold *s2 = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);

  /* Cause rc != UI_ERROR_NONE inside md3_adaptive */
  g_md3_adaptive_mock_comp_null = 1;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  md3_list_detail_pane_scaffold_destroy(s1);

  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  md3_supporting_pane_scaffold_destroy(s2);

  /* Cause comp == NULL */
  g_md3_adaptive_mock_comp_null = 2;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  md3_list_detail_pane_scaffold_destroy(s1);

  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  md3_supporting_pane_scaffold_destroy(s2);

  /* Cause comp->shadow_root == NULL */
  g_md3_adaptive_mock_comp_null = 3;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  md3_list_detail_pane_scaffold_destroy(s1);

  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  md3_supporting_pane_scaffold_destroy(s2);

  g_md3_adaptive_mock_comp_null = 0;

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_adaptive_destroy_null_comp(void) {
  /* To hit `if (scaffold->component != NULL)` branch taken 0 (i.e.
     scaffold->component == NULL in md3_navigation_suite_scaffold_destroy), we
     need a scaffold with NULL component. We can mock malloc for just the
     component inside create to fail, but it aborts create and cleans up
     properly. Let's manually allocate one, set component=NULL and destroy. */
  struct md3_navigation_suite_scaffold *s3;
  s3 = ui_mock_malloc(sizeof(*s3));
  memset(s3, 0, sizeof(*s3));

  md3_navigation_suite_scaffold_destroy(s3);
  PASS();
}

TEST test_md3_adaptive_mock_rc(void) {
  struct md3_list_detail_pane_scaffold *s1 = NULL;
  struct md3_supporting_pane_scaffold *s2 = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);

  g_md3_adaptive_mock_comp_null = 1;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  g_md3_adaptive_mock_comp_null = 0;

  ui_engine_destroy(engine);
  PASS();
}

TEST test_md3_adaptive_mock_comp_null_branch(void) {
  struct md3_list_detail_pane_scaffold *s1 = NULL;
  struct md3_supporting_pane_scaffold *s2 = NULL;
  struct ui_engine *engine = NULL;
  struct ui_engine_config config;
  ui_error_t rc;

  memset(&config, 0, sizeof(config));
  rc = ui_engine_create(&config, &engine);

  g_md3_adaptive_mock_comp_null = 4;
  rc = md3_list_detail_pane_scaffold_create(engine, &s1);
  rc = md3_supporting_pane_scaffold_create(engine, &s2);
  g_md3_adaptive_mock_comp_null = 0;

  ui_engine_destroy(engine);
  PASS();
}

SUITE(material3_adaptive_suite) {
  RUN_TEST(test_md3_list_detail_pane_scaffold);
  RUN_TEST(test_md3_supporting_pane_scaffold);
  RUN_TEST(test_md3_navigation_suite_scaffold);
  RUN_TEST(test_md3_adaptive_oom);
  RUN_TEST(test_md3_adaptive_mock_fail_branches);
  RUN_TEST(test_md3_adaptive_destroy_null_comp);
  RUN_TEST(test_md3_adaptive_mock_rc);
  RUN_TEST(test_md3_adaptive_mock_comp_null_branch);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material3_adaptive_suite);
  GREATEST_MAIN_END();
}
