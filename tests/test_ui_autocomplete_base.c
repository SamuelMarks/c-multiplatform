

/* clang-format off */
#include "greatest.h"
#include "ui_focus_manager.h"
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

extern int g_ac_mock_fail;

extern int g_ac_mock_fail;

extern int g_ac_mock_fail;

static void test_autocomplete_process_event_explicit() {
  struct ui_autocomplete_base *autocomplete = NULL;
  struct ui_event ev;
  ui_autocomplete_base_create(&autocomplete, NULL);

  /* Mock popover closed */
  g_ac_mock_fail = 5; /* mock_popover_is_open return 0 */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'A';
  ui_autocomplete_base_process_event(autocomplete, &ev, 100.0);

  g_ac_mock_fail = 0;

  /* Mock ui_dom_node_set_attribute failure inside process_event */
  ev.type = UI_EVENT_CLICK;
  g_ac_mock_fail = 23; /* mock_dom_node_set_attribute fails */
  ui_autocomplete_base_process_event(autocomplete, &ev, 100.0);
  g_ac_mock_fail = 0;

  /* Mock ui_listbox_base_get_active_index fails */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  g_ac_mock_fail = 24;
  ui_autocomplete_base_process_event(autocomplete, &ev, 100.0);
  g_ac_mock_fail = 0;

  ui_autocomplete_base_destroy(autocomplete);
}

extern int g_malloc_fail_countdown;

static int text_change_count = 0;
static int selection_count = 0;

static ui_error_t on_text_change(struct ui_autocomplete_base *autocomplete,
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

static ui_error_t on_selection(struct ui_autocomplete_base *autocomplete,
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

static ui_error_t dummy_cva_on_change(union ui_signal_payload payload,
                                      void *user_data) {
  (void)payload;
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  return UI_ERROR_NONE;
}

static ui_error_t dummy_cva_on_touched(void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  return UI_ERROR_NONE;
}

TEST test_autocomplete_edge_cases(void) {
  struct ui_autocomplete_base *autocomplete = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *tmp_comp = NULL;
  struct ui_input_base *tmp_input = NULL;
  struct ui_listbox_base *tmp_listbox = NULL;
  ui_error_t rc;
  int i;

  memset(&cva, 0, sizeof(cva));

  rc = ui_autocomplete_base_create(&autocomplete, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA methods */
  ASSERT(cva.write_value != NULL);
  ASSERT(cva.register_on_change != NULL);
  ASSERT(cva.register_on_touched != NULL);
  ASSERT(cva.set_disabled_state != NULL);

  /* CVA NULL checks */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.write_value(NULL, (union ui_signal_payload){.ptr_val = NULL}));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_change(NULL, dummy_cva_on_change, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_touched(NULL, dummy_cva_on_touched, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.set_disabled_state(NULL, 1));

  int cva_change_cnt = 0;
  int cva_touched_cnt = 0;

  ASSERT_EQ(UI_ERROR_NONE,
            cva.register_on_change(autocomplete, dummy_cva_on_change,
                                   &cva_change_cnt));
  ASSERT_EQ(UI_ERROR_NONE,
            cva.register_on_touched(autocomplete, dummy_cva_on_touched,
                                    &cva_touched_cnt));

  /* Write value */
  union ui_signal_payload val;
  val.ptr_val = (void *)"Hello";
  ASSERT_EQ(UI_ERROR_NONE, cva.write_value(autocomplete, val));
  val.ptr_val = NULL;
  ASSERT_EQ(UI_ERROR_NONE, cva.write_value(autocomplete, val)); /* Sets to "" */

  /* Set disabled */
  ASSERT_EQ(UI_ERROR_NONE, cva.set_disabled_state(autocomplete, 1));
  ASSERT_EQ(UI_ERROR_NONE, cva.set_disabled_state(autocomplete, 0));

  /* Event handling when open */
  struct ui_layout_node layout;
  memset(&layout, 0, sizeof(layout));

  struct ui_overlay_director *director = NULL;
  struct ui_focus_manager *focus = NULL;
  struct ui_dom_node *dummy_root = NULL;
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dummy_root);
  ASSERT_EQ(UI_ERROR_NONE, ui_overlay_director_create(dummy_root, &director));
  ASSERT_EQ(UI_ERROR_NONE, ui_focus_manager_create(&focus));
  ASSERT(director != NULL);
  ASSERT(focus != NULL);
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_set_overlay_dependencies(
                               autocomplete, director, focus));

  /* Open should succeed */
  rc = ui_autocomplete_base_open(autocomplete, &layout, 100, 100);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_autocomplete_base_open(autocomplete, &layout, 100, 100);
  ASSERT_EQ(UI_ERROR_NONE, rc); /* Already open */

  /* Send keys to test open behavior */
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  /* Listbox navigation */
  ev.event_data.keyboard.key_code = UI_KEY_DOWN;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  ev.event_data.keyboard.key_code = UI_KEY_UP;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  ev.event_data.keyboard.key_code = UI_KEY_END;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Send non-listbox key */
  ev.event_data.keyboard.key_code = 'B';
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Selection with ENTER */
  ui_autocomplete_base_get_listbox(autocomplete, &tmp_listbox);
  struct ui_selection_model *model = NULL;
  ui_listbox_base_get_selection_model(tmp_listbox, &model);
  ui_listbox_base_set_item_count(tmp_listbox, 1);
  ui_listbox_base_set_active_index(tmp_listbox, 0);

  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Set active index to -1 to trigger remaining branch */
  g_ac_mock_fail = 10;
  ASSERT_EQ(UI_ERROR_UNKNOWN,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));
  g_ac_mock_fail = 11;
  ASSERT_EQ(UI_ERROR_UNKNOWN,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));
  g_ac_mock_fail = 0;
  ev.type = UI_EVENT_KEY_UP;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));
  ev.type = UI_EVENT_KEY_DOWN;
  ui_listbox_base_set_active_index(tmp_listbox, -1);
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Send unhandled event while open */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Escape to close */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_process_event(autocomplete, &ev, 100.0));

  /* Open again and close explicitly to hit the is_open check in close() */
  ASSERT_EQ(UI_ERROR_NONE,
            ui_autocomplete_base_open(autocomplete, &layout, 100, 100));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_close(autocomplete));

  (void)ui_overlay_director_destroy(director);
  (void)ui_focus_manager_destroy(focus);
  (void)ui_autocomplete_base_destroy(autocomplete);
  (void)ui_dom_node_destroy(dummy_root);

  /* OOM loop for create */
  for (i = 0; i < 1000; i++) {
    struct ui_autocomplete_base *test_ac = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_autocomplete_base_create(&test_ac, NULL);
    if (rc == UI_ERROR_NONE) {
      (void)ui_autocomplete_base_destroy(test_ac);
      break;
    } else {
      ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    }
  }
  g_malloc_fail_countdown = -1;
  PASS();
}

