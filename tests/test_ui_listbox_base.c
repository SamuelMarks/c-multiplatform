/* clang-format off */
#include "ui_listbox_base.h"
#include "ui_selection_model.h"
#include "ui_control_value_accessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int change_count = 0;
static int is_multi_select_cva = 0;

static const char *items[] = {"Apple", "Banana",     "Cherry",
                              "Date",  "Elderberry", "Ch"};

static const char *get_item_text(struct ui_listbox_base *listbox, int index,
                                 void *user_data) {
  (void)listbox;
  (void)user_data;
  if (index >= 0 && index < 6)
    return items[index];
  return NULL;
}

static enum ui_error dummy_on_change(union ui_signal_payload value,
                                     void *user_data) {
  if (user_data)
    *(int *)user_data = 1;
  if (is_multi_select_cva && value.ptr_val) {
    ui_mock_free(value.ptr_val);
  }
  return UI_ERROR_NONE;
}

static enum ui_error dummy_on_touched(void *user_data) {
  *(int *)user_data = 1;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_listbox_base *listbox = NULL;
  struct ui_selection_model *model = NULL;
  struct ui_control_value_accessor cva;
  enum ui_error rc;
  struct ui_event ev;
  int is_selected;

  memset(&cva, 0, sizeof(cva));

  printf("Testing ui_listbox_base_create...\n");
  if (ui_listbox_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox)
    return 1;

