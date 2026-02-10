#include "m3/m3_core.h"
#include "m3/m3_tasks.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize free_calls;
  m3_usize realloc_calls;
  m3_usize fail_on;
  m3_usize free_fail_on;
} TestAllocator;

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
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

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
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

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->free_fail_on != 0 && alloc->free_calls == alloc->free_fail_on) {
    free(ptr);
    return M3_ERR_OUT_OF_MEMORY;
  }
  free(ptr);
  return M3_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->free_calls = 0;
  alloc->realloc_calls = 0;
  alloc->fail_on = 0;
  alloc->free_fail_on = 0;
}

typedef struct TaskState {
  M3Tasks tasks;
  M3Handle mutex;
  int counter;
  int fail;
} TaskState;

static int task_state_get(TaskState *state, int *out_value) {
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

static int task_increment(void *user) {
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

static int wait_for_counter(TaskState *state, int expected, m3_u32 timeout_ms) {
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

static int thread_entry(void *user) {
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

int main(void) {
  {
    M3TasksDefaultConfig config;

    M3_TEST_EXPECT(m3_tasks_default_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_tasks_default_config_init(&config));
    M3_TEST_ASSERT(config.worker_count > 0);
    M3_TEST_ASSERT(config.queue_capacity > 0);
    M3_TEST_ASSERT(config.handle_capacity > 0);
  }

  {
    M3_TEST_EXPECT(m3_tasks_default_create(NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
  }

#ifdef M3_TESTING
  {
    m3_usize out_value;

    M3_TEST_EXPECT(m3_tasks_test_mul_overflow(1, 1, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_tasks_test_mul_overflow(0, 16, &out_value));
    M3_TEST_ASSERT(out_value == 0);
    M3_TEST_EXPECT(
        m3_tasks_test_mul_overflow((m3_usize) ~(m3_usize)0, 2, &out_value),
        M3_ERR_OVERFLOW);

    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_TIME_NOW),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_MUTEX_INIT),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_MUTEX_DESTROY),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_MUTEX_LOCK),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_MUTEX_UNLOCK),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_INIT),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_DESTROY),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_SIGNAL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_BROADCAST),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_WAIT),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_COND_TIMEDWAIT),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_THREAD_CREATE),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_THREAD_JOIN),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_tasks_test_call_native(M3_TASKS_TEST_NATIVE_SLEEP));
    M3_TEST_EXPECT(m3_tasks_test_call_native(999u), M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3_TEST_EXPECT(
        m3_tasks_test_thread_destroy_case(M3_TASKS_TEST_THREAD_DESTROY_NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(
                       M3_TASKS_TEST_THREAD_DESTROY_NOT_JOINED),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(
                       M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(
                       M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_NULL_CTX),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(
                       M3_TASKS_TEST_THREAD_DESTROY_FREE_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(
                       M3_TASKS_TEST_THREAD_DESTROY_FREE_NULL_CTX),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(
        m3_tasks_test_thread_destroy_case(M3_TASKS_TEST_THREAD_DESTROY_OK));
    M3_TEST_EXPECT(m3_tasks_test_thread_destroy_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(
        m3_tasks_test_mutex_destroy_case(M3_TASKS_TEST_MUTEX_DESTROY_NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(
                       M3_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(
                       M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(
                       M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_NULL_CTX),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_destroy_case(M3_TASKS_TEST_MUTEX_DESTROY_FREE_FAIL),
        M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(
                       M3_TASKS_TEST_MUTEX_DESTROY_FREE_NULL_CTX),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(
        m3_tasks_test_mutex_destroy_case(M3_TASKS_TEST_MUTEX_DESTROY_OK));
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(m3_tasks_test_mutex_destroy_case(
                       M3_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_destroy_case(M3_TASKS_TEST_MUTEX_DESTROY_OK),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
  }

  {
    M3_TEST_OK(
        m3_tasks_test_thread_entry_case(M3_TASKS_TEST_THREAD_ENTRY_NULL));
    M3_TEST_OK(
        m3_tasks_test_thread_entry_case(M3_TASKS_TEST_THREAD_ENTRY_NO_ENTRY));
    M3_TEST_EXPECT(m3_tasks_test_thread_entry_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(
        m3_tasks_test_thread_create_case(M3_TASKS_TEST_THREAD_CREATE_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_thread_create_case(
                       M3_TASKS_TEST_THREAD_CREATE_ALLOC_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(m3_tasks_test_thread_create_case(
                       M3_TASKS_TEST_THREAD_CREATE_OBJECT_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_create_case(
                       M3_TASKS_TEST_THREAD_CREATE_NATIVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_create_case(
                       M3_TASKS_TEST_THREAD_CREATE_REGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_create_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_RESOLVE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(
                       M3_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(
                       M3_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(
                       M3_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(
                       M3_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(m3_tasks_test_thread_join_case(
                       M3_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(
        m3_tasks_test_thread_join_case(M3_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_EXPECT(
        m3_tasks_test_mutex_create_case(M3_TASKS_TEST_MUTEX_CREATE_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_create_case(M3_TASKS_TEST_MUTEX_CREATE_ALLOC_FAIL),
        M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_create_case(M3_TASKS_TEST_MUTEX_CREATE_OBJECT_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_create_case(M3_TASKS_TEST_MUTEX_CREATE_NATIVE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_mutex_create_case(
                       M3_TASKS_TEST_MUTEX_CREATE_REGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_mutex_create_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_STATE),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_RESOLVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TEST_MALLOC));
    M3_TEST_EXPECT(m3_tasks_test_default_mutex_destroy_case(
                       M3_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL),
                   M3_ERR_OUT_OF_MEMORY);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_EXPECT(
        m3_tasks_test_mutex_lock_case(M3_TASKS_TEST_MUTEX_LOCK_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_lock_case(M3_TASKS_TEST_MUTEX_LOCK_RESOLVE_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(
        m3_tasks_test_mutex_lock_case(M3_TASKS_TEST_MUTEX_LOCK_NOT_INITIALIZED),
        M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_mutex_lock_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(
        m3_tasks_test_mutex_unlock_case(M3_TASKS_TEST_MUTEX_UNLOCK_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_mutex_unlock_case(
                       M3_TASKS_TEST_MUTEX_UNLOCK_RESOLVE_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_mutex_unlock_case(
                       M3_TASKS_TEST_MUTEX_UNLOCK_NOT_INITIALIZED),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_mutex_unlock_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_tasks_test_sleep_case(M3_TASKS_TEST_SLEEP_STATE),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_sleep_case(999u), M3_ERR_INVALID_ARGUMENT);
  }

  {
    m3_u32 wait_ms;
    m3_u32 due_time;
    m3_usize count;

    M3_TEST_EXPECT(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_EMPTY,
                                            &wait_ms, &due_time, &count),
                   M3_ERR_NOT_FOUND);
    M3_TEST_EXPECT(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_INVALID,
                                            &wait_ms, &due_time, &count),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_NOT_READY,
                                            &wait_ms, &due_time, &count),
                   M3_ERR_NOT_READY);
    M3_TEST_OK(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_READY_EARLY,
                                        &wait_ms, &due_time, &count));
    M3_TEST_ASSERT(due_time == 0u);
    M3_TEST_OK(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_READY_LATE,
                                        &wait_ms, &due_time, &count));
    M3_TEST_OK(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_SWAP, &wait_ms,
                                        &due_time, &count));
    M3_TEST_ASSERT(count == 1u);
    M3_TEST_EXPECT(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_TIME_FAIL,
                                            &wait_ms, &due_time, &count),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_queue_case(M3_TASKS_TEST_QUEUE_CASE_BUSY,
                                            &wait_ms, &due_time, &count),
                   M3_ERR_BUSY);
    M3_TEST_EXPECT(m3_tasks_test_queue_case(999u, &wait_ms, &due_time, &count),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    m3_u32 type_id;

    M3_TEST_OK(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_RETAIN, &type_id));
    M3_TEST_OK(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_RELEASE, &type_id));
    M3_TEST_OK(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_GET_TYPE, &type_id));
    M3_TEST_ASSERT(type_id == 1u);
    M3_TEST_EXPECT(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_GET_TYPE, NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_RETAIN_NULL, &type_id),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_RELEASE_NULL, &type_id),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_tasks_test_object_op(M3_TASKS_TEST_OBJECT_OP_INIT_FAIL, &type_id),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_object_op(
                       M3_TASKS_TEST_OBJECT_OP_INIT_EARLY_FAIL, &type_id),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_test_object_op(999u, &type_id),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_stub_exercise());
    M3_TEST_OK(m3_tasks_test_call_noop());
  }

  {
    M3_TEST_OK(m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_LOCK_FAIL));
    M3_TEST_OK(m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_WAIT_FAIL));
    M3_TEST_OK(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_TIMEOUT));
    M3_TEST_OK(m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_TASK_ERROR));
    M3_TEST_OK(m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_PICK_ERROR));
    M3_TEST_OK(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_UNLOCK_FAIL));
    M3_TEST_OK(m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_NULL));
    M3_TEST_OK(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL));
    M3_TEST_OK(m3_tasks_test_signal_stop_null());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_JOIN));
    M3_TEST_EXPECT(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
    M3_TEST_OK(m3_tasks_test_worker_case(999u));
  }

  {
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        M3_ERR_TIMEOUT);
    M3_TEST_EXPECT(m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_ERROR),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_TIME_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_OK));
    M3_TEST_OK(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST));
    M3_TEST_OK(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_SIGNAL_NULL));
    M3_TEST_EXPECT(m3_tasks_test_timedwait_case(999u), M3_ERR_INVALID_ARGUMENT);
  }

  {
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_LOCK_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_worker_case(M3_TASKS_TEST_WORKER_CASE_LOCK_FAIL),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
  }

  {
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_LOCK));
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_OK),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_JOIN));
    M3_TEST_EXPECT(m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_OK),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(
        M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_INVALID));
    M3_TEST_EXPECT(
        m3_tasks_test_timedwait_case(M3_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
  }
#endif

  {
    M3Tasks tasks;
    M3TasksDefaultConfig config;

    memset(&tasks, 0, sizeof(tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_destroy_state(), M3_ERR_STATE);
    memset(&tasks, 0, sizeof(tasks));

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.worker_count = 0;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);
    config.worker_count = 1;
    config.queue_capacity = 0;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);
    config.queue_capacity = 4;
    config.handle_capacity = 0;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);
  }

  {
    TestAllocator alloc;
    M3Allocator alloc_iface;
    M3Tasks tasks;
    M3TasksDefaultConfig config;

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = NULL;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 2;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);

    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = NULL;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);

    alloc_iface.realloc = test_realloc;
    alloc_iface.free = NULL;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_INVALID_ARGUMENT);
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
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_OUT_OF_MEMORY);
  }

