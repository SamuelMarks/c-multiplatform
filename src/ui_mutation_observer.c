/* clang-format off */
#include "ui_mutation_observer.h"
#include "ui_internal_mem.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

#define MAX_GLOBAL_OBSERVERS 64

static struct ui_mutation_observer *g_observers[MAX_GLOBAL_OBSERVERS] = {NULL};

/** \brief ui_mutation_target_info */
struct ui_mutation_target_info {
  struct ui_dom_node *target;
  struct ui_mutation_observer_init options;
};

/** \brief ui_mutation_observer */
struct ui_mutation_observer {
  ui_mutation_observer_cb_t callback;
  void *user_data;
  struct ui_mutation_target_info *targets;
  int target_count;
  int target_capacity;
};

/** \brief ui_error */
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
      sizeof(struct ui_mutation_target_info) * obs->target_capacity);
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

/** \brief ui_error */
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
        sizeof(struct ui_mutation_target_info) * observer->target_capacity);
    if (new_targets == NULL) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_targets, observer->targets,
           sizeof(struct ui_mutation_target_info) * observer->target_count);
    C_MULTIPLATFORM_FREE(observer->targets);
    observer->targets = new_targets;
  }

  observer->targets[observer->target_count].target = target;
  observer->targets[observer->target_count].options = *options;
  observer->target_count++;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_mutation_observer_disconnect(struct ui_mutation_observer *observer) {
  if (observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  observer->target_count = 0;
  return UI_ERROR_NONE;
}

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

static ui_error_t dispatch_record(struct ui_mutation_observer *observer,
                                  struct ui_mutation_record *record) {
  ui_error_t cb_rc =
      observer->callback(observer, record, 1, observer->user_data);
  { (void)cb_rc; }
  return cb_rc;
}

/** \brief ui_error */
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
      if (!tinfo->options.child_list)
        continue;

      if (tinfo->target == target ||
          (tinfo->options.subtree && is_ancestor(tinfo->target, target))) {
        memset(&record, 0, sizeof(record));
        record.type = UI_MUTATION_TYPE_CHILD_LIST;
        record.target = target;
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
      if (!tinfo->options.attributes)
        continue;

      if (tinfo->target == target ||
          (tinfo->options.subtree && is_ancestor(tinfo->target, target))) {
        memset(&record, 0, sizeof(record));
        record.type = UI_MUTATION_TYPE_ATTRIBUTES;
        record.target = target;

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
            strcpy(record.attribute_name, name);
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
            strcpy(old_val_copy, old_value);
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

/** \brief ui_error */
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
      if (!tinfo->options.character_data)
        continue;

      if (tinfo->target == target ||
          (tinfo->options.subtree && is_ancestor(tinfo->target, target))) {
        memset(&record, 0, sizeof(record));
        record.type = UI_MUTATION_TYPE_CHARACTER_DATA;
        record.target = target;

        if (old_value != NULL) {
          old_val_len = strlen(old_value);
          old_val_copy = (char *)C_MULTIPLATFORM_MALLOC(old_val_len + 1);
          if (old_val_copy != NULL) {
#if defined(_MSC_VER)
            strcpy_s(old_val_copy, old_val_len + 1, old_value);
#else
            strcpy(old_val_copy, old_value);
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
