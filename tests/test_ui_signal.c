/* clang-format off */
#include "../include/ui_signal.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../src/ui_reactive_graph.h"
#include "../include/ui_thread_pool.h"
#include "../include/ui_atomic.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

struct ui_signal {
  union ui_signal_payload value;
  enum ui_signal_type type;
  ui_equality_fn equality_fn;
  ui_destructor_fn destructor_fn;
  enum ui_signal_mode mode;
  struct ui_arena *arena;
  ui_int32 ref_count;
  ui_atomic_t lock;

  struct ui_reactive_node **subscribers;
  size_t subscribers_count;
  size_t subscribers_capacity;
};

extern int g_malloc_fail_countdown;
static int g_eq_fail = 0;
static int g_dest_fail = 0;
static int g_notify_fail = 0;
static int g_notify_count = 0;

static ui_error_t eq_fn(union ui_signal_payload a, union ui_signal_payload b,
                        ui_bool_t *out_equal) {
  if (g_eq_fail) {
    return UI_ERROR_UNKNOWN;
  }
  *out_equal = (a.int_val == b.int_val) ? 1 : 0;
  return UI_ERROR_NONE;
}

static ui_error_t dest_fn(union ui_signal_payload payload) {
  (void)payload;
  if (g_dest_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static ui_error_t upd_fn(union ui_signal_payload current,
                         union ui_signal_payload *out_val) {
  out_val->int_val = current.int_val + 1;
  return UI_ERROR_NONE;
}

static ui_error_t fail_upd_fn(union ui_signal_payload current,
                              union ui_signal_payload *out_val) {
  (void)current;
  (void)out_val;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t notify_cb(void *user_data) {
  (void)user_data;
  g_notify_count++;
  if (g_notify_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

ui_atomic_t test_sync = 0;
static ui_error_t unlock_task(void *user_data) {
  ui_signal_t *sig = (ui_signal_t *)user_data;
  int k;
  while (test_sync == 0) {
  } /* wait for main thread */
  for (k = 0; k < 10000000; k++) {
    volatile int dummy = k; /* delay */
    (void)dummy;
  }
  return ui_atomic_store(&sig->lock, 0);
}

static int test_signal(void) {
  ui_signal_t *sig = NULL;
  union ui_signal_payload val;
  union ui_signal_payload out_val;
  struct ui_reactive_node node1 = {notify_cb, NULL};
  struct ui_reactive_node node2 = {notify_cb, NULL};
  struct ui_reactive_node node3 = {notify_cb, NULL};
  struct ui_reactive_node node4 = {notify_cb, NULL};
  struct ui_reactive_node node5 = {notify_cb, NULL};

  val.int_val = 10;

  if (ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                       UI_SIGNAL_MODE_SINGLE_THREADED, &sig) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  if (ui_signal_get(sig, &out_val) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (out_val.int_val != 10) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  val.int_val = 15;
  if (ui_signal_set(sig, val) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_signal_get(sig, &out_val) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (out_val.int_val != 15) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  val.int_val = 15; /* currently 15 after previous test */
  ui_signal_set(sig, val);

  if (ui_signal_update(sig, upd_fn) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (ui_signal_get(sig, &out_val) != UI_ERROR_NONE) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  if (out_val.int_val != 16) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Reactive graph subscribers test */
  ui_reactive_graph_set_current_node(&node1, NULL);
  ui_signal_get(sig, &out_val);
  /* Double subscribe node1 should just return */
  ui_signal_get(sig, &out_val);

  ui_reactive_graph_set_current_node(&node2, NULL);
  ui_signal_get(sig, &out_val);

  ui_reactive_graph_set_current_node(&node3, NULL);
  ui_signal_get(sig, &out_val);

  ui_reactive_graph_set_current_node(&node4, NULL);
  ui_signal_get(sig, &out_val);

  /* node5 causes capacity expansion */
  ui_reactive_graph_set_current_node(&node5, NULL);
  ui_signal_get(sig, &out_val);

  ui_reactive_graph_set_current_node(NULL, NULL);

  struct ui_reactive_node node_null = {NULL, NULL};
  ui_reactive_graph_set_current_node(&node_null, NULL);
  ui_signal_get(sig, &out_val);
  ui_reactive_graph_set_current_node(NULL, NULL);

  /* Set value to trigger notifications */
  g_notify_count = 0;
  val.int_val = 20;
  ui_signal_set(sig, val);
  if (g_notify_count != 5) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }

  /* Notify failure */
  val.int_val = 21;
  g_notify_fail = 1;
  if (ui_signal_set(sig, val) != UI_ERROR_UNKNOWN) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  g_notify_fail = 0;

  /* Subscriber copy malloc failure */
  val.int_val = 22;
  g_malloc_fail_countdown = 0;
  ui_signal_set(sig, val);
  g_malloc_fail_countdown = -1;

  /* eq_fn failure */
  val.int_val = 23;
  g_eq_fail = 1;
  if (ui_signal_set(sig, val) != UI_ERROR_UNKNOWN) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  g_eq_fail = 0;

  /* dest_fn failure */
  val.int_val = 24;
  g_dest_fail = 1;
  if (ui_signal_set(sig, val) != UI_ERROR_UNKNOWN) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  g_dest_fail = 0;

  /* Subscriber addition OOM */
  {

    struct ui_reactive_node dummies[20];
    int di;
    for (di = 0; di < 20; di++) {
      dummies[di].notify_fn = notify_cb;
      dummies[di].user_data = NULL;
      ui_reactive_graph_set_current_node(&dummies[di], NULL);
      g_malloc_fail_countdown = 0;
      ui_error_t rc_get = ui_signal_get(sig, &out_val);
      g_malloc_fail_countdown = -1;
      if (rc_get == UI_ERROR_OUT_OF_MEMORY) {
        break;
      }
    }

    ui_reactive_graph_set_current_node(NULL, NULL);
  }

  (void)ui_signal_destroy(sig);

  /* Update with failure fn */
  {
    ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
    if (ui_signal_update(sig, fail_upd_fn) != UI_ERROR_UNKNOWN) {
      printf("Failed at %d\n", __LINE__);
      return 1;
    }
    (void)ui_signal_destroy(sig);
  }

  /* Fallback eq_fn test (NULL eq_fn) */
  {
    ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
    val.int_val = 25;
    ui_signal_set(sig, val);
    (void)ui_signal_destroy(sig);
  }

  /* Multithreaded mode */
  {
    ui_signal_t *mtsig = NULL;
    val.int_val = 1;
    ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                     UI_SIGNAL_MODE_MULTI_THREADED, &mtsig);
    ui_signal_get(mtsig, &out_val);
    val.int_val = 2;
    ui_signal_set(mtsig, val);

    struct ui_thread_pool *pool;
    if (ui_thread_pool_create(1, &pool) == UI_ERROR_NONE) {

      ui_atomic_store(&mtsig->lock, 1);
      ui_thread_pool_schedule(pool, unlock_task, mtsig);
      ui_atomic_store(&test_sync, 1);
      ui_signal_get(mtsig, &out_val); /* will spin until thread unlocks */
                                      /* will spin until thread unlocks */
      ui_thread_pool_destroy(pool);
    }

    (void)ui_signal_destroy(mtsig);
  }

  /* Nulls */
  ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, NULL, NULL, 0, NULL);
  ui_signal_get(NULL, NULL);
  ui_signal_get(
      sig,
      NULL); /* Wait, sig is already destroyed, but let's test NULL signal */

  ui_signal_set(NULL, val);
  ui_signal_update(NULL, NULL);

  ui_signal_update(sig, NULL);

  (void)ui_signal_destroy(NULL);

  /* malloc fails */
  g_malloc_fail_countdown = 0;
  if (ui_signal_create(NULL, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                       UI_SIGNAL_MODE_SINGLE_THREADED,
                       &sig) != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  g_malloc_fail_countdown = -1;

  /* arena alloc */
  {
    struct ui_arena *arena;
    ui_arena_create(256, &arena);
    ui_signal_create(arena, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
    (void)ui_signal_destroy(sig);
    (void)ui_arena_destroy(arena);
  }

  /* Arena alloc fail test */
  {
    struct ui_arena *arena;
    ui_arena_create(256, &arena);
    g_malloc_fail_countdown = 0;
    ui_signal_create(arena, val, UI_SIGNAL_TYPE_INT32, eq_fn, dest_fn,
                     UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
    g_malloc_fail_countdown = -1;
    (void)ui_arena_destroy(arena);
  }

  return 0;
}

static int test_other_types(void) {
  ui_signal_t *sig_ptr = NULL;
  ui_signal_t *sig_float = NULL;
  ui_signal_t *sig_bool = NULL;
  ui_signal_t *sig_def = NULL;
  union ui_signal_payload val;

  val.ptr_val = (void *)0x1234;
  ui_signal_create(NULL, val, UI_SIGNAL_TYPE_POINTER, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_ptr);
  val.ptr_val = (void *)0x5678;
  ui_signal_set(sig_ptr, val);
  (void)ui_signal_destroy(sig_ptr);

  val.float_val = 1.0f;
  ui_signal_create(NULL, val, UI_SIGNAL_TYPE_FLOAT32, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_float);
  val.float_val = 2.0f;
  ui_signal_set(sig_float, val);
  (void)ui_signal_destroy(sig_float);

  val.bool_val = 1;
  ui_signal_create(NULL, val, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_bool);
  val.bool_val = 0;
  ui_signal_set(sig_bool, val);
  (void)ui_signal_destroy(sig_bool);

  val.ptr_val = (void *)0x1111;
  ui_signal_create(NULL, val, 999 /* unknown type */, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig_def);
  val.ptr_val = (void *)0x2222;
  ui_signal_set(sig_def, val);
  (void)ui_signal_destroy(sig_def);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_other_types();
  failed |= test_signal();
  if (failed) {
    printf("Failed at %d\n", __LINE__);
    return 1;
  }
  return 0;
}
