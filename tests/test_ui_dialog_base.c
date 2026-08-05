/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include "../include/ui_dialog_base.h"
#include "../include/ui_error.h"
#include "../src/ui_internal_mem.h"
#include "../include/ui_component.h"
#include "../include/ui_overlay_director.h"
#include "../include/ui_focus_manager.h"
#include "../include/ui_event.h"
#include "../include/ui_signal.h"
#include "../include/ui_computed.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_on_close(struct ui_dialog_base *dialog,
                                void *user_data) {
  int *called = (int *)user_data;
  (void)dialog;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_create_destroy(void) {
  struct ui_dialog_base *dialog = NULL;
  ui_error_t rc;

  rc = ui_dialog_base_create(&dialog);
  if (rc != UI_ERROR_NONE || !dialog)
    return 1;

  (void)ui_dialog_base_destroy(dialog);
  return 0;
}

static int test_errors(void) {
  struct ui_dialog_base *dialog = NULL;
  struct ui_component *comp = (struct ui_component *)1;
  struct ui_overlay_director *director = (struct ui_overlay_director *)2;
  struct ui_focus_manager *focus_manager = (struct ui_focus_manager *)3;
  struct ui_event event;
  int is_open;
  struct ui_signal *signal = (struct ui_signal *)4;
  struct ui_computed *computed;

  if (ui_dialog_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  (void)ui_dialog_base_destroy(NULL);

  if (ui_dialog_base_set_content(NULL, comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_set_overlay_director(NULL, director) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_set_focus_manager(NULL, focus_manager) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_set_open(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_is_open(NULL, &is_open) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_is_open((struct ui_dialog_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_set_on_close(NULL, mock_on_close, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_process_event(NULL, &event, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_process_event((struct ui_dialog_base *)1, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_get_component((struct ui_dialog_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_bind_open(NULL, signal) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_get_animating_signal(NULL, &computed) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_dialog_base_get_animating_signal((struct ui_dialog_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  int i;
  for (i = 0; i < 15; i++) {
    g_malloc_fail_countdown = i;
    if (ui_dialog_base_create(&dialog) == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      (void)ui_dialog_base_destroy(dialog);
    }
  }
  g_malloc_fail_countdown = -1;

  return 0;
}

static int test_getters_and_setters(void) {
  struct ui_dialog_base *dialog = NULL;
  struct ui_component *comp = NULL;
  struct ui_component *content = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_focus_manager *focus_manager = NULL;
  struct ui_dom_node *root_node = NULL;
  struct ui_signal *signal = (struct ui_signal *)4;
  struct ui_computed *computed = NULL;
  int is_open = -1;
  ui_error_t rc;
  struct ui_event event = {0};

  ui_dialog_base_create(&dialog);
  ui_component_create(&content);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  ui_overlay_director_create(root_node, &director);
  ui_focus_manager_create(&focus_manager);

  rc = ui_dialog_base_get_component(dialog, &comp);
  if (rc != UI_ERROR_NONE || comp == NULL)
    return 1;

  rc = ui_dialog_base_set_content(dialog, content);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_dialog_base_set_content(dialog, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dialog_base_set_overlay_director(dialog, director);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dialog_base_set_focus_manager(dialog, focus_manager);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dialog_base_bind_open(dialog, signal);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dialog_base_get_animating_signal(dialog, &computed);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_dialog_base_is_open(dialog, &is_open);
  if (rc != UI_ERROR_NONE || is_open != 0)
    return 1;

  /* Trigger early return by calling process_event while closed */
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  ui_dialog_base_set_open(dialog, 1);

  rc = ui_dialog_base_is_open(dialog, &is_open);
  if (rc != UI_ERROR_NONE || is_open != 1)
    return 1;

  /* Calling set_open with same value */
  rc = ui_dialog_base_set_open(dialog, 1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Calling process_event when open */
  int called = 0;

  rc = ui_dialog_base_set_on_close(dialog, mock_on_close, &called);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Mock should dismiss by sending an event outside bounds */
  event.type = UI_EVENT_MOUSE_DOWN;
  event.event_data.mouse.x = 100;
  event.event_data.mouse.y = 100;
  event.event_data.mouse.button = 0; /* Left */
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Now send pointer up to complete the click */
  event.type = UI_EVENT_MOUSE_UP;
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (!called)
    return 1; /* Was dismissed */

  /* Mock should not dismiss by clicking inside bounds - but wait! The dialog
     passes 0,0,0,0 as bounds. So all clicks are outside. To make it NOT
     dismiss, send an event that is NOT a click, like mouse move. */
  called = 0;
  event.type = UI_EVENT_MOUSE_MOVE;
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (called)
    return 1;

  /* Remove on_close callback and trigger dismiss to test default close */
  ui_dialog_base_set_on_close(dialog, NULL, NULL);
  event.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  event.type = UI_EVENT_MOUSE_UP;
  rc = ui_dialog_base_process_event(dialog, &event, 0);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_dialog_base_is_open(dialog, &is_open);
  if (rc != UI_ERROR_NONE || is_open != 0)
    return 1;

  /* Mock OOM for overlay mount */
  g_malloc_fail_countdown = 0;
  rc = ui_dialog_base_set_open(dialog, 1);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Mock OOM for push trap */
  /* overlay mount takes about 4 allocs typically, let's just use a loop to hit
   * the trap error */
  int hit_rollback = 0;
  int i;
  for (i = 1; i <= 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_dialog_base_set_open(dialog, 1);
    g_malloc_fail_countdown = -1;
    if (rc == UI_ERROR_NONE) {
      ui_dialog_base_set_open(dialog, 0);
      break;
    } else if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* Reset state since it failed */
      ui_dialog_base_set_open(dialog, 0);
    }
  }

  /* Open again successfully */
  ui_dialog_base_set_open(dialog, 1);

  /* Leave it open to test destroy while open */
  (void)ui_dialog_base_destroy(dialog);
  (void)ui_component_destroy(content);
  (void)ui_overlay_director_destroy(director);
  (void)ui_dom_node_destroy(root_node);
  (void)ui_focus_manager_destroy(focus_manager);
  return 0;
}

static int test_open_without_director_and_focus(void) {
  struct ui_dialog_base *dialog = NULL;
  struct ui_focus_manager *focus_manager = NULL;
  struct ui_overlay_director *director = NULL;
  ui_dialog_base_create(&dialog);

  /* Missing director and focus_manager should just set is_open flag */
  ui_dialog_base_set_open(dialog, 1);
  ui_dialog_base_set_open(dialog, 0);

  /* Set only director */
  ui_overlay_director_create(NULL, &director);
  ui_dialog_base_set_overlay_director(dialog, director);
  g_malloc_fail_countdown = 0;
  ui_dialog_base_set_open(dialog, 1);
  g_malloc_fail_countdown = -1;
  ui_dialog_base_set_open(dialog, 1);
  ui_dialog_base_set_open(dialog, 0);
  ui_dialog_base_set_open(dialog, 1);
  (void)ui_dialog_base_destroy(dialog);
  (void)ui_overlay_director_destroy(director);

  /* Set only focus */
  ui_dialog_base_create(&dialog);
  ui_focus_manager_create(&focus_manager);
  ui_dialog_base_set_focus_manager(dialog, focus_manager);
  int j;
  for (j = 1; j <= 20; j++) {
    g_malloc_fail_countdown = j;
    if (ui_dialog_base_set_open(dialog, 1) == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      ui_dialog_base_set_open(dialog, 0);
      break;
    }
    g_malloc_fail_countdown = -1;
    ui_dialog_base_set_open(dialog, 0); /* reset state */
  }
  ui_dialog_base_set_open(dialog, 1);
  ui_dialog_base_set_open(dialog, 0);
  ui_dialog_base_set_open(dialog, 1);
  (void)ui_dialog_base_destroy(dialog);
  (void)ui_focus_manager_destroy(focus_manager);
  ui_dialog_base_create(&dialog);
  ui_overlay_director_create(NULL, &director);
  ui_dialog_base_set_overlay_director(dialog, director);
  (void)ui_dialog_base_destroy(dialog);
  (void)ui_overlay_director_destroy(director);

  return 0;
}

static int test_oom_open(void) {
  struct ui_dialog_base *dialog = NULL;
  struct ui_overlay_director *director = NULL;
  struct ui_focus_manager *focus_manager = NULL;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;
  int i;

  for (i = 0; i < 20; i++) {
    ui_dialog_base_create(&dialog);
    ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
    ui_overlay_director_create(root_node, &director);
    ui_focus_manager_create(&focus_manager);

    ui_dialog_base_set_overlay_director(dialog, director);
    ui_dialog_base_set_focus_manager(dialog, focus_manager);

    g_malloc_fail_countdown = i;
    rc = ui_dialog_base_set_open(dialog, 1);
    g_malloc_fail_countdown = -1;

    (void)ui_dialog_base_destroy(dialog);
    (void)ui_focus_manager_destroy(focus_manager);
    (void)ui_overlay_director_destroy(director);
    (void)ui_dom_node_destroy(root_node);
  }

  for (i = 0; i < 5; i++) {
    ui_dialog_base_create(&dialog);
    ui_focus_manager_create(&focus_manager);
    ui_dialog_base_set_focus_manager(dialog, focus_manager);
    g_malloc_fail_countdown = i;
    rc = ui_dialog_base_set_open(dialog, 1);
    g_malloc_fail_countdown = -1;
    (void)ui_dialog_base_destroy(dialog);
    (void)ui_focus_manager_destroy(focus_manager);
  }
  return 0;
}
int main(void) {
  int failed = 0;
  failed |= test_create_destroy();
  failed |= test_errors();
  failed |= test_getters_and_setters();
  failed |= test_open_without_director_and_focus();
  failed |= test_oom_open();

  if (failed) {
    printf("Tests failed\n");
    return 1;
  }
  printf("All tests passed\n");
  return 0;
}
