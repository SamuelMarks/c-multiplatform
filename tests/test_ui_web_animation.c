/* clang-format off */
#include "ui_web_animation.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

static int test_web_animation_lifecycle(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_effect *effect = NULL;
  struct ui_web_animation_timeline *timeline = NULL;
  struct ui_web_animation *animation = NULL;
  struct ui_web_animation_timing timing;
  ui_error_t err;
  int failed = 0;

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  failed |= (err != UI_ERROR_NONE);

  timing.delay = 0.0;
  timing.end_delay = 0.0;
  timing.fill_mode = UI_WEB_ANIMATION_FILL_MODE_NONE;
  timing.iteration_start = 0.0;
  timing.iterations = 1.0;
  timing.duration = 1000.0;
  timing.direction = UI_WEB_ANIMATION_DIRECTION_NORMAL;

  err = ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing,
                                                       &effect);
  failed |= (err != UI_ERROR_NONE);

  err = ui_web_animation_timeline_create_document_timeline(&timeline);
  failed |= (err != UI_ERROR_NONE);

  err = ui_web_animation_create(effect, timeline, &animation);
  failed |= (err != UI_ERROR_NONE);

  {
    enum ui_web_animation_play_state state;
    double current_time;

    err = ui_web_animation_get_play_state(animation, &state);
    failed |= (err != UI_ERROR_NONE);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_IDLE);

    err = ui_web_animation_get_current_time(animation, &current_time);
    failed |= (err != UI_ERROR_NONE);
    failed |= (current_time != 0.0);

    err = ui_web_animation_play(animation);
    failed |= (err != UI_ERROR_NONE);

    err = ui_web_animation_get_play_state(animation, &state);
    failed |= (err != UI_ERROR_NONE);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_RUNNING);

    err = ui_web_animation_tick(animation, 500.0);
    failed |= (err != UI_ERROR_NONE);

    err = ui_web_animation_get_current_time(animation, &current_time);
    failed |= (err != UI_ERROR_NONE);
    failed |= (current_time != 500.0);

    err = ui_web_animation_tick(animation, 600.0);
    failed |= (err != UI_ERROR_NONE);

    err = ui_web_animation_get_play_state(animation, &state);
    failed |= (err != UI_ERROR_NONE);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_FINISHED);

    err = ui_web_animation_get_current_time(animation, &current_time);
    failed |= (err != UI_ERROR_NONE);
    failed |= (current_time != 1000.0);
  }

  ui_web_animation_destroy(animation);
  (void)ui_dom_node_destroy(node);
  return failed;
}

static int test_web_animation_reverse(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_effect *effect = NULL;
  struct ui_web_animation_timeline *timeline = NULL;
  struct ui_web_animation *animation = NULL;
  struct ui_web_animation_timing timing;
  int failed = 0;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  timing.delay = 0.0;
  timing.end_delay = 0.0;
  timing.fill_mode = UI_WEB_ANIMATION_FILL_MODE_NONE;
  timing.iteration_start = 0.0;
  timing.iterations = 2.0;
  timing.duration = 500.0;
  timing.direction = UI_WEB_ANIMATION_DIRECTION_NORMAL;

  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  ui_web_animation_timeline_create_document_timeline(&timeline);
  ui_web_animation_create(effect, timeline, &animation);

  ui_web_animation_play(animation);
  ui_web_animation_tick(animation, 500.0);

  {
    double current_time;
    enum ui_web_animation_play_state state;
    ui_web_animation_get_current_time(animation, &current_time);
    failed |= (current_time != 500.0);

    ui_web_animation_reverse(animation);

    ui_web_animation_tick(animation, 200.0);
    ui_web_animation_get_current_time(animation, &current_time);
    failed |= (current_time != 300.0);

    ui_web_animation_tick(animation, 400.0);
    ui_web_animation_get_current_time(animation, &current_time);
    failed |= (current_time != 0.0);

    ui_web_animation_get_play_state(animation, &state);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_FINISHED);
  }

  ui_web_animation_destroy(animation);
  (void)ui_dom_node_destroy(node);
  return failed;
}