#ifdef M3_TESTING
  {
    M3Tasks tasks;
    M3TasksDefaultConfig config;

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 4;

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_INIT));
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_CREATE));
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
  }
#endif

#ifdef M3_TESTING
  {
    M3Tasks tasks;
    M3TasksDefaultConfig config;
    TestAllocator alloc;
    M3Allocator alloc_iface;
    m3_usize max_value;

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_CONFIG_INIT));
    M3_TEST_EXPECT(m3_tasks_default_create(NULL, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_tasks_default_create(NULL, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 1;
    config.handle_capacity = 1;

    test_allocator_init(&alloc);
    alloc.fail_on = 2;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.fail_on = 4;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.fail_on = 5;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks),
                   M3_ERR_OUT_OF_MEMORY);

    alloc.fail_on = 0;
    max_value = (m3_usize) ~(m3_usize)0;
    config.allocator = NULL;
    config.worker_count = 1;
    config.queue_capacity = max_value;
    config.handle_capacity = 1;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_OVERFLOW);

    config.worker_count = max_value;
    config.queue_capacity = 1;
    config.handle_capacity = 1;
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_OVERFLOW);

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.worker_count = 2;
    config.queue_capacity = 1;
    config.handle_capacity = 1;
    M3_TEST_OK(m3_tasks_test_set_thread_create_fail_after(2));
    M3_TEST_EXPECT(m3_tasks_default_create(&config, &tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_set_thread_create_fail_after(0));
  }
