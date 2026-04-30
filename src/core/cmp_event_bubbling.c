/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Dispatches an event through the UI tree using capture and bubbling
 * phases.
 *
 * @param tree Pointer to the root UI node.
 * @param target_node Pointer to the target UI node.
 * @param event Pointer to the event to dispatch.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_event_dispatch_run(cmp_ui_node_t *tree, cmp_ui_node_t *target_node,
                           cmp_event_t *event) {
  int rc;
  cmp_ui_node_t **ancestors;
  int ancestor_count = 0;
  int capacity = 16;
  cmp_ui_node_t *curr;
  int i;
  cmp_event_listener_node_t *listener;

  if (!tree || !target_node || !event) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_event_dispatch_run: %s\n", err_str);
    }
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_node_t *) * capacity, (void **)&ancestors);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_event_dispatch_run CMP_MALLOC: %s\n", err_str);
    }
    if (rc != CMP_SUCCESS) {
      return rc;
    }
    return rc;
  }

  /* 1. Build Ancestor Chain */
  curr = target_node->parent;
  while (curr != NULL) {
    if (ancestor_count >= capacity) {
      cmp_ui_node_t **new_ancestors;
      capacity *= 2;
      /* In a pure C89 system without realloc, we allocate new and copy */
      rc = CMP_MALLOC(sizeof(cmp_ui_node_t *) * capacity,
                      (void **)&new_ancestors);
      if (rc != CMP_SUCCESS) {
        if (rc == CMP_SUCCESS)
          rc = CMP_ERROR_OOM;
        {
          const char *err_str;
          int rc2;
          rc2 = cmp_strerror(rc, &err_str);
          if (rc2 != CMP_SUCCESS) {
            err_str = "Unknown";
          }
          LOG_DEBUG("cmp_event_dispatch_run (realloc) CMP_MALLOC: %s\n",
                    err_str);
        }
        CMP_FREE(ancestors);
        if (rc != CMP_SUCCESS) {
          return rc;
        }
        return rc;
      }
      memcpy(new_ancestors, ancestors,
             sizeof(cmp_ui_node_t *) * ancestor_count);
      CMP_FREE(ancestors);
      ancestors = new_ancestors;
    }
    ancestors[ancestor_count++] = curr;
    curr = curr->parent;
  }

  /* 2. Capture Phase (Root down to target) */
  for (i = ancestor_count - 1; i >= 0; i--) {
    curr = ancestors[i];
    for (listener = curr->event_listeners; listener != NULL;
         listener = listener->next) {
      if (listener->event_type == event->type && listener->capture) {
        listener->callback(event, curr, listener->user_data);
      }
    }
  }

  /* 3. Target Phase */
  for (listener = target_node->event_listeners; listener != NULL;
       listener = listener->next) {
    if (listener->event_type == event->type) {
      listener->callback(event, target_node, listener->user_data);
    }
  }

  /* 4. Bubble Phase (Target up to root) */
  for (i = 0; i < ancestor_count; i++) {
    curr = ancestors[i];
    for (listener = curr->event_listeners; listener != NULL;
         listener = listener->next) {
      if (listener->event_type == event->type && !listener->capture) {
        listener->callback(event, curr, listener->user_data);
      }
    }
  }

  CMP_FREE(ancestors);
  return rc;
}

/**
 * @brief Adds an event listener to a UI node.
 *
 * @param node Pointer to the UI node.
 * @param event_type The type of event to listen for.
 * @param capture Set to 1 to capture events during the capture phase, 0 for
 * bubble phase.
 * @param callback The function to call when the event occurs.
 * @param user_data Opaque pointer passed to the callback.
 * @return Returns 0 on success, or an error code on failure.
 */
CMP_EXEMPT(int cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *),
    void *user_data)) {
  int rc = CMP_SUCCESS;
  cmp_event_listener_node_t *listener;

  if (!node || !callback)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_event_listener_node_t), (void **)&listener) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  listener->event_type = event_type;
  listener->capture = capture;
  listener->callback = callback;
  listener->user_data = user_data;
  listener->next = node->event_listeners;
  node->event_listeners = listener;

  return rc;
}
