/* clang-format off */
#include "ui_autocomplete_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_event.h"
#include "ui_keyboard_responder.h"
#include "ui_listbox_base.h"
#include "ui_overlay_director.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int text_change_count = 0;
static int selection_count = 0;

static enum ui_error on_text_change(struct ui_autocomplete_base *autocomplete,
                                    const char *text, void *user_data) {
  (void)autocomplete;
  (void)text;
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  text_change_count++;
  return UI_ERROR_NONE;
}

static enum ui_error on_selection(struct ui_autocomplete_base *autocomplete,
                                  int index, void *user_data) {
  (void)autocomplete;
  (void)index;
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  selection_count++;
  return UI_ERROR_NONE;
}

static enum ui_error dummy_cva_on_change(union ui_signal_payload payload,
                                         void *user_data) {
  (void)payload;
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  return UI_ERROR_NONE;
}

static enum ui_error dummy_cva_on_touched(void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  return UI_ERROR_NONE;
}

static void test_autocomplete_edge_cases(void) {
  struct ui_autocomplete_base *autocomplete = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *tmp_comp = NULL;
  struct ui_input_base *tmp_input = NULL;
  struct ui_listbox_base *tmp_listbox = NULL;
  enum ui_error rc;
  int i;

  memset(&cva, 0, sizeof(cva));

  rc = ui_autocomplete_base_create(&autocomplete, &cva);
  assert(rc == UI_ERROR_NONE);

  /* CVA methods */
  assert(cva.write_value != NULL);
  assert(cva.register_on_change != NULL);
  assert(cva.register_on_touched != NULL);
  assert(cva.set_disabled_state != NULL);

  /* CVA NULL checks */
  assert(cva.write_value(NULL, (union ui_signal_payload){.ptr_val = NULL}) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(cva.register_on_change(NULL, dummy_cva_on_change, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(cva.register_on_touched(NULL, dummy_cva_on_touched, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(cva.set_disabled_state(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);

  int cva_change_cnt = 0;
  int cva_touched_cnt = 0;

  assert(cva.register_on_change(autocomplete, dummy_cva_on_change,
                                &cva_change_cnt) == UI_ERROR_NONE);
  assert(cva.register_on_touched(autocomplete, dummy_cva_on_touched,
                                 &cva_touched_cnt) == UI_ERROR_NONE);

  /* Write value */
  union ui_signal_payload val;
  val.ptr_val = (void *)"Hello";
  assert(cva.write_value(autocomplete, val) == UI_ERROR_NONE);
  val.ptr_val = NULL;
  assert(cva.write_value(autocomplete, val) == UI_ERROR_NONE); /* Sets to "" */

  /* Set disabled */
  assert(cva.set_disabled_state(autocomplete, 1) == UI_ERROR_NONE);
  assert(cva.set_disabled_state(autocomplete, 0) == UI_ERROR_NONE);

  /* Event handling when open */
  struct ui_layout_node layout;
  memset(&layout, 0, sizeof(layout));

  struct ui_overlay_director *director = NULL;
  struct ui_focus_manager *focus = NULL;
  struct ui_dom_node *dummy_root = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_root);
  assert(ui_overlay_director_create(dummy_root, &director) == UI_ERROR_NONE);
  assert(ui_focus_manager_create(&focus) == UI_ERROR_NONE);
  assert(director != NULL);
  assert(focus != NULL);
  assert(ui_autocomplete_base_set_overlay_dependencies(autocomplete, director,
                                                       focus) == UI_ERROR_NONE);

  /* Open should succeed */
  rc = ui_autocomplete_base_open(autocomplete, &layout, 100, 100);
  assert(rc == UI_ERROR_NONE);
  rc = ui_autocomplete_base_open(autocomplete, &layout, 100, 100);
  assert(rc == UI_ERROR_NONE); /* Already open */

  /* Send keys to test open behavior */
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  /* Listbox navigation */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  ev.event_data.keyboard.key_code = UI_KEY_UP;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  ev.event_data.keyboard.key_code = UI_KEY_END;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Send non-listbox key */
  ev.event_data.keyboard.key_code = 'B';
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Selection with ENTER */
  ui_autocomplete_base_get_listbox(autocomplete, &tmp_listbox);
  struct ui_selection_model *model = NULL;
  ui_listbox_base_get_selection_model(tmp_listbox, &model);
  ui_listbox_base_set_item_count(tmp_listbox, 1);
  ui_listbox_base_set_active_index(tmp_listbox, 0);

  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Set active index to -1 to trigger remaining branch */
  ui_listbox_base_set_active_index(tmp_listbox, -1);
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Send unhandled event while open */
  ev.type = UI_EVENT_MOUSE_DOWN;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Escape to close */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  assert(ui_autocomplete_base_process_event(autocomplete, &ev, 100.0) ==
         UI_ERROR_NONE);

  /* Open again and close explicitly to hit the is_open check in close() */
  assert(ui_autocomplete_base_open(autocomplete, &layout, 100, 100) ==
         UI_ERROR_NONE);
  assert(ui_autocomplete_base_close(autocomplete) == UI_ERROR_NONE);

  ui_overlay_director_destroy(director);
  ui_focus_manager_destroy(focus);
  ui_autocomplete_base_destroy(autocomplete);
  ui_dom_node_destroy(dummy_root);

  /* OOM loop for create */
  for (i = 0; i < 1000; i++) {
    struct ui_autocomplete_base *test_ac = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_autocomplete_base_create(&test_ac, NULL);
    if (rc == UI_ERROR_NONE) {
      printf("Loop broke at i=%d\n", i);
      ui_autocomplete_base_destroy(test_ac);
      break;
    } else {
      assert(rc == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;
  printf("test_autocomplete_edge_cases passed\n");
}

int main(void) {
  struct ui_autocomplete_base *autocomplete = NULL;
  enum ui_error rc;
  int my_data = 0;
  struct ui_event ev;

  printf("Testing ui_autocomplete_base_create...\n");
  if (ui_autocomplete_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_autocomplete_base_create(&autocomplete, NULL);
  if (rc != UI_ERROR_NONE || !autocomplete)
    return 1;

  {
    struct ui_component *tmp_comp;
    struct ui_input_base *tmp_input;
    struct ui_listbox_base *tmp_listbox;

    if (ui_autocomplete_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_component(autocomplete, NULL) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_component(autocomplete, &tmp_comp) !=
            UI_ERROR_NONE ||
        tmp_comp == NULL)
      return 1;

    if (ui_autocomplete_base_get_input(NULL, &tmp_input) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_input(autocomplete, NULL) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_input(autocomplete, &tmp_input) !=
            UI_ERROR_NONE ||
        tmp_input == NULL)
      return 1;

    if (ui_autocomplete_base_get_listbox(NULL, &tmp_listbox) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_listbox(autocomplete, NULL) == UI_ERROR_NONE)
      return 1;
    if (ui_autocomplete_base_get_listbox(autocomplete, &tmp_listbox) !=
            UI_ERROR_NONE ||
        tmp_listbox == NULL)
      return 1;
  }

  if (ui_autocomplete_base_set_overlay_dependencies(NULL, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_autocomplete_base_set_overlay_dependencies(autocomplete, NULL, NULL) !=
      UI_ERROR_NONE)
    return 1;

  if (ui_autocomplete_base_set_on_text_change(NULL, on_text_change, &my_data) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_autocomplete_base_set_on_text_change(autocomplete, on_text_change,
                                              &my_data) != UI_ERROR_NONE)
    return 1;

  if (ui_autocomplete_base_set_on_selection(NULL, on_selection, &my_data) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_autocomplete_base_set_on_selection(autocomplete, on_selection,
                                            &my_data) != UI_ERROR_NONE)
    return 1;

  printf("Testing process_event when closed...\n");
  if (ui_autocomplete_base_process_event(NULL, &ev, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_autocomplete_base_process_event(autocomplete, NULL, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'A';

  rc = ui_autocomplete_base_process_event(autocomplete, &ev, 0.0);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Since we passed a char to input_base, text_change should have fired */
  if (text_change_count == 0)
    return 1;
  if (my_data == 0)
    return 1;

  printf("Testing open/close...\n");
  if (ui_autocomplete_base_open(NULL, NULL, 100, 100) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* We cannot fully test open without a real director, but we can verify it
   * handles NULL layout cleanly */
  if (ui_autocomplete_base_open(autocomplete, NULL, 100, 100) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Testing close */
  if (ui_autocomplete_base_close(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_autocomplete_base_close(autocomplete) != UI_ERROR_NONE)
    return 1;

  ui_autocomplete_base_destroy(NULL);
  ui_autocomplete_base_destroy(autocomplete);

  test_autocomplete_edge_cases();
  printf("test_ui_autocomplete_base passed.\n");
  return 0;
}