TEST test_autocomplete_main(void) {
  struct ui_autocomplete_base *autocomplete = NULL;
  ui_error_t rc;
  int my_data = 0;
  struct ui_event ev;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_autocomplete_base_create(NULL, NULL));

  rc = ui_autocomplete_base_create(&autocomplete, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(autocomplete != NULL);

  {
    struct ui_component *tmp_comp;
    struct ui_input_base *tmp_input;
    struct ui_listbox_base *tmp_listbox;

    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_component(NULL, &tmp_comp));
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_component(autocomplete, NULL));
    ASSERT_EQ(UI_ERROR_NONE,
              ui_autocomplete_base_get_component(autocomplete, &tmp_comp));
    ASSERT(tmp_comp != NULL);

    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_input(NULL, &tmp_input));
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_input(autocomplete, NULL));
    ASSERT_EQ(UI_ERROR_NONE,
              ui_autocomplete_base_get_input(autocomplete, &tmp_input));
    ASSERT(tmp_input != NULL);

    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_listbox(NULL, &tmp_listbox));
    ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
              ui_autocomplete_base_get_listbox(autocomplete, NULL));
    ASSERT_EQ(UI_ERROR_NONE,
              ui_autocomplete_base_get_listbox(autocomplete, &tmp_listbox));
    ASSERT(tmp_listbox != NULL);
  }

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_autocomplete_base_set_overlay_dependencies(NULL, NULL, NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_set_overlay_dependencies(
                               autocomplete, NULL, NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_autocomplete_base_set_on_text_change(
                                           NULL, on_text_change, &my_data));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_set_on_text_change(
                               autocomplete, on_text_change, &my_data));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_autocomplete_base_set_on_selection(
                                           NULL, on_selection, &my_data));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_set_on_selection(
                               autocomplete, on_selection, &my_data));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_autocomplete_base_process_event(NULL, &ev, 0.0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_autocomplete_base_process_event(autocomplete, NULL, 0.0));

  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'A';

  rc = ui_autocomplete_base_process_event(autocomplete, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Since we passed a char to input_base, text_change should have fired */
  ASSERT(text_change_count > 0);
  ASSERT(my_data > 0);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_autocomplete_base_open(NULL, NULL, 100, 100));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_autocomplete_base_open(autocomplete, NULL, 100, 100));

  /* Testing close */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_autocomplete_base_close(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_close(autocomplete));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_autocomplete_base_destroy(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_autocomplete_base_destroy(autocomplete));

  PASS();
}

SUITE(ui_autocomplete_suite) {
  RUN_TEST(test_autocomplete_main);
  RUN_TEST(test_autocomplete_edge_cases);
  test_autocomplete_process_event_explicit();
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_ac_coverage(void);
  run_ac_coverage();
#endif

  RUN_SUITE(ui_autocomplete_suite);

  GREATEST_MAIN_END();
}
