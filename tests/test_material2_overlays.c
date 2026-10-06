/**
 * @file test_material2_overlays.c
 * @brief Unit tests for Material Design 2 Overlays and Feedback.
 */

/* clang-format off */
#include "material2/md2_overlays.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_timer.h"
#include "ui_overlay_director.h"
#include "ui_test_mock_mem.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_dialog(void) {
  struct md2_dialog *dialog = NULL;
  struct ui_dialog_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_dialog_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_dialog_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_dialog_create(&dialog);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_dialog_get_base(dialog, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  rc = md2_dialog_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  ASSERT_NEQ(NULL, dialog);

  rc = md2_dialog_get_base(dialog, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_dialog_destroy(dialog);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_bottom_sheet(void) {
  struct md2_bottom_sheet *sheet = NULL;
  struct ui_bottom_sheet_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_bottom_sheet_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_bottom_sheet_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_bottom_sheet_create(&sheet);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_bottom_sheet_get_base(sheet, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  rc = md2_bottom_sheet_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  ASSERT_NEQ(NULL, sheet);

  rc = md2_bottom_sheet_get_base(sheet, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_bottom_sheet_destroy(sheet);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_snackbar(void) {
  struct md2_snackbar *snackbar = NULL;
  struct ui_snackbar_base *base = NULL;
  struct ui_timer *timer = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t rc;

  rc = ui_timer_create_monotonic(&timer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  /* Invalid args */
  rc = md2_snackbar_create(timer, director, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_snackbar_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_snackbar_create(timer, director, &snackbar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_snackbar_get_base(snackbar, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  rc = md2_snackbar_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  ASSERT_NEQ(NULL, snackbar);

  rc = md2_snackbar_get_base(snackbar, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_snackbar_destroy(snackbar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);
  ui_timer_destroy(timer);

  PASS();
}

TEST test_md2_tabs(void) {
  struct md2_tabs *tabs = NULL;
  struct ui_tabs_base *base = NULL;
  ui_error_t rc;

  /* Invalid args */
  rc = md2_tabs_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_tabs_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  /* Success */
  rc = md2_tabs_create(&tabs);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = md2_tabs_get_base(tabs, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  rc = md2_tabs_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  ASSERT_NEQ(NULL, tabs);

  rc = md2_tabs_get_base(tabs, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_tabs_destroy(tabs);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_overlays_oom(void) {
  struct md2_dialog *dialog = NULL;
  struct md2_bottom_sheet *sheet = NULL;
  struct md2_snackbar *snackbar = NULL;
  struct md2_tabs *tabs = NULL;
  struct ui_timer *timer = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t rc;
  int i;

  rc = ui_timer_create_monotonic(&timer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_dialog_create(&dialog);
    if (rc == UI_ERROR_NONE) {
      md2_dialog_destroy(dialog);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_bottom_sheet_create(&sheet);
    if (rc == UI_ERROR_NONE) {
      md2_bottom_sheet_destroy(sheet);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_snackbar_create(timer, director, &snackbar);
    if (rc == UI_ERROR_NONE) {
      md2_snackbar_destroy(snackbar);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_tabs_create(&tabs);
    if (rc == UI_ERROR_NONE) {
      md2_tabs_destroy(tabs);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);
  ui_timer_destroy(timer);

  PASS();
}

SUITE(material2_overlays_suite) {
  RUN_TEST(test_md2_dialog);
  RUN_TEST(test_md2_bottom_sheet);
  RUN_TEST(test_md2_snackbar);
  RUN_TEST(test_md2_tabs);
  RUN_TEST(test_md2_overlays_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_overlays_suite);
  GREATEST_MAIN_END();
}
