/* clang-format off */
#include "greatest.h"
#include "ui_toast_manager_base.h"
#include "ui_error.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_overlay_director.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_toast_entry {
  ui_toast_id id;
  struct ui_toast_config config;
  enum ui_toast_anim_state anim_state;
  double show_time;
  double total_paused_time;
  double pause_start_time;
  int is_paused;
  char *message;
  struct ui_component *overlay_component;
  struct ui_overlay *active_overlay;
};

struct ui_toast_region_stack {
  struct ui_toast_entry **toasts;
  size_t count;
  size_t capacity;
};

struct ui_toast_manager_base {
  struct ui_toast_region_stack regions[UI_TOAST_REGION_COUNT];
  ui_toast_id next_id;
  int is_hovered;
};

extern int g_malloc_fail_countdown;
extern int g_toast_mock_fail;
extern int g_toast_mock_set_attr_fail_target;
extern int g_toast_mock_destroy_target;
extern int g_toast_destroy_counter;

TEST test_toast_invalid_args(void) {
  struct ui_toast_manager_base *mgr = NULL;
  struct ui_toast_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_event ev;
  ui_toast_id tid;
  ui_error_t rc;

  memset(&cfg, 0, sizeof(cfg));
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toast_manager_base_create(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_toast_manager_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_show(NULL, &cfg, 0.0, &tid));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toast_manager_base_dismiss(NULL, 1));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toast_manager_base_tick(NULL, 0.0));
  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_handle_event(NULL, &ev, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_render(NULL, director));

  rc = ui_toast_manager_base_create(&mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_show(mgr, NULL, 0.0, &tid));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_show(mgr, &cfg, 0.0, NULL));

  cfg.region = UI_TOAST_REGION_COUNT;
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid));
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;

  ASSERT_EQ(UI_ERROR_NOT_FOUND, ui_toast_manager_base_dismiss(mgr, 9999));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_toast_manager_base_handle_event(mgr, NULL, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_toast_manager_base_render(mgr, NULL));

  rc = ui_toast_manager_base_destroy(mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toast_lifecycle_and_rendering(void) {
  struct ui_toast_manager_base *mgr = NULL;
  struct ui_toast_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_event ev;
  ui_toast_id ids[10];
  ui_error_t rc;
  int i;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toast_manager_base_create(&mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Render when no toasts exist -> returns NONE */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Show a toast in every region to cover region styling branches */
  for (i = 0; i < UI_TOAST_REGION_COUNT; i++) {
    memset(&cfg, 0, sizeof(cfg));
    cfg.region = (enum ui_toast_region)i;
    cfg.duration_secs = 2.0;
    cfg.message = "Region msg";
    cfg.is_error = (i % 2 == 0);
    rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &ids[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Show toast without message */
  memset(&cfg, 0, sizeof(cfg));
  cfg.region = UI_TOAST_REGION_TOP_LEFT;
  cfg.duration_secs = 0.0; /* Persistent */
  cfg.message = NULL;
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &ids[6]);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add multiple toasts in same region to trigger capacity reallocation (4 ->
   * 8) */
  for (i = 7; i < 10; i++) {
    memset(&cfg, 0, sizeof(cfg));
    cfg.region = UI_TOAST_REGION_BOTTOM_LEFT;
    cfg.duration_secs = 1.0;
    cfg.message = "Multi msg";
    rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &ids[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* Render toasts */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Render again: unmounts existing active_overlay on line 433 and rebuilds */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick before 0.3s while in SLIDE_IN (covers line 390 false branch) */
  rc = ui_toast_manager_base_tick(mgr, 0.1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Dismiss the 2nd toast in a multi-toast region (covers line 355 false
   * branch) */
  rc = ui_toast_manager_base_dismiss(mgr, ids[8]);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Event handling: hover flow */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN; /* Unhandled type */
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 0.1);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 0.1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Sending MOUSE_MOVE again while already hovered (no-op) */
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 0.2);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick while paused: active time doesn't progress toward expiration */
  rc = ui_toast_manager_base_tick(mgr, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Unpause with TOUCH_END */
  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 1.5);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Sending TOUCH_END again while already unpaused (no-op) */
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 1.6);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pause with TOUCH_START and unpause with TOUCH_CANCEL */
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 1.7);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_CANCEL;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 1.8);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Pause and unpause with MOUSE_UP */
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 1.9);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_toast_manager_base_handle_event(mgr, &ev, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Explicit dismiss of id 0 */
  rc = ui_toast_manager_base_dismiss(mgr, ids[0]);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Dismissing again while in SLIDE_OUT state returns NONE */
  rc = ui_toast_manager_base_dismiss(mgr, ids[0]);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick past auto dismiss for duration 1.0 and 2.0 toasts */
  rc = ui_toast_manager_base_tick(mgr, 10.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Persistent toast (id 6) remains; dismiss it explicitly */
  rc = ui_toast_manager_base_dismiss(mgr, ids[6]);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toast_manager_base_tick(mgr, 11.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_toast_manager_base_destroy(mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toast_error_branches(void) {
  struct ui_toast_manager_base *mgr = NULL;
  struct ui_toast_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_toast_id tid;
  ui_error_t rc;
  int i;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&cfg, 0, sizeof(cfg));
  cfg.region = UI_TOAST_REGION_TOP_LEFT;
  cfg.duration_secs = 2.0;
  cfg.message = "Err msg";

  rc = ui_toast_manager_base_create(&mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Show while manager is hovered (covers line 295-296) */
  mgr->is_hovered = 1;
  rc = ui_toast_manager_base_show(mgr, &cfg, 1.0, &tid);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  mgr->is_hovered = 0;

  /* Render once so active_overlay is set */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. Mock unmount failure in render (line 433) */
  g_toast_mock_fail = 3;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  /* 2. Mock get_region_style failure in render (line 463 and 594) */
  g_toast_mock_fail = 4;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  /* get_region_style failure with dom_node_destroy failure (line 619) */
  g_toast_mock_fail = 4;
  g_toast_mock_destroy_target = 1;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;
  g_toast_mock_destroy_target = 0;

  /* 3. Mock set_attribute failures in render (calls 1, 2, 3) without and with
   * destroy mock */
  for (i = 1; i <= 3; i++) {
    g_toast_mock_set_attr_fail_target = i;
    rc = ui_toast_manager_base_render(mgr, director);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toast_mock_set_attr_fail_target = 0;

    g_toast_mock_set_attr_fail_target = i;
    g_toast_mock_fail = 2; /* dom_node_destroy also fails */
    rc = ui_toast_manager_base_render(mgr, director);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_toast_mock_set_attr_fail_target = 0;
    g_toast_mock_fail = 0;
  }

  /* 4. Mock text_node create failure in render (line 628) without and with
   * destroy mock (line 630) */
  g_toast_mock_fail = 8;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  g_toast_mock_fail = 8;
  g_toast_mock_destroy_target = 1;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;
  g_toast_mock_destroy_target = 0;

  /* 5. Mock append_child failure in render (lines 644-652) with targets 1 and 2
   */
  g_toast_mock_fail = 5;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  g_toast_mock_fail = 5;
  g_toast_mock_destroy_target = 1;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;
  g_toast_mock_destroy_target = 0;

  g_toast_mock_fail = 5;
  g_toast_mock_destroy_target = 2;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;
  g_toast_mock_destroy_target = 0;

  /* 6. Mock mount_component failure in render */
  g_toast_mock_fail = 6;
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  /* Successful render to set shadow_root, then destroy shadow_root mock failure
   */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toast_mock_fail = 2; /* shadow_root destroy fails */
  rc = ui_toast_manager_base_render(mgr, director);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_toast_mock_fail = 0;

  /* 7. Render strdup OOM (lines 635-641) */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    ui_toast_manager_base_render(mgr, director);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = i;
    g_toast_mock_destroy_target = 1;
    ui_toast_manager_base_render(mgr, director);
    g_malloc_fail_countdown = -1;
    g_toast_mock_destroy_target = 0;

    g_malloc_fail_countdown = i;
    g_toast_mock_destroy_target = 2;
    ui_toast_manager_base_render(mgr, director);
    g_malloc_fail_countdown = -1;
    g_toast_mock_destroy_target = 0;
  }

  /* 6. Mock free_toast_entry failure (lines 179, 189, 203) */
  g_toast_mock_fail = 10;
  g_malloc_fail_countdown = 1; /* Causes strdup to fail in show */
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_malloc_fail_countdown = -1;
  g_toast_mock_fail = 0;

  /* Component create failure in show (message = NULL, countdown = 1) */
  cfg.message = NULL;
  g_malloc_fail_countdown = 1;
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_toast_mock_fail = 10;
  g_malloc_fail_countdown = 1;
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_malloc_fail_countdown = -1;
  g_toast_mock_fail = 0;
  cfg.message = "Err msg";

  /* Realloc failure in show with and without free_toast_entry mock */
  cfg.region = UI_TOAST_REGION_TOP_CENTER;
  for (i = 0; i < 4; i++) {
    rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  /* Now capacity 4 is full. Next show triggers realloc (allocation 3) */
  g_malloc_fail_countdown = 3;
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  g_toast_mock_fail = 10;
  g_malloc_fail_countdown = 3;
  rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_malloc_fail_countdown = -1;
  g_toast_mock_fail = 0;
  cfg.region = UI_TOAST_REGION_TOP_LEFT;

  /* 7. Free NULL entry during destroy (line 226) */
  mgr->regions[UI_TOAST_REGION_TOP_LEFT].toasts[0] = NULL;

  /* 8. Mock component_destroy failure during manager destroy (line 31) */
  g_toast_mock_fail = 1;
  rc = ui_toast_manager_base_destroy(mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_toast_mock_fail = 0;

  /* 9. Render strdup OOM with destroy targets 1 and 2 (lines 662, 665) */
  {
    struct ui_toast_manager_base *m_fresh = NULL;
    rc = ui_toast_manager_base_create(&m_fresh);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_toast_manager_base_show(m_fresh, &cfg, 0.0, &tid);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_toast_destroy_counter = 0;
    g_toast_mock_destroy_target = 1;
    g_malloc_fail_countdown = 11;
    rc = ui_toast_manager_base_render(m_fresh, director);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_malloc_fail_countdown = -1;
    g_toast_mock_destroy_target = 0;
    g_toast_destroy_counter = 0;

    g_toast_mock_destroy_target = 2;
    g_malloc_fail_countdown = 11;
    rc = ui_toast_manager_base_render(m_fresh, director);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_malloc_fail_countdown = -1;
    g_toast_mock_destroy_target = 0;
    g_toast_destroy_counter = 0;

    rc = ui_toast_manager_base_destroy(m_fresh);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_toast_oom(void) {
  struct ui_toast_manager_base *mgr = NULL;
  struct ui_toast_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root = NULL;
  ui_toast_id tid;
  ui_error_t rc;
  int i;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&cfg, 0, sizeof(cfg));
  cfg.region = UI_TOAST_REGION_TOP_RIGHT;
  cfg.duration_secs = 2.0;
  cfg.message = "OOM test message";

  /* Creation OOM loop */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    mgr = NULL;
    rc = ui_toast_manager_base_create(&mgr);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_toast_manager_base_destroy(mgr);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(mgr == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* Show OOM loop */
  rc = ui_toast_manager_base_create(&mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_toast_manager_base_show(mgr, &cfg, 0.0, &tid);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  /* Render OOM loop */
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_toast_manager_base_render(mgr, director);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      break;
    }
  }
  g_malloc_fail_countdown = -1;

  rc = ui_toast_manager_base_destroy(mgr);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_toast_manager_base_suite) {
  RUN_TEST(test_toast_invalid_args);
  RUN_TEST(test_toast_lifecycle_and_rendering);
  RUN_TEST(test_toast_error_branches);
  RUN_TEST(test_toast_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_toast_manager_base_suite);
  GREATEST_MAIN_END();
}
