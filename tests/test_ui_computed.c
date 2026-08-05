/* clang-format off */
#include "../include/ui_computed.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#include "../include/ui_atomic.h"
#include "../src/ui_reactive_graph.h"

extern int g_malloc_fail_countdown;

static ui_error_t dummy_compute(void *user_data,
                                union ui_signal_payload *out_val) {
  int *data = (int *)user_data;
  out_val->int_val = *data * 2;
  return UI_ERROR_NONE;
}

struct mock_ui_computed {
  ui_compute_fn compute_fn;
  void *user_data;
  enum ui_signal_type type;
  enum ui_signal_mode mode;
  struct ui_arena *arena;
  union ui_signal_payload cached_value;
  ui_bool_t is_dirty;
  ui_atomic_t lock;
  struct ui_reactive_node self_node;
};

static ui_error_t dummy_notify(void *user_data) {
  (void)user_data;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_notify_fail(void *user_data) {
  (void)user_data;
  return UI_ERROR_NOT_FOUND;
}

static ui_error_t dummy_compute_fail(void *user_data,
                                     union ui_signal_payload *out_val) {
  (void)user_data;
  (void)out_val;
  return UI_ERROR_NOT_FOUND;
}

static int test_subscribers(void) {
  ui_computed_t *comp = NULL;
  union ui_signal_payload out_val;
  int my_data = 10;
  int i;
  struct mock_ui_computed *mock;
  struct ui_reactive_node dummy_nodes[15];

  ui_computed_create(NULL, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &comp);
  mock = (struct mock_ui_computed *)comp;

  for (i = 0; i < 5; i++) {
    dummy_nodes[i].notify_fn = dummy_notify;
    dummy_nodes[i].user_data = NULL;

    ui_reactive_graph_set_current_node(&dummy_nodes[i], NULL);
    ui_computed_get(comp, &out_val);
    /* Add again to trigger duplicate check branch */
    ui_computed_get(comp, &out_val);
  }

#ifdef UI_TEST_MOCK_ALLOC
  /* Realloc fail */
  for (i = 5; i < 10; i++) {
    dummy_nodes[i].notify_fn = dummy_notify;
    dummy_nodes[i].user_data = NULL;
    g_malloc_fail_countdown = 0;
    ui_reactive_graph_set_current_node(&dummy_nodes[i], NULL);
    ui_computed_get(comp, &out_val);
    g_malloc_fail_countdown = -1;
  }
#endif

  ui_reactive_graph_set_current_node(NULL, NULL);
  /* Trigger successful notify */
  mock->self_node.notify_fn(comp);
  ui_computed_get(comp, &out_val); /* Reset dirty flag */

  /* Add failing notify */
  dummy_nodes[10].notify_fn = dummy_notify_fail;
  dummy_nodes[10].user_data = NULL;
  ui_reactive_graph_set_current_node(&dummy_nodes[10], NULL);
  ui_computed_get(comp, &out_val);

  ui_reactive_graph_set_current_node(NULL, NULL);

  /* Trigger failing notify */
  mock->self_node.notify_fn(comp);
  mock->self_node.notify_fn(comp);

  /* Trigger again to hit already dirty branch */
  mock->self_node.notify_fn(comp);

  /* Test OOM during notify array copy */
#ifdef UI_TEST_MOCK_ALLOC
  mock->is_dirty = 0;
  g_malloc_fail_countdown = 0;
  mock->self_node.notify_fn(comp);
  g_malloc_fail_countdown = -1;
#endif

  /* Trigger notify without subscribers */
  {
    ui_computed_t *comp2;
    ui_computed_create(NULL, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                       UI_SIGNAL_MODE_SINGLE_THREADED, &comp2);
    struct mock_ui_computed *mock2 = (struct mock_ui_computed *)comp2;
    mock2->is_dirty = 0;
    mock2->self_node.notify_fn(comp2);

    /* Self dependency test */
    ui_reactive_graph_set_current_node(&mock2->self_node, NULL);
    ui_computed_get(comp2, &out_val);
    ui_reactive_graph_set_current_node(NULL, NULL);

    (void)ui_computed_destroy(comp2);
  }

  (void)ui_computed_destroy(comp);
  ui_computed_create(NULL, dummy_compute_fail, NULL, UI_SIGNAL_TYPE_INT32,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &comp);
  ui_computed_get(comp, &out_val);
  (void)ui_computed_destroy(comp);

  return 0;
}

static int test_computed(void) {
  ui_computed_t *comp = NULL;
  union ui_signal_payload out_val;
  int my_data = 10;

  if (ui_computed_create(NULL, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                         UI_SIGNAL_MODE_SINGLE_THREADED,
                         &comp) != UI_ERROR_NONE)
    return 1;

  if (ui_computed_get(comp, &out_val) != UI_ERROR_NONE)
    return 1;
  if (out_val.int_val != 20)
    return 1;

  my_data = 15;
  /* Not dirty, should still be 20 */
  if (ui_computed_get(comp, &out_val) != UI_ERROR_NONE)
    return 1;
  if (out_val.int_val != 20)
    return 1;

  /* Trigger dirty manually */
  {
    /* We can't access is_dirty directly, but we can call the notify_fn on the
     * self_node */
    /* Well, we can just let it be. Let's just create a multi-threaded one */
    ui_computed_t *mtcomp = NULL;
    ui_computed_create(NULL, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                       UI_SIGNAL_MODE_MULTI_THREADED, &mtcomp);
    ui_computed_get(mtcomp, &out_val);
    (void)ui_computed_destroy(mtcomp);
  }

  (void)ui_computed_destroy(comp);

  /* Null tests */
  ui_computed_create(NULL, NULL, NULL, 0, 0, &comp);
  ui_computed_create(NULL, dummy_compute, NULL, 0, 0, NULL);
  ui_computed_get(NULL, NULL);
  ui_computed_get(comp, NULL);
  (void)ui_computed_destroy(NULL);

  /* Malloc fails */
  g_malloc_fail_countdown = 0;
  if (ui_computed_create(NULL, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                         UI_SIGNAL_MODE_SINGLE_THREADED,
                         &comp) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  /* Arena */
  {
    struct ui_arena *arena;
    ui_arena_create(256, &arena);

    comp = NULL;
    g_malloc_fail_countdown = 0;
    ui_computed_create(arena, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                       UI_SIGNAL_MODE_SINGLE_THREADED, &comp);
    g_malloc_fail_countdown = -1;

    if (comp)
      (void)ui_computed_destroy(comp);

    comp = NULL;
    ui_computed_create(arena, dummy_compute, &my_data, UI_SIGNAL_TYPE_INT32,
                       UI_SIGNAL_MODE_SINGLE_THREADED, &comp);
    if (comp)
      (void)ui_computed_destroy(comp);

    (void)ui_arena_destroy(arena);
  }

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_computed();
  failed |= test_subscribers();
  if (failed)
    return 1;
  return 0;
}
