/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>

#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
/**
 * @brief cmp_semaphore_init
 *
 * @param sem Parameter description.
 * @param count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_init(cmp_semaphore_t *sem, int count) {
int rc = CMP_SUCCESS;
  (void)sem;
  (void)count;
  return rc;
}
/**
 * @brief cmp_semaphore_wait
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_wait(cmp_semaphore_t *sem) {
int rc = CMP_SUCCESS;
  (void)sem;
  return rc;
}
/**
 * @brief cmp_semaphore_post
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_post(cmp_semaphore_t *sem) {
int rc = CMP_SUCCESS;
  (void)sem;
  return rc;
}
/**
 * @brief cmp_semaphore_destroy
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_destroy(cmp_semaphore_t *sem) {
int rc = CMP_SUCCESS;
  (void)sem;
  return rc;
}
/**
 * @brief cmp_mutex_init
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_init(cmp_mutex_t *mutex) {
int rc = CMP_SUCCESS;
  (void)mutex;
  return rc;
}
/**
 * @brief cmp_mutex_lock
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_lock(cmp_mutex_t *mutex) {
int rc = CMP_SUCCESS;
  (void)mutex;
  return rc;
}
/**
 * @brief cmp_mutex_unlock
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_unlock(cmp_mutex_t *mutex) {
int rc = CMP_SUCCESS;
  (void)mutex;
  return rc;
}
/**
 * @brief cmp_mutex_destroy
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_destroy(cmp_mutex_t *mutex) {
int rc = CMP_SUCCESS;
  (void)mutex;
  return rc;
}
/**
 * @brief cmp_cond_init
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_init(cmp_cond_t *cond) {
int rc = CMP_SUCCESS;
  (void)cond;
  return rc;
}
/**
 * @brief cmp_cond_wait
 *
 * @param cond Parameter description.
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_wait(cmp_cond_t *cond, cmp_mutex_t *mutex) {
int rc = CMP_SUCCESS;
  (void)cond;
  (void)mutex;
  return rc;
}
/**
 * @brief cmp_cond_signal
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_signal(cmp_cond_t *cond) {
int rc = CMP_SUCCESS;
  (void)cond;
  return rc;
}
/**
 * @brief cmp_cond_broadcast
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_broadcast(cmp_cond_t *cond) {
int rc = CMP_SUCCESS;
  (void)cond;
  return rc;
}
/**
 * @brief cmp_cond_destroy
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_destroy(cmp_cond_t *cond) {
int rc = CMP_SUCCESS;
  (void)cond;
  return rc;
}
#else
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#if !defined(__MINGW32__) && !defined(__MINGW64__)
/* Forward declare Windows APIs to avoid <windows.h> */
__declspec(dllimport) void *__stdcall CreateMutexA(void *lpMutexAttributes, int bInitialOwner, const char *lpName);
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);
__declspec(dllimport) int __stdcall ReleaseMutex(void *hMutex);
__declspec(dllimport) int __stdcall CloseHandle(void *hObject);
__declspec(dllimport) void *__stdcall CreateSemaphoreA(void *lpSemaphoreAttributes, long lInitialCount, long lMaximumCount, const char *lpName);
__declspec(dllimport) int __stdcall ReleaseSemaphore(void *hSemaphore, long lReleaseCount, long *lpPreviousCount);
__declspec(dllimport) void *__stdcall CreateEventA(void *lpEventAttributes, int bManualReset, int bInitialState, const char *lpName);
__declspec(dllimport) unsigned long __stdcall WaitForMultipleObjects(unsigned long nCount, const void *lpHandles, int bWaitAll, unsigned long dwMilliseconds);
__declspec(dllimport) int __stdcall ResetEvent(void *hEvent);
__declspec(dllimport) int __stdcall SetEvent(void *hEvent);
#endif

/*
 * NOTE: For full compatibility, we will use a naive auto-reset event + semaphore approach
 * if true CONDITION_VARIABLE requires Vista+. But since target is MSVC 2005 onwards,
 * we will use CreateEvent/SetEvent for now to keep it safe on older targets.
 */
typedef struct cmp_win32_cond {
    void *events[2]; /* 0: signal, 1: broadcast */
    unsigned int waiters_count;
    void *waiters_count_lock;
} cmp_win32_cond_t;

#endif
/* clang-format on */

#if defined(__APPLE__)
#include <pthread.h>
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
} cmp_apple_sem_t;
#elif !defined(_WIN32)
#include <pthread.h>
#include <semaphore.h>
#endif

