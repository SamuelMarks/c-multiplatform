/* clang-format off */
#include "cmpc/cmp_deadlock.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <pthread.h>
#endif
/* clang-format on */

typedef struct CMPDeadlockNode {
  CMPMutex *mutex;
  cmp_usize owner_thread_id;
  cmp_usize waiting_thread_id;
  const char *location;
  struct CMPDeadlockNode *next;
} CMPDeadlockNode;

static CMPAllocator g_deadlock_alloc;
static CMPDeadlockNode *g_deadlock_graph = NULL;
#if defined(_WIN32)
static CRITICAL_SECTION g_deadlock_cs;
#else
static pthread_mutex_t g_deadlock_m = PTHREAD_MUTEX_INITIALIZER;
#endif

static cmp_usize get_current_thread_id(void) {
#if defined(_WIN32)
  return (cmp_usize)GetCurrentThreadId();
#else
  return (cmp_usize)pthread_self();
#endif
}

CMP_API int CMP_CALL cmp_deadlock_detector_init(CMPAllocator *alloc) {
  if (alloc == NULL || alloc->alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  g_deadlock_alloc = *alloc;
  g_deadlock_graph = NULL;

#if defined(_WIN32)
  InitializeCriticalSection(&g_deadlock_cs);
#endif
  return CMP_OK;
}

CMP_API void CMP_CALL cmp_deadlock_detector_destroy(void) {
  CMPDeadlockNode *curr;
  CMPDeadlockNode *next;

#if defined(_WIN32)
  EnterCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_lock(&g_deadlock_m);
#endif

  curr = g_deadlock_graph;
  while (curr != NULL) {
    next = curr->next;
    g_deadlock_alloc.free(g_deadlock_alloc.ctx, curr);
    curr = next;
  }
  g_deadlock_graph = NULL;

#if defined(_WIN32)
  LeaveCriticalSection(&g_deadlock_cs);
  DeleteCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_unlock(&g_deadlock_m);
#endif
}

static CMPBool detect_cycle(cmp_usize start_thread_id,
                            cmp_usize target_thread_id) {
  CMPDeadlockNode *curr;

  /* Traverse waiting edges to see if target_thread_id is waiting for
   * start_thread_id, or any transitively waiting threads. Simple DFS for MVP.
   */
  for (curr = g_deadlock_graph; curr != NULL; curr = curr->next) {
    if (curr->waiting_thread_id == target_thread_id) {
      if (curr->owner_thread_id == start_thread_id) {
        return 1;
      }
      /* Recursive check for transitive cycle */
      if (curr->owner_thread_id != 0 &&
          curr->owner_thread_id != target_thread_id) {
        if (detect_cycle(start_thread_id, curr->owner_thread_id)) {
          return 1;
        }
      }
    }
  }
  return 0;
}

CMP_API int CMP_CALL cmp_deadlock_check_before_lock(CMPMutex *mutex,
                                                    const char *location) {
  cmp_usize tid;
  CMPDeadlockNode *curr;
  CMPDeadlockNode *node = NULL;
  int rc = CMP_OK;

  CMP_UNUSED(location);

  if (g_deadlock_alloc.alloc == NULL || mutex == NULL) {
    return CMP_OK; /* Not initialized or invalid */
  }

  tid = get_current_thread_id();

#if defined(_WIN32)
  EnterCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_lock(&g_deadlock_m);
#endif

  /* Check if already owned by someone else */
  for (curr = g_deadlock_graph; curr != NULL; curr = curr->next) {
    if (curr->mutex == mutex) {
      node = curr;
      break;
    }
  }

  if (node != NULL && node->owner_thread_id != 0 &&
      node->owner_thread_id != tid) {
    /* Thread is going to wait. Check for cycle. */
    node->waiting_thread_id = tid;
    if (detect_cycle(tid, node->owner_thread_id)) {
      /* Deadlock detected! */
      rc = CMP_ERR_UNKNOWN;
    }
  }

#if defined(_WIN32)
  LeaveCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_unlock(&g_deadlock_m);
#endif

  return rc;
}

CMP_API void CMP_CALL cmp_deadlock_record_lock(CMPMutex *mutex) {
  cmp_usize tid;
  CMPDeadlockNode *curr;
  CMPDeadlockNode *node = NULL;

  if (g_deadlock_alloc.alloc == NULL || mutex == NULL) {
    return;
  }

  tid = get_current_thread_id();

#if defined(_WIN32)
  EnterCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_lock(&g_deadlock_m);
#endif

  for (curr = g_deadlock_graph; curr != NULL; curr = curr->next) {
    if (curr->mutex == mutex) {
      node = curr;
      break;
    }
  }

  if (node == NULL) {
    if (g_deadlock_alloc.alloc(g_deadlock_alloc.ctx, sizeof(CMPDeadlockNode),
                               (void **)&node) == CMP_OK) {
      node->mutex = mutex;
      node->next = g_deadlock_graph;
      g_deadlock_graph = node;
    }
  }

  if (node != NULL) {
    node->owner_thread_id = tid;
    if (node->waiting_thread_id == tid) {
      node->waiting_thread_id = 0; /* No longer waiting */
    }
  }

#if defined(_WIN32)
  LeaveCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_unlock(&g_deadlock_m);
#endif
}

CMP_API void CMP_CALL cmp_deadlock_record_unlock(CMPMutex *mutex) {
  CMPDeadlockNode *curr;
  CMPDeadlockNode *prev = NULL;

  if (g_deadlock_alloc.alloc == NULL || mutex == NULL) {
    return;
  }

#if defined(_WIN32)
  EnterCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_lock(&g_deadlock_m);
#endif

  for (curr = g_deadlock_graph; curr != NULL; curr = curr->next) {
    if (curr->mutex == mutex) {
      if (prev == NULL) {
        g_deadlock_graph = curr->next;
      } else {
        prev->next = curr->next;
      }
      g_deadlock_alloc.free(g_deadlock_alloc.ctx, curr);
      break;
    }
    prev = curr;
  }

#if defined(_WIN32)
  LeaveCriticalSection(&g_deadlock_cs);
#else
  pthread_mutex_unlock(&g_deadlock_m);
#endif
}