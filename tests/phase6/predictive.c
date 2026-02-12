#include "m3/m3_predictive.h"
#include "test_utils.h"

#include <string.h>

int M3_CALL m3_predictive_test_set_event_init_fail(M3Bool enable);

typedef struct PredictiveHandlerState {
  int start_count;
  int progress_count;
  int commit_count;
  int cancel_count;
  M3Bool fail_start;
  M3Bool fail_progress;
  M3Bool fail_commit;
  M3Bool fail_cancel;
} PredictiveHandlerState;

static int predictive_on_start(void *ctx, const M3PredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->start_count += 1;
  if (state->fail_start) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int predictive_on_progress(void *ctx,
                                  const M3PredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->progress_count += 1;
  if (state->fail_progress) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int predictive_on_commit(void *ctx, const M3PredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->commit_count += 1;
  if (state->fail_commit) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int predictive_on_cancel(void *ctx, const M3PredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->cancel_count += 1;
  if (state->fail_cancel) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int test_event_and_handler_init(void) {
  M3PredictiveBackEvent event;
  M3PredictiveBackHandler handler;
  M3PredictiveBackHandlerVTable vtable;
  int rc;

  M3_TEST_EXPECT(m3_predictive_back_event_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_predictive_back_event_init(&event));
  M3_TEST_ASSERT(event.progress == 0.0f);
  M3_TEST_ASSERT(event.x == 0.0f);
  M3_TEST_ASSERT(event.y == 0.0f);
  M3_TEST_ASSERT(event.velocity_x == 0.0f);
  M3_TEST_ASSERT(event.velocity_y == 0.0f);
  M3_TEST_ASSERT(event.edge == M3_PREDICTIVE_BACK_EDGE_UNKNOWN);
  M3_TEST_ASSERT(event.time_ms == 0u);

  M3_TEST_EXPECT(m3_predictive_back_handler_init(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_handler_init(&handler, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  memset(&vtable, 0, sizeof(vtable));
  vtable.on_start = predictive_on_start;
  vtable.on_progress = predictive_on_progress;
  vtable.on_commit = predictive_on_commit;
  vtable.on_cancel = predictive_on_cancel;

  rc = m3_predictive_back_handler_init(&handler, &event, &vtable);
  M3_TEST_OK(rc);
  M3_TEST_ASSERT(handler.ctx == &event);
  M3_TEST_ASSERT(handler.vtable == &vtable);

  return M3_OK;
}

static int test_controller_init_and_handler(void) {
  M3PredictiveBack predictive;
  M3PredictiveBackHandler handler;
  M3PredictiveBackHandlerVTable vtable;
  M3PredictiveBackHandlerVTable empty_vtable;
  M3PredictiveBackEvent event;
  M3PredictiveBackState state;

  M3_TEST_EXPECT(m3_predictive_back_init(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_get_state(NULL, &state),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_get_state(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  memset(&predictive, 0, sizeof(predictive));
  M3_TEST_OK(m3_predictive_test_set_event_init_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_predictive_back_init(&predictive, NULL), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_predictive_test_set_event_init_fail(M3_FALSE));

  memset(&predictive, 0, sizeof(predictive));
  memset(&vtable, 0, sizeof(vtable));
  handler.ctx = NULL;
  handler.vtable = NULL;
  M3_TEST_EXPECT(m3_predictive_back_init(&predictive, &handler),
                 M3_ERR_INVALID_ARGUMENT);

  vtable.on_start = predictive_on_start;
  handler.vtable = &vtable;
  M3_TEST_OK(m3_predictive_back_init(&predictive, &handler));
  M3_TEST_EXPECT(m3_predictive_back_init(&predictive, &handler), M3_ERR_STATE);

  M3_TEST_ASSERT(predictive.initialized == M3_TRUE);
  M3_TEST_ASSERT(predictive.active == M3_FALSE);
  M3_TEST_ASSERT(predictive.handler.vtable == &vtable);

  M3_TEST_EXPECT(m3_predictive_back_get_handler(NULL, &handler),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_get_handler(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_predictive_back_get_handler(&predictive, &handler));
  M3_TEST_ASSERT(handler.vtable == &vtable);

  M3_TEST_EXPECT(m3_predictive_back_set_handler(NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  handler.ctx = &event;
  handler.vtable = NULL;
  M3_TEST_EXPECT(m3_predictive_back_set_handler(&predictive, &handler),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_predictive_back_set_handler(&predictive, NULL));
  M3_TEST_ASSERT(predictive.handler.vtable == NULL);

  M3_TEST_OK(m3_predictive_back_get_state(&predictive, &state));
  M3_TEST_ASSERT(state.active == M3_FALSE);
  M3_TEST_ASSERT(state.event.edge == M3_PREDICTIVE_BACK_EDGE_UNKNOWN);

  M3_TEST_OK(m3_predictive_back_event_init(&event));
  event.edge = M3_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.0f;
  M3_TEST_OK(m3_predictive_back_start(&predictive, &event));
  M3_TEST_OK(m3_predictive_back_cancel(&predictive, &event));

  memset(&empty_vtable, 0, sizeof(empty_vtable));
  handler.vtable = &empty_vtable;
  M3_TEST_OK(m3_predictive_back_set_handler(&predictive, &handler));
  M3_TEST_OK(m3_predictive_back_start(&predictive, &event));
  M3_TEST_OK(m3_predictive_back_cancel(&predictive, &event));

  M3_TEST_OK(m3_predictive_back_get_state(&predictive, &state));
  M3_TEST_ASSERT(state.active == M3_FALSE);
  M3_TEST_ASSERT(state.event.edge == M3_PREDICTIVE_BACK_EDGE_LEFT);

  M3_TEST_OK(m3_predictive_test_set_event_init_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_predictive_back_shutdown(&predictive), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_predictive_test_set_event_init_fail(M3_FALSE));

  M3_TEST_OK(m3_predictive_back_shutdown(&predictive));
  M3_TEST_EXPECT(m3_predictive_back_shutdown(&predictive), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_get_handler(&predictive, &handler),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_get_state(&predictive, &state),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_set_handler(&predictive, NULL),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, &event), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_progress(&predictive, &event),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_commit(&predictive, &event), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_cancel(&predictive, &event), M3_ERR_STATE);

  return M3_OK;
}

static int test_event_flow_and_errors(void) {
  M3PredictiveBack predictive;
  M3PredictiveBackHandler handler;
  M3PredictiveBackHandlerVTable vtable;
  PredictiveHandlerState handler_state;
  M3PredictiveBackEvent event;
  M3PredictiveBackState state;
  int rc;

  memset(&handler_state, 0, sizeof(handler_state));
  memset(&vtable, 0, sizeof(vtable));
  vtable.on_start = predictive_on_start;
  vtable.on_progress = predictive_on_progress;
  vtable.on_commit = predictive_on_commit;
  vtable.on_cancel = predictive_on_cancel;

  handler.ctx = &handler_state;
  handler.vtable = &vtable;

  memset(&predictive, 0, sizeof(predictive));
  M3_TEST_OK(m3_predictive_back_init(&predictive, &handler));

  M3_TEST_OK(m3_predictive_back_event_init(&event));
  event.edge = M3_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.0f;

  M3_TEST_EXPECT(m3_predictive_back_start(NULL, &event),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_progress(NULL, &event),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_progress(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_commit(NULL, &event),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_commit(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_cancel(NULL, &event),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_predictive_back_cancel(&predictive, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_predictive_back_progress(&predictive, &event),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_commit(&predictive, &event), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_predictive_back_cancel(&predictive, &event), M3_ERR_STATE);

  M3_TEST_OK(m3_predictive_back_start(&predictive, &event));
  M3_TEST_ASSERT(handler_state.start_count == 1);

  event.progress = -0.1f;
  M3_TEST_EXPECT(m3_predictive_back_progress(&predictive, &event),
                 M3_ERR_RANGE);
  event.progress = 0.2f;
  event.edge = 77u;
  M3_TEST_EXPECT(m3_predictive_back_commit(&predictive, &event), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_predictive_back_cancel(&predictive, &event), M3_ERR_RANGE);
  event.edge = M3_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.5f;

  rc = m3_predictive_back_start(&predictive, &event);
  M3_TEST_EXPECT(rc, M3_ERR_STATE);

  event.progress = 0.5f;
  M3_TEST_OK(m3_predictive_back_progress(&predictive, &event));
  M3_TEST_ASSERT(handler_state.progress_count == 1);

  event.progress = 1.0f;
  M3_TEST_OK(m3_predictive_back_commit(&predictive, &event));
  M3_TEST_ASSERT(handler_state.commit_count == 1);

  M3_TEST_OK(m3_predictive_back_start(&predictive, &event));
  M3_TEST_OK(m3_predictive_back_cancel(&predictive, &event));
  M3_TEST_ASSERT(handler_state.cancel_count == 1);

  rc = m3_predictive_back_get_state(&predictive, &state);
  M3_TEST_OK(rc);
  M3_TEST_ASSERT(state.active == M3_FALSE);

  event.progress = -0.1f;
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, &event), M3_ERR_RANGE);
  event.progress = 1.1f;
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, &event), M3_ERR_RANGE);
  event.progress = 0.2f;
  event.edge = 77u;
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, &event), M3_ERR_RANGE);

  event.edge = M3_PREDICTIVE_BACK_EDGE_RIGHT;
  event.progress = 0.2f;
  handler_state.fail_start = M3_TRUE;
  M3_TEST_EXPECT(m3_predictive_back_start(&predictive, &event), M3_ERR_UNKNOWN);
  handler_state.fail_start = M3_FALSE;
  M3_TEST_OK(m3_predictive_back_start(&predictive, &event));
  handler_state.fail_progress = M3_TRUE;
  event.progress = 0.4f;
  M3_TEST_EXPECT(m3_predictive_back_progress(&predictive, &event),
                 M3_ERR_UNKNOWN);
  handler_state.fail_progress = M3_FALSE;
  M3_TEST_OK(m3_predictive_back_progress(&predictive, &event));

  handler_state.fail_commit = M3_TRUE;
  M3_TEST_EXPECT(m3_predictive_back_commit(&predictive, &event),
                 M3_ERR_UNKNOWN);
  handler_state.fail_commit = M3_FALSE;
  handler_state.fail_cancel = M3_TRUE;
  M3_TEST_EXPECT(m3_predictive_back_cancel(&predictive, &event),
                 M3_ERR_UNKNOWN);
  M3_TEST_ASSERT(predictive.active == M3_TRUE);
  handler_state.fail_cancel = M3_FALSE;
  M3_TEST_OK(m3_predictive_back_cancel(&predictive, &event));

  M3_TEST_OK(m3_predictive_back_shutdown(&predictive));
  return M3_OK;
}

static int test_validation_wrappers(void) {
  M3PredictiveBackEvent event;
  M3PredictiveBackHandler handler;
  M3PredictiveBackHandlerVTable vtable;

  M3_TEST_EXPECT(m3_predictive_back_test_validate_event(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_predictive_back_event_init(&event));
  event.edge = 44u;
  M3_TEST_EXPECT(m3_predictive_back_test_validate_event(&event), M3_ERR_RANGE);
  event.edge = M3_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.5f;
  M3_TEST_OK(m3_predictive_back_test_validate_event(&event));

  M3_TEST_EXPECT(m3_predictive_back_test_validate_handler(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  handler.ctx = NULL;
  handler.vtable = NULL;
  M3_TEST_EXPECT(m3_predictive_back_test_validate_handler(&handler),
                 M3_ERR_INVALID_ARGUMENT);
  memset(&vtable, 0, sizeof(vtable));
  handler.vtable = &vtable;
  M3_TEST_OK(m3_predictive_back_test_validate_handler(&handler));

  return M3_OK;
}

int main(void) {
  M3_TEST_OK(test_event_and_handler_init());
  M3_TEST_OK(test_controller_init_and_handler());
  M3_TEST_OK(test_event_flow_and_errors());
  M3_TEST_OK(test_validation_wrappers());
  return 0;
}
