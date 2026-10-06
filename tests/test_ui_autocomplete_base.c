

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

static ui_error_t failing_cva_on_change(union ui_signal_payload payload,
                                        void *user_data) {
  if (payload.ptr_val || user_data) {
  }
  return UI_ERROR_UNKNOWN;
}

static ui_error_t failing_text_change(struct ui_autocomplete_base *ac,
                                      const char *text, void *user_data) {
  if (ac || text || user_data) {
  }
  return UI_ERROR_UNKNOWN;
}

static ui_error_t failing_on_selection(struct ui_autocomplete_base *a, int idx,
                                       void *u) {
  if (a || idx || u) {
  }
  return UI_ERROR_UNKNOWN;
}

static ui_error_t succeeding_on_selection(struct ui_autocomplete_base *a,
                                          int idx, void *u) {
  if (a || idx || u) {
  }
  return UI_ERROR_NONE;
}

static void test_autocomplete_process_event_explicit(void) {
  fprintf(stderr, "RUNNING explicit\n");
  struct ui_autocomplete_base *ac = NULL;
  struct ui_layout_node dummy_layout;
  struct ui_event ev;
  ui_error_t rc;

  memset(&dummy_layout, 0, sizeof(dummy_layout));

  /* Test create failures */
  g_ac_mock_fail = 1;
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  g_ac_mock_fail = 2;
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  g_ac_mock_fail = 5;
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  g_ac_mock_fail = 15;
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  g_ac_mock_fail = 16;
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Successful create for open/close/event testing */
  rc = ui_autocomplete_base_create(&ac, NULL);
  assert(rc == UI_ERROR_NONE);

  /* Open failure: popover_is_open fails */
  g_ac_mock_fail = 12;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Open failure: listbox get_comp fails */
  g_ac_mock_fail = 17;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Open failure: popover open fails */
  g_ac_mock_fail = 7;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Open failure: set_attribute fails */
  extern int g_ac_mock_is_open;
  g_ac_mock_is_open = 0;
  g_ac_mock_fail = 8;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Successfully open */
  g_ac_mock_is_open = 0;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_NONE);

  /* Open again when already open */
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_NONE);

  /* Close failure: popover_is_open fails */
  g_ac_mock_fail = 12;
  rc = ui_autocomplete_base_close(ac);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Close failure: popover_close fails */
  g_ac_mock_fail = 9;
  rc = ui_autocomplete_base_close(ac);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Close failure: set_attribute fails */
  g_ac_mock_fail = 8;
  rc = ui_autocomplete_base_close(ac);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Re-open for process_event tests */
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_NONE);

  /* process_event: popover_is_open fails */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'A';
  g_ac_mock_fail = 12;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* process_event: popover_process_event fails */
  g_ac_mock_fail = 13;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* process_event: second popover_is_open fails */
  extern int g_popover_is_open_calls;
  g_popover_is_open_calls = 0;
  g_ac_mock_fail = 19;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* process_event: mouse click outside closes popover, set_attribute fails */
  ev.type = UI_EVENT_MOUSE_DOWN;
  g_ac_mock_fail = 8;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Re-open for keyboard tests */
  g_ac_mock_is_open = 0;
  rc = ui_autocomplete_base_open(ac, &dummy_layout, 100.0f, 100.0f);
  assert(rc == UI_ERROR_NONE);
  g_ac_mock_is_open = 1;

  /* ENTER key: get_active_index fails */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  g_ac_mock_fail = 18;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* ENTER key: get_selection_model fails */
  g_ac_mock_fail = 10;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* ENTER key: select fails */
  g_ac_mock_fail = 11;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Successfully handle ENTER */
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_NONE);

  /* Trigger the error branches in on_listbox_selection_change directly */
  extern ui_error_t (*g_ac_captured_on_change)(struct ui_selection_model *,
                                               void *);
  if (g_ac_captured_on_change) {
    struct ui_listbox_base *lb = NULL;
    ui_autocomplete_base_get_listbox(ac, &lb);
    struct ui_selection_model *model = NULL;
    ui_listbox_base_get_selection_model(lb, &model);

    /* 1. !ac */
    rc = g_ac_captured_on_change(model, NULL);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);

    /* 2. get_selected_count fails */
    g_ac_mock_fail = 20;
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_UNKNOWN);
    g_ac_mock_fail = 0;

    /* Make sure count > 0 so that get_selected is called */
    ui_listbox_base_set_item_count(lb, 1);
    ui_selection_model_select(model, (void *)0);

    /* 3. get_selected fails */
    g_ac_mock_fail = 21;
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_UNKNOWN);
    g_ac_mock_fail = 0;

    /* 4. ac->on_selection fails */
    ui_autocomplete_base_set_on_selection(ac, failing_on_selection, NULL);
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_UNKNOWN);
    ui_autocomplete_base_set_on_selection(ac, NULL, NULL);

    ui_selection_model_clear(model);
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_NONE);

    /* 5. ac->on_selection succeeds */
    ui_selection_model_select(model, (void *)0);
    ui_autocomplete_base_set_on_selection(ac, succeeding_on_selection, NULL);
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_NONE);
    ui_autocomplete_base_set_on_selection(ac, NULL, NULL);

    ui_selection_model_clear(model);
    rc = g_ac_captured_on_change(model, ac);
    assert(rc == UI_ERROR_NONE);
  }

  /* Fallback: when popover is closed, input_process_event fails */
  rc = ui_autocomplete_base_close(ac);
  assert(rc == UI_ERROR_NONE);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 'B';
  g_ac_mock_fail = 14;
  rc = ui_autocomplete_base_process_event(ac, &ev, 100.0);
  assert(rc == UI_ERROR_UNKNOWN);
  g_ac_mock_fail = 0;

  /* Test get_selection_model fails in create (10) */
  g_ac_mock_fail = 10;
  {
    struct ui_autocomplete_base *ac_temp = NULL;
    rc = ui_autocomplete_base_create(&ac_temp, NULL);
    assert(rc == UI_ERROR_UNKNOWN);
  }
  g_ac_mock_fail = 0;

  /* Test set_on_change fails in create (22) */
  g_ac_mock_fail = 22;
  {
    struct ui_autocomplete_base *ac_temp = NULL;
    rc = ui_autocomplete_base_create(&ac_temp, NULL);
    assert(rc == UI_ERROR_UNKNOWN);
  }
  g_ac_mock_fail = 0;

  /* Test on_input_text_change callback errors */
  {
    struct ui_control_value_accessor cva;
    struct ui_input_base *inp = NULL;
    ui_autocomplete_base_destroy(ac);
    ac = NULL;

    rc = ui_autocomplete_base_create(&ac, &cva);
    assert(rc == UI_ERROR_NONE);

    rc = cva.register_on_change(ac, failing_cva_on_change, NULL);
    assert(rc == UI_ERROR_NONE);
    rc = ui_autocomplete_base_get_input(ac, &inp);
    assert(rc == UI_ERROR_NONE);

    /* Changing text causes failing_cva_on_change to return error */
    rc = ui_input_base_set_text(inp, "fail");
    assert(rc == UI_ERROR_UNKNOWN);

    /* Unset cva_on_change, test on_text_change failure */
    rc = cva.register_on_change(ac, NULL, NULL);
    assert(rc == UI_ERROR_NONE);
    rc = ui_autocomplete_base_set_on_text_change(ac, failing_text_change, NULL);
    assert(rc == UI_ERROR_NONE);

    rc = ui_input_base_set_text(inp, "fail2");
    assert(rc == UI_ERROR_UNKNOWN);
  }

  ui_autocomplete_base_destroy(ac);
}

