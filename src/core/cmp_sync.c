/* clang-format off */
#include "cmpc/cmp_sync.h"

#include <stdlib.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#endif
/* clang-format on */

struct CMPMutex {
#if defined(_WIN32)
  CRITICAL_SECTION cs;
#else
  pthread_mutex_t m;
#endif
};

CMP_API int CMP_CALL cmp_mutex_create(CMPAllocator *alloc,
                                      CMPMutex **out_mutex) {
  CMPMutex *mutex;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPMutex), (void **)&mutex);
  if (rc != CMP_OK || mutex == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

#if defined(_WIN32)
  InitializeCriticalSection(&mutex->cs);
#else
  if (pthread_mutex_init(&mutex->m, NULL) != 0) {
    alloc->free(alloc->ctx, mutex);
    return CMP_ERR_UNKNOWN;
  }
#endif

  *out_mutex = mutex;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_mutex_destroy(CMPAllocator *alloc, CMPMutex *mutex) {
  if (alloc == NULL || alloc->free == NULL || mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  DeleteCriticalSection(&mutex->cs);
#else
  pthread_mutex_destroy(&mutex->m);
#endif

  alloc->free(alloc->ctx, mutex);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_mutex_lock(CMPMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  EnterCriticalSection(&mutex->cs);
#else
  if (pthread_mutex_lock(&mutex->m) != 0) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_mutex_trylock(CMPMutex *mutex) {
#if !defined(_WIN32)
  int rc;
#endif
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (TryEnterCriticalSection(&mutex->cs)) {
    return CMP_OK;
  }
  return CMP_ERR_NOT_READY;
#else
  rc = pthread_mutex_trylock(&mutex->m);
  if (rc == 0) {
    return CMP_OK;
  } else if (rc == EBUSY) {
    return CMP_ERR_NOT_READY;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_mutex_unlock(CMPMutex *mutex) {
  if (mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  LeaveCriticalSection(&mutex->cs);
#else
  if (pthread_mutex_unlock(&mutex->m) != 0) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  return CMP_OK;
}

struct CMPSemaphore {
#if defined(_WIN32)
  HANDLE handle;
#elif defined(__APPLE__)
  /* macOS does not support unnamed POSIX semaphores via sem_init. */
  /* Fallback using mutex and condvar for simplicity in MVP. */
  pthread_mutex_t m;
  pthread_cond_t c;
  cmp_u32 count;
#else
  sem_t s;
#endif
};

CMP_API int CMP_CALL cmp_semaphore_create(CMPAllocator *alloc,
                                          cmp_u32 initial_count,
                                          CMPSemaphore **out_sem) {
  CMPSemaphore *sem;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_sem == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPSemaphore), (void **)&sem);
  if (rc != CMP_OK || sem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

#if defined(_WIN32)
  sem->handle = CreateSemaphoreA(NULL, (LONG)initial_count, 0x7FFFFFFF, NULL);
  if (sem->handle == NULL) {
    alloc->free(alloc->ctx, sem);
    return CMP_ERR_UNKNOWN;
  }
#elif defined(__APPLE__)
  sem->count = initial_count;
  pthread_mutex_init(&sem->m, NULL);
  pthread_cond_init(&sem->c, NULL);
#else
  if (sem_init(&sem->s, 0, initial_count) != 0) {
    alloc->free(alloc->ctx, sem);
    return CMP_ERR_UNKNOWN;
  }
#endif

  *out_sem = sem;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_semaphore_destroy(CMPAllocator *alloc,
                                           CMPSemaphore *sem) {
  if (alloc == NULL || alloc->free == NULL || sem == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  CloseHandle(sem->handle);
#elif defined(__APPLE__)
  pthread_mutex_destroy(&sem->m);
  pthread_cond_destroy(&sem->c);
#else
  sem_destroy(&sem->s);
#endif

  alloc->free(alloc->ctx, sem);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_semaphore_wait(CMPSemaphore *sem) {
  if (sem == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (WaitForSingleObject(sem->handle, INFINITE) == WAIT_OBJECT_0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#elif defined(__APPLE__)
  pthread_mutex_lock(&sem->m);
  while (sem->count == 0) {
    pthread_cond_wait(&sem->c, &sem->m);
  }
  sem->count--;
  pthread_mutex_unlock(&sem->m);
  return CMP_OK;
#else
  while (sem_wait(&sem->s) != 0) {
    if (errno != EINTR) {
      return CMP_ERR_UNKNOWN;
    }
  }
  return CMP_OK;
#endif
}

CMP_API int CMP_CALL cmp_semaphore_post(CMPSemaphore *sem) {
  if (sem == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (ReleaseSemaphore(sem->handle, 1, NULL)) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#elif defined(__APPLE__)
  pthread_mutex_lock(&sem->m);
  sem->count++;
  pthread_cond_signal(&sem->c);
  pthread_mutex_unlock(&sem->m);
  return CMP_OK;
#else
  if (sem_post(&sem->s) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

struct CMPCondVar {
#if defined(_WIN32)
  CONDITION_VARIABLE cv;
#else
  pthread_cond_t c;
#endif
};

CMP_API int CMP_CALL cmp_condvar_create(CMPAllocator *alloc,
                                        CMPCondVar **out_cond) {
  CMPCondVar *cond;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPCondVar), (void **)&cond);
  if (rc != CMP_OK || cond == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

#if defined(_WIN32)
  InitializeConditionVariable(&cond->cv);
#else
  if (pthread_cond_init(&cond->c, NULL) != 0) {
    alloc->free(alloc->ctx, cond);
    return CMP_ERR_UNKNOWN;
  }
#endif

  *out_cond = cond;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_condvar_destroy(CMPAllocator *alloc,
                                         CMPCondVar *cond) {
  if (alloc == NULL || alloc->free == NULL || cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  /* CONDITION_VARIABLE does not need explicit destruction in Windows */
#else
  pthread_cond_destroy(&cond->c);
#endif

  alloc->free(alloc->ctx, cond);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_condvar_wait(CMPCondVar *cond, CMPMutex *mutex) {
  if (cond == NULL || mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  if (SleepConditionVariableCS(&cond->cv, &mutex->cs, INFINITE)) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#else
  if (pthread_cond_wait(&cond->c, &mutex->m) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_condvar_signal(CMPCondVar *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  WakeConditionVariable(&cond->cv);
  return CMP_OK;
#else
  if (pthread_cond_signal(&cond->c) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}

CMP_API int CMP_CALL cmp_condvar_broadcast(CMPCondVar *cond) {
  if (cond == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(_WIN32)
  WakeAllConditionVariable(&cond->cv);
  return CMP_OK;
#else
  if (pthread_cond_broadcast(&cond->c) == 0) {
    return CMP_OK;
  }
  return CMP_ERR_UNKNOWN;
#endif
}