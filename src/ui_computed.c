/**
 * @file ui_computed.c
 * @brief ui_computed.c implementation.
 */
/* clang-format off */
#include "../include/ui_computed.h"
#include "../include/ui_types.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_signal.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
#include "ui_reactive_graph.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_computed_mock_cas_fail = 0;
int g_computed_mock_graph_get_fail = 0;
int g_computed_mock_graph_set_fail = 0;

/**
 * @brief mock_computed_atomic_cas.
 * @param target Target atomic.
 * @param expected Expected value.
 * @param new_value New value.
 * @param out_swapped Output swapped flag.
 * @return Return value.
 */
static ui_error_t mock_computed_atomic_cas(ui_atomic_t *target, long expected,
                                           long new_value, int *out_swapped) {
  if (g_computed_mock_cas_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_atomic_cas)(target, expected, new_value, out_swapped);
}
#undef ui_atomic_cas
/** @cond */
#define ui_atomic_cas mock_computed_atomic_cas
/** @endcond */

/**
 * @brief mock_computed_graph_get_current_node.
 * @param out_node Output node.
 * @return Return value.
 */
static ui_error_t
mock_computed_graph_get_current_node(struct ui_reactive_node **out_node) {
  if (g_computed_mock_graph_get_fail != 0) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_reactive_graph_get_current_node)(out_node);
}
#undef ui_reactive_graph_get_current_node
/** @cond */
#define ui_reactive_graph_get_current_node mock_computed_graph_get_current_node
/** @endcond */

/**
 * @brief mock_computed_graph_set_current_node.
 * @param node Node to set.
 * @param out_prev_node Output previous node.
 * @return Return value.
 */
static ui_error_t
mock_computed_graph_set_current_node(struct ui_reactive_node *node,
                                     struct ui_reactive_node **out_prev_node) {
  if (g_computed_mock_graph_set_fail == 1 && out_prev_node != NULL) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_computed_mock_graph_set_fail == 2 && out_prev_node == NULL) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_reactive_graph_set_current_node)(node, out_prev_node);
}
#undef ui_reactive_graph_set_current_node
/** @cond */
#define ui_reactive_graph_set_current_node mock_computed_graph_set_current_node
/** @endcond */
#endif

/**
 * @struct ui_computed
 * \brief ui_computed
 */
struct ui_computed {
  ui_compute_fn compute_fn;             /**< compute_fn */
  void *user_data;                      /**< user_data */
  enum ui_signal_type type;             /**< type */
  enum ui_signal_mode mode;             /**< mode */
  struct ui_arena *arena;               /**< arena */
  union ui_signal_payload cached_value; /**< cached_value */
  ui_bool_t is_dirty;                   /**< is_dirty */
  ui_atomic_t lock;                     /**< lock */

  struct ui_reactive_node self_node; /**< self_node */

  struct ui_reactive_node **subscribers; /**< subscribers */
  size_t subscribers_count;              /**< subscribers_count */
  size_t subscribers_capacity;           /**< subscribers_capacity */
};

/**
 * @brief ui_computed_lock.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t ui_computed_lock(ui_computed_t *comp) {
  if (comp->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    ui_int32 is_swapped = 0;
    while (!is_swapped) {
      ui_error_t rc = ui_atomic_cas(&comp->lock, 0, 1, &is_swapped);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_computed_unlock.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t ui_computed_unlock(ui_computed_t *comp) {
  if (comp->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    return ui_atomic_store(&comp->lock, 0);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_computed_add_subscriber.
 * @param comp Parameter comp.
 * @param node Parameter node.
 * @return Return value.
 */
