/* clang-format off */
#include "ui_hover_card_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

#include "../include/ui_component.h"
#include "../include/ui_dom_node.h"

struct ui_hover_card_base {
  struct ui_component *component;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

static int test_null_args(void) {
  struct ui_hover_card_base *hover_card = NULL;
  struct ui_component *comp = NULL;
  struct ui_computed *comp_sig = NULL;

  if (ui_hover_card_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hover_card_base_destroy(NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_hover_card_base_create(&hover_card) != UI_ERROR_NONE)
    return 1;

  if (ui_hover_card_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hover_card_base_get_component(hover_card, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_hover_card_base_on_mouse_enter(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hover_card_base_on_mouse_leave(NULL, 0.0f, 0.0f) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_hover_card_base_bind_open(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_hover_card_base_get_animating_signal(NULL, &comp_sig) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_hover_card_base_get_animating_signal(hover_card, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Test destroy with NULL component */
  if (hover_card) {
    void *temp_c = hover_card->component;
    hover_card->component = NULL;
    if (ui_hover_card_base_destroy(hover_card) != UI_ERROR_NONE)
      return 1;
    /* We must free temp_c to avoid memory leak since we hijacked it */
    if (ui_component_destroy(temp_c) != UI_ERROR_NONE)
      return 1;
    hover_card = NULL;
  }

  return 0;
}

static int test_normal_lifecycle(void) {
  struct ui_hover_card_base *hover_card = NULL;
  struct ui_component *comp = NULL;
  struct ui_computed *comp_sig = NULL;
  struct ui_signal *open_sig = NULL;

  if (ui_hover_card_base_create(&hover_card) != UI_ERROR_NONE)
    return 1;

  if (ui_hover_card_base_get_component(hover_card, &comp) != UI_ERROR_NONE)
    return 1;
  if (comp == NULL)
    return 1;

  if (ui_hover_card_base_on_mouse_enter(hover_card) != UI_ERROR_NONE)
    return 1;
  if (ui_hover_card_base_on_mouse_leave(hover_card, 10.0f, 20.0f) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_hover_card_base_bind_open(hover_card, open_sig) != UI_ERROR_NONE)
    return 1;
  if (ui_hover_card_base_get_animating_signal(hover_card, &comp_sig) !=
      UI_ERROR_NONE)
    return 1;

  /* Test destroy with NULL component */
  if (hover_card) {
    void *temp_c = hover_card->component;
    hover_card->component = NULL;
    if (ui_hover_card_base_destroy(hover_card) != UI_ERROR_NONE)
      return 1;
    /* We must free temp_c to avoid memory leak since we hijacked it */
    if (ui_component_destroy(temp_c) != UI_ERROR_NONE)
      return 1;
    hover_card = NULL;
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_hover_card_base *hover_card = NULL;
  int i;
  int failed_at_least_once = 0;

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    if (ui_hover_card_base_create(&hover_card) != UI_ERROR_NONE) {
      failed_at_least_once = 1;
    } else {
      if (ui_hover_card_base_destroy(hover_card) != UI_ERROR_NONE)
        return 1;
      break; /* We succeeded, no more allocs to fail */
    }
  }

  g_malloc_fail_countdown = -1;
  if (!failed_at_least_once)
    return 1;
  return 0;
}

static int test_shadow_root_null(void) {
  struct ui_hover_card_base *hover_card = NULL;
  struct ui_component *comp = NULL;

  if (ui_hover_card_base_create(&hover_card) != UI_ERROR_NONE)
    return 1;
  if (ui_hover_card_base_get_component(hover_card, &comp) != UI_ERROR_NONE)
    return 1;

  /* Temporarily remove shadow_root to hit branches */
  if (comp && comp->shadow_root) {
    void *temp = comp->shadow_root;
    comp->shadow_root = NULL;
    if (ui_hover_card_base_on_mouse_enter(hover_card) != UI_ERROR_NONE)
      return 1;
    if (ui_hover_card_base_on_mouse_leave(hover_card, 0.0f, 0.0f) !=
        UI_ERROR_NONE)
      return 1;
    comp->shadow_root = temp;
  }

  /* Test component itself being null */
  if (hover_card) {
    void *temp_c = hover_card->component;
    hover_card->component = NULL;
    if (ui_hover_card_base_on_mouse_enter(hover_card) != UI_ERROR_NONE)
      return 1;
    if (ui_hover_card_base_on_mouse_leave(hover_card, 0.0f, 0.0f) !=
        UI_ERROR_NONE)
      return 1;
    hover_card->component = temp_c;
  }

  /* Test destroy with NULL component */
  if (hover_card) {
    void *temp_c = hover_card->component;
    hover_card->component = NULL;
    if (ui_hover_card_base_destroy(hover_card) != UI_ERROR_NONE)
      return 1;
    /* We must free temp_c to avoid memory leak since we hijacked it */
    if (ui_component_destroy(temp_c) != UI_ERROR_NONE)
      return 1;
    hover_card = NULL;
  }

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_null_args();
  failed |= test_normal_lifecycle();
  failed |= run_oom_tests();
  failed |= test_shadow_root_null();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
