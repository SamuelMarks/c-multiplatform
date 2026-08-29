/**
 * @file ui_mutation_observer.c
 * @brief Implementation of DOM mutation observation.
 * @details Provides the ability to watch for changes to the DOM tree
 * (attributes, child lists, text).
 */
/* clang-format off */
#include "ui_mutation_observer.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/** @def MAX_GLOBAL_OBSERVERS
 * @brief Maximum number of global mutation observers.
 */
#define MAX_GLOBAL_OBSERVERS 64

/** @brief Global observer array */
static struct ui_mutation_observer *g_observers[MAX_GLOBAL_OBSERVERS] = {NULL};

/**
 * @struct ui_mutation_target_info
 * @brief Information about a registered observation target.
 */
struct ui_mutation_target_info {
  struct ui_dom_node *target;               /**< The DOM node being observed */
  struct ui_mutation_observer_init options; /**< Options for this target */
};

/**
 * @struct ui_mutation_observer
 * @brief State and configuration for a mutation observer.
 */
struct ui_mutation_observer {
  ui_mutation_observer_cb_t callback;      /**< User callback */
  void *user_data;                         /**< Opaque user data for callback */
  struct ui_mutation_target_info *targets; /**< Array of observed targets */
  int target_count;                        /**< Number of active targets */
  int target_capacity;                     /**< Capacity of the targets array */
};

/**
 * @brief Creates a new mutation observer.
 * @param[in] callback The callback function triggered on mutations.
 * @param[in] user_data User data for the callback.
 * @param[out] out_observer Pointer to store the created observer.
 * @return UI_ERROR_NONE on success, or a relevant error code.
 */