static int test_scroll_driven_timelines(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_timeline *scroll_timeline = NULL;
  struct ui_web_animation_timeline *view_timeline = NULL;
  struct ui_web_animation_timeline *pointer_timeline = NULL;
  ui_error_t err;
  int failed = 0;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  err = ui_web_animation_timeline_create_scroll_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK, &scroll_timeline);
  failed |= (err != UI_ERROR_NONE);

  err = ui_web_animation_timeline_create_view_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_Y, &view_timeline);
  failed |= (err != UI_ERROR_NONE);

  err = ui_web_animation_timeline_create_pointer_timeline(
      node, UI_WEB_ANIMATION_POINTER_AXIS_X, &pointer_timeline);
  failed |= (err != UI_ERROR_NONE);

  err = ui_web_animation_timeline_set_current_time(scroll_timeline, 50.0);
  failed |= (err != UI_ERROR_NONE);

  ui_web_animation_timeline_destroy(scroll_timeline);
  ui_web_animation_timeline_destroy(view_timeline);
  ui_web_animation_timeline_destroy(pointer_timeline);
  (void)ui_dom_node_destroy(node);
  return failed;
}

static int test_web_animation_extra(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_effect *effect = NULL;
  struct ui_web_animation_timeline *timeline = NULL;
  struct ui_web_animation *animation = NULL;
  struct ui_web_animation_timing timing;
  int failed = 0;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  timing.delay = 0.0;
  timing.end_delay = 0.0;
  timing.fill_mode = UI_WEB_ANIMATION_FILL_MODE_NONE;
  timing.iteration_start = 0.0;
  timing.iterations = 1.0;
  timing.duration = 1000.0;
  timing.direction = UI_WEB_ANIMATION_DIRECTION_NORMAL;

  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  ui_web_animation_timeline_create_document_timeline(&timeline);
  ui_web_animation_create(effect, timeline, &animation);

  /* Test set/get playback rate */
  ui_web_animation_set_playback_rate(animation, 2.0);
  {
    double rate = 0.0;
    ui_web_animation_get_playback_rate(animation, &rate);
    failed |= (rate != 2.0);
  }

  /* Test set current time */
  ui_web_animation_set_current_time(animation, 200.0);
  {
    double ct = 0.0;
    ui_web_animation_get_current_time(animation, &ct);
    failed |= (ct != 200.0);
  }

  /* Test pause */
  ui_web_animation_play(animation);
  ui_web_animation_pause(animation);
  {
    enum ui_web_animation_play_state state;
    ui_web_animation_get_play_state(animation, &state);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_PAUSED);
  }

  /* Test finish */
  ui_web_animation_finish(animation);
  {
    enum ui_web_animation_play_state state;
    ui_web_animation_get_play_state(animation, &state);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_FINISHED);
  }

  /* Test cancel */
  ui_web_animation_cancel(animation);
  {
    enum ui_web_animation_play_state state;
    ui_web_animation_get_play_state(animation, &state);
    failed |= (state != UI_WEB_ANIMATION_PLAY_STATE_IDLE);
  }

  ui_web_animation_destroy(animation);

  /* Test effect destroy alone */
  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  ui_web_animation_effect_destroy(effect);

  (void)ui_dom_node_destroy(node);
  return failed;
}

