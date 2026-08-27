/* clang-format off */
#include "../include/ui_signal.h"
#include "../include/ui_types.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_atomic.h"
#include "ui_internal_mem.h"
#include "ui_reactive_graph.h"
/* clang-format on */

/*
 * \file ui_signal.c
 * \brief Signal implementation.
 */

/**
 * @struct ui_signal
 * \brief ui_signal structure.
 * \details Internal state for a signal.
 */
struct ui_signal {
  union ui_signal_payload value;  /**< value */
  enum ui_signal_type type;       /**< type */
  ui_equality_fn equality_fn;     /**< equality_fn */
  ui_destructor_fn destructor_fn; /**< destructor_fn */
  enum ui_signal_mode mode;       /**< mode */
  struct ui_arena *arena;         /**< arena */
  ui_int32 ref_count;             /**< ref_count */
  ui_atomic_t lock;               /**< lock */

  struct ui_reactive_node **subscribers; /**< subscribers */
  size_t subscribers_count;              /**< subscribers_count */
  size_t subscribers_capacity;           /**< subscribers_capacity */
};

/*
 * \brief Locks a signal.
 * \param sig The signal to lock.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_signal_lock(ui_signal_t *sig) {
  if (sig->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    int is_swapped = 0;
    while (is_swapped == 0) {
      {
        ui_error_t _ign_rc = ui_atomic_cas(&sig->lock, 0, 1, &is_swapped);
        (void)_ign_rc;
      }
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Unlocks a signal.
 * \param sig The signal to unlock.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_signal_unlock(ui_signal_t *sig) {
  if (sig->mode == UI_SIGNAL_MODE_MULTI_THREADED) {
    {
      ui_error_t _ign_rc = ui_atomic_store(&sig->lock, 0);
      (void)_ign_rc;
    }
  }
  return UI_ERROR_NONE;
}

/*
 * \brief Adds a subscriber to a signal.
 * \param sig The signal.
 * \param node The subscriber node.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t ui_signal_add_subscriber(ui_signal_t *sig,
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
    new_array = (struct ui_reactive_node **)C_MULTIPLATFORM_REALLOC(
        sig->subscribers, (size_t)new_cap * sizeof(struct ui_reactive_node *));
    if (!new_array) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    sig->subscribers = new_array;
    sig->subscribers_capacity = new_cap;
  }

  sig->subscribers[sig->subscribers_count++] = node;
  return UI_ERROR_NONE;
}

/*
 * \brief Creates a new signal.
 * \param arena The arena to allocate from.
 * \param initial_value The initial value.
 * \param type The signal type.
 * \param equality_fn The equality function.
 * \param destructor_fn The destructor function.
 * \param mode The signal mode.
 * \param out_signal Pointer to store the created signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_signal_create(struct ui_arena *arena, union ui_signal_payload initial_value,
                 enum ui_signal_type type, ui_equality_fn equality_fn,
                 ui_destructor_fn destructor_fn, enum ui_signal_mode mode,
                 ui_signal_t **out_signal) {
  ui_error_t rc = UI_ERROR_NONE;
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
    sig = (ui_signal_t *)C_MULTIPLATFORM_MALLOC(sizeof(ui_signal_t));
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

/*
 * \brief Gets the value of a signal.
 * \param signal The signal.
 * \param out_value Pointer to store the value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_signal_get(ui_signal_t *signal,
                         union ui_signal_payload *out_value) {
  struct ui_reactive_node *current_node = NULL;
  ui_error_t rc = UI_ERROR_NONE;

  ui_error_t lock_rc;
  if (!signal || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  lock_rc = ui_signal_lock(signal);
  (void)lock_rc;

  /* Dependency tracking for reactive graph */
  {
    ui_error_t _ign_rc = ui_reactive_graph_get_current_node(&current_node);
    (void)_ign_rc;
  }

  if (current_node) {
    ui_error_t add_rc = ui_signal_add_subscriber(signal, current_node);
    /* In case of OOM on array growth, we pass rc up */
    if (add_rc != UI_ERROR_NONE) {
      {
        ui_error_t unlock_rc = ui_signal_unlock(signal);
        (void)unlock_rc;
      }
      return add_rc;
    }
  }

  *out_value = signal->value;

  {
    ui_error_t unlock_rc = ui_signal_unlock(signal);
    (void)unlock_rc;
  }

  return rc;
}

/*
 * \brief Sets the value of a signal.
 * \param signal The signal.
 * \param new_value The new value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_signal_set(ui_signal_t *signal,
                         union ui_signal_payload new_value) {
  ui_bool_t equal = UI_FALSE;
  size_t i;
  struct ui_reactive_node **subs_copy = NULL;
  size_t subs_count = 0;
  ui_error_t rc = UI_ERROR_NONE;

  ui_error_t lock_rc;
  if (!signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  lock_rc = ui_signal_lock(signal);
  (void)lock_rc;

  if (signal->equality_fn) {
    ui_error_t eq_rc = signal->equality_fn(signal->value, new_value, &equal);
    if (eq_rc != UI_ERROR_NONE) {
      {
        ui_error_t unlock_rc = ui_signal_unlock(signal);
        (void)unlock_rc;
      }
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
      ui_error_t dest_rc = signal->destructor_fn(signal->value);
      if (dest_rc != UI_ERROR_NONE) {
        ui_error_t unlock_rc = ui_signal_unlock(signal);
        (void)unlock_rc;
        return dest_rc;
      }
    }

    signal->value = new_value;

    /* Make a copy of subscribers to notify outside the lock to prevent
     * deadlocks */
    if (signal->subscribers_count > 0) {
      subs_copy = (struct ui_reactive_node **)C_MULTIPLATFORM_MALLOC(
          (size_t)signal->subscribers_count *
          sizeof(struct ui_reactive_node *));
      if (subs_copy) {
        subs_count = signal->subscribers_count;
        for (i = 0; i < subs_count; i++) {
          subs_copy[i] = signal->subscribers[i];
        }
      }
    }
  }

  {
    ui_error_t unlock_rc = ui_signal_unlock(signal);
    (void)unlock_rc;
  }

  if (subs_copy) {
    for (i = 0; i < subs_count; i++) {
      if (subs_copy[i]->notify_fn) {
        ui_error_t notify_rc = subs_copy[i]->notify_fn(subs_copy[i]->user_data);
        if (notify_rc != UI_ERROR_NONE && rc == UI_ERROR_NONE) {
          rc = notify_rc;
        }
      }
    }
    C_MULTIPLATFORM_FREE(subs_copy);
  }

  return rc;
}

/*
 * \brief Updates the value of a signal using a callback.
 * \param signal The signal.
 * \param update_fn The update function.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_signal_update(ui_signal_t *signal, ui_update_fn update_fn) {
  union ui_signal_payload new_val;
  ui_error_t rc = UI_ERROR_NONE;

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
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a signal.
 * \param signal The signal to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_signal_destroy(ui_signal_t *signal) {
  if (!signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Don't need lock if we assume single-owner destruction */
  if (signal->destructor_fn) {
    ui_error_t dest_rc = signal->destructor_fn(signal->value);
    if (dest_rc != UI_ERROR_NONE)
      return dest_rc;
  }

  if (signal->subscribers) {
    C_MULTIPLATFORM_FREE(signal->subscribers);
    signal->subscribers = NULL;
  }

  if (!signal->arena) {
    C_MULTIPLATFORM_FREE(signal);
  }

  return UI_ERROR_NONE;
}
