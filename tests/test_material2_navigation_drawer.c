/**
 * @file test_material2_navigation_drawer.c
 * @brief Unit tests for Material Design 2 Navigation Drawer component.
 */

/* clang-format off */
#include "material2/md2_navigation_drawer.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_sidenav_base.h"
#include "ui_test_mock_mem.h"
#include "ui_event.h"
#include <greatest.h>
/* clang-format on */

struct md2_navigation_drawer {
  struct ui_sidenav_base *base;
  enum md2_navigation_drawer_type type;
  md2_navigation_drawer_on_close_t on_close;
  void *user_data;
};

static ui_error_t mock_on_close(struct md2_navigation_drawer *drawer,
                                void *user_data) {
  int *called = (int *)user_data;
  if (drawer == NULL)
    return UI_ERROR_INVALID_ARGUMENT;
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

TEST test_md2_navigation_drawer_setters(void) {
  struct md2_navigation_drawer *drawer = NULL;
  struct ui_component *content = NULL;
  struct ui_component *main_content = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t rc;

  rc = ui_component_create(&content);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_component_create(&main_content);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_drawer_content(drawer, content);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_main_content(drawer, main_content);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_overlay_director(drawer, director);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md2_navigation_drawer_destroy(drawer);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root);
  PASS();
}

TEST test_md2_navigation_drawer_on_close(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;
  int called = 0;
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));

  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_MODAL, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_on_close(drawer, mock_on_close, &called);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_open(drawer, 1);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;

  rc = ui_sidenav_base_process_event(drawer->base, &ev, 100.0);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  ASSERT_EQ_FMT(1, called, "%d");

  /* Call when drawer->on_close == NULL to hit branch */
  rc = md2_navigation_drawer_set_on_close(drawer, NULL, NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = ui_sidenav_base_process_event(drawer->base, &ev, 100.0);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  rc = md2_navigation_drawer_set_on_close(drawer, mock_on_close, NULL);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");
  rc = ui_sidenav_base_process_event(drawer->base, &ev, 100.0);
  ASSERT_EQ_FMT(UI_ERROR_NONE, rc, "%d");

  md2_navigation_drawer_destroy(drawer);
  PASS();
}

TEST test_md2_navigation_drawer_oom(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
    if (rc == UI_ERROR_NONE) {
      md2_navigation_drawer_destroy(drawer);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  for (i = 0; i < 500; ++i) {
    g_malloc_fail_countdown = i;
    rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_MODAL, &drawer);
    if (rc == UI_ERROR_NONE) {
      md2_navigation_drawer_destroy(drawer);
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  PASS();
}

extern int g_md2_nav_mock_set_on_close_fail;

TEST test_md2_navigation_drawer_mock_fail(void) {
  struct md2_navigation_drawer *drawer = NULL;
  ui_error_t rc;

  g_md2_nav_mock_set_on_close_fail = 1;
  rc = md2_navigation_drawer_create(MD2_NAVIGATION_DRAWER_STANDARD, &drawer);
  ASSERT_EQ_FMT(UI_ERROR_INVALID_ARGUMENT, rc, "%d");
  g_md2_nav_mock_set_on_close_fail = 0;

  PASS();
}

SUITE(material2_navigation_drawer_suite) {
  RUN_TEST(test_md2_navigation_drawer_create_destroy);
  RUN_TEST(test_md2_navigation_drawer_invalid_args);
  RUN_TEST(test_md2_navigation_drawer_open_close);
  RUN_TEST(test_md2_navigation_drawer_get_component);
  RUN_TEST(test_md2_navigation_drawer_setters);
  RUN_TEST(test_md2_navigation_drawer_on_close);
  RUN_TEST(test_md2_navigation_drawer_oom);
  RUN_TEST(test_md2_navigation_drawer_mock_fail);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(material2_navigation_drawer_suite);
  GREATEST_MAIN_END();
}
