/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <c_abstract_http/event_loop.h>

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
} cmp_modality_sync_multi_state_t;

#if defined(_WIN32)
/**
 * @brief cmp_worker_thread_func
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static unsigned long __stdcall cmp_worker_thread_func(void *arg) {
#else
/**
 * @brief cmp_worker_thread_func
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void *cmp_worker_thread_func(void *arg) {
#endif
  cmp_modality_sync_multi_state_t *state =
      (cmp_modality_sync_multi_state_t *)arg;
  cmp_task_node_t *node;

  while (state->parent != NULL && state->parent->is_running) {
    if (cmp_ring_buffer_pop(&state->queue, (void **)&node) == CMP_SUCCESS) {
      node->fn(node->arg);
      CMP_FREE(node);
    } else {
#if defined(_WIN32)
      Sleep(1);
#elif defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
      /* no-op or delay */
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

/**
 * @brief cmp_modality_sync_multi_init
 *
 * @param mod Parameter description.
 * @param num_workers Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_sync_multi_init(cmp_modality_t *mod, int num_workers) {
  cmp_modality_sync_multi_state_t *state;
  int res;
  int i;

  if (mod == NULL || num_workers <= 0) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = CMP_MALLOC(sizeof(cmp_modality_sync_multi_state_t), (void **)&state);
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

  mod->type = CMP_MODALITY_SYNC_MULTI;
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

/**
 * @brief cmp_modality_async_single_init
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_async_single_init(cmp_modality_t *mod) {
  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_modality_async_multi_init
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_async_multi_init(cmp_modality_t *mod) {
  if (mod == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_modality_sync_single_init
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_sync_single_init(cmp_modality_t *mod) {
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

  mod->type = CMP_MODALITY_SYNC_SINGLE;
  mod->internal_state = state;
  mod->is_running = 0;

  return CMP_SUCCESS;
}

/**
 * @brief cmp_modality_queue_task
 *
 * @param mod Parameter description.
 * @param task Parameter description.
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_queue_task(cmp_modality_t *mod, cmp_task_fn_t task,
                            void *arg) {
  cmp_modality_single_state_t *state;
  cmp_task_node_t *node;
  int res;

  if (mod == NULL || task == NULL || mod->internal_state == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_SYNC_SINGLE) {
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
    return CMP_SUCCESS;
  } else if (mod->type == CMP_MODALITY_SYNC_MULTI) {
    cmp_modality_sync_multi_state_t *tstate =
        (cmp_modality_sync_multi_state_t *)mod->internal_state;

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
    return CMP_SUCCESS;
  }

  return CMP_ERROR_INVALID_ARG;
}

/**
 * @brief cmp_modality_run
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_run(cmp_modality_t *mod) {
  int rc = CMP_SUCCESS;
  cmp_modality_single_state_t *state = NULL;
  cmp_task_node_t *node = NULL;

  if (mod == NULL || mod->internal_state == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_run: Invalid argument\n");
    return rc;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    struct ModalityEventLoop *loop =
        (struct ModalityEventLoop *)mod->internal_state;
    if (loop) {
      http_loop_run(loop);
    }
    return rc;
  }

  if (mod->type == CMP_MODALITY_SYNC_MULTI) {
    /* Threaded modality is already running its workers */
    /* You could block main thread here optionally, but we return for now */
    return rc;
  }

  if (mod->type != CMP_MODALITY_SYNC_SINGLE) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_run: Unsupported modality type\n");
    return rc;
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
#elif defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
      /* no-op or delay */
#else
      usleep(1000);
#endif
    }
  }

  return rc;
}

