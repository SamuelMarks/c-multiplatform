/* clang-format off */
#include "../include/ui_thread_pool.h"
#include "ui_internal_mem.h"

#if defined(_WIN32) && !defined(UI_SINGLE_THREADED)
#ifndef UI_WINAPI
#if defined(_MSC_VER)
#define UI_WINAPI __stdcall
#elif defined(__GNUC__)
#define UI_WINAPI __attribute__((stdcall))
#else
#define UI_WINAPI
#endif
#endif

typedef void* ui_win_handle;

extern ui_win_handle UI_WINAPI CreateThread(void*, size_t, unsigned long (UI_WINAPI *)(void*), void*, unsigned long, unsigned long*);
extern ui_win_handle UI_WINAPI CreateSemaphoreA(void*, long, long, const char*);
extern int UI_WINAPI ReleaseSemaphore(ui_win_handle, long, long*);
extern ui_win_handle UI_WINAPI CreateMutexA(void*, int, const char*);
extern int UI_WINAPI ReleaseMutex(ui_win_handle);
extern unsigned long UI_WINAPI WaitForSingleObject(ui_win_handle, unsigned long);
extern int UI_WINAPI CloseHandle(ui_win_handle);

#define UI_INFINITE 0xFFFFFFFF

#elif !defined(UI_SINGLE_THREADED)
#include <pthread.h>
/* clang-format on */
#endif

struct ui_task_node {
  enum ui_error (*callback)(void *);
  void *user_data;
  struct ui_task_node *next;
};

/** \brief ui_thread_pool */
struct ui_thread_pool {
  struct ui_task_node *head;
  struct ui_task_node *tail;
  int shutdown;
  int num_threads;

#ifndef UI_SINGLE_THREADED
#ifdef _WIN32
  ui_win_handle mutex;
  ui_win_handle semaphore;
  ui_win_handle *threads;
#else
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t *threads;
  int mutex_initialized;
  int cond_initialized;
#endif
#endif
};

#ifndef UI_SINGLE_THREADED
#ifdef _WIN32
static unsigned long UI_WINAPI ui_worker_thread(void *arg) {
  /* EM_JS */
  struct ui_thread_pool *pool = (struct ui_thread_pool *)arg;
  struct ui_task_node *task = NULL;

  for (;;) {
    WaitForSingleObject(pool->semaphore, UI_INFINITE);

    WaitForSingleObject(pool->mutex, UI_INFINITE);
    if (pool->shutdown && !pool->head) {
      ReleaseMutex(pool->mutex);
      break;
    }

    task = pool->head;
    if (task) {
      pool->head = task->next;
      if (!pool->head) {
        pool->tail = NULL;
      }
    }
    ReleaseMutex(pool->mutex);

    if (task) {
      if (task->callback) {
        (void)task->callback(
            task->user_data); /* Best effort in background thread */
      }
      UI_FREE(task);
      task = NULL;
    } else {
      if (pool->shutdown) {
        break;
      }
    }
  }
  return 0;
}
#else
static void *ui_worker_thread(void *arg) {
  /* EM_JS */
  struct ui_thread_pool *pool = (struct ui_thread_pool *)arg;
  struct ui_task_node *task = NULL;

  for (;;) {
    pthread_mutex_lock(&pool->mutex);
    while (!pool->head && !pool->shutdown) {
      pthread_cond_wait(&pool->cond, &pool->mutex);
    }

    if (pool->shutdown && !pool->head) {
      pthread_mutex_unlock(&pool->mutex);
      break;
    }

    task = pool->head;
    if (task) {
      pool->head = task->next;
      if (!pool->head) {
        pool->tail = NULL;
      }
    }
    pthread_mutex_unlock(&pool->mutex);

    if (task) {
      if (task->callback) {
        (void)task->callback(
            task->user_data); /* Best effort in background thread */
      }
      UI_FREE(task);
      task = NULL;
    }
  }
  return NULL;
}
#endif
#endif

