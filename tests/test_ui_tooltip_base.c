/* clang-format off */
#include "greatest.h"
#include "ui_tooltip_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_overlay_director.h"
#include "ui_geometry_anchor.h"
#include "ui_signal.h"
#include "ui_computed.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_tooltip_base {
  struct ui_tooltip_config config;
  int state;
  double state_enter_time;
  char *text;
  struct ui_component *overlay_component;
  struct ui_overlay *active_overlay;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

extern int g_malloc_fail_countdown;
extern int g_tooltip_mock_fail;

TEST test_tooltip_invalid_args(void) {
  struct ui_tooltip_base *tt = NULL;
  struct ui_tooltip_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_layout_node trig_layout;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  int is_visible = 0;
  struct ui_computed *comp = NULL;
  ui_error_t rc;

  memset(&cfg, 0, sizeof(cfg));
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_create(NULL, &cfg));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_create(&tt, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_tooltip_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_set_text(NULL, "txt"));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_is_visible(NULL, &is_visible));
  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_handle_event(NULL, &ev, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_tick(NULL, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_hide(NULL));

  memset(&trig_layout, 0, sizeof(trig_layout));
  memset(&anchor, 0, sizeof(anchor));
  ASSERT_EQ(
      UI_ERROR_INVALID_ARGUMENT,
      ui_tooltip_base_render(NULL, director, &trig_layout, &anchor, 800, 600));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_bind_open(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_get_animating_signal(NULL, &comp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_get_animating_signal(NULL, NULL));

  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tooltip_base_is_visible(tt, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_handle_event(tt, NULL, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_render(tt, NULL, &trig_layout, &anchor, 800, 600));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_render(tt, director, NULL, &anchor, 800, 600));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_render(tt, director, &trig_layout, NULL, 800, 600));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tooltip_base_get_animating_signal(tt, NULL));

  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tooltip_lifecycle_and_rendering(void) {
  struct ui_tooltip_base *tt = NULL;
  struct ui_tooltip_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_layout_node trig_layout;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  int is_visible = 0;
  struct ui_computed *comp = NULL;
  ui_error_t rc;

  cfg.hover_delay_secs = 0.5;
  cfg.focus_delay_secs = 0.2;
  cfg.touch_hold_delay_secs = 0.8;
  cfg.hide_delay_secs = 0.3;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_tooltip_base_bind_open(tt, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_get_animating_signal(tt, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set text: test overwriting and clearing */
  rc = ui_tooltip_base_set_text(tt, "temp");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_set_text(tt, "final");
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Initial state: not visible */
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Tick while IDLE (hits default branch) */
  rc = ui_tooltip_base_tick(tt, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Send dismiss events while IDLE (hits false branch of state != IDLE) */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_PEN_UP;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 1. Hover flow */
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Unhandled event type */
  ev.type = UI_EVENT_MOUSE_UP;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* While in HOVER_DELAY, sending MOUSE_MOVE, TOUCH_START, PEN_DOWN hits false
   * branch of state == IDLE */
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_PEN_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Tick before hover delay */
  rc = ui_tooltip_base_tick(tt, 0.2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Tick past hover delay -> becomes visible */
  rc = ui_tooltip_base_tick(tt, 0.6);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Tick while VISIBLE (hits default branch) */
  rc = ui_tooltip_base_tick(tt, 0.7);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Render while visible */
  trig_layout.x = 100.0f;
  trig_layout.y = 100.0f;
  trig_layout.width = 50.0f;
  trig_layout.height = 20.0f;
  anchor.overlay_x = UI_ANCHOR_EDGE_CENTER;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.target_x = UI_ANCHOR_EDGE_CENTER;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.offset_x = 0;
  anchor.offset_y = 5;

  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800.0f,
                              600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Render again while already mounted -> early exit line 318 */
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800.0f,
                              600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Dismiss with KEY_DOWN */
  ev.type = UI_EVENT_KEY_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Render while NOT visible unmounts active overlay */
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800.0f,
                              600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(tt->active_overlay == NULL);

  /* Render again while not visible and active_overlay is NULL */
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800.0f,
                              600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2. Focus flow */
  ev.type = UI_EVENT_PEN_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick before focus delay expires (false branch) */
  rc = ui_tooltip_base_tick(tt, 2.1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick past focus delay -> visible */
  rc = ui_tooltip_base_tick(tt, 2.3);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Blur flow: PEN_UP */
  ev.type = UI_EVENT_PEN_UP;
  rc = ui_tooltip_base_handle_event(tt, &ev, 3.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* PEN_UP second time while already in HIDE_DELAY (covers line 332 false
   * branch) */
  rc = ui_tooltip_base_handle_event(tt, &ev, 3.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick before hide delay expires (false branch) */
  rc = ui_tooltip_base_tick(tt, 3.1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick past hide delay -> IDLE */
  rc = ui_tooltip_base_tick(tt, 3.4);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* 3. Touch hold flow */
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 4.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick before touch hold delay expires (false branch) */
  rc = ui_tooltip_base_tick(tt, 4.2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* Tick past delay -> visible */
  rc = ui_tooltip_base_tick(tt, 4.9);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, is_visible);

  /* Hide delay via TOUCH_END, TOUCH_CANCEL, WINDOW_RESIZE */
  ev.type = UI_EVENT_TOUCH_CANCEL;
  rc = ui_tooltip_base_handle_event(tt, &ev, 5.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  /* When already in HIDE_DELAY, sending another resize does nothing */
  ev.type = UI_EVENT_WINDOW_RESIZE;
  rc = ui_tooltip_base_handle_event(tt, &ev, 5.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Hard hide immediately */
  rc = ui_tooltip_base_hide(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_is_visible(tt, &is_visible);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, is_visible);

  /* Render without text */
  rc = ui_tooltip_base_set_text(tt, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 6.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_tick(tt, 6.9);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800.0f,
                              600.0f);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Destroy with overlay_component == NULL */
  rc = ui_component_destroy(tt->overlay_component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  tt->overlay_component = NULL;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tooltip_error_branches(void) {
  struct ui_tooltip_base *tt = NULL;
  struct ui_tooltip_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_layout_node trig_layout;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  ui_error_t rc;

  cfg.hover_delay_secs = 0.5;
  cfg.focus_delay_secs = 0.2;
  cfg.touch_hold_delay_secs = 0.8;
  cfg.hide_delay_secs = 0.3;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  trig_layout.x = 100.0f;
  trig_layout.y = 100.0f;
  trig_layout.width = 50.0f;
  trig_layout.height = 20.0f;
  anchor.overlay_x = UI_ANCHOR_EDGE_CENTER;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.target_x = UI_ANCHOR_EDGE_CENTER;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.offset_x = 0;
  anchor.offset_y = 5;

  /* 1. transition_state mock failures in handle_event */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  memset(&ev, 0, sizeof(ev));
  g_tooltip_mock_fail = 10;

  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.type = UI_EVENT_PEN_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  g_tooltip_mock_fail = 0;
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  g_tooltip_mock_fail = 10;
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_tooltip_base_handle_event(tt, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.type = UI_EVENT_PEN_UP;
  rc = ui_tooltip_base_handle_event(tt, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 2. transition_state mock failures in tick */
  /* HOVER_DELAY */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tooltip_mock_fail = 10;
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* FOCUS_DELAY */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_PEN_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tooltip_mock_fail = 10;
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* TOUCH_HOLD_DELAY */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tooltip_mock_fail = 10;
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* HIDE_DELAY */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_END;
  rc = ui_tooltip_base_handle_event(tt, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tooltip_mock_fail = 10;
  rc = ui_tooltip_base_tick(tt, 2.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 3. Render failures */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_set_text(tt, "render_err");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Mock append_child failure in render */
  g_tooltip_mock_fail = 6;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* Mock is_visible failure in render (line 436) */
  g_tooltip_mock_fail = 7;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* Mock geometry_anchor failure in render (line 438) */
  g_tooltip_mock_fail = 4;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* Mock mount_component failure in render */
  g_tooltip_mock_fail = 2;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* Mock destroy shadow_root failure in render */
  g_tooltip_mock_fail = 1;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* Successful render */
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Dismiss, then mock unmount failure */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_tooltip_base_handle_event(tt, &ev, 2.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tooltip_mock_fail = 3;
  rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  /* 4. Component destroy failure in destroy */
  g_tooltip_mock_fail = 5;
  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tooltip_mock_fail = 0;

  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tooltip_oom(void) {
  struct ui_tooltip_base *tt = NULL;
  struct ui_tooltip_config cfg;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_layout_node trig_layout;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  cfg.hover_delay_secs = 0.5;
  cfg.focus_delay_secs = 0.2;
  cfg.touch_hold_delay_secs = 0.8;
  cfg.hide_delay_secs = 0.3;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_create(root_node, &director);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  trig_layout.x = 100.0f;
  trig_layout.y = 100.0f;
  trig_layout.width = 50.0f;
  trig_layout.height = 20.0f;
  anchor.overlay_x = UI_ANCHOR_EDGE_CENTER;
  anchor.overlay_y = UI_ANCHOR_EDGE_START;
  anchor.target_x = UI_ANCHOR_EDGE_CENTER;
  anchor.target_y = UI_ANCHOR_EDGE_END;
  anchor.offset_x = 0;
  anchor.offset_y = 5;

  /* Creation OOM loop */
  for (i = 0; i < 5; i++) {
    g_malloc_fail_countdown = i;
    tt = NULL;
    rc = ui_tooltip_base_create(&tt, &cfg);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_tooltip_base_destroy(tt);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(tt == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* set_text OOM */
  rc = ui_tooltip_base_create(&tt, &cfg);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_malloc_fail_countdown = 0;
  rc = ui_tooltip_base_set_text(tt, "oom_text");
  ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  g_malloc_fail_countdown = -1;

  /* Render OOM */
  rc = ui_tooltip_base_set_text(tt, "valid_text");
  ASSERT_EQ(UI_ERROR_NONE, rc);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_TOUCH_START;
  rc = ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tooltip_base_tick(tt, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  for (i = 0; i < 15; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
  }
  g_malloc_fail_countdown = -1;

  rc = ui_tooltip_base_destroy(tt);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_overlay_director_destroy(director);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(root_node);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

SUITE(ui_tooltip_base_suite) {
  RUN_TEST(test_tooltip_invalid_args);
  RUN_TEST(test_tooltip_lifecycle_and_rendering);
  RUN_TEST(test_tooltip_error_branches);
  RUN_TEST(test_tooltip_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_tooltip_base_suite);
  GREATEST_MAIN_END();
}
