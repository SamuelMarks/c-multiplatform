#include "cmpc/cmp_predictive.h"
#include "test_utils.h"

#include <string.h>

int CMP_CALL cmp_predictive_test_set_event_init_fail(CMPBool enable);

typedef struct PredictiveHandlerState {
  int start_count;
  int progress_count;
  int commit_count;
  int cancel_count;
  CMPBool fail_start;
  CMPBool fail_progress;
  CMPBool fail_commit;
  CMPBool fail_cancel;
} PredictiveHandlerState;

static int predictive_on_start(void *ctx, const CMPPredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->start_count += 1;
  if (state->fail_start) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int predictive_on_progress(void *ctx,
                                  const CMPPredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->progress_count += 1;
  if (state->fail_progress) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int predictive_on_commit(void *ctx,
                                const CMPPredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->commit_count += 1;
  if (state->fail_commit) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int predictive_on_cancel(void *ctx,
                                const CMPPredictiveBackEvent *event) {
  PredictiveHandlerState *state;

  if (ctx == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  state = (PredictiveHandlerState *)ctx;
  state->cancel_count += 1;
  if (state->fail_cancel) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int test_event_and_handler_init(void) {
  CMPPredictiveBackEvent event;
  CMPPredictiveBackHandler handler;
  CMPPredictiveBackHandlerVTable vtable;
  int rc;

  CMP_TEST_EXPECT(cmp_predictive_back_event_init(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_predictive_back_event_init(&event));
  CMP_TEST_ASSERT(event.progress == 0.0f);
  CMP_TEST_ASSERT(event.x == 0.0f);
  CMP_TEST_ASSERT(event.y == 0.0f);
  CMP_TEST_ASSERT(event.velocity_x == 0.0f);
  CMP_TEST_ASSERT(event.velocity_y == 0.0f);
  CMP_TEST_ASSERT(event.edge == CMP_PREDICTIVE_BACK_EDGE_UNKNOWN);
  CMP_TEST_ASSERT(event.time_ms == 0u);

  CMP_TEST_EXPECT(cmp_predictive_back_handler_init(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_handler_init(&handler, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&vtable, 0, sizeof(vtable));
  vtable.on_start = predictive_on_start;
  vtable.on_progress = predictive_on_progress;
  vtable.on_commit = predictive_on_commit;
  vtable.on_cancel = predictive_on_cancel;

  rc = cmp_predictive_back_handler_init(&handler, &event, &vtable);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(handler.ctx == &event);
  CMP_TEST_ASSERT(handler.vtable == &vtable);

  return CMP_OK;
}

static int test_controller_init_and_handler(void) {
  CMPPredictiveBack predictive;
  CMPPredictiveBackHandler handler;
  CMPPredictiveBackHandlerVTable vtable;
  CMPPredictiveBackHandlerVTable empty_vtable;
  CMPPredictiveBackEvent event;
  CMPPredictiveBackState state;

  CMP_TEST_EXPECT(cmp_predictive_back_init(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_get_state(NULL, &state),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_get_state(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&predictive, 0, sizeof(predictive));
  CMP_TEST_OK(cmp_predictive_test_set_event_init_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_predictive_back_init(&predictive, NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_predictive_test_set_event_init_fail(CMP_FALSE));

  memset(&predictive, 0, sizeof(predictive));
  memset(&vtable, 0, sizeof(vtable));
  handler.ctx = NULL;
  handler.vtable = NULL;
  CMP_TEST_EXPECT(cmp_predictive_back_init(&predictive, &handler),
                  CMP_ERR_INVALID_ARGUMENT);

  vtable.on_start = predictive_on_start;
  handler.vtable = &vtable;
  CMP_TEST_OK(cmp_predictive_back_init(&predictive, &handler));
  CMP_TEST_EXPECT(cmp_predictive_back_init(&predictive, &handler),
                  CMP_ERR_STATE);

  CMP_TEST_ASSERT(predictive.initialized == CMP_TRUE);
  CMP_TEST_ASSERT(predictive.active == CMP_FALSE);
  CMP_TEST_ASSERT(predictive.handler.vtable == &vtable);

  CMP_TEST_EXPECT(cmp_predictive_back_get_handler(NULL, &handler),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_get_handler(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_predictive_back_get_handler(&predictive, &handler));
  CMP_TEST_ASSERT(handler.vtable == &vtable);

  CMP_TEST_EXPECT(cmp_predictive_back_set_handler(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  handler.ctx = &event;
  handler.vtable = NULL;
  CMP_TEST_EXPECT(cmp_predictive_back_set_handler(&predictive, &handler),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_predictive_back_set_handler(&predictive, NULL));
  CMP_TEST_ASSERT(predictive.handler.vtable == NULL);

  CMP_TEST_OK(cmp_predictive_back_get_state(&predictive, &state));
  CMP_TEST_ASSERT(state.active == CMP_FALSE);
  CMP_TEST_ASSERT(state.event.edge == CMP_PREDICTIVE_BACK_EDGE_UNKNOWN);

  CMP_TEST_OK(cmp_predictive_back_event_init(&event));
  event.edge = CMP_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.0f;
  CMP_TEST_OK(cmp_predictive_back_start(&predictive, &event));
  CMP_TEST_OK(cmp_predictive_back_cancel(&predictive, &event));

  memset(&empty_vtable, 0, sizeof(empty_vtable));
  handler.vtable = &empty_vtable;
  CMP_TEST_OK(cmp_predictive_back_set_handler(&predictive, &handler));
  CMP_TEST_OK(cmp_predictive_back_start(&predictive, &event));
  CMP_TEST_OK(cmp_predictive_back_cancel(&predictive, &event));

  CMP_TEST_OK(cmp_predictive_back_get_state(&predictive, &state));
  CMP_TEST_ASSERT(state.active == CMP_FALSE);
  CMP_TEST_ASSERT(state.event.edge == CMP_PREDICTIVE_BACK_EDGE_LEFT);

  CMP_TEST_OK(cmp_predictive_test_set_event_init_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_predictive_back_shutdown(&predictive), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_predictive_test_set_event_init_fail(CMP_FALSE));

  CMP_TEST_OK(cmp_predictive_back_shutdown(&predictive));
  CMP_TEST_EXPECT(cmp_predictive_back_shutdown(&predictive), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_get_handler(&predictive, &handler),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_get_state(&predictive, &state),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_set_handler(&predictive, NULL),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, &event),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_progress(&predictive, &event),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_commit(&predictive, &event),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(&predictive, &event),
                  CMP_ERR_STATE);

  return CMP_OK;
}

static int test_event_flow_and_errors(void) {
  CMPPredictiveBack predictive;
  CMPPredictiveBackHandler handler;
  CMPPredictiveBackHandlerVTable vtable;
  PredictiveHandlerState handler_state;
  CMPPredictiveBackEvent event;
  CMPPredictiveBackState state;
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
  CMP_TEST_OK(cmp_predictive_back_init(&predictive, &handler));

  CMP_TEST_OK(cmp_predictive_back_event_init(&event));
  event.edge = CMP_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.0f;

  CMP_TEST_EXPECT(cmp_predictive_back_start(NULL, &event),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_progress(NULL, &event),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_progress(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_commit(NULL, &event),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_commit(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(NULL, &event),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(&predictive, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_predictive_back_progress(&predictive, &event),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_commit(&predictive, &event),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(&predictive, &event),
                  CMP_ERR_STATE);

  CMP_TEST_OK(cmp_predictive_back_start(&predictive, &event));
  CMP_TEST_ASSERT(handler_state.start_count == 1);

  event.progress = -0.1f;
  CMP_TEST_EXPECT(cmp_predictive_back_progress(&predictive, &event),
                  CMP_ERR_RANGE);
  event.progress = 0.2f;
  event.edge = 77u;
  CMP_TEST_EXPECT(cmp_predictive_back_commit(&predictive, &event),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(&predictive, &event),
                  CMP_ERR_RANGE);
  event.edge = CMP_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.5f;

  rc = cmp_predictive_back_start(&predictive, &event);
  CMP_TEST_EXPECT(rc, CMP_ERR_STATE);

  event.progress = 0.5f;
  CMP_TEST_OK(cmp_predictive_back_progress(&predictive, &event));
  CMP_TEST_ASSERT(handler_state.progress_count == 1);

  event.progress = 1.0f;
  CMP_TEST_OK(cmp_predictive_back_commit(&predictive, &event));
  CMP_TEST_ASSERT(handler_state.commit_count == 1);

  CMP_TEST_OK(cmp_predictive_back_start(&predictive, &event));
  CMP_TEST_OK(cmp_predictive_back_cancel(&predictive, &event));
  CMP_TEST_ASSERT(handler_state.cancel_count == 1);

  rc = cmp_predictive_back_get_state(&predictive, &state);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(state.active == CMP_FALSE);

  event.progress = -0.1f;
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, &event),
                  CMP_ERR_RANGE);
  event.progress = 1.1f;
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, &event),
                  CMP_ERR_RANGE);
  event.progress = 0.2f;
  event.edge = 77u;
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, &event),
                  CMP_ERR_RANGE);

  event.edge = CMP_PREDICTIVE_BACK_EDGE_RIGHT;
  event.progress = 0.2f;
  handler_state.fail_start = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_predictive_back_start(&predictive, &event),
                  CMP_ERR_UNKNOWN);
  handler_state.fail_start = CMP_FALSE;
  CMP_TEST_OK(cmp_predictive_back_start(&predictive, &event));
  handler_state.fail_progress = CMP_TRUE;
  event.progress = 0.4f;
  CMP_TEST_EXPECT(cmp_predictive_back_progress(&predictive, &event),
                  CMP_ERR_UNKNOWN);
  handler_state.fail_progress = CMP_FALSE;
  CMP_TEST_OK(cmp_predictive_back_progress(&predictive, &event));

  handler_state.fail_commit = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_predictive_back_commit(&predictive, &event),
                  CMP_ERR_UNKNOWN);
  handler_state.fail_commit = CMP_FALSE;
  handler_state.fail_cancel = CMP_TRUE;
  CMP_TEST_EXPECT(cmp_predictive_back_cancel(&predictive, &event),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_ASSERT(predictive.active == CMP_TRUE);
  handler_state.fail_cancel = CMP_FALSE;
  CMP_TEST_OK(cmp_predictive_back_cancel(&predictive, &event));

  CMP_TEST_OK(cmp_predictive_back_shutdown(&predictive));
  return CMP_OK;
}

static int test_validation_wrappers(void) {
  CMPPredictiveBackEvent event;
  CMPPredictiveBackHandler handler;
  CMPPredictiveBackHandlerVTable vtable;

  CMP_TEST_EXPECT(cmp_predictive_back_test_validate_event(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_predictive_back_event_init(&event));
  event.edge = 44u;
  CMP_TEST_EXPECT(cmp_predictive_back_test_validate_event(&event),
                  CMP_ERR_RANGE);
  event.edge = CMP_PREDICTIVE_BACK_EDGE_LEFT;
  event.progress = 0.5f;
  CMP_TEST_OK(cmp_predictive_back_test_validate_event(&event));

  CMP_TEST_EXPECT(cmp_predictive_back_test_validate_handler(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  handler.ctx = NULL;
  handler.vtable = NULL;
  CMP_TEST_EXPECT(cmp_predictive_back_test_validate_handler(&handler),
                  CMP_ERR_INVALID_ARGUMENT);
  memset(&vtable, 0, sizeof(vtable));
  handler.vtable = &vtable;
  CMP_TEST_OK(cmp_predictive_back_test_validate_handler(&handler));

  return CMP_OK;
}

int main(void) {
  CMP_TEST_OK(test_event_and_handler_init());
  CMP_TEST_OK(test_controller_init_and_handler());
  CMP_TEST_OK(test_event_flow_and_errors());
  CMP_TEST_OK(test_validation_wrappers());
  return 0;
}
