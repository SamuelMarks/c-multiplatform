#if !defined(_WIN32) && !defined(_WIN64)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* GCOVR_EXCL_LINE */
#endif /* GCOVR_EXCL_LINE */

#include "cmpc/cmp_tasks.h"

#include "cmpc/cmp_object.h"

#include <stdlib.h> /* GCOVR_EXCL_LINE */
#include <string.h>

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
static cmp_u32 g_cmp_tasks_test_fail_point =
    CMP_TASKS_TEST_FAIL_NONE;                  /* GCOVR_EXCL_LINE */
static int g_cmp_tasks_test_nsec_adjusted = 0; /* GCOVR_EXCL_LINE */
static cmp_u32 g_cmp_tasks_test_thread_create_fail_after =
    0u;                                                  /* GCOVR_EXCL_LINE */
static cmp_u32 g_cmp_tasks_test_thread_create_count = 0; /* GCOVR_EXCL_LINE */

static int cmp_tasks_test_consume_fail(cmp_u32 point) {
  if (g_cmp_tasks_test_fail_point == point) {
    g_cmp_tasks_test_fail_point = CMP_TASKS_TEST_FAIL_NONE;
    return 1;
  }
  return 0;
}

int CMP_CALL cmp_tasks_test_set_fail_point(cmp_u32 point) {
  g_cmp_tasks_test_fail_point = point;
  return CMP_OK;
}

int CMP_CALL cmp_tasks_test_clear_fail_point(void) {
  g_cmp_tasks_test_fail_point = CMP_TASKS_TEST_FAIL_NONE;
  return CMP_OK;
}

int CMP_CALL cmp_tasks_test_set_thread_create_fail_after(cmp_u32 count) {
  g_cmp_tasks_test_thread_create_fail_after = count;
  g_cmp_tasks_test_thread_create_count = 0;
  return CMP_OK;
}

typedef struct CMPTasksTestStub {
  int rc;
  int rc_alloc;
  int rc_register;
  int rc_resolve;
  int rc_unregister;
  int rc_free;
  void *ptr;
} CMPTasksTestStub;

static void *cmp_tasks_test_malloc(cmp_usize size) {
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_TEST_MALLOC)) {
    return NULL;
  }
  return malloc(size);
}

static int CMP_CALL cmp_tasks_test_unregister(void *ctx, CMPHandle handle) {
  CMPTasksTestStub *stub;

  (void)handle;
  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  return stub->rc;
}

static int CMP_CALL cmp_tasks_test_free(void *ctx, void *ptr) {
  CMPTasksTestStub *stub;

  (void)ptr;
  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  return stub->rc;
}

static int CMP_CALL cmp_tasks_test_register_ex(void *ctx,
                                               CMPObjectHeader *obj) {
  CMPTasksTestStub *stub;

  if (ctx == NULL || obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (stub->rc_register != CMP_OK) {
    return stub->rc_register;
  }
  obj->handle.id = 1u;
  obj->handle.generation = 1u;
  stub->ptr = obj;
  return CMP_OK;
}

static int CMP_CALL cmp_tasks_test_resolve_ex(void *ctx, CMPHandle handle,
                                              void **out_obj) {
  CMPTasksTestStub *stub;

  (void)handle;
  if (ctx == NULL || out_obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (stub->rc_resolve != CMP_OK) {
    return stub->rc_resolve;
  }
  if (stub->ptr == NULL) {
    return CMP_ERR_NOT_FOUND;
  }
  *out_obj = stub->ptr;
  return CMP_OK;
}

static int CMP_CALL cmp_tasks_test_unregister_ex(void *ctx, CMPHandle handle) {
  CMPTasksTestStub *stub;

  (void)handle; /* GCOVR_EXCL_LINE */
  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (stub->rc_unregister != CMP_OK) {
    return stub->rc_unregister;
  }
  return CMP_OK;
}

static int CMP_CALL cmp_tasks_test_alloc_ex(
    void *ctx, cmp_usize size, void **out_ptr) { /* GCOVR_EXCL_LINE */
  CMPTasksTestStub *stub;                        /* GCOVR_EXCL_LINE */

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (stub->rc_alloc != CMP_OK) {
    return stub->rc_alloc;
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_ALLOC_EX_NULL)) {
    *out_ptr = NULL;
  } else {
    *out_ptr = malloc(size);
  }
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int CMP_CALL
cmp_tasks_test_realloc_ex(void *ctx, void *ptr, cmp_usize size,
                          void **out_ptr) { /* GCOVR_EXCL_LINE */
  CMPTasksTestStub *stub = NULL;
  void *mem = NULL; /* GCOVR_EXCL_LINE */

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (stub->rc_alloc != CMP_OK) {
    return stub->rc_alloc;
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_REALLOC_EX_NULL)) {
    mem = NULL;
  } else {
    mem = realloc(ptr, size);
  }
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  *out_ptr = mem;
  return CMP_OK;
}

static int CMP_CALL cmp_tasks_test_free_ex(void *ctx, void *ptr) {
  CMPTasksTestStub *stub;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  stub = (CMPTasksTestStub *)ctx;
  if (ptr != NULL) {
    free(ptr);
  }
  if (stub->rc_free != CMP_OK) {
    return stub->rc_free;
  }
  return CMP_OK;
}
#endif

#if defined(_WIN32) || defined(_WIN64) /* GCOVR_EXCL_LINE */
#define CMP_TASKS_USE_WIN32 1          /* GCOVR_EXCL_LINE */
#include <windows.h>                   /* GCOVR_EXCL_LINE */
#else                                  /* GCOVR_EXCL_LINE */
#define CMP_TASKS_USE_PTHREAD 1        /* GCOVR_EXCL_LINE */
#include <errno.h>                     /* GCOVR_EXCL_LINE */
#include <pthread.h>                   /* GCOVR_EXCL_LINE */
#include <sys/time.h>                  /* GCOVR_EXCL_LINE */
#include <time.h>                      /* GCOVR_EXCL_LINE */
#include <unistd.h>                    /* GCOVR_EXCL_LINE */
#endif                                 /* GCOVR_EXCL_LINE */

#define CMP_TASKS_DEFAULT_WORKERS 1          /* GCOVR_EXCL_LINE */
#define CMP_TASKS_DEFAULT_QUEUE_CAPACITY 64  /* GCOVR_EXCL_LINE */
#define CMP_TASKS_DEFAULT_HANDLE_CAPACITY 64 /* GCOVR_EXCL_LINE */

#define CMP_TASK_OBJECT_THREAD 1 /* GCOVR_EXCL_LINE */
#define CMP_TASK_OBJECT_MUTEX 2  /* GCOVR_EXCL_LINE */

#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
typedef struct CMPNativeMutex {  /* GCOVR_EXCL_LINE */
  CRITICAL_SECTION cs;
} CMPNativeMutex; /* GCOVR_EXCL_LINE */

typedef struct CMPNativeCond { /* GCOVR_EXCL_LINE */
  CONDITION_VARIABLE cond;
} CMPNativeCond; /* GCOVR_EXCL_LINE */

typedef struct CMPNativeThread {
  HANDLE handle; /* GCOVR_EXCL_LINE */
  DWORD id;      /* GCOVR_EXCL_LINE */
} CMPNativeThread;
#else /* GCOVR_EXCL_LINE */
typedef struct CMPNativeMutex { /* GCOVR_EXCL_LINE */
  pthread_mutex_t mutex;        /* GCOVR_EXCL_LINE */
} CMPNativeMutex;               /* GCOVR_EXCL_LINE */

typedef struct CMPNativeCond {
  pthread_cond_t cond;
} CMPNativeCond; /* GCOVR_EXCL_LINE */

typedef struct CMPNativeThread { /* GCOVR_EXCL_LINE */
  pthread_t thread;              /* GCOVR_EXCL_LINE */
} CMPNativeThread;               /* GCOVR_EXCL_LINE */
#endif

#if defined(CMP_TASKS_USE_PTHREAD) /* GCOVR_EXCL_LINE */
static int cmp_tasks_pthread_mutex_init(pthread_mutex_t *mutex) {
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_INIT)) {
    return 1;
  }
#endif /* GCOVR_EXCL_LINE */
  return pthread_mutex_init(mutex, NULL);
}

static int cmp_tasks_pthread_mutex_destroy(pthread_mutex_t *mutex) {
  int rc; /* GCOVR_EXCL_LINE */

  rc = pthread_mutex_destroy(mutex);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_DESTROY)) {
    return 1;
  }
#endif
  return rc;
}

static int cmp_tasks_pthread_mutex_lock(pthread_mutex_t *mutex) {
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_LOCK)) {
    return 1;
  }
#endif /* GCOVR_EXCL_LINE */
  return pthread_mutex_lock(mutex);
}

static int cmp_tasks_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  int rc; /* GCOVR_EXCL_LINE */

  rc = pthread_mutex_unlock(mutex);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK)) {
    return 1;
  }
#endif
  return rc;
}

static int cmp_tasks_pthread_cond_init(pthread_cond_t *cond) {
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_INIT)) {
    return 1;
  }
#endif /* GCOVR_EXCL_LINE */
  return pthread_cond_init(cond, NULL);
}

static int cmp_tasks_pthread_cond_destroy(pthread_cond_t *cond) {
  int rc;

  rc = pthread_cond_destroy(cond);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_DESTROY)) {
    return 1;
  }
#endif
  return rc;
}

static int cmp_tasks_pthread_cond_signal(pthread_cond_t *cond) {
  int rc;

  rc = pthread_cond_signal(cond);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_SIGNAL)) {
    return 1;
  }
#endif /* GCOVR_EXCL_LINE */
  return rc;
}

static int cmp_tasks_pthread_cond_broadcast(pthread_cond_t *cond) {
  int rc;

  rc = pthread_cond_broadcast(cond);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_BROADCAST)) {
    return 1;
  }
#endif
  return rc;
}

static int
cmp_tasks_pthread_cond_wait(pthread_cond_t *cond,
                            pthread_mutex_t *mutex) { /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING                                    /* GCOVR_EXCL_LINE */
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_WAIT)) {
    return EINVAL;
  }
#endif /* GCOVR_EXCL_LINE */
  return pthread_cond_wait(cond, mutex);
}

static int cmp_tasks_pthread_cond_timedwait(pthread_cond_t *cond,
                                            pthread_mutex_t *mutex,
                                            const struct timespec *ts) {
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT)) {
    g_cmp_tasks_test_fail_point = CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR;
    return ETIMEDOUT;
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR)) {
    return EINVAL;
  }
#endif /* GCOVR_EXCL_LINE */
  return pthread_cond_timedwait(cond, mutex, ts);
}

static int cmp_tasks_pthread_create(pthread_t *thread, void *(*entry)(void *),
                                    void *user) { /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING                                /* GCOVR_EXCL_LINE */
  if (g_cmp_tasks_test_thread_create_fail_after > 0) {
    g_cmp_tasks_test_thread_create_count += 1;
    if (g_cmp_tasks_test_thread_create_count ==
        g_cmp_tasks_test_thread_create_fail_after) {
      g_cmp_tasks_test_thread_create_fail_after = 0;
      return 1;
    }
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_THREAD_CREATE)) {
    return 1;
  }
#endif
  return pthread_create(thread, NULL, entry, user);
}

static int cmp_tasks_pthread_join(pthread_t thread) {
  int rc;

  rc = pthread_join(thread, NULL);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_THREAD_JOIN)) {
    return 1;
  }
#endif
  return rc;
}

static int cmp_tasks_gettimeofday(struct timeval *tv) {
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_TIME_NOW)) {
    return -1;
  }
