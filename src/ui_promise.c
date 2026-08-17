/**
 * \file ui_promise.c
 * \brief Implementation of the UI Promise component.
 */

/* clang-format off */
#include "ui_promise.h"
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * \brief Node in the callback chain of a promise.
 */
struct ui_promise_callback_node {
  ui_error_t (*on_resolve)(void *, void *, void **);    /**< Resolve callback */
  ui_error_t (*on_reject)(ui_error_t, void *, void **); /**< Reject callback */
  ui_error_t (*on_finally)(void *);                     /**< Finally callback */
  void *user_data;                       /**< User data for callbacks */
  struct ui_promise *chained_promise;    /**< Next promise in the chain */
  struct ui_promise_callback_node *next; /**< Next node in the list */
};

/**
 * \brief Internal structure representing a promise.
 */
struct ui_promise {
  enum ui_promise_state state;           /**< Current state of the promise */
  void *result;                          /**< Result value if fulfilled */
  ui_error_t error;                      /**< Error discriminant if rejected */
  struct ui_promise_callback_node *head; /**< Head of callback list */
  struct ui_promise_callback_node *tail; /**< Tail of callback list */
};

/**
 * \brief Creates a new pending promise.
 *
 * \param out_promise Pointer to receive the new promise handle.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_promise_create(struct ui_promise **out_promise) {
  struct ui_promise *promise = NULL;

  if (!out_promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  promise =
      (struct ui_promise *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_promise));
  if (!promise) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  promise->state = UI_PROMISE_PENDING;
  promise->result = NULL;
  promise->error = UI_ERROR_NONE;
  promise->head = NULL;
  promise->tail = NULL;

  *out_promise = promise;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a promise and frees its resources.
 *
 * \param promise The promise to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_destroy(struct ui_promise *promise) {
  struct ui_promise_callback_node *current = NULL;
  struct ui_promise_callback_node *next = NULL;

  if (!promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = promise->head;
  while (current) {
    next = current->next;
    if (current->chained_promise) {
      (void)ui_promise_destroy(current->chained_promise);
    }
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  C_MULTIPLATFORM_FREE(promise);
  return UI_ERROR_NONE;
}

/**
 * \brief Triggers a callback node based on the promise state.
 *
 * \param node The callback node to trigger.
 * \param state The current state of the promise.
 * \param result The result value if fulfilled.
 * \param error The error discriminant if rejected.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t trigger_callback(struct ui_promise_callback_node *node,
                                   enum ui_promise_state state, void *result,
                                   ui_error_t error) {
  void *out_result = NULL;
  ui_error_t cb_rc = UI_ERROR_NONE;

  if (node->on_finally) {
    {
      ui_error_t rc = node->on_finally(node->user_data);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    if (node->chained_promise) {
      if (state == UI_PROMISE_FULFILLED) {
        (void)ui_promise_resolve(node->chained_promise, result);
      } else {
        (void)ui_promise_reject(node->chained_promise, error);
      }
    }
    return UI_ERROR_NONE;
  }

  if (state == UI_PROMISE_FULFILLED) {
    if (node->on_resolve) {
      cb_rc = node->on_resolve(result, node->user_data, &out_result);
      if (node->chained_promise) {
        if (cb_rc == UI_ERROR_NONE) {
          (void)ui_promise_resolve(node->chained_promise, out_result);
        } else {
          (void)ui_promise_reject(node->chained_promise, cb_rc);
        }
      } else if (cb_rc != UI_ERROR_NONE) {
        return cb_rc;
      }
    } else if (node->chained_promise) {
      /* bubble up */
      (void)ui_promise_resolve(node->chained_promise, result);
    }
  } else {
    if (node->on_reject) {
      cb_rc = node->on_reject(error, node->user_data, &out_result);
      if (node->chained_promise) {
        if (cb_rc == UI_ERROR_NONE) {
          (void)ui_promise_resolve(node->chained_promise, out_result);
        } else {
          (void)ui_promise_reject(node->chained_promise, cb_rc);
        }
      } else if (cb_rc != UI_ERROR_NONE) {
        return cb_rc;
      }
    } else if (node->chained_promise) {
      /* bubble up */
      (void)ui_promise_reject(node->chained_promise, error);
    }
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Internal function to add a callback to the promise.
 *
 * \param promise The promise.
 * \param on_resolve Callback invoked if the promise is fulfilled.
 * \param on_reject Callback invoked if the promise is rejected.
 * \param on_finally Callback invoked regardless of outcome.
 * \param user_data Opaque pointer passed to the callbacks.
 * \param out_promise Pointer to receive the chained promise.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t
add_callback(struct ui_promise *promise,
             ui_error_t (*on_resolve)(void *, void *, void **),
             ui_error_t (*on_reject)(ui_error_t, void *, void **),
             ui_error_t (*on_finally)(void *), void *user_data,
             struct ui_promise **out_promise) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_promise_callback_node *node = NULL;
  struct ui_promise *chained = NULL;

  if (!promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (out_promise) {
    rc = ui_promise_create(&chained);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    *out_promise = chained;
  }

  node = (struct ui_promise_callback_node *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_promise_callback_node));
  if (!node) {
    if (chained) {
      (void)ui_promise_destroy(chained);
    }
    if (out_promise)
      *out_promise = NULL;
    return UI_ERROR_OUT_OF_MEMORY;
  }

  node->on_resolve = on_resolve;
  node->on_reject = on_reject;
  node->on_finally = on_finally;
  node->user_data = user_data;
  node->chained_promise = chained;
  node->next = NULL;

  if (promise->state != UI_PROMISE_PENDING) {
    rc =
        trigger_callback(node, promise->state, promise->result, promise->error);
    if (rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(node);
      return rc;
    }
  }

  if (promise->tail) {
    promise->tail->next = node;
  } else {
    promise->head = node;
  }
  promise->tail = node;

  return UI_ERROR_NONE;
}

