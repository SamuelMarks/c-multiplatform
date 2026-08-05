/* clang-format off */
#include "ui_select_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_count = 0;
static int g_last_selected_index = -1;
static int g_open_change_count = 0;
static int g_last_open_state = 0;

static ui_error_t on_select_change(struct ui_select_base *select, int index,
                                   void *user_data) {
  (void)select;
  (void)user_data;
  g_change_count++;
  g_last_selected_index = index;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t on_select_open_change(struct ui_select_base *select,
                                        int is_open, void *user_data) {
  (void)select;
  (void)user_data;
  g_open_change_count++;
  g_last_open_state = is_open;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_select_base *select = NULL;
  ui_error_t err;
  struct ui_event ev;

  printf("Testing invalid arguments...\n");
  if (ui_select_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_select_base_destroy(NULL); /* Should not crash */
  if (ui_select_base_set_disabled(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_set_item_count(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_set_open(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    int is_open = 0;
    if (ui_select_base_is_open(NULL, &is_open) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  if (ui_select_base_set_highlighted_index(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    int index = 0;
    if (ui_select_base_get_highlighted_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  if (ui_select_base_set_selected_index(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    int index = 0;
    if (ui_select_base_get_selected_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  if (ui_select_base_set_on_change(NULL, on_select_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_set_on_open_change(NULL, on_select_open_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  memset(&ev, 0, sizeof(ev));
  if (ui_select_base_process_event(NULL, &ev, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  {
    struct ui_component *tmp_comp;
    if (ui_select_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
  }

  err = ui_select_base_create(&select);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create select base\n");
    return 1;
  }

  if (ui_select_base_add_option(NULL, "a", "1") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_add_option(select, NULL, "1") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_add_option(select, "Label", "val") != UI_ERROR_NONE)
    return 1;
  ui_select_base_set_item_count(
      select, 0); /* reset count so subsequent index checks pass */

  if (ui_select_base_process_event(select, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    struct ui_component *tmp_comp;
    if (ui_select_base_get_component(select, &tmp_comp) != UI_ERROR_NONE ||
        tmp_comp == NULL) {
      printf("Failed to get component\n");
      return 1;
    }
  }

  g_open_change_count = 0;
  g_last_open_state = 0;

  err = ui_select_base_set_on_open_change(select, on_select_open_change, NULL);
  if (err != UI_ERROR_NONE)
    return 1;

  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0) {
      printf("Select should be closed by default\n");
      return 1;
    }
  }

  ui_select_base_set_open(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 1) {
      printf("Select should be open\n");
      return 1;
    }
  }

  if (g_open_change_count != 1 || g_last_open_state != 1) {
    printf("Open change callback failed\n");
    return 1;
  }

  ui_select_base_set_disabled(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0) {
      printf("Select should close when disabled\n");
      return 1;
    }
  }

  /* Set open while disabled shouldn't work */
  ui_select_base_set_open(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0)
      return 1;
  }

  ui_select_base_set_disabled(select, 0);

  /* Keyboard navigation */
  g_change_count = 0;
  g_last_selected_index = -1;

  ui_select_base_set_on_change(select, on_select_change, NULL);

  /* Test index clamping before items added */
  ui_select_base_set_highlighted_index(select, 5);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != -1)
      return 1;
  }
  ui_select_base_set_selected_index(select, 5);
  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != -1)
      return 1;
  }

  ui_select_base_set_item_count(select, -1); /* Negative count -> 0 */
  ui_select_base_set_item_count(select, 3);  /* 3 items */

  /* Test bounds adjustment when shrinking */
  ui_select_base_set_highlighted_index(select, 2);
  ui_select_base_set_selected_index(select, 2);
  ui_select_base_set_item_count(select, 2);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 1)
      return 1;
  }
  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != 1)
      return 1;
  }

  ui_select_base_set_item_count(select, 3);

  /* Open the select via space */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  ui_select_base_process_event(select, &ev, 0.0);

  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (!is_open) {
      printf("Select should open on Space\n");
      return 1;
    }
  }

  /* Open again when already open - should process inner keys */
  /* Close with Escape */
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open)
      return 1;
  }

  /* Open with Enter */
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (!is_open)
      return 1;
  }

  /* Highlight down */
  ui_select_base_set_highlighted_index(select, 0);
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_select_base_process_event(select, &ev, 0.0);

  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 1) {
      printf("Highlighted index should be 1 (was 0 on open), got %d\n", index);
      return 1;
    }
  }

  /* Down past end clamps to last */
  ui_select_base_set_highlighted_index(select, 2);
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 2)
      return 1;
  }

  /* Up */
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 1)
      return 1;
  }

  /* Up past beginning clamps to 0 */
  ui_select_base_set_highlighted_index(select, 0);
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 0)
      return 1;
  }

  /* End */
  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 2)
      return 1;
  }

  /* Home */
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 0)
      return 1;
  }

  /* Highlight a specific index for enter test */
  ui_select_base_set_highlighted_index(select, 1);

  /* Confirm selection with Enter */
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_select_base_process_event(select, &ev, 0.0);

  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open) {
      printf("Select should close on Enter\n");
      return 1;
    }
  }

  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != 1) {
      printf("Selected index should be 1, got %d\n", index);
      return 1;
    }
  }

  /* Open with UP/DOWN keys when closed */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (!is_open)
      return 1;
  }

  /* Unhandled key when open */
  ev.event_data.keyboard.key_code = 'a';
  ui_select_base_process_event(select, &ev, 0.0);

  /* Process event when disabled */
  ui_select_base_set_disabled(select, 1);
  ui_select_base_process_event(select, &ev, 0.0);

  (void)ui_select_base_destroy(select);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_select_base *select = NULL;
  ui_error_t err;
  int i;

  printf("Running select base OOM tests...\n");

  /* Creation OOM */
  for (i = 0; i < 400; i++) {
    g_malloc_fail_countdown = i;
    err = ui_select_base_create(&select);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      (void)ui_select_base_destroy(select);
      break;
    }
  }

  if (i == 400)
    return 1;

  ui_select_base_create(&select);

  /* Add option OOM */
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    err = ui_select_base_add_option(select, "L", "V");
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      break;
    }
  }

  if (i == 100)
    return 1;

  (void)ui_select_base_destroy(select);
  return 0;
}
static ui_error_t mock_cva_callback(union ui_signal_payload p,
                                    void *user_data) {
  int *called = (int *)user_data;
  *called = p.int_val;
  return UI_ERROR_NONE;
}