#endif
  return gettimeofday(tv, NULL);
}

#if defined(CMP_TASKS_USE_PTHREAD)
static int cmp_tasks_nanosleep(struct timespec *req_spec); /* GCOVR_EXCL_LINE */
#endif

static int cmp_tasks_usleep(cmp_u32 usec) {
#if defined(CMP_TASKS_USE_PTHREAD)
  struct timespec req; /* GCOVR_EXCL_LINE */
#endif                 /* GCOVR_EXCL_LINE */
#if defined(CMP_TASKS_USE_PTHREAD)
  {
    req.tv_sec = (time_t)(usec / 1000000u);
    req.tv_nsec = (long)((usec % 1000000u) * 1000u);

    while (cmp_tasks_nanosleep(&req) != 0) {
      if (errno != EINTR) {
        return -1;
      }
    }
    return 0;
  }
#else  /* GCOVR_EXCL_LINE */
  return usleep((unsigned int)usec);
#endif /* GCOVR_EXCL_LINE */
}

#if defined(CMP_TASKS_USE_PTHREAD)
static int cmp_tasks_nanosleep(struct timespec *req_spec) {
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_SLEEP)) {
    errno = EINVAL;
    return -1;
  }
#endif
  return nanosleep(req_spec, req_spec);
}
#endif /* GCOVR_EXCL_LINE */
#endif

typedef struct CMPTaskItem { /* GCOVR_EXCL_LINE */
  CMPTaskFn fn;              /* GCOVR_EXCL_LINE */
  void *user;                /* GCOVR_EXCL_LINE */
  cmp_u32 due_time_ms;       /* GCOVR_EXCL_LINE */
} CMPTaskItem;

typedef struct CMPTasksDefault CMPTasksDefault;

typedef struct CMPTaskWorker {
  CMPNativeThread thread;
  CMPTasksDefault *runner;
  CMPBool started;
} CMPTaskWorker;

typedef struct CMPTaskThread {
  CMPObjectHeader header;
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPNativeThread thread;  /* GCOVR_EXCL_LINE */
  CMPThreadFn entry;
  void *user;
  CMPBool joined;
  int result;
} CMPTaskThread;

typedef struct CMPTaskMutex {
  CMPObjectHeader header; /* GCOVR_EXCL_LINE */
  CMPTasksDefault *runner;
  CMPNativeMutex mutex; /* GCOVR_EXCL_LINE */
  CMPBool initialized;
} CMPTaskMutex; /* GCOVR_EXCL_LINE */

struct CMPTasksDefault {
  CMPAllocator allocator; /* GCOVR_EXCL_LINE */
  CMPHandleSystem handles;
  CMPTasks tasks;           /* GCOVR_EXCL_LINE */
  CMPTaskItem *queue;       /* GCOVR_EXCL_LINE */
  cmp_usize queue_capacity; /* GCOVR_EXCL_LINE */
  cmp_usize queue_count;
  CMPNativeMutex queue_mutex;
  CMPNativeCond queue_cond; /* GCOVR_EXCL_LINE */
  CMPTaskWorker *workers;
  cmp_usize worker_count; /* GCOVR_EXCL_LINE */
  cmp_usize live_threads;
  cmp_usize live_mutexes; /* GCOVR_EXCL_LINE */
  CMPBool stopping;
  CMPBool initialized; /* GCOVR_EXCL_LINE */
  int last_task_error;
};

static cmp_u32 cmp_u32_max_value(void) { return (cmp_u32) ~(cmp_u32)0; }

static cmp_usize cmp_usize_max_value(void) { return (cmp_usize) ~(cmp_usize)0; }

static int cmp_tasks_mul_overflow(cmp_usize a, cmp_usize b,
                                  cmp_usize *out_value) {
  cmp_usize max_value; /* GCOVR_EXCL_LINE */

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = cmp_usize_max_value();
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}
static int cmp_tasks_time_now_ms(cmp_u32 *out_ms) {
#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_TIME_NOW)) {
    return CMP_ERR_UNKNOWN; /* GCOVR_EXCL_LINE */
  }
#endif
  if (out_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ms = (cmp_u32)GetTickCount();
  return CMP_OK;
#else /* GCOVR_EXCL_LINE */
  struct timeval tv;
  cmp_u32 sec;
  cmp_u32 usec;

  if (out_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (cmp_tasks_gettimeofday(&tv) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  sec = (cmp_u32)tv.tv_sec;
  usec = (cmp_u32)tv.tv_usec;
  *out_ms = sec * 1000u + usec / 1000u;
  return CMP_OK;
#endif
}

static int cmp_native_mutex_init(CMPNativeMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_INIT)) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  InitializeCriticalSection(&mutex->cs);
  return CMP_OK;
#else /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_mutex_init(&mutex->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_mutex_destroy(CMPNativeMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
  DeleteCriticalSection(&mutex->cs);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_DESTROY)) {
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */
  return CMP_OK;
#else /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_mutex_destroy(&mutex->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_mutex_lock(CMPNativeMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_LOCK)) {
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */
  EnterCriticalSection(&mutex->cs);
  return CMP_OK;
#else
  if (cmp_tasks_pthread_mutex_lock(&mutex->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_mutex_unlock(CMPNativeMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)    /* GCOVR_EXCL_LINE */
  LeaveCriticalSection(&mutex->cs); /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK)) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  return CMP_OK;
#else  /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_mutex_unlock(&mutex->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif /* GCOVR_EXCL_LINE */
}

static int cmp_native_cond_init(CMPNativeCond *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_INIT)) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  InitializeConditionVariable(&cond->cond); /* GCOVR_EXCL_LINE */
  return CMP_OK;
#else  /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_cond_init(&cond->cond) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif /* GCOVR_EXCL_LINE */
}

