/* clang-format off */
#include "ui_toast_manager_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_overlay_director.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)
#define ACCUM_FAIL(failed, expr) failed |= (expr)

static int run_extra_events(void);
static int run_edge_oom(void);
static int test_normal(void) {
  struct ui_toast_manager_base *tm = NULL;
  struct ui_toast_config cfg;
  ui_toast_id id1, id2;
  struct ui_event ev;
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  int failed = 0;

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node));
  ACCUM_ERR(failed, ui_overlay_director_create(root_node, &director));

  failed |= (ui_toast_manager_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ui_toast_manager_base_destroy(NULL);

  ACCUM_ERR(failed, ui_toast_manager_base_create(&tm));

  cfg.region = UI_TOAST_REGION_TOP_RIGHT;
  cfg.duration_secs = 2.0;
  cfg.message = "Hello World";
  cfg.is_error = 0;

  failed |= (ui_toast_manager_base_show(NULL, &cfg, 0.0, &id1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toast_manager_base_show(tm, NULL, 0.0, &id1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);

  cfg.region = UI_TOAST_REGION_COUNT;
  failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id1) !=
             UI_ERROR_INVALID_ARGUMENT);
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;

  ACCUM_ERR(failed, ui_toast_manager_base_show(tm, &cfg, 0.0, &id1));

  cfg.region = UI_TOAST_REGION_BOTTOM_LEFT;
  cfg.is_error = 1;
  ACCUM_ERR(failed, ui_toast_manager_base_show(tm, &cfg, 0.0, &id2));

  failed |=
      (ui_toast_manager_base_dismiss(NULL, id1) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toast_manager_base_dismiss(tm, 999) != UI_ERROR_NOT_FOUND);

  failed |=
      (ui_toast_manager_base_tick(NULL, 1.0) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_toast_manager_base_tick(tm, 0.1));

  failed |= (ui_toast_manager_base_handle_event(NULL, &ev, 0.1) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_toast_manager_base_handle_event(tm, NULL, 0.1) !=
             UI_ERROR_INVALID_ARGUMENT);

  ev.type = UI_EVENT_MOUSE_MOVE;
  ACCUM_ERR(failed, ui_toast_manager_base_handle_event(tm, &ev, 0.1));
  ACCUM_ERR(failed,
            ui_toast_manager_base_handle_event(tm, &ev, 0.2)); /* hover again */

  ev.type = UI_EVENT_MOUSE_UP;
  ACCUM_ERR(failed, ui_toast_manager_base_handle_event(tm, &ev, 0.5));
  ACCUM_ERR(failed,
            ui_toast_manager_base_handle_event(tm, &ev, 0.6)); /* leave again */

  failed |= (ui_toast_manager_base_render(NULL, director) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |=
      (ui_toast_manager_base_render(tm, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_toast_manager_base_render(tm, director));

  /* Render again to hit the active overlay unmount path */
  ACCUM_ERR(failed, ui_toast_manager_base_render(tm, director));

  /* Tick past 0.3s to enter visible state */
  ACCUM_ERR(failed, ui_toast_manager_base_tick(tm, 0.4));

  /* Tick past duration to auto-dismiss */
  ACCUM_ERR(failed, ui_toast_manager_base_tick(tm, 3.0));

  /* Tick again to cleanup */
  ACCUM_ERR(failed, ui_toast_manager_base_tick(tm, 3.1));

  /* Show again to dismiss manually */
  ACCUM_ERR(failed, ui_toast_manager_base_show(tm, &cfg, 3.1, &id1));
  ACCUM_ERR(failed, ui_toast_manager_base_dismiss(tm, id1));
  ACCUM_ERR(failed,
            ui_toast_manager_base_dismiss(tm, id1)); /* already dismissing */
  ACCUM_ERR(failed, ui_toast_manager_base_tick(tm, 3.2)); /* cleanup */

  /* Test regions for style branch coverage */
  int i;
  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    cfg.region = (enum ui_toast_region)i;
    ACCUM_ERR(failed, ui_toast_manager_base_show(tm, &cfg, 0.0, &id1));
  }
  ACCUM_ERR(failed, ui_toast_manager_base_render(tm, director));

  ui_toast_manager_base_destroy(tm);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root_node);
  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toast_manager_base *tm;
  struct ui_toast_config cfg;
  ui_toast_id id;
  int i;

  g_malloc_fail_countdown = 0;
  failed |= (ui_toast_manager_base_create(&tm) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  ui_toast_manager_base_create(&tm);

  cfg.region = UI_TOAST_REGION_TOP_RIGHT;
  cfg.duration_secs = 2.0;
  cfg.message = "Hello";
  cfg.is_error = 0;

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
    failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
    failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
  }
  g_malloc_fail_countdown = -1;

  /* Force stack reallocation OOM */
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id);
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id);
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id);
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id);

  /* Next add triggers realloc */
  g_malloc_fail_countdown =
      3; /* 0=entry, 1=message, 2=component, 3=realloc array */
  failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
  failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
  failed |= (ui_toast_manager_base_show(tm, &cfg, 0.0, &id) == UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  ui_toast_manager_base_destroy(tm);
#endif
  return failed;
}

static int run_extra_events(void) {
  struct ui_toast_manager_base *tm = NULL;
  struct ui_event ev;
  ui_toast_manager_base_create(&tm);

  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_toast_manager_base_handle_event(tm, &ev, 0.0);
  ev.type = UI_EVENT_KEY_DOWN;
  ui_toast_manager_base_handle_event(tm, &ev, 0.0);

  struct ui_toast_config cfg;
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;
  cfg.duration_secs = 2.0;
  cfg.message = "Hello";
  cfg.is_error = 0;
  ui_toast_id id1, id2;

  /* Add multiple toasts to same region to hit shift array path on dismiss */
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id1);
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id2);
  ui_toast_manager_base_tick(tm, 0.5);    /* make visible */
  ui_toast_manager_base_dismiss(tm, id1); /* marks first for slide out */
  ui_toast_manager_base_tick(tm, 0.6);    /* sweeps first out, shifts array */

  /* Create toast while paused */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ui_toast_manager_base_handle_event(tm, &ev, 1.0); /* hovered */
  ui_toast_manager_base_show(tm, &cfg, 1.1, &id1);
  ui_toast_manager_base_tick(tm, 1.5); /* make visible */
  ui_toast_manager_base_tick(tm, 4.0); /* duration check while paused */

  ui_toast_manager_base_destroy(tm);
  return 0;
}

static int run_edge_oom(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_toast_manager_base *tm;
  struct ui_toast_config cfg;
  ui_toast_id id;
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);

  ui_toast_manager_base_create(&tm);
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;
  cfg.duration_secs = 2.0;
  cfg.message = "Hello";
  cfg.is_error = 0;
  ui_toast_manager_base_show(tm, &cfg, 0.0, &id);

  /* Trigger OOM inside render: ui_dom_node_create(ELEMENT) */
  g_malloc_fail_countdown = 0;
  ui_toast_manager_base_render(tm, director);
  g_malloc_fail_countdown = -1;

  ui_toast_manager_base_destroy(tm);
  ui_overlay_director_destroy(director);
  ui_dom_node_destroy(root_node);
#endif
  return 0;
}

static int run_edge_unreachable(void) { return 0; }

int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();
  failed |= run_extra_events();
  failed |= run_edge_oom();
  failed |= run_edge_unreachable();

  if (!failed) {
    printf("All ui_toast_manager_base tests passed.\n");
  }
  return failed;
}
