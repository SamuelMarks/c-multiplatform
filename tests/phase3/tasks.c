#include "cmpc/cmp_core.h"
#include "cmpc/cmp_tasks.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  cmp_usize alloc_calls;
  cmp_usize free_calls;
  cmp_usize realloc_calls;
  cmp_usize fail_on;
  cmp_usize free_fail_on;
} TestAllocator;

static int test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_on != 0 && alloc->alloc_calls == alloc->fail_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int test_realloc(void *ctx, void *ptr, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  mem = realloc(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->free_fail_on != 0 && alloc->free_calls == alloc->free_fail_on) {
    free(ptr);
    return CMP_ERR_OUT_OF_MEMORY;
  }
  free(ptr);
  return CMP_OK;
}

static void test_allocator_init(TestAllocator *alloc) {
  alloc->alloc_calls = 0;
  alloc->free_calls = 0;
  alloc->realloc_calls = 0;
  alloc->fail_on = 0;
  alloc->free_fail_on = 0;
}

typedef struct TaskState {
  CMPTasks tasks;
  CMPHandle mutex;
  int counter;
  int fail;
} TaskState;

static int task_state_get(TaskState *state, int *out_value) {
  int rc;

  if (state == NULL || out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = state->tasks.vtable->mutex_lock(state->tasks.ctx, state->mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  *out_value = state->counter;
  rc = state->tasks.vtable->mutex_unlock(state->tasks.ctx, state->mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int task_increment(void *user) {
  TaskState *state;
  int rc;

  if (user == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TaskState *)user;
  rc = state->tasks.vtable->mutex_lock(state->tasks.ctx, state->mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  state->counter += 1;
  rc = state->tasks.vtable->mutex_unlock(state->tasks.ctx, state->mutex);
  if (rc != CMP_OK) {
    return rc;
  }

  if (state->fail) {
    return CMP_ERR_UNKNOWN;
  }

  return CMP_OK;
}

static int wait_for_counter(TaskState *state, int expected,
                            cmp_u32 timeout_ms) {
  cmp_u32 elapsed;
  int value;
  int rc;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  elapsed = 0;
  while (elapsed < timeout_ms) {
    rc = task_state_get(state, &value);
    if (rc != CMP_OK) {
      return rc;
    }
    if (value >= expected) {
      return CMP_OK;
    }
    rc = state->tasks.vtable->sleep_ms(state->tasks.ctx, 10);
    if (rc != CMP_OK) {
      return rc;
    }
    elapsed += 10;
  }

  return CMP_ERR_TIMEOUT;
}

typedef struct ThreadCtx {
  int *counter;
} ThreadCtx;

static int thread_entry(void *user) {
  ThreadCtx *ctx;

  if (user == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ctx = (ThreadCtx *)user;
  if (ctx->counter == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *ctx->counter += 1;
  return CMP_OK;
}

int main(void) {
  {
    CMPTasksDefaultConfig config;

    CMP_TEST_EXPECT(cmp_tasks_default_config_init(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    CMP_TEST_ASSERT(config.worker_count > 0);
    CMP_TEST_ASSERT(config.queue_capacity > 0);
    CMP_TEST_ASSERT(config.handle_capacity > 0);
  }

  {
    CMP_TEST_EXPECT(cmp_tasks_default_create(NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
  }

#ifdef CMP_TESTING
  {
    cmp_usize out_value;

    CMP_TEST_EXPECT(cmp_tasks_test_mul_overflow(1, 1, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_tasks_test_mul_overflow(0, 16, &out_value));
    CMP_TEST_ASSERT(out_value == 0);
    CMP_TEST_EXPECT(
        cmp_tasks_test_mul_overflow((cmp_usize) ~(cmp_usize)0, 2, &out_value),
        CMP_ERR_OVERFLOW);

    CMP_TEST_EXPECT(cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_TIME_NOW),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_MUTEX_INIT),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_MUTEX_DESTROY),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_MUTEX_LOCK),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_MUTEX_UNLOCK),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_INIT),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_DESTROY),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_SIGNAL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_BROADCAST),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_WAIT),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_COND_TIMEDWAIT),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_THREAD_CREATE),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_THREAD_JOIN),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_tasks_test_call_native(CMP_TASKS_TEST_NATIVE_SLEEP));
    CMP_TEST_EXPECT(cmp_tasks_test_call_native(999u), CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_destroy_case(CMP_TASKS_TEST_THREAD_DESTROY_NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(
                        CMP_TASKS_TEST_THREAD_DESTROY_NOT_JOINED),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(
                        CMP_TASKS_TEST_THREAD_DESTROY_UNREGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(
                        CMP_TASKS_TEST_THREAD_DESTROY_UNREGISTER_NULL_CTX),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(
                        CMP_TASKS_TEST_THREAD_DESTROY_FREE_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(
                        CMP_TASKS_TEST_THREAD_DESTROY_FREE_NULL_CTX),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        cmp_tasks_test_thread_destroy_case(CMP_TASKS_TEST_THREAD_DESTROY_OK));
    CMP_TEST_EXPECT(cmp_tasks_test_thread_destroy_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_destroy_case(CMP_TASKS_TEST_MUTEX_DESTROY_NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_NULL_CTX),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_FREE_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_FREE_NULL_CTX),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        cmp_tasks_test_mutex_destroy_case(CMP_TASKS_TEST_MUTEX_DESTROY_OK));
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_destroy_case(
                        CMP_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_destroy_case(CMP_TASKS_TEST_MUTEX_DESTROY_OK),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
  }

  {
    CMP_TEST_OK(
        cmp_tasks_test_thread_entry_case(CMP_TASKS_TEST_THREAD_ENTRY_NULL));
    CMP_TEST_OK(
        cmp_tasks_test_thread_entry_case(CMP_TASKS_TEST_THREAD_ENTRY_NO_ENTRY));
    CMP_TEST_EXPECT(cmp_tasks_test_thread_entry_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_create_case(CMP_TASKS_TEST_THREAD_CREATE_STATE),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_create_case(
                        CMP_TASKS_TEST_THREAD_CREATE_ALLOC_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_create_case(
                        CMP_TASKS_TEST_THREAD_CREATE_OBJECT_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_create_case(
                        CMP_TASKS_TEST_THREAD_CREATE_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_create_case(
                        CMP_TASKS_TEST_THREAD_CREATE_REGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_create_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_STATE),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_RESOLVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(cmp_tasks_test_thread_join_case(
                        CMP_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(
        cmp_tasks_test_thread_join_case(CMP_TASKS_TEST_THREAD_JOIN_FREE_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_create_case(CMP_TASKS_TEST_MUTEX_CREATE_STATE),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_create_case(
                        CMP_TASKS_TEST_MUTEX_CREATE_ALLOC_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_create_case(
                        CMP_TASKS_TEST_MUTEX_CREATE_OBJECT_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_create_case(
                        CMP_TASKS_TEST_MUTEX_CREATE_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_create_case(
                        CMP_TASKS_TEST_MUTEX_CREATE_REGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_create_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_STATE),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_RESOLVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TEST_MALLOC));
    CMP_TEST_EXPECT(cmp_tasks_test_default_mutex_destroy_case(
                        CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL),
                    CMP_ERR_OUT_OF_MEMORY);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_lock_case(CMP_TASKS_TEST_MUTEX_LOCK_STATE),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_lock_case(CMP_TASKS_TEST_MUTEX_LOCK_RESOLVE_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_lock_case(
                        CMP_TASKS_TEST_MUTEX_LOCK_NOT_INITIALIZED),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_lock_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(
        cmp_tasks_test_mutex_unlock_case(CMP_TASKS_TEST_MUTEX_UNLOCK_STATE),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_unlock_case(
                        CMP_TASKS_TEST_MUTEX_UNLOCK_RESOLVE_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_unlock_case(
                        CMP_TASKS_TEST_MUTEX_UNLOCK_NOT_INITIALIZED),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_mutex_unlock_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_tasks_test_sleep_case(CMP_TASKS_TEST_SLEEP_STATE),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_sleep_case(999u), CMP_ERR_INVALID_ARGUMENT);
  }

  {
    cmp_u32 wait_ms;
    cmp_u32 due_time;
    cmp_usize count;

    CMP_TEST_EXPECT(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_EMPTY,
                                              &wait_ms, &due_time, &count),
                    CMP_ERR_NOT_FOUND);
    CMP_TEST_EXPECT(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_INVALID,
                                              &wait_ms, &due_time, &count),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_NOT_READY, &wait_ms,
                                  &due_time, &count),
        CMP_ERR_NOT_READY);
    CMP_TEST_OK(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_READY_EARLY,
                                          &wait_ms, &due_time, &count));
    CMP_TEST_ASSERT(due_time == 0u);
    CMP_TEST_OK(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_READY_LATE,
                                          &wait_ms, &due_time, &count));
    CMP_TEST_OK(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_SWAP,
                                          &wait_ms, &due_time, &count));
    CMP_TEST_ASSERT(count == 1u);
    CMP_TEST_EXPECT(
        cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_TIME_FAIL, &wait_ms,
                                  &due_time, &count),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_queue_case(CMP_TASKS_TEST_QUEUE_CASE_BUSY,
                                              &wait_ms, &due_time, &count),
                    CMP_ERR_BUSY);
    CMP_TEST_EXPECT(
        cmp_tasks_test_queue_case(999u, &wait_ms, &due_time, &count),
        CMP_ERR_INVALID_ARGUMENT);
  }

  {
    cmp_u32 type_id;

    CMP_TEST_OK(
        cmp_tasks_test_object_op(CMP_TASKS_TEST_OBJECT_OP_RETAIN, &type_id));
    CMP_TEST_OK(
        cmp_tasks_test_object_op(CMP_TASKS_TEST_OBJECT_OP_RELEASE, &type_id));
    CMP_TEST_OK(
        cmp_tasks_test_object_op(CMP_TASKS_TEST_OBJECT_OP_GET_TYPE, &type_id));
    CMP_TEST_ASSERT(type_id == 1u);
    CMP_TEST_EXPECT(
        cmp_tasks_test_object_op(CMP_TASKS_TEST_OBJECT_OP_GET_TYPE, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_object_op(
                        CMP_TASKS_TEST_OBJECT_OP_RETAIN_NULL, &type_id),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_object_op(
                        CMP_TASKS_TEST_OBJECT_OP_RELEASE_NULL, &type_id),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_object_op(CMP_TASKS_TEST_OBJECT_OP_INIT_FAIL, &type_id),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_object_op(
                        CMP_TASKS_TEST_OBJECT_OP_INIT_EARLY_FAIL, &type_id),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_test_object_op(999u, &type_id),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_stub_exercise());
    CMP_TEST_OK(cmp_tasks_test_call_noop());
  }

  {
    CMP_TEST_OK(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_LOCK_FAIL));
    CMP_TEST_OK(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_WAIT_FAIL));
    CMP_TEST_OK(cmp_tasks_test_worker_case(
        CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_TIMEOUT));
    CMP_TEST_OK(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_TASK_ERROR));
    CMP_TEST_OK(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_PICK_ERROR));
    CMP_TEST_OK(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_UNLOCK_FAIL));
    CMP_TEST_OK(cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_NULL));
    CMP_TEST_OK(cmp_tasks_test_worker_case(
        CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL));
    CMP_TEST_OK(cmp_tasks_test_signal_stop_null());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_JOIN));
    CMP_TEST_EXPECT(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
    CMP_TEST_OK(cmp_tasks_test_worker_case(999u));
  }

  {
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        CMP_ERR_TIMEOUT);
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_ERROR),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_TIME_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_OK));
    CMP_TEST_OK(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST));
    CMP_TEST_OK(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_SIGNAL_NULL));
    CMP_TEST_EXPECT(cmp_tasks_test_timedwait_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_LOCK_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_worker_case(CMP_TASKS_TEST_WORKER_CASE_LOCK_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
  }

  {
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_LOCK));
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_TIMEOUT),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_OK),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_JOIN));
    CMP_TEST_EXPECT(cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_OK),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(
        CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_INVALID));
    CMP_TEST_EXPECT(
        cmp_tasks_test_timedwait_case(CMP_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
  }
#endif

  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;

    memset(&tasks, 0, sizeof(tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_destroy_state(), CMP_ERR_STATE);
    memset(&tasks, 0, sizeof(tasks));

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 0;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);
    config.worker_count = 1;
    config.queue_capacity = 0;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);
    config.queue_capacity = 4;
    config.handle_capacity = 0;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestAllocator alloc;
    CMPAllocator alloc_iface;
    CMPTasks tasks;
    CMPTasksDefaultConfig config;

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = NULL;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 2;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);

    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = NULL;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);

    alloc_iface.realloc = test_realloc;
    alloc_iface.free = NULL;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    TestAllocator alloc;
    CMPAllocator alloc_iface;
    CMPTasks tasks;
    CMPTasksDefaultConfig config;

    test_allocator_init(&alloc);
    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 2;

    alloc.fail_on = 1;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OUT_OF_MEMORY);
  }