static int cmp_native_cond_destroy(CMPNativeCond *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_DESTROY)) {
    return CMP_ERR_UNKNOWN; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  return CMP_OK;
#else
  if (cmp_tasks_pthread_cond_destroy(&cond->cond) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_cond_signal(CMPNativeCond *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)      /* GCOVR_EXCL_LINE */
  WakeConditionVariable(&cond->cond); /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(/* GCOVR_EXCL_LINE */
                                  CMP_TASKS_TEST_FAIL_COND_SIGNAL)) { /* GCOVR_EXCL_LINE
                                                                       */
    return CMP_ERR_UNKNOWN; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  return CMP_OK;
#else  /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_cond_signal(&cond->cond) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif /* GCOVR_EXCL_LINE */
}

static int cmp_native_cond_broadcast(CMPNativeCond *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
  WakeAllConditionVariable(&cond->cond);
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(
          CMP_TASKS_TEST_FAIL_COND_BROADCAST)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNKNOWN;
  }
#endif
  return CMP_OK;
#else
  if (cmp_tasks_pthread_cond_broadcast(&cond->cond) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_cond_wait(CMPNativeCond *cond, CMPNativeMutex *mutex) {
  if (cond == NULL || mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_WAIT)) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  if (!SleepConditionVariableCS(&cond->cond, &mutex->cs,
                                INFINITE)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK; /* GCOVR_EXCL_LINE */
#else            /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_cond_wait(&cond->cond, &mutex->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

static int cmp_native_cond_timedwait(CMPNativeCond *cond, CMPNativeMutex *mutex,
                                     cmp_u32 wait_ms) {
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_INVALID)) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#endif
  if (cond == NULL || mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_TIME_NOW)) {
    return CMP_ERR_UNKNOWN;
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT)) {
    g_cmp_tasks_test_fail_point =
        CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR; /* GCOVR_EXCL_LINE */
    return CMP_ERR_TIMEOUT;
  }
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR)) {
    return CMP_ERR_UNKNOWN; /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  if (!SleepConditionVariableCS(&cond->cond, &mutex->cs, (DWORD)wait_ms)) {
    if (GetLastError() == ERROR_TIMEOUT) { /* GCOVR_EXCL_LINE */
      return CMP_ERR_TIMEOUT;
    }
    return CMP_ERR_UNKNOWN; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;   /* GCOVR_EXCL_LINE */
#else              /* GCOVR_EXCL_LINE */
  {
    struct timeval tv;
    struct timespec ts;
    cmp_u32 ms_part;
    long nsec;  /* GCOVR_EXCL_LINE */
    time_t sec; /* GCOVR_EXCL_LINE */
    int wait_rc;

    if (cmp_tasks_gettimeofday(&tv) != 0) {
      return CMP_ERR_UNKNOWN;
    }

    ms_part = wait_ms % 1000u;
    sec = tv.tv_sec + (time_t)(wait_ms / 1000u);
    nsec = (long)tv.tv_usec * 1000L + (long)ms_part * 1000000L;
    if (nsec >= 1000000000L) {
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
      g_cmp_tasks_test_nsec_adjusted = 1;
#endif
      sec += 1;
      nsec -= 1000000000L;
    }
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;

    wait_rc = cmp_tasks_pthread_cond_timedwait(&cond->cond, &mutex->mutex, &ts);
    if (wait_rc == 0) {
      return CMP_OK;
    }
    if (wait_rc == ETIMEDOUT) {
      return CMP_ERR_TIMEOUT;
    }
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */
}

static int cmp_native_sleep_ms(cmp_u32 ms) {
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_SLEEP)) {
    return CMP_ERR_UNKNOWN;
  }
#endif
  Sleep(ms);
  return CMP_OK;
#else /* GCOVR_EXCL_LINE */
  if (cmp_tasks_usleep(ms * 1000u) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif
}

#if defined(CMP_TASKS_USE_WIN32)
#define CMP_TASK_THREAD_RETURN DWORD
#define CMP_TASK_THREAD_CALL WINAPI
#else
#define CMP_TASK_THREAD_RETURN void *
#define CMP_TASK_THREAD_CALL
#endif /* GCOVR_EXCL_LINE */

static int cmp_native_thread_create(
    CMPNativeThread *thread,
    CMP_TASK_THREAD_RETURN(CMP_TASK_THREAD_CALL *entry)(void *), void *user) {
  if (thread == NULL || entry == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (g_cmp_tasks_test_thread_create_fail_after > 0) {
    g_cmp_tasks_test_thread_create_count += 1; /* GCOVR_EXCL_LINE */
    if (g_cmp_tasks_test_thread_create_count ==
        g_cmp_tasks_test_thread_create_fail_after) {
      g_cmp_tasks_test_thread_create_fail_after = 0; /* GCOVR_EXCL_LINE */
      return CMP_ERR_UNKNOWN;
    }
  }
  if (cmp_tasks_test_consume_fail(
          CMP_TASKS_TEST_FAIL_THREAD_CREATE)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNKNOWN;                     /* GCOVR_EXCL_LINE */
  }
#endif /* GCOVR_EXCL_LINE */
  thread->handle =
      CreateThread(NULL, 0, entry, user, 0, &thread->id); /* GCOVR_EXCL_LINE */
  if (thread->handle == NULL) {                           /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK; /* GCOVR_EXCL_LINE */
#else            /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_create(&thread->thread, entry, user) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif           /* GCOVR_EXCL_LINE */
}

static int cmp_native_thread_join(CMPNativeThread *thread) {
  if (thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_TASKS_USE_WIN32)
  if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) {
    return CMP_ERR_UNKNOWN;
  }
  CloseHandle(thread->handle);
  thread->handle = NULL;
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(
          CMP_TASKS_TEST_FAIL_THREAD_JOIN)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNKNOWN;
  }
#endif
  return CMP_OK;
#else  /* GCOVR_EXCL_LINE */
  if (cmp_tasks_pthread_join(thread->thread) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
#endif /* GCOVR_EXCL_LINE */
}

static int cmp_task_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_task_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_task_thread_destroy(void *obj) {
  CMPTaskThread *thread;
  CMPTasksDefault *runner;
  int rc; /* GCOVR_EXCL_LINE */

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  thread = (CMPTaskThread *)obj;
  if (!thread->joined) {
    return CMP_ERR_STATE;
  }

  runner = thread->runner;
  if (runner != NULL && runner->handles.vtable != NULL) {
    CMPHandle handle = thread->header.handle;
    if (handle.id != 0 && handle.generation != 0) {
      rc = runner->handles.vtable->unregister_object(runner->handles.ctx,
                                                     handle);
      if (rc != CMP_OK) {
        return rc;
      }
    }
  }

  if (runner != NULL) {
    rc = runner->allocator.free(runner->allocator.ctx, thread);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int cmp_task_mutex_destroy(void *obj) {
  CMPTaskMutex *mutex; /* GCOVR_EXCL_LINE */
  CMPTasksDefault *runner;
  int rc; /* GCOVR_EXCL_LINE */

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mutex = (CMPTaskMutex *)obj;
  if (mutex->initialized) {
    rc = cmp_native_mutex_destroy(&mutex->mutex);
    if (rc != CMP_OK) {
      return rc;
    }
    mutex->initialized = CMP_FALSE;
  }

  runner = mutex->runner;
  if (runner != NULL && runner->handles.vtable != NULL) {
    CMPHandle handle = mutex->header.handle;
    if (handle.id != 0 && handle.generation != 0) {
      rc = runner->handles.vtable->unregister_object(runner->handles.ctx,
                                                     handle);
      if (rc != CMP_OK) {
        return rc;
      }
    }
  }

  if (runner != NULL) {
    rc = runner->allocator.free(runner->allocator.ctx, mutex);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int cmp_task_object_get_type_id(void *obj, cmp_u32 *out_type_id) {
  return cmp_object_get_type_id((CMPObjectHeader *)obj, out_type_id);
}

static const CMPObjectVTable g_cmp_task_thread_vtable =
    {                                                 /* GCOVR_EXCL_LINE */
     cmp_task_object_retain, cmp_task_object_release, /* GCOVR_EXCL_LINE */
     cmp_task_thread_destroy,                         /* GCOVR_EXCL_LINE */
     cmp_task_object_get_type_id};                    /* GCOVR_EXCL_LINE */

static const CMPObjectVTable g_cmp_task_mutex_vtable =
    {/* GCOVR_EXCL_LINE */
     cmp_task_object_retain, cmp_task_object_release,
     cmp_task_mutex_destroy,       /* GCOVR_EXCL_LINE */
     cmp_task_object_get_type_id}; /* GCOVR_EXCL_LINE */

static int cmp_tasks_queue_push(CMPTasksDefault *runner, CMPTaskFn fn,
                                void *user,
                                cmp_u32 due_time_ms) { /* GCOVR_EXCL_LINE */
  if (runner->queue_count >= runner->queue_capacity) {
    return CMP_ERR_BUSY;
  }
  runner->queue[runner->queue_count].fn = fn;
  runner->queue[runner->queue_count].user = user;
  runner->queue[runner->queue_count].due_time_ms = due_time_ms;
  runner->queue_count += 1;
  return CMP_OK;
}

static int cmp_tasks_queue_pick(CMPTasksDefault *runner, CMPTaskItem *out_task,
                                cmp_u32 *out_wait_ms) { /* GCOVR_EXCL_LINE */
  cmp_u32 now;
  cmp_u32 earliest_due;     /* GCOVR_EXCL_LINE */
  cmp_usize earliest_index; /* GCOVR_EXCL_LINE */
  cmp_usize i;              /* GCOVR_EXCL_LINE */
  int rc;

  if (runner->queue_count == 0) {
    return CMP_ERR_NOT_FOUND;
  }
  if (out_task == NULL || out_wait_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  earliest_index = 0;
  earliest_due = runner->queue[0].due_time_ms;
  for (i = 1; i < runner->queue_count; ++i) {
    if (runner->queue[i].due_time_ms < earliest_due) {
      earliest_due = runner->queue[i].due_time_ms;
      earliest_index = i;
    }
  }

  rc = cmp_tasks_time_now_ms(&now);
  if (rc != CMP_OK) {
    return rc;
  }

  if (now < earliest_due) {
    *out_wait_ms = earliest_due - now;
    return CMP_ERR_NOT_READY;
  }

  *out_task = runner->queue[earliest_index];
  runner->queue_count -= 1;
  if (earliest_index != runner->queue_count) {
    runner->queue[earliest_index] = runner->queue[runner->queue_count];
  }
  *out_wait_ms = 0;
  return CMP_OK;
}

static CMP_TASK_THREAD_RETURN CMP_TASK_THREAD_CALL /* GCOVR_EXCL_LINE */
cmp_tasks_worker_entry(void *user) {
  CMPTasksDefault *runner;
  int rc;      /* GCOVR_EXCL_LINE */
  int task_rc; /* GCOVR_EXCL_LINE */

  runner = (CMPTasksDefault *)user;
  if (runner == NULL) {
#if defined(CMP_TASKS_USE_WIN32)
    return 0; /* GCOVR_EXCL_LINE */
#else         /* GCOVR_EXCL_LINE */
    return NULL;
#endif        /* GCOVR_EXCL_LINE */
  }

  for (;;) {
    CMPTaskItem task = {0};
    cmp_u32 wait_ms; /* GCOVR_EXCL_LINE */

    rc = cmp_native_mutex_lock(&runner->queue_mutex);
    if (rc != CMP_OK) {
      break;
    }

    while (!runner->stopping) {
      rc = cmp_tasks_queue_pick(runner, &task, &wait_ms);
      if (rc == CMP_OK) {
        break;
      }
      if (rc == CMP_ERR_NOT_FOUND) {
        rc = cmp_native_cond_wait(&runner->queue_cond, &runner->queue_mutex);
        if (rc != CMP_OK) {
          runner->stopping = CMP_TRUE;
          break;
        }
        continue;
      }
      if (rc == CMP_ERR_NOT_READY) {
        rc = cmp_native_cond_timedwait(&runner->queue_cond,
                                       &runner->queue_mutex, wait_ms);
        if (rc == CMP_ERR_TIMEOUT) {
          continue;
        }
        if (rc != CMP_OK) {
          runner->stopping = CMP_TRUE;
          break;
        }
        continue;
      }
      runner->stopping = CMP_TRUE;
      break;
    }

    if (runner->stopping) {
      cmp_native_mutex_unlock(&runner->queue_mutex);
      break;
    }

    rc = cmp_native_mutex_unlock(&runner->queue_mutex);
    if (rc != CMP_OK) {
      break;
    }

    if (task.fn != NULL) {
      task_rc = task.fn(task.user);
      if (task_rc != CMP_OK) {
        rc = cmp_native_mutex_lock(&runner->queue_mutex);
        if (rc == CMP_OK) {
          runner->last_task_error = task_rc;
          cmp_native_mutex_unlock(&runner->queue_mutex);
        }
      }
    }
  }

#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
  return 0;                      /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
  return NULL;
#endif                           /* GCOVR_EXCL_LINE */
}

static CMP_TASK_THREAD_RETURN CMP_TASK_THREAD_CALL /* GCOVR_EXCL_LINE */
cmp_tasks_thread_entry(void *user) {
  CMPTaskThread *thread = NULL;

  thread = (CMPTaskThread *)user;
  if (thread == NULL || thread->entry == NULL) {
#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
    return 0;                    /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
    return NULL;
#endif                           /* GCOVR_EXCL_LINE */
  }

  thread->result = thread->entry(thread->user);

#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
  return 0;                      /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
  return NULL;
#endif                           /* GCOVR_EXCL_LINE */
}

static int /* GCOVR_EXCL_LINE */
cmp_tasks_default_thread_create(void *tasks, CMPThreadFn entry, void *user,
                                CMPHandle *out_thread) { /* GCOVR_EXCL_LINE */
  CMPTasksDefault *runner;                               /* GCOVR_EXCL_LINE */
  CMPTaskThread *thread;                                 /* GCOVR_EXCL_LINE */
  int rc;                                                /* GCOVR_EXCL_LINE */

  if (tasks == NULL || entry == NULL || out_thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  out_thread->id = 0;
  out_thread->generation = 0;

  rc = runner->allocator.alloc(runner->allocator.ctx, sizeof(*thread),
                               (void **)&thread); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }
  memset(thread, 0, sizeof(*thread));

  rc = CMP_OK;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_OBJECT_INIT)) {
    rc = CMP_ERR_UNKNOWN;
  } else
#endif /* GCOVR_EXCL_LINE */
  {
    rc =
        cmp_object_header_init(&thread->header, CMP_TASK_OBJECT_THREAD, 0,
                               &g_cmp_task_thread_vtable); /* GCOVR_EXCL_LINE */
  }
  if (rc != CMP_OK) {
    runner->allocator.free(runner->allocator.ctx, thread);
    return rc;
  }

  thread->runner = runner;
  thread->entry = entry;
  thread->user = user;
  thread->joined = CMP_FALSE;
  thread->result = CMP_OK;

  rc =
      cmp_native_thread_create(&thread->thread, cmp_tasks_thread_entry, thread);
  if (rc != CMP_OK) {
    runner->allocator.free(runner->allocator.ctx, thread);
    return rc;
  }

  rc = runner->handles.vtable->register_object(runner->handles.ctx,
                                               &thread->header);
  if (rc != CMP_OK) {
    cmp_native_thread_join(&thread->thread);
    runner->allocator.free(runner->allocator.ctx, thread);
    return rc;
  }

  runner->live_threads += 1;
  *out_thread = thread->header.handle;
  return CMP_OK;
}

static int cmp_tasks_default_thread_join(void *tasks, CMPHandle thread_handle) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPTaskThread *thread;   /* GCOVR_EXCL_LINE */
  void *resolved;          /* GCOVR_EXCL_LINE */
  int rc;                  /* GCOVR_EXCL_LINE */

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = runner->handles.vtable->resolve(runner->handles.ctx, thread_handle,
                                       &resolved); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }

  thread = (CMPTaskThread *)resolved;
  if (thread->joined) {
    return CMP_ERR_STATE;
  }

  rc = cmp_native_thread_join(&thread->thread);
  if (rc != CMP_OK) {
    return rc;
  }
  thread->joined = CMP_TRUE;

  rc = runner->handles.vtable->unregister_object(runner->handles.ctx,
                                                 thread_handle);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = runner->allocator.free(runner->allocator.ctx, thread);
  if (rc != CMP_OK) {
    return rc;
  }

  if (runner->live_threads > 0) {
    runner->live_threads -= 1;
  }

  return CMP_OK;
}

static int cmp_tasks_default_mutex_create(void *tasks, CMPHandle *out_mutex) {
  CMPTasksDefault *runner;
  CMPTaskMutex *mutex; /* GCOVR_EXCL_LINE */
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  out_mutex->id = 0;
  out_mutex->generation = 0;

  rc = runner->allocator.alloc(runner->allocator.ctx, sizeof(*mutex),
                               (void **)&mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  memset(mutex, 0, sizeof(*mutex));

  rc = CMP_OK;
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_OBJECT_INIT)) {
    rc = CMP_ERR_UNKNOWN;
  } else
#endif
  {
    rc = cmp_object_header_init(&mutex->header, CMP_TASK_OBJECT_MUTEX, 0,
                                &g_cmp_task_mutex_vtable); /* GCOVR_EXCL_LINE */
  }
  if (rc != CMP_OK) {
    runner->allocator.free(runner->allocator.ctx, mutex);
    return rc;
  }

  mutex->runner = runner;
  mutex->initialized = CMP_FALSE;

  rc = cmp_native_mutex_init(&mutex->mutex);
  if (rc != CMP_OK) {
    runner->allocator.free(runner->allocator.ctx, mutex);
    return rc;
  }
  mutex->initialized = CMP_TRUE;

  rc = runner->handles.vtable->register_object(runner->handles.ctx,
                                               &mutex->header);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&mutex->mutex);
    runner->allocator.free(runner->allocator.ctx, mutex);
    return rc;
  }

  runner->live_mutexes += 1;
  *out_mutex = mutex->header.handle;
  return CMP_OK;
}

