/* clang-format off */
#include "ui_toast_manager_base.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_overlay_director.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != UI_ERROR_NONE) {                                               \
      printf("Failed at line %d: %d\n", __LINE__, _err);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != (expected)) {                                                  \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             _err);                                                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_ui_toast_manager_base_create_destroy(void) {
  struct ui_toast_manager_base *manager = NULL;

  ASSERT_EQ(ui_toast_manager_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));
  if (!manager)
    return 1;

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_ui_toast_manager_base_show_dismiss(void) {
  struct ui_toast_manager_base *manager = NULL;
  struct ui_toast_config config;
  ui_toast_id id1, id2;

  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));

  config.region = UI_TOAST_REGION_TOP_RIGHT;
  config.duration_secs = 5.0;
  config.message = "Test message";
  config.is_error = 0;

  ASSERT_EQ(ui_toast_manager_base_show(NULL, &config, 0.0, &id1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_toast_manager_base_show(manager, NULL, 0.0, &id1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_toast_manager_base_show(manager, &config, 0.0, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  config.region = UI_TOAST_REGION_COUNT;
  ASSERT_EQ(ui_toast_manager_base_show(manager, &config, 0.0, &id1),
            UI_ERROR_INVALID_ARGUMENT);

  config.region = UI_TOAST_REGION_TOP_RIGHT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id2));

  ASSERT_EQ(ui_toast_manager_base_dismiss(NULL, id1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_toast_manager_base_dismiss(manager, 999), UI_ERROR_NOT_FOUND);
  ASSERT_SUCCESS(ui_toast_manager_base_dismiss(manager, id1));
  ASSERT_SUCCESS(ui_toast_manager_base_dismiss(
      manager, id1)); /* Dismissing again is valid if it's animating */

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_ui_toast_manager_base_tick(void) {
  struct ui_toast_manager_base *manager = NULL;
  struct ui_toast_config config;
  ui_toast_id id1, id2, id3;

  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));

  config.region = UI_TOAST_REGION_BOTTOM_LEFT;
  config.duration_secs = 2.0;
  config.message = "Toast 1";
  config.is_error = 0;

  ASSERT_EQ(ui_toast_manager_base_tick(NULL, 0.0), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));

  config.duration_secs = 0.0; /* infinite */
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id2));

  /* Wait to cover SLIDE_IN animation progression exactly at boundary */
  ASSERT_SUCCESS(ui_toast_manager_base_tick(manager, 0.29));
  ASSERT_SUCCESS(
      ui_toast_manager_base_tick(manager, 0.31)); /* Becomes visible */

  /* Trigger auto dismiss directly after visible */
  ASSERT_SUCCESS(
      ui_toast_manager_base_tick(manager, 0.31 + 2.0)); /* Triggers SLIDE_OUT */
  ASSERT_SUCCESS(
      ui_toast_manager_base_tick(manager, 3.0)); /* Cleans up slide out */

  /* Dismiss infinite toast manually */
  ASSERT_SUCCESS(ui_toast_manager_base_dismiss(manager, id2));
  ASSERT_SUCCESS(
      ui_toast_manager_base_tick(manager, 3.5)); /* Slide out starts */
  ASSERT_SUCCESS(ui_toast_manager_base_tick(manager, 4.0)); /* Removed */

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_ui_toast_manager_base_events(void) {
  struct ui_toast_manager_base *manager = NULL;
  struct ui_toast_config config;
  ui_toast_id id1;
  struct ui_event ev;

  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));

  config.region = UI_TOAST_REGION_TOP_CENTER;
  config.duration_secs = 1.0;
  config.message = "Hover test";
  config.is_error = 0;

  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));

  /* Second show to hit capacity branch once initialized */
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));

  ASSERT_EQ(ui_toast_manager_base_handle_event(NULL, &ev, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_toast_manager_base_handle_event(manager, NULL, 0.0),
            UI_ERROR_INVALID_ARGUMENT);

  /* Hover starts */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 0.5));
  /* Duplicate hover to hit false branch of !manager->is_hovered */
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 0.6));

  /* Hover starts when already paused by newly added toast during hover state */
  config.region = UI_TOAST_REGION_TOP_CENTER;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.5, &id1));

  /* Force hover state logic again on same toast */
  ev.type = UI_EVENT_MOUSE_UP;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.5));
  /* Duplicate unhover to hit false branch of manager->is_hovered */
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.6));

  ev.type = UI_EVENT_TOUCH_START;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.6));

  /* Add new toast while hovered */
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 2.6, &id1));

  ASSERT_SUCCESS(ui_toast_manager_base_tick(
      manager, 2.0)); /* Shouldn't dismiss because paused */

  /* Hover ends */
  ev.type = UI_EVENT_MOUSE_UP;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.5));
  /* Duplicate unhover */
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.6));

  /* Test touch end/cancel */
  ev.type = UI_EVENT_TOUCH_START;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.7));
  ev.type = UI_EVENT_TOUCH_END;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.8));

  ev.type = UI_EVENT_TOUCH_START;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.9));
  ev.type = UI_EVENT_TOUCH_CANCEL;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 3.0));

  /* Unknown event */
  ev.type = UI_EVENT_WINDOW_RESIZE;
  ASSERT_SUCCESS(ui_toast_manager_base_handle_event(manager, &ev, 2.6));

  ASSERT_SUCCESS(
      ui_toast_manager_base_tick(manager, 4.0)); /* Now they dismiss */
  ASSERT_SUCCESS(ui_toast_manager_base_tick(manager, 4.5)); /* Cleaned up */

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_ui_toast_manager_base_render(void) {
  struct ui_toast_manager_base *manager = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_toast_config config;
  ui_toast_id id1;

  struct ui_dom_node *root_node = NULL;

  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node));
  ASSERT_SUCCESS(ui_overlay_director_create(root_node, &director));
  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));

  ASSERT_EQ(ui_toast_manager_base_render(NULL, director),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_toast_manager_base_render(manager, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  /* Empty render */
  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));

  config.region = UI_TOAST_REGION_BOTTOM_RIGHT;
  config.duration_secs = 2.0;
  config.message = "Render me";
  config.is_error = 1;

  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));

  /* Render again to trigger the active_overlay unmount/remount logic */
  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));

  /* Test all regions */
  config.region = (enum ui_toast_region)999; /* Tests default case which is
                                                TOP_LEFT logic now */
  ASSERT_EQ(ui_toast_manager_base_show(manager, &config, 0.0, &id1),
            UI_ERROR_INVALID_ARGUMENT);

  config.region = UI_TOAST_REGION_TOP_LEFT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  config.region = UI_TOAST_REGION_TOP_CENTER;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  config.region = UI_TOAST_REGION_TOP_RIGHT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  config.region = UI_TOAST_REGION_BOTTOM_LEFT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  config.region = UI_TOAST_REGION_BOTTOM_CENTER;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  config.region = UI_TOAST_REGION_BOTTOM_RIGHT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));

  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));