#ifdef CMP_TESTING
  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 4;

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_INIT));
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE));
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
  }
#endif

#ifdef CMP_TESTING
  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;
    TestAllocator alloc;
    CMPAllocator alloc_iface;
    cmp_usize max_value;

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_CONFIG_INIT));
    CMP_TEST_EXPECT(cmp_tasks_default_create(NULL, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_tasks_default_create(NULL, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 1;
    config.handle_capacity = 1;

    test_allocator_init(&alloc);
    alloc.fail_on = 2;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.fail_on = 4;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.fail_on = 5;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OUT_OF_MEMORY);

    alloc.fail_on = 0;
    max_value = (cmp_usize) ~(cmp_usize)0;
    config.allocator = NULL;
    config.worker_count = 1;
    config.queue_capacity = max_value;
    config.handle_capacity = 1;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OVERFLOW);

    config.worker_count = max_value;
    config.queue_capacity = 1;
    config.handle_capacity = 1;
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks),
                    CMP_ERR_OVERFLOW);

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 2;
    config.queue_capacity = 1;
    config.handle_capacity = 1;
    CMP_TEST_OK(cmp_tasks_test_set_thread_create_fail_after(2));
    CMP_TEST_EXPECT(cmp_tasks_default_create(&config, &tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_set_thread_create_fail_after(0));
  }