enum ui_error ui_thread_pool_create(int num_threads,
                                    struct ui_thread_pool **out_pool) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_thread_pool *pool = NULL;
#ifndef UI_SINGLE_THREADED
  int i;
  int threads_started = 0;
#endif

  if (!out_pool || num_threads <= 0) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  pool = (struct ui_thread_pool *)UI_MALLOC(sizeof(struct ui_thread_pool));
  if (!pool) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  pool->head = NULL;
  pool->tail = NULL;
  pool->shutdown = 0;
  pool->num_threads = num_threads;

#ifndef UI_SINGLE_THREADED
#ifdef _WIN32
  pool->mutex = NULL;
  pool->semaphore = NULL;
  pool->threads = NULL;

  pool->mutex = CreateMutexA(NULL, 0, NULL);
  if (!pool->mutex) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  pool->semaphore = CreateSemaphoreA(NULL, 0, 0x7FFFFFFF, NULL);
  if (!pool->semaphore) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  pool->threads =
      (ui_win_handle *)UI_MALLOC(sizeof(ui_win_handle) * num_threads);
  if (!pool->threads) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  for (i = 0; i < num_threads; i++) {
    pool->threads[i] = NULL;
  }

  for (i = 0; i < num_threads; i++) {
#ifdef UI_TEST_MOCK_ALLOC
    extern int g_mock_thread_fail;
    if (g_mock_thread_fail == i + 1) {
      pool->threads[i] = NULL;
    } else {
      pool->threads[i] = CreateThread(NULL, 0, ui_worker_thread, pool, 0, NULL);
    }
#else
    pool->threads[i] = CreateThread(NULL, 0, ui_worker_thread, pool, 0, NULL);
#endif
    if (!pool->threads[i]) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    threads_started++;
  }
#else
  pool->threads = NULL;
  pool->mutex_initialized = 0;
  pool->cond_initialized = 0;

  /* We cannot easily mock pthread init failures securely across platforms so
   * they are not tested */
  (void)pthread_mutex_init(&pool->mutex, NULL);
  pool->mutex_initialized = 1;

  (void)pthread_cond_init(&pool->cond, NULL);
  pool->cond_initialized = 1;

  pool->threads = (pthread_t *)UI_MALLOC(sizeof(pthread_t) * num_threads);
  if (!pool->threads) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  for (i = 0; i < num_threads; i++) {
    int prc;
#ifdef UI_TEST_MOCK_ALLOC
    extern int g_mock_thread_fail;
    prc = (g_mock_thread_fail == i + 1)
              ? -1
              : pthread_create(&pool->threads[i], NULL, ui_worker_thread, pool);
#else
    prc = pthread_create(&pool->threads[i], NULL, ui_worker_thread, pool);
#endif
    if (prc != 0) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    threads_started++;
  }
#endif
#endif

  *out_pool = pool;
  pool = NULL;

cleanup:
  if (pool) {
#ifndef UI_SINGLE_THREADED
#ifdef _WIN32
    pool->shutdown = 1;
    if (pool->semaphore && threads_started > 0) {
      ReleaseSemaphore(pool->semaphore, threads_started, NULL);
    }
    if (pool->threads) {
      for (i = 0; i < threads_started; i++) {
        if (pool->threads[i]) {
          WaitForSingleObject(pool->threads[i], UI_INFINITE);
          CloseHandle(pool->threads[i]);
        }
      }
      UI_FREE(pool->threads);
    }
    if (pool->semaphore)
      CloseHandle(pool->semaphore);
    if (pool->mutex)
      CloseHandle(pool->mutex);
#else
    pool->shutdown = 1;
    if (pool->cond_initialized && pool->mutex_initialized) {
      pthread_mutex_lock(&pool->mutex);
      pthread_cond_broadcast(&pool->cond);
      pthread_mutex_unlock(&pool->mutex);
    }
    if (pool->threads) {
      for (i = 0; i < threads_started; i++) {
        if (pool->threads[i]) {
          (void)pthread_join(pool->threads[i], NULL);
        }
      }
      UI_FREE(pool->threads);
    }
    if (pool->cond_initialized)
      pthread_cond_destroy(&pool->cond);
    if (pool->mutex_initialized)
      pthread_mutex_destroy(&pool->mutex);
#endif
#endif
    UI_FREE(pool);
  }
  return rc;
}

