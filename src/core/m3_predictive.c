#include "m3/m3_predictive.h"

#include <string.h>

#ifdef M3_TESTING
static M3Bool g_m3_predictive_test_fail_event_init = M3_FALSE;

int M3_CALL m3_predictive_test_set_event_init_fail(M3Bool enable) {
  g_m3_predictive_test_fail_event_init = enable ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif

static int m3_predictive_back_validate_edge(m3_u32 edge) {
  if (edge == M3_PREDICTIVE_BACK_EDGE_UNKNOWN ||
      edge == M3_PREDICTIVE_BACK_EDGE_LEFT ||
      edge == M3_PREDICTIVE_BACK_EDGE_RIGHT) {
    return M3_OK;
  }
  return M3_ERR_RANGE;
}

static int
m3_predictive_back_validate_event(const M3PredictiveBackEvent *event) {
  int rc;

  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (event->progress < 0.0f || event->progress > 1.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_predictive_back_validate_edge(event->edge);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int
m3_predictive_back_validate_handler(const M3PredictiveBackHandler *handler) {
  if (handler == NULL || handler->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_predictive_back_call(const M3PredictiveBackHandler *handler,
                                   m3_u32 kind,
                                   const M3PredictiveBackEvent *event) {
  if (handler == NULL || handler->vtable == NULL) {
    return M3_OK;
  }

  if (kind == 0u) {
    if (handler->vtable->on_start != NULL) {
      return handler->vtable->on_start(handler->ctx, event);
    }
  } else if (kind == 1u) {
    if (handler->vtable->on_progress != NULL) {
      return handler->vtable->on_progress(handler->ctx, event);
    }
  } else if (kind == 2u) {
    if (handler->vtable->on_commit != NULL) {
      return handler->vtable->on_commit(handler->ctx, event);
    }
  } else if (kind == 3u) {
    if (handler->vtable->on_cancel != NULL) {
      return handler->vtable->on_cancel(handler->ctx, event);
    }
  }

  return M3_OK;
}

int M3_CALL m3_predictive_back_event_init(M3PredictiveBackEvent *event) {
  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#ifdef M3_TESTING
  if (g_m3_predictive_test_fail_event_init == M3_TRUE) {
    g_m3_predictive_test_fail_event_init = M3_FALSE;
    return M3_ERR_UNKNOWN;
  }
#endif

  memset(event, 0, sizeof(*event));
  event->edge = M3_PREDICTIVE_BACK_EDGE_UNKNOWN;
  return M3_OK;
}

int M3_CALL
m3_predictive_back_handler_init(M3PredictiveBackHandler *handler, void *ctx,
                                const M3PredictiveBackHandlerVTable *vtable) {
  if (handler == NULL || vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  handler->ctx = ctx;
  handler->vtable = vtable;
  return M3_OK;
}

int M3_CALL m3_predictive_back_init(M3PredictiveBack *predictive,
                                    const M3PredictiveBackHandler *handler) {
  int rc;

  if (predictive == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (predictive->initialized) {
    return M3_ERR_STATE;
  }

  if (handler != NULL) {
    rc = m3_predictive_back_validate_handler(handler);
    if (rc != M3_OK) {
      return rc;
    }
    predictive->handler = *handler;
  } else {
    predictive->handler.ctx = NULL;
    predictive->handler.vtable = NULL;
  }

  rc = m3_predictive_back_event_init(&predictive->last_event);
  if (rc != M3_OK) {
    return rc;
  }

  predictive->active = M3_FALSE;
  predictive->initialized = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_predictive_back_shutdown(M3PredictiveBack *predictive) {
  int rc;

  if (predictive == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_predictive_back_event_init(&predictive->last_event);
  if (rc != M3_OK) {
    return rc;
  }

  predictive->active = M3_FALSE;
  predictive->handler.ctx = NULL;
  predictive->handler.vtable = NULL;
  predictive->initialized = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_predictive_back_set_handler(
    M3PredictiveBack *predictive, const M3PredictiveBackHandler *handler) {
  int rc;

  if (predictive == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }

  if (handler != NULL) {
    rc = m3_predictive_back_validate_handler(handler);
    if (rc != M3_OK) {
      return rc;
    }
    predictive->handler = *handler;
  } else {
    predictive->handler.ctx = NULL;
    predictive->handler.vtable = NULL;
  }

  return M3_OK;
}

int M3_CALL m3_predictive_back_get_handler(
    const M3PredictiveBack *predictive, M3PredictiveBackHandler *out_handler) {
  if (predictive == NULL || out_handler == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }

  *out_handler = predictive->handler;
  return M3_OK;
}

int M3_CALL m3_predictive_back_get_state(const M3PredictiveBack *predictive,
                                         M3PredictiveBackState *out_state) {
  if (predictive == NULL || out_state == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }

  out_state->active = predictive->active;
  out_state->event = predictive->last_event;
  return M3_OK;
}

int M3_CALL m3_predictive_back_start(M3PredictiveBack *predictive,
                                     const M3PredictiveBackEvent *event) {
  M3PredictiveBackEvent prev_event;
  int rc;

  if (predictive == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }
  if (predictive->active) {
    return M3_ERR_STATE;
  }

  rc = m3_predictive_back_validate_event(event);
  if (rc != M3_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  predictive->active = M3_TRUE;
  predictive->last_event = *event;

  rc = m3_predictive_back_call(&predictive->handler, 0u, event);
  if (rc != M3_OK) {
    predictive->active = M3_FALSE;
    predictive->last_event = prev_event;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_predictive_back_progress(M3PredictiveBack *predictive,
                                        const M3PredictiveBackEvent *event) {
  M3PredictiveBackEvent prev_event; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }
  if (!predictive->active) {
    return M3_ERR_STATE;
  }

  rc = m3_predictive_back_validate_event(event);
  if (rc != M3_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  predictive->last_event = *event;

  rc = m3_predictive_back_call(&predictive->handler, 1u, event);
  if (rc != M3_OK) {
    predictive->last_event = prev_event;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_predictive_back_commit(M3PredictiveBack *predictive,
                                      const M3PredictiveBackEvent *event) {
  M3PredictiveBackEvent prev_event;
  M3Bool prev_active; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }
  if (!predictive->active) {
    return M3_ERR_STATE;
  }

  rc = m3_predictive_back_validate_event(event);
  if (rc != M3_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  prev_active = predictive->active;
  predictive->active = M3_FALSE;
  predictive->last_event = *event;

  rc = m3_predictive_back_call(&predictive->handler, 2u, event);
  if (rc != M3_OK) {
    predictive->active = prev_active;
    predictive->last_event = prev_event;
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_predictive_back_cancel(M3PredictiveBack *predictive,
                                      const M3PredictiveBackEvent *event) {
  M3PredictiveBackEvent prev_event;
  M3Bool prev_active; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return M3_ERR_STATE;
  }
  if (!predictive->active) {
    return M3_ERR_STATE;
  }

  rc = m3_predictive_back_validate_event(event);
  if (rc != M3_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  prev_active = predictive->active;
  predictive->active = M3_FALSE;
  predictive->last_event = *event;

  rc = m3_predictive_back_call(&predictive->handler, 3u, event);
  if (rc != M3_OK) {
    predictive->active = prev_active;
    predictive->last_event = prev_event;
    return rc;
  }

  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL
m3_predictive_back_test_validate_event(const M3PredictiveBackEvent *event) {
  return m3_predictive_back_validate_event(event);
}

int M3_CALL m3_predictive_back_test_validate_handler(
    const M3PredictiveBackHandler *handler) {
  return m3_predictive_back_validate_handler(handler);
}
#endif
