#include "test_utils.h"
#include "m3/m3_tasks.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
    m3_usize alloc_calls;
    m3_usize free_calls;
    m3_usize realloc_calls;
    m3_usize fail_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr)
{
    TestAllocator *alloc;

    if (ctx == NULL || out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAllocator *)ctx;
    alloc->alloc_calls += 1;
    if (alloc->fail_on != 0 && alloc->alloc_calls == alloc->fail_on) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    *out_ptr = malloc(size);
    if (*out_ptr == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr)
{
    TestAllocator *alloc;
    void *mem;

    if (ctx == NULL || out_ptr == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAllocator *)ctx;
    alloc->realloc_calls += 1;
    mem = realloc(ptr, size);
    if (mem == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }
    *out_ptr = mem;
    return M3_OK;
}

static int test_free(void *ctx, void *ptr)
{
    TestAllocator *alloc;

    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    alloc = (TestAllocator *)ctx;
    alloc->free_calls += 1;
    free(ptr);
    return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc)
{
    alloc->alloc_calls = 0;
    alloc->free_calls = 0;
    alloc->realloc_calls = 0;
    alloc->fail_on = 0;
}

typedef struct TaskState {
    M3Tasks tasks;
    M3Handle mutex;
    int counter;
    int fail;
} TaskState;

static int task_state_get(TaskState *state, int *out_value)
{
    int rc;

    if (state == NULL || out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = state->tasks.vtable->mutex_lock(state->tasks.ctx, state->mutex);
    if (rc != M3_OK) {
        return rc;
    }
    *out_value = state->counter;
    rc = state->tasks.vtable->mutex_unlock(state->tasks.ctx, state->mutex);
    if (rc != M3_OK) {
        return rc;
    }
    return M3_OK;
}

static int task_increment(void *user)
{
    TaskState *state;
    int rc;

    if (user == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TaskState *)user;
    rc = state->tasks.vtable->mutex_lock(state->tasks.ctx, state->mutex);
    if (rc != M3_OK) {
        return rc;
    }
    state->counter += 1;
    rc = state->tasks.vtable->mutex_unlock(state->tasks.ctx, state->mutex);
    if (rc != M3_OK) {
        return rc;
    }

    if (state->fail) {
        return M3_ERR_UNKNOWN;
    }

    return M3_OK;
}

static int wait_for_counter(TaskState *state, int expected, m3_u32 timeout_ms)
{
    m3_u32 elapsed;
    int value;
    int rc;

    if (state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    elapsed = 0;
    while (elapsed < timeout_ms) {
        rc = task_state_get(state, &value);
        if (rc != M3_OK) {
            return rc;
        }
        if (value >= expected) {
            return M3_OK;
        }
        rc = state->tasks.vtable->sleep_ms(state->tasks.ctx, 10);
        if (rc != M3_OK) {
            return rc;
        }
        elapsed += 10;
    }

    return M3_ERR_TIMEOUT;
}

typedef struct ThreadCtx {
    int *counter;
} ThreadCtx;

static int thread_entry(void *user)
{
    ThreadCtx *ctx;

    if (user == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    ctx = (ThreadCtx *)user;
    if (ctx->counter == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *ctx->counter += 1;
    return M3_OK;
}

int main(void)
{
    {
        M3TasksDefaultConfig config;

        M3_TEST_EXPECT(m3_tasks_default_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_OK(m3_tasks_default_config_init(&config));
        M3_TEST_ASSERT(config.worker_count > 0);
        M3_TEST_ASSERT(config.queue_capacity > 0);
        M3_TEST_ASSERT(config.handle_capacity > 0);
    }

    {
        M3Tasks tasks;
        M3TasksDefaultConfig config;

        memset(&tasks, 0, sizeof(tasks));
        M3_TEST_EXPECT(m3_tasks_default_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_STATE);

        M3_TEST_OK(m3_tasks_default_config_init(&config));
        config.worker_count = 0;
        M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_INVALID_ARGUMENT);
        config.worker_count = 1;
        config.queue_capacity = 0;
        M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_INVALID_ARGUMENT);
        config.queue_capacity = 4;
        config.handle_capacity = 0;
        M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_INVALID_ARGUMENT);
    }

    {
        TestAllocator alloc;
        M3Allocator alloc_iface;
        M3Tasks tasks;
        M3TasksDefaultConfig config;

        test_allocator_init(&alloc);
        alloc_iface.ctx = &alloc;
        alloc_iface.alloc = test_alloc;
        alloc_iface.realloc = test_realloc;
        alloc_iface.free = test_free;

        M3_TEST_OK(m3_tasks_default_config_init(&config));
        config.allocator = &alloc_iface;
        config.worker_count = 1;
        config.queue_capacity = 2;
        config.handle_capacity = 2;

        alloc.fail_on = 1;
        M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_OUT_OF_MEMORY);
    }

    {
        M3Tasks tasks;
        M3TasksDefaultConfig config;
        M3Handle mutex;
        M3Handle thread_handle;
        M3Handle bad_handle;
        TaskState state;
        ThreadCtx thread_ctx;
        int counter;
        int value;
        int rc;

        M3_TEST_OK(m3_tasks_default_config_init(&config));
        config.worker_count = 1;
        config.queue_capacity = 4;
        config.handle_capacity = 8;

        M3_TEST_OK(m3_tasks_default_create(&config, &tasks));

        bad_handle.id = 0;
        bad_handle.generation = 0;
        M3_TEST_EXPECT(tasks.vtable->thread_create(NULL, thread_entry, NULL, &thread_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->thread_create(tasks.ctx, NULL, NULL, &thread_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->thread_join(NULL, bad_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->mutex_create(NULL, &mutex), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->mutex_lock(NULL, bad_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->mutex_unlock(NULL, bad_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->sleep_ms(NULL, 1), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->task_post(NULL, task_increment, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, NULL, NULL), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->task_post_delayed(NULL, task_increment, NULL, 1), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, NULL, NULL, 1), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, bad_handle), M3_ERR_INVALID_ARGUMENT);
        M3_TEST_EXPECT(tasks.vtable->mutex_lock(tasks.ctx, bad_handle), M3_ERR_INVALID_ARGUMENT);

        M3_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &mutex));
        M3_TEST_OK(tasks.vtable->mutex_lock(tasks.ctx, mutex));
        M3_TEST_OK(tasks.vtable->mutex_unlock(tasks.ctx, mutex));

        M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_BUSY);
        M3_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, mutex));

        counter = 0;
        thread_ctx.counter = &counter;
        M3_TEST_OK(tasks.vtable->thread_create(tasks.ctx, thread_entry, &thread_ctx, &thread_handle));
        M3_TEST_OK(tasks.vtable->thread_join(tasks.ctx, thread_handle));
        M3_TEST_ASSERT(counter == 1);
        M3_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, thread_handle), M3_ERR_NOT_FOUND);

        state.tasks = tasks;
        state.counter = 0;
        state.fail = 0;
        M3_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &state.mutex));

        M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
        M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
        M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
        M3_TEST_OK(wait_for_counter(&state, 3, 500));

        state.counter = 0;
        M3_TEST_OK(tasks.vtable->task_post_delayed(tasks.ctx, task_increment, &state, 100));
        M3_TEST_OK(tasks.vtable->sleep_ms(tasks.ctx, 20));
        M3_TEST_OK(task_state_get(&state, &value));
        M3_TEST_ASSERT(value == 0);
        M3_TEST_OK(wait_for_counter(&state, 1, 500));

        rc = tasks.vtable->mutex_destroy(tasks.ctx, state.mutex);
        M3_TEST_OK(rc);

        M3_TEST_OK(m3_tasks_default_destroy(&tasks));
        M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_STATE);
    }

    {
        M3Tasks tasks;
        M3TasksDefaultConfig config;
        TaskState state;
        int rc;

        M3_TEST_OK(m3_tasks_default_config_init(&config));
        config.worker_count = 1;
        config.queue_capacity = 1;
        config.handle_capacity = 4;

        M3_TEST_OK(m3_tasks_default_create(&config, &tasks));

        state.tasks = tasks;
        state.counter = 0;
        state.fail = 0;
        M3_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &state.mutex));

        rc = tasks.vtable->task_post_delayed(tasks.ctx, task_increment, &state, 200);
        M3_TEST_OK(rc);
        M3_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, task_increment, &state), M3_ERR_BUSY);

        M3_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, state.mutex));
        M3_TEST_OK(m3_tasks_default_destroy(&tasks));
    }

    return 0;
}
