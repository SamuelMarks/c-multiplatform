#if !defined(_WIN32) && !defined(_WIN64)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "m3/m3_tasks.h"

#include "m3/m3_object.h"

#include <string.h>

#ifdef M3_TESTING
static m3_u32 g_m3_tasks_test_fail_point = M3_TASKS_TEST_FAIL_NONE;
static int g_m3_tasks_test_nsec_adjusted = 0;

static int m3_tasks_test_consume_fail(m3_u32 point)
{
    if (g_m3_tasks_test_fail_point == point) {
        g_m3_tasks_test_fail_point = M3_TASKS_TEST_FAIL_NONE;
        return 1;
    }
    return 0;
}

int M3_CALL m3_tasks_test_set_fail_point(m3_u32 point)
{
    g_m3_tasks_test_fail_point = point;
    return M3_OK;
}

int M3_CALL m3_tasks_test_clear_fail_point(void)
{
    g_m3_tasks_test_fail_point = M3_TASKS_TEST_FAIL_NONE;
    return M3_OK;
}

typedef struct M3TasksTestStub {
    int rc;
} M3TasksTestStub;

static int M3_CALL m3_tasks_test_unregister(void *ctx, M3Handle handle)
{
    M3TasksTestStub *stub;

    (void)handle;
    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    stub = (M3TasksTestStub *)ctx;
    return stub->rc;
}

static int M3_CALL m3_tasks_test_free(void *ctx, void *ptr)
{
    M3TasksTestStub *stub;

    (void)ptr;
    if (ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    stub = (M3TasksTestStub *)ctx;
    return stub->rc;
}
#endif

#if defined(_WIN32) || defined(_WIN64)
#define M3_TASKS_USE_WIN32 1
#include <windows.h>
#else
#define M3_TASKS_USE_PTHREAD 1
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#endif

#define M3_TASKS_DEFAULT_WORKERS 1
#define M3_TASKS_DEFAULT_QUEUE_CAPACITY 64
#define M3_TASKS_DEFAULT_HANDLE_CAPACITY 64

#define M3_TASK_OBJECT_THREAD 1
#define M3_TASK_OBJECT_MUTEX 2

#if defined(M3_TASKS_USE_WIN32)
typedef struct M3NativeMutex {
    CRITICAL_SECTION cs;
} M3NativeMutex;

typedef struct M3NativeCond {
    CONDITION_VARIABLE cond;
} M3NativeCond;

typedef struct M3NativeThread {
    HANDLE handle;
    DWORD id;
} M3NativeThread;
#else
typedef struct M3NativeMutex {
    pthread_mutex_t mutex;
} M3NativeMutex;

typedef struct M3NativeCond {
    pthread_cond_t cond;
} M3NativeCond;

typedef struct M3NativeThread {
    pthread_t thread;
} M3NativeThread;
#endif

#if defined(M3_TASKS_USE_PTHREAD)
static int m3_tasks_pthread_mutex_init(pthread_mutex_t *mutex)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_MUTEX_INIT)) {
        return 1;
    }
#endif
    return pthread_mutex_init(mutex, NULL);
}

static int m3_tasks_pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    int rc;

    rc = pthread_mutex_destroy(mutex);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_MUTEX_DESTROY)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_pthread_mutex_lock(pthread_mutex_t *mutex)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_MUTEX_LOCK)) {
        return 1;
    }
#endif
    return pthread_mutex_lock(mutex);
}

static int m3_tasks_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int rc;

    rc = pthread_mutex_unlock(mutex);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_MUTEX_UNLOCK)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_pthread_cond_init(pthread_cond_t *cond)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_INIT)) {
        return 1;
    }
#endif
    return pthread_cond_init(cond, NULL);
}

static int m3_tasks_pthread_cond_destroy(pthread_cond_t *cond)
{
    int rc;

    rc = pthread_cond_destroy(cond);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_DESTROY)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_pthread_cond_signal(pthread_cond_t *cond)
{
    int rc;

    rc = pthread_cond_signal(cond);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_SIGNAL)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_pthread_cond_broadcast(pthread_cond_t *cond)
{
    int rc;

    rc = pthread_cond_broadcast(cond);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_BROADCAST)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_WAIT)) {
        return EINVAL;
    }
#endif
    return pthread_cond_wait(cond, mutex);
}

static int m3_tasks_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *ts)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT)) {
        g_m3_tasks_test_fail_point = M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR;
        return ETIMEDOUT;
    }
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR)) {
        return EINVAL;
    }
#endif
    return pthread_cond_timedwait(cond, mutex, ts);
}

static int m3_tasks_pthread_create(pthread_t *thread, void *(*entry)(void *), void *user)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_THREAD_CREATE)) {
        return 1;
    }
#endif
    return pthread_create(thread, NULL, entry, user);
}

static int m3_tasks_pthread_join(pthread_t thread)
{
    int rc;

    rc = pthread_join(thread, NULL);
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_THREAD_JOIN)) {
        return 1;
    }
#endif
    return rc;
}

static int m3_tasks_gettimeofday(struct timeval *tv)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_TIME_NOW)) {
        return -1;
    }
#endif
    return gettimeofday(tv, NULL);
}

#if defined(M3_TASKS_USE_PTHREAD)
static int m3_tasks_nanosleep(struct timespec *req_spec);
#endif

static int m3_tasks_usleep(m3_u32 usec)
{
#if defined(M3_TASKS_USE_PTHREAD)
    struct timespec req;
#endif
#if defined(M3_TASKS_USE_PTHREAD)
    {
        req.tv_sec = (time_t)(usec / 1000000u);
        req.tv_nsec = (long)((usec % 1000000u) * 1000u);

        while (m3_tasks_nanosleep(&req) != 0) {
            if (errno != EINTR) {
                return -1;
            }
        }
        return 0;
    }
#else
    return usleep((unsigned int)usec);
#endif
}

#if defined(M3_TASKS_USE_PTHREAD)
static int m3_tasks_nanosleep(struct timespec *req_spec)
{
#ifdef M3_TESTING
    if (m3_tasks_test_consume_fail(M3_TASKS_TEST_FAIL_SLEEP)) {
        errno = EINVAL;
        return -1;
    }
#endif
    return nanosleep(req_spec, req_spec);
}
#endif
#endif

typedef struct M3TaskItem {
    M3TaskFn fn;
    void *user;
    m3_u32 due_time_ms;
} M3TaskItem;

typedef struct M3TasksDefault M3TasksDefault;

typedef struct M3TaskWorker {
    M3NativeThread thread;
    M3TasksDefault *runner;
    M3Bool started;
} M3TaskWorker;

typedef struct M3TaskThread {
    M3ObjectHeader header;
    M3TasksDefault *runner;
    M3NativeThread thread;
    M3ThreadFn entry;
    void *user;
    M3Bool joined;
    int result;
} M3TaskThread;

typedef struct M3TaskMutex {
    M3ObjectHeader header;
    M3TasksDefault *runner;
    M3NativeMutex mutex;
    M3Bool initialized;
} M3TaskMutex;