ui_error_t
ui_mutation_observer_create(ui_mutation_observer_cb_t callback, void *user_data,
                            struct ui_mutation_observer **out_observer) {
  struct ui_mutation_observer *obs;
  int i;

  if (out_observer == NULL || callback == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  obs = (struct ui_mutation_observer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_mutation_observer));
  if (obs == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  obs->callback = callback;
  obs->user_data = user_data;
  obs->target_capacity = 4;
  obs->target_count = 0;

  obs->targets = (struct ui_mutation_target_info *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_mutation_target_info) * (size_t)obs->target_capacity);
  if (obs->targets == NULL) {
    C_MULTIPLATFORM_FREE(obs);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  for (i = 0; i < MAX_GLOBAL_OBSERVERS; i++) {
    if (g_observers[i] == NULL) {
      g_observers[i] = obs;
      *out_observer = obs;
      return UI_ERROR_NONE;
    }
  }

  /* Global registry full */
  C_MULTIPLATFORM_FREE(obs->targets);
  C_MULTIPLATFORM_FREE(obs);
  return UI_ERROR_OUT_OF_BOUNDS;
}

/**
 * @brief Destroys a mutation observer.
 * @param[in,out] observer The observer to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_mutation_observer_destroy(struct ui_mutation_observer *observer) {
  int i;
  if (observer == NULL) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < MAX_GLOBAL_OBSERVERS; i++) {
    if (g_observers[i] == observer) {
      g_observers[i] = NULL;
      break;
    }
  }

  if (observer->targets != NULL) {
    C_MULTIPLATFORM_FREE(observer->targets);
  }

  C_MULTIPLATFORM_FREE(observer);
  return UI_ERROR_NONE;
}

/**
 * @brief Observes a target DOM node for mutations.
 * @param[in,out] observer The mutation observer.
 * @param[in,out] target The DOM node to observe.
 * @param[in] options The observation configuration options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_observe(struct ui_mutation_observer *observer,
                             struct ui_dom_node *target,
                             const struct ui_mutation_observer_init *options) {
  int i;
  struct ui_mutation_target_info *new_targets;

  if (observer == NULL || target == NULL || options == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < observer->target_count; i++) {
    if (observer->targets[i].target == target) {
      observer->targets[i].options = *options;
      return UI_ERROR_NONE;
    }
  }

  if (observer->target_count >= observer->target_capacity) {
    observer->target_capacity *= 2;
    new_targets = (struct ui_mutation_target_info *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_mutation_target_info) *
        (size_t)observer->target_capacity);
    if (new_targets == NULL) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_targets, observer->targets,
           sizeof(struct ui_mutation_target_info) *
               (size_t)observer->target_count);
    C_MULTIPLATFORM_FREE(observer->targets);
    observer->targets = new_targets;
  }

  observer->targets[observer->target_count].target = target;
  observer->targets[observer->target_count].options = *options;
  observer->target_count++;

  return UI_ERROR_NONE;
}

/**
 * @brief Disconnects a mutation observer, stopping all observation.
 * @param[in,out] observer The mutation observer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_disconnect(struct ui_mutation_observer *observer) {
  if (observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  observer->target_count = 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Checks if a node is an ancestor of another node.
 * @param[in] ancestor The potential ancestor node.
 * @param[in] node The child node.
 * @return 1 if true, 0 otherwise.
 */
static int is_ancestor(struct ui_dom_node *ancestor, struct ui_dom_node *node) {
  struct ui_dom_node *curr = node;
  while (curr != NULL) {
    if (curr == ancestor) {
      return 1;
    }
    curr = curr->parent;
  }
  return 0;
}

/**
 * @brief Dispatches a single mutation record to the observer's callback.
 * @param[in,out] observer The observer.
 * @param[in,out] record The mutation record.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t dispatch_record(struct ui_mutation_observer *observer,
                                  struct ui_mutation_record *record) {
  ui_error_t cb_rc =
      observer->callback(observer, record, 1, observer->user_data);
  {
    (void)cb_rc;
  }
  return cb_rc;
}

/**
 * @brief System entry point: Notifies observers of a child list mutation.
 * @param[in,out] target The parent node affected.
 * @param[in,out] added The node that was added (or NULL).
 * @param[in,out] removed The node that was removed (or NULL).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_mutation_observer_notify_child_list(struct ui_dom_node *target,
                                                  struct ui_dom_node *added,
                                                  struct ui_dom_node *removed) {
  int i, j;
  ui_error_t rc;
  struct ui_mutation_record record;

  if (target == NULL) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < MAX_GLOBAL_OBSERVERS; i++) {
    struct ui_mutation_observer *obs = g_observers[i];
    if (obs == NULL)
      continue;

    for (j = 0; j < obs->target_count; j++) {
      struct ui_mutation_target_info *tinfo = &obs->targets[j];
      int is_match = 0;
      if (!tinfo->options.child_list)
        continue;

      if (tinfo->target == target) {
        is_match = 1;
      } else if (tinfo->options.subtree) {
        if (is_ancestor(tinfo->target, target)) {
          is_match = 1;
        }
      }
      if (is_match) {
        record.type = UI_MUTATION_TYPE_CHILD_LIST;
        record.target = target;
        record.added_nodes = NULL;
        record.added_nodes_count = 0;
        record.removed_nodes = NULL;
        record.removed_nodes_count = 0;
        record.attribute_name = NULL;
        record.old_value = NULL;
        if (added != NULL) {
          record.added_nodes = &added;
          record.added_nodes_count = 1;
        }
        if (removed != NULL) {
          record.removed_nodes = &removed;
          record.removed_nodes_count = 1;
        }
        rc = dispatch_record(obs, &record);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        break; /* Only dispatch once per observer per mutation */
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief System entry point: Notifies observers of an attribute mutation.
 * @param[in,out] target The node affected.
 * @param[in] name The name of the attribute.
 * @param[in] old_value The previous value of the attribute.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_mutation_observer_notify_attribute(struct ui_dom_node *target,
                                                 const char *name,
                                                 const char *old_value) {
  int i, j;
  ui_error_t rc;
  struct ui_mutation_record record;
  char *old_val_copy = NULL;
  size_t old_val_len;

  if (target == NULL || name == NULL) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < MAX_GLOBAL_OBSERVERS; i++) {
    struct ui_mutation_observer *obs = g_observers[i];
    if (obs == NULL)
      continue;

    for (j = 0; j < obs->target_count; j++) {
      struct ui_mutation_target_info *tinfo = &obs->targets[j];
      int is_match = 0;
      if (!tinfo->options.attributes)
        continue;

      if (tinfo->target == target) {
        is_match = 1;
      } else if (tinfo->options.subtree) {
        if (is_ancestor(tinfo->target, target)) {
          is_match = 1;
        }
      }
      if (is_match) {
        record.type = UI_MUTATION_TYPE_ATTRIBUTES;
        record.target = target;
        record.added_nodes = NULL;
        record.added_nodes_count = 0;
        record.removed_nodes = NULL;
        record.removed_nodes_count = 0;
        record.attribute_name = NULL;
        record.old_value = NULL;

#if defined(_MSC_VER)
        {
          size_t name_len = strlen(name);
          record.attribute_name = (char *)C_MULTIPLATFORM_MALLOC(name_len + 1);
          if (record.attribute_name != NULL) {
            strcpy_s(record.attribute_name, name_len + 1, name);
          }
        }
#else
        {
          size_t name_len = strlen(name);
          record.attribute_name = (char *)C_MULTIPLATFORM_MALLOC(name_len + 1);
          if (record.attribute_name != NULL) {
            UI_STRCPY(record.attribute_name, sizeof(record.attribute_name),
                      name);
          }
        }
#endif

        if (tinfo->options.attribute_old_value && old_value != NULL) {
          old_val_len = strlen(old_value);
          old_val_copy = (char *)C_MULTIPLATFORM_MALLOC(old_val_len + 1);
          if (old_val_copy != NULL) {
#if defined(_MSC_VER)
            strcpy_s(old_val_copy, old_val_len + 1, old_value);
#else
            UI_STRCPY(old_val_copy, 256, old_value);
#endif
            record.old_value = old_val_copy;
          }
        }

        rc = dispatch_record(obs, &record);

        if (record.attribute_name != NULL) {
          C_MULTIPLATFORM_FREE(record.attribute_name);
        }
        if (record.old_value != NULL) {
          C_MULTIPLATFORM_FREE(record.old_value);
        }
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        break;
      }
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief System entry point: Notifies observers of a text content mutation.
 * @param[in,out] target The text node affected.
 * @param[in] old_value The previous text content.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_mutation_observer_notify_character_data(struct ui_dom_node *target,
                                           const char *old_value) {
  int i, j;
  ui_error_t rc;
  struct ui_mutation_record record;
  char *old_val_copy = NULL;
  size_t old_val_len;

  if (target == NULL) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < MAX_GLOBAL_OBSERVERS; i++) {
    struct ui_mutation_observer *obs = g_observers[i];
    if (obs == NULL)
      continue;

    for (j = 0; j < obs->target_count; j++) {
      struct ui_mutation_target_info *tinfo = &obs->targets[j];
      int is_match = 0;
      if (!tinfo->options.character_data)
        continue;

      if (tinfo->target == target) {
        is_match = 1;
      } else if (tinfo->options.subtree) {
        if (is_ancestor(tinfo->target, target)) {
          is_match = 1;
        }
      }
      if (is_match) {
        record.type = UI_MUTATION_TYPE_CHARACTER_DATA;
        record.target = target;
        record.added_nodes = NULL;
        record.added_nodes_count = 0;
        record.removed_nodes = NULL;
        record.removed_nodes_count = 0;
        record.attribute_name = NULL;
        record.old_value = NULL;

        if (old_value != NULL) {
          old_val_len = strlen(old_value);
          old_val_copy = (char *)C_MULTIPLATFORM_MALLOC(old_val_len + 1);
          if (old_val_copy != NULL) {
#if defined(_MSC_VER)
            strcpy_s(old_val_copy, old_val_len + 1, old_value);
#else
            UI_STRCPY(old_val_copy, 256, old_value);
#endif
            record.old_value = old_val_copy;
          }
        }

        rc = dispatch_record(obs, &record);

        if (record.old_value != NULL) {
          C_MULTIPLATFORM_FREE(record.old_value);
        }
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        break;
      }
    }
  }
  return UI_ERROR_NONE;
}