  {
    struct ui_component *tmp_comp;
    if (ui_listbox_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return 1;
    if (ui_listbox_base_get_component(listbox, &tmp_comp) != UI_ERROR_NONE)
      return 1;
    if (tmp_comp == NULL)
      return 1;
  }

  if (ui_listbox_base_get_selection_model(NULL, &model) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_get_selection_model(listbox, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_listbox_base_get_selection_model(listbox, &model) != UI_ERROR_NONE ||
      !model)
    return 1;

  printf("Testing item count and multi-select...\n");
  if (ui_listbox_base_set_item_count(NULL, 6) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_set_item_count(listbox, -1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_set_item_count(listbox, 6) != UI_ERROR_NONE)
    return 1;

  if (ui_listbox_base_set_multi_select(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_set_multi_select(listbox, 1) != UI_ERROR_NONE)
    return 1;

  if (ui_listbox_base_set_item_text_provider(NULL, get_item_text, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL) !=
      UI_ERROR_NONE)
    return 1;

  printf("Testing selection APIs...\n");
  ui_selection_model_select(model, (void *)(size_t)0);
  ui_selection_model_select(model, (void *)(size_t)2);

  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 1)
    return 1;
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 0)
    return 1;
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 1)
    return 1;

  /* Convert back to single select */
  is_multi_select_cva = 0;
  ui_listbox_base_set_multi_select(listbox, 0);
  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 1)
    return 1;
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 0)
    return 1;

  /* In single select, selecting item 1 should deselect item 0 */
  ui_selection_model_select(model, (void *)(size_t)1);
  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 0)
    return 1;
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 1)
    return 1;

  ui_selection_model_clear(model);
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 0)
    return 1;

  printf("Testing active index...\n");
  if (ui_listbox_base_set_active_index(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_set_active_index(listbox, -2) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;
  if (ui_listbox_base_set_active_index(listbox, 6) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  ui_listbox_base_set_active_index(listbox, 2);
  {
    int index = 0;
    if (ui_listbox_base_get_active_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
  }
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2)
      return 1;
  }

  printf("Testing keyboard navigation...\n");
  if (ui_listbox_base_process_event(NULL, &ev, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_listbox_base_process_event(listbox, NULL, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  /* DOWN arrow from index 2 -> 3 */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 3)
      return 1;
  }
  /* In single select, it also auto-selects */
  ui_selection_model_is_selected(model, (void *)(size_t)3, &is_selected);
  if (is_selected != 1)
    return 1;

  /* UP arrow from index 3 -> 2 */
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2)
      return 1;
  }
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 1)
    return 1;

  /* HOME */
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 0)
      return 1;
  }

  /* END */
  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 5)
      return 1;
  }

  /* Typeahead search 'C' -> Cherry (index 2) */
  ev.event_data.keyboard.key_code = 'C';
  ui_listbox_base_process_event(listbox, &ev, 100.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2)
      return 1;
  }

  /* Typeahead search 'h' -> Cherry (still index 2? No! Index 5 'Ch') */
  ev.event_data.keyboard.key_code = 'H';
  ui_listbox_base_process_event(listbox, &ev, 150.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 5) {
      printf("failed %d, index=%d\n", __LINE__, index);
      return 1;
    }
  }

  /* Typeahead space -> 'Ch ' -> no match, remains at 5 */
  ev.event_data.keyboard.key_code = ' ';
  ui_listbox_base_process_event(listbox, &ev, 200.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 5) {
      printf("failed %d, index=%d\n", __LINE__, index);
      return 1;
    }
  }

  /* Typeahead with multi select */
  ui_listbox_base_set_multi_select(listbox, 1);
  ev.event_data.keyboard.key_code = 'E';
  ui_listbox_base_process_event(listbox, &ev, 2050.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 4) {
      printf("failed %d, index=%d\n", __LINE__, index);
      return 1;
    }
  }
  is_multi_select_cva = 0;
  ui_listbox_base_set_multi_select(listbox, 0);

  /* Typeahead timeout */
  ev.event_data.keyboard.key_code = 'B';
  ui_listbox_base_process_event(listbox, &ev, 3500.0); /* > 1000ms later */
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 1) {
      printf("failed %d, index=%d\n", __LINE__, index);
      return 1;
    }
  } /* Banana */

  /* Typeahead timeout space */
  ev.event_data.keyboard.key_code = ' ';
  ui_listbox_base_process_event(listbox, &ev, 4000.0); /* > 1000ms later */
  /* space alone should just toggle selection in single select */

  /* Enter to toggle selection */
  ui_listbox_base_set_multi_select(listbox, 1);
  ui_selection_model_clear(model);
  ev.event_data.keyboard.key_code = ' ';
  ui_listbox_base_process_event(listbox, &ev,
                                5500.0); /* Toggles active index */

  if (cva.register_on_change) {
    int called = 0;
    cva.register_on_change(listbox, dummy_on_change, &called);

    is_multi_select_cva = 1;
    ui_listbox_base_set_multi_select(listbox, 1);

    /* Let's select two items */
    ui_listbox_base_set_active_index(listbox, 0);
    ev.event_data.keyboard.key_code = ' ';
    ui_listbox_base_process_event(listbox, &ev, 5600.0);

    ui_listbox_base_set_active_index(listbox, 1);
    ev.event_data.keyboard.key_code = ' ';
    ui_listbox_base_process_event(listbox, &ev, 5700.0);

    is_multi_select_cva = 0;
    ui_listbox_base_set_multi_select(listbox, 0);
    ev.event_data.keyboard.key_code = UI_KEY_ENTER;
    ui_listbox_base_process_event(listbox, &ev, 5800.0);
  }

  if (cva.register_on_touched) {
    int called = 0;
    cva.register_on_touched(listbox, dummy_on_touched, &called);
    ev.type = UI_EVENT_KEY_UP;
    ui_listbox_base_process_event(listbox, &ev, 5900.0);
  }

  if (cva.set_disabled_state) {
    cva.set_disabled_state(NULL, UI_TRUE);
    cva.set_disabled_state(listbox, UI_TRUE);

    ev.type = UI_EVENT_KEY_DOWN;
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    ui_listbox_base_process_event(listbox, &ev, 6000.0);

    cva.set_disabled_state(listbox, UI_FALSE);
  }

  if (cva.write_value) {
    union ui_signal_payload payload;
    memset(&payload, 0, sizeof(payload));
    cva.write_value(NULL, payload);

    payload.int_val = 1;
    cva.write_value(listbox, payload);

    payload.int_val = -1;
    cva.write_value(listbox, payload);

    is_multi_select_cva = 1;
    ui_listbox_base_set_multi_select(listbox, 1);
    payload.ptr_val = NULL;
    cva.write_value(listbox, payload);
  }

  /* trigger shrinking list */
  ui_listbox_base_set_active_index(listbox, 5);
  ui_listbox_base_set_item_count(listbox, 2);

  ui_listbox_base_set_item_count(listbox, 0);
  ui_listbox_base_set_active_index(listbox, -1);

  /* Typeahead on empty list */
  ev.event_data.keyboard.key_code = 'A';
  ui_listbox_base_process_event(listbox, &ev, 7000.0);

  /* CVA register on change with NULL */
  if (cva.register_on_change)
    cva.register_on_change(NULL, dummy_on_change, NULL);
  if (cva.register_on_touched)
    cva.register_on_touched(NULL, dummy_on_touched, NULL);

  ui_listbox_base_destroy(listbox);
  ui_listbox_base_destroy(NULL);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_listbox_base *listbox = NULL;
  enum ui_error rc;
  int i;
  printf("Testing OOM handling...\n");
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_listbox_base_create(&listbox, NULL);
    if (rc == UI_ERROR_NONE) {
      ui_listbox_base_destroy(listbox);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  int failed = 0;

  printf("Running ui_listbox_base tests...\n");

  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