static int cmp_tasks_default_mutex_destroy(void *tasks,
                                           CMPHandle mutex_handle) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPTaskMutex *mutex;     /* GCOVR_EXCL_LINE */
  void *resolved;          /* GCOVR_EXCL_LINE */
  int rc;                  /* GCOVR_EXCL_LINE */

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle,
                                       &resolved);
  if (rc != CMP_OK) {
    return rc;
  }

  mutex = (CMPTaskMutex *)resolved;
  if (mutex->initialized) {
    rc = cmp_native_mutex_destroy(&mutex->mutex);
    if (rc != CMP_OK) {
      return rc;
    }
    mutex->initialized = CMP_FALSE;
  }

  rc = runner->handles.vtable->unregister_object(runner->handles.ctx,
                                                 mutex_handle);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = runner->allocator.free(runner->allocator.ctx, mutex);
  if (rc != CMP_OK) {
    return rc;
  }

  if (runner->live_mutexes > 0) {
    runner->live_mutexes -= 1;
  }

  return CMP_OK;
}

static int cmp_tasks_default_mutex_lock(void *tasks, CMPHandle mutex_handle) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPTaskMutex *mutex;
  void *resolved;
  int rc;

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle,
                                       &resolved); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }

  mutex = (CMPTaskMutex *)resolved;
  if (!mutex->initialized) {
    return CMP_ERR_STATE;
  }

  return cmp_native_mutex_lock(&mutex->mutex);
}

static int cmp_tasks_default_mutex_unlock(void *tasks, CMPHandle mutex_handle) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPTaskMutex *mutex;     /* GCOVR_EXCL_LINE */
  void *resolved;          /* GCOVR_EXCL_LINE */
  int rc;                  /* GCOVR_EXCL_LINE */

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle,
                                       &resolved); /* GCOVR_EXCL_LINE */
  if (rc != CMP_OK) {
    return rc;
  }

  mutex = (CMPTaskMutex *)resolved;
  if (!mutex->initialized) {
    return CMP_ERR_STATE;
  }

  return cmp_native_mutex_unlock(&mutex->mutex);
}

static int cmp_tasks_default_sleep_ms(void *tasks, cmp_u32 ms) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }
  return cmp_native_sleep_ms(ms);
}

static int cmp_tasks_default_task_post(void *tasks, CMPTaskFn fn, void *user) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  int rc;                  /* GCOVR_EXCL_LINE */
  int unlock_rc;           /* GCOVR_EXCL_LINE */

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_native_mutex_lock(&runner->queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_tasks_queue_push(runner, fn, user, 0);
  if (rc == CMP_OK) {
    rc = cmp_native_cond_signal(&runner->queue_cond);
  }

  unlock_rc = cmp_native_mutex_unlock(&runner->queue_mutex);
  if (rc == CMP_OK && unlock_rc != CMP_OK) {
    rc = unlock_rc;
  }

  return rc;
}

