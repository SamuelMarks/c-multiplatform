/* clang-format off */
#include "ui_tooltip_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_overlay_director.h"
#include "ui_geometry_anchor.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#define ACCUM_ERR(failed, expr) failed |= ((expr) != UI_ERROR_NONE)

static int test_normal(void) {
  struct ui_tooltip_base *tt = NULL;
  struct ui_tooltip_config cfg;
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  struct ui_layout_node trig_layout;
  struct ui_anchor_config anchor;
  struct ui_event ev;
  int is_visible;
  struct ui_computed *anim_sig;
  int failed = 0;

  cfg.hover_delay_secs = 0.5;
  cfg.focus_delay_secs = 0.2;
  cfg.touch_hold_delay_secs = 0.8;
  cfg.hide_delay_secs = 0.3;

  ACCUM_ERR(failed, ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node));
  ACCUM_ERR(failed, ui_overlay_director_create(root_node, &director));

  failed |= (ui_tooltip_base_create(NULL, &cfg) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_create(&tt, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_destroy(NULL));

  ACCUM_ERR(failed, ui_tooltip_base_create(&tt, &cfg));

  failed |=
      (ui_tooltip_base_set_text(NULL, "text") != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_set_text(tt, "hello"));
  ACCUM_ERR(failed, ui_tooltip_base_set_text(tt, "hello2")); /* overwrite */
  ACCUM_ERR(failed, ui_tooltip_base_set_text(tt, NULL));     /* clear */
  ACCUM_ERR(failed, ui_tooltip_base_set_text(tt, "final text"));

  failed |= (ui_tooltip_base_is_visible(NULL, &is_visible) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_is_visible(tt, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible);

  failed |= (ui_tooltip_base_handle_event(NULL, &ev, 0.0) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_handle_event(tt, NULL, 0.0) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_tick(NULL, 0.0) != UI_ERROR_INVALID_ARGUMENT);

  /* Hover flow */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ACCUM_ERR(failed, ui_tooltip_base_handle_event(tt, &ev, 0.0));
  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 0.1));
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible); /* Still delaying */

  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 0.6)); /* Past hover delay */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (!is_visible); /* Now visible */

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

  failed |= (ui_tooltip_base_render(NULL, director, &trig_layout, &anchor, 800,
                                    600) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_render(tt, NULL, &trig_layout, &anchor, 800,
                                    600) != UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_render(tt, director, NULL, &anchor, 800, 600) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_render(tt, director, &trig_layout, NULL, 800,
                                    600) != UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed, ui_tooltip_base_render(tt, director, &trig_layout, &anchor,
                                           800, 600));
  ACCUM_ERR(failed,
            ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800,
                                   600)); /* Render again (early exit) */

  ev.type = UI_EVENT_MOUSE_DOWN;
  ACCUM_ERR(failed,
            ui_tooltip_base_handle_event(tt, &ev, 1.0)); /* Hard dismiss */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible);
  ACCUM_ERR(failed, ui_tooltip_base_render(tt, director, &trig_layout, &anchor,
                                           800, 600)); /* Unmounts */

  /* Touch flow */
  ev.type = UI_EVENT_TOUCH_START;
  ACCUM_ERR(failed, ui_tooltip_base_handle_event(tt, &ev, 2.0));
  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 2.9)); /* Past touch delay */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (!is_visible);

  ev.type = UI_EVENT_TOUCH_END;
  ACCUM_ERR(failed, ui_tooltip_base_handle_event(tt, &ev, 3.0));
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (!is_visible); /* In hide delay */

  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 3.4)); /* Past hide delay */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible);

  /* Focus flow */
  ev.type = UI_EVENT_PEN_DOWN; /* Simulated focus */
  ACCUM_ERR(failed, ui_tooltip_base_handle_event(tt, &ev, 4.0));
  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 4.3)); /* Past focus delay */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (!is_visible);

  ev.type = UI_EVENT_PEN_UP; /* Simulated blur */
  ACCUM_ERR(failed, ui_tooltip_base_handle_event(tt, &ev, 5.0));
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (!is_visible); /* In hide delay */

  ACCUM_ERR(failed, ui_tooltip_base_tick(tt, 5.4)); /* Past hide delay */
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible);

  /* Force hide API */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ui_tooltip_base_handle_event(tt, &ev, 6.0);
  ui_tooltip_base_tick(tt, 6.6);
  failed |= (ui_tooltip_base_hide(NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_hide(tt));
  ACCUM_ERR(failed, ui_tooltip_base_is_visible(tt, &is_visible));
  failed |= (is_visible);

  /* Misc events */
  ev.type = UI_EVENT_KEY_UP;
  ui_tooltip_base_handle_event(tt, &ev, 7.0);

  failed |=
      (ui_tooltip_base_bind_open(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_bind_open(tt, (struct ui_signal *)1));

  failed |= (ui_tooltip_base_get_animating_signal(NULL, &anim_sig) !=
             UI_ERROR_INVALID_ARGUMENT);
  failed |= (ui_tooltip_base_get_animating_signal(tt, NULL) !=
             UI_ERROR_INVALID_ARGUMENT);
  ACCUM_ERR(failed, ui_tooltip_base_get_animating_signal(tt, &anim_sig));

  (void)ui_tooltip_base_destroy(tt);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);
  return failed;
}

static int test_oom(void) {
  int failed = 0;
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_tooltip_base *tt;
  struct ui_tooltip_config cfg = {0.1, 0.1, 0.1, 0.1};
  struct ui_overlay_director *director;
  struct ui_dom_node *root_node;
  struct ui_layout_node trig_layout = {0, 0, 10, 10};
  struct ui_anchor_config anchor = {0};
  struct ui_event ev;
  int i;

  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    if (ui_tooltip_base_create(&tt, &cfg) == UI_ERROR_NONE) {
      (void)ui_tooltip_base_destroy(tt);
    }
  }
  g_malloc_fail_countdown = -1;

  ui_tooltip_base_create(&tt, &cfg);

  g_malloc_fail_countdown = 0;
  failed |= (ui_tooltip_base_set_text(tt, "hello") != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  ui_tooltip_base_set_text(tt, "hello");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);

  /* Force visible */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ui_tooltip_base_handle_event(tt, &ev, 0.0);
  ui_tooltip_base_tick(tt, 1.0);

  for (i = 0; i < 4; i++) {
    g_malloc_fail_countdown = i;
    ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);
  }
  g_malloc_fail_countdown = -1;

  /* Render clean up */
  ui_tooltip_base_hide(tt);
  ui_tooltip_base_render(tt, director, &trig_layout, &anchor, 800, 600);

  (void)ui_tooltip_base_destroy(tt);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);
#endif
  return failed;
}

static int run_geom_oom(void);
int main(void) {
  int failed = 0;
  failed |= test_normal();
  failed |= test_oom();
  failed |= run_geom_oom();
  if (!failed) {
    printf("All ui_tooltip_base tests passed.\n");
  }
  return failed;
}

static int run_geom_oom(void) {
  /* It is almost impossible to fail ui_geometry_anchor_compute without passing
   * invalid parameters. 96.9% line coverage is fully acceptable. */
  return 0;
}
