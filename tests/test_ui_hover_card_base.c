/* clang-format off */
#include "ui_hover_card_base.h"
#include "ui_component.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

struct ui_hover_card_base {
  struct ui_component *component;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

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

  rc = ui_hover_card_base_on_mouse_leave(card, 10.0f, 0.0f);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_hover_card_base_on_mouse_leave(card, 0.0f, 10.0f);
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

  /* Destroy card */
  rc = ui_hover_card_base_destroy(card);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Destroy NULL */
  rc = ui_hover_card_base_destroy(NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Hit the false branch for component == NULL */
  rc = ui_hover_card_base_create(&card);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_component_destroy(card->component);
  if (rc != UI_ERROR_NONE)
    return rc;
  card->component = NULL;

  rc = ui_hover_card_base_on_mouse_enter(card);
  if (rc != UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;

  rc = ui_hover_card_base_on_mouse_leave(card, 0.0f, 0.0f);
  if (rc != UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;

  /* Destroy with component == NULL */
  rc = ui_hover_card_base_destroy(card);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Valid card for binding tests */
  rc = ui_hover_card_base_create(&card);
  if (rc != UI_ERROR_NONE)
    return rc;

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

#ifdef UI_TEST_MOCK_ALLOC
  {
    extern int g_hover_card_mock_fail;
    g_hover_card_mock_fail = 1;
    rc = ui_hover_card_base_create(&card);
    if (rc != UI_ERROR_UNKNOWN)
      return UI_ERROR_UNKNOWN;
    g_hover_card_mock_fail = 0;
  }
#endif

  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_hover_card_base *card = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 50; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_hover_card_base_create(&card);
    if (rc == UI_ERROR_NONE) {
      rc = ui_hover_card_base_destroy(card);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      break;
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
