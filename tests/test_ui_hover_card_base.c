/* clang-format off */
#include "ui_hover_card_base.h"
#include "ui_component.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t run_normal_tests(void) {
  struct ui_hover_card_base *card = NULL;
  struct ui_component *comp = NULL;
  struct ui_computed *computed = NULL;
  ui_error_t rc;

  printf("Testing ui_hover_card_base_create...\n");

  /* NULL out param */
  rc = ui_hover_card_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT) {
    return UI_ERROR_UNKNOWN;
  }

  /* Successful creation */
  rc = ui_hover_card_base_create(&card);
  if (rc != UI_ERROR_NONE || !card) {
    return rc;
  }

  printf("Testing ui_hover_card_base_get_component...\n");
  rc = ui_hover_card_base_get_component(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_hover_card_base_get_component(card, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_get_component(card, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return rc;

  printf("Testing ui_hover_card_base_on_mouse_enter...\n");
  rc = ui_hover_card_base_on_mouse_enter(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_on_mouse_enter(card);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Hit the false branch in on_mouse_enter (shadow_root == NULL) */
  {
    struct ui_dom_node *saved_root = comp->shadow_root;
    comp->shadow_root = NULL;
    rc = ui_hover_card_base_on_mouse_enter(card);
    if (rc != UI_ERROR_NONE)
      return UI_ERROR_UNKNOWN;
    comp->shadow_root = saved_root;
  }

  printf("Testing ui_hover_card_base_on_mouse_leave...\n");
  rc = ui_hover_card_base_on_mouse_leave(NULL, 0.0f, 0.0f);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_on_mouse_leave(card, 0.0f, 0.0f);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Hit the false branch in on_mouse_leave (shadow_root == NULL) */
  {
    struct ui_dom_node *saved_root = comp->shadow_root;
    comp->shadow_root = NULL;
    rc = ui_hover_card_base_on_mouse_leave(card, 0.0f, 0.0f);
    if (rc != UI_ERROR_NONE)
      return UI_ERROR_UNKNOWN;
    comp->shadow_root = saved_root;
  }

  /* Hit the false branch for component == NULL */
  {
    struct mock_card {
      void *comp;
      void *sig1;
      void *sig2;
    };
    struct mock_card m;
    m.comp = NULL;
    m.sig1 = NULL;
    m.sig2 = NULL;
    rc = ui_hover_card_base_on_mouse_enter((struct ui_hover_card_base *)&m);
    if (rc != UI_ERROR_NONE)
      return UI_ERROR_UNKNOWN;
    rc = ui_hover_card_base_on_mouse_leave((struct ui_hover_card_base *)&m,
                                           0.0f, 0.0f);
    if (rc != UI_ERROR_NONE)
      return UI_ERROR_UNKNOWN;
  }

  printf("Testing ui_hover_card_base_bind_open...\n");
  rc = ui_hover_card_base_bind_open(NULL, (struct ui_signal *)1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_bind_open(card, (struct ui_signal *)1);
  if (rc != UI_ERROR_NONE)
    return rc;

  printf("Testing ui_hover_card_base_get_animating_signal...\n");
  rc = ui_hover_card_base_get_animating_signal(NULL, &computed);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_hover_card_base_get_animating_signal(card, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_get_animating_signal(card, &computed);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_hover_card_base_destroy(card);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Destroy NULL */
  rc = ui_hover_card_base_destroy(NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* We also need to cover the if (hover_card->component) false branch in
   * destroy. Since we can't easily do it normally, we simulate it by casting.
   */
  {
    struct mock_card {
      void *comp;
      void *sig1;
      void *sig2;
    };
    /* We can malloc it ourselves */
    struct mock_card *m = (struct mock_card *)malloc(sizeof(struct mock_card));
    if (m) {
      m->comp = NULL;
      m->sig1 = NULL;
      m->sig2 = NULL;
      (void)ui_hover_card_base_destroy((struct ui_hover_card_base *)m);
    }
  }

  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_hover_card_base *card = NULL;
  ui_error_t rc;
  int i;

  /* There are multiple allocations/creations in ui_hover_card_base_create.
     We iterate through countdown values to hit all failure paths.
     50 is large enough to exhaust all allocation paths in creation. */
  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_hover_card_base_create(&card);
    if (rc == UI_ERROR_NONE) {
      /* Reached success. The i-th allocation didn't fail because there are < i
       * allocations. */
      (void)ui_hover_card_base_destroy(card);
      break; /* We've exhausted all error paths. */
    }
  }
  g_malloc_fail_countdown = -1;

  return UI_ERROR_NONE;
}

int main(void) {
  if (run_normal_tests() != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
