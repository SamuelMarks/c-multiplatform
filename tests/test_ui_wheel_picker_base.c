/* clang-format off */
#include "ui_wheel_picker_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int change_count = 0;
static int last_selected_index = -1;

static enum ui_error on_change(struct ui_wheel_picker_base *picker,
                               int selected_index, void *user_data) {
  (void)picker;
  (void)user_data;
  change_count++;
  last_selected_index = selected_index;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_wheel_picker_base *picker = NULL;
  enum ui_error rc;
  const char *items[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
  struct ui_event ev;
  int failed = 0;

  rc = ui_wheel_picker_base_create(&picker, NULL);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (picker == NULL);

  rc = ui_wheel_picker_base_set_items(picker, items, 5);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_wheel_picker_base_set_on_change(picker, on_change, NULL);
  failed |= (rc != UI_ERROR_NONE);

  /* Test basic programmatic selection */
  rc = ui_wheel_picker_base_set_selected_index(picker, 2);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (change_count != 1);
  failed |= (last_selected_index != 2);
  {
    int index = 0;
    ui_wheel_picker_base_get_selected_index(picker, &index);
    failed |= (index != 2);
  }

  /* Bounds test non-looping */
  rc = ui_wheel_picker_base_set_selected_index(picker, -5);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 0);

  rc = ui_wheel_picker_base_set_selected_index(picker, 10);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 4);

  /* Looping bounds test */
  rc = ui_wheel_picker_base_set_looping(picker, 1);
  failed |= (rc != UI_ERROR_NONE);

  rc = ui_wheel_picker_base_set_selected_index(picker, 6); /* 6 % 5 = 1 */
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 1);

  rc = ui_wheel_picker_base_set_selected_index(picker, -1); /* -1 -> 4 */
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 4);

  /* Keyboard test */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;

  rc = ui_wheel_picker_base_process_event(picker, &ev, 100.0);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 3); /* From 4 going UP (-1) -> 3 */

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  rc = ui_wheel_picker_base_process_event(picker, &ev, 100.0);
  failed |= (rc != UI_ERROR_NONE);
  failed |= (last_selected_index != 4); /* From 3 going DOWN (+1) -> 4 */

  ui_wheel_picker_base_destroy(picker);

  if (!failed) {
    printf("test_ui_wheel_picker_base passed\n");
  }
  return failed;
}
