/* clang-format off */
#include "ui_form_group.h"
#include "ui_form_node_internal.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_form_group_entry {
  char *name;
  ui_form_node_t node;
};

/** \brief ui_form_group */
struct ui_form_group {
  struct ui_arena *arena;
  enum ui_signal_mode mode;

  struct ui_form_group_entry *entries;
  size_t count;
  size_t capacity;
};

enum ui_error ui_form_group_create(struct ui_arena *arena,
                                   enum ui_signal_mode mode,
                                   ui_form_group_t **out_group) {
  struct ui_form_group *group;
  enum ui_error rc;

  if (!arena || !out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_arena_alloc(arena, sizeof(struct ui_form_group), 8, (void **)&group);
  if (rc != UI_ERROR_NONE)
    return rc;

  group->arena = arena;
  group->mode = mode;
  group->entries = NULL;
  group->count = 0;
  group->capacity = 0;

  *out_group = group;
  return UI_ERROR_NONE;
}

static enum ui_error strdup_arena(struct ui_arena *arena, const char *str,
                                  char **out_str) {
  size_t len = strlen(str);
  char *copy;
  enum ui_error rc = ui_arena_alloc(arena, len + 1, 1, (void **)&copy);
  if (rc == UI_ERROR_NONE) {
    memcpy(copy, str, len + 1);
    *out_str = copy;
  }
  return rc;
}

enum ui_error ui_form_group_add_node(ui_form_group_t *group, const char *name,
                                     ui_form_node_t node) {
  enum ui_error rc;
  struct ui_form_group_entry *new_entries = NULL;
  size_t new_cap;

  if (!group || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->count >= group->capacity) {
    new_cap = group->capacity == 0 ? 4 : group->capacity * 2;
    ui_arena_alloc(group->arena, new_cap * sizeof(struct ui_form_group_entry),
                   8, (void **)&new_entries);
    if (!new_entries)
      return UI_ERROR_OUT_OF_MEMORY;

    if (group->count > 0) {
      memcpy(new_entries, group->entries,
             group->count * sizeof(struct ui_form_group_entry));
    }
    group->entries = new_entries;
    group->capacity = new_cap;
  }

  rc = strdup_arena(group->arena, name, &group->entries[group->count].name);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  group->entries[group->count].node = node;
  group->count++;

  return UI_ERROR_NONE;
}

enum ui_error ui_form_group_add_control(ui_form_group_t *group,
                                        const char *name,
                                        ui_form_control_t *control) {
  ui_form_node_t node = {0};
  node.type = UI_FORM_NODE_CONTROL;
  node.node.control = control;
  return ui_form_group_add_node(group, name, node);
}

enum ui_error ui_form_group_get_node(ui_form_group_t *group, const char *name,
                                     ui_form_node_t *out_node) {
  size_t i;
  if (!group || !name || !out_node)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < group->count; i++) {
    if (strcmp(group->entries[i].name, name) == 0) {
      *out_node = group->entries[i].node;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

enum ui_error ui_form_group_get_control(ui_form_group_t *group,
                                        const char *name,
                                        ui_form_control_t **out_control) {
  ui_form_node_t node = {0};
  enum ui_error rc = ui_form_group_get_node(group, name, &node);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (node.type != UI_FORM_NODE_CONTROL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_control = node.node.control;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
_ui_form_group_get_status_internal(struct ui_form_group *group,
                                   enum ui_form_status *out_status,
                                   size_t depth) {
  size_t i;
  enum ui_form_status aggregated_status = UI_FORM_STATUS_VALID;
  enum ui_form_status child_status;
  enum ui_error rc;

  if (!group || !out_status)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < group->count; i++) {
    rc = _ui_form_node_get_status_internal(group->entries[i].node,
                                           &child_status, depth);
    if (rc != UI_ERROR_NONE)
      return rc;

    if (child_status == UI_FORM_STATUS_INVALID) {
      aggregated_status = UI_FORM_STATUS_INVALID;
      break; /* Fast fail */
    } else if (child_status == UI_FORM_STATUS_PENDING) {
      aggregated_status = UI_FORM_STATUS_PENDING;
    }
  }

  *out_status = aggregated_status;
  return UI_ERROR_NONE;
}

enum ui_error ui_form_group_get_status(ui_form_group_t *group,
                                       enum ui_form_status *out_status) {
  return _ui_form_group_get_status_internal(group, out_status, 0);
}

enum ui_error ui_form_group_is_valid(ui_form_group_t *group,
                                     ui_bool_t *out_valid) {
  enum ui_form_status status;
  enum ui_error rc = ui_form_group_get_status(group, &status);
  if (rc != UI_ERROR_NONE)
    return rc;
  *out_valid = (status == UI_FORM_STATUS_VALID);
  return UI_ERROR_NONE;
}

enum ui_error ui_form_group_destroy(ui_form_group_t *group) {
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}