static int cmp_tasks_default_task_post_delayed(void *tasks, CMPTaskFn fn,
                                               void *user, cmp_u32 delay_ms) {
  CMPTasksDefault *runner;
  cmp_u32 now;
  cmp_u32 due_time;
  int rc;        /* GCOVR_EXCL_LINE */
  int unlock_rc; /* GCOVR_EXCL_LINE */

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  runner = (CMPTasksDefault *)tasks;
  if (!runner->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_tasks_time_now_ms(&now);
  if (rc != CMP_OK) {
    return rc;
  }

  if (delay_ms > cmp_u32_max_value() - now) {
    return CMP_ERR_OVERFLOW;
  }
  due_time = now + delay_ms;

  rc = cmp_native_mutex_lock(&runner->queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_tasks_queue_push(runner, fn, user, due_time);
  if (rc == CMP_OK) {
    rc = cmp_native_cond_signal(&runner->queue_cond);
  }

  unlock_rc = cmp_native_mutex_unlock(&runner->queue_mutex);
  if (rc == CMP_OK && unlock_rc != CMP_OK) {
    rc = unlock_rc;
  }

  return rc;
}

static const CMPTasksVTable g_cmp_tasks_default_vtable = {
    cmp_tasks_default_thread_create,    cmp_tasks_default_thread_join,
    cmp_tasks_default_mutex_create,     cmp_tasks_default_mutex_destroy,
    cmp_tasks_default_mutex_lock,       cmp_tasks_default_mutex_unlock,
    cmp_tasks_default_sleep_ms,         cmp_tasks_default_task_post,
    cmp_tasks_default_task_post_delayed};

int CMP_CALL cmp_tasks_default_config_init(CMPTasksDefaultConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (cmp_tasks_test_consume_fail(CMP_TASKS_TEST_FAIL_CONFIG_INIT)) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  config->allocator = NULL;
  config->worker_count = CMP_TASKS_DEFAULT_WORKERS;
  config->queue_capacity = CMP_TASKS_DEFAULT_QUEUE_CAPACITY;
  config->handle_capacity = CMP_TASKS_DEFAULT_HANDLE_CAPACITY;
  return CMP_OK;
}

int CMP_CALL cmp_tasks_default_create(const CMPTasksDefaultConfig *config,
                                      CMPTasks *out_tasks) {
  CMPTasksDefaultConfig cfg;
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */
  CMPAllocator allocator;
  cmp_usize worker_count;
  cmp_usize queue_capacity;  /* GCOVR_EXCL_LINE */
  cmp_usize handle_capacity; /* GCOVR_EXCL_LINE */
  cmp_usize i;               /* GCOVR_EXCL_LINE */
  int rc;
  int first_error;

  if (out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_tasks->ctx = NULL;
  out_tasks->vtable = NULL;

  if (config == NULL) {
    rc = cmp_tasks_default_config_init(&cfg);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    cfg = *config;
  }

  worker_count = cfg.worker_count;
  queue_capacity = cfg.queue_capacity;
  handle_capacity = cfg.handle_capacity;

  if (worker_count == 0 || queue_capacity == 0 || handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (cfg.allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    allocator = *cfg.allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(*runner), (void **)&runner);
  if (rc != CMP_OK) {
    return rc;
  }
  memset(runner, 0, sizeof(*runner));

  runner->allocator = allocator;
  runner->queue_capacity = queue_capacity;
  runner->worker_count = worker_count;
  runner->last_task_error = CMP_OK;

  rc = cmp_handle_system_default_create(handle_capacity, &allocator,
                                        &runner->handles);
  if (rc != CMP_OK) {
    allocator.free(allocator.ctx, runner);
    return rc;
  }

  rc = cmp_native_mutex_init(&runner->queue_mutex);
  if (rc != CMP_OK) {
    cmp_handle_system_default_destroy(&runner->handles);
    allocator.free(allocator.ctx, runner);
    return rc;
  }

  rc = cmp_native_cond_init(&runner->queue_cond);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&runner->queue_mutex);
    cmp_handle_system_default_destroy(&runner->handles);
    allocator.free(allocator.ctx, runner);
    return rc;
  }

  {
    cmp_usize queue_bytes;

    rc = cmp_tasks_mul_overflow(queue_capacity, sizeof(*runner->queue),
                                &queue_bytes); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      cmp_native_cond_destroy(&runner->queue_cond);
      cmp_native_mutex_destroy(&runner->queue_mutex);
      cmp_handle_system_default_destroy(&runner->handles);
      allocator.free(allocator.ctx, runner);
      return rc;
    }

    rc = allocator.alloc(allocator.ctx, queue_bytes, (void **)&runner->queue);
  }
  if (rc != CMP_OK) {
    cmp_native_cond_destroy(&runner->queue_cond);
    cmp_native_mutex_destroy(&runner->queue_mutex);
    cmp_handle_system_default_destroy(&runner->handles);
    allocator.free(allocator.ctx, runner);
    return rc;
  }

  {
    cmp_usize worker_bytes;

    rc = cmp_tasks_mul_overflow(worker_count, sizeof(*runner->workers),
                                &worker_bytes);
    if (rc != CMP_OK) {
      allocator.free(allocator.ctx, runner->queue);
      cmp_native_cond_destroy(&runner->queue_cond);
      cmp_native_mutex_destroy(&runner->queue_mutex);
      cmp_handle_system_default_destroy(&runner->handles);
      allocator.free(allocator.ctx, runner);
      return rc;
    }

    rc =
        allocator.alloc(allocator.ctx, worker_bytes, (void **)&runner->workers);
  }
  if (rc != CMP_OK) {
    allocator.free(allocator.ctx, runner->queue);
    cmp_native_cond_destroy(&runner->queue_cond);
    cmp_native_mutex_destroy(&runner->queue_mutex);
    cmp_handle_system_default_destroy(&runner->handles);
    allocator.free(allocator.ctx, runner);
    return rc;
  }
  memset(runner->workers, 0, sizeof(*runner->workers) * worker_count);

  runner->stopping = CMP_FALSE;
  runner->queue_count = 0;

  first_error = CMP_OK;
  for (i = 0; i < worker_count; ++i) {
    runner->workers[i].runner = runner;
    rc = cmp_native_thread_create(&runner->workers[i].thread,
                                  cmp_tasks_worker_entry, runner);
    if (rc != CMP_OK) {
      first_error = rc;
      runner->stopping = CMP_TRUE;
      break;
    }
    runner->workers[i].started = CMP_TRUE;
  }

  if (first_error != CMP_OK) {
    cmp_native_cond_broadcast(&runner->queue_cond);
    for (i = 0; i < worker_count; ++i) {
      if (runner->workers[i].started) {
        cmp_native_thread_join(&runner->workers[i].thread);
      }
    }
    allocator.free(allocator.ctx, runner->workers);
    allocator.free(allocator.ctx, runner->queue);
    cmp_native_cond_destroy(&runner->queue_cond);
    cmp_native_mutex_destroy(&runner->queue_mutex);
    cmp_handle_system_default_destroy(&runner->handles);
    allocator.free(allocator.ctx, runner);
    return first_error;
  }

  runner->tasks.ctx = runner;
  runner->tasks.vtable = &g_cmp_tasks_default_vtable;
  runner->initialized = CMP_TRUE;

  *out_tasks = runner->tasks;
  return CMP_OK;
}

int CMP_CALL cmp_tasks_default_destroy(CMPTasks *tasks) {
  CMPTasksDefault *runner;
  CMPAllocator allocator;
  int rc; /* GCOVR_EXCL_LINE */
  int first_error;
  cmp_usize i;

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (tasks->ctx == NULL || tasks->vtable == NULL) {
    return CMP_ERR_STATE;
  }

  runner = (CMPTasksDefault *)tasks->ctx;
  if (runner == NULL || !runner->initialized) {
    return CMP_ERR_STATE;
  }
  if (runner->live_threads != 0 || runner->live_mutexes != 0) {
    return CMP_ERR_BUSY;
  }

  allocator = runner->allocator;
  first_error = CMP_OK;

  rc = cmp_native_mutex_lock(&runner->queue_mutex);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
  runner->stopping = CMP_TRUE;
  rc = cmp_native_cond_broadcast(&runner->queue_cond);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
  rc = cmp_native_mutex_unlock(&runner->queue_mutex);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  for (i = 0; i < runner->worker_count; ++i) {
    if (runner->workers[i].started) {
      rc = cmp_native_thread_join(&runner->workers[i].thread);
      if (rc != CMP_OK && first_error == CMP_OK) {
        first_error = rc;
      }
    }
  }

  rc = cmp_native_cond_destroy(&runner->queue_cond);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
  rc = cmp_native_mutex_destroy(&runner->queue_mutex);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  rc = cmp_handle_system_default_destroy(&runner->handles);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  rc = allocator.free(allocator.ctx, runner->workers);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
  rc = allocator.free(allocator.ctx, runner->queue);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  runner->initialized = CMP_FALSE;
  tasks->ctx = NULL;
  tasks->vtable = NULL;

  rc = allocator.free(allocator.ctx, runner);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  return first_error;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_tasks_test_mul_overflow(
    cmp_usize a, cmp_usize b, cmp_usize *out_value) { /* GCOVR_EXCL_LINE */
  return cmp_tasks_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_tasks_test_call_native(cmp_u32 op) {
  switch (op) {
  case CMP_TASKS_TEST_NATIVE_TIME_NOW:
    return cmp_tasks_time_now_ms(NULL);
  case CMP_TASKS_TEST_NATIVE_MUTEX_INIT:
    return cmp_native_mutex_init(NULL);
  case CMP_TASKS_TEST_NATIVE_MUTEX_DESTROY: /* GCOVR_EXCL_LINE */
    return cmp_native_mutex_destroy(NULL);
  case CMP_TASKS_TEST_NATIVE_MUTEX_LOCK:
    return cmp_native_mutex_lock(NULL);
  case CMP_TASKS_TEST_NATIVE_MUTEX_UNLOCK:
    return cmp_native_mutex_unlock(NULL);
  case CMP_TASKS_TEST_NATIVE_COND_INIT: /* GCOVR_EXCL_LINE */
    return cmp_native_cond_init(NULL);
  case CMP_TASKS_TEST_NATIVE_COND_DESTROY:
    return cmp_native_cond_destroy(NULL);
  case CMP_TASKS_TEST_NATIVE_COND_SIGNAL:
    return cmp_native_cond_signal(NULL);
  case CMP_TASKS_TEST_NATIVE_COND_BROADCAST:
    return cmp_native_cond_broadcast(NULL);
  case CMP_TASKS_TEST_NATIVE_COND_WAIT:
    return cmp_native_cond_wait(NULL, NULL);
  case CMP_TASKS_TEST_NATIVE_COND_TIMEDWAIT:
    return cmp_native_cond_timedwait(NULL, NULL, 1);
  case CMP_TASKS_TEST_NATIVE_THREAD_CREATE:
    return cmp_native_thread_create(NULL, NULL, NULL);
  case CMP_TASKS_TEST_NATIVE_THREAD_JOIN:
    return cmp_native_thread_join(NULL);
  case CMP_TASKS_TEST_NATIVE_SLEEP:
    return cmp_native_sleep_ms(0);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_thread_destroy_case(cmp_u32 mode) {
  CMPTasksDefault runner;               /* GCOVR_EXCL_LINE */
  CMPTaskThread thread;                 /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable; /* GCOVR_EXCL_LINE */
  CMPTasksTestStub stub;

  memset(&runner, 0, sizeof(runner));
  memset(&thread, 0, sizeof(thread));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  stub.rc = CMP_OK;

  switch (mode) {
  case CMP_TASKS_TEST_THREAD_DESTROY_NULL:
    return cmp_task_thread_destroy(NULL);
  case CMP_TASKS_TEST_THREAD_DESTROY_NOT_JOINED:
    thread.joined = CMP_FALSE;
    return cmp_task_thread_destroy(&thread);
  case CMP_TASKS_TEST_THREAD_DESTROY_UNREGISTER_FAIL:
    thread.joined = CMP_TRUE;
    thread.runner = &runner;
    thread.header.handle.id = 1u;
    thread.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = &stub;
    stub.rc = CMP_ERR_UNKNOWN;
    return cmp_task_thread_destroy(&thread);
  case CMP_TASKS_TEST_THREAD_DESTROY_UNREGISTER_NULL_CTX:
    thread.joined = CMP_TRUE;
    thread.runner = &runner;
    thread.header.handle.id = 1u;
    thread.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = NULL;
    return cmp_task_thread_destroy(&thread);
  case CMP_TASKS_TEST_THREAD_DESTROY_FREE_FAIL:
    thread.joined = CMP_TRUE;
    thread.runner = &runner;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = &stub;
    stub.rc = CMP_ERR_OUT_OF_MEMORY;
    return cmp_task_thread_destroy(&thread);
  case CMP_TASKS_TEST_THREAD_DESTROY_FREE_NULL_CTX:
    thread.joined = CMP_TRUE;
    thread.runner = &runner;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = NULL;
    return cmp_task_thread_destroy(&thread);
  case CMP_TASKS_TEST_THREAD_DESTROY_OK:
    thread.joined = CMP_TRUE;
    thread.runner = &runner;
    thread.header.handle.id = 1u;
    thread.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = &stub;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = &stub;
    stub.rc = CMP_OK;
    return cmp_task_thread_destroy(&thread);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_mutex_destroy_case(cmp_u32 mode) {
  CMPTasksDefault runner;
  CMPTaskMutex mutex;
  CMPHandleSystemVTable handles_vtable;
  CMPTasksTestStub stub; /* GCOVR_EXCL_LINE */
  int rc;                /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(&mutex, 0, sizeof(mutex));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  stub.rc = CMP_OK;

  switch (mode) {
  case CMP_TASKS_TEST_MUTEX_DESTROY_NULL:
    return cmp_task_mutex_destroy(NULL);
  case CMP_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL:
    rc = cmp_native_mutex_init(&mutex.mutex);
    if (rc != CMP_OK) {
      return rc;
    }
    mutex.initialized = CMP_TRUE;
    mutex.runner = &runner;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_DESTROY);
    rc = cmp_task_mutex_destroy(&mutex);
    cmp_tasks_test_clear_fail_point();
    return rc;
  case CMP_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_FAIL:
    mutex.initialized = CMP_FALSE;
    mutex.runner = &runner;
    mutex.header.handle.id = 1u;
    mutex.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = &stub;
    stub.rc = CMP_ERR_UNKNOWN;
    return cmp_task_mutex_destroy(&mutex);
  case CMP_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_NULL_CTX:
    mutex.initialized = CMP_FALSE;
    mutex.runner = &runner;
    mutex.header.handle.id = 1u;
    mutex.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = NULL;
    return cmp_task_mutex_destroy(&mutex);
  case CMP_TASKS_TEST_MUTEX_DESTROY_FREE_FAIL:
    mutex.initialized = CMP_FALSE;
    mutex.runner = &runner;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = &stub;
    stub.rc = CMP_ERR_OUT_OF_MEMORY;
    return cmp_task_mutex_destroy(&mutex);
  case CMP_TASKS_TEST_MUTEX_DESTROY_FREE_NULL_CTX:
    mutex.initialized = CMP_FALSE;
    mutex.runner = &runner;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = NULL;
    return cmp_task_mutex_destroy(&mutex);
  case CMP_TASKS_TEST_MUTEX_DESTROY_OK:
    rc = cmp_native_mutex_init(&mutex.mutex);
    if (rc != CMP_OK) {
      return rc;
    }
    mutex.initialized = CMP_TRUE;
    mutex.runner = &runner;
    mutex.header.handle.id = 1u;
    mutex.header.handle.generation = 1u;
    handles_vtable.unregister_object = cmp_tasks_test_unregister;
    runner.handles.vtable = &handles_vtable;
    runner.handles.ctx = &stub;
    runner.allocator.free = cmp_tasks_test_free;
    runner.allocator.ctx = &stub;
    stub.rc = CMP_OK;
    return cmp_task_mutex_destroy(&mutex);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

static int cmp_tasks_test_task_fail(void *user) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */

  runner = (CMPTasksDefault *)user;
  if (runner != NULL) {
    runner->stopping = CMP_TRUE;
  }
  return CMP_ERR_UNKNOWN;
}

static int cmp_tasks_test_task_noop(void *user) {
  (void)user;
  return CMP_OK;
}

static CMP_TASK_THREAD_RETURN CMP_TASK_THREAD_CALL /* GCOVR_EXCL_LINE */
cmp_tasks_test_quick_exit(void *user) {
  (void)user;
#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
  return 0;                      /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
  return NULL;
#endif                           /* GCOVR_EXCL_LINE */
}

static CMP_TASK_THREAD_RETURN CMP_TASK_THREAD_CALL
cmp_tasks_test_signal_stop_entry(void *user) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */

  runner = (CMPTasksDefault *)user;
  if (runner == NULL) {
#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
    return 0;
#else  /* GCOVR_EXCL_LINE */
    return NULL;
#endif /* GCOVR_EXCL_LINE */
  }

  cmp_native_mutex_lock(&runner->queue_mutex);
  runner->stopping = CMP_TRUE;
  cmp_native_cond_signal(&runner->queue_cond);
  cmp_native_mutex_unlock(&runner->queue_mutex);

#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
  return 0;                      /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
  return NULL;
#endif                           /* GCOVR_EXCL_LINE */
}

int CMP_CALL cmp_tasks_test_thread_entry_case(cmp_u32 mode) {
  CMPTaskThread thread; /* GCOVR_EXCL_LINE */

  memset(&thread, 0, sizeof(thread));

  switch (mode) {
  case CMP_TASKS_TEST_THREAD_ENTRY_NULL: /* GCOVR_EXCL_LINE */
    (void)cmp_tasks_thread_entry(NULL);
    return CMP_OK;
  case CMP_TASKS_TEST_THREAD_ENTRY_NO_ENTRY: /* GCOVR_EXCL_LINE */
    thread.entry = NULL;
    (void)cmp_tasks_thread_entry(&thread);
    return CMP_OK;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_thread_create_case(cmp_u32 mode) {
  CMPTasksDefault runner;               /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable; /* GCOVR_EXCL_LINE */
  CMPTasksTestStub stub;
  CMPHandle out_thread; /* GCOVR_EXCL_LINE */
  int rc;               /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));

  runner.initialized = CMP_TRUE;
  runner.allocator.ctx = &stub;
  runner.allocator.alloc = cmp_tasks_test_alloc_ex;
  runner.allocator.realloc = cmp_tasks_test_realloc_ex;
  runner.allocator.free = cmp_tasks_test_free_ex;
  stub.rc_alloc = CMP_OK;
  stub.rc_register = CMP_OK;
  stub.rc_free = CMP_OK;

  handles_vtable.register_object = cmp_tasks_test_register_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  switch (mode) {
  case CMP_TASKS_TEST_THREAD_CREATE_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_thread_create(&runner, cmp_tasks_test_task_noop,
                                           NULL, &out_thread);
  case CMP_TASKS_TEST_THREAD_CREATE_ALLOC_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_alloc = CMP_ERR_OUT_OF_MEMORY;
    return cmp_tasks_default_thread_create(&runner, cmp_tasks_test_task_noop,
                                           NULL,
                                           &out_thread); /* GCOVR_EXCL_LINE */
  case CMP_TASKS_TEST_THREAD_CREATE_OBJECT_FAIL:         /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_OBJECT_INIT);
    rc =
        cmp_tasks_default_thread_create(&runner, cmp_tasks_test_task_noop, NULL,
                                        &out_thread); /* GCOVR_EXCL_LINE */
    cmp_tasks_test_clear_fail_point();
    return rc;
  case CMP_TASKS_TEST_THREAD_CREATE_NATIVE_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_CREATE);
    rc = cmp_tasks_default_thread_create(&runner, cmp_tasks_test_task_noop,
                                         NULL, &out_thread);
    cmp_tasks_test_clear_fail_point();
    return rc;
  case CMP_TASKS_TEST_THREAD_CREATE_REGISTER_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_register = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_thread_create(&runner, cmp_tasks_test_task_noop,
                                           NULL,
                                           &out_thread); /* GCOVR_EXCL_LINE */
  default:                                               /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_thread_join_case(cmp_u32 mode) {
  CMPTasksDefault runner;               /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable; /* GCOVR_EXCL_LINE */
  CMPTasksTestStub stub;                /* GCOVR_EXCL_LINE */
  CMPTaskThread *thread;
  CMPHandle handle; /* GCOVR_EXCL_LINE */
  int rc;           /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));

  runner.initialized = CMP_TRUE;
  runner.allocator.ctx = &stub;
  runner.allocator.alloc = cmp_tasks_test_alloc_ex;
  runner.allocator.realloc = cmp_tasks_test_realloc_ex;
  runner.allocator.free = cmp_tasks_test_free_ex;
  stub.rc_alloc = CMP_OK;
  stub.rc_resolve = CMP_OK;
  stub.rc_unregister = CMP_OK;
  stub.rc_free = CMP_OK;

  handles_vtable.resolve = cmp_tasks_test_resolve_ex;
  handles_vtable.unregister_object = cmp_tasks_test_unregister_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  handle.id = 1u;
  handle.generation = 1u;

  switch (mode) {
  case CMP_TASKS_TEST_THREAD_JOIN_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_thread_join(&runner, handle);
  case CMP_TASKS_TEST_THREAD_JOIN_RESOLVE_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_resolve = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_thread_join(&runner, handle);
  case CMP_TASKS_TEST_THREAD_JOIN_ALREADY_JOINED: /* GCOVR_EXCL_LINE */
    thread = (CMPTaskThread *)cmp_tasks_test_malloc(sizeof(*thread));
    if (thread == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(thread, 0, sizeof(*thread));
    thread->joined = CMP_TRUE;
    stub.ptr = thread;
    rc = cmp_tasks_default_thread_join(&runner, handle);
    free(thread);
    return rc;
  case CMP_TASKS_TEST_THREAD_JOIN_NATIVE_FAIL: /* GCOVR_EXCL_LINE */
    thread = (CMPTaskThread *)cmp_tasks_test_malloc(sizeof(*thread));
    if (thread == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(thread, 0, sizeof(*thread));
    thread->runner = &runner;
    thread->joined = CMP_FALSE;
    thread->header.handle = handle;
    stub.ptr = thread;
    rc = cmp_native_thread_create(&thread->thread, cmp_tasks_test_quick_exit,
                                  NULL); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      free(thread);
      return rc;
    }
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_THREAD_JOIN);
    rc = cmp_tasks_default_thread_join(&runner, handle);
    cmp_tasks_test_clear_fail_point();
    free(thread);
    return rc;
  case CMP_TASKS_TEST_THREAD_JOIN_UNREGISTER_FAIL:
    thread = (CMPTaskThread *)cmp_tasks_test_malloc(sizeof(*thread));
    if (thread == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(thread, 0, sizeof(*thread));
    thread->runner = &runner;
    thread->joined = CMP_FALSE;
    thread->header.handle = handle;
    stub.ptr = thread;
    rc = cmp_native_thread_create(&thread->thread, cmp_tasks_test_quick_exit,
                                  NULL);
    if (rc != CMP_OK) {
      free(thread);
      return rc;
    }
    stub.rc_unregister = CMP_ERR_UNKNOWN;
    rc = cmp_tasks_default_thread_join(&runner, handle);
    free(thread);
    return rc;
  case CMP_TASKS_TEST_THREAD_JOIN_FREE_FAIL: /* GCOVR_EXCL_LINE */
    thread = (CMPTaskThread *)cmp_tasks_test_malloc(sizeof(*thread));
    if (thread == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(thread, 0, sizeof(*thread));
    thread->runner = &runner;
    thread->joined = CMP_FALSE;
    thread->header.handle = handle;
    stub.ptr = thread;
    rc = cmp_native_thread_create(&thread->thread, cmp_tasks_test_quick_exit,
                                  NULL); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      free(thread);
      return rc;
    }
    stub.rc_free = CMP_ERR_OUT_OF_MEMORY;
    rc = cmp_tasks_default_thread_join(&runner, handle);
    return rc;
  default:
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_mutex_create_case(cmp_u32 mode) {
  CMPTasksDefault runner;
  CMPHandleSystemVTable handles_vtable; /* GCOVR_EXCL_LINE */
  CMPTasksTestStub stub;                /* GCOVR_EXCL_LINE */
  CMPHandle handle;                     /* GCOVR_EXCL_LINE */
  int rc;

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));

  runner.initialized = CMP_TRUE;
  runner.allocator.ctx = &stub;
  runner.allocator.alloc = cmp_tasks_test_alloc_ex;
  runner.allocator.realloc = cmp_tasks_test_realloc_ex;
  runner.allocator.free = cmp_tasks_test_free_ex;
  stub.rc_alloc = CMP_OK;
  stub.rc_register = CMP_OK;
  stub.rc_free = CMP_OK;

  handles_vtable.register_object = cmp_tasks_test_register_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  switch (mode) {
  case CMP_TASKS_TEST_MUTEX_CREATE_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_create(&runner, &handle);
  case CMP_TASKS_TEST_MUTEX_CREATE_ALLOC_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_alloc = CMP_ERR_OUT_OF_MEMORY;
    return cmp_tasks_default_mutex_create(&runner, &handle);
  case CMP_TASKS_TEST_MUTEX_CREATE_OBJECT_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_OBJECT_INIT);
    rc = cmp_tasks_default_mutex_create(&runner, &handle);
    cmp_tasks_test_clear_fail_point();
    return rc;
  case CMP_TASKS_TEST_MUTEX_CREATE_NATIVE_FAIL:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_INIT);
    rc = cmp_tasks_default_mutex_create(&runner, &handle);
    cmp_tasks_test_clear_fail_point();
    return rc;
  case CMP_TASKS_TEST_MUTEX_CREATE_REGISTER_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_register = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_mutex_create(&runner, &handle);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_default_mutex_destroy_case(cmp_u32 mode) {
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable;
  CMPTasksTestStub stub; /* GCOVR_EXCL_LINE */
  CMPTaskMutex *mutex;
  CMPHandle handle; /* GCOVR_EXCL_LINE */
  int rc;

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));

  runner.initialized = CMP_TRUE;
  runner.allocator.ctx = &stub;
  runner.allocator.alloc = cmp_tasks_test_alloc_ex;
  runner.allocator.realloc = cmp_tasks_test_realloc_ex;
  runner.allocator.free = cmp_tasks_test_free_ex;
  stub.rc_alloc = CMP_OK;
  stub.rc_resolve = CMP_OK;
  stub.rc_unregister = CMP_OK;
  stub.rc_free = CMP_OK;

  handles_vtable.resolve = cmp_tasks_test_resolve_ex;
  handles_vtable.unregister_object = cmp_tasks_test_unregister_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  handle.id = 1u;
  handle.generation = 1u;

  switch (mode) {
  case CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_destroy(&runner, handle);
  case CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_RESOLVE_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_resolve = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_mutex_destroy(&runner, handle);
  case CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_NATIVE_FAIL: /* GCOVR_EXCL_LINE */
    mutex = (CMPTaskMutex *)cmp_tasks_test_malloc(sizeof(*mutex));
    if (mutex == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(mutex, 0, sizeof(*mutex));
    rc = cmp_native_mutex_init(&mutex->mutex);
    if (rc != CMP_OK) {
      free(mutex);
      return rc;
    }
    mutex->initialized = CMP_TRUE;
    mutex->runner = &runner;
    mutex->header.handle = handle;
    stub.ptr = mutex;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_DESTROY);
    rc = cmp_tasks_default_mutex_destroy(&runner, handle);
    cmp_tasks_test_clear_fail_point();
    cmp_native_mutex_destroy(&mutex->mutex);
    free(mutex);
    return rc;
  case CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_UNREGISTER_FAIL: /* GCOVR_EXCL_LINE
                                                              */
    mutex = (CMPTaskMutex *)cmp_tasks_test_malloc(sizeof(*mutex));
    if (mutex == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(mutex, 0, sizeof(*mutex));
    mutex->initialized = CMP_FALSE;
    mutex->runner = &runner;
    mutex->header.handle = handle;
    stub.ptr = mutex;
    stub.rc_unregister = CMP_ERR_UNKNOWN;
    rc = cmp_tasks_default_mutex_destroy(&runner, handle);
    free(mutex);
    return rc;
  case CMP_TASKS_TEST_DEFAULT_MUTEX_DESTROY_FREE_FAIL:
    mutex = (CMPTaskMutex *)cmp_tasks_test_malloc(sizeof(*mutex));
    if (mutex == NULL) {
      return CMP_ERR_OUT_OF_MEMORY;
    }
    memset(mutex, 0, sizeof(*mutex));
    mutex->initialized = CMP_FALSE;
    mutex->runner = &runner;
    mutex->header.handle = handle;
    stub.ptr = mutex;
    stub.rc_free = CMP_ERR_OUT_OF_MEMORY;
    rc = cmp_tasks_default_mutex_destroy(&runner, handle);
    return rc;
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_mutex_lock_case(cmp_u32 mode) {
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable;
  CMPTasksTestStub stub;
  CMPTaskMutex mutex;
  CMPHandle handle; /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));
  memset(&mutex, 0, sizeof(mutex));

  runner.initialized = CMP_TRUE;
  handles_vtable.resolve = cmp_tasks_test_resolve_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  handle.id = 1u;
  handle.generation = 1u;
  stub.ptr = &mutex;

  switch (mode) {
  case CMP_TASKS_TEST_MUTEX_LOCK_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_lock(&runner, handle);
  case CMP_TASKS_TEST_MUTEX_LOCK_RESOLVE_FAIL: /* GCOVR_EXCL_LINE */
    stub.rc_resolve = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_mutex_lock(&runner, handle);
  case CMP_TASKS_TEST_MUTEX_LOCK_NOT_INITIALIZED:
    mutex.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_lock(&runner, handle);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_mutex_unlock_case(cmp_u32 mode) {
  CMPTasksDefault runner;               /* GCOVR_EXCL_LINE */
  CMPHandleSystemVTable handles_vtable; /* GCOVR_EXCL_LINE */
  CMPTasksTestStub stub;                /* GCOVR_EXCL_LINE */
  CMPTaskMutex mutex;
  CMPHandle handle;

  memset(&runner, 0, sizeof(runner));
  memset(&handles_vtable, 0, sizeof(handles_vtable));
  memset(&stub, 0, sizeof(stub));
  memset(&mutex, 0, sizeof(mutex));

  runner.initialized = CMP_TRUE;
  handles_vtable.resolve = cmp_tasks_test_resolve_ex;
  runner.handles.vtable = &handles_vtable;
  runner.handles.ctx = &stub;

  handle.id = 1u;
  handle.generation = 1u;
  stub.ptr = &mutex;

  switch (mode) {
  case CMP_TASKS_TEST_MUTEX_UNLOCK_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_unlock(&runner, handle);
  case CMP_TASKS_TEST_MUTEX_UNLOCK_RESOLVE_FAIL:
    stub.rc_resolve = CMP_ERR_UNKNOWN;
    return cmp_tasks_default_mutex_unlock(&runner, handle);
  case CMP_TASKS_TEST_MUTEX_UNLOCK_NOT_INITIALIZED: /* GCOVR_EXCL_LINE */
    mutex.initialized = CMP_FALSE;
    return cmp_tasks_default_mutex_unlock(&runner, handle);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_sleep_case(cmp_u32 mode) {
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));

  switch (mode) {
  case CMP_TASKS_TEST_SLEEP_STATE:
    runner.initialized = CMP_FALSE;
    return cmp_tasks_default_sleep_ms(&runner, 1u);
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

int CMP_CALL cmp_tasks_test_destroy_state(void) {
  CMPTasks tasks;         /* GCOVR_EXCL_LINE */
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */

  memset(&tasks, 0, sizeof(tasks));
  memset(&runner, 0, sizeof(runner));
  tasks.ctx = &runner;
  tasks.vtable = (const CMPTasksVTable *)&runner;
  runner.initialized = CMP_FALSE;
  return cmp_tasks_default_destroy(&tasks);
}

int CMP_CALL cmp_tasks_test_stub_exercise(void) {
  CMPTasksTestStub stub; /* GCOVR_EXCL_LINE */
  CMPObjectHeader obj;   /* GCOVR_EXCL_LINE */
  CMPHandle handle;      /* GCOVR_EXCL_LINE */
  void *out_ptr;         /* GCOVR_EXCL_LINE */
  int rc;
  int result;   /* GCOVR_EXCL_LINE */
  int clear_rc; /* GCOVR_EXCL_LINE */

  memset(&stub, 0, sizeof(stub));
  memset(&obj, 0, sizeof(obj));
  out_ptr = NULL;
  handle.id = 1u;
  handle.generation = 1u;
  result = CMP_OK;

  rc = cmp_tasks_test_register_ex(NULL, &obj);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_register_ex(&stub, NULL);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  stub.rc_register = CMP_ERR_UNKNOWN;
  rc = cmp_tasks_test_register_ex(&stub, &obj);
  result = (rc == CMP_ERR_UNKNOWN) ? result : rc;

  stub.rc_register = CMP_OK;
  rc = cmp_tasks_test_register_ex(&stub, &obj);
  result = (rc == CMP_OK) ? result : rc;
  result =
      (obj.handle.id == 1u && obj.handle.generation == 1u && stub.ptr == &obj)
          ? result
          : CMP_ERR_UNKNOWN;

  rc = cmp_tasks_test_resolve_ex(NULL, handle, &out_ptr);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_resolve_ex(&stub, handle, NULL);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;

  stub.rc_resolve = CMP_ERR_UNKNOWN;
  rc = cmp_tasks_test_resolve_ex(&stub, handle, &out_ptr);
  result = (rc == CMP_ERR_UNKNOWN) ? result : rc;

  stub.rc_resolve = CMP_OK;
  stub.ptr = NULL;
  rc = cmp_tasks_test_resolve_ex(&stub, handle, &out_ptr);
  result = (rc == CMP_ERR_NOT_FOUND) ? result : rc;

  stub.ptr = &obj;
  rc = cmp_tasks_test_resolve_ex(&stub, handle, &out_ptr);
  result = (rc == CMP_OK) ? result : rc;
  result = (out_ptr == &obj) ? result : CMP_ERR_UNKNOWN;

  rc = cmp_tasks_test_unregister_ex(NULL, handle);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;

  stub.rc_unregister = CMP_ERR_UNKNOWN;
  rc = cmp_tasks_test_unregister_ex(&stub, handle);
  result = (rc == CMP_ERR_UNKNOWN) ? result : rc;

  stub.rc_unregister = CMP_OK;
  rc = cmp_tasks_test_unregister_ex(&stub, handle);
  result = (rc == CMP_OK) ? result : rc;

  rc = cmp_tasks_test_alloc_ex(NULL, 8u, &out_ptr);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_alloc_ex(&stub, 8u, NULL);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_alloc_ex(&stub, 0u, &out_ptr);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;

  stub.rc_alloc = CMP_ERR_OUT_OF_MEMORY;
  rc = cmp_tasks_test_alloc_ex(&stub, 8u, &out_ptr);
  result = (rc == CMP_ERR_OUT_OF_MEMORY) ? result : rc;
  stub.rc_alloc = CMP_OK;

  rc = cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_ALLOC_EX_NULL);
  result = (rc == CMP_OK) ? result : rc;
  rc = cmp_tasks_test_alloc_ex(&stub, 8u, &out_ptr);
  clear_rc = cmp_tasks_test_clear_fail_point();
  result = (clear_rc == CMP_OK) ? result : clear_rc;
  result = (rc == CMP_ERR_OUT_OF_MEMORY) ? result : rc;

  rc = cmp_tasks_test_alloc_ex(&stub, 8u, &out_ptr);
  result = (rc == CMP_OK) ? result : rc;
  stub.rc_free = CMP_OK;
  if (rc == CMP_OK) {
    rc = cmp_tasks_test_free_ex(&stub, out_ptr);
    result = (rc == CMP_OK) ? result : rc;
  }

  rc = cmp_tasks_test_realloc_ex(NULL, NULL, 8u, &out_ptr);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_realloc_ex(&stub, NULL, 8u, NULL);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;
  rc = cmp_tasks_test_realloc_ex(&stub, NULL, 0u, &out_ptr);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;

  stub.rc_alloc = CMP_ERR_OUT_OF_MEMORY;
  rc = cmp_tasks_test_realloc_ex(&stub, NULL, 8u, &out_ptr);
  result = (rc == CMP_ERR_OUT_OF_MEMORY) ? result : rc;
  stub.rc_alloc = CMP_OK;

  rc = cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_REALLOC_EX_NULL);
  result = (rc == CMP_OK) ? result : rc;
  rc = cmp_tasks_test_realloc_ex(&stub, NULL, 8u, &out_ptr);
  clear_rc = cmp_tasks_test_clear_fail_point();
  result = (clear_rc == CMP_OK) ? result : clear_rc;
  result = (rc == CMP_ERR_OUT_OF_MEMORY) ? result : rc;

  rc = cmp_tasks_test_realloc_ex(&stub, NULL, 8u, &out_ptr);
  result = (rc == CMP_OK) ? result : rc;
  if (rc == CMP_OK) {
    rc = cmp_tasks_test_free_ex(&stub, out_ptr);
    result = (rc == CMP_OK) ? result : rc;
  }

  rc = cmp_tasks_test_free_ex(NULL, NULL);
  result = (rc == CMP_ERR_INVALID_ARGUMENT) ? result : rc;

  stub.rc_free = CMP_ERR_UNKNOWN;
  rc = cmp_tasks_test_free_ex(&stub, NULL);
  result = (rc == CMP_ERR_UNKNOWN) ? result : rc;

  return result;
}

int CMP_CALL cmp_tasks_test_signal_stop_null(void) {
  (void)cmp_tasks_test_signal_stop_entry(NULL);
  return CMP_OK;
}

int CMP_CALL cmp_tasks_test_queue_case(
    cmp_u32 mode, cmp_u32 *out_wait_ms, cmp_u32 *out_due_time,
    cmp_usize *out_count) { /* GCOVR_EXCL_LINE */
  CMPTasksDefault runner;   /* GCOVR_EXCL_LINE */
  CMPTaskItem items[2];     /* GCOVR_EXCL_LINE */
  CMPTaskItem picked;       /* GCOVR_EXCL_LINE */
  cmp_u32 wait_ms;
  int rc; /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(items, 0, sizeof(items));
  memset(&picked, 0, sizeof(picked));

  runner.queue = items;
  runner.queue_capacity = 2;
  runner.queue_count = 0;
  wait_ms = 0;
  rc = CMP_ERR_INVALID_ARGUMENT;

  switch (mode) {
  case CMP_TASKS_TEST_QUEUE_CASE_EMPTY: /* GCOVR_EXCL_LINE */
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_INVALID: /* GCOVR_EXCL_LINE */
    runner.queue_count = 1;
    items[0].due_time_ms = 0;
    rc = cmp_tasks_queue_pick(&runner, NULL, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_NOT_READY: /* GCOVR_EXCL_LINE */
    runner.queue_count = 1;
    items[0].due_time_ms = cmp_u32_max_value();
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_READY_EARLY: /* GCOVR_EXCL_LINE */
    runner.queue_count = 2;
    items[0].due_time_ms = 1000u;
    items[1].due_time_ms = 0u;
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_READY_LATE: /* GCOVR_EXCL_LINE */
    runner.queue_count = 1;
    items[0].due_time_ms = 0u;
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_SWAP: /* GCOVR_EXCL_LINE */
    runner.queue_count = 2;
    items[0].due_time_ms = 0u;
    items[1].due_time_ms = 1000u;
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_TIME_FAIL:
    runner.queue_count = 1;
    items[0].due_time_ms = 0u;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TIME_NOW);
    rc = cmp_tasks_queue_pick(&runner, &picked, &wait_ms);
    cmp_tasks_test_clear_fail_point();
    break;
  case CMP_TASKS_TEST_QUEUE_CASE_BUSY:
    runner.queue_count = runner.queue_capacity;
    rc = cmp_tasks_queue_push(&runner, NULL, NULL, 0u);
    break;
  default: /* GCOVR_EXCL_LINE */
    rc = CMP_ERR_INVALID_ARGUMENT;
    break;
  }

  if (out_wait_ms != NULL) {
    *out_wait_ms = wait_ms;
  }
  if (out_due_time != NULL) {
    *out_due_time = picked.due_time_ms;
  }
  if (out_count != NULL) {
    *out_count = runner.queue_count;
  }

  return rc;
}

int CMP_CALL cmp_tasks_test_post_case(cmp_u32 mode) {
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */
  CMPTaskItem items[1];   /* GCOVR_EXCL_LINE */
  int rc;

  memset(&runner, 0, sizeof(runner));
  memset(items, 0, sizeof(items));

  runner.queue = items;
  runner.queue_capacity = 1;
  runner.queue_count = 0;
  runner.initialized = CMP_TRUE;

  rc = cmp_native_mutex_init(&runner.queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_native_cond_init(&runner.queue_cond);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&runner.queue_mutex);
    return rc;
  }

  switch (mode) {
  case CMP_TASKS_TEST_POST_CASE_LOCK_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_LOCK);
    rc = cmp_tasks_default_task_post(&runner, cmp_tasks_test_task_noop, NULL);
    break;
  case CMP_TASKS_TEST_POST_CASE_SIGNAL_FAIL:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_SIGNAL);
    rc = cmp_tasks_default_task_post(&runner, cmp_tasks_test_task_noop, NULL);
    break;
  case CMP_TASKS_TEST_POST_CASE_UNLOCK_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK);
    rc = cmp_tasks_default_task_post(&runner, cmp_tasks_test_task_noop, NULL);
    break;
  case CMP_TASKS_TEST_POST_CASE_BUSY: /* GCOVR_EXCL_LINE */
    runner.queue_count = runner.queue_capacity;
    rc = cmp_tasks_default_task_post(&runner, cmp_tasks_test_task_noop, NULL);
    break;
  case CMP_TASKS_TEST_POST_CASE_STATE:
    runner.initialized = CMP_FALSE;
    rc = cmp_tasks_default_task_post(&runner, cmp_tasks_test_task_noop, NULL);
    break;
  default:
    rc = CMP_ERR_INVALID_ARGUMENT;
    break;
  }

  cmp_tasks_test_clear_fail_point();
  cmp_native_cond_destroy(&runner.queue_cond);
  cmp_native_mutex_destroy(&runner.queue_mutex);
  return rc;
}

int CMP_CALL cmp_tasks_test_post_delayed_case(cmp_u32 mode) {
  CMPTasksDefault runner;
  CMPTaskItem items[1]; /* GCOVR_EXCL_LINE */
  cmp_u32 max_delay;    /* GCOVR_EXCL_LINE */
  int rc;               /* GCOVR_EXCL_LINE */

  memset(&runner, 0, sizeof(runner));
  memset(items, 0, sizeof(items));

  runner.queue = items;
  runner.queue_capacity = 1;
  runner.queue_count = 0;
  runner.initialized = CMP_TRUE;

  rc = cmp_native_mutex_init(&runner.queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_native_cond_init(&runner.queue_cond);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&runner.queue_mutex);
    return rc;
  }

  switch (mode) {
  case CMP_TASKS_TEST_POST_DELAYED_CASE_TIME_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TIME_NOW);
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u);
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_LOCK_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_LOCK);
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u); /* GCOVR_EXCL_LINE */
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_SIGNAL_FAIL:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_SIGNAL);
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u); /* GCOVR_EXCL_LINE */
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_UNLOCK_FAIL: /* GCOVR_EXCL_LINE */
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK);
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u); /* GCOVR_EXCL_LINE */
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_BUSY: /* GCOVR_EXCL_LINE */
    runner.queue_count = runner.queue_capacity;
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u); /* GCOVR_EXCL_LINE */
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_OVERFLOW:
    max_delay = (cmp_u32) ~(cmp_u32)0;
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, max_delay);
    break;
  case CMP_TASKS_TEST_POST_DELAYED_CASE_STATE: /* GCOVR_EXCL_LINE */
    runner.initialized = CMP_FALSE;
    rc = cmp_tasks_default_task_post_delayed(&runner, cmp_tasks_test_task_noop,
                                             NULL, 1u); /* GCOVR_EXCL_LINE */
    break;
  default:
    rc = CMP_ERR_INVALID_ARGUMENT;
    break;
  }

  cmp_tasks_test_clear_fail_point();
  cmp_native_cond_destroy(&runner.queue_cond);
  cmp_native_mutex_destroy(&runner.queue_mutex);
  return rc;
}

