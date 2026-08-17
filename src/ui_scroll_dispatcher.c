/**
 * \file ui_scroll_dispatcher.c
 * \brief Implementation of the UI Scroll Dispatcher component.
 */

/* clang-format off */
#include "ui_scroll_dispatcher.h"
#include "ui_layout_observer.h"
#include "ui_internal_mem.h"
/* clang-format on */

/** \brief Initial capacity for the subscriber array */
#define INITIAL_SUBSCRIBER_CAPACITY 8

/**
 * \brief Internal structure representing a scroll event subscriber.
 */
struct ui_scroll_subscriber {
  int id;                             /**< Registration ID */
  ui_scroll_dispatcher_cb_t callback; /**< Event callback */
  void *user_data;                    /**< Callback user data */
};

/**
 * \brief Internal structure representing a scroll dispatcher.
 */
struct ui_scroll_dispatcher {
  struct ui_scroll_subscriber *subscribers; /**< Array of subscribers */
  size_t count;    /**< Current number of subscribers */
  size_t capacity; /**< Capacity of the array */
  int next_id;     /**< ID generator for the next subscriber */
};

/**
 * \brief Layout observer callback to handle layout changes by triggering a mock
 * scroll event.
 *
 * \param observer The layout observer.
 * \param breakpoint_id The active breakpoint ID.
 * \param is_active Whether the breakpoint is active.
 * \param user_data Opaque pointer to the scroll dispatcher.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t layout_observer_callback(struct ui_layout_observer *observer,
                                           int breakpoint_id, int is_active,
                                           void *user_data) {
  struct ui_scroll_dispatcher *dispatcher =
      (struct ui_scroll_dispatcher *)user_data;
  struct ui_scroll_info info;

  (void)observer;
  (void)breakpoint_id;
  (void)is_active;

  info.scroll_x = 0.0f;
  info.scroll_y = 0.0f;
  info.delta_x = 0.0f;
  info.delta_y = 0.0f;
  info.source_id = -1; /* -1 typically represents the window/document */

  {
    ui_error_t n_rc = ui_scroll_dispatcher_notify(dispatcher, &info);
    if (n_rc != UI_ERROR_NONE) {
      return n_rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new scroll dispatcher instance.
 *
 * \param out_dispatcher Pointer to receive the created dispatcher.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_create(struct ui_scroll_dispatcher **out_dispatcher) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_scroll_dispatcher *dispatcher = NULL;

  if (!out_dispatcher) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  dispatcher = (struct ui_scroll_dispatcher *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scroll_dispatcher));
  if (!dispatcher) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  dispatcher->subscribers =
      (struct ui_scroll_subscriber *)C_MULTIPLATFORM_MALLOC(
          sizeof(struct ui_scroll_subscriber) * INITIAL_SUBSCRIBER_CAPACITY);
  if (!dispatcher->subscribers) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  dispatcher->count = 0;
  dispatcher->capacity = INITIAL_SUBSCRIBER_CAPACITY;
  dispatcher->next_id = 1;

  *out_dispatcher = dispatcher;
  dispatcher = NULL;

cleanup:
  if (dispatcher) {
    C_MULTIPLATFORM_FREE(dispatcher);
  }
  return rc;
}

/**
 * \brief Destroys a scroll dispatcher and frees its resources.
 *
 * \param dispatcher The dispatcher to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_destroy(struct ui_scroll_dispatcher *dispatcher) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!dispatcher) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  C_MULTIPLATFORM_FREE(dispatcher->subscribers);
  dispatcher->subscribers = NULL;

  C_MULTIPLATFORM_FREE(dispatcher);

cleanup:
  return rc;
}

/**
 * \brief Registers a callback to be notified of scroll events.
 *
 * \param dispatcher The scroll dispatcher.
 * \param callback The function to call when a scroll event occurs.
 * \param user_data Opaque user data to pass to the callback.
 * \param out_registration_id Pointer to receive a unique registration ID.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_register(struct ui_scroll_dispatcher *dispatcher,
                              ui_scroll_dispatcher_cb_t callback,
                              void *user_data, int *out_registration_id) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_scroll_subscriber *new_array;
  size_t new_capacity;

  if (!dispatcher || !callback || !out_registration_id) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (dispatcher->count >= dispatcher->capacity) {
    new_capacity = dispatcher->capacity * 2;
    new_array = (struct ui_scroll_subscriber *)C_MULTIPLATFORM_REALLOC(
        dispatcher->subscribers,
        sizeof(struct ui_scroll_subscriber) * new_capacity);
    if (!new_array) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    dispatcher->subscribers = new_array;
    dispatcher->capacity = new_capacity;
  }

  dispatcher->subscribers[dispatcher->count].id = dispatcher->next_id;
  dispatcher->subscribers[dispatcher->count].callback = callback;
  dispatcher->subscribers[dispatcher->count].user_data = user_data;

  *out_registration_id = dispatcher->next_id;
  dispatcher->count++;
  dispatcher->next_id++;

cleanup:
  return rc;
}

/**
 * \brief Unregisters a previously registered scroll callback.
 *
 * \param dispatcher The scroll dispatcher.
 * \param registration_id The ID of the callback to unregister.
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if ID doesn't exist, or
 * an appropriate error code.
 */
ui_error_t
ui_scroll_dispatcher_unregister(struct ui_scroll_dispatcher *dispatcher,
                                int registration_id) {
  ui_error_t rc = UI_ERROR_NOT_FOUND;
  size_t i;
  size_t j;

  if (!dispatcher) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  for (i = 0; i < dispatcher->count; ++i) {
    if (dispatcher->subscribers[i].id == registration_id) {
      for (j = i; j < dispatcher->count - 1; ++j) {
        dispatcher->subscribers[j] = dispatcher->subscribers[j + 1];
      }
      dispatcher->count--;
      rc = UI_ERROR_NONE;
      break;
    }
  }

cleanup:
  return rc;
}

/**
 * \brief Dispatches a scroll event to all registered callbacks.
 *
 * \param dispatcher The scroll dispatcher.
 * \param info Information about the scroll event.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_dispatcher_notify(struct ui_scroll_dispatcher *dispatcher,
                                       const struct ui_scroll_info *info) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t i;

  if (!dispatcher || !info) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  for (i = 0; i < dispatcher->count; ++i) {
    ui_error_t cb_rc = dispatcher->subscribers[i].callback(
        dispatcher, info, dispatcher->subscribers[i].user_data);
    if (cb_rc != UI_ERROR_NONE) {
      rc = cb_rc;
      goto cleanup;
    }
  }

cleanup:
  return rc;
}

/**
 * \brief Integrates the scroll dispatcher with a layout observer.
 *
 * \param dispatcher The scroll dispatcher.
 * \param layout_observer The layout observer to integrate with.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_dispatcher_bind_layout_observer(
    struct ui_scroll_dispatcher *dispatcher,
    struct ui_layout_observer *layout_observer) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!dispatcher || !layout_observer) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  rc = ui_layout_observer_subscribe(layout_observer, layout_observer_callback,
                                    dispatcher);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

cleanup:
  return rc;
}
