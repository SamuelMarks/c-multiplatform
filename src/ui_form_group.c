/*
 * \file ui_form_group.c
 * \brief Implementation of form group nodes.
 */
/* clang-format off */
#include "ui_form_group.h"
#include "ui_form_node_internal.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @struct ui_form_group_entry
 * \struct ui_form_group_entry
 * \brief An entry mapping a name to a form node within a group.
 */
struct ui_form_group_entry {
  char *name;          /**< name */
  ui_form_node_t node; /**< node */
};

/**
 * @struct ui_form_group
 * \struct ui_form_group
 * \brief Represents a group of named form nodes.
 */
struct ui_form_group {
  struct ui_arena *arena;   /**< arena */
  enum ui_signal_mode mode; /**< mode */

  struct ui_form_group_entry *entries; /**< entries */
  size_t count;                        /**< count */
  size_t capacity;                     /**< capacity */
};

/*
 * \brief Creates a new form group.
 * \param[in,out] arena The memory arena.
 * \param[in] mode The signaling mode.
 * \param[out] out_group Pointer to store the created form group.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_create(struct ui_arena *arena,
                                enum ui_signal_mode mode,
                                ui_form_group_t **out_group) {
  struct ui_form_group *group;
  ui_error_t rc;

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

/*
 * \brief Duplicates a string using an arena allocator.
 * \param[in,out] arena The arena to allocate from.
 * \param[in] str The string to copy.
 * \param[out] out_str Pointer to store the copied string.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t strdup_arena(struct ui_arena *arena, const char *str,
                               char **out_str) {
  size_t len = strlen(str);
  char *copy;
  ui_error_t rc = ui_arena_alloc(arena, len + 1, 1, (void **)&copy);
  if (rc == UI_ERROR_NONE) {
    memcpy(copy, str, len + 1);
    *out_str = copy;
  }
  return rc;
}

/*
 * \brief Adds a form node to the group with a specified name.
 * \param[in,out] group The form group.
 * \param[in] name The name for the node.
 * \param[in] node The form node to add.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_add_node(ui_form_group_t *group, const char *name,
                                  ui_form_node_t node) {
  ui_error_t rc;
  struct ui_form_group_entry *new_entries = NULL;
  size_t new_cap;

  if (!group || !name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->count >= group->capacity) {
    new_cap = group->capacity == 0 ? 4 : group->capacity * 2;
    ui_arena_alloc(group->arena,
                   (size_t)new_cap * sizeof(struct ui_form_group_entry), 8,
                   (void **)&new_entries);
    if (!new_entries)
      return UI_ERROR_OUT_OF_MEMORY;

    if (group->count > 0) {
      memcpy(new_entries, group->entries,
             (size_t)group->count * sizeof(struct ui_form_group_entry));
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

/*
 * \brief Adds a form control directly to the group.
 * \param[in,out] group The form group.
 * \param[in] name The name for the control.
 * \param[in,out] control The form control to add.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_add_control(ui_form_group_t *group, const char *name,
                                     ui_form_control_t *control) {
  ui_form_node_t node = {0};
  node.type = UI_FORM_NODE_CONTROL;
  node.node.control = control;
  return ui_form_group_add_node(group, name, node);
}

/*
 * \brief Gets a form node by name from the group.
 * \param[in] group The form group.
 * \param[in] name The name of the node.
 * \param[out] out_node Pointer to store the retrieved node.
 * \return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t ui_form_group_get_node(ui_form_group_t *group, const char *name,
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

/*
 * \brief Gets a form control by name from the group.
 * \param[in] group The form group.
 * \param[in] name The name of the control.
 * \param[out] out_control Pointer to store the retrieved control.
 * \return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
ui_error_t ui_form_group_get_control(ui_form_group_t *group, const char *name,
                                     ui_form_control_t **out_control) {
  ui_form_node_t node = {0};
  ui_error_t rc = ui_form_group_get_node(group, name, &node);
  if (rc != UI_ERROR_NONE)
    return rc;

  if (node.type != UI_FORM_NODE_CONTROL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_control = node.node.control;
  return UI_ERROR_NONE;
}

/*
 * \brief Internal function to recursively get the status of a form group.
 * \param[in] group The form group.
 * \param[out] out_status Pointer to store the aggregated status.
 * \param[in] depth The current recursion depth.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t _ui_form_group_get_status_internal(struct ui_form_group *group,
                                              enum ui_form_status *out_status,
                                              size_t depth) {
  size_t i;
  enum ui_form_status aggregated_status = UI_FORM_STATUS_VALID;
  enum ui_form_status child_status;
  ui_error_t rc;

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

/*
 * \brief Gets the overall validation status of the form group.
 * \param[in] group The form group.
 * \param[out] out_status Pointer to store the status.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_get_status(ui_form_group_t *group,
                                    enum ui_form_status *out_status) {
  return _ui_form_group_get_status_internal(group, out_status, 0);
}

/*
 * \brief Checks if the entire form group is valid.
 * \param[in] group The form group.
 * \param[out] out_valid Set to UI_TRUE if valid, UI_FALSE otherwise.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_is_valid(ui_form_group_t *group,
                                  ui_bool_t *out_valid) {
  enum ui_form_status status;
  ui_error_t rc = ui_form_group_get_status(group, &status);
  if (rc != UI_ERROR_NONE)
    return rc;
  *out_valid = (status == UI_FORM_STATUS_VALID);
  return UI_ERROR_NONE;
}

/*
 * \brief Destroys a form group.
 * \param[in,out] group The form group to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_group_destroy(ui_form_group_t *group) {
  if (!group)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}