#endif

  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;
    CMPHandle mutex;
    CMPHandle thread_handle;
    CMPHandle bad_handle;
    TaskState state;
    ThreadCtx thread_ctx;
    int counter;
    int value;
    int rc;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 4;
    config.handle_capacity = 8;

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));

    bad_handle.id = 0;
    bad_handle.generation = 0;
    CMP_TEST_EXPECT(
        tasks.vtable->thread_create(NULL, thread_entry, NULL, &thread_handle),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        tasks.vtable->thread_create(tasks.ctx, NULL, NULL, &thread_handle),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->thread_join(NULL, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_create(NULL, &mutex),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_lock(NULL, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_unlock(NULL, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_destroy(NULL, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->sleep_ms(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->task_post(NULL, task_increment, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        tasks.vtable->task_post_delayed(NULL, task_increment, NULL, 1),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, NULL, NULL, 1),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_lock(tasks.ctx, bad_handle),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &mutex));
    CMP_TEST_OK(tasks.vtable->mutex_lock(tasks.ctx, mutex));
    CMP_TEST_OK(tasks.vtable->mutex_unlock(tasks.ctx, mutex));

    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_BUSY);
    CMP_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, mutex));

    counter = 0;
    thread_ctx.counter = &counter;
    CMP_TEST_OK(tasks.vtable->thread_create(tasks.ctx, thread_entry,
                                            &thread_ctx, &thread_handle));
    CMP_TEST_OK(tasks.vtable->thread_join(tasks.ctx, thread_handle));
    CMP_TEST_ASSERT(counter == 1);
    CMP_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, thread_handle),
                    CMP_ERR_NOT_FOUND);

    state.tasks = tasks;
    state.counter = 0;
    state.fail = 0;
    CMP_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &state.mutex));

    CMP_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    CMP_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    CMP_TEST_OK(tasks.vtable->task_post(tasks.ctx, task_increment, &state));
    CMP_TEST_OK(wait_for_counter(&state, 3, 500));

    state.counter = 0;
    CMP_TEST_OK(tasks.vtable->task_post_delayed(tasks.ctx, task_increment,
                                                &state, 100));
    CMP_TEST_OK(tasks.vtable->sleep_ms(tasks.ctx, 20));
    CMP_TEST_OK(task_state_get(&state, &value));
    CMP_TEST_ASSERT(value == 0);
    CMP_TEST_OK(wait_for_counter(&state, 1, 500));

    rc = tasks.vtable->mutex_destroy(tasks.ctx, state.mutex);
    CMP_TEST_OK(rc);

    CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_STATE);
  }