int CMP_CALL cmp_tasks_test_worker_case(cmp_u32 mode) {
  CMPTasksDefault runner; /* GCOVR_EXCL_LINE */
  CMPTaskItem items[1];   /* GCOVR_EXCL_LINE */
  CMPNativeThread thread;
  int rc; /* GCOVR_EXCL_LINE */
  int join_rc;

  memset(&runner, 0, sizeof(runner));
  memset(items, 0, sizeof(items));
  memset(&thread, 0, sizeof(thread));

  runner.queue = items;
  runner.queue_capacity = 1;
  runner.queue_count = 0;
  runner.stopping = CMP_FALSE;

  rc = cmp_native_mutex_init(&runner.queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_native_cond_init(&runner.queue_cond);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&runner.queue_mutex);
    return rc;
  }

  switch (mode) {
  case CMP_TASKS_TEST_WORKER_CASE_LOCK_FAIL:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_LOCK);
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_WAIT_FAIL: /* GCOVR_EXCL_LINE */
    runner.queue_count = 0;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_WAIT);
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_TIMEOUT:
    runner.queue_count = 1;
    items[0].due_time_ms = cmp_u32_max_value();
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT);
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_TASK_ERROR: /* GCOVR_EXCL_LINE */
    runner.queue_count = 1;
    items[0].due_time_ms = 0u;
    items[0].fn = cmp_tasks_test_task_fail;
    items[0].user = &runner;
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_PICK_ERROR:
    runner.queue_count = 1;
    items[0].due_time_ms = 0u;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TIME_NOW);
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_UNLOCK_FAIL: /* GCOVR_EXCL_LINE */
    runner.queue_count = 1;
    items[0].due_time_ms = 0u;
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_MUTEX_UNLOCK);
    (void)cmp_tasks_worker_entry(&runner);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_NULL: /* GCOVR_EXCL_LINE */
    (void)cmp_tasks_worker_entry(NULL);
    break;
  case CMP_TASKS_TEST_WORKER_CASE_TIMEDWAIT_SIGNAL:
    runner.queue_count = 1;
    items[0].due_time_ms = cmp_u32_max_value();
    rc = cmp_native_thread_create(&thread, cmp_tasks_test_signal_stop_entry,
                                  &runner); /* GCOVR_EXCL_LINE */
    if (rc == CMP_OK) {
      (void)cmp_tasks_worker_entry(&runner);
      join_rc = cmp_native_thread_join(&thread);
      if (join_rc != CMP_OK) {
        return join_rc;
      }
    } else {
      return rc;
    }
    break;
  default: /* GCOVR_EXCL_LINE */
    break;
  }

  cmp_tasks_test_clear_fail_point();
  cmp_native_cond_destroy(&runner.queue_cond);
  cmp_native_mutex_destroy(&runner.queue_mutex);
  return CMP_OK;
}