#endif

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
    M3_TEST_EXPECT(
        tasks.vtable->thread_create(NULL, thread_entry, NULL, &thread_handle),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        tasks.vtable->thread_create(tasks.ctx, NULL, NULL, &thread_handle),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->thread_join(NULL, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_create(NULL, &mutex),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_lock(NULL, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_unlock(NULL, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_destroy(NULL, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->sleep_ms(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->task_post(NULL, task_increment, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        tasks.vtable->task_post_delayed(NULL, task_increment, NULL, 1),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, NULL, NULL, 1),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_lock(tasks.ctx, bad_handle),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &mutex));
    M3_TEST_OK(tasks.vtable->mutex_lock(tasks.ctx, mutex));
    M3_TEST_OK(tasks.vtable->mutex_unlock(tasks.ctx, mutex));

    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_BUSY);
    M3_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, mutex));

    counter = 0;
    thread_ctx.counter = &counter;
    M3_TEST_OK(tasks.vtable->thread_create(tasks.ctx, thread_entry, &thread_ctx,
                                           &thread_handle));
    M3_TEST_OK(tasks.vtable->thread_join(tasks.ctx, thread_handle));
    M3_TEST_ASSERT(counter == 1);
    M3_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, thread_handle),
                   M3_ERR_NOT_FOUND);

    state.tasks = tasks;
    state.counter = 0;
    state.fail = 0;
    M3_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &state.mutex));

    M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    M3_TEST_OK(wait_for_counter(&state, 3, 500));

    state.counter = 0;
    M3_TEST_OK(tasks.vtable->task_post_delayed(tasks.ctx, task_increment,
                                               &state, 100));
    M3_TEST_OK(tasks.vtable->sleep_ms(tasks.ctx, 20));
    M3_TEST_OK(task_state_get(&state, &value));
    M3_TEST_ASSERT(value == 0);
    M3_TEST_OK(wait_for_counter(&state, 1, 500));

    rc = tasks.vtable->mutex_destroy(tasks.ctx, state.mutex);
    M3_TEST_OK(rc);

    M3_TEST_OK(m3_tasks_default_destroy(&tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_STATE);
  }