/**
 * @brief cmp_mutex_init
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_init(cmp_mutex_t *mutex) {
  int rc = CMP_SUCCESS;
  if (mutex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_init: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  *mutex = CreateMutexA(NULL, 0, NULL);
  if (*mutex == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_mutex_init: CreateMutexA failed\n");
    return rc;
  }
#else
  if (pthread_mutex_init(mutex, NULL) != 0) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_mutex_init: pthread_mutex_init failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_mutex_lock
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_lock(cmp_mutex_t *mutex) {
  int rc = CMP_SUCCESS;
  if (mutex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_lock: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (WaitForSingleObject(*mutex, 0xFFFFFFFF) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_lock: WaitForSingleObject failed\n");
    return rc;
  }
#else
  if (pthread_mutex_lock(mutex) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_lock: pthread_mutex_lock failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_mutex_unlock
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_unlock(cmp_mutex_t *mutex) {
  int rc = CMP_SUCCESS;
  if (mutex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_unlock: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (ReleaseMutex(*mutex) == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_unlock: ReleaseMutex failed\n");
    return rc;
  }
#else
  if (pthread_mutex_unlock(mutex) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_unlock: pthread_mutex_unlock failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_mutex_destroy
 *
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_mutex_destroy(cmp_mutex_t *mutex) {
  int rc = CMP_SUCCESS;
  if (mutex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_destroy: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (CloseHandle(*mutex) == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_destroy: CloseHandle failed\n");
    return rc;
  }
  *mutex = NULL;
#else
  if (pthread_mutex_destroy(mutex) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_mutex_destroy: pthread_mutex_destroy failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_semaphore_init
 *
 * @param sem Parameter description.
 * @param initial_count Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_init(cmp_semaphore_t *sem, int initial_count) {
  int rc = CMP_SUCCESS;
  if (sem == NULL || initial_count < 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_init: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  *sem = CreateSemaphoreA(NULL, initial_count, 2147483647, NULL);
  if (*sem == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_semaphore_init: CreateSemaphoreA failed\n");
    return rc;
  }
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as;
    int res;

    res = CMP_MALLOC(sizeof(cmp_apple_sem_t), (void **)&as);
    if (res != CMP_SUCCESS || as == NULL) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_semaphore_init: CMP_MALLOC failed\n");
      return rc;
    }

    if (pthread_mutex_init(&as->mutex, NULL) != 0) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_semaphore_init: pthread_mutex_init failed\n");
      res = CMP_FREE(as);
      if (res != CMP_SUCCESS) {
        LOG_DEBUG("cmp_semaphore_init: CMP_FREE failed\n");
      }
      return rc;
    }

    if (pthread_cond_init(&as->cond, NULL) != 0) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_semaphore_init: pthread_cond_init failed\n");
      pthread_mutex_destroy(&as->mutex);
      res = CMP_FREE(as);
      if (res != CMP_SUCCESS) {
        LOG_DEBUG("cmp_semaphore_init: CMP_FREE failed\n");
      }
      return rc;
    }

    as->count = initial_count;
    *sem = (cmp_semaphore_t)as;
  }
#else
  if (sem_init(sem, 0, initial_count) != 0) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_semaphore_init: sem_init failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_semaphore_wait
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_wait(cmp_semaphore_t *sem) {
  int rc = CMP_SUCCESS;
  if (sem == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_wait: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (WaitForSingleObject(*sem, 0xFFFFFFFF) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_wait: WaitForSingleObject failed\n");
    return rc;
  }
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as = (cmp_apple_sem_t *)(*sem);
    pthread_mutex_lock(&as->mutex);
    while (as->count <= 0) {
      pthread_cond_wait(&as->cond, &as->mutex);
    }
    as->count--;
    pthread_mutex_unlock(&as->mutex);
  }
#else
  while (sem_wait(sem) != 0) {
    /* Retry on EINTR */
  }
#endif
  return rc;
}

