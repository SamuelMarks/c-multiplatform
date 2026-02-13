#include "cmpc/cmp_predictive.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_predictive_test_fail_event_init = CMP_FALSE;

int CMP_CALL cmp_predictive_test_set_event_init_fail(CMPBool enable) {
  g_cmp_predictive_test_fail_event_init = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif

static int cmp_predictive_back_validate_edge(cmp_u32 edge) {
  if (edge == CMP_PREDICTIVE_BACK_EDGE_UNKNOWN ||
      edge == CMP_PREDICTIVE_BACK_EDGE_LEFT ||
      edge == CMP_PREDICTIVE_BACK_EDGE_RIGHT) {
    return CMP_OK;
  }
  return CMP_ERR_RANGE;
}

static int
cmp_predictive_back_validate_event(const CMPPredictiveBackEvent *event) {
  int rc;

  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (event->progress < 0.0f || event->progress > 1.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_predictive_back_validate_edge(event->edge);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int
cmp_predictive_back_validate_handler(const CMPPredictiveBackHandler *handler) {
  if (handler == NULL || handler->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_predictive_back_call(const CMPPredictiveBackHandler *handler,
                                   cmp_u32 kind,
                                   const CMPPredictiveBackEvent *event) {
  if (handler == NULL || handler->vtable == NULL) {
    return CMP_OK;
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

  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_event_init(CMPPredictiveBackEvent *event) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_predictive_test_fail_event_init == CMP_TRUE) {
    g_cmp_predictive_test_fail_event_init = CMP_FALSE;
    return CMP_ERR_UNKNOWN;
  }
#endif

  memset(event, 0, sizeof(*event));
  event->edge = CMP_PREDICTIVE_BACK_EDGE_UNKNOWN;
  return CMP_OK;
}

int CMP_CALL
cmp_predictive_back_handler_init(CMPPredictiveBackHandler *handler, void *ctx,
                                const CMPPredictiveBackHandlerVTable *vtable) {
  if (handler == NULL || vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  handler->ctx = ctx;
  handler->vtable = vtable;
  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_init(CMPPredictiveBack *predictive,
                                    const CMPPredictiveBackHandler *handler) {
  int rc;

  if (predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (predictive->initialized) {
    return CMP_ERR_STATE;
  }

  if (handler != NULL) {
    rc = cmp_predictive_back_validate_handler(handler);
    if (rc != CMP_OK) {
      return rc;
    }
    predictive->handler = *handler;
  } else {
    predictive->handler.ctx = NULL;
    predictive->handler.vtable = NULL;
  }

  rc = cmp_predictive_back_event_init(&predictive->last_event);
  if (rc != CMP_OK) {
    return rc;
  }

  predictive->active = CMP_FALSE;
  predictive->initialized = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_shutdown(CMPPredictiveBack *predictive) {
  int rc;

  if (predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_predictive_back_event_init(&predictive->last_event);
  if (rc != CMP_OK) {
    return rc;
  }

  predictive->active = CMP_FALSE;
  predictive->handler.ctx = NULL;
  predictive->handler.vtable = NULL;
  predictive->initialized = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_set_handler(
    CMPPredictiveBack *predictive, const CMPPredictiveBackHandler *handler) {
  int rc;

  if (predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }

  if (handler != NULL) {
    rc = cmp_predictive_back_validate_handler(handler);
    if (rc != CMP_OK) {
      return rc;
    }
    predictive->handler = *handler;
  } else {
    predictive->handler.ctx = NULL;
    predictive->handler.vtable = NULL;
  }

  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_get_handler(
    const CMPPredictiveBack *predictive, CMPPredictiveBackHandler *out_handler) {
  if (predictive == NULL || out_handler == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }

  *out_handler = predictive->handler;
  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_get_state(const CMPPredictiveBack *predictive,
                                         CMPPredictiveBackState *out_state) {
  if (predictive == NULL || out_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }

  out_state->active = predictive->active;
  out_state->event = predictive->last_event;
  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_start(CMPPredictiveBack *predictive,
                                     const CMPPredictiveBackEvent *event) {
  CMPPredictiveBackEvent prev_event;
  int rc;

  if (predictive == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }
  if (predictive->active) {
    return CMP_ERR_STATE;
  }

  rc = cmp_predictive_back_validate_event(event);
  if (rc != CMP_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  predictive->active = CMP_TRUE;
  predictive->last_event = *event;

  rc = cmp_predictive_back_call(&predictive->handler, 0u, event);
  if (rc != CMP_OK) {
    predictive->active = CMP_FALSE;
    predictive->last_event = prev_event;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_progress(CMPPredictiveBack *predictive,
                                        const CMPPredictiveBackEvent *event) {
  CMPPredictiveBackEvent prev_event; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }
  if (!predictive->active) {
    return CMP_ERR_STATE;
  }

  rc = cmp_predictive_back_validate_event(event);
  if (rc != CMP_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  predictive->last_event = *event;

  rc = cmp_predictive_back_call(&predictive->handler, 1u, event);
  if (rc != CMP_OK) {
    predictive->last_event = prev_event;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_commit(CMPPredictiveBack *predictive,
                                      const CMPPredictiveBackEvent *event) {
  CMPPredictiveBackEvent prev_event;
  CMPBool prev_active; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }
  if (!predictive->active) {
    return CMP_ERR_STATE;
  }

  rc = cmp_predictive_back_validate_event(event);
  if (rc != CMP_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  prev_active = predictive->active;
  predictive->active = CMP_FALSE;
  predictive->last_event = *event;

  rc = cmp_predictive_back_call(&predictive->handler, 2u, event);
  if (rc != CMP_OK) {
    predictive->active = prev_active;
    predictive->last_event = prev_event;
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_predictive_back_cancel(CMPPredictiveBack *predictive,
                                      const CMPPredictiveBackEvent *event) {
  CMPPredictiveBackEvent prev_event;
  CMPBool prev_active; /* GCOVR_EXCL_LINE */
  int rc;

  if (predictive == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!predictive->initialized) {
    return CMP_ERR_STATE;
  }
  if (!predictive->active) {
    return CMP_ERR_STATE;
  }

  rc = cmp_predictive_back_validate_event(event);
  if (rc != CMP_OK) {
    return rc;
  }

  prev_event = predictive->last_event;
  prev_active = predictive->active;
  predictive->active = CMP_FALSE;
  predictive->last_event = *event;

  rc = cmp_predictive_back_call(&predictive->handler, 3u, event);
  if (rc != CMP_OK) {
    predictive->active = prev_active;
    predictive->last_event = prev_event;
    return rc;
  }

  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_predictive_back_test_validate_event(const CMPPredictiveBackEvent *event) {
  return cmp_predictive_back_validate_event(event);
}

int CMP_CALL cmp_predictive_back_test_validate_handler(
    const CMPPredictiveBackHandler *handler) {
  return cmp_predictive_back_validate_handler(handler);
}
#endif