#ifdef CMP_TESTING
  {
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_LOCK_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_SIGNAL_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_UNLOCK_FAIL),
        CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_BUSY),
                    CMP_ERR_BUSY);
    CMP_TEST_EXPECT(cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_STATE),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_post_case(999u), CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_BUSY),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_INIT));
    CMP_TEST_EXPECT(cmp_tasks_test_post_case(CMP_TASKS_TEST_POST_CASE_BUSY),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_TIME_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_LOCK_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_SIGNAL_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_UNLOCK_FAIL),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_delayed_case(CMP_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        CMP_ERR_BUSY);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_OVERFLOW),
                    CMP_ERR_OVERFLOW);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(
                        CMP_TASKS_TEST_POST_DELAYED_CASE_STATE),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_tasks_test_post_delayed_case(999u),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_delayed_case(CMP_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_INIT));
    CMP_TEST_EXPECT(
        cmp_tasks_test_post_delayed_case(CMP_TASKS_TEST_POST_DELAYED_CASE_BUSY),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    {
      CMPTasks tasks;
      CMPTasksDefaultConfig config;

      CMP_TEST_OK(cmp_tasks_default_config_init(&config));
      config.worker_count = 1;
      config.queue_capacity = 1;
      config.handle_capacity = 1;
      CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
      CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_SLEEP));
      CMP_TEST_EXPECT(tasks.vtable->sleep_ms(tasks.ctx, 1), CMP_ERR_UNKNOWN);
      CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));
    }
  }

  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 2;
    config.handle_capacity = 4;

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_LOCK));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_tasks_test_clear_fail_point());

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_BROADCAST));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_JOIN));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_DESTROY));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_OK(
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_DESTROY));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_UNKNOWN);
  }
