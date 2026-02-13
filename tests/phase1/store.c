#include "cmpc/cmp_store.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct CounterState {
  int value;
} CounterState;

typedef struct TestAllocCtx {
  int fail_after;
  int alloc_calls;
  int free_calls;
} TestAllocCtx;

static int CMP_CALL test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_after >= 0 &&
      state->alloc_calls >= state->fail_after) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = malloc((size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL test_realloc(void *ctx, void *ptr, cmp_usize size,
                                 void **out_ptr) {
  TestAllocCtx *state;
  void *mem;

  if (out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestAllocCtx *)ctx;
  if (state != NULL && state->fail_after >= 0 &&
      state->alloc_calls >= state->fail_after) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, (size_t)size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (state != NULL) {
    state->alloc_calls += 1;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL test_free(void *ctx, void *ptr) {
  TestAllocCtx *state;

  state = (TestAllocCtx *)ctx;
  if (ptr != NULL) {
    free(ptr);
  }
  if (state != NULL) {
    state->free_calls += 1;
  }
  return CMP_OK;
}

#define ACTION_ADD 1
#define ACTION_SET 2
#define ACTION_FAIL 3
#define ACTION_UNSUPPORTED 4

static int CMP_CALL counter_reducer(void *ctx, const CMPAction *action,
                                    const void *prev_state, void *next_state) {
  const CounterState *prev;
  CounterState *next;

  CMP_UNUSED(ctx);

  if (action == NULL || prev_state == NULL || next_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  prev = (const CounterState *)prev_state;
  next = (CounterState *)next_state;
  *next = *prev;

  switch (action->type) {
  case ACTION_ADD:
    if (action->data == NULL || action->size != sizeof(int)) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    next->value += *(const int *)action->data;
    return CMP_OK;
  case ACTION_SET:
    if (action->data == NULL || action->size != sizeof(int)) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    next->value = *(const int *)action->data;
    return CMP_OK;
  case ACTION_FAIL:
    return CMP_ERR_UNKNOWN;
  default:
    return CMP_ERR_UNSUPPORTED;
  }
}

static void init_alloc(TestAllocCtx *ctx, int fail_after) {
  ctx->fail_after = fail_after;
  ctx->alloc_calls = 0;
  ctx->free_calls = 0;
}

int main(void) {
  CMPStore store;
  CMPStore store_zero;
  CMPStore temp_store;
  CMPStoreConfig config;
  CMPStoreConfig bad_config;
  CMPAllocator bad_alloc;
  CMPAllocator test_allocator;
  TestAllocCtx alloc_ctx;
  CounterState initial;
  CounterState state;
  CounterState snapshot;
  const void *state_ptr;
  cmp_usize size;
  cmp_usize count;
  CMPBool can_undo;
  CMPBool can_redo;
  CMPAction action;
  int delta;
  cmp_usize max_size;

  memset(&store, 0, sizeof(store));
  memset(&store_zero, 0, sizeof(store_zero));
  memset(&temp_store, 0, sizeof(temp_store));
  memset(&bad_alloc, 0, sizeof(bad_alloc));

  initial.value = 1;

  memset(&config, 0, sizeof(config));
  config.allocator = NULL;
  config.state_size = sizeof(CounterState);
  config.history_capacity = 2;
  config.reducer = counter_reducer;
  config.reducer_ctx = NULL;

  CMP_TEST_EXPECT(cmp_store_init(NULL, &config, &initial),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_init(&store, NULL, &initial),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_config = config;
  bad_config.state_size = 0;
  bad_config.reducer = counter_reducer;
  CMP_TEST_EXPECT(cmp_store_init(&store, &bad_config, &initial),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_config.state_size = sizeof(CounterState);
  bad_config.reducer = NULL;
  CMP_TEST_EXPECT(cmp_store_init(&store, &bad_config, &initial),
                  CMP_ERR_INVALID_ARGUMENT);

  config.allocator = &bad_alloc;
  config.state_size = sizeof(CounterState);
  config.history_capacity = 2;
  config.reducer = counter_reducer;
  config.reducer_ctx = NULL;
  CMP_TEST_EXPECT(cmp_store_init(&store, &config, &initial),
                  CMP_ERR_INVALID_ARGUMENT);

  max_size = (cmp_usize) ~(cmp_usize)0;
  config.allocator = NULL;
  config.state_size = max_size;
  config.history_capacity = 2;
  config.reducer = counter_reducer;
  config.reducer_ctx = NULL;
  CMP_TEST_EXPECT(cmp_store_init(&store, &config, &initial), CMP_ERR_OVERFLOW);

  config.state_size = sizeof(CounterState);

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_store_init(&store, &config, &initial), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

  CMP_TEST_EXPECT(cmp_store_test_mul_overflow(1, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  {
    cmp_u8 history_buffer[8];
    cmp_usize history_count;
    CounterState history_state;

    history_state.value = 0;
    history_count = 0;
    CMP_TEST_EXPECT(cmp_store_test_history_push(NULL, 1, sizeof(history_state),
                                                &history_count, &history_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_history_push(history_buffer, 1,
                                                sizeof(history_state), NULL,
                                                &history_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_history_push(history_buffer, 1,
                                                sizeof(history_state),
                                                &history_count, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_store_test_history_pop(NULL, 1, sizeof(history_state),
                                               &history_count, &history_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_history_pop(history_buffer, 1,
                                               sizeof(history_state), NULL,
                                               &history_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_history_pop(history_buffer, 1,
                                               sizeof(history_state),
                                               &history_count, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_history_pop(history_buffer, 0,
                                               sizeof(history_state),
                                               &history_count, &history_state),
                    CMP_ERR_NOT_FOUND);

    CMP_TEST_EXPECT(cmp_store_test_copy_history(NULL, 1, sizeof(history_state),
                                                1, 0, &history_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_store_test_copy_history(history_buffer, 0,
                                                sizeof(history_state), 1, 0,
                                                &history_state),
                    CMP_ERR_NOT_FOUND);
  }
#endif

  init_alloc(&alloc_ctx, 0);
  test_allocator.ctx = &alloc_ctx;
  test_allocator.alloc = test_alloc;
  test_allocator.realloc = test_realloc;
  test_allocator.free = test_free;
  config.allocator = &test_allocator;
  config.history_capacity = 1;
  CMP_TEST_EXPECT(cmp_store_init(&temp_store, &config, &initial),
                  CMP_ERR_OUT_OF_MEMORY);

  init_alloc(&alloc_ctx, 1);
  CMP_TEST_EXPECT(cmp_store_init(&temp_store, &config, &initial),
                  CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_ASSERT(alloc_ctx.free_calls == 1);

  init_alloc(&alloc_ctx, 2);
  CMP_TEST_EXPECT(cmp_store_init(&temp_store, &config, &initial),
                  CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_ASSERT(alloc_ctx.free_calls == 2);

  init_alloc(&alloc_ctx, 3);
  CMP_TEST_EXPECT(cmp_store_init(&temp_store, &config, &initial),
                  CMP_ERR_OUT_OF_MEMORY);
  CMP_TEST_ASSERT(alloc_ctx.free_calls == 3);

  config.allocator = NULL;
  config.history_capacity = 2;
  CMP_TEST_OK(cmp_store_init(&store, &config, &initial));

  CMP_TEST_EXPECT(cmp_store_get_state_ptr(NULL, &state_ptr, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_state_ptr(&store, NULL, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_state_ptr(&store, &state_ptr, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_store_get_state_ptr(&store, &state_ptr, &size));
  CMP_TEST_ASSERT(size == sizeof(CounterState));
  CMP_TEST_ASSERT(((const CounterState *)state_ptr)->value == 1);

  CMP_TEST_EXPECT(cmp_store_get_state(NULL, &state, sizeof(state)),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_state(&store, NULL, sizeof(state)),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_state(&store, &state, sizeof(state) - 1),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 1);

  CMP_TEST_EXPECT(cmp_store_can_undo(NULL, &can_undo),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_can_undo(&store, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_can_redo(NULL, &can_redo),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_can_redo(&store, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_store_can_undo(&store, &can_undo));
  CMP_TEST_OK(cmp_store_can_redo(&store, &can_redo));
  CMP_TEST_ASSERT(can_undo == CMP_FALSE);
  CMP_TEST_ASSERT(can_redo == CMP_FALSE);

  delta = 0;
  action.type = ACTION_SET;
  action.data = &delta;
  action.size = sizeof(delta);
  CMP_TEST_EXPECT(cmp_store_dispatch(NULL, &action), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_dispatch(&store, NULL), CMP_ERR_INVALID_ARGUMENT);

  action.type = ACTION_ADD;
  action.data = NULL;
  action.size = sizeof(int);
  CMP_TEST_EXPECT(cmp_store_dispatch(&store, &action),
                  CMP_ERR_INVALID_ARGUMENT);

  action.type = ACTION_UNSUPPORTED;
  action.data = NULL;
  action.size = 0;
  CMP_TEST_EXPECT(cmp_store_dispatch(&store, &action), CMP_ERR_UNSUPPORTED);

  action.type = ACTION_FAIL;
  action.data = NULL;
  action.size = 0;
  CMP_TEST_EXPECT(cmp_store_dispatch(&store, &action), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 1);

  delta = 1;
  action.type = ACTION_ADD;
  action.data = &delta;
  action.size = sizeof(delta);
  CMP_TEST_OK(cmp_store_dispatch(&store, &action));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 2);

  CMP_TEST_OK(cmp_store_get_undo_count(&store, &count));
  CMP_TEST_ASSERT(count == 1);
  CMP_TEST_OK(cmp_store_get_redo_count(&store, &count));
  CMP_TEST_ASSERT(count == 0);

  CMP_TEST_OK(cmp_store_dispatch(&store, &action));
  CMP_TEST_OK(cmp_store_dispatch(&store, &action));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 4);

  CMP_TEST_OK(cmp_store_get_undo_count(&store, &count));
  CMP_TEST_ASSERT(count == 2);

  CMP_TEST_OK(
      cmp_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot)));
  CMP_TEST_ASSERT(snapshot.value == 3);
  CMP_TEST_OK(
      cmp_store_copy_undo_state(&store, 1, &snapshot, sizeof(snapshot)));
  CMP_TEST_ASSERT(snapshot.value == 2);
  CMP_TEST_EXPECT(
      cmp_store_copy_undo_state(&store, 2, &snapshot, sizeof(snapshot)),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(
      cmp_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot) - 1),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_store_copy_undo_state(&store, 0, NULL, sizeof(snapshot)),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_store_undo(&store));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 3);
  CMP_TEST_OK(cmp_store_undo(&store));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 2);
  CMP_TEST_EXPECT(cmp_store_undo(&store), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_store_get_redo_count(&store, &count));
  CMP_TEST_ASSERT(count == 2);
  CMP_TEST_OK(
      cmp_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot)));
  CMP_TEST_ASSERT(snapshot.value == 3);
  CMP_TEST_OK(
      cmp_store_copy_redo_state(&store, 1, &snapshot, sizeof(snapshot)));
  CMP_TEST_ASSERT(snapshot.value == 4);
  CMP_TEST_EXPECT(cmp_store_copy_redo_state(&store, 0, NULL, sizeof(snapshot)),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_store_redo(&store));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 3);
  CMP_TEST_OK(cmp_store_redo(&store));
  CMP_TEST_OK(cmp_store_get_state(&store, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 4);
  CMP_TEST_EXPECT(cmp_store_redo(&store), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_store_clear_history(&store));
  CMP_TEST_OK(cmp_store_get_undo_count(&store, &count));
  CMP_TEST_ASSERT(count == 0);
  CMP_TEST_OK(cmp_store_get_redo_count(&store, &count));
  CMP_TEST_ASSERT(count == 0);

  CMP_TEST_EXPECT(
      cmp_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(
      cmp_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(
      cmp_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot) - 1),
      CMP_ERR_RANGE);

  CMP_TEST_EXPECT(cmp_store_get_undo_count(NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_undo_count(&store, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_redo_count(NULL, &count),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_redo_count(&store, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    CMPStore temp_store_local;
    void *saved_undo;
    void *saved_redo;
    cmp_usize saved_undo_count;
    cmp_usize saved_redo_count;

    memset(&temp_store_local, 0, sizeof(temp_store_local));
    CMP_TEST_OK(cmp_store_init(&temp_store_local, &config, &initial));

    delta = 1;
    action.type = ACTION_ADD;
    action.data = &delta;
    action.size = sizeof(delta);
    CMP_TEST_OK(cmp_store_dispatch(&temp_store_local, &action));

    saved_undo = temp_store_local.undo_buffer;
    saved_redo = temp_store_local.redo_buffer;
    saved_undo_count = temp_store_local.undo_count;
    saved_redo_count = temp_store_local.redo_count;

    temp_store_local.undo_buffer = NULL;
    CMP_TEST_EXPECT(cmp_store_dispatch(&temp_store_local, &action),
                    CMP_ERR_INVALID_ARGUMENT);
    temp_store_local.undo_buffer = saved_undo;

    temp_store_local.redo_buffer = NULL;
    temp_store_local.undo_count = saved_undo_count;
    CMP_TEST_EXPECT(cmp_store_undo(&temp_store_local),
                    CMP_ERR_INVALID_ARGUMENT);
    temp_store_local.redo_buffer = saved_redo;

    temp_store_local.undo_buffer = NULL;
    temp_store_local.undo_count = saved_undo_count;
    temp_store_local.redo_count = saved_redo_count;
    CMP_TEST_EXPECT(cmp_store_undo(&temp_store_local),
                    CMP_ERR_INVALID_ARGUMENT);
    temp_store_local.undo_buffer = saved_undo;

    temp_store_local.undo_buffer = NULL;
    temp_store_local.redo_count = 1;
    CMP_TEST_EXPECT(cmp_store_redo(&temp_store_local),
                    CMP_ERR_INVALID_ARGUMENT);
    temp_store_local.undo_buffer = saved_undo;

    temp_store_local.redo_buffer = NULL;
    temp_store_local.redo_count = 1;
    CMP_TEST_EXPECT(cmp_store_redo(&temp_store_local),
                    CMP_ERR_INVALID_ARGUMENT);
    temp_store_local.redo_buffer = saved_redo;
    temp_store_local.redo_count = saved_redo_count;
    temp_store_local.undo_count = saved_undo_count;

    CMP_TEST_OK(cmp_store_shutdown(&temp_store_local));
  }

  CMP_TEST_OK(cmp_store_shutdown(&store));
  CMP_TEST_EXPECT(cmp_store_shutdown(&store), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_store_dispatch(&store, &action), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_get_state(&store, &state, sizeof(state)),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_can_undo(&store, &can_undo), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_can_redo(&store, &can_redo), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_undo(&store), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_redo(&store), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_get_undo_count(&store, &count), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_get_redo_count(&store, &count), CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_STATE);

  config.allocator = NULL;
  config.history_capacity = 0;
  CMP_TEST_OK(cmp_store_init(&store_zero, &config, NULL));
  action.type = ACTION_SET;
  delta = 5;
  action.data = &delta;
  action.size = sizeof(delta);
  CMP_TEST_OK(cmp_store_dispatch(&store_zero, &action));
  CMP_TEST_OK(cmp_store_get_state(&store_zero, &state, sizeof(state)));
  CMP_TEST_ASSERT(state.value == 5);
  CMP_TEST_EXPECT(cmp_store_undo(&store_zero), CMP_ERR_NOT_FOUND);
  CMP_TEST_EXPECT(cmp_store_redo(&store_zero), CMP_ERR_NOT_FOUND);
  CMP_TEST_OK(cmp_store_shutdown(&store_zero));

  CMP_TEST_EXPECT(cmp_store_undo(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_redo(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_clear_history(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_store_copy_undo_state(NULL, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_store_copy_redo_state(NULL, 0, &snapshot, sizeof(snapshot)),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_store_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_store_get_state_ptr(&store, &state_ptr, &size),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_store_clear_history(&store), CMP_ERR_STATE);

  return 0;
}
