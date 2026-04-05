/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_event_dispatch_run(cmp_ui_node_t *tree, cmp_ui_node_t *target_node,
                           cmp_event_t *event) {
  cmp_ui_node_t **ancestors;
  int ancestor_count = 0;
  int capacity = 16;
  cmp_ui_node_t *curr;
  int i;
  cmp_event_listener_node_t *listener;

  if (!tree || !target_node || !event)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_ui_node_t *) * capacity, (void **)&ancestors) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  /* 1. Build Ancestor Chain */
  curr = target_node->parent;
  while (curr != NULL) {
    if (ancestor_count >= capacity) {
      cmp_ui_node_t **new_ancestors;
      capacity *= 2;
      /* In a pure C89 system without realloc, we allocate new and copy */
      if (CMP_MALLOC(sizeof(cmp_ui_node_t *) * capacity,
                     (void **)&new_ancestors) != CMP_SUCCESS) {
        CMP_FREE(ancestors);
        return CMP_ERROR_OOM;
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
  return CMP_SUCCESS;
}

int cmp_ui_node_add_event_listener(
    cmp_ui_node_t *node, uint32_t event_type, int capture,
    void (*callback)(cmp_event_t *, cmp_ui_node_t *, void *), void *user_data) {
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

  return CMP_SUCCESS;
}