#ifdef M3_TESTING
  {
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_LOCK_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_SIGNAL_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_UNLOCK_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_BUSY),
                   M3_ERR_BUSY);
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_STATE),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_post_case(999u), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_BUSY),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_INIT));
    M3_TEST_EXPECT(m3_tasks_test_post_case(M3_TASKS_TEST_POST_CASE_BUSY),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(
                       M3_TASKS_TEST_POST_DELAYED_CASE_TIME_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(
                       M3_TASKS_TEST_POST_DELAYED_CASE_LOCK_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(
                       M3_TASKS_TEST_POST_DELAYED_CASE_SIGNAL_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(
                       M3_TASKS_TEST_POST_DELAYED_CASE_UNLOCK_FAIL),
                   M3_ERR_UNKNOWN);
    M3_TEST_EXPECT(
        m3_tasks_test_post_delayed_case(M3_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        M3_ERR_BUSY);
    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(
                       M3_TASKS_TEST_POST_DELAYED_CASE_OVERFLOW),
                   M3_ERR_OVERFLOW);
    M3_TEST_EXPECT(
        m3_tasks_test_post_delayed_case(M3_TASKS_TEST_POST_DELAYED_CASE_STATE),
        M3_ERR_STATE);
    M3_TEST_EXPECT(m3_tasks_test_post_delayed_case(999u),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_post_delayed_case(M3_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_INIT));
    M3_TEST_EXPECT(
        m3_tasks_test_post_delayed_case(M3_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    {
      M3Tasks tasks;
      M3TasksDefaultConfig config;

      M3_TEST_OK(m3_tasks_default_config_init(&config));
      config.worker_count = 1;
      config.queue_capacity = 1;
      config.handle_capacity = 1;
      M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
      M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_SLEEP));
      M3_TEST_EXPECT(tasks.vtable->sleep_ms(tasks.ctx, 1), M3_ERR_UNKNOWN);
      M3_TEST_OK(m3_tasks_default_destroy(&tasks));
    }
  }

  {
    M3Tasks tasks;
    M3TasksDefaultConfig config;

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 4;

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_LOCK));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_UNLOCK));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_tasks_test_clear_fail_point());

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_BROADCAST));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_THREAD_JOIN));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_DESTROY));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);

    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_OK(m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_DESTROY));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_UNKNOWN);
  }
#endif

#ifdef M3_TESTING
  {
    M3Tasks tasks;
    M3TasksDefaultConfig config;
    TestAllocator alloc;
    M3Allocator alloc_iface;

    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    M3_TEST_OK(m3_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 1;
    config.handle_capacity = 1;

    test_allocator_init(&alloc);
    alloc.free_fail_on = 1;
    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 2;
    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 3;
    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 4;
    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 5;
    M3_TEST_OK(m3_tasks_default_create(&config, &tasks));
    M3_TEST_EXPECT(m3_tasks_default_destroy(&tasks), M3_ERR_OUT_OF_MEMORY);
  }
#endif

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

    rc =
        tasks.vtable->task_post_delayed(tasks.ctx, task_increment, &state, 200);
    M3_TEST_OK(rc);
    M3_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, task_increment, &state),
                   M3_ERR_BUSY);

    M3_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, state.mutex));
    M3_TEST_OK(m3_tasks_default_destroy(&tasks));
  }

  return 0;
}
