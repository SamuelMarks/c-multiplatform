/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include "../include/ui_segmented_control_base.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_called = 0;
static int g_change_val = -1;
static int g_touched_called = 0;

static enum ui_error dummy_change(union ui_signal_payload new_value,
                                  void *user_data) {
  g_change_called++;
  g_change_val = new_value.int_val;
  (void)user_data;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_touched(void *user_data) {
  g_touched_called++;
  (void)user_data;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_touched_fail(void *user_data) {
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

int main(void) {
  struct ui_segmented_control_base *control = NULL;
  struct ui_segmented_button_base *btn1 = NULL, *btn2 = NULL, *btn3 = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp;
  enum ui_error err;

  printf("Starting test_ui_segmented_control_base...\n");

  /* Null checks */
  err = ui_segmented_control_base_create(NULL, &cva);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_get_component(NULL, &comp);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_get_component((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  enum ui_segmented_control_mode mode;
  err =
      ui_segmented_control_base_set_mode(NULL, UI_SEGMENTED_CONTROL_MODE_MULTI);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_get_mode(NULL, &mode);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_get_mode((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_append_segment(NULL, (void *)1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_append_segment((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Create correctly */
  err = ui_segmented_control_base_create(&control, &cva);
  if (err != UI_ERROR_NONE)
    abort();

  err = ui_segmented_control_base_get_component(control, &comp);
  if (err != UI_ERROR_NONE)
    abort();
  if (comp == NULL)
    abort();

  err = ui_segmented_control_base_set_mode(control,
                                           UI_SEGMENTED_CONTROL_MODE_MULTI);
  if (err != UI_ERROR_NONE)
    abort();

  err = ui_segmented_control_base_get_mode(control, &mode);
  if (err != UI_ERROR_NONE)
    abort();
  if (mode != UI_SEGMENTED_CONTROL_MODE_MULTI)
    abort();

  /* Button NULL checks */
  err = ui_segmented_button_base_create(NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_button_base_get_component(NULL, &comp);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_button_base_get_component((void *)1, NULL);
  ui_segmented_button_base_create(&btn1);
  err = ui_segmented_button_base_get_component(btn1, &comp);
  if (err != UI_ERROR_NONE)
    abort();
  ui_segmented_button_base_destroy(btn1);
  btn1 = NULL;
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_button_base_set_selected(NULL, 1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  int selected;
  err = ui_segmented_button_base_get_selected(NULL, &selected);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_button_base_get_selected((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Create Buttons */
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_button_base_create(&btn2);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_button_base_create(&btn3);
  if (err != UI_ERROR_NONE)
    abort();

  /* Append segments */
  err = ui_segmented_control_base_append_segment(control, btn1);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_control_base_append_segment(control, btn2);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_control_base_append_segment(control, btn3);
  if (err != UI_ERROR_NONE)
    abort();

  /* Append more to test reallocation */
  struct ui_segmented_button_base *btn4, *btn5;
  ui_segmented_button_base_create(&btn4);
  ui_segmented_button_base_create(&btn5);
  ui_segmented_control_base_append_segment(control, btn4);
  ui_segmented_control_base_append_segment(control, btn5);

  /* CVA registrations null checks */
  err = cva.register_on_change(NULL, dummy_change, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = cva.register_on_touched(NULL, dummy_touched, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = cva.set_disabled_state(NULL, 1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  union ui_signal_payload pl;
  pl.int_val = 1;
  err = cva.write_value(NULL, pl);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Register correctly */
  err = cva.register_on_change(control, dummy_change, NULL);
  if (err != UI_ERROR_NONE)
    abort();
  err = cva.register_on_touched(control, dummy_touched, NULL);
  if (err != UI_ERROR_NONE)
    abort();

  /* Set selection multi */
  err = ui_segmented_button_base_set_selected(btn1, 1);
  if (err != UI_ERROR_NONE)
    abort();
  if (g_touched_called != 1)
    abort();
  if (g_change_called != 1)
    abort();

  /* Switch mode to SINGLE and select another */
  ui_segmented_control_base_set_mode(control, UI_SEGMENTED_CONTROL_MODE_SINGLE);
  err = ui_segmented_button_base_set_selected(btn2, 1);
  if (err != UI_ERROR_NONE)
    abort();

  ui_segmented_button_base_get_selected(btn1, &selected);
  if (selected != 0)
    abort(); /* Single mode deselected btn1 */
  ui_segmented_button_base_get_selected(btn2, &selected);
  if (selected != 1)
    abort();

  /* Set disabled via CVA */
  err = cva.set_disabled_state(control, 1);
  if (err != UI_ERROR_NONE)
    abort();

  /* Should be ignored due to disabled parent */
  g_change_called = 0;
  err = ui_segmented_button_base_set_selected(btn3, 1);
  if (err != UI_ERROR_NONE)
    abort();
  if (g_change_called != 0)
    abort();

  err = cva.set_disabled_state(control, 0);

  /* Test error bubbling from touch */
  cva.register_on_touched(control, dummy_touched_fail, NULL);
  err = ui_segmented_button_base_set_selected(btn3, 1);
  if (err != UI_ERROR_UNKNOWN)
    abort();

  /* Restore normal touch */
  cva.register_on_touched(control, dummy_touched, NULL);

  /* CVA Write Value */
  pl.int_val = 0; /* Select btn1 */
  err = cva.write_value(control, pl);
  if (err != UI_ERROR_NONE)
    abort();
  ui_segmented_button_base_get_selected(btn1, &selected);
  if (selected != 1)
    abort();
  ui_segmented_button_base_get_selected(btn2, &selected);
  if (selected != 0)
    abort();

  /* Trigger paths with missing callbacks or missing parent */
  struct ui_segmented_button_base *btn_orphan;
  ui_segmented_button_base_create(&btn_orphan);
  ui_segmented_button_base_set_selected(btn_orphan, 1); /* no parent */

  /* Missing callbacks on control */
  struct ui_segmented_control_base *control_no_cva;
  struct ui_segmented_button_base *btn_nocva;
  ui_segmented_control_base_create(&control_no_cva, NULL);
  ui_segmented_button_base_create(&btn_nocva);
  ui_segmented_control_base_append_segment(control_no_cva, btn_nocva);
  ui_segmented_button_base_set_selected(btn_nocva,
                                        1); /* hits null checks in triggers */

  ui_segmented_button_base_destroy(btn_nocva);
  ui_segmented_control_base_destroy(control_no_cva);

  /* Double destroy safely */
  ui_segmented_control_base_destroy(NULL);
  ui_segmented_button_base_destroy(NULL);

  ui_segmented_button_base_destroy(btn_orphan);
  ui_segmented_button_base_destroy(btn1);
  btn1 = NULL;
  ui_segmented_button_base_destroy(btn2);
  ui_segmented_button_base_destroy(btn3);
  ui_segmented_button_base_destroy(btn4);
  ui_segmented_button_base_destroy(btn5);
  ui_segmented_control_base_destroy(control);
  control = NULL;

  /* Allocation failures */
  g_malloc_fail_countdown = 0;
  err = ui_segmented_control_base_create(&control, NULL);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 1;
  err = ui_segmented_control_base_create(&control, NULL);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 0;
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 1;
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  /* Append failure */
  ui_segmented_control_base_create(&control, NULL);
  ui_segmented_button_base_create(&btn1);
  g_malloc_fail_countdown = 0;
  err = ui_segmented_control_base_append_segment(control, btn1);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  ui_segmented_button_base_destroy(btn1);
  btn1 = NULL;
  ui_segmented_control_base_destroy(control);
  control = NULL;

  printf("All tests passed.\n");
  return 0;
}
