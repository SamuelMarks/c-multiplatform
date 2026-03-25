/* clang-format off */
#include "cmp.h"
#include <stdlib.h>

#if defined(_WIN32)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
__declspec(dllimport) void *__stdcall CreateThread(void *lpThreadAttributes, size_t dwStackSize, unsigned long (__stdcall *lpStartAddress)(void *), void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId);
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);
__declspec(dllimport) int __stdcall CloseHandle(void *hObject);
#else
#include <unistd.h>
#endif
/* clang-format on */

typedef struct cmp_task_node {
  cmp_task_fn_t fn;
  void *arg;
  struct cmp_task_node *next;
} cmp_task_node_t;

typedef struct cmp_modality_single_state {
  cmp_task_node_t *head;
  cmp_task_node_t *tail;
} cmp_modality_single_state_t;

typedef struct cmp_modality_threaded_state {
  cmp_ring_buffer_t queue;
  cmp_thread_t *workers;
  int num_workers;
  cmp_modality_t *parent;
} cmp_modality_threaded_state_t;

#if defined(_WIN32)
static unsigned long __stdcall cmp_worker_thread_func(void *arg) {
#else
static void *cmp_worker_thread_func(void *arg) {
#endif
  cmp_modality_threaded_state_t *state = (cmp_modality_threaded_state_t *)arg;
  cmp_task_node_t *node;

  while (state->parent != NULL && state->parent->is_running) {
    if (cmp_ring_buffer_pop(&state->queue, (void **)&node) == CMP_SUCCESS) {
      node->fn(node->arg);
      CMP_FREE(node);
    } else {
#if defined(_WIN32)
      Sleep(1);
#else
      usleep(1000);
#endif
    }
  }

#if defined(_WIN32)
  return 0;
#else
  return NULL;
#endif
}

int cmp_modality_threaded_init(cmp_modality_t *mod, int num_workers) {
  cmp_modality_threaded_state_t *state;
  int res;
  int i;

  if (mod == NULL || num_workers <= 0) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = CMP_MALLOC(sizeof(cmp_modality_threaded_state_t), (void **)&state);
  if (res != CMP_SUCCESS || state == NULL) {
    return CMP_ERROR_OOM;
  }

  res =
      cmp_ring_buffer_init(&state->queue, 1024 * 16); /* 16K tasks max queue */
  if (res != CMP_SUCCESS) {
    CMP_FREE(state);
    return res;
  }

  res =
      CMP_MALLOC(sizeof(cmp_thread_t) * num_workers, (void **)&state->workers);
  if (res != CMP_SUCCESS || state->workers == NULL) {
    cmp_ring_buffer_destroy(&state->queue);
    CMP_FREE(state);
    return CMP_ERROR_OOM;
  }

  state->num_workers = num_workers;
  state->parent = mod;

  mod->type = CMP_MODALITY_THREADED;
  mod->internal_state = state;
  mod->is_running = 1;

  for (i = 0; i < num_workers; i++) {
#if defined(_WIN32)
    state->workers[i] =
        CreateThread(NULL, 0, cmp_worker_thread_func, state, 0, NULL);
#else
    pthread_create(&state->workers[i], NULL, cmp_worker_thread_func, state);
#endif
  }

  return CMP_SUCCESS;
}

int cmp_modality_async_init(cmp_modality_t *mod) {
  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_modality_eventloop_init(cmp_modality_t *mod) {
  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

int cmp_modality_single_init(cmp_modality_t *mod) {
  cmp_modality_single_state_t *state;
  int res;

  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = CMP_MALLOC(sizeof(cmp_modality_single_state_t), (void **)&state);
  if (res != CMP_SUCCESS || state == NULL) {
    return CMP_ERROR_OOM;
  }

  state->head = NULL;
  state->tail = NULL;

  mod->type = CMP_MODALITY_SINGLE;
  mod->internal_state = state;
  mod->is_running = 0;

  return CMP_SUCCESS;
}

int cmp_modality_queue_task(cmp_modality_t *mod, cmp_task_fn_t task,
                            void *arg) {
  cmp_modality_single_state_t *state;
  cmp_task_node_t *node;
  int res;

  if (mod == NULL || task == NULL || mod->internal_state == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_SINGLE) {
    state = (cmp_modality_single_state_t *)mod->internal_state;

    res = CMP_MALLOC(sizeof(cmp_task_node_t), (void **)&node);
    if (res != CMP_SUCCESS || node == NULL) {
      return CMP_ERROR_OOM;
    }

    node->fn = task;
    node->arg = arg;
    node->next = NULL;

    if (state->tail == NULL) {
      state->head = node;
      state->tail = node;
    } else {
      state->tail->next = node;
      state->tail = node;
    }
  } else if (mod->type == CMP_MODALITY_THREADED) {
    cmp_modality_threaded_state_t *tstate =
        (cmp_modality_threaded_state_t *)mod->internal_state;

    res = CMP_MALLOC(sizeof(cmp_task_node_t), (void **)&node);
    if (res != CMP_SUCCESS || node == NULL) {
      return CMP_ERROR_OOM;
    }

    node->fn = task;
    node->arg = arg;
    node->next = NULL;

    res = cmp_ring_buffer_push(&tstate->queue, node);
    if (res != CMP_SUCCESS) {
      CMP_FREE(node);
      return res;
    }
  } else {
    return CMP_ERROR_INVALID_ARG;
  }

  return CMP_SUCCESS;
}

int cmp_modality_run(cmp_modality_t *mod) {
  cmp_modality_single_state_t *state;
  cmp_task_node_t *node;

  if (mod == NULL || mod->internal_state == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_THREADED) {
    /* Threaded modality is already running its workers */
    /* You could block main thread here optionally, but we return for now */
    return CMP_SUCCESS;
  }

  if (mod->type != CMP_MODALITY_SINGLE) {
    return CMP_ERROR_INVALID_ARG;
  }

  state = (cmp_modality_single_state_t *)mod->internal_state;
  mod->is_running = 1;

  while (mod->is_running) {
    if (state->head != NULL) {
      node = state->head;
      state->head = node->next;
      if (state->head == NULL) {
        state->tail = NULL;
      }

      node->fn(node->arg);
      CMP_FREE(node);
    } else {
      /* Sleep to prevent 100% CPU on idle */
#if defined(_WIN32)
      Sleep(1);
#else
      usleep(1000);
#endif
    }
  }

  return CMP_SUCCESS;
}

int cmp_modality_stop(cmp_modality_t *mod) {
  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  mod->is_running = 0;
  return CMP_SUCCESS;
}

int cmp_modality_destroy(cmp_modality_t *mod) {
  if (mod == NULL || mod->internal_state == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_SINGLE) {
    cmp_modality_single_state_t *state =
        (cmp_modality_single_state_t *)mod->internal_state;
    cmp_task_node_t *curr = state->head;
    cmp_task_node_t *next;

    while (curr != NULL) {
      next = curr->next;
      CMP_FREE(curr);
      curr = next;
    }

    CMP_FREE(state);
  } else if (mod->type == CMP_MODALITY_THREADED) {
    cmp_modality_threaded_state_t *state =
        (cmp_modality_threaded_state_t *)mod->internal_state;
    int i;
    cmp_task_node_t *node;

    mod->is_running = 0;

    for (i = 0; i < state->num_workers; i++) {
#if defined(_WIN32)
      if (state->workers[i] != NULL) {
        WaitForSingleObject(state->workers[i], 0xFFFFFFFF); /* INFINITE */
        CloseHandle(state->workers[i]);
      }
#else
      pthread_join(state->workers[i], NULL);
#endif
    }

    while (cmp_ring_buffer_pop(&state->queue, (void **)&node) == CMP_SUCCESS) {
      CMP_FREE(node);
    }

    cmp_ring_buffer_destroy(&state->queue);
    CMP_FREE(state->workers);
    CMP_FREE(state);
  }

  mod->internal_state = NULL;

  return CMP_SUCCESS;
}