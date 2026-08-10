/* clang-format off */
#include "../include/ui_web_bridge.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_web_bridge(void) {
  int failed = 0;

  if ((ui_web_bridge_create_node(1, "div") != UI_ERROR_NONE)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_set_text(1, "hello");
  ui_web_bridge_append_child(0, 1);
  ui_web_bridge_insert_before(0, 1, 0);
  ui_web_bridge_set_bounds(1, 0, 0, 100, 100);
  ui_web_bridge_set_style(1, "color", "red");
  ui_web_bridge_set_aria(1, "button", "label", 0, 0, -1, -1);
  ui_web_bridge_set_attribute(1, "id", "my-div");
  ui_web_bridge_set_property(1, "value", "text");

  /* Fill buffer to trigger flush in ensure_buffer */
  {
    int i;
    for (i = 0; i < 700; i++) {
      ui_web_bridge_create_node((ui_uint32)i, "span");
    }
  }

  ui_web_bridge_push_state("/path");
  ui_web_bridge_replace_state("/path");

  ui_web_bridge_remove_child(0, 1);
  ui_web_bridge_destroy_node(1);

  ui_web_bridge_flush();

  ui_web_bridge_dispatch_event(1, 0, 0, 1);
  ui_web_bridge_dispatch_event(2, 0, 0, 1);       /* MOUSE_UP */
  ui_web_bridge_dispatch_event(3, 0, 0, 0);       /* MOUSE_MOVE */
  ui_web_bridge_dispatch_event(5, 10.0, 20.0, 0); /* MOUSE_WHEEL */
  ui_web_bridge_dispatch_event(30, 1.0f, 0, 0);   /* DEEP_LINK */
  ui_web_bridge_dispatch_event(99, 0.0f, 0, 0);   /* Unknown type */
  ui_web_bridge_dispatch_event(0, 0.0f, 0, 0);    /* Zero type */

  ui_web_bridge_dispatch_resize(800, 600, 1.0f);
  ui_web_bridge_dispatch_key(10, "A", 0); /* KEY_DOWN */
  ui_web_bridge_dispatch_key(11, "A", 0); /* KEY_UP */

  /* Exceed buffer to hit flush branch */
  ui_web_bridge_set_bounds(1, 0, 0, 100, 100);
  ui_web_bridge_set_bounds(1, 0, 0, 100, 100);

  /* nulls */
  ui_web_bridge_create_node(2, NULL);
  ui_web_bridge_set_text(2, NULL);
  ui_web_bridge_set_style(2, NULL, NULL);
  ui_web_bridge_set_aria(2, NULL, NULL, 0, 0, -1, -1);
  ui_web_bridge_push_state(NULL);
  ui_web_bridge_replace_state(NULL);
  ui_web_bridge_set_attribute(2, NULL, NULL);
  ui_web_bridge_set_property(2, NULL, NULL);
  ui_web_bridge_dispatch_key(10, "A", 0);

  ui_web_bridge_shutdown();

#ifdef UI_TEST_MOCK_ALLOC
  /* Test OOM in ensure_buffer */
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_create_node(1, "div") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_destroy_node(1) != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_text(1, "x") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_append_child(0, 1) != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_remove_child(0, 1) != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_bounds(1, 0, 0, 0, 0) != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_insert_before(0, 1, 0) != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_style(1, "x", "y") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_aria(1, "x", "y", 0, 0, 0, 0) !=
       UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_push_state("x") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_replace_state("x") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_attribute(1, "x", "y") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  ui_web_bridge_shutdown();
  g_malloc_fail_countdown = 0;
  if ((ui_web_bridge_set_property(1, "x", "y") != UI_ERROR_OUT_OF_MEMORY)) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  g_malloc_fail_countdown = -1;
#endif

  return failed;
}

int main(void) {
  int failed = 0;
  if (test_web_bridge()) {
    printf("Failed at line %d\n", __LINE__);
    failed = 1;
  }
  if (!failed) {
    printf("test_ui_web_bridge passed\n");
  }
  return failed;
}