/**
 * @brief cmp_semaphore_post
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_post(cmp_semaphore_t *sem) {
  int rc = CMP_SUCCESS;
  if (sem == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_post: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (ReleaseSemaphore(*sem, 1, NULL) == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_post: ReleaseSemaphore failed\n");
    return rc;
  }
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as = (cmp_apple_sem_t *)(*sem);
    pthread_mutex_lock(&as->mutex);
    as->count++;
    pthread_cond_signal(&as->cond);
    pthread_mutex_unlock(&as->mutex);
  }
#else
  if (sem_post(sem) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_post: sem_post failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_semaphore_destroy
 *
 * @param sem Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_semaphore_destroy(cmp_semaphore_t *sem) {
  int rc = CMP_SUCCESS;
  if (sem == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_destroy: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  if (CloseHandle(*sem) == 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_destroy: CloseHandle failed\n");
    return rc;
  }
  *sem = NULL;
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as = (cmp_apple_sem_t *)(*sem);
    int res;
    pthread_mutex_destroy(&as->mutex);
    pthread_cond_destroy(&as->cond);
    res = CMP_FREE(as);
    if (res != CMP_SUCCESS) {
      LOG_DEBUG("cmp_semaphore_destroy: CMP_FREE failed\n");
      rc = res;
    }
    *sem = NULL;
  }
#else
  if (sem_destroy(sem) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_semaphore_destroy: sem_destroy failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_cond_init
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_init(cmp_cond_t *cond) {
  int rc = CMP_SUCCESS;
  if (cond == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_init: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  {
    cmp_win32_cond_t *cv;
    int res;

    res = CMP_MALLOC(sizeof(cmp_win32_cond_t), (void **)&cv);
    if (res != CMP_SUCCESS || cv == NULL) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_cond_init: CMP_MALLOC failed\n");
      return rc;
    }

    cv->waiters_count = 0;
    cv->events[0] = CreateEventA(NULL, 0, 0, NULL); /* Signal */
    cv->events[1] = CreateEventA(NULL, 1, 0, NULL); /* Broadcast */
    cv->waiters_count_lock = CreateMutexA(NULL, 0, NULL);

    if (cv->events[0] == NULL || cv->events[1] == NULL ||
        cv->waiters_count_lock == NULL) {
      if (cv->events[0])
        CloseHandle(cv->events[0]);
      if (cv->events[1])
        CloseHandle(cv->events[1]);
      if (cv->waiters_count_lock)
        CloseHandle(cv->waiters_count_lock);
      res = CMP_FREE(cv);
      if (res != CMP_SUCCESS) {
        LOG_DEBUG("cmp_cond_init: CMP_FREE failed during cleanup\n");
      }
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_cond_init: CreateEventA or CreateMutexA failed\n");
      return rc;
    }

    *cond = (cmp_cond_t)cv;
  }
#else
  if (pthread_cond_init(cond, NULL) != 0) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_cond_init: pthread_cond_init failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_cond_wait
 *
 * @param cond Parameter description.
 * @param mutex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_wait(cmp_cond_t *cond, cmp_mutex_t *mutex) {
  int rc = CMP_SUCCESS;
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int result;
  int last_waiter;
#endif

  if (cond == NULL || mutex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_wait: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  WaitForSingleObject(cv->waiters_count_lock, 0xFFFFFFFF);
  cv->waiters_count++;
  ReleaseMutex(cv->waiters_count_lock);

  cmp_mutex_unlock(mutex);

  result = WaitForMultipleObjects(2, (const void **)cv->events, 0, 0xFFFFFFFF);

  WaitForSingleObject(cv->waiters_count_lock, 0xFFFFFFFF);
  cv->waiters_count--;
  last_waiter = (result == 1) && (cv->waiters_count == 0); /* 1 is broadcast */
  ReleaseMutex(cv->waiters_count_lock);

  if (last_waiter) {
    ResetEvent(cv->events[1]); /* Reset broadcast */
  }

  cmp_mutex_lock(mutex);
#else
  if (pthread_cond_wait(cond, mutex) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_wait: pthread_cond_wait failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_cond_signal
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_signal(cmp_cond_t *cond) {
  int rc = CMP_SUCCESS;
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int have_waiters;
#endif

  if (cond == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_signal: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  WaitForSingleObject(cv->waiters_count_lock, 0xFFFFFFFF);
  have_waiters = cv->waiters_count > 0;
  ReleaseMutex(cv->waiters_count_lock);

  if (have_waiters) {
    SetEvent(cv->events[0]);
  }
#else
  if (pthread_cond_signal(cond) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_signal: pthread_cond_signal failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_cond_broadcast
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_broadcast(cmp_cond_t *cond) {
  int rc = CMP_SUCCESS;
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int have_waiters;
#endif

  if (cond == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_broadcast: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  WaitForSingleObject(cv->waiters_count_lock, 0xFFFFFFFF);
  have_waiters = cv->waiters_count > 0;
  ReleaseMutex(cv->waiters_count_lock);

  if (have_waiters) {
    SetEvent(cv->events[1]);
  }
#else
  if (pthread_cond_broadcast(cond) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_broadcast: pthread_cond_broadcast failed\n");
    return rc;
  }
#endif
  return rc;
}

/**
 * @brief cmp_cond_destroy
 *
 * @param cond Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_cond_destroy(cmp_cond_t *cond) {
  int rc = CMP_SUCCESS;
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int res;
#endif

  if (cond == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_destroy: invalid argument\n");
    return rc;
  }
#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  CloseHandle(cv->events[0]);
  CloseHandle(cv->events[1]);
  CloseHandle(cv->waiters_count_lock);

  res = CMP_FREE(cv);
  if (res != CMP_SUCCESS) {
    LOG_DEBUG("cmp_cond_destroy: CMP_FREE failed\n");
    rc = res;
  }
  *cond = NULL;
#else
  if (pthread_cond_destroy(cond) != 0) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_cond_destroy: pthread_cond_destroy failed\n");
    return rc;
  }
#endif
  return rc;
}
#endif
