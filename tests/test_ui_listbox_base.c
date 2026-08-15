#include "../src/ui_internal_mem.h"
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
  if (index == 3)
    return NULL;
  if (index >= 0 && index < 6)
    return items[index];
  return NULL;
}

static ui_error_t dummy_on_change(union ui_signal_payload value,
                                  void *user_data) {
  if (user_data)
    *(int *)user_data = 1;
  if (is_multi_select_cva && value.ptr_val) {
    C_MULTIPLATFORM_FREE(value.ptr_val);
  }
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) {
  if (user_data)
    *(int *)user_data = 1;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_listbox_base *listbox = NULL;
  struct ui_selection_model *model = NULL;
  struct ui_control_value_accessor cva;
  ui_error_t rc;
  struct ui_event ev;
  int is_selected;
  int on_change_called = 0;
  int on_touched_called = 0;

  memset(&cva, 0, sizeof(cva));

  printf("Testing ui_listbox_base_create...\n");
  if (ui_listbox_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox)
    return 1;

  {
    struct ui_listbox_base *lb_no_cva = NULL;
    if (ui_listbox_base_create(&lb_no_cva, NULL) == UI_ERROR_NONE) {
      ui_listbox_base_destroy(lb_no_cva);
    }
  }

  {
    struct ui_component *tmp_comp;
    if (ui_listbox_base_get_component(NULL, &tmp_comp) == UI_ERROR_NONE)
      return __LINE__;
    if (ui_listbox_base_get_component(listbox, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return __LINE__;
    if (ui_listbox_base_get_component(listbox, &tmp_comp) != UI_ERROR_NONE)
      return __LINE__;
    if (tmp_comp == NULL)
      return __LINE__;
  }

  if (ui_listbox_base_get_selection_model(NULL, &model) !=
      UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_get_selection_model(listbox, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;

  if (ui_listbox_base_get_selection_model(listbox, &model) != UI_ERROR_NONE ||
      !model)
    return __LINE__;

  printf("Testing item count and multi-select...\n");
  if (ui_listbox_base_set_item_count(NULL, 6) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_set_item_count(listbox, -1) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_set_item_count(listbox, 6) != UI_ERROR_NONE)
    return __LINE__;

  if (ui_listbox_base_set_multi_select(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_set_multi_select(listbox, 1) != UI_ERROR_NONE)
    return __LINE__;

  if (ui_listbox_base_set_item_text_provider(NULL, get_item_text, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL) !=
      UI_ERROR_NONE)
    return __LINE__;

  printf("Testing selection APIs...\n");
  ui_selection_model_select(model, (void *)(size_t)0);
  ui_selection_model_select(model, (void *)(size_t)2);

  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 1)
    return __LINE__;
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 0)
    return __LINE__;
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 1)
    return __LINE__;

  /* Convert back to single select */
  is_multi_select_cva = 0;
  ui_listbox_base_set_multi_select(listbox, 0);
  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 1)
    return __LINE__;
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 0)
    return __LINE__;

  /* In single select, selecting item 1 should deselect item 0 */
  ui_selection_model_select(model, (void *)(size_t)1);
  ui_selection_model_is_selected(model, (void *)(size_t)0, &is_selected);
  if (is_selected != 0)
    return __LINE__;
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 1)
    return __LINE__;

  ui_selection_model_clear(model);
  ui_selection_model_is_selected(model, (void *)(size_t)1, &is_selected);
  if (is_selected != 0)
    return __LINE__;

  printf("Testing active index...\n");
  if (ui_listbox_base_set_active_index(NULL, 0) != UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  {
    int index;
    if (ui_listbox_base_get_active_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT)
      return __LINE__;
    if (ui_listbox_base_get_active_index(listbox, NULL) !=
        UI_ERROR_INVALID_ARGUMENT)
      return __LINE__;
  }
  if (ui_listbox_base_set_active_index(listbox, -2) != UI_ERROR_OUT_OF_BOUNDS)
    return __LINE__;
  if (ui_listbox_base_set_active_index(listbox, 6) != UI_ERROR_OUT_OF_BOUNDS)
    return __LINE__;

  ui_listbox_base_set_active_index(listbox, 2);
  {
    int index = 0;
    if (ui_listbox_base_get_active_index(NULL, &index) !=
        UI_ERROR_INVALID_ARGUMENT)
      return __LINE__;
  }
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2)
      return __LINE__;
  }

  printf("Testing keyboard navigation...\n");
  (void)ui_listbox_base_set_multi_select(listbox, 0);
  if (ui_listbox_base_process_event(NULL, &ev, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;
  if (ui_listbox_base_process_event(listbox, NULL, 0.0) !=
      UI_ERROR_INVALID_ARGUMENT)
    return __LINE__;

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  /* DOWN arrow from index 2 -> 3 */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 3)
      return __LINE__;
  }
  /* In single select, it also auto-selects */
  ui_selection_model_is_selected(model, (void *)(size_t)3, &is_selected);
  if (is_selected != 1)
    return __LINE__;

  /* UP arrow from index 3 -> 2 */
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2)
      return __LINE__;
  }
  ui_selection_model_is_selected(model, (void *)(size_t)2, &is_selected);
  if (is_selected != 1)
    return __LINE__;

  /* HOME */
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 0)
      return __LINE__;
  }

  /* END */
  ev.event_data.keyboard.key_code = UI_KEY_END;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);
  {
    int index = 0;
    (void)ui_listbox_base_get_active_index(listbox, &index);
    if (index != 5)
      return __LINE__;
  }

  /* Test keyboard navigation with multi_select=1 */
  (void)ui_listbox_base_set_multi_select(listbox, 1);
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  ev.event_data.keyboard.key_code = UI_KEY_END;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  ev.event_data.keyboard.key_code = ' ';
  (void)ui_listbox_base_process_event(listbox, &ev, 0.0);

  /* Typeahead with multi select */
  ev.event_data.keyboard.key_code = 'E';
  (void)ui_listbox_base_process_event(listbox, &ev, 50.0);

  (void)ui_listbox_base_set_multi_select(listbox, 0);

  /* Typeahead search 'C' -> Ch (index 5) because active index was 4 */
  ev.event_data.keyboard.key_code = 'C';
  ui_listbox_base_process_event(listbox, &ev, 2000.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 5) {
      return 1;
    }
  }

  /* Typeahead search 'h' -> Cherry (index 2) */
  ev.event_data.keyboard.key_code = 'H';
  ui_listbox_base_process_event(listbox, &ev, 2050.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2) {
      return 1;
    }
  }

  /* Typeahead space -> 'Ch ' -> no match, remains at 2 */
  ev.event_data.keyboard.key_code = ' ';
  ui_listbox_base_process_event(listbox, &ev, 2100.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 2) {
      return 1;
    }
  }

  /* Typeahead with multi select */
  ui_listbox_base_set_multi_select(listbox, 1);
  ev.event_data.keyboard.key_code = 'E';
  ui_listbox_base_process_event(listbox, &ev, 3550.0);
  {
    int index = 0;
    ui_listbox_base_get_active_index(listbox, &index);
    if (index != 4) {
      printf("failed %d, index=%d\n", __LINE__, index);
      return __LINE__;
    }
  }
  is_multi_select_cva = 0;
  ui_listbox_base_set_multi_select(listbox, 0);

  /* Typeahead timeout */
  ev.event_data.keyboard.key_code = 'B';
  (void)ui_listbox_base_process_event(listbox, &ev, 3500.0);

  /* Overflow typeahead buffer (64 bytes) */
  {
    int i;
    for (i = 0; i < 70; i++) {
      ev.event_data.keyboard.key_code = 'A';
      (void)ui_listbox_base_process_event(listbox, &ev, 3500.0 + i * 10);
    }
    /* Try overflowing with space as well */
    ev.event_data.keyboard.key_code = ' ';
    (void)ui_listbox_base_process_event(listbox, &ev, 3500.0 + 70 * 10);
  }

  {
    int index = 0;
    (void)ui_listbox_base_get_active_index(listbox, &index);
  }

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

  /* Key code >= 127 */
  ev.event_data.keyboard.key_code = 128;
  ui_listbox_base_process_event(listbox, &ev, 6000.0);

  /* Key code < 32 */
  ev.event_data.keyboard.key_code = 10;
  ui_listbox_base_process_event(listbox, &ev, 6000.0);

  /* Active index >= num_items */
  ui_listbox_base_set_active_index(listbox, 10);
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_listbox_base_process_event(listbox, &ev, 6000.0);

  if (cva.register_on_change) {
    on_change_called = 0;
    cva.register_on_change(listbox, dummy_on_change, &on_change_called);

    /* Test OOM in CVA array allocation */
    ui_listbox_base_set_multi_select(listbox, 1);
    ui_selection_model_select(model, (void *)(size_t)1);
    g_malloc_fail_countdown = 0;
    ev.event_data.keyboard.key_code = ' ';
    ui_listbox_base_process_event(listbox, &ev, 5550.0);
    g_malloc_fail_countdown = -1;
    ui_selection_model_clear(model);

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
    on_touched_called = 0;
    cva.register_on_touched(listbox, dummy_on_touched, &on_touched_called);
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
    {
      struct ui_component *comp;
      ui_listbox_base_get_component(listbox, &comp);
      ui_dom_node_remove_attribute(comp->shadow_root, "aria-multiselectable");
    }
    cva.write_value(listbox, payload);

    is_multi_select_cva = 1;
    ui_listbox_base_set_multi_select(listbox, 1);
    payload.ptr_val = NULL;
    cva.write_value(listbox, payload);
  }

  is_multi_select_cva = 0;
  (void)ui_listbox_base_set_multi_select(listbox, 0);

  /* trigger shrinking list */
  ui_listbox_base_set_active_index(listbox, 5);
  ui_listbox_base_set_item_count(listbox, 2);

  /* Press enter while out of bounds */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_listbox_base_process_event(listbox, &ev, 6010.0);

  ui_listbox_base_set_item_count(listbox, 0);
  ui_listbox_base_set_active_index(listbox, -1);

  /* Typeahead and navigation on empty list */
  ev.event_data.keyboard.key_code = 'A';
  ui_listbox_base_process_event(listbox, &ev, 7000.0);
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ui_listbox_base_process_event(listbox, &ev, 7000.0);
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ui_listbox_base_process_event(listbox, &ev, 7000.0);
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ui_listbox_base_process_event(listbox, &ev, 7000.0);
  ev.event_data.keyboard.key_code = UI_KEY_END;
  ui_listbox_base_process_event(listbox, &ev, 7000.0);
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ui_listbox_base_process_event(listbox, &ev, 7000.0);

  /* Restore item count for further tests */
  (void)ui_listbox_base_set_item_count(listbox, 6);
  (void)ui_listbox_base_set_active_index(listbox, 0);

  /* Typeahead with NULL text provider */
  ui_listbox_base_set_item_text_provider(listbox, NULL, NULL);
  ev.event_data.keyboard.key_code = 'B';
  ui_listbox_base_process_event(listbox, &ev, 7500.0);
  ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL);

  /* Test shadow_root == NULL branches */
  {
    struct ui_component *comp;
    struct ui_dom_node *saved_root;
    (void)ui_listbox_base_get_component(listbox, &comp);
    saved_root = comp->shadow_root;
    comp->shadow_root = NULL;

    /* typeahead with no shadow root */
    ev.event_data.keyboard.key_code = 'A';
    (void)ui_listbox_base_process_event(listbox, &ev, 8000.0);

    /* other keys with no shadow root */
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    (void)ui_listbox_base_process_event(listbox, &ev, 8000.0);

    /* set_multi_select with no shadow root */
    (void)ui_listbox_base_set_multi_select(listbox, 1);

    comp->shadow_root = saved_root;
  }

  /* Test attribute missing branch */
  {
    struct ui_component *comp;
    (void)ui_listbox_base_get_component(listbox, &comp);
    (void)ui_dom_node_remove_attribute(comp->shadow_root,
                                       "aria-multiselectable");

    ev.event_data.keyboard.key_code = 'A';
    (void)ui_listbox_base_process_event(listbox, &ev, 8100.0);

    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    (void)ui_listbox_base_process_event(listbox, &ev, 8100.0);
  }

  /* Test text_provider == NULL branch */
  {
    (void)ui_listbox_base_set_item_text_provider(listbox, NULL, NULL);
    ev.event_data.keyboard.key_code = 'A';
    (void)ui_listbox_base_process_event(listbox, &ev, 8200.0);
  }

  /* DEL key */
  ev.event_data.keyboard.key_code = 127;
  (void)ui_listbox_base_process_event(listbox, &ev, 8250.0);

  /* Space with no active index */
  ui_listbox_base_set_active_index(listbox, -1);
  ev.event_data.keyboard.key_code = ' ';
  (void)ui_listbox_base_process_event(listbox, &ev, 8300.0);

  /* CVA register on change with NULL */
  if (cva.register_on_change)
    cva.register_on_change(NULL, dummy_on_change, NULL);
  if (cva.register_on_touched)
    cva.register_on_touched(NULL, dummy_on_touched, NULL);

  /* Test OOM in CVA array allocation */
  {
    ui_listbox_base_set_multi_select(listbox, 1);
    ui_selection_model_select(model, (void *)(size_t)1);
    g_malloc_fail_countdown = 0;
    ev.event_data.keyboard.key_code = ' ';
    ui_listbox_base_process_event(listbox, &ev, 5500.0);
    g_malloc_fail_countdown = -1;
  }

  (void)ui_listbox_base_destroy(listbox);
  (void)ui_listbox_base_destroy(NULL);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_listbox_base *listbox = NULL;
  ui_error_t rc;
  int i;
  printf("Testing OOM handling...\n");
  for (i = 0; i < 20; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_listbox_base_create(&listbox, NULL);
    printf("i=%d rc=%d\n", i, rc);
    if (rc == UI_ERROR_NONE) {
      (void)ui_listbox_base_destroy(listbox);
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
    printf("Tests failed. failed=%d\n", failed);
    return __LINE__;
  }

  printf("All tests passed.\n");
  return 0;
}
