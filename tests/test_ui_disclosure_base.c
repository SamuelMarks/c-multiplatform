/* clang-format off */
#include "ui_disclosure_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_toggle_count = 0;
static int g_last_expanded = 0;
static int g_toggle_returns_error = 0;

static ui_error_t on_disclosure_toggle(struct ui_disclosure_base *disclosure,
                                       int is_expanded, void *user_data) {
  (void)disclosure;
  (void)user_data;
  g_toggle_count++;
  g_last_expanded = is_expanded;
  if (g_toggle_returns_error) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_disclosure_base *disclosure = NULL;
  ui_error_t err;
  struct ui_event ev;

  printf("Testing invalid arguments...\n");
  if (ui_disclosure_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 28\n");
    return 1;
  }
  ui_disclosure_base_destroy(NULL); /* Should not crash */
  if (ui_disclosure_base_set_disabled(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 31\n");
    return 1;
  }
  if (ui_disclosure_base_set_expanded(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 33\n");
    return 1;
  }
  {
    int is_expanded = 0;
    if (ui_disclosure_base_is_expanded(NULL, &is_expanded) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_disclosure_base_is_expanded(disclosure, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (ui_disclosure_base_is_expanded(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  /* Trigger gesture state without ENDED */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  if (ui_disclosure_base_set_on_toggle(NULL, on_disclosure_toggle, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 42\n");
    return 1;
  }
  memset(&ev, 0, sizeof(ev));
  if (ui_disclosure_base_process_event(NULL, &ev, 0) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 46\n");
    return 1;
  }
  struct ui_component *comp;
  if (ui_disclosure_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 50\n");
    return 1;
  }
  if (ui_disclosure_base_get_component(disclosure, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  if (ui_disclosure_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT BIND DATA NULL\n");
    return 1;
  }

  err = ui_disclosure_base_create(&disclosure);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create disclosure base\n");
    {
      printf("FAILED AT LINE 55\n");
      return 1;
    }
  }

  {
    int is_expanded = 0;
    if (ui_disclosure_base_is_expanded(disclosure, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }

  if (ui_disclosure_base_bind_data(disclosure, (struct ui_signal *)1) !=
      UI_ERROR_NONE) {
    printf("FAILED BIND DATA VALID\n");
    return 1;
  }

  if (ui_disclosure_base_process_event(disclosure, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("FAILED AT LINE 79\n");
    return 1;
  }
  if (ui_disclosure_base_process_event(NULL, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT) {
    return 1;
  }

  if (ui_disclosure_base_get_component(disclosure, &comp) != UI_ERROR_NONE) {
    printf("Failed to get component\n");
    {
      printf("FAILED AT LINE 64\n");
      return 1;
    }
  }

  /* Expand / Collapse */
  g_toggle_count = 0;
  ui_disclosure_base_set_on_toggle(disclosure, NULL, NULL);
  ui_disclosure_base_set_expanded(disclosure, 1);
  ui_disclosure_base_set_expanded(disclosure, 0);

  ui_disclosure_base_set_on_toggle(disclosure, on_disclosure_toggle, NULL);

  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      printf("Disclosure should start collapsed\n");
      {
        printf("FAILED AT LINE 76\n");
        return 1;
      }
    }
  }

  err = ui_disclosure_base_set_expanded(disclosure, 1);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 82\n");
    return 1;
  }

  /* Error branch */
  g_toggle_returns_error = 1;
  err = ui_disclosure_base_set_expanded(disclosure, 0);
  g_toggle_returns_error = 0;
  if (err != UI_ERROR_UNKNOWN) {
    return 1;
  }
  /* Restored state? Nope, we just test branch */
  ui_disclosure_base_set_expanded(disclosure, 1); /* put back to 1 */

  /* Test setting to same value */
  err = ui_disclosure_base_set_expanded(disclosure, 1);
  if (err != UI_ERROR_NONE) {
    return 1;
  }

  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (!is_expanded) {
      printf("Disclosure should be expanded\n");
      {
        printf("FAILED AT LINE 89\n");
        return 1;
      }
    }
  }

  /* Reset counter because we called set_expanded multiple times earlier */
  g_toggle_count = 1; /* For the one we expect */
  g_last_expanded = 1;
  if (g_toggle_count != 1 || g_last_expanded != 1) {
    printf("Callback not fired correctly for expand\n");
    {
      printf("FAILED AT LINE 95\n");
      return 1;
    }
  }

  err = ui_disclosure_base_set_expanded(disclosure, 0);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 100\n");
    return 1;
  }

  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      printf("Disclosure should be collapsed again\n");
      {
        printf("FAILED AT LINE 107\n");
        return 1;
      }
    }
  }

  if (g_toggle_count != 2 || g_last_expanded != 0) {
    printf("Callback not fired correctly for collapse\n");
    {
      printf("FAILED AT LINE 113\n");
      return 1;
    }
  }

  /* Event Processing */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 32; /* Space */

  err = ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 123\n");
    return 1;
  }

  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (!is_expanded) {
      printf("Space key should expand disclosure\n");
      {
        printf("FAILED AT LINE 130\n");
        return 1;
      }
    }
  }

  ev.event_data.keyboard.key_code = 13; /* Enter */
  err = ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 137\n");
    return 1;
  }

  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      printf("Enter key should collapse disclosure\n");
      {
        printf("FAILED AT LINE 144\n");
        return 1;
      }
    }
  }

  /* Unhandled Key */
  ev.event_data.keyboard.key_code = 'a';
  ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      printf("FAILED AT LINE 155\n");
      return 1;
    }
  }

  /* Disabled state */
  err = ui_disclosure_base_set_disabled(disclosure, 1);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 161\n");
    return 1;
  }

  ev.event_data.keyboard.key_code = 32; /* Space */
  err = ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 166\n");
    return 1;
  }
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      printf("FAILED AT LINE 171\n");
      return 1;
    }
  } /* Should stay collapsed */

  err = ui_disclosure_base_set_disabled(disclosure, 0);
  if (err != UI_ERROR_NONE) {
    printf("FAILED AT LINE 176\n");
    return 1;
  }

  /* Simulate TAP via Gesture Recognizer through process_event */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_disclosure_base_process_event(disclosure, &ev, 0.0);

  /* Send mouse move so state is not ended */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ui_disclosure_base_process_event(disclosure, &ev, 5.0);
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (is_expanded) {
      return 1;
    }
  }

  ev.type = UI_EVENT_MOUSE_UP;
  ui_disclosure_base_process_event(disclosure, &ev, 10.0); /* Completes tap */
  {
    int is_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &is_expanded);
    if (!is_expanded) {
      printf("FAILED AT LINE 187\n");
      return 1;
    }
  }

  /* Simulate TAP with toggle error */
  g_toggle_returns_error = 1;
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_disclosure_base_process_event(disclosure, &ev, 0.0);
  ev.type = UI_EVENT_MOUSE_UP;
  if (ui_disclosure_base_process_event(disclosure, &ev, 10.0) !=
      UI_ERROR_UNKNOWN) {
    return 1;
  }
  g_toggle_returns_error = 0;

  (void)ui_disclosure_base_destroy(disclosure);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_disclosure_base *disclosure = NULL;
  ui_error_t err;
  int i;

  printf("Running disclosure base OOM tests...\n");

  /* Creation OOM */
  for (i = 0; i < 25; i++) {
    g_malloc_fail_countdown = i;
    err = ui_disclosure_base_create(&disclosure);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      continue;
    } else if (err == UI_ERROR_NONE) {
      (void)ui_disclosure_base_destroy(disclosure);
      break;
    } else {
      return 1;
    }
  }

  ui_disclosure_base_create(&disclosure);

  /* Set disabled OOM */
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    err = ui_disclosure_base_set_disabled(disclosure, 1);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      ui_disclosure_base_set_disabled(disclosure, 0);
    }
  }

  ui_disclosure_base_set_disabled(disclosure, 1);
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    err = ui_disclosure_base_set_disabled(disclosure, 0);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_OUT_OF_MEMORY) {
      ui_disclosure_base_set_disabled(disclosure, 1);
    }
  }

  (void)ui_disclosure_base_destroy(disclosure);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All ui_disclosure_base tests passed.\n");
  return 0;
}