static ui_error_t ui_computed_add_subscriber(ui_computed_t *comp,
                                             struct ui_reactive_node *node) {
  size_t i;
  struct ui_reactive_node **new_array = NULL;
  size_t new_cap = 0;

  for (i = 0; i < comp->subscribers_count; i++) {
    if (comp->subscribers[i] == node) {
      return UI_ERROR_NONE;
    }
  }

  if (comp->subscribers_count >= comp->subscribers_capacity) {
    new_cap =
        comp->subscribers_capacity == 0 ? 4 : comp->subscribers_capacity * 2;
    new_array = (struct ui_reactive_node **)C_MULTIPLATFORM_REALLOC(
        comp->subscribers, (size_t)new_cap * sizeof(struct ui_reactive_node *));
    if (!new_array) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    comp->subscribers = new_array;
    comp->subscribers_capacity = new_cap;
  }

  comp->subscribers[comp->subscribers_count++] = node;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_computed_on_notify.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t ui_computed_on_notify(void *user_data) {
  ui_computed_t *comp = (ui_computed_t *)user_data;
  size_t i;
  struct ui_reactive_node **subs_copy = NULL;
  size_t subs_count = 0;
  ui_error_t rc;

  {
    ui_error_t _rc_c = ui_computed_lock(comp);
    if (_rc_c != UI_ERROR_NONE)
      return _rc_c;
  }

  if (comp->is_dirty) {
    return ui_computed_unlock(comp); /* Already dirty, no need to re-notify */
  }

  comp->is_dirty = UI_TRUE;

  /* Make a copy of subscribers to notify outside the lock */
  if (comp->subscribers_count > 0) {
    subs_copy = (struct ui_reactive_node **)C_MULTIPLATFORM_MALLOC(
        (size_t)comp->subscribers_count * sizeof(struct ui_reactive_node *));
    if (subs_copy) {
      subs_count = comp->subscribers_count;
      for (i = 0; i < subs_count; i++) {
        subs_copy[i] = comp->subscribers[i];
      }
    }
  }

  rc = ui_computed_unlock(comp);

  if (subs_copy) {
    for (i = 0; i < subs_count; i++) {
      ui_error_t notify_rc = subs_copy[i]->notify_fn(subs_copy[i]->user_data);
      if (notify_rc != UI_ERROR_NONE) {
        C_MULTIPLATFORM_FREE(subs_copy);
        return (rc != UI_ERROR_NONE) ? rc : notify_rc;
      }
    }
    C_MULTIPLATFORM_FREE(subs_copy);
  }

  return rc;
}

/**
 * @brief ui_computed_create.
 * @param arena Parameter arena.
 * @param compute_fn Parameter compute_fn.
 * @param user_data Parameter user_data.
 * @param type Parameter type.
 * @param mode Parameter mode.
 * @param out_computed Parameter out_computed.
 * @return Return value.
 */
ui_error_t ui_computed_create(struct ui_arena *arena, ui_compute_fn compute_fn,
                              void *user_data, enum ui_signal_type type,
                              enum ui_signal_mode mode,
                              ui_computed_t **out_computed) {
  ui_computed_t *comp = NULL;
  ui_error_t rc;

  if (!out_computed || !compute_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (arena) {
    void *ptr = NULL;
    rc = ui_arena_alloc(arena, sizeof(ui_computed_t), sizeof(void *), &ptr);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    comp = (ui_computed_t *)ptr;
  } else {
    comp = (ui_computed_t *)C_MULTIPLATFORM_MALLOC(sizeof(ui_computed_t));
  }

  if (!comp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  comp->compute_fn = compute_fn;
  comp->user_data = user_data;
  comp->type = type;
  comp->mode = mode;
  comp->arena = arena;
  comp->cached_value.ptr_val = NULL;
  comp->is_dirty = UI_TRUE;
  comp->lock = 0;
  comp->self_node.notify_fn = ui_computed_on_notify;
  comp->self_node.user_data = comp;
  comp->subscribers = NULL;
  comp->subscribers_count = 0;
  comp->subscribers_capacity = 0;

  *out_computed = comp;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_computed_get.
 * @param computed Parameter computed.
 * @param out_value Parameter out_value.
 * @return Return value.
 */
ui_error_t ui_computed_get(ui_computed_t *computed,
                           union ui_signal_payload *out_value) {
  struct ui_reactive_node *current_node = NULL;
  struct ui_reactive_node *prev_node = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  if (!computed || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t _rc_c = ui_computed_lock(computed);
    if (_rc_c != UI_ERROR_NONE)
      return _rc_c;
  }

  /* Dependency tracking */
  {
    ui_error_t _rc_g = ui_reactive_graph_get_current_node(&current_node);
    if (_rc_g != UI_ERROR_NONE) {
      ui_error_t unlock_rc = ui_computed_unlock(computed);
      return (unlock_rc != UI_ERROR_NONE) ? unlock_rc : _rc_g;
    }
  }

  if (current_node && current_node != &computed->self_node) {
    rc = ui_computed_add_subscriber(computed, current_node);
    if (rc != UI_ERROR_NONE) {
      ui_error_t unlock_rc = ui_computed_unlock(computed);
      return (unlock_rc != UI_ERROR_NONE) ? unlock_rc : rc;
    }
  }

  if (computed->is_dirty) {
    /* Push self to graph to track inner dependencies */
    {
      ui_error_t _rc_g =
          ui_reactive_graph_set_current_node(&computed->self_node, &prev_node);
      if (_rc_g != UI_ERROR_NONE) {
        ui_error_t unlock_rc = ui_computed_unlock(computed);
        return (unlock_rc != UI_ERROR_NONE) ? unlock_rc : _rc_g;
      }
    }

    rc = computed->compute_fn(computed->user_data, &computed->cached_value);
    if (rc != UI_ERROR_NONE) {
      ui_error_t graph_rc = ui_reactive_graph_set_current_node(prev_node, NULL);
      ui_error_t unlock_rc = ui_computed_unlock(computed);
      return (graph_rc != UI_ERROR_NONE)
                 ? graph_rc
                 : ((unlock_rc != UI_ERROR_NONE) ? unlock_rc : rc);
    }

    {
      ui_error_t _rc_g = ui_reactive_graph_set_current_node(prev_node, NULL);
      if (_rc_g != UI_ERROR_NONE) {
        ui_error_t unlock_rc = ui_computed_unlock(computed);
        return (unlock_rc != UI_ERROR_NONE) ? unlock_rc : _rc_g;
      }
    }
    computed->is_dirty = UI_FALSE;
  }

  *out_value = computed->cached_value;
  return ui_computed_unlock(computed);
}

/**
 * @brief ui_computed_destroy.
 * @param computed Parameter computed.
 * @return Return value.
 */
ui_error_t ui_computed_destroy(ui_computed_t *computed) {
  if (!computed) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (computed->subscribers) {
    C_MULTIPLATFORM_FREE(computed->subscribers);
    computed->subscribers = NULL;
  }

  if (!computed->arena) {
    C_MULTIPLATFORM_FREE(computed);
  }

  return UI_ERROR_NONE;
}
