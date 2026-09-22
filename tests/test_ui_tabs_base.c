/* clang-format off */
#include "greatest.h"
#include "ui_tabs_base.h"
#include "ui_error.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_signal.h"
#include "ui_test_mock_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_tab_entry {
  char *id;
  struct ui_dom_node *header_node;
  struct ui_dom_node *panel_node;
};

struct ui_tabs_base {
  struct ui_component *component;
  struct ui_dom_node *tablist_node;
  struct ui_dom_node *panels_node;
  struct ui_tab_entry *tabs;
  int tab_count;
  int tab_capacity;
  int active_index;
  ui_tabs_on_change_t on_change;
  void *user_data;
  struct ui_signal *active_index_signal;
};

extern int g_malloc_fail_countdown;
extern int g_tabs_mock_fail;
extern int g_tabs_mock_append_fail_target;
extern int g_tabs_mock_set_attr_fail_target;
extern int g_tabs_mock_remove_attr_fail_target;

static int g_cb_called = 0;

static ui_error_t mock_on_change(struct ui_tabs_base *tabs, int new_index,
                                 void *user_data) {
  int unused_idx = new_index;
  void *unused_ud = user_data;
  struct ui_tabs_base *unused_t = tabs;
  tabs = unused_t;
  new_index = unused_idx;
  user_data = unused_ud;
  g_cb_called++;
  return UI_ERROR_NONE;
}

static ui_error_t mock_on_change_fail(struct ui_tabs_base *tabs, int new_index,
                                      void *user_data) {
  int unused_idx = new_index;
  void *unused_ud = user_data;
  struct ui_tabs_base *unused_t = tabs;
  tabs = unused_t;
  new_index = unused_idx;
  user_data = unused_ud;
  return UI_ERROR_UNKNOWN;
}

