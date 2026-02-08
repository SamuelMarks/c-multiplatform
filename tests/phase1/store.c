#include "test_utils.h"
#include "m3/m3_store.h"

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

static int M3_CALL test_alloc(void *ctx, m3_usize size, void **out_ptr)
{
    TestAllocCtx *state;
    void *mem;

    if (out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_ptr = NULL;

    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestAllocCtx *)ctx;
    if (state != NULL && state->fail_after >= 0 && state->alloc_calls >= state->fail_after) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    mem = malloc((size_t)size);
    if (mem == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    if (state != NULL) {
        state->alloc_calls += 1;
    }

    *out_ptr = mem;
    return M3_OK;
}

static int M3_CALL test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
    TestAllocCtx *state;
    void *mem;

    if (out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_ptr = NULL;

    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestAllocCtx *)ctx;
    if (state != NULL && state->fail_after >= 0 && state->alloc_calls >= state->fail_after) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    mem = realloc(ptr, (size_t)size);
    if (mem == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    if (state != NULL) {
        state->alloc_calls += 1;
    }

    *out_ptr = mem;
    return M3_OK;
}

static int M3_CALL test_free(void *ctx, void *ptr)
{
    TestAllocCtx *state;

    state = (TestAllocCtx *)ctx;
    if (ptr != NULL) {
        free(ptr);
    }
    if (state != NULL) {
        state->free_calls += 1;
    }
    return M3_OK;
}

#define ACTION_ADD 1
#define ACTION_SET 2
#define ACTION_FAIL 3
#define ACTION_UNSUPPORTED 4

static int M3_CALL counter_reducer(void *ctx, const M3Action *action, const void *prev_state, void *next_state)
{
    const CounterState *prev;
    CounterState *next;

    M3_UNUSED(ctx);

    if (action == NULL || prev_state == NULL || next_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    prev = (const CounterState *)prev_state;
    next = (CounterState *)next_state;
    *next = *prev;

    switch (action->type) {
    case ACTION_ADD:
        if (action->data == NULL || action->size != sizeof(int)) {
            return M3_ERR_INVALID_ARGUMENT;
        }
        next->value += *(const int *)action->data;
        return M3_OK;
    case ACTION_SET:
        if (action->data == NULL || action->size != sizeof(int)) {
            return M3_ERR_INVALID_ARGUMENT;
        }
        next->value = *(const int *)action->data;
        return M3_OK;
    case ACTION_FAIL:
        return M3_ERR_UNKNOWN;
    default:
        return M3_ERR_UNSUPPORTED;
    }
}

static void init_alloc(TestAllocCtx *ctx, int fail_after)
{
    ctx->fail_after = fail_after;
    ctx->alloc_calls = 0;
    ctx->free_calls = 0;
}

int main(void)
{
    M3Store store;
    M3Store store_zero;
    M3Store temp_store;
    M3StoreConfig config;
    M3StoreConfig bad_config;
    M3Allocator bad_alloc;
    M3Allocator test_allocator;
    TestAllocCtx alloc_ctx;
    CounterState initial;
    CounterState state;
    CounterState snapshot;
    const void *state_ptr;
    m3_usize size;
    m3_usize count;
    M3Bool can_undo;
    M3Bool can_redo;
    M3Action action;
    int delta;
    m3_usize max_size;

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

    M3_TEST_EXPECT(m3_store_init(NULL, &config, &initial), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_init(&store, NULL, &initial), M3_ERR_INVALID_ARGUMENT);

    bad_config = config;
    bad_config.state_size = 0;
    bad_config.reducer = counter_reducer;
    M3_TEST_EXPECT(m3_store_init(&store, &bad_config, &initial), M3_ERR_INVALID_ARGUMENT);

    bad_config.state_size = sizeof(CounterState);
    bad_config.reducer = NULL;
    M3_TEST_EXPECT(m3_store_init(&store, &bad_config, &initial), M3_ERR_INVALID_ARGUMENT);

    config.allocator = &bad_alloc;
    config.state_size = sizeof(CounterState);
    config.history_capacity = 2;
    config.reducer = counter_reducer;
    config.reducer_ctx = NULL;
    M3_TEST_EXPECT(m3_store_init(&store, &config, &initial), M3_ERR_INVALID_ARGUMENT);

    max_size = (m3_usize)~(m3_usize)0;
    config.allocator = NULL;
    config.state_size = max_size;
    config.history_capacity = 2;
    config.reducer = counter_reducer;
    config.reducer_ctx = NULL;
    M3_TEST_EXPECT(m3_store_init(&store, &config, &initial), M3_ERR_OVERFLOW);

    config.state_size = sizeof(CounterState);

    init_alloc(&alloc_ctx, 0);
    test_allocator.ctx = &alloc_ctx;
    test_allocator.alloc = test_alloc;
    test_allocator.realloc = test_realloc;
    test_allocator.free = test_free;
    config.allocator = &test_allocator;
    config.history_capacity = 1;
    M3_TEST_EXPECT(m3_store_init(&temp_store, &config, &initial), M3_ERR_OUT_OF_MEMORY);

    init_alloc(&alloc_ctx, 1);
    M3_TEST_EXPECT(m3_store_init(&temp_store, &config, &initial), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(alloc_ctx.free_calls == 1);

    init_alloc(&alloc_ctx, 2);
    M3_TEST_EXPECT(m3_store_init(&temp_store, &config, &initial), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(alloc_ctx.free_calls == 2);

    init_alloc(&alloc_ctx, 3);
    M3_TEST_EXPECT(m3_store_init(&temp_store, &config, &initial), M3_ERR_OUT_OF_MEMORY);
    M3_TEST_ASSERT(alloc_ctx.free_calls == 3);

    config.allocator = NULL;
    config.history_capacity = 2;
    M3_TEST_OK(m3_store_init(&store, &config, &initial));

    M3_TEST_EXPECT(m3_store_get_state_ptr(NULL, &state_ptr, &size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_state_ptr(&store, NULL, &size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_state_ptr(&store, &state_ptr, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_store_get_state_ptr(&store, &state_ptr, &size));
    M3_TEST_ASSERT(size == sizeof(CounterState));
    M3_TEST_ASSERT(((const CounterState *)state_ptr)->value == 1);

    M3_TEST_EXPECT(m3_store_get_state(NULL, &state, sizeof(state)), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_state(&store, NULL, sizeof(state)), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_state(&store, &state, sizeof(state) - 1), M3_ERR_RANGE);
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 1);

    M3_TEST_EXPECT(m3_store_can_undo(NULL, &can_undo), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_can_undo(&store, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_can_redo(NULL, &can_redo), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_can_redo(&store, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_store_can_undo(&store, &can_undo));
    M3_TEST_OK(m3_store_can_redo(&store, &can_redo));
    M3_TEST_ASSERT(can_undo == M3_FALSE);
    M3_TEST_ASSERT(can_redo == M3_FALSE);

    action.type = ACTION_ADD;
    action.data = NULL;
    action.size = sizeof(int);
    M3_TEST_EXPECT(m3_store_dispatch(&store, &action), M3_ERR_INVALID_ARGUMENT);

    action.type = ACTION_UNSUPPORTED;
    action.data = NULL;
    action.size = 0;
    M3_TEST_EXPECT(m3_store_dispatch(&store, &action), M3_ERR_UNSUPPORTED);

    action.type = ACTION_FAIL;
    action.data = NULL;
    action.size = 0;
    M3_TEST_EXPECT(m3_store_dispatch(&store, &action), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 1);

    delta = 1;
    action.type = ACTION_ADD;
    action.data = &delta;
    action.size = sizeof(delta);
    M3_TEST_OK(m3_store_dispatch(&store, &action));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 2);

    M3_TEST_OK(m3_store_get_undo_count(&store, &count));
    M3_TEST_ASSERT(count == 1);
    M3_TEST_OK(m3_store_get_redo_count(&store, &count));
    M3_TEST_ASSERT(count == 0);

    M3_TEST_OK(m3_store_dispatch(&store, &action));
    M3_TEST_OK(m3_store_dispatch(&store, &action));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 4);

    M3_TEST_OK(m3_store_get_undo_count(&store, &count));
    M3_TEST_ASSERT(count == 2);

    M3_TEST_OK(m3_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot)));
    M3_TEST_ASSERT(snapshot.value == 3);
    M3_TEST_OK(m3_store_copy_undo_state(&store, 1, &snapshot, sizeof(snapshot)));
    M3_TEST_ASSERT(snapshot.value == 2);
    M3_TEST_EXPECT(m3_store_copy_undo_state(&store, 2, &snapshot, sizeof(snapshot)), M3_ERR_NOT_FOUND);

    M3_TEST_OK(m3_store_undo(&store));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 3);
    M3_TEST_OK(m3_store_undo(&store));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 2);
    M3_TEST_EXPECT(m3_store_undo(&store), M3_ERR_NOT_FOUND);

    M3_TEST_OK(m3_store_get_redo_count(&store, &count));
    M3_TEST_ASSERT(count == 2);
    M3_TEST_OK(m3_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot)));
    M3_TEST_ASSERT(snapshot.value == 3);
    M3_TEST_OK(m3_store_copy_redo_state(&store, 1, &snapshot, sizeof(snapshot)));
    M3_TEST_ASSERT(snapshot.value == 4);

    M3_TEST_OK(m3_store_redo(&store));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 3);
    M3_TEST_OK(m3_store_redo(&store));
    M3_TEST_OK(m3_store_get_state(&store, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 4);
    M3_TEST_EXPECT(m3_store_redo(&store), M3_ERR_NOT_FOUND);

    M3_TEST_OK(m3_store_clear_history(&store));
    M3_TEST_OK(m3_store_get_undo_count(&store, &count));
    M3_TEST_ASSERT(count == 0);
    M3_TEST_OK(m3_store_get_redo_count(&store, &count));
    M3_TEST_ASSERT(count == 0);

    M3_TEST_EXPECT(m3_store_copy_undo_state(&store, 0, &snapshot, sizeof(snapshot)), M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(m3_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot)), M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(m3_store_copy_redo_state(&store, 0, &snapshot, sizeof(snapshot) - 1), M3_ERR_RANGE);

    M3_TEST_EXPECT(m3_store_get_undo_count(NULL, &count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_undo_count(&store, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_redo_count(NULL, &count), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_redo_count(&store, NULL), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_store_shutdown(&store));
    M3_TEST_EXPECT(m3_store_shutdown(&store), M3_ERR_STATE);

    config.allocator = NULL;
    config.history_capacity = 0;
    M3_TEST_OK(m3_store_init(&store_zero, &config, NULL));
    action.type = ACTION_SET;
    delta = 5;
    action.data = &delta;
    action.size = sizeof(delta);
    M3_TEST_OK(m3_store_dispatch(&store_zero, &action));
    M3_TEST_OK(m3_store_get_state(&store_zero, &state, sizeof(state)));
    M3_TEST_ASSERT(state.value == 5);
    M3_TEST_EXPECT(m3_store_undo(&store_zero), M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(m3_store_redo(&store_zero), M3_ERR_NOT_FOUND);
    M3_TEST_OK(m3_store_shutdown(&store_zero));

    M3_TEST_EXPECT(m3_store_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_store_get_state_ptr(&store, &state_ptr, &size), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_store_clear_history(&store), M3_ERR_STATE);

    return 0;
}