static int test_web_animation_coverage(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_effect *effect = NULL;
  struct ui_web_animation_timeline *timeline = NULL;
  struct ui_web_animation_timeline *scroll_timeline = NULL;
  struct ui_web_animation_timeline *view_timeline = NULL;
  struct ui_web_animation_timeline *pointer_timeline = NULL;
  struct ui_web_animation *animation = NULL;
  struct ui_web_animation_timing timing;
  int failed = 0;
  enum ui_web_animation_play_state state;
  double dval;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  timing.delay = 0.0;
  timing.end_delay = 0.0;
  timing.fill_mode = UI_WEB_ANIMATION_FILL_MODE_NONE;
  timing.iteration_start = 0.0;
  timing.iterations = 1.0;
  timing.duration = 1000.0;
  timing.direction = UI_WEB_ANIMATION_DIRECTION_NORMAL;

  /* Null checks */
  ui_web_animation_effect_create_keyframe_effect(NULL, NULL, &timing, NULL);
  ui_web_animation_effect_create_keyframe_effect(node, NULL, NULL, &effect);
  ui_web_animation_timeline_create_document_timeline(NULL);
  ui_web_animation_timeline_create_scroll_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK, NULL);
  ui_web_animation_timeline_create_view_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK, NULL);
  ui_web_animation_timeline_create_pointer_timeline(
      node, UI_WEB_ANIMATION_POINTER_AXIS_X, NULL);
  ui_web_animation_timeline_set_current_time(NULL, 0.0);
  ui_web_animation_timeline_destroy(NULL);
  ui_web_animation_effect_destroy(NULL);
  ui_web_animation_create(effect, timeline, NULL);
  ui_web_animation_destroy(NULL);
  ui_web_animation_play(NULL);
  ui_web_animation_pause(NULL);
  ui_web_animation_reverse(NULL);
  ui_web_animation_cancel(NULL);
  ui_web_animation_finish(NULL);
  ui_web_animation_set_current_time(NULL, 0.0);
  ui_web_animation_set_playback_rate(NULL, 0.0);
  ui_web_animation_tick(NULL, 0.0);

  ui_web_animation_get_play_state(NULL, &state);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = 0;
  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  g_malloc_fail_countdown = 0;
  ui_web_animation_timeline_create_document_timeline(&timeline);
  g_malloc_fail_countdown = 0;
  ui_web_animation_timeline_create_scroll_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK, &scroll_timeline);
  g_malloc_fail_countdown = 0;
  ui_web_animation_timeline_create_view_timeline(
      node, UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK, &view_timeline);
  g_malloc_fail_countdown = 0;
  ui_web_animation_timeline_create_pointer_timeline(
      node, UI_WEB_ANIMATION_POINTER_AXIS_X, &pointer_timeline);
  g_malloc_fail_countdown = 0;
  ui_web_animation_create(
      effect, timeline,
      &animation); /* effect and timeline are NULL here due to failures, but
                      that's fine, create handles it if it fails first on MALLOC
                    */
  g_malloc_fail_countdown = -1;
#endif

  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  ui_web_animation_timeline_create_document_timeline(&timeline);
  ui_web_animation_create(effect, timeline, &animation);

  ui_web_animation_get_play_state(animation, NULL);
  ui_web_animation_get_current_time(animation, NULL);
  ui_web_animation_get_playback_rate(animation, NULL);

  /* Play with rate = 0 */
  ui_web_animation_set_playback_rate(animation, 0.0);
  ui_web_animation_play(animation);
  ui_web_animation_finish(animation);
  ui_web_animation_tick(animation, 10.0);

  /* Play with current_time < 0 */
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_set_current_time(animation, -1.0);
  ui_web_animation_play(animation);

  /* Play reverse with current_time > duration */
  ui_web_animation_cancel(animation);
  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_set_current_time(animation, 2000.0);
  ui_web_animation_play(animation);

  /* Set current_time when finished in reverse */
  ui_web_animation_finish(animation);
  ui_web_animation_set_current_time(animation, 500.0);

  /* Pause from IDLE */
  ui_web_animation_cancel(animation);
  ui_web_animation_pause(animation);

  /* Pause from FINISHED */
  ui_web_animation_finish(animation);
  ui_web_animation_pause(animation);

  /* Infinite duration */
  ui_web_animation_destroy(animation);
  timing.iterations = -1.0;
  ui_web_animation_effect_create_keyframe_effect(node, NULL, &timing, &effect);
  ui_web_animation_timeline_create_document_timeline(&timeline);
  ui_web_animation_create(effect, timeline, &animation);

  /* Reverse infinite duration at time 0 (hits play active_duration < 0) */
  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_play(animation);

  /* Set current_time > 0 and play reverse to hit the short-circuit */
  ui_web_animation_cancel(animation);
  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_set_current_time(animation, 500.0);
  ui_web_animation_play(animation);

  /* Finish infinite duration (hits finish active_duration < 0) */
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_finish(animation);

  /* Play infinite duration with current_time < 0 */
  ui_web_animation_cancel(animation);
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_set_current_time(animation, -500.0);
  ui_web_animation_play(animation);

  /* set_current_time on RUNNING infinite animation */
  ui_web_animation_set_current_time(animation, 500.0);

  ui_web_animation_play(animation);
  ui_web_animation_tick(animation, 2000.0);

  ui_web_animation_destroy(animation);
  (void)ui_dom_node_destroy(node);
  return failed;
}

