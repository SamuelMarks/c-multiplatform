/* clang-format off */
#include "cmp.h"
#include <stdlib.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

/*
 * NOTE: For full compatibility, we will use a naive auto-reset event + semaphore approach
 * if true CONDITION_VARIABLE requires Vista+. But since target is MSVC 2005 onwards,
 * we will use CreateEvent/SetEvent for now to keep it safe on older targets.
 */
typedef struct cmp_win32_cond {
    HANDLE events[2]; /* 0: signal, 1: broadcast */
    unsigned int waiters_count;
    HANDLE waiters_count_lock;
} cmp_win32_cond_t;

#endif
/* clang-format on */

#if defined(__APPLE__)
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
} cmp_apple_sem_t;
#endif

int cmp_mutex_init(cmp_mutex_t *mutex) {
  if (mutex == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  *mutex = CreateMutexA(NULL, 0, NULL);
  if (*mutex == NULL) {
    return CMP_ERROR_OOM;
  }
#else
  if (pthread_mutex_init(mutex, NULL) != 0) {
    return CMP_ERROR_OOM;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_mutex_lock(cmp_mutex_t *mutex) {
  if (mutex == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (WaitForSingleObject(*mutex, 0xFFFFFFFF) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#else
  if (pthread_mutex_lock(mutex) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_mutex_unlock(cmp_mutex_t *mutex) {
  if (mutex == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (ReleaseMutex(*mutex) == 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#else
  if (pthread_mutex_unlock(mutex) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_mutex_destroy(cmp_mutex_t *mutex) {
  if (mutex == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (CloseHandle(*mutex) == 0) {
    return CMP_ERROR_INVALID_ARG;
  }
  *mutex = NULL;
#else
  if (pthread_mutex_destroy(mutex) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_semaphore_init(cmp_semaphore_t *sem, int initial_count) {
  if (sem == NULL || initial_count < 0) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  *sem = CreateSemaphoreA(NULL, initial_count, 2147483647, NULL);
  if (*sem == NULL) {
    return CMP_ERROR_OOM;
  }
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as;
    int res;

    res = CMP_MALLOC(sizeof(cmp_apple_sem_t), (void **)&as);
    if (res != CMP_SUCCESS || as == NULL) {
      return CMP_ERROR_OOM;
    }

    if (pthread_mutex_init(&as->mutex, NULL) != 0) {
      CMP_FREE(as);
      return CMP_ERROR_OOM;
    }

    if (pthread_cond_init(&as->cond, NULL) != 0) {
      pthread_mutex_destroy(&as->mutex);
      CMP_FREE(as);
      return CMP_ERROR_OOM;
    }

    as->count = initial_count;
    *sem = (cmp_semaphore_t)as;
  }
#else
  if (sem_init(sem, 0, initial_count) != 0) {
    return CMP_ERROR_OOM;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_semaphore_wait(cmp_semaphore_t *sem) {
  if (sem == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (WaitForSingleObject(*sem, 0xFFFFFFFF) != 0) {
    return CMP_ERROR_INVALID_ARG;
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
  return CMP_SUCCESS;
}

int cmp_semaphore_post(cmp_semaphore_t *sem) {
  if (sem == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (ReleaseSemaphore(*sem, 1, NULL) == 0) {
    return CMP_ERROR_INVALID_ARG;
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
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_semaphore_destroy(cmp_semaphore_t *sem) {
  if (sem == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  if (CloseHandle(*sem) == 0) {
    return CMP_ERROR_INVALID_ARG;
  }
  *sem = NULL;
#elif defined(__APPLE__)
  {
    cmp_apple_sem_t *as = (cmp_apple_sem_t *)(*sem);
    pthread_mutex_destroy(&as->mutex);
    pthread_cond_destroy(&as->cond);
    CMP_FREE(as);
    *sem = NULL;
  }
#else
  if (sem_destroy(sem) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_cond_init(cmp_cond_t *cond) {
  if (cond == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  {
    cmp_win32_cond_t *cv;
    int res;

    res = CMP_MALLOC(sizeof(cmp_win32_cond_t), (void **)&cv);
    if (res != CMP_SUCCESS || cv == NULL) {
      return CMP_ERROR_OOM;
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
      CMP_FREE(cv);
      return CMP_ERROR_OOM;
    }

    *cond = (cmp_cond_t)cv;
  }
#else
  if (pthread_cond_init(cond, NULL) != 0) {
    return CMP_ERROR_OOM;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_cond_wait(cmp_cond_t *cond, cmp_mutex_t *mutex) {
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int result;
  int last_waiter;
#endif

  if (cond == NULL || mutex == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  WaitForSingleObject(cv->waiters_count_lock, 0xFFFFFFFF);
  cv->waiters_count++;
  ReleaseMutex(cv->waiters_count_lock);

  cmp_mutex_unlock(mutex);

  result = WaitForMultipleObjects(2, (const HANDLE *)cv->events, 0, 0xFFFFFFFF);

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
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_cond_signal(cmp_cond_t *cond) {
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int have_waiters;
#endif

  if (cond == NULL) {
    return CMP_ERROR_INVALID_ARG;
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
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_cond_broadcast(cmp_cond_t *cond) {
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
  int have_waiters;
#endif

  if (cond == NULL) {
    return CMP_ERROR_INVALID_ARG;
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
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}

int cmp_cond_destroy(cmp_cond_t *cond) {
#if defined(_WIN32)
  cmp_win32_cond_t *cv;
#endif

  if (cond == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
  cv = (cmp_win32_cond_t *)(*cond);

  CloseHandle(cv->events[0]);
  CloseHandle(cv->events[1]);
  CloseHandle(cv->waiters_count_lock);

  CMP_FREE(cv);
  *cond = NULL;
#else
  if (pthread_cond_destroy(cond) != 0) {
    return CMP_ERROR_INVALID_ARG;
  }
#endif
  return CMP_SUCCESS;
}
