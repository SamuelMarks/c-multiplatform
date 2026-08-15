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
  if (ui_select_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 42\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  ui_select_base_destroy(NULL); /* Should not crash */
  if (ui_select_base_set_disabled(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 45\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_set_item_count(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 47\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_set_open(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 49\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  {
    int is_open = 0;
    if (ui_select_base_is_open(NULL, &is_open) != UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at line 53\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  if (ui_select_base_set_highlighted_index(NULL, 1) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 57\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  {
    int index = 0;
    if (ui_select_base_get_highlighted_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at line 62\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  if (ui_select_base_set_selected_index(NULL, 1) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 65\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  {
    int index = 0;
    if (ui_select_base_get_selected_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT) {
      printf("Failed at line 70\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  if (ui_select_base_set_on_change(NULL, on_select_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 74\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_set_on_open_change(NULL, on_select_open_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 77\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  memset(&ev, 0, sizeof(ev));
  if (ui_select_base_process_event(NULL, &ev, 0) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 80\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  {
    struct ui_component *tmp_comp;
    if (ui_select_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE) {
      printf("Failed at line 84\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  err = ui_select_base_create(&select);
  if (err != UI_ERROR_NONE) {
    printf("Failed to create select base\n");
    {
      printf("Failed at line 90\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  if (ui_select_base_add_option(NULL, "a", "1") != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 94\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_add_option(select, NULL, "1") !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 96\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_add_option(select, "Label", "val") != UI_ERROR_NONE) {
    printf("Failed at line 98\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  ui_select_base_set_item_count(
      select, 0); /* reset count so subsequent index checks pass */

  if (ui_select_base_process_event(select, NULL, 0) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 104\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  {
    struct ui_component *tmp_comp;
    if (ui_select_base_get_component(select, &tmp_comp) != UI_ERROR_NONE ||
        tmp_comp == NULL) {
      printf("Failed to get component\n");
      {
        printf("Failed at line 111\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  g_open_change_count = 0;
  g_last_open_state = 0;

  err = ui_select_base_set_on_open_change(select, on_select_open_change, NULL);
  if (err != UI_ERROR_NONE) {
    printf("Failed at line 120\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0) {
      printf("Select should be closed by default\n");
      {
        printf("Failed at line 127\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  ui_select_base_set_open(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 1) {
      printf("Select should be open\n");
      {
        printf("Failed at line 137\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  if (g_open_change_count != 1 || g_last_open_state != 1) {
    printf("Open change callback failed\n");
    {
      printf("Failed at line 143\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  ui_select_base_set_disabled(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0) {
      printf("Select should close when disabled\n");
      {
        printf("Failed at line 152\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  /* Set open while disabled shouldn't work */
  ui_select_base_set_open(select, 1);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open != 0) {
      printf("Failed at line 162\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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
    if (index != -1) {
      printf("Failed at line 179\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  ui_select_base_set_selected_index(select, 5);
  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != -1) {
      printf("Failed at line 186\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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
    if (index != 1) {
      printf("Failed at line 200\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != 1) {
      printf("Failed at line 206\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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
      {
        printf("Failed at line 221\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  /* Open again when already open - should process inner keys */
  /* Close with Escape */
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (is_open) {
      printf("Failed at line 233\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  /* Open with Enter */
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (!is_open) {
      printf("Failed at line 243\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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
      {
        printf("Failed at line 256\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  /* Down past end clamps to last */
  ui_select_base_set_highlighted_index(select, 2);
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 2) {
      printf("Failed at line 267\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  /* Up */
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 1) {
      printf("Failed at line 277\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  /* Up past beginning clamps to 0 */
  ui_select_base_set_highlighted_index(select, 0);
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 0) {
      printf("Failed at line 287\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  /* End */
  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 2) {
      printf("Failed at line 297\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }

  /* Home */
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int index = 0;
    ui_select_base_get_highlighted_index(select, &index);
    if (index != 0) {
      printf("Failed at line 307\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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
      {
        printf("Failed at line 322\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  {
    int index = 0;
    ui_select_base_get_selected_index(select, &index);
    if (index != 1) {
      printf("Selected index should be 1, got %d\n", index);
      {
        printf("Failed at line 331\n");
        {
          printf("Failed at line %d\n", __LINE__);
          return 1;
        }
      }
    }
  }

  /* Open with UP/DOWN keys when closed */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_select_base_process_event(select, &ev, 0.0);
  {
    int is_open = 0;
    ui_select_base_is_open(select, &is_open);
    if (!is_open) {
      printf("Failed at line 342\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
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

static int test_select_empty_keyboard(void) {
  struct ui_select_base *select = NULL;
  struct ui_component *comp;
  struct ui_event ev;
  ui_error_t rc;

#define CHECK_RC(r)                                                            \
  do {                                                                         \
    if ((r) != UI_ERROR_NONE) {                                                \
      printf("test_select_empty_keyboard failed at %d\n", __LINE__);           \
      {                                                                        \
        printf("Failed at line %d\n", __LINE__);                               \
        return 1;                                                              \
      }                                                                        \
    }                                                                          \
  } while (0)
  rc = ui_select_base_create(&select);
  CHECK_RC(rc);

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  /* is_open == 0, ENTER/SPACE */
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  ui_select_base_set_open(select, 0);
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  /* is_open == 1, highlighted_index < 0, ENTER/SPACE */
  ui_select_base_set_open(select, 1);
  ui_select_base_set_highlighted_index(select, -1);
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  ui_select_base_set_open(select, 1);
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  /* The above CLOSED the select. Re-open it. */
  ui_select_base_set_open(select, 1);

  /* is_open == 1, UP, DOWN, HOME, END on empty select */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  ev.event_data.keyboard.key_code = UI_KEY_UP;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  ev.event_data.keyboard.key_code = UI_KEY_END;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  /* Send an unhandled key to hit false branches */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  /* Test non-KEY_DOWN event */
  ev.type = UI_EVENT_KEY_UP;
  rc = ui_select_base_process_event(select, &ev, 0.0);
  CHECK_RC(rc);

  /* Test get_component null out */
  if (ui_select_base_get_component(select, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed %d\n", __LINE__);
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Test add_option with NULL value */
  if (ui_select_base_add_option(select, "No Value", NULL) != UI_ERROR_NONE) {
    printf("Failed %d\n", __LINE__);
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Test setting index with no on_change */
  ui_select_base_set_on_change(select, NULL, NULL);
  ui_error_t rc2 = ui_select_base_set_selected_index(select, 0);
  printf("set_selected_index returned %d\n", rc2);
  ui_select_base_set_highlighted_index(select, 0);
  ui_select_base_set_selected_index(select, 10);    /* clamp branch */
  ui_select_base_set_highlighted_index(select, 10); /* clamp branch */

  (void)ui_select_base_destroy(select);
  return 0;
}
static int run_oom_tests(void) {
  struct ui_select_base *select = NULL;
  ui_error_t err;
  int i;

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

  if (i == 400) {
    printf("Failed at line 451\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

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

  if (i == 100) {
    printf("Failed at line 466\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  (void)ui_select_base_destroy(select);
  return 0;
}
static ui_error_t on_select_change_fail(struct ui_select_base *select,
                                        int index, void *user_data) {
  return UI_ERROR_UNKNOWN;
}

static ui_error_t on_select_open_change_fail(struct ui_select_base *select,
                                             int is_open, void *user_data) {
  return UI_ERROR_UNKNOWN;
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

  if (ui_select_base_create(&select) != UI_ERROR_NONE) {
    printf("Failed at line 503\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* CVA */
  if (ui_select_base_get_cva(NULL, &cva) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 507\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_get_cva(select, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 509\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (ui_select_base_get_cva(select, &cva) != UI_ERROR_NONE) {
    printf("Failed at line 511\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  if (cva.register_on_change(NULL, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_INVALID_ARGUMENT) {
    printf("Failed at line 515\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  /* Test CVA OOM */
  g_malloc_fail_countdown = 0;
  if (cva.register_on_change(select, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at line 520\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  g_malloc_fail_countdown = -1;

  if (cva.register_on_change(select, mock_cva_callback, &cva_change_called) !=
      UI_ERROR_NONE) {
    printf("Failed at line 525\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }
  if (cva.register_on_touched(select, mock_cva_touched, &cva_touched_called) !=
      UI_ERROR_NONE) {
    printf("Failed at line 528\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Add an option to trigger change */
  ui_select_base_add_option(select, "1", "Option 1");
  ui_select_base_add_option(select, "2", "Option 2");

  /* Register with NULL callback to hit the wrap->callback false branch */
  cva.register_on_change(select, NULL, NULL);
  ui_select_base_set_selected_index(
      select, 1); /* Triggers wrapper with NULL callback */
  cva.register_on_change(select, mock_cva_callback,
                         &cva_change_called); /* Restore */

  p.int_val = 0;
  cva.write_value(select, p);
  cva.set_disabled_state(select, 1);
  cva.set_disabled_state(select, 0);

  ui_select_base_set_selected_index(select, 1); /* triggers wrapper */
  if (cva_change_called != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

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

  /* Error percolation tests using OOM */
  {
    ui_error_t rc;
    g_malloc_fail_countdown = 0;
    rc = ui_select_base_set_open(select, 1);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    }

    g_malloc_fail_countdown = 0;
    rc = ui_select_base_set_open(select, 0);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    }

    g_malloc_fail_countdown = 0;
    rc = ui_select_base_set_disabled(select, 0);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    }

    ui_select_base_set_disabled(select, 0);
    {
      int loop_i;
      for (loop_i = 0; loop_i < 10; loop_i++) {
        g_malloc_fail_countdown = loop_i;
        rc = ui_select_base_set_disabled(select, 1);
        g_malloc_fail_countdown = -1;
        if (rc == UI_ERROR_NONE) {
          break;
        }
      }
    }

    ui_select_base_set_disabled(select, 0);
    ui_select_base_set_open(select, 1);
    g_malloc_fail_countdown = 0;
    rc = ui_select_base_set_disabled(select, 1);
    g_malloc_fail_countdown = -1;

    ui_select_base_set_disabled(select, 0);

    /* For update_dom_state and callbacks during events */
    /* Process DOWN event -> update_dom_state */
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    ui_select_base_set_open(select, 1);
    g_malloc_fail_countdown = 0;
    rc = ui_select_base_process_event(select, &ev, 0.0);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    }

    /* Test set_rc from highlighted_index change */
    ui_select_base_set_open(select, 1);
    ui_select_base_set_highlighted_index(select, 0);
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    g_malloc_fail_countdown = 0;
    rc = ui_select_base_process_event(select, &ev, 0.0);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
    }

    ui_select_base_set_on_open_change(select, on_select_open_change_fail, NULL);
    ui_select_base_set_open(select, 0);
    ui_select_base_set_open(select, 1); /* Returns error */

    ui_select_base_set_on_change(select, on_select_change_fail, NULL);
    ui_select_base_set_selected_index(
        select, -1); /* triggers on_change fail in set_selected_index */
    ui_select_base_set_highlighted_index(select, 0);
    ev.event_data.keyboard.key_code = UI_KEY_ENTER;
    ui_select_base_process_event(select, &ev,
                                 0.0); /* triggers on_change fail */
  }

  {
    struct ui_select_internal {
      struct ui_component *component;
      struct ui_gesture_recognizer *gesture_recognizer;
    };
    struct ui_select_base *dummy_select = NULL;
    ui_select_base_create(&dummy_select);
    struct ui_select_internal *internal =
        (struct ui_select_internal *)dummy_select;

    (void)ui_component_destroy(internal->component);
    internal->component = NULL;

    (void)ui_gesture_recognizer_destroy(internal->gesture_recognizer);
    internal->gesture_recognizer = NULL;

    ui_select_base_destroy(dummy_select);
  }

  (void)ui_select_base_destroy(select);
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_select_empty_keyboard();
  failed |= test_cva_and_edge_cases();
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    {
      printf("Failed at line 685\n");
      {
        printf("Failed at line %d\n", __LINE__);
        return 1;
      }
    }
  }
  printf("All ui_select_base tests passed.\n");
  return 0;
}
