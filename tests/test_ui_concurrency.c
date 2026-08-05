/* clang-format off */
#include "ui_arena.h"
#include "ui_signal.h"
#include "ui_thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#define NUM_THREADS 4
#define INCREMENTS 1000

struct task_data {
  ui_signal_t *sig;
};
static ui_error_t increment_task(void *user_data) {
  struct task_data *data = (struct task_data *)user_data;
  int i;
  ui_error_t rc = UI_ERROR_NONE;
  for (i = 0; i < INCREMENTS; i++) {
    union ui_signal_payload val;
    /* Spinlock or lock-free increment in real system, here we use update_fn if
     * available */
    /* For this prototype test, we just set and get wildly to stress the MT
     * mechanism */
    rc = ui_signal_get(data->sig, &val);
    if (rc == UI_ERROR_NONE) {
      val.int_val += 1;
      rc = ui_signal_set(data->sig, val);
      if (rc != UI_ERROR_NONE)
        return rc;
    } else {
      return rc;
    }
  }
  return rc;
}
}

int main(void) {
  struct ui_arena *arena;
  struct ui_thread_pool *pool;
  ui_signal_t *sig;
  union ui_signal_payload init;
  struct task_data task;
  int i;

  init.int_val = 0;

  if (ui_arena_create(1024, &arena) != UI_ERROR_NONE)
    return 1;
  if (ui_thread_pool_create(NUM_THREADS, &pool) != UI_ERROR_NONE)
    return 1;

  if (ui_signal_create(arena, init, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                       UI_SIGNAL_MODE_MULTI_THREADED, &sig) != UI_ERROR_NONE)
    return 1;

  task.sig = sig;

  for (i = 0; i < NUM_THREADS; i++) {
    ui_thread_pool_schedule(pool, increment_task, &task);
  }

  for (i = 0; i < 100; i++) {
    ui_thread_pool_tick(pool);
  }

  (void)ui_signal_destroy(sig);
  ui_thread_pool_destroy(pool);
  (void)ui_arena_destroy(arena);

  printf("test_ui_concurrency passed\\n");
  return 0;
}
