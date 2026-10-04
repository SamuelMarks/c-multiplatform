/**
 * @file test_material2_navigation_drawer.c
 * @brief Unit tests for Material Design 2 Navigation Drawer component.
 */

/* clang-format off */
#include "material2/md2_navigation_drawer.h"
#include "ui_error.h"
#include <greatest.h>
/* clang-format on */

static ui_error_t mock_on_close(struct md2_navigation_drawer *drawer,
                                void *user_data) {
  int *called = (int *)user_data;
  (void)drawer;
  if (called) {
    *called = 1;
  }
  return UI_ERROR_NONE;
}

TEST test_md2_navigation_drawer_create_destroy(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, drawer);

  rc = md2_navigation_drawer_destroy(drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  PASS();
}

TEST test_md2_navigation_drawer_invalid_args(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;
  int is_open;
  struct ui_component *comp = NULL;

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_destroy(NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_set_drawer_content(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_set_main_content(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_set_open(NULL, 1);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_is_open(NULL, &is_open);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_is_open(drawer, NULL);
  /* The underlying base might return INVALID_ARGUMENT if out_is_open is NULL */
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_set_overlay_director(NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_set_on_close(NULL, mock_on_close, NULL);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = md2_navigation_drawer_get_component(NULL, &comp);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");

  md2_navigation_drawer_destroy(drawer);
  PASS();
}

TEST test_md2_navigation_drawer_open_close(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;
  int is_open = -1;

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_MODAL, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_is_open(drawer, &is_open);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(0, is_open, "%d");

  rc = md2_navigation_drawer_set_open(drawer, 1);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_is_open(drawer, &is_open);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(1, is_open, "%d");

  md2_navigation_drawer_destroy(drawer);
  PASS();
}

TEST test_md2_navigation_drawer_get_component(void) {
  struct md2_navigation_drawer *drawer = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_get_component(drawer, &comp);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_NEQ(NULL, comp);

  md2_navigation_drawer_destroy(drawer);
  PASS();
}

SUITE(material2_navigation_drawer_suite) {
  RUN_TEST(test_md2_navigation_drawer_create_destroy);
  RUN_TEST(test_md2_navigation_drawer_invalid_args);
  RUN_TEST(test_md2_navigation_drawer_open_close);
  RUN_TEST(test_md2_navigation_drawer_get_component);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_navigation_drawer_suite);
  GREATEST_MAIN_END();
}
