/* clang-format off */
#include "m3_state.h"
#include "m3_memory.h"
#include <stddef.h>
/* clang-format on */

int m3_state_engine_init(m3_state_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  engine->dependencies = NULL;
  engine->debounce_queue = NULL;
  return 0;
}

int m3_state_engine_cleanup(m3_state_engine_t *engine) {
  if (!engine) {
    return 1;
  }
  engine->dependencies = NULL;
  engine->debounce_queue = NULL;
  return 0;
}

/*
 * Assuming g_mem_state exists somewhere or we pass it around, but we
 * just use standard cmp_arena or malloc if needed. For now we will use
 * malloc/free for debounce tasks and dependencies, as they are not tied
 * strictly to frame renders, but to app lifecycle.
 */
#include <stdlib.h>

int m3_state_bind_node(m3_state_engine_t *engine,
                       material_catalog_state_t *state, cmp_ui_node_t *node,
                       void *state_variable, m3_state_update_cb update_cb,
                       void *user_data) {
  m3_state_dep_t *dep;
  (void)state;

  if (!engine || !node || !state_variable) {
    return 1;
  }

  dep = (m3_state_dep_t *)malloc(sizeof(m3_state_dep_t));
  if (!dep) {
    return 1;
  }

  dep->node = node;
  dep->state_variable = state_variable;
  dep->update_cb = update_cb;
  dep->user_data = user_data;

  dep->next = engine->dependencies;
  engine->dependencies = dep;

  return 0;
}

int m3_state_notify_changed(m3_state_engine_t *engine, void *state_variable) {
  m3_state_dep_t *current;

  if (!engine || !state_variable) {
    return 1;
  }

  current = engine->dependencies;
  while (current) {
    if (current->state_variable == state_variable) {
      if (current->update_cb) {
        current->update_cb(current->node, state_variable, current->user_data);
      }
    }
    current = current->next;
  }

  return 0;
}

void m3_invalidate_ui(material_catalog_state_t *state, cmp_ui_node_t *node) {
  (void)node;
  if (!state)
    return;
  state->is_ui_dirty = 1;
}

int m3_debounce_task(m3_state_engine_t *engine, material_catalog_state_t *state,
                     int id, uint32_t delay_ms,
                     void (*callback)(material_catalog_state_t *state,
                                      void *user_data),
                     void *user_data) {
  /* Find if it already exists */
  m3_debounce_task_t *current;
  (void)state;

  /* Mock timestamp for now, assume 0 is not used or handled in process */
  /* Real implementation would query the OS time or use dt from state */

  if (!engine || !callback)
    return 1;

  current = engine->debounce_queue;
  while (current) {
    if (current->id == id) {
      /* Reset timer */
      current->target_time_ms =
          delay_ms; /* We will use delay_ms as a "frames" or relative offset in
                       the process loop, or add to real timestamp */
      current->callback = callback;
      current->user_data = user_data;
      return 0;
    }
    current = current->next;
  }

  /* Create new task */
  current = (m3_debounce_task_t *)malloc(sizeof(m3_debounce_task_t));
  if (!current)
    return 1;

  current->id = id;
  current->target_time_ms = delay_ms;
  current->callback = callback;
  current->user_data = user_data;

  current->next = engine->debounce_queue;
  engine->debounce_queue = current;

  return 0;
}

void m3_process_debounce_queue(m3_state_engine_t *engine,
                               material_catalog_state_t *state,
                               uint32_t dt_ms) {
  m3_debounce_task_t *current;
  m3_debounce_task_t *prev = NULL;

  if (!engine || !state)
    return;

  current = engine->debounce_queue;
  while (current) {
    if (current->target_time_ms <= dt_ms) {
      /* Trigger and remove */
      current->callback(state, current->user_data);

      if (prev) {
        prev->next = current->next;
      } else {
        engine->debounce_queue = current->next;
      }

      {
        m3_debounce_task_t *to_free = current;
        current = current->next;
        free(to_free);
      }
    } else {
      current->target_time_ms -= dt_ms;
      prev = current;
      current = current->next;
    }
  }
}