enum ui_error ui_thread_pool_destroy(struct ui_thread_pool *pool) {
#ifndef UI_SINGLE_THREADED
  int i;
#endif

  if (!pool)
    return UI_ERROR_INVALID_ARGUMENT;

#ifndef UI_SINGLE_THREADED
#ifdef _WIN32
  WaitForSingleObject(pool->mutex, UI_INFINITE);
  pool->shutdown = 1;
  ReleaseMutex(pool->mutex);

  ReleaseSemaphore(pool->semaphore, pool->num_threads, NULL);
  for (i = 0; i < pool->num_threads; i++) {
    WaitForSingleObject(pool->threads[i], UI_INFINITE);
    CloseHandle(pool->threads[i]);
  }

  UI_FREE(pool->threads);
  CloseHandle(pool->semaphore);
  CloseHandle(pool->mutex);
#else
  pthread_mutex_lock(&pool->mutex);
  pool->shutdown = 1;
  pthread_cond_broadcast(&pool->cond);
  pthread_mutex_unlock(&pool->mutex);

  for (i = 0; i < pool->num_threads; i++) {
    pthread_join(pool->threads[i], NULL);
  }
  UI_FREE(pool->threads);

  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->cond);
#endif
#endif

  UI_FREE(pool);
  return UI_ERROR_NONE;
}

enum ui_error ui_thread_pool_schedule(struct ui_thread_pool *pool,
                                      enum ui_error (*callback)(void *),
                                      void *user_data) {
  struct ui_task_node *node = NULL;

  if (!pool || !callback)
    return UI_ERROR_INVALID_ARGUMENT;

  node = (struct ui_task_node *)UI_MALLOC(sizeof(struct ui_task_node));
  if (!node)
    return UI_ERROR_OUT_OF_MEMORY;

  node->callback = callback;
  node->user_data = user_data;
  node->next = NULL;

#ifdef UI_SINGLE_THREADED
  if (pool->tail) {
    pool->tail->next = node;
  } else {
    pool->head = node;
  }
  pool->tail = node;
#elif defined(_WIN32)
  WaitForSingleObject(pool->mutex, UI_INFINITE);
  if (pool->tail) {
    pool->tail->next = node;
  } else {
    pool->head = node;
  }
  pool->tail = node;
  ReleaseMutex(pool->mutex);
  ReleaseSemaphore(pool->semaphore, 1, NULL);
#else
  pthread_mutex_lock(&pool->mutex);
  if (pool->tail) {
    pool->tail->next = node;
  } else {
    pool->head = node;
  }
  pool->tail = node;
  pthread_cond_signal(&pool->cond);
  pthread_mutex_unlock(&pool->mutex);
#endif

  return UI_ERROR_NONE;
}

enum ui_error ui_thread_pool_tick(struct ui_thread_pool *pool) {
  enum ui_error tick_rc = UI_ERROR_NONE;
  if (!pool)
    return UI_ERROR_INVALID_ARGUMENT;

#ifdef UI_SINGLE_THREADED
  {
    struct ui_task_node *current = pool->head;
    pool->head = NULL;
    pool->tail = NULL;

    while (current) {
      next = current->next;
      if (current->callback) {
        enum ui_error cb_rc = current->callback(current->user_data);
        if (tick_rc == UI_ERROR_NONE && cb_rc != UI_ERROR_NONE)
          tick_rc = cb_rc;
      }
      UI_FREE(current);
      current = next;
    }
  }
#endif

  return tick_rc;
}