struct M3TasksDefault {
    M3Allocator allocator;
    M3HandleSystem handles;
    M3Tasks tasks;
    M3TaskItem *queue;
    m3_usize queue_capacity;
    m3_usize queue_count;
    M3NativeMutex queue_mutex;
    M3NativeCond queue_cond;
    M3TaskWorker *workers;
    m3_usize worker_count;
    m3_usize live_threads;
    m3_usize live_mutexes;
    M3Bool stopping;
    M3Bool initialized;
    int last_task_error;
};

static m3_u32 m3_u32_max_value(void)
{
    return (m3_u32)~(m3_u32)0;
}

static m3_usize m3_usize_max_value(void)
{
    return (m3_usize)~(m3_usize)0;
}

static int m3_tasks_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = m3_usize_max_value();
    if (a != 0 && b > max_value / a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return M3_OK;
}
static int m3_tasks_time_now_ms(m3_u32 *out_ms)
{
#if defined(M3_TASKS_USE_WIN32)
    if (out_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_ms = (m3_u32)GetTickCount();
    return M3_OK;
#else
    struct timeval tv;
    m3_u32 sec;
    m3_u32 usec;

    if (out_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (m3_tasks_gettimeofday(&tv) != 0) {
        return M3_ERR_UNKNOWN;
    }
    sec = (m3_u32)tv.tv_sec;
    usec = (m3_u32)tv.tv_usec;
    *out_ms = sec * 1000u + usec / 1000u;
    return M3_OK;
#endif
}

static int m3_native_mutex_init(M3NativeMutex *mutex)
{
    if (mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    InitializeCriticalSection(&mutex->cs);
    return M3_OK;
#else
    if (m3_tasks_pthread_mutex_init(&mutex->mutex) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_mutex_destroy(M3NativeMutex *mutex)
{
    if (mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    DeleteCriticalSection(&mutex->cs);
    return M3_OK;
#else
    if (m3_tasks_pthread_mutex_destroy(&mutex->mutex) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_mutex_lock(M3NativeMutex *mutex)
{
    if (mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    EnterCriticalSection(&mutex->cs);
    return M3_OK;
#else
    if (m3_tasks_pthread_mutex_lock(&mutex->mutex) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_mutex_unlock(M3NativeMutex *mutex)
{
    if (mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    LeaveCriticalSection(&mutex->cs);
    return M3_OK;
#else
    if (m3_tasks_pthread_mutex_unlock(&mutex->mutex) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_init(M3NativeCond *cond)
{
    if (cond == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    InitializeConditionVariable(&cond->cond);
    return M3_OK;
#else
    if (m3_tasks_pthread_cond_init(&cond->cond) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_destroy(M3NativeCond *cond)
{
    if (cond == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    return M3_OK;
#else
    if (m3_tasks_pthread_cond_destroy(&cond->cond) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_signal(M3NativeCond *cond)
{
    if (cond == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    WakeConditionVariable(&cond->cond);
    return M3_OK;
#else
    if (m3_tasks_pthread_cond_signal(&cond->cond) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_broadcast(M3NativeCond *cond)
{
    if (cond == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    WakeAllConditionVariable(&cond->cond);
    return M3_OK;
#else
    if (m3_tasks_pthread_cond_broadcast(&cond->cond) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_wait(M3NativeCond *cond, M3NativeMutex *mutex)
{
    if (cond == NULL || mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    if (!SleepConditionVariableCS(&cond->cond, &mutex->cs, INFINITE)) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#else
    if (m3_tasks_pthread_cond_wait(&cond->cond, &mutex->mutex) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_cond_timedwait(M3NativeCond *cond, M3NativeMutex *mutex, m3_u32 wait_ms)
{
    if (cond == NULL || mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    if (!SleepConditionVariableCS(&cond->cond, &mutex->cs, (DWORD)wait_ms)) {
        if (GetLastError() == ERROR_TIMEOUT) {
            return M3_ERR_TIMEOUT;
        }
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#else
    {
        struct timeval tv;
        struct timespec ts;
        m3_u32 ms_part;
        long nsec;
        time_t sec;
        int wait_rc;

        if (m3_tasks_gettimeofday(&tv) != 0) {
            return M3_ERR_UNKNOWN;
        }

        ms_part = wait_ms % 1000u;
        sec = tv.tv_sec + (time_t)(wait_ms / 1000u);
        nsec = (long)tv.tv_usec * 1000L + (long)ms_part * 1000000L;
        if (nsec >= 1000000000L) {
#ifdef M3_TESTING
            g_m3_tasks_test_nsec_adjusted = 1;
#endif
            sec += 1;
            nsec -= 1000000000L;
        }
        ts.tv_sec = sec;
        ts.tv_nsec = nsec;

        wait_rc = m3_tasks_pthread_cond_timedwait(&cond->cond, &mutex->mutex, &ts);
        if (wait_rc == 0) {
            return M3_OK;
        }
        if (wait_rc == ETIMEDOUT) {
            return M3_ERR_TIMEOUT;
        }
        return M3_ERR_UNKNOWN;
    }
#endif
}

static int m3_native_sleep_ms(m3_u32 ms)
{
#if defined(M3_TASKS_USE_WIN32)
    Sleep(ms);
    return M3_OK;
#else
    if (m3_tasks_usleep(ms * 1000u) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

#if defined(M3_TASKS_USE_WIN32)
#define M3_TASK_THREAD_RETURN DWORD
#define M3_TASK_THREAD_CALL WINAPI
#else
#define M3_TASK_THREAD_RETURN void *
#define M3_TASK_THREAD_CALL
#endif

static int m3_native_thread_create(M3NativeThread *thread, M3_TASK_THREAD_RETURN(M3_TASK_THREAD_CALL *entry)(void *), void *user)
{
    if (thread == NULL || entry == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    thread->handle = CreateThread(NULL, 0, entry, user, 0, &thread->id);
    if (thread->handle == NULL) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#else
    if (m3_tasks_pthread_create(&thread->thread, entry, user) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_native_thread_join(M3NativeThread *thread)
{
    if (thread == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_TASKS_USE_WIN32)
    if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) {
        return M3_ERR_UNKNOWN;
    }
    CloseHandle(thread->handle);
    thread->handle = NULL;
    return M3_OK;
#else
    if (m3_tasks_pthread_join(thread->thread) != 0) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
#endif
}

static int m3_task_object_retain(void *obj)
{
    return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_task_object_release(void *obj)
{
    return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_task_thread_destroy(void *obj)
{
    M3TaskThread *thread;
    M3TasksDefault *runner;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    thread = (M3TaskThread *)obj;
    if (!thread->joined) {
        return M3_ERR_STATE;
    }

    runner = thread->runner;
    if (runner != NULL && runner->handles.vtable != NULL) {
        M3Handle handle = thread->header.handle;
        if (handle.id != 0 && handle.generation != 0) {
            rc = runner->handles.vtable->unregister_object(runner->handles.ctx, handle);
            if (rc != M3_OK) {
                return rc;
            }
        }
    }

    if (runner != NULL) {
        rc = runner->allocator.free(runner->allocator.ctx, thread);
        if (rc != M3_OK) {
            return rc;
        }
    }

    return M3_OK;
}

static int m3_task_mutex_destroy(void *obj)
{
    M3TaskMutex *mutex;
    M3TasksDefault *runner;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    mutex = (M3TaskMutex *)obj;
    if (mutex->initialized) {
        rc = m3_native_mutex_destroy(&mutex->mutex);
        if (rc != M3_OK) {
            return rc;
        }
        mutex->initialized = M3_FALSE;
    }

    runner = mutex->runner;
    if (runner != NULL && runner->handles.vtable != NULL) {
        M3Handle handle = mutex->header.handle;
        if (handle.id != 0 && handle.generation != 0) {
            rc = runner->handles.vtable->unregister_object(runner->handles.ctx, handle);
            if (rc != M3_OK) {
                return rc;
            }
        }
    }

    if (runner != NULL) {
        rc = runner->allocator.free(runner->allocator.ctx, mutex);
        if (rc != M3_OK) {
            return rc;
        }
    }

    return M3_OK;
}

static int m3_task_object_get_type_id(void *obj, m3_u32 *out_type_id)
{
    return m3_object_get_type_id((M3ObjectHeader *)obj, out_type_id);
}

static const M3ObjectVTable g_m3_task_thread_vtable = {
    m3_task_object_retain,
    m3_task_object_release,
    m3_task_thread_destroy,
    m3_task_object_get_type_id
};

static const M3ObjectVTable g_m3_task_mutex_vtable = {
    m3_task_object_retain,
    m3_task_object_release,
    m3_task_mutex_destroy,
    m3_task_object_get_type_id
};

static int m3_tasks_queue_push(M3TasksDefault *runner, M3TaskFn fn, void *user, m3_u32 due_time_ms)
{
    if (runner->queue_count >= runner->queue_capacity) {
        return M3_ERR_BUSY;
    }
    runner->queue[runner->queue_count].fn = fn;
    runner->queue[runner->queue_count].user = user;
    runner->queue[runner->queue_count].due_time_ms = due_time_ms;
    runner->queue_count += 1;
    return M3_OK;
}

static int m3_tasks_queue_pick(M3TasksDefault *runner, M3TaskItem *out_task, m3_u32 *out_wait_ms)
{
    m3_u32 now;
    m3_u32 earliest_due;
    m3_usize earliest_index;
    m3_usize i;
    int rc;

    if (runner->queue_count == 0) {
        return M3_ERR_NOT_FOUND;
    }
    if (out_task == NULL || out_wait_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    earliest_index = 0;
    earliest_due = runner->queue[0].due_time_ms;
    for (i = 1; i < runner->queue_count; ++i) {
        if (runner->queue[i].due_time_ms < earliest_due) {
            earliest_due = runner->queue[i].due_time_ms;
            earliest_index = i;
        }
    }

    rc = m3_tasks_time_now_ms(&now);
    if (rc != M3_OK) {
        return rc;
    }

    if (now < earliest_due) {
        *out_wait_ms = earliest_due - now;
        return M3_ERR_NOT_READY;
    }

    *out_task = runner->queue[earliest_index];
    runner->queue_count -= 1;
    if (earliest_index != runner->queue_count) {
        runner->queue[earliest_index] = runner->queue[runner->queue_count];
    }
    *out_wait_ms = 0;
    return M3_OK;
}

static M3_TASK_THREAD_RETURN M3_TASK_THREAD_CALL m3_tasks_worker_entry(void *user)
{
    M3TasksDefault *runner;
    int rc;
    int task_rc;

    runner = (M3TasksDefault *)user;
    if (runner == NULL) {
#if defined(M3_TASKS_USE_WIN32)
        return 0;
#else
        return NULL;
#endif
    }

    for (;;) {
        M3TaskItem task;
        m3_u32 wait_ms;

        rc = m3_native_mutex_lock(&runner->queue_mutex);
        if (rc != M3_OK) {
            break;
        }

        while (!runner->stopping) {
            rc = m3_tasks_queue_pick(runner, &task, &wait_ms);
            if (rc == M3_OK) {
                break;
            }
            if (rc == M3_ERR_NOT_FOUND) {
                rc = m3_native_cond_wait(&runner->queue_cond, &runner->queue_mutex);
                if (rc != M3_OK) {
                    runner->stopping = M3_TRUE;
                    break;
                }
                continue;
            }
            if (rc == M3_ERR_NOT_READY) {
                rc = m3_native_cond_timedwait(&runner->queue_cond, &runner->queue_mutex, wait_ms);
                if (rc == M3_ERR_TIMEOUT) {
                    continue;
                }
                if (rc != M3_OK) {
                    runner->stopping = M3_TRUE;
                    break;
                }
                continue;
            }
            runner->stopping = M3_TRUE;
            break;
        }

        if (runner->stopping) {
            m3_native_mutex_unlock(&runner->queue_mutex);
            break;
        }

        rc = m3_native_mutex_unlock(&runner->queue_mutex);
        if (rc != M3_OK) {
            break;
        }

        if (task.fn != NULL) {
            task_rc = task.fn(task.user);
            if (task_rc != M3_OK) {
                rc = m3_native_mutex_lock(&runner->queue_mutex);
                if (rc == M3_OK) {
                    runner->last_task_error = task_rc;
                    m3_native_mutex_unlock(&runner->queue_mutex);
                }
            }
        }
    }

#if defined(M3_TASKS_USE_WIN32)
    return 0;
#else
    return NULL;
#endif
}

static M3_TASK_THREAD_RETURN M3_TASK_THREAD_CALL m3_tasks_thread_entry(void *user)
{
    M3TaskThread *thread;

    thread = (M3TaskThread *)user;
    if (thread == NULL || thread->entry == NULL) {
#if defined(M3_TASKS_USE_WIN32)
        return 0;
#else
        return NULL;
#endif
    }

    thread->result = thread->entry(thread->user);

#if defined(M3_TASKS_USE_WIN32)
    return 0;
#else
    return NULL;
#endif
}

static int m3_tasks_default_thread_create(void *tasks, M3ThreadFn entry, void *user, M3Handle *out_thread)
{
    M3TasksDefault *runner;
    M3TaskThread *thread;
    int rc;

    if (tasks == NULL || entry == NULL || out_thread == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    out_thread->id = 0;
    out_thread->generation = 0;

    rc = runner->allocator.alloc(runner->allocator.ctx, sizeof(*thread), (void **)&thread);
    if (rc != M3_OK) {
        return rc;
    }
    memset(thread, 0, sizeof(*thread));

    rc = m3_object_header_init(&thread->header, M3_TASK_OBJECT_THREAD, 0, &g_m3_task_thread_vtable);
    if (rc != M3_OK) {
        runner->allocator.free(runner->allocator.ctx, thread);
        return rc;
    }

    thread->runner = runner;
    thread->entry = entry;
    thread->user = user;
    thread->joined = M3_FALSE;
    thread->result = M3_OK;

    rc = m3_native_thread_create(&thread->thread, m3_tasks_thread_entry, thread);
    if (rc != M3_OK) {
        runner->allocator.free(runner->allocator.ctx, thread);
        return rc;
    }

    rc = runner->handles.vtable->register_object(runner->handles.ctx, &thread->header);
    if (rc != M3_OK) {
        m3_native_thread_join(&thread->thread);
        runner->allocator.free(runner->allocator.ctx, thread);
        return rc;
    }

    runner->live_threads += 1;
    *out_thread = thread->header.handle;
    return M3_OK;
}

static int m3_tasks_default_thread_join(void *tasks, M3Handle thread_handle)
{
    M3TasksDefault *runner;
    M3TaskThread *thread;
    void *resolved;
    int rc;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = runner->handles.vtable->resolve(runner->handles.ctx, thread_handle, &resolved);
    if (rc != M3_OK) {
        return rc;
    }

    thread = (M3TaskThread *)resolved;
    if (thread->joined) {
        return M3_ERR_STATE;
    }

    rc = m3_native_thread_join(&thread->thread);
    if (rc != M3_OK) {
        return rc;
    }
    thread->joined = M3_TRUE;

    rc = runner->handles.vtable->unregister_object(runner->handles.ctx, thread_handle);
    if (rc != M3_OK) {
        return rc;
    }

    rc = runner->allocator.free(runner->allocator.ctx, thread);
    if (rc != M3_OK) {
        return rc;
    }

    if (runner->live_threads > 0) {
        runner->live_threads -= 1;
    }

    return M3_OK;
}

static int m3_tasks_default_mutex_create(void *tasks, M3Handle *out_mutex)
{
    M3TasksDefault *runner;
    M3TaskMutex *mutex;
    int rc;

    if (tasks == NULL || out_mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    out_mutex->id = 0;
    out_mutex->generation = 0;

    rc = runner->allocator.alloc(runner->allocator.ctx, sizeof(*mutex), (void **)&mutex);
    if (rc != M3_OK) {
        return rc;
    }
    memset(mutex, 0, sizeof(*mutex));

    rc = m3_object_header_init(&mutex->header, M3_TASK_OBJECT_MUTEX, 0, &g_m3_task_mutex_vtable);
    if (rc != M3_OK) {
        runner->allocator.free(runner->allocator.ctx, mutex);
        return rc;
    }

    mutex->runner = runner;
    mutex->initialized = M3_FALSE;

    rc = m3_native_mutex_init(&mutex->mutex);
    if (rc != M3_OK) {
        runner->allocator.free(runner->allocator.ctx, mutex);
        return rc;
    }
    mutex->initialized = M3_TRUE;

    rc = runner->handles.vtable->register_object(runner->handles.ctx, &mutex->header);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&mutex->mutex);
        runner->allocator.free(runner->allocator.ctx, mutex);
        return rc;
    }

    runner->live_mutexes += 1;
    *out_mutex = mutex->header.handle;
    return M3_OK;
}

static int m3_tasks_default_mutex_destroy(void *tasks, M3Handle mutex_handle)
{
    M3TasksDefault *runner;
    M3TaskMutex *mutex;
    void *resolved;
    int rc;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle, &resolved);
    if (rc != M3_OK) {
        return rc;
    }

    mutex = (M3TaskMutex *)resolved;
    if (mutex->initialized) {
        rc = m3_native_mutex_destroy(&mutex->mutex);
        if (rc != M3_OK) {
            return rc;
        }
        mutex->initialized = M3_FALSE;
    }

    rc = runner->handles.vtable->unregister_object(runner->handles.ctx, mutex_handle);
    if (rc != M3_OK) {
        return rc;
    }

    rc = runner->allocator.free(runner->allocator.ctx, mutex);
    if (rc != M3_OK) {
        return rc;
    }

    if (runner->live_mutexes > 0) {
        runner->live_mutexes -= 1;
    }

    return M3_OK;
}

static int m3_tasks_default_mutex_lock(void *tasks, M3Handle mutex_handle)
{
    M3TasksDefault *runner;
    M3TaskMutex *mutex;
    void *resolved;
    int rc;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle, &resolved);
    if (rc != M3_OK) {
        return rc;
    }

    mutex = (M3TaskMutex *)resolved;
    if (!mutex->initialized) {
        return M3_ERR_STATE;
    }

    return m3_native_mutex_lock(&mutex->mutex);
}

static int m3_tasks_default_mutex_unlock(void *tasks, M3Handle mutex_handle)
{
    M3TasksDefault *runner;
    M3TaskMutex *mutex;
    void *resolved;
    int rc;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = runner->handles.vtable->resolve(runner->handles.ctx, mutex_handle, &resolved);
    if (rc != M3_OK) {
        return rc;
    }

    mutex = (M3TaskMutex *)resolved;
    if (!mutex->initialized) {
        return M3_ERR_STATE;
    }

    return m3_native_mutex_unlock(&mutex->mutex);
}

static int m3_tasks_default_sleep_ms(void *tasks, m3_u32 ms)
{
    M3TasksDefault *runner;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }
    return m3_native_sleep_ms(ms);
}

static int m3_tasks_default_task_post(void *tasks, M3TaskFn fn, void *user)
{
    M3TasksDefault *runner;
    int rc;
    int unlock_rc;

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = m3_native_mutex_lock(&runner->queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_tasks_queue_push(runner, fn, user, 0);
    if (rc == M3_OK) {
        rc = m3_native_cond_signal(&runner->queue_cond);
    }

    unlock_rc = m3_native_mutex_unlock(&runner->queue_mutex);
    if (rc == M3_OK && unlock_rc != M3_OK) {
        rc = unlock_rc;
    }

    return rc;
}

static int m3_tasks_default_task_post_delayed(void *tasks, M3TaskFn fn, void *user, m3_u32 delay_ms)
{
    M3TasksDefault *runner;
    m3_u32 now;
    m3_u32 due_time;
    int rc;
    int unlock_rc;

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    runner = (M3TasksDefault *)tasks;
    if (!runner->initialized) {
        return M3_ERR_STATE;
    }

    rc = m3_tasks_time_now_ms(&now);
    if (rc != M3_OK) {
        return rc;
    }

    if (delay_ms > m3_u32_max_value() - now) {
        return M3_ERR_OVERFLOW;
    }
    due_time = now + delay_ms;

    rc = m3_native_mutex_lock(&runner->queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }

    rc = m3_tasks_queue_push(runner, fn, user, due_time);
    if (rc == M3_OK) {
        rc = m3_native_cond_signal(&runner->queue_cond);
    }

    unlock_rc = m3_native_mutex_unlock(&runner->queue_mutex);
    if (rc == M3_OK && unlock_rc != M3_OK) {
        rc = unlock_rc;
    }

    return rc;
}

static const M3TasksVTable g_m3_tasks_default_vtable = {
    m3_tasks_default_thread_create,
    m3_tasks_default_thread_join,
    m3_tasks_default_mutex_create,
    m3_tasks_default_mutex_destroy,
    m3_tasks_default_mutex_lock,
    m3_tasks_default_mutex_unlock,
    m3_tasks_default_sleep_ms,
    m3_tasks_default_task_post,
    m3_tasks_default_task_post_delayed
};

int M3_CALL m3_tasks_default_config_init(M3TasksDefaultConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->worker_count = M3_TASKS_DEFAULT_WORKERS;
    config->queue_capacity = M3_TASKS_DEFAULT_QUEUE_CAPACITY;
    config->handle_capacity = M3_TASKS_DEFAULT_HANDLE_CAPACITY;
    return M3_OK;
}

int M3_CALL m3_tasks_default_create(const M3TasksDefaultConfig *config, M3Tasks *out_tasks)
{
    M3TasksDefaultConfig cfg;
    M3TasksDefault *runner;
    M3Allocator allocator;
    m3_usize worker_count;
    m3_usize queue_capacity;
    m3_usize handle_capacity;
    m3_usize i;
    int rc;
    int first_error;

    if (out_tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    out_tasks->ctx = NULL;
    out_tasks->vtable = NULL;

    if (config == NULL) {
        rc = m3_tasks_default_config_init(&cfg);
        if (rc != M3_OK) {
            return rc;
        }
    } else {
        cfg = *config;
    }

    worker_count = cfg.worker_count;
    queue_capacity = cfg.queue_capacity;
    handle_capacity = cfg.handle_capacity;

    if (worker_count == 0 || queue_capacity == 0 || handle_capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (cfg.allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        if (rc != M3_OK) {
            return rc;
        }
    } else {
        allocator = *cfg.allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(*runner), (void **)&runner);
    if (rc != M3_OK) {
        return rc;
    }
    memset(runner, 0, sizeof(*runner));

    runner->allocator = allocator;
    runner->queue_capacity = queue_capacity;
    runner->worker_count = worker_count;
    runner->last_task_error = M3_OK;

    rc = m3_handle_system_default_create(handle_capacity, &allocator, &runner->handles);
    if (rc != M3_OK) {
        allocator.free(allocator.ctx, runner);
        return rc;
    }

    rc = m3_native_mutex_init(&runner->queue_mutex);
    if (rc != M3_OK) {
        m3_handle_system_default_destroy(&runner->handles);
        allocator.free(allocator.ctx, runner);
        return rc;
    }

    rc = m3_native_cond_init(&runner->queue_cond);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&runner->queue_mutex);
        m3_handle_system_default_destroy(&runner->handles);
        allocator.free(allocator.ctx, runner);
        return rc;
    }

    {
        m3_usize queue_bytes;

        rc = m3_tasks_mul_overflow(queue_capacity, sizeof(*runner->queue), &queue_bytes);
        if (rc != M3_OK) {
            m3_native_cond_destroy(&runner->queue_cond);
            m3_native_mutex_destroy(&runner->queue_mutex);
            m3_handle_system_default_destroy(&runner->handles);
            allocator.free(allocator.ctx, runner);
            return rc;
        }

        rc = allocator.alloc(allocator.ctx, queue_bytes, (void **)&runner->queue);
    }
    if (rc != M3_OK) {
        m3_native_cond_destroy(&runner->queue_cond);
        m3_native_mutex_destroy(&runner->queue_mutex);
        m3_handle_system_default_destroy(&runner->handles);
        allocator.free(allocator.ctx, runner);
        return rc;
    }

    {
        m3_usize worker_bytes;

        rc = m3_tasks_mul_overflow(worker_count, sizeof(*runner->workers), &worker_bytes);
        if (rc != M3_OK) {
            allocator.free(allocator.ctx, runner->queue);
            m3_native_cond_destroy(&runner->queue_cond);
            m3_native_mutex_destroy(&runner->queue_mutex);
            m3_handle_system_default_destroy(&runner->handles);
            allocator.free(allocator.ctx, runner);
            return rc;
        }

        rc = allocator.alloc(allocator.ctx, worker_bytes, (void **)&runner->workers);
    }
    if (rc != M3_OK) {
        allocator.free(allocator.ctx, runner->queue);
        m3_native_cond_destroy(&runner->queue_cond);
        m3_native_mutex_destroy(&runner->queue_mutex);
        m3_handle_system_default_destroy(&runner->handles);
        allocator.free(allocator.ctx, runner);
        return rc;
    }
    memset(runner->workers, 0, sizeof(*runner->workers) * worker_count);

    runner->stopping = M3_FALSE;
    runner->queue_count = 0;

    first_error = M3_OK;
    for (i = 0; i < worker_count; ++i) {
        runner->workers[i].runner = runner;
        rc = m3_native_thread_create(&runner->workers[i].thread, m3_tasks_worker_entry, runner);
        if (rc != M3_OK) {
            first_error = rc;
            runner->stopping = M3_TRUE;
            break;
        }
        runner->workers[i].started = M3_TRUE;
    }

    if (first_error != M3_OK) {
        m3_native_cond_broadcast(&runner->queue_cond);
        for (i = 0; i < worker_count; ++i) {
            if (runner->workers[i].started) {
                m3_native_thread_join(&runner->workers[i].thread);
            }
        }
        allocator.free(allocator.ctx, runner->workers);
        allocator.free(allocator.ctx, runner->queue);
        m3_native_cond_destroy(&runner->queue_cond);
        m3_native_mutex_destroy(&runner->queue_mutex);
        m3_handle_system_default_destroy(&runner->handles);
        allocator.free(allocator.ctx, runner);
        return first_error;
    }

    runner->tasks.ctx = runner;
    runner->tasks.vtable = &g_m3_tasks_default_vtable;
    runner->initialized = M3_TRUE;

    *out_tasks = runner->tasks;
    return M3_OK;
}

int M3_CALL m3_tasks_default_destroy(M3Tasks *tasks)
{
    M3TasksDefault *runner;
    M3Allocator allocator;
    int rc;
    int first_error;
    m3_usize i;

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (tasks->ctx == NULL || tasks->vtable == NULL) {
        return M3_ERR_STATE;
    }

    runner = (M3TasksDefault *)tasks->ctx;
    if (runner == NULL || !runner->initialized) {
        return M3_ERR_STATE;
    }
    if (runner->live_threads != 0 || runner->live_mutexes != 0) {
        return M3_ERR_BUSY;
    }

    allocator = runner->allocator;
    first_error = M3_OK;

    rc = m3_native_mutex_lock(&runner->queue_mutex);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }
    runner->stopping = M3_TRUE;
    rc = m3_native_cond_broadcast(&runner->queue_cond);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }
    rc = m3_native_mutex_unlock(&runner->queue_mutex);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    for (i = 0; i < runner->worker_count; ++i) {
        if (runner->workers[i].started) {
            rc = m3_native_thread_join(&runner->workers[i].thread);
            if (rc != M3_OK && first_error == M3_OK) {
                first_error = rc;
            }
        }
    }

    rc = m3_native_cond_destroy(&runner->queue_cond);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }
    rc = m3_native_mutex_destroy(&runner->queue_mutex);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    rc = m3_handle_system_default_destroy(&runner->handles);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    rc = allocator.free(allocator.ctx, runner->workers);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }
    rc = allocator.free(allocator.ctx, runner->queue);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    runner->initialized = M3_FALSE;
    tasks->ctx = NULL;
    tasks->vtable = NULL;

    rc = allocator.free(allocator.ctx, runner);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    return first_error;
}

#ifdef M3_TESTING
int M3_CALL m3_tasks_test_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    return m3_tasks_mul_overflow(a, b, out_value);
}

int M3_CALL m3_tasks_test_call_native(m3_u32 op)
{
    switch (op) {
        case M3_TASKS_TEST_NATIVE_TIME_NOW:
            return m3_tasks_time_now_ms(NULL);
        case M3_TASKS_TEST_NATIVE_MUTEX_INIT:
            return m3_native_mutex_init(NULL);
        case M3_TASKS_TEST_NATIVE_MUTEX_DESTROY:
            return m3_native_mutex_destroy(NULL);
        case M3_TASKS_TEST_NATIVE_MUTEX_LOCK:
            return m3_native_mutex_lock(NULL);
        case M3_TASKS_TEST_NATIVE_MUTEX_UNLOCK:
            return m3_native_mutex_unlock(NULL);
        case M3_TASKS_TEST_NATIVE_COND_INIT:
            return m3_native_cond_init(NULL);
        case M3_TASKS_TEST_NATIVE_COND_DESTROY:
            return m3_native_cond_destroy(NULL);
        case M3_TASKS_TEST_NATIVE_COND_SIGNAL:
            return m3_native_cond_signal(NULL);
        case M3_TASKS_TEST_NATIVE_COND_BROADCAST:
            return m3_native_cond_broadcast(NULL);
        case M3_TASKS_TEST_NATIVE_COND_WAIT:
            return m3_native_cond_wait(NULL, NULL);
        case M3_TASKS_TEST_NATIVE_COND_TIMEDWAIT:
            return m3_native_cond_timedwait(NULL, NULL, 1);
        case M3_TASKS_TEST_NATIVE_THREAD_CREATE:
            return m3_native_thread_create(NULL, NULL, NULL);
        case M3_TASKS_TEST_NATIVE_THREAD_JOIN:
            return m3_native_thread_join(NULL);
        case M3_TASKS_TEST_NATIVE_SLEEP:
            return m3_native_sleep_ms(0);
        default:
            return M3_ERR_INVALID_ARGUMENT;
    }
}

int M3_CALL m3_tasks_test_thread_destroy_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3TaskThread thread;
    M3HandleSystemVTable handles_vtable;
    M3TasksTestStub stub;

    memset(&runner, 0, sizeof(runner));
    memset(&thread, 0, sizeof(thread));
    memset(&handles_vtable, 0, sizeof(handles_vtable));
    stub.rc = M3_OK;

    switch (mode) {
        case M3_TASKS_TEST_THREAD_DESTROY_NULL:
            return m3_task_thread_destroy(NULL);
        case M3_TASKS_TEST_THREAD_DESTROY_NOT_JOINED:
            thread.joined = M3_FALSE;
            return m3_task_thread_destroy(&thread);
        case M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_FAIL:
            thread.joined = M3_TRUE;
            thread.runner = &runner;
            thread.header.handle.id = 1u;
            thread.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = &stub;
            stub.rc = M3_ERR_UNKNOWN;
            return m3_task_thread_destroy(&thread);
        case M3_TASKS_TEST_THREAD_DESTROY_UNREGISTER_NULL_CTX:
            thread.joined = M3_TRUE;
            thread.runner = &runner;
            thread.header.handle.id = 1u;
            thread.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = NULL;
            return m3_task_thread_destroy(&thread);
        case M3_TASKS_TEST_THREAD_DESTROY_FREE_FAIL:
            thread.joined = M3_TRUE;
            thread.runner = &runner;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = &stub;
            stub.rc = M3_ERR_OUT_OF_MEMORY;
            return m3_task_thread_destroy(&thread);
        case M3_TASKS_TEST_THREAD_DESTROY_FREE_NULL_CTX:
            thread.joined = M3_TRUE;
            thread.runner = &runner;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = NULL;
            return m3_task_thread_destroy(&thread);
        case M3_TASKS_TEST_THREAD_DESTROY_OK:
            thread.joined = M3_TRUE;
            thread.runner = &runner;
            thread.header.handle.id = 1u;
            thread.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = &stub;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = &stub;
            stub.rc = M3_OK;
            return m3_task_thread_destroy(&thread);
        default:
            return M3_ERR_INVALID_ARGUMENT;
    }
}

int M3_CALL m3_tasks_test_mutex_destroy_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3TaskMutex mutex;
    M3HandleSystemVTable handles_vtable;
    M3TasksTestStub stub;
    int rc;

    memset(&runner, 0, sizeof(runner));
    memset(&mutex, 0, sizeof(mutex));
    memset(&handles_vtable, 0, sizeof(handles_vtable));
    stub.rc = M3_OK;

    switch (mode) {
        case M3_TASKS_TEST_MUTEX_DESTROY_NULL:
            return m3_task_mutex_destroy(NULL);
        case M3_TASKS_TEST_MUTEX_DESTROY_NATIVE_FAIL:
            rc = m3_native_mutex_init(&mutex.mutex);
            if (rc != M3_OK) {
                return rc;
            }
            mutex.initialized = M3_TRUE;
            mutex.runner = &runner;
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_DESTROY);
            rc = m3_task_mutex_destroy(&mutex);
            m3_tasks_test_clear_fail_point();
            return rc;
        case M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_FAIL:
            mutex.initialized = M3_FALSE;
            mutex.runner = &runner;
            mutex.header.handle.id = 1u;
            mutex.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = &stub;
            stub.rc = M3_ERR_UNKNOWN;
            return m3_task_mutex_destroy(&mutex);
        case M3_TASKS_TEST_MUTEX_DESTROY_UNREGISTER_NULL_CTX:
            mutex.initialized = M3_FALSE;
            mutex.runner = &runner;
            mutex.header.handle.id = 1u;
            mutex.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = NULL;
            return m3_task_mutex_destroy(&mutex);
        case M3_TASKS_TEST_MUTEX_DESTROY_FREE_FAIL:
            mutex.initialized = M3_FALSE;
            mutex.runner = &runner;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = &stub;
            stub.rc = M3_ERR_OUT_OF_MEMORY;
            return m3_task_mutex_destroy(&mutex);
        case M3_TASKS_TEST_MUTEX_DESTROY_FREE_NULL_CTX:
            mutex.initialized = M3_FALSE;
            mutex.runner = &runner;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = NULL;
            return m3_task_mutex_destroy(&mutex);
        case M3_TASKS_TEST_MUTEX_DESTROY_OK:
            rc = m3_native_mutex_init(&mutex.mutex);
            if (rc != M3_OK) {
                return rc;
            }
            mutex.initialized = M3_TRUE;
            mutex.runner = &runner;
            mutex.header.handle.id = 1u;
            mutex.header.handle.generation = 1u;
            handles_vtable.unregister_object = m3_tasks_test_unregister;
            runner.handles.vtable = &handles_vtable;
            runner.handles.ctx = &stub;
            runner.allocator.free = m3_tasks_test_free;
            runner.allocator.ctx = &stub;
            stub.rc = M3_OK;
            return m3_task_mutex_destroy(&mutex);
        default:
            return M3_ERR_INVALID_ARGUMENT;
    }
}

static int m3_tasks_test_task_fail(void *user)
{
    M3TasksDefault *runner;

    runner = (M3TasksDefault *)user;
    if (runner != NULL) {
        runner->stopping = M3_TRUE;
    }
    return M3_ERR_UNKNOWN;
}

static int m3_tasks_test_task_noop(void *user)
{
    (void)user;
    return M3_OK;
}

int M3_CALL m3_tasks_test_queue_case(m3_u32 mode, m3_u32 *out_wait_ms, m3_u32 *out_due_time, m3_usize *out_count)
{
    M3TasksDefault runner;
    M3TaskItem items[2];
    M3TaskItem picked;
    m3_u32 wait_ms;
    int rc;

    memset(&runner, 0, sizeof(runner));
    memset(items, 0, sizeof(items));
    memset(&picked, 0, sizeof(picked));

    runner.queue = items;
    runner.queue_capacity = 2;
    runner.queue_count = 0;
    wait_ms = 0;
    rc = M3_ERR_INVALID_ARGUMENT;

    switch (mode) {
        case M3_TASKS_TEST_QUEUE_CASE_EMPTY:
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_INVALID:
            runner.queue_count = 1;
            items[0].due_time_ms = 0;
            rc = m3_tasks_queue_pick(&runner, NULL, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_NOT_READY:
            runner.queue_count = 1;
            items[0].due_time_ms = m3_u32_max_value();
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_READY_EARLY:
            runner.queue_count = 2;
            items[0].due_time_ms = 1000u;
            items[1].due_time_ms = 0u;
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_READY_LATE:
            runner.queue_count = 1;
            items[0].due_time_ms = 0u;
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_SWAP:
            runner.queue_count = 2;
            items[0].due_time_ms = 0u;
            items[1].due_time_ms = 1000u;
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            break;
        case M3_TASKS_TEST_QUEUE_CASE_TIME_FAIL:
            runner.queue_count = 1;
            items[0].due_time_ms = 0u;
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TIME_NOW);
            rc = m3_tasks_queue_pick(&runner, &picked, &wait_ms);
            m3_tasks_test_clear_fail_point();
            break;
        case M3_TASKS_TEST_QUEUE_CASE_BUSY:
            runner.queue_count = runner.queue_capacity;
            rc = m3_tasks_queue_push(&runner, NULL, NULL, 0u);
            break;
        default:
            rc = M3_ERR_INVALID_ARGUMENT;
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

int M3_CALL m3_tasks_test_post_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3TaskItem items[1];
    int rc;

    memset(&runner, 0, sizeof(runner));
    memset(items, 0, sizeof(items));

    runner.queue = items;
    runner.queue_capacity = 1;
    runner.queue_count = 0;
    runner.initialized = M3_TRUE;

    rc = m3_native_mutex_init(&runner.queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_native_cond_init(&runner.queue_cond);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&runner.queue_mutex);
        return rc;
    }

    switch (mode) {
        case M3_TASKS_TEST_POST_CASE_LOCK_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_LOCK);
            rc = m3_tasks_default_task_post(&runner, m3_tasks_test_task_noop, NULL);
            break;
        case M3_TASKS_TEST_POST_CASE_SIGNAL_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_SIGNAL);
            rc = m3_tasks_default_task_post(&runner, m3_tasks_test_task_noop, NULL);
            break;
        case M3_TASKS_TEST_POST_CASE_UNLOCK_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_UNLOCK);
            rc = m3_tasks_default_task_post(&runner, m3_tasks_test_task_noop, NULL);
            break;
        case M3_TASKS_TEST_POST_CASE_BUSY:
            runner.queue_count = runner.queue_capacity;
            rc = m3_tasks_default_task_post(&runner, m3_tasks_test_task_noop, NULL);
            break;
        default:
            rc = M3_ERR_INVALID_ARGUMENT;
            break;
    }

    m3_tasks_test_clear_fail_point();
    m3_native_cond_destroy(&runner.queue_cond);
    m3_native_mutex_destroy(&runner.queue_mutex);
    return rc;
}

int M3_CALL m3_tasks_test_post_delayed_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3TaskItem items[1];
    m3_u32 max_delay;
    int rc;

    memset(&runner, 0, sizeof(runner));
    memset(items, 0, sizeof(items));

    runner.queue = items;
    runner.queue_capacity = 1;
    runner.queue_count = 0;
    runner.initialized = M3_TRUE;

    rc = m3_native_mutex_init(&runner.queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_native_cond_init(&runner.queue_cond);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&runner.queue_mutex);
        return rc;
    }

    switch (mode) {
        case M3_TASKS_TEST_POST_DELAYED_CASE_TIME_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TIME_NOW);
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, 1u);
            break;
        case M3_TASKS_TEST_POST_DELAYED_CASE_LOCK_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_LOCK);
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, 1u);
            break;
        case M3_TASKS_TEST_POST_DELAYED_CASE_SIGNAL_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_SIGNAL);
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, 1u);
            break;
        case M3_TASKS_TEST_POST_DELAYED_CASE_UNLOCK_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_UNLOCK);
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, 1u);
            break;
        case M3_TASKS_TEST_POST_DELAYED_CASE_BUSY:
            runner.queue_count = runner.queue_capacity;
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, 1u);
            break;
        case M3_TASKS_TEST_POST_DELAYED_CASE_OVERFLOW:
            max_delay = (m3_u32)~(m3_u32)0;
            rc = m3_tasks_default_task_post_delayed(&runner, m3_tasks_test_task_noop, NULL, max_delay);
            break;
        default:
            rc = M3_ERR_INVALID_ARGUMENT;
            break;
    }

    m3_tasks_test_clear_fail_point();
    m3_native_cond_destroy(&runner.queue_cond);
    m3_native_mutex_destroy(&runner.queue_mutex);
    return rc;
}

int M3_CALL m3_tasks_test_worker_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3TaskItem items[1];
    int rc;

    memset(&runner, 0, sizeof(runner));
    memset(items, 0, sizeof(items));

    runner.queue = items;
    runner.queue_capacity = 1;
    runner.queue_count = 0;
    runner.stopping = M3_FALSE;

    rc = m3_native_mutex_init(&runner.queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_native_cond_init(&runner.queue_cond);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&runner.queue_mutex);
        return rc;
    }

    switch (mode) {
        case M3_TASKS_TEST_WORKER_CASE_LOCK_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_LOCK);
            (void)m3_tasks_worker_entry(&runner);
            break;
        case M3_TASKS_TEST_WORKER_CASE_WAIT_FAIL:
            runner.queue_count = 0;
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_WAIT);
            (void)m3_tasks_worker_entry(&runner);
            break;
        case M3_TASKS_TEST_WORKER_CASE_TIMEDWAIT_TIMEOUT:
            runner.queue_count = 1;
            items[0].due_time_ms = m3_u32_max_value();
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_TIMEOUT);
            (void)m3_tasks_worker_entry(&runner);
            break;
        case M3_TASKS_TEST_WORKER_CASE_TASK_ERROR:
            runner.queue_count = 1;
            items[0].due_time_ms = 0u;
            items[0].fn = m3_tasks_test_task_fail;
            items[0].user = &runner;
            (void)m3_tasks_worker_entry(&runner);
            break;
        case M3_TASKS_TEST_WORKER_CASE_PICK_ERROR:
            runner.queue_count = 1;
            items[0].due_time_ms = 0u;
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TIME_NOW);
            (void)m3_tasks_worker_entry(&runner);
            break;
        case M3_TASKS_TEST_WORKER_CASE_UNLOCK_FAIL:
            runner.queue_count = 1;
            items[0].due_time_ms = 0u;
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_MUTEX_UNLOCK);
            (void)m3_tasks_worker_entry(&runner);
            break;
        default:
            break;
    }

    m3_tasks_test_clear_fail_point();
    m3_native_cond_destroy(&runner.queue_cond);
    m3_native_mutex_destroy(&runner.queue_mutex);
    return M3_OK;
}

int M3_CALL m3_tasks_test_object_op(m3_u32 mode, m3_u32 *out_type_id)
{
    M3TaskThread thread;
    int rc;

    memset(&thread, 0, sizeof(thread));
    rc = m3_object_header_init(&thread.header, M3_TASK_OBJECT_THREAD, 0, &g_m3_task_thread_vtable);
    if (rc != M3_OK) {
        return rc;
    }

    switch (mode) {
        case M3_TASKS_TEST_OBJECT_OP_RETAIN:
            return m3_task_object_retain(&thread);
        case M3_TASKS_TEST_OBJECT_OP_RELEASE:
            thread.header.ref_count = 2;
            return m3_task_object_release(&thread);
        case M3_TASKS_TEST_OBJECT_OP_GET_TYPE:
            if (out_type_id == NULL) {
                return M3_ERR_INVALID_ARGUMENT;
            }
            return m3_task_object_get_type_id(&thread, out_type_id);
        case M3_TASKS_TEST_OBJECT_OP_RETAIN_NULL:
            return m3_task_object_retain(NULL);
        case M3_TASKS_TEST_OBJECT_OP_RELEASE_NULL:
            return m3_task_object_release(NULL);
        default:
            return M3_ERR_INVALID_ARGUMENT;
    }
}

static M3_TASK_THREAD_RETURN M3_TASK_THREAD_CALL m3_tasks_test_signal_entry(void *user)
{
    M3TasksDefault *runner;

    runner = (M3TasksDefault *)user;
    if (runner == NULL) {
#if defined(M3_TASKS_USE_WIN32)
        return 0;
#else
        return NULL;
#endif
    }

    m3_native_mutex_lock(&runner->queue_mutex);
    m3_native_cond_signal(&runner->queue_cond);
    m3_native_mutex_unlock(&runner->queue_mutex);

#if defined(M3_TASKS_USE_WIN32)
    return 0;
#else
    return NULL;
#endif
}

int M3_CALL m3_tasks_test_timedwait_case(m3_u32 mode)
{
    M3TasksDefault runner;
    M3NativeThread thread;
    int rc;
    int join_rc;
    int i;

    memset(&runner, 0, sizeof(runner));
    rc = m3_native_mutex_init(&runner.queue_mutex);
    if (rc != M3_OK) {
        return rc;
    }
    rc = m3_native_cond_init(&runner.queue_cond);
    if (rc != M3_OK) {
        m3_native_mutex_destroy(&runner.queue_mutex);
        return rc;
    }

    rc = m3_native_mutex_lock(&runner.queue_mutex);
    if (rc != M3_OK) {
        m3_native_cond_destroy(&runner.queue_cond);
        m3_native_mutex_destroy(&runner.queue_mutex);
        return rc;
    }

    switch (mode) {
        case M3_TASKS_TEST_TIMEDWAIT_TIMEOUT:
            rc = m3_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 0u);
            break;
        case M3_TASKS_TEST_TIMEDWAIT_ERROR:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR);
            rc = m3_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 1u);
            break;
        case M3_TASKS_TEST_TIMEDWAIT_TIME_FAIL:
            m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_TIME_NOW);
            rc = m3_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 1u);
            break;
        case M3_TASKS_TEST_TIMEDWAIT_OK:
            rc = m3_native_thread_create(&thread, m3_tasks_test_signal_entry, &runner);
            if (rc != M3_OK) {
                break;
            }
            rc = m3_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 100u);
            join_rc = m3_native_thread_join(&thread);
            if (rc == M3_OK && join_rc != M3_OK) {
                rc = join_rc;
            }
            break;
        case M3_TASKS_TEST_TIMEDWAIT_NSEC_ADJUST:
            g_m3_tasks_test_nsec_adjusted = 0;
            for (i = 0; i < 2000 && g_m3_tasks_test_nsec_adjusted == 0; ++i) {
                m3_tasks_test_set_fail_point(M3_TASKS_TEST_FAIL_COND_TIMEDWAIT_ERROR);
                rc = m3_native_cond_timedwait(&runner.queue_cond, &runner.queue_mutex, 999u);
                if (rc != M3_ERR_UNKNOWN && rc != M3_ERR_TIMEOUT && rc != M3_OK) {
                    break;
                }
            }
            if (g_m3_tasks_test_nsec_adjusted == 0) {
                rc = M3_ERR_UNKNOWN;
            } else {
                rc = M3_OK;
            }
            break;
        default:
            rc = M3_ERR_INVALID_ARGUMENT;
            break;
    }

    m3_tasks_test_clear_fail_point();
    m3_native_mutex_unlock(&runner.queue_mutex);
    m3_native_cond_destroy(&runner.queue_cond);
    m3_native_mutex_destroy(&runner.queue_mutex);
    return rc;
}

int M3_CALL m3_tasks_test_call_noop(void)
{
    return m3_tasks_test_task_noop(NULL);
}
#endif
