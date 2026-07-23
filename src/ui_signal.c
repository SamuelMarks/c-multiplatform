/* clang-format off */
#include "../include/ui_signal.h"
#include "../include/ui_types.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
#include "ui_reactive_graph.h"
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

static void ui_signal_lock(ui_signal_t *sig) {
  if (sig->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    int is_swapped = 0;
    while (ui_atomic_cas(&sig->lock, 0, 1, &is_swapped) == UI_ERROR_NONE &&
           is_swapped == 0) {
      /* Spinlock */
    }
  }
}

static void ui_signal_unlock(ui_signal_t *sig) {
  if (sig->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    (void)ui_atomic_store(&sig->lock, 0);
  }
}

static enum ui_error ui_signal_add_subscriber(ui_signal_t *sig,
                                              struct ui_reactive_node *node) {
  size_t i;
  struct ui_reactive_node **new_array = NULL;
  size_t new_cap = 0;

  for (i = 0; i < sig->subscribers_count; i++) {
    if (sig->subscribers[i] == node) {
      return UI_ERROR_NONE; /* Already subscribed */
    }
  }

  if (sig->subscribers_count >= sig->subscribers_capacity) {
    new_cap =
        sig->subscribers_capacity == 0 ? 4 : sig->subscribers_capacity * 2;
    new_array = (struct ui_reactive_node **)UI_REALLOC(
        sig->subscribers, new_cap * sizeof(struct ui_reactive_node *));
    if (!new_array) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    sig->subscribers = new_array;
    sig->subscribers_capacity = new_cap;
  }

  sig->subscribers[sig->subscribers_count++] = node;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_signal_create(struct ui_arena *arena, union ui_signal_payload initial_value,
                 enum ui_signal_type type, ui_equality_fn equality_fn,
                 ui_destructor_fn destructor_fn, enum ui_signal_mode mode,
                 ui_signal_t **out_signal) {
  enum ui_error rc = UI_ERROR_NONE;
  ui_signal_t *sig = NULL;

  if (!out_signal) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    return rc;
  }

  /* We allocate from arena if provided, else normal malloc */
  if (arena) {
    void *ptr = NULL;
    rc = ui_arena_alloc(arena, sizeof(ui_signal_t), sizeof(void *), &ptr);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    sig = (ui_signal_t *)ptr;
  } else {
    sig = (ui_signal_t *)UI_MALLOC(sizeof(ui_signal_t));
  }

  if (!sig) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  sig->value = initial_value;
  sig->type = type;
  sig->equality_fn = equality_fn;
  sig->destructor_fn = destructor_fn;
  sig->mode = mode;
  sig->arena = arena;
  sig->ref_count = 1;
  sig->lock = 0;
  sig->subscribers = NULL;
  sig->subscribers_count = 0;
  sig->subscribers_capacity = 0;

  *out_signal = sig;
  return UI_ERROR_NONE;
}

enum ui_error ui_signal_get(ui_signal_t *signal,
                            union ui_signal_payload *out_value) {
  struct ui_reactive_node *current_node = NULL;
  enum ui_error rc = UI_ERROR_NONE;

  if (!signal || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_signal_lock(signal);

  /* Dependency tracking for reactive graph */
  (void)ui_reactive_graph_get_current_node(&current_node);

  if (current_node) {
    rc = ui_signal_add_subscriber(signal, current_node);
    /* In case of OOM on array growth, we pass rc up */
    if (rc != UI_ERROR_NONE) {
      ui_signal_unlock(signal);
      return rc;
    }
  }

  *out_value = signal->value;

  ui_signal_unlock(signal);

  return rc;
}

enum ui_error ui_signal_set(ui_signal_t *signal,
                            union ui_signal_payload new_value) {
  ui_bool_t equal = UI_FALSE;
  size_t i;
  struct ui_reactive_node **subs_copy = NULL;
  size_t subs_count = 0;
  enum ui_error rc = UI_ERROR_NONE;

  if (!signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_signal_lock(signal);

  if (signal->equality_fn) {
    enum ui_error eq_rc = signal->equality_fn(signal->value, new_value, &equal);
    if (eq_rc != UI_ERROR_NONE) {
      ui_signal_unlock(signal);
      return eq_rc;
    }
  } else {
    switch (signal->type) {
    case UI_SIGNAL_TYPE_POINTER:
      equal = (signal->value.ptr_val == new_value.ptr_val);
      break;
    case UI_SIGNAL_TYPE_INT32:
      equal = (signal->value.int_val == new_value.int_val);
      break;
    case UI_SIGNAL_TYPE_FLOAT32:
      equal = (signal->value.float_val == new_value.float_val);
      break;
    case UI_SIGNAL_TYPE_BOOL:
      equal = (signal->value.bool_val == new_value.bool_val);
      break;
    default:
      equal = (signal->value.ptr_val == new_value.ptr_val);
      break;
    }
  }

  if (!equal) {
    if (signal->destructor_fn) {
      enum ui_error dest_rc = signal->destructor_fn(signal->value);
      if (dest_rc != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
        rc = dest_rc;
      }
    }

    signal->value = new_value;

    /* Make a copy of subscribers to notify outside the lock to prevent
     * deadlocks */
    if (signal->subscribers_count > 0) {
      subs_copy = (struct ui_reactive_node **)UI_MALLOC(
          signal->subscribers_count * sizeof(struct ui_reactive_node *));
      if (subs_copy) {
        subs_count = signal->subscribers_count;
        for (i = 0; i < subs_count; i++) {
          subs_copy[i] = signal->subscribers[i];
        }
      }
    }
  }

  ui_signal_unlock(signal);

  if (!equal && subs_copy) {
    for (i = 0; i < subs_count; i++) {
      if (subs_copy[i] && subs_copy[i]->notify_fn) {
        enum ui_error notify_rc =
            subs_copy[i]->notify_fn(subs_copy[i]->user_data);
        if (notify_rc != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
          rc = notify_rc;
        }
      }
    }
    UI_FREE(subs_copy);
  }

  return rc;
}

enum ui_error ui_signal_update(ui_signal_t *signal, ui_update_fn update_fn) {
  union ui_signal_payload new_val;
  enum ui_error rc = UI_ERROR_NONE;

  if (!signal || !update_fn) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* TODO: thread safe lock for update */
  new_val.ptr_val = NULL;
  rc = update_fn(signal->value, &new_val);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_signal_set(signal, new_val);

  return rc;
}

enum ui_error ui_signal_destroy(ui_signal_t *signal) {
  if (!signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Don't need lock if we assume single-owner destruction */
  if (signal->destructor_fn) {
    signal->destructor_fn(signal->value);
  }

  if (signal->subscribers) {
    UI_FREE(signal->subscribers);
    signal->subscribers = NULL;
  }

  if (!signal->arena) {
    UI_FREE(signal);
  }

  return UI_ERROR_NONE;
}
