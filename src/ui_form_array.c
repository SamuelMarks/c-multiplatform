/**
 * \file ui_form_array.c
 * \brief Implementation of form array nodes.
 */
/* clang-format off */
#include "ui_form_array.h"
#include "ui_form_node_internal.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * \struct ui_form_array
 * \brief Represents an array of form nodes.
 */
struct ui_form_array {
  struct ui_arena *arena;
  enum ui_signal_mode mode;

  ui_form_node_t *nodes;
  size_t count;
  size_t capacity;
};

/**
 * \brief Creates a new form array.
 * \param[in,out] arena The memory arena.
 * \param[in] mode The signaling mode.
 * \param[out] out_array Pointer to store the created form array.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_create(struct ui_arena *arena,
                                enum ui_signal_mode mode,
                                ui_form_array_t **out_array) {
  struct ui_form_array *array;
  ui_error_t rc;

  if (!arena || !out_array) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_arena_alloc(arena, sizeof(struct ui_form_array), 8, (void **)&array);
  if (rc != UI_ERROR_NONE)
    return rc;

  array->arena = arena;
  array->mode = mode;
  array->nodes = NULL;
  array->count = 0;
  array->capacity = 0;

  *out_array = array;
  return UI_ERROR_NONE;
}

/**
 * \brief Pushes a form node to the end of the array.
 * \param[in,out] array The form array.
 * \param[in] node The form node to push.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_push(ui_form_array_t *array, ui_form_node_t node) {
  ui_form_node_t *new_nodes = NULL;
  size_t new_cap;

  if (!array) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (array->count >= array->capacity) {
    new_cap = array->capacity == 0 ? 4 : array->capacity * 2;
    ui_arena_alloc(array->arena, new_cap * sizeof(ui_form_node_t), 8,
                   (void **)&new_nodes);
    if (!new_nodes)
      return UI_ERROR_OUT_OF_MEMORY;

    if (array->count > 0) {
      memcpy(new_nodes, array->nodes, array->count * sizeof(ui_form_node_t));
    }
    array->nodes = new_nodes;
    array->capacity = new_cap;
  }

  array->nodes[array->count] = node;
  array->count++;

  return UI_ERROR_NONE;
}

/**
 * \brief Inserts a form node at a specific index.
 * \param[in,out] array The form array.
 * \param[in] index The index to insert at.
 * \param[in] node The form node to insert.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_insert_at(ui_form_array_t *array, size_t index,
                                   ui_form_node_t node) {
  ui_form_node_t *new_nodes = NULL;
  size_t new_cap, i;

  if (!array)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index > array->count)
    return UI_ERROR_OUT_OF_BOUNDS;

  if (array->count >= array->capacity) {
    new_cap = array->capacity == 0 ? 4 : array->capacity * 2;
    ui_arena_alloc(array->arena, new_cap * sizeof(ui_form_node_t), 8,
                   (void **)&new_nodes);
    if (!new_nodes)
      return UI_ERROR_OUT_OF_MEMORY;

    if (array->count > 0) {
      memcpy(new_nodes, array->nodes, array->count * sizeof(ui_form_node_t));
    }
    array->nodes = new_nodes;
    array->capacity = new_cap;
  }

  for (i = array->count; i > index; i--) {
    array->nodes[i] = array->nodes[i - 1];
  }

  array->nodes[index] = node;
  array->count++;

  return UI_ERROR_NONE;
}

/**
 * \brief Removes a form node at a specific index.
 * \param[in,out] array The form array.
 * \param[in] index The index to remove from.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_remove_at(ui_form_array_t *array, size_t index) {
  size_t i;
  if (!array)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index >= array->count)
    return UI_ERROR_OUT_OF_BOUNDS;

  for (i = index; i < array->count - 1; i++) {
    array->nodes[i] = array->nodes[i + 1];
  }

  array->count--;
  return UI_ERROR_NONE;
}

/**
 * \brief Clears all form nodes from the array.
 * \param[in,out] array The form array.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_clear(ui_form_array_t *array) {
  if (!array)
    return UI_ERROR_INVALID_ARGUMENT;
  array->count = 0;
  return UI_ERROR_NONE;
}

/**
 * \brief Gets a form node at a specific index.
 * \param[in] array The form array.
 * \param[in] index The index of the node.
 * \param[out] out_node Pointer to store the retrieved node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_get_node(ui_form_array_t *array, size_t index,
                                  ui_form_node_t *out_node) {
  if (!array || !out_node)
    return UI_ERROR_INVALID_ARGUMENT;
  if (index >= array->count)
    return UI_ERROR_OUT_OF_BOUNDS;

  *out_node = array->nodes[index];
  return UI_ERROR_NONE;
}

/**
 * \brief Internal function to get the validation status of a form array
 * recursively.
 * \param[in] array The form array.
 * \param[out] out_status Pointer to store the aggregated status.
 * \param[in] depth The current recursion depth.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t _ui_form_array_get_status_internal(struct ui_form_array *array,
                                              enum ui_form_status *out_status,
                                              size_t depth) {
  size_t i;
  enum ui_form_status aggregated_status = UI_FORM_STATUS_VALID;
  enum ui_form_status child_status;
  ui_error_t rc;

  if (!array || !out_status)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < array->count; i++) {
    rc = _ui_form_node_get_status_internal(array->nodes[i], &child_status,
                                           depth);
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

/**
 * \brief Gets the aggregated validation status of a form array.
 * \param[in] array The form array.
 * \param[out] out_status Pointer to store the status.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_get_status(ui_form_array_t *array,
                                    enum ui_form_status *out_status) {
  return _ui_form_array_get_status_internal(array, out_status, 0);
}

/**
 * \brief Destroys a form array.
 * \param[in,out] array The form array to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_form_array_destroy(ui_form_array_t *array) {
  if (!array)
    return UI_ERROR_INVALID_ARGUMENT;
  return UI_ERROR_NONE;
}