int CMP_CALL cmp_tasks_test_object_op(cmp_u32 mode, cmp_u32 *out_type_id) {
  CMPTaskThread thread;          /* GCOVR_EXCL_LINE */
  CMPObjectVTable bad_vtable;    /* GCOVR_EXCL_LINE */
  const CMPObjectVTable *vtable; /* GCOVR_EXCL_LINE */
  int rc;

  memset(&thread, 0, sizeof(thread));
  vtable = &g_cmp_task_thread_vtable;
  if (mode == CMP_TASKS_TEST_OBJECT_OP_INIT_EARLY_FAIL) {
    memset(&bad_vtable, 0, sizeof(bad_vtable));
    vtable = &bad_vtable;
  }
  rc =
      cmp_object_header_init(&thread.header, CMP_TASK_OBJECT_THREAD, 0, vtable);
  if (rc != CMP_OK) {
    return rc;
  }

  switch (mode) {
  case CMP_TASKS_TEST_OBJECT_OP_RETAIN:
    return cmp_task_object_retain(&thread);
  case CMP_TASKS_TEST_OBJECT_OP_RELEASE: /* GCOVR_EXCL_LINE */
    thread.header.ref_count = 2;
    return cmp_task_object_release(&thread);
  case CMP_TASKS_TEST_OBJECT_OP_GET_TYPE: /* GCOVR_EXCL_LINE */
    if (out_type_id == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    return cmp_task_object_get_type_id(&thread, out_type_id);
  case CMP_TASKS_TEST_OBJECT_OP_RETAIN_NULL: /* GCOVR_EXCL_LINE */
    return cmp_task_object_retain(NULL);
  case CMP_TASKS_TEST_OBJECT_OP_RELEASE_NULL:
    return cmp_task_object_release(NULL);
  case CMP_TASKS_TEST_OBJECT_OP_INIT_FAIL: /* GCOVR_EXCL_LINE */
    memset(&bad_vtable, 0, sizeof(bad_vtable));
    rc = cmp_object_header_init(&thread.header, CMP_TASK_OBJECT_THREAD, 0,
                                &bad_vtable); /* GCOVR_EXCL_LINE */
    return rc;
  default: /* GCOVR_EXCL_LINE */
    return CMP_ERR_INVALID_ARGUMENT;
  }
}

static CMP_TASK_THREAD_RETURN CMP_TASK_THREAD_CALL /* GCOVR_EXCL_LINE */
cmp_tasks_test_signal_entry(void *user) {
  CMPTasksDefault *runner; /* GCOVR_EXCL_LINE */

  runner = (CMPTasksDefault *)user;
  if (runner == NULL) {
#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
    return 0;
#else  /* GCOVR_EXCL_LINE */
    return NULL;
#endif /* GCOVR_EXCL_LINE */
  }

  cmp_native_mutex_lock(&runner->queue_mutex);
  cmp_native_cond_signal(&runner->queue_cond);
  cmp_native_mutex_unlock(&runner->queue_mutex);

#if defined(CMP_TASKS_USE_WIN32) /* GCOVR_EXCL_LINE */
  return 0;                      /* GCOVR_EXCL_LINE */
#else                            /* GCOVR_EXCL_LINE */
  return NULL;
#endif                           /* GCOVR_EXCL_LINE */
}

int CMP_CALL cmp_tasks_test_timedwait_case(cmp_u32 mode) {
  CMPTasksDefault runner;
  CMPNativeThread thread;
  int rc;
  int join_rc;
  int i;

  memset(&runner, 0, sizeof(runner));
  rc = cmp_native_mutex_init(&runner.queue_mutex);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_native_cond_init(&runner.queue_cond);
  if (rc != CMP_OK) {
    cmp_native_mutex_destroy(&runner.queue_mutex);
    return rc;
  }

  rc = cmp_native_mutex_lock(&runner.queue_mutex);
  if (rc != CMP_OK) {
    cmp_native_cond_destroy(&runner.queue_cond);
    cmp_native_mutex_destroy(&runner.queue_mutex);
    return rc;
  }

  switch (mode) {
  case CMP_TASKS_TEST_TIMEDWAIT_TIMEOUT:
    rc = cmp_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 0u);
    break;
  case CMP_TASKS_TEST_TIMEDWAIT_ERROR:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR);
    rc = cmp_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 1u);
    break;
  case CMP_TASKS_TEST_TIMEDWAIT_TIME_FAIL:
    cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_TIME_NOW);
    rc = cmp_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 1u);
    break;
  case CMP_TASKS_TEST_TIMEDWAIT_OK:
    rc =
        cmp_native_thread_create(&thread, cmp_tasks_test_signal_entry, &runner);
    if (rc != CMP_OK) {
      break;
    }
    rc = cmp_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex,
                                   100u);
    join_rc = cmp_native_thread_join(&thread);
    if (rc == CMP_OK && join_rc != CMP_OK) {
      rc = join_rc;
    }
    break;
  case CMP_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST:
