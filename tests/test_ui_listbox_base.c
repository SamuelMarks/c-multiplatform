/* clang-format off */
#include "../src/ui_internal_mem.h"
#include "ui_listbox_base.h"
#include "ui_selection_model.h"
#include "ui_control_value_accessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int change_count = 0;
static int is_multi_select_cva = 0;

static const char *items[] = {"Apple", "Banana",     "Cherry",
                              "Date",  "Elderberry", "Ch"};

static const char *get_item_text(struct ui_listbox_base *listbox, int index,
                                 void *user_data) {
  if (listbox) {
  }
  if (user_data) {
  }
  if (index == 3)
    return NULL;
  if (index >= 0 && index < 6)
    return items[index];
  return NULL;
}

static const char *get_item_text_with_space(struct ui_listbox_base *listbox,
                                            int index, void *user_data) {
  if (listbox) {
  }
  if (user_data) {
  }
  if (index == 0)
    return "A B";
  return "Other";
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

static ui_error_t dummy_on_touched_err(void *user_data) {
  if (user_data) {
  }
  return UI_ERROR_INVALID_ARGUMENT;
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
  {
    ui_error_t rc_cleanup = ui_listbox_base_set_multi_select(listbox, 0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
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
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    int index = 0;
    {
      ui_error_t rc_cleanup = ui_listbox_base_get_active_index(listbox, &index);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
    if (index != 5)
      return __LINE__;
  }

  /* Test keyboard navigation with multi_select=1 */
  {
    ui_error_t rc_cleanup = ui_listbox_base_set_multi_select(listbox, 1);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ev.event_data.keyboard.key_code = UI_KEY_END;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  ev.event_data.keyboard.key_code = ' ';
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 0.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Typeahead with multi select */
  ev.event_data.keyboard.key_code = 'E';
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 50.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  {
    ui_error_t rc_cleanup = ui_listbox_base_set_multi_select(listbox, 0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

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
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 3500.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Overflow typeahead buffer (64 bytes) */
  {
    int i;
    for (i = 0; i < 70; i++) {
      ev.event_data.keyboard.key_code = 'A';
      {
        ui_error_t rc_cleanup =
            ui_listbox_base_process_event(listbox, &ev, 3500.0 + i * 10);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
    }
    /* Try overflowing with space as well */
    ev.event_data.keyboard.key_code = ' ';
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 3500.0 + 70 * 10);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
  }

  {
    int index = 0;
    {
      ui_error_t rc_cleanup = ui_listbox_base_get_active_index(listbox, &index);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
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

    /* Test touched returning error */
    cva.register_on_touched(listbox, dummy_on_touched_err, NULL);
    ui_listbox_base_process_event(listbox, &ev, 5950.0);
    cva.register_on_touched(listbox, dummy_on_touched, &on_touched_called);
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
  {
    ui_error_t rc_cleanup = ui_listbox_base_set_multi_select(listbox, 0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

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
  {
    ui_error_t rc_cleanup = ui_listbox_base_set_item_count(listbox, 6);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_listbox_base_set_active_index(listbox, 0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Typeahead with NULL text provider */
  ui_listbox_base_set_item_text_provider(listbox, NULL, NULL);
  ev.event_data.keyboard.key_code = 'B';
  ui_listbox_base_process_event(listbox, &ev, 7500.0);
  ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL);

  /* Test shadow_root == NULL branches */
  {
    struct ui_component *comp;
    struct ui_dom_node *saved_root;
    {
      ui_error_t rc_cleanup = ui_listbox_base_get_component(listbox, &comp);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
    saved_root = comp->shadow_root;
    comp->shadow_root = NULL;

    /* typeahead with no shadow root */
    ev.event_data.keyboard.key_code = 'A';
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 8000.0);
      assert(rc_cleanup == UI_ERROR_INVALID_ARGUMENT);
    }

    /* other keys with no shadow root */
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 8000.0);
      assert(rc_cleanup == UI_ERROR_INVALID_ARGUMENT);
    }

    /* set_multi_select with no shadow root */
    {
      ui_error_t rc_cleanup = ui_listbox_base_set_multi_select(listbox, 1);
      assert(rc_cleanup == UI_ERROR_NONE);
    }

    comp->shadow_root = saved_root;
  }

  /* Test attribute missing branch */
  {
    struct ui_component *comp;
    {
      ui_error_t rc_cleanup = ui_listbox_base_get_component(listbox, &comp);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
    {
      ui_error_t rc_cleanup = ui_dom_node_remove_attribute(
          comp->shadow_root, "aria-multiselectable");
      assert(rc_cleanup == UI_ERROR_NONE);
    }

    ev.event_data.keyboard.key_code = 'A';
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 8100.0);
      assert(rc_cleanup == UI_ERROR_NONE);
    }

    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 8100.0);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
  }

  /* Test text_provider == NULL branch */
  {
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_set_item_text_provider(listbox, NULL, NULL);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
    ev.event_data.keyboard.key_code = 'A';
    {
      ui_error_t rc_cleanup =
          ui_listbox_base_process_event(listbox, &ev, 8200.0);
      assert(rc_cleanup == UI_ERROR_NONE);
    }
  }

  /* DEL key */
  ev.event_data.keyboard.key_code = 127;
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 8250.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Space with no active index */
  ui_listbox_base_set_active_index(listbox, -1);
  ev.event_data.keyboard.key_code = ' ';
  {
    ui_error_t rc_cleanup = ui_listbox_base_process_event(listbox, &ev, 8300.0);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* Test model == NULL branch in on_selection_change */
  {
    struct ui_selection_model_internal {
      int is_multi;
      void **selected_ids;
      int capacity;
      int count;
      ui_selection_model_on_change_t on_change;
      void *on_change_user_data;
    } *smi = (struct ui_selection_model_internal *)model;

    if (smi && smi->on_change) {
      smi->on_change(NULL, listbox);
    }
  }

  /* Test disabled listbox process_event */
  if (cva.set_disabled_state) {
    cva.set_disabled_state(listbox, 1);
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    if (ui_listbox_base_process_event(listbox, &ev, 8400.0) != UI_ERROR_NONE)
      return __LINE__;
    cva.set_disabled_state(listbox, 0);
  }

  /* Test touched callback returning error */
  if (cva.register_on_touched) {
    cva.register_on_touched(listbox, dummy_on_touched_err, NULL);
    ev.event_data.keyboard.key_code = UI_KEY_DOWN;
    if (ui_listbox_base_process_event(listbox, &ev, 8500.0) == UI_ERROR_NONE)
      return __LINE__;
    cva.register_on_touched(listbox, dummy_on_touched, NULL);
  }

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

  {
    ui_error_t rc_cleanup = ui_listbox_base_destroy(listbox);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_listbox_base_destroy(NULL);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

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
      {
        ui_error_t rc_cleanup = ui_listbox_base_destroy(listbox);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_listbox_mock_sel_destroy_fail;
extern int g_listbox_mock_comp_destroy_fail;
extern int g_listbox_mock_sel_select_fail;
extern int g_listbox_mock_sel_toggle_fail;
extern int g_listbox_mock_sel_clear_fail;
extern int g_listbox_mock_sel_get_selected_fail;
extern int g_listbox_mock_sel_count_fail;
extern int g_listbox_mock_sel_set_on_change_fail;
extern int g_listbox_mock_set_attr_fail;
extern int g_listbox_mock_get_attr_fail;
extern int g_listbox_mock_set_style_fail;

static int test_ui_listbox_base_mock_failures(void) {
  struct ui_listbox_base *listbox = NULL;
  struct ui_control_value_accessor cva;
  struct ui_selection_model *model = NULL;
  struct ui_event ev;
  union ui_signal_payload val;
  ui_error_t rc;

  memset(&cva, 0, sizeof(cva));
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  val.int_val = 1;

  /* Creation style failure */
  g_listbox_mock_set_style_fail = 1;
  rc = ui_listbox_base_create(&listbox, &cva);
  g_listbox_mock_set_style_fail = 0;
  if (rc == UI_ERROR_NONE || listbox != NULL) {
    return __LINE__;
  }

  /* Creation set_on_change failure */
  g_listbox_mock_sel_set_on_change_fail = 1;
  rc = ui_listbox_base_create(&listbox, &cva);
  g_listbox_mock_sel_set_on_change_fail = 0;
  if (rc == UI_ERROR_NONE || listbox != NULL) {
    return __LINE__;
  }

  /* Creation role attribute failure */
  g_listbox_mock_set_attr_fail = 1;
  rc = ui_listbox_base_create(&listbox, &cva);
  g_listbox_mock_set_attr_fail = 0;
  if (rc == UI_ERROR_NONE || listbox != NULL) {
    return __LINE__;
  }

  /* Create clean listbox */
  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox) {
    return __LINE__;
  }
  ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL);
  ui_listbox_base_set_item_count(listbox, 6);
  ui_listbox_base_get_selection_model(listbox, &model);

  /* Destroy failure of selection model */
  g_listbox_mock_sel_destroy_fail = 1;
  rc = ui_listbox_base_destroy(listbox);
  g_listbox_mock_sel_destroy_fail = 0;
  if (rc == UI_ERROR_NONE) {
    return __LINE__;
  }

  /* Re-create for component destroy failure */
  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox) {
    return __LINE__;
  }
  g_listbox_mock_comp_destroy_fail = 1;
  rc = ui_listbox_base_destroy(listbox);
  g_listbox_mock_comp_destroy_fail = 0;
  if (rc == UI_ERROR_NONE) {
    return __LINE__;
  }

  /* Re-create for double destroy failure */
  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox) {
    return __LINE__;
  }
  g_listbox_mock_sel_destroy_fail = 1;
  g_listbox_mock_comp_destroy_fail = 1;
  rc = ui_listbox_base_destroy(listbox);
  g_listbox_mock_sel_destroy_fail = 0;
  g_listbox_mock_comp_destroy_fail = 0;
  if (rc == UI_ERROR_NONE) {
    return __LINE__;
  }

  /* Create listbox for CVA and operational tests */
  rc = ui_listbox_base_create(&listbox, &cva);
  if (rc != UI_ERROR_NONE || !listbox) {
    return __LINE__;
  }
  ui_listbox_base_set_item_text_provider(listbox, get_item_text, NULL);
  ui_listbox_base_set_item_count(listbox, 6);
  ui_listbox_base_get_selection_model(listbox, &model);
  cva.register_on_change(listbox, dummy_on_change, NULL);

  /* Disabled state attribute failures */
  g_listbox_mock_set_attr_fail = 1;
  if (cva.set_disabled_state(listbox, 1) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_set_attr_fail = 0;

  g_listbox_mock_set_attr_fail = 2;
  if (cva.set_disabled_state(listbox, 1) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_set_attr_fail = 0;

  g_listbox_mock_set_attr_fail = 2;
  if (cva.set_disabled_state(listbox, 0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_set_attr_fail = 0;

  /* CVA write value failures */
  g_listbox_mock_get_attr_fail = 1;
  if (cva.write_value(listbox, val) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_get_attr_fail = 0;

  g_listbox_mock_sel_clear_fail = 1;
  if (cva.write_value(listbox, val) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_clear_fail = 0;

  g_listbox_mock_sel_select_fail = 1;
  if (cva.write_value(listbox, val) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  /* set_multi_select attribute failure */
  g_listbox_mock_set_attr_fail = 1;
  if (ui_listbox_base_set_multi_select(listbox, 1) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_set_attr_fail = 0;

  /* set_item_count clear failure */
  g_listbox_mock_sel_clear_fail = 1;
  if (ui_listbox_base_set_item_count(listbox, 0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_clear_fail = 0;
  ui_listbox_base_set_item_count(listbox, 6);

  /* Selection change CVA trigger failures */
  /* Count failure */
  g_listbox_mock_sel_count_fail = 1;
  if (ui_selection_model_select(model, (void *)(size_t)1) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_count_fail = 0;

  /* Single-select get_selected failure */
  ui_listbox_base_set_multi_select(listbox, 0);
  g_listbox_mock_sel_get_selected_fail = 1;
  if (ui_selection_model_select(model, (void *)(size_t)2) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_get_selected_fail = 0;

  /* Multi-select get_selected failure */
  ui_listbox_base_set_multi_select(listbox, 1);
  g_listbox_mock_sel_get_selected_fail = 1;
  if (ui_selection_model_select(model, (void *)(size_t)3) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_get_selected_fail = 0;

  /* Selection change get_attribute failure */
  g_listbox_mock_get_attr_fail = 1;
  if (ui_selection_model_select(model, (void *)(size_t)4) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_get_attr_fail = 0;

  /* Event failures in single-select */
  ui_listbox_base_set_multi_select(listbox, 0);
  ui_listbox_base_set_active_index(listbox, 1);

  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  if (ui_listbox_base_process_event(listbox, &ev, 9000.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  if (ui_listbox_base_process_event(listbox, &ev, 9050.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  if (ui_listbox_base_process_event(listbox, &ev, 9100.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_END;
  if (ui_listbox_base_process_event(listbox, &ev, 9150.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  if (ui_listbox_base_process_event(listbox, &ev, 9200.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  /* Multi-select space toggle failure */
  ui_listbox_base_set_multi_select(listbox, 1);
  g_listbox_mock_sel_toggle_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  if (ui_listbox_base_process_event(listbox, &ev, 9250.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_toggle_fail = 0;

  /* Perform typeahead select failure on character input */
  ui_listbox_base_set_multi_select(listbox, 0);
  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = 'B';
  if (ui_listbox_base_process_event(listbox, &ev, 11000.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  /* Perform typeahead select failure on space in typeahead */
  ui_listbox_base_set_multi_select(listbox, 0);
  ui_listbox_base_set_item_text_provider(listbox, get_item_text_with_space,
                                         NULL);
  ev.event_data.keyboard.key_code = 'A';
  ui_listbox_base_process_event(listbox, &ev, 15000.0);
  g_listbox_mock_sel_select_fail = 1;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  if (ui_listbox_base_process_event(listbox, &ev, 15050.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_sel_select_fail = 0;

  /* Perform typeahead get_attribute failure */
  g_listbox_mock_get_attr_fail = 2;
  ev.event_data.keyboard.key_code = 'A';
  if (ui_listbox_base_process_event(listbox, &ev, 17000.0) == UI_ERROR_NONE) {
    return __LINE__;
  }
  g_listbox_mock_get_attr_fail = 0;

  ui_listbox_base_destroy(listbox);

  /* Destroy with NULL fields */
  {
    struct ui_listbox_internal {
      struct ui_component *component;
      struct ui_selection_model *selection_model;
    } *internal;
    rc = ui_listbox_base_create(&listbox, NULL);
    if (rc != UI_ERROR_NONE || !listbox) {
      return __LINE__;
    }
    internal = (struct ui_listbox_internal *)listbox;
    ui_selection_model_destroy(internal->selection_model);
    internal->selection_model = NULL;
    ui_component_destroy(internal->component);
    internal->component = NULL;
    rc = ui_listbox_base_destroy(listbox);
    if (rc != UI_ERROR_NONE) {
      return __LINE__;
    }
  }

  return 0;
}
#endif

int main(void) {
  int failed = 0;

  printf("Running ui_listbox_base tests...\n");

  failed |= run_normal_tests();
  failed |= run_oom_tests();
#ifdef UI_TEST_MOCK_ALLOC
  failed |= test_ui_listbox_base_mock_failures();
#endif

  if (failed) {
    printf("Tests failed. failed=%d\n", failed);
    return __LINE__;
  }

  printf("All tests passed.\n");
  return 0;
}
