/**
 * @file test_material2_overlays.c
 * @brief Unit tests for Material Design 2 Overlays and Feedback.
 */

/* clang-format off */
#include "material2/md2_overlays.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

TEST test_md2_dialog_lifecycle(void) {
  struct md2_dialog *dialog = NULL;
  struct ui_dialog_base *base = NULL;
  ui_error_t rc;

  rc = md2_dialog_create(&dialog);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, dialog);

  rc = md2_dialog_get_base(dialog, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_dialog_destroy(dialog);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_dialog_invalid_args(void) {
  struct md2_dialog *dialog = NULL;
  struct ui_dialog_base *base = NULL;
  ui_error_t rc;

  rc = md2_dialog_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_dialog_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_dialog_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_dialog_create(&dialog);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_dialog_get_base(dialog, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_dialog_destroy(dialog);
  PASS();
}

TEST test_md2_bottom_sheet_lifecycle(void) {
  struct md2_bottom_sheet *sheet = NULL;
  struct ui_bottom_sheet_base *base = NULL;
  ui_error_t rc;

  rc = md2_bottom_sheet_create(&sheet);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, sheet);

  rc = md2_bottom_sheet_get_base(sheet, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_bottom_sheet_destroy(sheet);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_bottom_sheet_invalid_args(void) {
  struct md2_bottom_sheet *sheet = NULL;
  struct ui_bottom_sheet_base *base = NULL;
  ui_error_t rc;

  rc = md2_bottom_sheet_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_bottom_sheet_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_bottom_sheet_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_bottom_sheet_create(&sheet);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_bottom_sheet_get_base(sheet, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_bottom_sheet_destroy(sheet);
  PASS();
}

#include "ui_engine.h"
#include "ui_overlay_director.h"
#include "ui_timer.h"
#include <stdlib.h>

/* Helper to create dummy timer and director */
static ui_error_t create_dummies(struct ui_engine **engine,
                                 struct ui_timer **timer,
                                 struct ui_overlay_director **director) {
  ui_error_t rc;
  struct ui_engine_config config = {0};

  rc = ui_engine_create(&config, engine);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* In a real app we would use ui_timer_create and ui_overlay_director_create.
     For this test we just allocate dummies since base might only check for
     NULL. */
  *timer = (struct ui_timer *)malloc(1);
  *director = (struct ui_overlay_director *)malloc(1);
  return UI_ERROR_NONE;
}

static void destroy_dummies(struct ui_engine *engine, struct ui_timer *timer,
                            struct ui_overlay_director *director) {
  free(timer);
  free(director);
  ui_engine_destroy(engine);
}

TEST test_md2_snackbar_lifecycle(void) {
  struct md2_snackbar *snackbar = NULL;
  struct ui_snackbar_base *base = NULL;
  struct ui_engine *engine = NULL;
  struct ui_timer *timer = NULL;
  struct ui_overlay_director *director = NULL;
  ui_error_t rc;

  rc = create_dummies(&engine, &timer, &director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_snackbar_create(timer, director, &snackbar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, snackbar);

  rc = md2_snackbar_get_base(snackbar, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_snackbar_destroy(snackbar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  destroy_dummies(engine, timer, director);
  PASS();
}

TEST test_md2_snackbar_invalid_args(void) {
  struct md2_snackbar *snackbar = NULL;
  struct ui_snackbar_base *base = NULL;
  struct ui_engine *engine = NULL;
  struct ui_timer *timer = NULL;
  struct ui_overlay_director *director = NULL;
  ui_error_t rc;

  rc = create_dummies(&engine, &timer, &director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_snackbar_create(NULL, NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_snackbar_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_snackbar_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_snackbar_create(timer, director, &snackbar);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_snackbar_get_base(snackbar, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_snackbar_destroy(snackbar);
  destroy_dummies(engine, timer, director);
  PASS();
}

TEST test_md2_tabs_lifecycle(void) {
  struct md2_tabs *tabs = NULL;
  struct ui_tabs_base *base = NULL;
  ui_error_t rc;

  rc = md2_tabs_create(&tabs);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, tabs);

  rc = md2_tabs_get_base(tabs, &base);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, base);

  rc = md2_tabs_destroy(tabs);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_tabs_invalid_args(void) {
  struct md2_tabs *tabs = NULL;
  struct ui_tabs_base *base = NULL;
  ui_error_t rc;

  rc = md2_tabs_create(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_tabs_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_tabs_get_base(NULL, &base);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_tabs_create(&tabs);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_tabs_get_base(tabs, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_tabs_destroy(tabs);
  PASS();
}

SUITE(material2_overlays_suite) {
  RUN_TEST(test_md2_dialog_lifecycle);
  RUN_TEST(test_md2_dialog_invalid_args);
  RUN_TEST(test_md2_bottom_sheet_lifecycle);
  RUN_TEST(test_md2_bottom_sheet_invalid_args);
  RUN_TEST(test_md2_snackbar_lifecycle);
  RUN_TEST(test_md2_snackbar_invalid_args);
  RUN_TEST(test_md2_tabs_lifecycle);
  RUN_TEST(test_md2_tabs_invalid_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_overlays_suite);
  GREATEST_MAIN_END();
}