static int test_web_animation_more_coverage(void) {
  struct ui_dom_node *node = NULL;
  struct ui_web_animation_effect *effect = NULL;
  struct ui_web_animation_timeline *timeline = NULL;
  struct ui_web_animation *animation = NULL;
  struct ui_web_animation_timing timing;
  int failed = 0;
  double dval;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);

  timing.delay = 0.0;
  timing.end_delay = 0.0;
  timing.fill_mode = UI_WEB_ANIMATION_FILL_MODE_NONE;
  timing.iteration_start = 0.0;
  timing.iterations = 1.0;
  timing.duration = 1000.0;
  timing.direction = UI_WEB_ANIMATION_DIRECTION_NORMAL;

  /* Add dummy properties to hit the free loop */
  struct ui_web_animation_keyframe *kf =
      C_MULTIPLATFORM_MALLOC(sizeof(struct ui_web_animation_keyframe));
  {
    struct ui_web_animation_keyframe_property *prop = C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_web_animation_keyframe_property));
    char *prop_name = C_MULTIPLATFORM_MALLOC(10);
    char *prop_val = C_MULTIPLATFORM_MALLOC(10);
    prop_name[0] = '\0';
    prop_val[0] = '\0';
    prop->property_name = prop_name;
    prop->value = prop_val;
    prop->next = NULL;
    kf->properties = prop;
    kf->next = NULL;
  }

  ui_web_animation_effect_create_keyframe_effect(node, kf, &timing, &effect);
  ui_web_animation_timeline_create_document_timeline(&timeline);
  ui_web_animation_create(effect, timeline, &animation);

  /* Hit line 252 (anim without effect in get_active_duration) */
  {
    struct ui_web_animation *anim2 = NULL;
    ui_web_animation_create(NULL, NULL, &anim2);
    ui_web_animation_set_current_time(anim2, 500.0);
    ui_web_animation_destroy(anim2);
  }

  /* Missing getters */
  ui_web_animation_get_current_time(NULL, &dval);
  ui_web_animation_get_playback_rate(NULL, &dval);

  /* Play when FINISHED */
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_finish(animation);
  ui_web_animation_play(animation);

  /* Play reverse when FINISHED */
  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_finish(animation);
  ui_web_animation_play(animation);

  /* set_current_time when RUNNING */
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_play(animation);
  ui_web_animation_set_current_time(
      animation, 500.0); /* hits RUNNING rate > 0, time < duration */
  ui_web_animation_set_current_time(animation, 2000.0); /* triggers FINISHED */
  ui_web_animation_set_current_time(
      animation, 3000.0); /* hits FINISHED rate > 0, time > duration */

  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_play(animation);
  ui_web_animation_set_current_time(
      animation, 500.0); /* hits RUNNING rate < 0, time > 0 */
  ui_web_animation_set_current_time(animation, -1000.0); /* triggers FINISHED */
  ui_web_animation_set_current_time(
      animation, -2000.0); /* hits FINISHED rate < 0, time < 0 */

  /* tick without timeline */
  {
    struct ui_web_animation *anim3 = NULL;
    ui_web_animation_create(NULL, NULL, &anim3);
    ui_web_animation_play(anim3);
    ui_web_animation_tick(anim3, 10.0);
    ui_web_animation_destroy(anim3);
  }

  /* Test set_current_time transitioning RUNNING -> FINISHED */
  ui_web_animation_play(animation);
  ui_web_animation_set_playback_rate(animation, 1.0);
  ui_web_animation_set_current_time(animation, 2000.0);

  ui_web_animation_set_current_time(animation, 500.0);
  ui_web_animation_play(animation);
  ui_web_animation_set_playback_rate(animation, -1.0);
  ui_web_animation_set_current_time(animation, -500.0);

  ui_web_animation_destroy(animation);
  (void)ui_dom_node_destroy(node);

  return failed;
}

int main(void) {
  int failed = 0;
  failed |= test_web_animation_lifecycle();
  failed |= test_web_animation_reverse();
  failed |= test_scroll_driven_timelines();
  failed |= test_web_animation_extra();
  failed |= test_web_animation_coverage();
  failed |= test_web_animation_more_coverage();
  if (!failed) {
    printf("All Web Animation tests passed.\n");
  }
  return failed;
}
/* clang-format off */
#include "ui_web_animation.h"
#include <stdio.h>
#include <stdlib.h>

/* clang-format on */
extern int g_malloc_fail_countdown;

//... I will write it all in tests/test_ui_web_animation.c directly.