TEST test_tabs_invalid_args(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *p = NULL;
  struct ui_event ev;
  int idx = -1;
  ui_error_t rc;

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tabs_base_create(NULL));
  ASSERT_EQ(UI_ERROR_NONE, ui_tabs_base_destroy(NULL));

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tabs_base_get_component(NULL, &comp));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_bind_active_index(NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_add_tab(NULL, "t1", NULL, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tabs_base_set_active_index(NULL, 0));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_get_active_index(NULL, &idx));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_set_on_change(NULL, NULL, NULL));

  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_process_event(NULL, &ev, 0.0));

  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tabs_base_get_component(tabs, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_get_active_index(tabs, NULL));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_process_event(tabs, NULL, 0.0));

  /* Process event when empty tab count returns NONE */
  rc = ui_tabs_base_process_event(tabs, &ev, 0.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT, ui_tabs_base_add_tab(tabs, NULL, h, p));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_add_tab(tabs, "t1", NULL, p));
  ASSERT_EQ(UI_ERROR_INVALID_ARGUMENT,
            ui_tabs_base_add_tab(tabs, "t1", h, NULL));

  rc = ui_dom_node_destroy(h);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_dom_node_destroy(p);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_tabs_base_destroy(tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tabs_lifecycle_and_events(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *h[6];
  struct ui_dom_node *p[6];
  struct ui_event ev;
  int idx = -1;
  ui_error_t rc;
  int i;

  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  rc = ui_tabs_base_get_component(tabs, &comp);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT(comp != NULL);

  rc = ui_tabs_base_bind_active_index(tabs, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Add 5 tabs to test reallocation capacity growth (4 -> 8) */
  for (i = 0; i < 5; i++) {
    char tid[16];
    sprintf(tid, "tab%d", i);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_add_tab(tabs, tid, h[i], p[i]);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* Setting same index is no-op */
  rc = ui_tabs_base_set_active_index(tabs, 0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Change active index while on_change is NULL (covers false branch) */
  rc = ui_tabs_base_set_active_index(tabs, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, idx);

  /* Set on_change callback */
  rc = ui_tabs_base_set_on_change(tabs, mock_on_change, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Set out of bounds */
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, ui_tabs_base_set_active_index(tabs, -1));
  ASSERT_EQ(UI_ERROR_OUT_OF_BOUNDS, ui_tabs_base_set_active_index(tabs, 5));

  /* Change active index */
  g_cb_called = 0;
  rc = ui_tabs_base_set_active_index(tabs, 2);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(1, g_cb_called);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  /* Process events */
  memset(&ev, 0, sizeof(ev));

  /* Non key event */
  ev.type = UI_EVENT_MOUSE_DOWN;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Unhandled key event */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_UP;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* Right arrow */
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(3, idx);

  /* Left arrow */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(2, idx);

  /* Home key */
  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* Left arrow at 0 wraps to end */
  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, idx);

  /* Right arrow at 4 wraps to 0 */
  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(0, idx);

  /* End key */
  ev.event_data.keyboard.key_code = UI_KEY_END;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_get_active_index(tabs, &idx);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  ASSERT_EQ(4, idx);

  rc = ui_tabs_base_destroy(tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tabs_error_branches(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *p = NULL;
  struct ui_event ev;
  ui_error_t rc;
  int i;

  /* 1. Append failures in create */
  for (i = 1; i <= 2; i++) {
    g_tabs_mock_append_fail_target = i;
    tabs = NULL;
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    ASSERT(tabs == NULL);
  }
  g_tabs_mock_append_fail_target = 0;

  /* 2. Component set default style failure in create */
  g_tabs_mock_fail = 1;
  tabs = NULL;
  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tabs_mock_fail = 0;

  /* 3. Cleanup failures in create */
  for (i = 2; i <= 3; i++) {
    g_tabs_mock_append_fail_target = 1;
    g_tabs_mock_fail = i;
    tabs = NULL;
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_fail = 0;
    g_tabs_mock_append_fail_target = 0;
  }

  /* 4. Component destroy failure in destroy */
  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  g_tabs_mock_fail = 3;
  rc = ui_tabs_base_destroy(tabs);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tabs_mock_fail = 0;

  /* 5. Destroy with tabs->component == NULL */
  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_component_destroy(tabs->component);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  tabs->component = NULL;
  rc = ui_tabs_base_destroy(tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  /* 6. Append failures in add_tab */
  for (i = 1; i <= 2; i++) {
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_tabs_mock_append_fail_target = i;
    rc = ui_tabs_base_add_tab(tabs, "t", h, p);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_append_fail_target = 0;
    if (i == 1) {
      rc = ui_dom_node_destroy(h);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    rc = ui_dom_node_destroy(p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 7. Set attribute mock failures in add_tab (first tab: 8 calls) */
  for (i = 1; i <= 8; i++) {
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_tabs_mock_set_attr_fail_target = i;
    rc = ui_tabs_base_add_tab(tabs, "t", h, p);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_set_attr_fail_target = 0;
    rc = ui_dom_node_destroy(h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 8. Set attribute mock failures in add_tab (second tab: calls 7, 8, 9) */
  for (i = 7; i <= 9; i++) {
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_add_tab(tabs, "t1", h, p);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_tabs_mock_set_attr_fail_target = i;
    rc = ui_tabs_base_add_tab(tabs, "t2", h, p);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_set_attr_fail_target = 0;
    rc = ui_dom_node_destroy(h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 9. format_id mock failures */
  for (i = 4; i <= 5; i++) {
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    g_tabs_mock_fail = i;
    rc = ui_tabs_base_add_tab(tabs, "t", h, p);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_fail = 0;
    rc = ui_dom_node_destroy(h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_destroy(p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* 10. Failures in set_active_index: set_attribute, remove_attribute,
   * on_change */
  rc = ui_tabs_base_create(&tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  for (i = 0; i < 2; i++) {
    char tid[16];
    sprintf(tid, "t%d", i);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_tabs_base_add_tab(tabs, tid, h, p);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }

  /* mock set attribute in set_active_index */
  for (i = 1; i <= 5; i++) {
    g_tabs_mock_set_attr_fail_target = i;
    rc = ui_tabs_base_set_active_index(tabs, 1);
    ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
    g_tabs_mock_set_attr_fail_target = 0;
  }

  /* mock remove attribute in set_active_index */
  g_tabs_mock_remove_attr_fail_target = 1;
  rc = ui_tabs_base_set_active_index(tabs, 1);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tabs_mock_remove_attr_fail_target = 0;

  /* mock remove attribute target 2 to cover non-failing branch */
  g_tabs_mock_remove_attr_fail_target = 2;
  rc = ui_tabs_base_set_active_index(tabs, 1);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_set_active_index(tabs, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);
  g_tabs_mock_remove_attr_fail_target = 0;

  /* on_change error in set_active_index */
  rc = ui_tabs_base_set_on_change(tabs, mock_on_change_fail, NULL);
  ASSERT_EQ(UI_ERROR_NONE, rc);
  rc = ui_tabs_base_set_active_index(tabs, 0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  /* Process event failures when set_active_index fails (lines 443, 452, 456,
   * 460) */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;

  ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.event_data.keyboard.key_code = UI_KEY_HOME;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.event_data.keyboard.key_code = UI_KEY_END;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  ev.event_data.keyboard.key_code = UI_KEY_LEFT;
  rc = ui_tabs_base_process_event(tabs, &ev, 1.0);
  ASSERT_EQ(UI_ERROR_UNKNOWN, rc);

  rc = ui_tabs_base_destroy(tabs);
  ASSERT_EQ(UI_ERROR_NONE, rc);

  PASS();
}

TEST test_tabs_oom(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_dom_node *h = NULL;
  struct ui_dom_node *p = NULL;
  ui_error_t rc;
  int i;

  /* Creation OOM loop */
  for (i = 0; i < 100; i++) {
    g_malloc_fail_countdown = i;
    tabs = NULL;
    rc = ui_tabs_base_create(&tabs);
    if (rc == UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      rc = ui_tabs_base_destroy(tabs);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    ASSERT(tabs == NULL);
  }
  g_malloc_fail_countdown = -1;

  /* Add tab OOM loop */
  for (i = 0; i < 50; i++) {
    rc = ui_tabs_base_create(&tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h);
    ASSERT_EQ(UI_ERROR_NONE, rc);
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &p);
    ASSERT_EQ(UI_ERROR_NONE, rc);

    g_malloc_fail_countdown = i;
    rc = ui_tabs_base_add_tab(tabs, "t_oom", h, p);
    g_malloc_fail_countdown = -1;

    if (rc == UI_ERROR_NONE) {
      rc = ui_tabs_base_destroy(tabs);
      ASSERT_EQ(UI_ERROR_NONE, rc);
      break;
    }
    ASSERT_EQ(UI_ERROR_OUT_OF_MEMORY, rc);
    if (!h->parent) {
      rc = ui_dom_node_destroy(h);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    if (!p->parent) {
      rc = ui_dom_node_destroy(p);
      ASSERT_EQ(UI_ERROR_NONE, rc);
    }
    rc = ui_tabs_base_destroy(tabs);
    ASSERT_EQ(UI_ERROR_NONE, rc);
  }
  g_malloc_fail_countdown = -1;

  PASS();
}

SUITE(ui_tabs_base_suite) {
  RUN_TEST(test_tabs_invalid_args);
  RUN_TEST(test_tabs_lifecycle_and_events);
  RUN_TEST(test_tabs_error_branches);
  RUN_TEST(test_tabs_oom);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ui_tabs_base_suite);
  GREATEST_MAIN_END();
}
