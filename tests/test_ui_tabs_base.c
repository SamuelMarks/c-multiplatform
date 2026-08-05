/* clang-format off */
#include "ui_tabs_base.h"
#include "ui_error.h"
#include "../src/ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif

#define ASSERT_SUCCESS(expr)                                                   \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != UI_ERROR_NONE) {                                               \
      printf("Failed at line %d: %d\n", __LINE__, _err);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(expr, expected)                                              \
  do {                                                                         \
    ui_error_t _err = (expr);                                                  \
    if (_err != (expected)) {                                                  \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             _err);                                                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)

#define ASSERT_INT_EQ(expr, expected)                                          \
  do {                                                                         \
    int val = (expr);                                                          \
    if (val != (expected)) {                                                   \
      printf("Failed at line %d: expected %d, got %d\n", __LINE__, (expected), \
             val);                                                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)

static int test_ui_tabs_base_create_destroy(void) {
  struct ui_tabs_base *tabs = NULL;

  ASSERT_EQ(ui_tabs_base_create(NULL), UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");
  if (!tabs)
    return 1;

  (void)ui_tabs_base_destroy(tabs);
  (void)ui_tabs_base_destroy(NULL);
  return 0;
}

static int test_ui_tabs_base_get_component(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_component *comp = NULL;

  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");
  ASSERT_EQ(ui_tabs_base_get_component(NULL, &comp), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_get_component(tabs, NULL), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_tabs_base_get_component(tabs, &comp));

  (void)ui_tabs_base_destroy(tabs);
  return 0;
}

static int g_cb_called = 0;
static ui_error_t mock_on_change(struct ui_tabs_base *tabs, int new_index,
                                 void *user_data) {
  (void)tabs;
  (void)new_index;
  (void)user_data;
  g_cb_called++;
  return UI_ERROR_NONE;
}

static int test_ui_tabs_base_add_and_navigate(void) {
  struct ui_tabs_base *tabs = NULL;
  struct ui_dom_node *h1 = NULL, *c1 = NULL, *h2 = NULL, *c2 = NULL, *h3 = NULL,
                     *c3 = NULL;
  int index;

  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");

  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h2));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c2));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h3));
  ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c3));

  ASSERT_EQ(ui_tabs_base_add_tab(NULL, "t1", h1, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_add_tab(tabs, NULL, h1, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_add_tab(tabs, "t1", NULL, c1),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_add_tab(tabs, "t1", h1, NULL),
            UI_ERROR_INVALID_ARGUMENT);

  ASSERT_SUCCESS(ui_tabs_base_add_tab(tabs, "t1", h1, c1));
  ASSERT_SUCCESS(ui_tabs_base_add_tab(tabs, "t2", h2, c2));
  ASSERT_SUCCESS(ui_tabs_base_add_tab(tabs, "t3", h3, c3));

  /* Test 4+ to hit expansion branch if capacity=4 initial but realloc logic
   * tested below */
  {
    struct ui_dom_node *h4 = NULL, *c4 = NULL;
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h4));
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c4));
    ASSERT_SUCCESS(ui_tabs_base_add_tab(tabs, "t4", h4, c4));
  }
  {
    struct ui_dom_node *h5 = NULL, *c5 = NULL;
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h5));
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c5));
    ASSERT_SUCCESS(ui_tabs_base_add_tab(tabs, "t5", h5, c5));
  }

  ASSERT_EQ(ui_tabs_base_get_active_index(NULL, &index),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_get_active_index(tabs, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
  ASSERT_INT_EQ(index, 0);

  ASSERT_EQ(ui_tabs_base_set_on_change(NULL, mock_on_change, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_tabs_base_set_on_change(tabs, mock_on_change, NULL));

  ASSERT_EQ(ui_tabs_base_set_active_index(NULL, 1), UI_ERROR_INVALID_ARGUMENT);
  ASSERT_EQ(ui_tabs_base_set_active_index(tabs, -1), UI_ERROR_OUT_OF_BOUNDS);
  ASSERT_EQ(ui_tabs_base_set_active_index(tabs, 999), UI_ERROR_OUT_OF_BOUNDS);

  ASSERT_SUCCESS(ui_tabs_base_set_active_index(tabs, 1));
  ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
  ASSERT_INT_EQ(index, 1);
  ASSERT_INT_EQ(g_cb_called, 1);

  ASSERT_SUCCESS(ui_tabs_base_set_active_index(tabs, 1)); /* No change */
  ASSERT_INT_EQ(g_cb_called, 1);                          /* Still 1 */

  /* Set active index without hook */
  ASSERT_SUCCESS(ui_tabs_base_set_on_change(tabs, NULL, NULL));
  ASSERT_SUCCESS(ui_tabs_base_set_active_index(tabs, 0));
  ASSERT_SUCCESS(ui_tabs_base_set_on_change(tabs, mock_on_change, NULL));

  /* Process Event Navigation */
  {
    struct ui_event ev;
    ev.type = UI_EVENT_KEY_DOWN;

    ev.event_data.keyboard.key_code = UI_KEY_RIGHT;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
    ASSERT_INT_EQ(index, 1);

    ev.event_data.keyboard.key_code = UI_KEY_LEFT;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
    ASSERT_INT_EQ(index, 0);

    ev.event_data.keyboard.key_code = UI_KEY_LEFT;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ev.event_data.keyboard.key_code = UI_KEY_LEFT; /* wraparound */
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
    ASSERT_INT_EQ(index, 3);

    ev.event_data.keyboard.key_code = UI_KEY_HOME;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
    ASSERT_INT_EQ(index, 0);

    ev.event_data.keyboard.key_code = UI_KEY_END;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_SUCCESS(ui_tabs_base_get_active_index(tabs, &index));
    ASSERT_INT_EQ(index, 4);

    /* Edge cases for process_event */
    ev.event_data.keyboard.key_code = UI_KEY_UP; /* Ignore */
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ev.type = UI_EVENT_MOUSE_DOWN; /* Ignore */
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
    ASSERT_EQ(ui_tabs_base_process_event(NULL, &ev, 0.0),
              UI_ERROR_INVALID_ARGUMENT);
    ASSERT_EQ(ui_tabs_base_process_event(tabs, NULL, 0.0),
              UI_ERROR_INVALID_ARGUMENT);
  }

  (void)ui_tabs_base_destroy(tabs);

  /* Empty tabs process_event */
  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");
  {
    struct ui_event ev;
    ev.type = UI_EVENT_KEY_DOWN;
    ASSERT_SUCCESS(ui_tabs_base_process_event(tabs, &ev, 0.0));
  }
  (void)ui_tabs_base_destroy(tabs);

  return 0;
}

static int test_ui_tabs_base_bindings(void) {
  struct ui_tabs_base *tabs = NULL;

  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");
  ASSERT_EQ(ui_tabs_base_bind_active_index(NULL, NULL),
            UI_ERROR_INVALID_ARGUMENT);
  ASSERT_SUCCESS(ui_tabs_base_bind_active_index(tabs, NULL));
  (void)ui_tabs_base_destroy(tabs);
  return 0;
}

static int test_ui_tabs_base_allocation_failures(void) {
#ifdef UI_TEST_MOCK_ALLOC
  struct ui_tabs_base *tabs = NULL;
  struct ui_dom_node *h1 = NULL, *c1 = NULL;
  int i;
  ui_error_t err;

  /* Fail split button struct alloc */
  g_malloc_fail_countdown = 0;
  ASSERT_EQ(ui_tabs_base_create(&tabs), UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  for (i = 1; i < 2000; ++i) {
    g_malloc_fail_countdown = i;
    err = ui_tabs_base_create(&tabs);
    g_malloc_fail_countdown = -1;
    if (err == UI_ERROR_NONE) {
      (void)ui_tabs_base_destroy(tabs);
      break;
    }
  }

  ASSERT_SUCCESS(ui_tabs_base_create(&tabs));
  printf("created\n");

  printf("starting loop alloc fails\n");
  for (i = 0; i < 5; ++i) {
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &h1));
    ASSERT_SUCCESS(ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &c1));

    g_malloc_fail_countdown = i;
    err = ui_tabs_base_add_tab(tabs, "t1", h1, c1);
    g_malloc_fail_countdown = -1;

    if (err == UI_ERROR_NONE) {
      break;
    }

    ASSERT_EQ(err, UI_ERROR_OUT_OF_MEMORY);
    (void)ui_dom_node_destroy(h1);
    (void)ui_dom_node_destroy(c1);
  }

  (void)ui_tabs_base_destroy(tabs);
#endif
  return 0;
}

int main(void) {
  printf("start\n");
  printf("test 1\n");
  if (test_ui_tabs_base_create_destroy())
    return 1;
  printf("test 2\n");
  if (test_ui_tabs_base_get_component())
    return 1;
  printf("test 3\n");
  if (test_ui_tabs_base_add_and_navigate())
    return 1;
  printf("test 4\n");
  if (test_ui_tabs_base_bindings())
    return 1;
  printf("test 5\n");
  if (test_ui_tabs_base_allocation_failures())
    return 1;
  return 0;
}