static ui_error_t mock_cva_touched(void *user_data) {
  int *called = (int *)user_data;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_cva_and_edge_cases(void) {
  struct ui_select_base *select = NULL;
  struct ui_control_value_accessor cva;
  union ui_signal_payload p;
  struct ui_event ev;
  int cva_change_called = -1;
  int cva_touched_called = 0;

  if (ui_select_base_create(&select) != UI_ERROR_NONE)
    return 1;

  /* CVA */
  if (ui_select_base_get_cva(NULL, &cva) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_get_cva(select, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_select_base_get_cva(select, &cva) != UI_ERROR_NONE)
    return 1;

  if (cva.register_on_change(NULL, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  /* Test CVA OOM */
  g_malloc_fail_countdown = 0;
  if (cva.register_on_change(select, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  if (cva.register_on_change(select, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_NONE)
    return 1;
  if (cva.register_on_touched(select, mock_cva_touched, &cva_touched_called) !=
      UI_ERROR_NONE)
    return 1;

  p.int_val = 0;
  cva.write_value(select, p);
  cva.set_disabled_state(select, 1);
  cva.set_disabled_state(select, 0);

  /* Add an option to trigger change */
  ui_select_base_add_option(select, "1", "Option 1");
  ui_select_base_set_selected_index(select, 0); /* triggers wrapper */
  if (cva_change_called != 0)
    return 1;

  /* Edge cases for indices */
  ui_select_base_set_highlighted_index(select, -2);
  ui_select_base_set_selected_index(select, -2);
  ui_select_base_set_highlighted_index(select, 10);
  ui_select_base_set_selected_index(select, 10);

  /* Empty options limit */
  {
    struct ui_select_base *empty_select = NULL;
    ui_select_base_create(&empty_select);
    ui_select_base_set_highlighted_index(empty_select, 0);
    ui_select_base_set_selected_index(empty_select, 0);
    (void)ui_select_base_destroy(empty_select);
  }

  /* Test ESCAPE and UP/DOWN key */
  ui_select_base_set_open(select, 0);
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_select_base_process_event(select, &ev, 0.0); /* opens */

  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_select_base_process_event(select, &ev, 0.0); /* closes */

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_select_base_process_event(select, &ev, 0.0); /* opens */

  /* Unhandled key when closed */
  ui_select_base_set_open(select, 0);
  ev.event_data.keyboard.key_code = 0; /* Unhandled */
  ui_select_base_process_event(select, &ev, 0.0);

  (void)ui_select_base_destroy(select);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_cva_and_edge_cases();
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All ui_select_base tests passed.\n");
  return 0;
}
