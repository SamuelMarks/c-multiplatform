/* clang-format off */
#include "ui_side_sheet_base.h"
#include "ui_error.h"
#include "ui_event.h"
#include <stdio.h>
/* clang-format on */

static ui_error_t mock_on_close(struct ui_side_sheet_base *sheet,
                                void *user_data) {
  int *called = (int *)user_data;
  (void)sheet;
  *called = 1;
  return UI_ERROR_NONE;
}

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != UI_ERROR_NONE) {                                                \
      printf("Failed at line %d: %d\n", __LINE__, err);                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t err = (expr);                                                   \
    if (err != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             err);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_side_sheet_base *sheet = NULL;
  struct ui_component *comp = NULL;
  struct ui_event ev;
  int is_open;
  int close_called = 0;
  ui_signal_t *signal = NULL;
  int i;

  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    if (ui_side_sheet_base_create(&sheet) == UI_ERROR_NONE) {
      (void)ui_side_sheet_base_destroy(sheet);
      return 1;
    }
  }
  g_malloc_fail_countdown = -1;

  /* Null checks */
  ASSERT_EQ(ui_side_sheet_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  /* Destroy null */
  (void)ui_side_sheet_base_destroy(NULL);

  ASSERT_EQ(ui_side_sheet_base_set_content(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_set_edge(NULL, UI_SIDE_SHEET_EDGE_LEFT),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_set_mode(NULL, UI_SIDE_SHEET_MODE_SLIDE_OVER),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_set_open(NULL, 1), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_is_open(NULL, &is_open),
            UI_ERROR_INVALID_ARGUMENT);

  /* Create for further null checks */
  ASSERT_SUCCESS(ui_side_sheet_base_create(&sheet));
  ASSERT_EQ(ui_side_sheet_base_is_open(sheet, NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_set_overlay_director(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_set_on_close(NULL, mock_on_close, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_process_event(NULL, &ev, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_process_event(sheet, NULL, 0.0),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_get_component(NULL, &comp),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_get_component(sheet, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_bind_open(NULL, signal),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_side_sheet_base_bind_open(sheet, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  /* Functionality checks */
  ASSERT_SUCCESS(ui_side_sheet_base_set_content(sheet, NULL));
  ASSERT_SUCCESS(ui_side_sheet_base_set_edge(sheet, UI_SIDE_SHEET_EDGE_RIGHT));
  ASSERT_SUCCESS(ui_side_sheet_base_set_mode(sheet, UI_SIDE_SHEET_MODE_PUSH));
  ASSERT_SUCCESS(
      ui_side_sheet_base_set_on_close(sheet, mock_on_close, &close_called));
  ASSERT_SUCCESS(ui_side_sheet_base_set_overlay_director(sheet, NULL));
  ASSERT_SUCCESS(ui_side_sheet_base_get_component(sheet, &comp));
  ASSERT_SUCCESS(ui_side_sheet_base_bind_open(sheet, (ui_signal_t *)1));

  ASSERT_SUCCESS(ui_side_sheet_base_is_open(sheet, &is_open));
  if (is_open)
    return 1;

  ASSERT_SUCCESS(ui_side_sheet_base_set_open(sheet, 1));
  ASSERT_SUCCESS(ui_side_sheet_base_is_open(sheet, &is_open));
  if (!is_open)
    return 1;

  /* Event process when open */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_SUCCESS(ui_side_sheet_base_process_event(sheet, &ev, 0.0));

  /* Should be closed now */
  ASSERT_SUCCESS(ui_side_sheet_base_is_open(sheet, &is_open));
  if (is_open)
    return 1;
  /* closing should have triggered the mock on_close */
  if (!close_called)
    return 1;

  /* Try sending irrelevant event when open */
  ASSERT_SUCCESS(ui_side_sheet_base_set_open(sheet, 1));
  ev.type = UI_EVENT_MOUSE_MOVE;
  ASSERT_SUCCESS(ui_side_sheet_base_process_event(sheet, &ev, 0.0));
  ASSERT_SUCCESS(ui_side_sheet_base_is_open(sheet, &is_open));
  if (!is_open)
    return 1;

  /* Send escape when closed */
  ASSERT_SUCCESS(ui_side_sheet_base_set_open(sheet, 0));
  ASSERT_SUCCESS(ui_side_sheet_base_process_event(sheet, &ev, 0.0));

  (void)ui_side_sheet_base_destroy(sheet);

  printf("All tests passed.\n");
  return 0;
}