#ifdef UI_TEST_MOCK_ALLOC
  /* Fail text_content MALLOC during render */
  g_malloc_fail_countdown = 0;
  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));
  g_malloc_fail_countdown = -1;
#endif

  /* Test NULL message in show */
  config.message = NULL;
  config.region = UI_TOAST_REGION_TOP_LEFT;
  ASSERT_SUCCESS(ui_toast_manager_base_show(manager, &config, 0.0, &id1));
  ASSERT_SUCCESS(ui_toast_manager_base_render(manager, director));

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_ui_toast_manager_base_allocation_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toast_manager_base *manager = NULL;
  struct ui_toast_config config;
  ui_toast_id id1;
  int i;
  ui_error_t err;

  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_toast_manager_base_create(&manager), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ASSERT_SUCCESS(ui_toast_manager_base_create(&manager));

  config.region = UI_TOAST_REGION_BOTTOM_RIGHT;
  config.duration_secs = 2.0;
  config.message = "Failing message";
  config.is_error = 0;

  for (i = 0; i < 5; ++i) {
    g_malloc_fail_countdown = i;
    err = ui_toast_manager_base_show(manager, &config, 0.0, &id1);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      break;
    }
    ASSERT_EQ(err, UI_ERROR_OUT_OF_MEMORY);
  }

  {
    ui_error_t rc_cleanup = ui_toast_manager_base_destroy(manager);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif
  return 0;
}

int main(void) {
  if (test_ui_toast_manager_base_create_destroy())
    return 1;
  if (test_ui_toast_manager_base_show_dismiss())
    return 1;
  if (test_ui_toast_manager_base_tick())
    return 1;
  if (test_ui_toast_manager_base_events())
    return 1;
  if (test_ui_toast_manager_base_render())
    return 1;
  if (test_ui_toast_manager_base_allocation_failures())
    return 1;
  return 0;
}
