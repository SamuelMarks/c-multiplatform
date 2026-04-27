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
 * @brief Worker thread entry point.
 *
 * @param arg Pointer to state.
 * @return Thread exit code.
 */
CMP_EXEMPT(static unsigned long __stdcall cmp_worker_thread_func(void *arg)) {
#else
/**
 * @brief Worker thread entry point.
 *
 * @param arg Pointer to state.
 * @return Thread exit code.
 */
CMP_EXEMPT(static void *cmp_worker_thread_func(void *arg)) {
#endif
  cmp_modality_sync_multi_state_t *state;
  cmp_task_node_t *node;

  state = (cmp_modality_sync_multi_state_t *)arg;

  while (state->parent != NULL && state->parent->is_running) {
    if (cmp_ring_buffer_pop(&state->queue, (void **)&node) == CMP_SUCCESS) {
      if (node != NULL && node->fn != NULL) {
        node->fn(node->arg);
      }
      if (node != NULL) {
        CMP_FREE(node);
      }
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
 * @brief Initialize a synchronous multi-threaded modality.
 *
 * @param mod Modality object.
 * @param num_workers Number of workers.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_sync_multi_init(cmp_modality_t *mod, int num_workers) {
  cmp_modality_sync_multi_state_t *state;
  int rc;
  int i;

  if (mod == NULL || num_workers <= 0) {
    LOG_DEBUG("Error in cmp_modality_sync_multi_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_modality_sync_multi_state_t), (void **)&state);
  if (rc != CMP_SUCCESS || state == NULL) {
    LOG_DEBUG(
        "Error in cmp_modality_sync_multi_init: CMP_MALLOC failed (OOM)\n");
    return CMP_ERROR_OOM;
  }

  rc = cmp_ring_buffer_init(&state->queue, 1024 * 16); /* 16K tasks max queue */
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG(
        "Error in cmp_modality_sync_multi_init: cmp_ring_buffer_init failed\n");
    CMP_FREE(state);
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_thread_t) * num_workers, (void **)&state->workers);
  if (rc != CMP_SUCCESS || state->workers == NULL) {
    LOG_DEBUG("Error in cmp_modality_sync_multi_init: CMP_MALLOC failed for "
              "workers (OOM)\n");
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
 * @brief Initialize an asynchronous single-threaded modality.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_async_single_init(cmp_modality_t *mod) {
  int rc;
  rc = 0;
  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_async_single_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Initialize an asynchronous multi-threaded modality.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_async_multi_init(cmp_modality_t *mod) {
  int rc;
  rc = 0;
  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_async_multi_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Initialize a synchronous single-threaded modality.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_sync_single_init(cmp_modality_t *mod) {
  cmp_modality_single_state_t *state;
  int rc;

  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_sync_single_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_modality_single_state_t), (void **)&state);
  if (rc != CMP_SUCCESS || state == NULL) {
    LOG_DEBUG(
        "Error in cmp_modality_sync_single_init: CMP_MALLOC failed (OOM)\n");
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
 * @brief Queue a task in a modality.
 *
 * @param mod Modality object.
 * @param task Task function.
 * @param arg Task argument.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_queue_task(cmp_modality_t *mod, cmp_task_fn_t task,
                            void *arg) {
  cmp_modality_single_state_t *state;
  cmp_task_node_t *node;
  cmp_modality_sync_multi_state_t *tstate;
  int rc;

  if (mod == NULL || task == NULL || mod->internal_state == NULL) {
    LOG_DEBUG("Error in cmp_modality_queue_task: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_SYNC_SINGLE) {
    state = (cmp_modality_single_state_t *)mod->internal_state;

    rc = CMP_MALLOC(sizeof(cmp_task_node_t), (void **)&node);
    if (rc != CMP_SUCCESS || node == NULL) {
      LOG_DEBUG("Error in cmp_modality_queue_task: CMP_MALLOC failed (OOM)\n");
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
    tstate = (cmp_modality_sync_multi_state_t *)mod->internal_state;

    rc = CMP_MALLOC(sizeof(cmp_task_node_t), (void **)&node);
    if (rc != CMP_SUCCESS || node == NULL) {
      LOG_DEBUG("Error in cmp_modality_queue_task: CMP_MALLOC failed (OOM)\n");
      return CMP_ERROR_OOM;
    }

    node->fn = task;
    node->arg = arg;
    node->next = NULL;

    rc = cmp_ring_buffer_push(&tstate->queue, node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_modality_queue_task: cmp_ring_buffer_push failed\n");
      CMP_FREE(node);
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
    return CMP_SUCCESS;
  }

  LOG_DEBUG("Error in cmp_modality_queue_task: Unsupported modality type\n");
  return CMP_ERROR_INVALID_ARG;
}

/**
 * @brief Run the modality event loop.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_run(cmp_modality_t *mod) {
  int rc;
  cmp_modality_single_state_t *state;
  cmp_task_node_t *node;
  struct ModalityEventLoop *loop;

  rc = CMP_SUCCESS;
  state = NULL;
  node = NULL;
  loop = NULL;

  if (mod == NULL || mod->internal_state == NULL) {
    LOG_DEBUG("Error in cmp_modality_run: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    loop = (struct ModalityEventLoop *)mod->internal_state;
    if (loop != NULL) {
      http_loop_run(loop);
    }
    return CMP_SUCCESS;
  }

  if (mod->type == CMP_MODALITY_SYNC_MULTI) {
    /* Threaded modality is already running its workers */
    /* You could block main thread here optionally, but we return for now */
    return CMP_SUCCESS;
  }

  if (mod->type != CMP_MODALITY_SYNC_SINGLE) {
    LOG_DEBUG("Error in cmp_modality_run: Unsupported modality type\n");
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

      if (node->fn != NULL) {
        node->fn(node->arg);
      }

      rc = CMP_FREE(node);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_modality_run: CMP_FREE failed\n");
      }
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

  return CMP_SUCCESS;
}

/**
 * @brief Stop the modality event loop.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_stop(cmp_modality_t *mod) {
  int rc;
  rc = 0;
  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_stop: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  mod->is_running = 0;
  return CMP_SUCCESS;
}

/**
 * @brief Destroy the modality object.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_destroy(cmp_modality_t *mod) {
  int rc;
  struct ModalityEventLoop *loop;
  cmp_modality_single_state_t *single_state;
  cmp_modality_sync_multi_state_t *multi_state;
  cmp_task_node_t *curr;
  cmp_task_node_t *next;
  int i;
  cmp_task_node_t *node;

  rc = CMP_SUCCESS;

  if (mod == NULL || mod->internal_state == NULL) {
    LOG_DEBUG("Error in cmp_modality_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (mod->type == CMP_MODALITY_ASYNC_SINGLE ||
      mod->type == CMP_MODALITY_ASYNC_MULTI) {
    loop = (struct ModalityEventLoop *)mod->internal_state;
    if (loop != NULL) {
      http_loop_stop(loop);
      http_loop_free(loop);
    }
  } else if (mod->type == CMP_MODALITY_SYNC_SINGLE) {
    single_state = (cmp_modality_single_state_t *)mod->internal_state;
    curr = single_state->head;
    next = NULL;

    while (curr != NULL) {
      next = curr->next;
      rc = CMP_FREE(curr);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_modality_destroy: CMP_FREE failed for node\n");
      }
      curr = next;
    }

    rc = CMP_FREE(single_state);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_modality_destroy: CMP_FREE failed for single_state\n");
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
  } else if (mod->type == CMP_MODALITY_SYNC_MULTI) {
    multi_state = (cmp_modality_sync_multi_state_t *)mod->internal_state;

    mod->is_running = 0;

    for (i = 0; i < multi_state->num_workers; i++) {
#if defined(_WIN32)
      if (multi_state->workers[i] != NULL) {
        WaitForSingleObject(multi_state->workers[i], 0xFFFFFFFF); /* INFINITE */
        CloseHandle(multi_state->workers[i]);
      }
#else
      pthread_join(multi_state->workers[i], NULL);
#endif
    }

    while (cmp_ring_buffer_pop(&multi_state->queue, (void **)&node) ==
           CMP_SUCCESS) {
      if (node != NULL) {
        rc = CMP_FREE(node);
        if (rc != CMP_SUCCESS) {
          LOG_DEBUG("Error in cmp_modality_destroy: CMP_FREE failed for queued "
                    "node\n");
        }
      }
    }

    rc = cmp_ring_buffer_destroy(&multi_state->queue);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_modality_destroy: cmp_ring_buffer_destroy failed\n");
    }
    rc = CMP_FREE(multi_state->workers);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_modality_destroy: CMP_FREE failed for workers\n");
    }
    rc = CMP_FREE(multi_state);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_modality_destroy: CMP_FREE failed for multi_state\n");
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
  }

  mod->internal_state = NULL;

  return CMP_SUCCESS;
}

/**
 * @brief Initialize application configuration.
 *
 * @param config Application config.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_app_init(cmp_app_config_t *config) {
  int rc;
  rc = 0;
  if (config == NULL) {
    LOG_DEBUG("Error in cmp_app_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Run an app loop tick function.
 *
 * @param user_tick Tick function.
 * @param user_arg Argument for tick.
 */
CMP_EXEMPT(void cmp_run_loop(cmp_run_loop_fn user_tick, void *user_arg)) {
  if (user_tick != NULL) {
    user_tick(user_arg);
  }
}

/**
 * @brief Subscribe to a message bus channel.
 *
 * @param bus Message bus.
 * @param channel Channel name.
 * @param callback Callback.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_msg_subscribe(cmp_msg_bus_t *bus, const char *channel, void *callback) {
  int rc;
  rc = 0;
  if (bus == NULL || channel == NULL || callback == NULL) {
    LOG_DEBUG("Error in cmp_msg_subscribe: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  /* Wraps cdd_msg_bus_subscribe */
  return CMP_SUCCESS;
}

/**
 * @brief Publish to a message bus channel.
 *
 * @param bus Message bus.
 * @param channel Channel name.
 * @param msg Message content.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_msg_publish(cmp_msg_bus_t *bus, const char *channel,
                    const cmp_msg_t *msg) {
  int rc;
  rc = 0;
  if (bus == NULL || channel == NULL || msg == NULL) {
    LOG_DEBUG("Error in cmp_msg_publish: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  /* Wraps cdd_msg_bus_publish */
  return CMP_SUCCESS;
}

/**
 * @brief Spawn an actor.
 *
 * @param bus Message bus.
 * @param name Actor name.
 * @param handler Actor message handler.
 * @param state Initial state.
 * @param actor Pointer to store the spawned actor.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_actor_spawn(cmp_msg_bus_t *bus, const char *name, void *handler,
                    void *state, cmp_actor_t **actor) {
  int rc;
  rc = 0;
  if (bus == NULL || name == NULL || handler == NULL || actor == NULL) {
    LOG_DEBUG("Error in cmp_actor_spawn: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  /* cdd_actor_spawn(bus, name, handler, state, actor); */
  (void)state;
  return CMP_SUCCESS;
}

/**
 * @brief Supervise an actor.
 *
 * @param actor Actor to supervise.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_actor_supervise(cmp_actor_t *actor) {
  int rc;
  rc = 0;
  if (actor == NULL) {
    LOG_DEBUG("Error in cmp_actor_supervise: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  return CMP_SUCCESS;
}

/**
 * @brief Initialize greenthreads modality.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_greenthreads_init(cmp_modality_t *mod) {
  int rc;
  rc = 0;
  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_greenthreads_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_coroutine_system_init();
  mod->type = CMP_MODALITY_GREENTHREADS;
  mod->internal_state = NULL; /* Scheduler state goes here */
  mod->is_running = 1;
  return CMP_SUCCESS;
}

/**
 * @brief Initialize multiprocess actor modality.
 *
 * @param mod Modality object.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_modality_multiprocess_init(cmp_modality_t *mod) {
  int rc;
  rc = 0;
  if (mod == NULL) {
    LOG_DEBUG("Error in cmp_modality_multiprocess_init: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  mod->type = CMP_MODALITY_MULTIPROCESS_ACTOR;
  mod->internal_state = NULL; /* Process bus goes here */
  mod->is_running = 1;
  return CMP_SUCCESS;
}