extern int g_malloc_fail_countdown;

static int text_change_count = 0;
static int selection_count = 0;

static ui_error_t on_text_change(struct ui_autocomplete_base *autocomplete,
                                 const char *text, void *user_data) {
  if (autocomplete) {
  }
  if (text) {
  }
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  text_change_count++;
  return UI_ERROR_NONE;
}

static ui_error_t on_selection(struct ui_autocomplete_base *autocomplete,
                               int index, void *user_data) {
  if (autocomplete) {
  }
  if (index) {
  }
  if (user_data) {
    int *val = (int *)user_data;
    (*val)++;
  }
  selection_count++;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_cva_on_change(union ui_signal_payload payload,
                                      void *user_data) {
  if (payload.ptr_val) {
  }
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
  union ui_signal_payload dummy_payload;
  ui_error_t rc;
  int i;
  int cva_change_cnt = 0;
  int cva_touched_cnt = 0;

  memset(&cva, 0, sizeof(cva));
  memset(&dummy_payload, 0, sizeof(dummy_payload));

  rc = ui_autocomplete_base_create(&autocomplete, &cva);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* CVA methods */
  ASSERT(cva.write_value != NULL);
  ASSERT(cva.register_on_change != NULL);
  ASSERT(cva.register_on_touched != NULL);
  ASSERT(cva.set_disabled_state != NULL);

  /* CVA NULL checks */
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.write_value(NULL, dummy_payload));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_change(NULL, dummy_cva_on_change, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            cva.register_on_touched(NULL, dummy_cva_on_touched, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, cva.set_disabled_state(NULL, 1));

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

  {
    ui_error_t rc_cleanup = ui_overlay_director_destroy(director);
    ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
  }
  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus);
    ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
  }
  {
    ui_error_t rc_cleanup = ui_autocomplete_base_destroy(autocomplete);
    ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(dummy_root);
    ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
  }

  /* OOM loop for create */
  for (i = 0; i < 1000; i++) {
    struct ui_autocomplete_base *test_ac = NULL;
    g_malloc_fail_countdown = i;
    rc = ui_autocomplete_base_create(&test_ac, NULL);
    if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_autocomplete_base_destroy(test_ac);
        ASSERT_EQ(UI_ERROR_NONE, rc_cleanup);
      }
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
  assert(run_ac_coverage() == UI_ERROR_NONE);
#endif

  RUN_SUITE(ui_autocomplete_suite);

  GREATEST_MAIN_END();
}