/**
 * @brief cmp_modality_stop
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_stop(cmp_modality_t *mod) {
  int rc = CMP_SUCCESS;
  if (mod == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_stop: Invalid argument\n");
    return rc;
  }

  mod->is_running = 0;
  return rc;
}

/**
 * @brief cmp_modality_destroy
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_destroy(cmp_modality_t *mod) {
  int rc = CMP_SUCCESS;
  if (mod == NULL || mod->internal_state == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_destroy: Invalid argument\n");
    return rc;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    struct ModalityEventLoop *loop =
        (struct ModalityEventLoop *)mod->internal_state;
    if (loop) {
      http_loop_stop(loop);
      http_loop_free(loop);
    }
  } else if (mod->type == CMP_MODALITY_SYNC_SINGLE) {
    cmp_modality_single_state_t *state =
        (cmp_modality_single_state_t *)mod->internal_state;
    cmp_task_node_t *curr = state->head;
    cmp_task_node_t *next = NULL;

    while (curr != NULL) {
      next = curr->next;
      CMP_FREE(curr);
      curr = next;
    }

    CMP_FREE(state);
  } else if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
             mod->type == CMP_MODALITY_ASYNC_MULTI) {
    struct ModalityEventLoop *loop =
        (struct ModalityEventLoop *)mod->internal_state;
    if (loop) {
      http_loop_run(loop);
    }
    return rc;
  }

  if (mod->type == CMP_MODALITY_SYNC_MULTI) {
    cmp_modality_sync_multi_state_t *state =
        (cmp_modality_sync_multi_state_t *)mod->internal_state;
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

  return rc;
}

/**
 * @brief cmp_app_init
 *
 * @param config Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_app_init(cmp_app_config_t *config) {
  int rc = CMP_SUCCESS;
  if (config == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_app_init: Invalid argument\n");
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_run_loop
 *
 * @param user_tick Parameter description.
 * @param user_arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_run_loop(cmp_run_loop_fn user_tick, void *user_arg) {
  if (user_tick) {
    user_tick(user_arg);
  }
}

/**
 * @brief cmp_msg_subscribe
 *
 * @param bus Parameter description.
 * @param channel Parameter description.
 * @param callback Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_subscribe(cmp_msg_bus_t *bus, const char *channel, void *callback) {
  int rc = CMP_SUCCESS;
  if (!bus || !channel || !callback) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_subscribe: Invalid argument\n");
    return rc;
  }
  /* Wraps cdd_msg_bus_subscribe */
  return rc;
}

/**
 * @brief cmp_msg_publish
 *
 * @param bus Parameter description.
 * @param channel Parameter description.
 * @param msg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_msg_publish(cmp_msg_bus_t *bus, const char *channel,
                    const cmp_msg_t *msg) {
  int rc = CMP_SUCCESS;
  if (!bus || !channel || !msg) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_msg_publish: Invalid argument\n");
    return rc;
  }
  /* Wraps cdd_msg_bus_publish */
  return rc;
}

/**
 * @brief cmp_actor_spawn
 *
 * @param bus Parameter description.
 * @param name Parameter description.
 * @param handler Parameter description.
 * @param state Parameter description.
 * @param actor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_actor_spawn(cmp_msg_bus_t *bus, const char *name, void *handler,
                    void *state, cmp_actor_t **actor) {
  int rc = CMP_SUCCESS;
  if (!bus || !name || !handler || !actor) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_actor_spawn: Invalid argument\n");
    return rc;
  }
  /* cdd_actor_spawn(bus, name, handler, state, actor); */
  (void)state;
  return rc;
}

/**
 * @brief cmp_actor_supervise
 *
 * @param actor Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_actor_supervise(cmp_actor_t *actor) {
  int rc = CMP_SUCCESS;
  if (!actor) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_actor_supervise: Invalid argument\n");
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_modality_greenthreads_init
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_greenthreads_init(cmp_modality_t *mod) {
  int rc = CMP_SUCCESS;
  if (mod == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_greenthreads_init: Invalid argument\n");
    return rc;
  }
  cmp_coroutine_system_init();
  mod->type = CMP_MODALITY_GREENTHREADS;
  mod->internal_state = NULL; /* Scheduler state goes here */
  mod->is_running = 1;
  return rc;
}

/**
 * @brief cmp_modality_multiprocess_init
 *
 * @param mod Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_modality_multiprocess_init(cmp_modality_t *mod) {
  int rc = CMP_SUCCESS;
  if (mod == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_modality_multiprocess_init: Invalid argument\n");
    return rc;
  }
  mod->type = CMP_MODALITY_MULTIPROCESS_ACTOR;
  mod->internal_state = NULL; /* Process bus goes here */
  mod->is_running = 1;
  return rc;
}