/**
 * \brief Adds callbacks to be executed when the promise is settled.
 *        If already settled, the callback is invoked immediately.
 *
 * \param promise The promise.
 * \param on_resolve Callback invoked if the promise is fulfilled.
 * \param on_reject Callback invoked if the promise is rejected.
 * \param user_data Opaque pointer passed to the callbacks.
 * \param out_promise Pointer to receive the chained promise.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_then(struct ui_promise *promise,
                           ui_error_t (*on_resolve)(void *, void *, void **),
                           ui_error_t (*on_reject)(ui_error_t, void *, void **),
                           void *user_data, struct ui_promise **out_promise) {
  return add_callback(promise, on_resolve, on_reject, NULL, user_data,
                      out_promise);
}

/**
 * \brief Adds a rejection callback to the promise.
 *
 * \param promise The promise.
 * \param on_reject Callback invoked if the promise is rejected.
 * \param user_data Opaque pointer passed to the callback.
 * \param out_promise Pointer to receive the chained promise.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_catch(struct ui_promise *promise,
                            ui_error_t (*on_reject)(ui_error_t, void *,
                                                    void **),
                            void *user_data, struct ui_promise **out_promise) {
  return add_callback(promise, NULL, on_reject, NULL, user_data, out_promise);
}

/**
 * \brief Adds a finally callback to the promise.
 *
 * \param promise The promise.
 * \param on_finally Callback invoked regardless of outcome.
 * \param user_data Opaque pointer passed to the callback.
 * \param out_promise Pointer to receive the chained promise.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_finally(struct ui_promise *promise,
                              ui_error_t (*on_finally)(void *), void *user_data,
                              struct ui_promise **out_promise) {
  return add_callback(promise, NULL, NULL, on_finally, user_data, out_promise);
}

/**
 * \brief Resolves the promise with a value, triggering on_resolve callbacks.
 *
 * \param promise The promise to resolve.
 * \param result The result value (cast to void*).
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_resolve(struct ui_promise *promise, void *result) {
  struct ui_promise_callback_node *current = NULL;
  struct ui_promise_callback_node *next = NULL;

  if (!promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (promise->state != UI_PROMISE_PENDING) {
    return UI_ERROR_NONE; /* Standard behavior: ignore subsequent resolutions */
  }

  promise->state = UI_PROMISE_FULFILLED;
  promise->result = result;

  current = promise->head;

  /* Use iterative loop to prevent stack overflow on deep chains */
  while (current) {
    ui_error_t rc;
    next = current->next;
    rc = trigger_callback(current, UI_PROMISE_FULFILLED, result, UI_ERROR_NONE);
    if (rc != UI_ERROR_NONE)
      return rc;
    current = next;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Rejects the promise with an error, triggering on_reject callbacks.
 *
 * \param promise The promise to reject.
 * \param error The error discriminant.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_reject(struct ui_promise *promise, ui_error_t error) {
  struct ui_promise_callback_node *current = NULL;
  struct ui_promise_callback_node *next = NULL;

  if (!promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (promise->state != UI_PROMISE_PENDING) {
    return UI_ERROR_NONE;
  }

  promise->state = UI_PROMISE_REJECTED;
  promise->error = error;

  current = promise->head;

  while (current) {
    ui_error_t rc;
    next = current->next;
    rc = trigger_callback(current, UI_PROMISE_REJECTED, NULL, error);
    if (rc != UI_ERROR_NONE)
      return rc;
    current = next;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current state of the promise.
 *
 * \param promise The promise.
 * \param out_state Pointer to receive the current state.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_promise_get_state(struct ui_promise *promise,
                                enum ui_promise_state *out_state) {
  if (!promise || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = promise->state;
  return UI_ERROR_NONE;
}