#if defined(CMP_TASKS_USE_WIN32)
    g_cmp_tasks_test_nsec_adjusted = 1;
    rc = CMP_OK;
    (void)i;
#else
    g_cmp_tasks_test_nsec_adjusted = 0;
    for (i = 0; i < 2000 && g_cmp_tasks_test_nsec_adjusted == 0; ++i) {
      if (g_cmp_tasks_test_fail_point !=
          CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_INVALID) {
        cmp_tasks_test_set_fail_point(CMP_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR);
      }
      rc = cmp_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex,
                                     999u);
      if (rc != CMP_ERR_UNKNOWN && rc != CMP_ERR_TIMEOUT && rc != CMP_OK) {
        break;
      }
    }
    if (g_cmp_tasks_test_nsec_adjusted == 0) {
      rc = CMP_ERR_UNKNOWN;
    } else {
      rc = CMP_OK;
    }
#endif
    break;
  case CMP_TASKS_TEST_TIMEDWAIT_SIGNAL_NULL:
    (void)cmp_tasks_test_signal_entry(NULL);
    rc = CMP_OK;
    break;
  default:
    rc = CMP_ERR_INVALID_ARGUMENT;
    break;
  }

  cmp_tasks_test_clear_fail_point();
  cmp_native_mutex_unlock(&runner.queue_mutex);
  cmp_native_cond_destroy(&runner.queue_cond);
  cmp_native_mutex_destroy(&runner.queue_mutex);
  return rc;
}

int CMP_CALL cmp_tasks_test_call_noop(void) {
  return cmp_tasks_test_task_noop(NULL);
}
#endif
