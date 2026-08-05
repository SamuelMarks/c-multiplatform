/* clang-format off */
#include "../include/ui_coachmark_base.h"
#include "../include/ui_error.h"
#include "../include/ui_overlay_director.h"
#include "../include/ui_event.h"
#include "../include/ui_dom_node.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t dummy_on_step_change(struct ui_coachmark_tour *tour,
                                       int current_step, void *user_data) {
  (void)tour;
  (void)current_step;
  if (user_data)
    return UI_ERROR_INVALID_ARGUMENT; /* simulate error */
  return UI_ERROR_NONE;
}

static int test_coachmark(void) {
  struct ui_coachmark_tour *tour = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_coachmark_step steps[2];
  struct ui_event ev;
  struct ui_computed *anim = NULL;

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);

  if (ui_coachmark_tour_create(director, &tour) != UI_ERROR_NONE) {
    printf("create fail\n");
    return 1;
  }

  memset(&steps, 0, sizeof(steps));
  steps[0].allow_skip = 1;
  steps[1].allow_skip = 0;

  if (ui_coachmark_tour_set_steps(tour, steps, 2) != UI_ERROR_NONE) {
    printf("set_steps fail\n");
    return 1;
  }
  if (ui_coachmark_tour_set_on_step_change(tour, dummy_on_step_change, NULL) !=
      UI_ERROR_NONE) {
    printf("set_on_step_change fail\n");
    return 1;
  }

  ui_coachmark_tour_bind_open(tour, NULL);
  ui_coachmark_tour_get_animating_signal(tour, &anim);

  /* Start */
  if (ui_coachmark_tour_start(tour) != UI_ERROR_NONE) {
    printf("start fail\n");
    return 1;
  }
  ui_coachmark_tour_start(tour); /* Already active */

  /* Next */
  ui_coachmark_tour_next(tour);
  ui_coachmark_tour_prev(tour);
  ui_coachmark_tour_next(tour);
  ui_coachmark_tour_next(tour); /* Should skip */

  ui_coachmark_tour_start(tour);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_coachmark_tour_process_event(tour, &ev); /* Skip */

  ui_coachmark_tour_update_layout(tour, 800, 600);

  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_skip(tour);

  /* Set up open signal for coverage */
  struct ui_arena *arena;
  struct ui_signal *sig;
  union ui_signal_payload val;
  val.bool_val = 0;
  ui_arena_create(1024, &arena);
  ui_signal_create(arena, val, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  ui_coachmark_tour_bind_open(tour, sig);

  /* Trigger start and skip to test open_signal branches */
  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_skip(tour);

  /* Test branching inside process_event */
  ui_coachmark_tour_process_event(tour, &ev); /* Not active */

  ui_coachmark_tour_start(tour);

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_coachmark_tour_process_event(tour, &ev); /* Not ESCAPE */

  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_coachmark_tour_process_event(tour, &ev); /* Not KEY_DOWN */

  ui_coachmark_tour_next(tour); /* Go to step 1 where allow_skip = 0 */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_coachmark_tour_process_event(tour, &ev); /* ESCAPE but skip not allowed */

  /* Test invalid steps */
  ui_coachmark_tour_set_steps(tour, NULL, 5);

  ui_coachmark_tour_skip(tour); /* Skips before removing steps to avoid crash */

  /* Test setting steps to NULL (count 0) to hit free */
  ui_coachmark_tour_set_steps(tour, NULL, 0);
  ui_coachmark_tour_start(tour); /* Hits step_count == 0 branch */

  ui_coachmark_tour_set_steps(tour, steps, 2); /* Reset valid steps */

  /* Test missing on_step_change */
  ui_coachmark_tour_set_on_step_change(tour, NULL, NULL);

  /* Start and call prev at step 0 */
  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_prev(tour); /* Renders step 0 without callback, and hits
                                   false branch for current_step > 0 */

  ui_coachmark_tour_skip(tour); /* Skips without callback */

  /* Trigger error in callback */
  ui_coachmark_tour_set_on_step_change(tour, dummy_on_step_change, (void *)1);
  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_skip(tour);

  ui_coachmark_tour_set_on_step_change(tour, NULL, NULL);
  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_set_on_step_change(tour, dummy_on_step_change, (void *)1);
  ui_coachmark_tour_next(tour);

  ui_coachmark_tour_set_on_step_change(tour, NULL, NULL);
  ui_coachmark_tour_start(tour);
  ui_coachmark_tour_next(tour);
  ui_coachmark_tour_set_on_step_change(tour, dummy_on_step_change, (void *)1);
  ui_coachmark_tour_prev(tour);
  ui_coachmark_tour_skip(tour);

  ui_coachmark_tour_skip(tour); /* Skips without callback */

  ui_coachmark_tour_next(tour); /* Hits !is_active true branch */
  ui_coachmark_tour_prev(tour); /* Hits !is_active true branch */

  ui_coachmark_tour_bind_open(tour, NULL);
  (void)ui_signal_destroy(sig);
  (void)ui_arena_destroy(arena);

  /* Add dummy content component to test node appending */
  {
    struct ui_component dummy_comp;
    struct ui_component dummy_comp2;
    memset(&dummy_comp, 0, sizeof(dummy_comp));
    memset(&dummy_comp2, 0, sizeof(dummy_comp2));
    steps[0].content_component = &dummy_comp;
    steps[1].content_component = &dummy_comp2;

    struct ui_dom_node *dummy_root;
    if (ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_root) ==
        UI_ERROR_NONE) {
      dummy_comp.shadow_root = dummy_root;
      dummy_comp2.shadow_root = NULL; /* Hits the NULL shadow root branch */
      ui_coachmark_tour_set_steps(tour, steps, 2);
      ui_coachmark_tour_start(tour);
      ui_coachmark_tour_update_layout(tour, 800, 600); /* Layout while active */
      ui_coachmark_tour_next(tour); /* Renders step 1 with NULL shadow root */
      ui_coachmark_tour_skip(tour);
      /* dummy_root is destroyed by the tour's container destruction, do not
       * double-free it */
      dummy_comp.shadow_root = NULL;
    }
  }

  ui_coachmark_tour_destroy(tour);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);

  /* Test NULLs */
  ui_coachmark_tour_create(NULL, NULL);
  ui_coachmark_tour_create(director, NULL);
  ui_coachmark_tour_destroy(NULL);
  ui_coachmark_tour_set_steps(NULL, NULL, 0);
  ui_coachmark_tour_set_on_step_change(NULL, NULL, NULL);
  ui_coachmark_tour_start(NULL);
  ui_coachmark_tour_next(NULL);
  ui_coachmark_tour_prev(NULL);
  ui_coachmark_tour_skip(NULL);
  ui_coachmark_tour_update_layout(NULL, 0, 0);
  ui_coachmark_tour_process_event(NULL, NULL);
  ui_coachmark_tour_bind_open(NULL, NULL);
  ui_coachmark_tour_get_animating_signal(NULL, NULL);

  /* Valid tour, NULL other args */
  ui_coachmark_tour_create(director, &tour);
  ui_coachmark_tour_process_event(tour, NULL);
  ui_coachmark_tour_get_animating_signal(tour, NULL);
  ui_coachmark_tour_destroy(tour);

  /* Test malloc fails */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  {
    int i;
    for (i = 0; i < 30; i++) {
      struct ui_coachmark_tour *temp_tour = NULL;
      g_malloc_fail_countdown = i;
      if (ui_coachmark_tour_create(director, &temp_tour) != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        if (temp_tour)
          ui_coachmark_tour_destroy(temp_tour);
        continue;
      }
      g_malloc_fail_countdown = -1;
      ui_coachmark_tour_destroy(temp_tour);
      break;
    }
    for (i = 0; i < 30; i++) {
      struct ui_coachmark_tour *temp_tour = NULL;
      ui_coachmark_tour_create(director, &temp_tour);
      g_malloc_fail_countdown = i;
      if (ui_coachmark_tour_set_steps(temp_tour, steps, 2) != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        ui_coachmark_tour_destroy(temp_tour);
        continue;
      }
      g_malloc_fail_countdown = -1;
      ui_coachmark_tour_destroy(temp_tour);
      break;
    }
    for (i = 0; i < 30; i++) {
      struct ui_coachmark_tour *temp_tour = NULL;
      ui_coachmark_tour_create(director, &temp_tour);
      ui_coachmark_tour_set_steps(temp_tour, steps, 2);
      g_malloc_fail_countdown = i;
      if (ui_coachmark_tour_start(temp_tour) != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        ui_coachmark_tour_destroy(temp_tour);
        continue;
      }
      g_malloc_fail_countdown = -1;
      ui_coachmark_tour_destroy(temp_tour);
      break;
    }
    for (i = 0; i < 50; i++) {
      struct ui_coachmark_tour *temp_tour = NULL;
      ui_coachmark_tour_create(director, &temp_tour);
      ui_coachmark_tour_set_steps(temp_tour, steps, 2);
      ui_coachmark_tour_start(temp_tour);
      g_malloc_fail_countdown = i;
      if (ui_coachmark_tour_next(temp_tour) != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        ui_coachmark_tour_destroy(temp_tour);
        continue;
      }
      g_malloc_fail_countdown = -1;
      ui_coachmark_tour_destroy(temp_tour);
      break;
    }
    for (i = 0; i < 50; i++) {
      struct ui_coachmark_tour *temp_tour = NULL;
      ui_coachmark_tour_create(director, &temp_tour);
      ui_coachmark_tour_set_steps(temp_tour, steps, 2);
      ui_coachmark_tour_start(temp_tour);
      ui_coachmark_tour_next(temp_tour);
      g_malloc_fail_countdown = i;
      if (ui_coachmark_tour_prev(temp_tour) != UI_ERROR_NONE) {
        g_malloc_fail_countdown = -1;
        ui_coachmark_tour_destroy(temp_tour);
        continue;
      }
      g_malloc_fail_countdown = -1;
      ui_coachmark_tour_destroy(temp_tour);
      break;
    }
    g_malloc_fail_countdown = -1;
  }

  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_coachmark();
  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