#endif

#ifdef CMP_TESTING
  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;
    TestAllocator alloc;
    CMPAllocator alloc_iface;

    alloc_iface.ctx = &alloc;
    alloc_iface.alloc = test_alloc;
    alloc_iface.realloc = test_realloc;
    alloc_iface.free = test_free;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.allocator = &alloc_iface;
    config.worker_count = 1;
    config.queue_capacity = 1;
    config.handle_capacity = 1;

    test_allocator_init(&alloc);
    alloc.free_fail_on = 1;
    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 2;
    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 3;
    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 4;
    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_OUT_OF_MEMORY);

    test_allocator_init(&alloc);
    alloc.free_fail_on = 5;
    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));
    CMP_TEST_EXPECT(cmp_tasks_default_destroy(&tasks), CMP_ERR_OUT_OF_MEMORY);
  }
#endif

  {
    CMPTasks tasks;
    CMPTasksDefaultConfig config;
    TaskState state;
    int rc;

    CMP_TEST_OK(cmp_tasks_default_config_init(&config));
    config.worker_count = 1;
    config.queue_capacity = 1;
    config.handle_capacity = 4;

    CMP_TEST_OK(cmp_tasks_default_create(&config, &tasks));

    state.tasks = tasks;
    state.counter = 0;
    state.fail = 0;
    CMP_TEST_OK(tasks.vtable->mutex_create(tasks.ctx, &state.mutex));

    rc =
        tasks.vtable->task_post_delayed(tasks.ctx, task_increment, &state, 200);
    CMP_TEST_OK(rc);
    CMP_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, task_increment, &state),
                    CMP_ERR_BUSY);

    CMP_TEST_OK(tasks.vtable->mutex_destroy(tasks.ctx, state.mutex));
    CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));
  }

  return 0;
}
