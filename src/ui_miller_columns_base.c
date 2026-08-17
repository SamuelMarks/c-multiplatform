/**
 * \file ui_miller_columns_base.c
 * \brief Implementation of Miller Columns for hierarchical browsing.
 */
/* clang-format off */
#include "ui_miller_columns_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include "ui_internal_mem.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

#define UI_MILLER_MAX_DEPTH 32

/**
 * \struct ui_miller_columns_base
 * \brief State and tree context for a Miller Columns navigation widget.
 */
struct ui_miller_columns_base {
  struct ui_arena *arena;
  struct ui_tree_model tree_model;
  void *model_user_data;

  struct ui_miller_column_state columns[UI_MILLER_MAX_DEPTH];
  int active_column_count;
  int focused_column_index;

  ui_signal_t *topology_changed_signal;
};

/**
 * \brief Signal equality callback for column topology changes.
 * \param[in] a The first payload.
 * \param[in] b The second payload.
 * \param[out] out_equal Pointer to store the boolean equality result.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t topology_equality(union ui_signal_payload a,
                                    union ui_signal_payload b,
                                    ui_bool_t *out_equal) {
  *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/** \brief ui_miller_columns_base_create */
/**
 * \brief Creates a new Miller columns base widget.
 * \param[in,out] arena The memory arena.
 * \param[in] tree_model The hierarchical tree model.
 * \param[in] model_user_data User data for the tree model.
 * \param[out] out_miller Pointer to store the created widget.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_miller_columns_base_create(
    struct ui_arena *arena, const struct ui_tree_model *tree_model,
    void *model_user_data, struct ui_miller_columns_base **out_miller) {
  ui_error_t err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !tree_model || !out_miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_miller_columns_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  memset(ptr, 0, sizeof(struct ui_miller_columns_base));

  *out_miller = (struct ui_miller_columns_base *)ptr;
  (*out_miller)->arena = arena;
  (*out_miller)->tree_model = *tree_model;
  (*out_miller)->model_user_data = model_user_data;
  (*out_miller)->active_column_count = 1; /* Always start with root column */
  (*out_miller)->focused_column_index = 0;

  /* Initialize root column */
  (*out_miller)->columns[0].column_index = 0;
  (*out_miller)->columns[0].parent_node_id = NULL; /* Root */
  (*out_miller)->columns[0].selected_child_id = NULL;

  initial_payload.int_val = 1;
  {
    ui_error_t _ign_rc = ui_signal_create(
        arena, initial_payload, UI_SIGNAL_TYPE_INT32, topology_equality, NULL,
        UI_SIGNAL_MODE_SINGLE_THREADED,
        &(*out_miller)->topology_changed_signal);
    (void)_ign_rc;
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a Miller columns base widget.
 * \param[in,out] miller The widget to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_miller_columns_base_destroy(struct ui_miller_columns_base *miller) {
  if (!miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (miller->topology_changed_signal) {
    (void)ui_signal_destroy(miller->topology_changed_signal);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Selects an item in a specific column, dynamically spawning child
 * columns if necessary.
 * \param[in,out] miller The Miller columns widget.
 * \param[in] column_index The index of the column where the selection occurred.
 * \param[in] node_id The ID of the selected node.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_miller_columns_base_select_item(struct ui_miller_columns_base *miller,
                                   int column_index, void *node_id) {

  union ui_signal_payload payload;
  size_t child_count = 0;

  if (!miller || column_index < 0 ||
      column_index >= miller->active_column_count || !node_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Update selection in the target column */
  miller->columns[column_index].selected_child_id = node_id;
  miller->focused_column_index = column_index;

  /* 2. Prune any columns beyond the next child level */
  miller->active_column_count = column_index + 1;

  /* 3. Query the tree model to see if this node has children */
  if (miller->tree_model.get_child_count) {
    child_count =
        miller->tree_model.get_child_count(node_id, miller->model_user_data);
  }

  /* 4. If it has children, spawn the next column */
  if (child_count > 0) {
    if (miller->active_column_count < UI_MILLER_MAX_DEPTH) {
      int next_col = miller->active_column_count;
      miller->columns[next_col].column_index = next_col;
      miller->columns[next_col].parent_node_id = node_id;
      miller->columns[next_col].selected_child_id = NULL;
      miller->active_column_count++;

      /* Automatically focus the newly spawned child column */
      miller->focused_column_index = next_col;
    } else {
      return UI_ERROR_OUT_OF_BOUNDS;
    }
  }

  /* Emit topology change signal */
  payload.int_val = miller->active_column_count;
  {
    ui_error_t set_rc = ui_signal_set(miller->topology_changed_signal, payload);
    { (void)set_rc; }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Navigates focus to the column on the left (parent level).
 * \param[in,out] miller The Miller columns widget.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_miller_columns_base_navigate_left(struct ui_miller_columns_base *miller) {
  if (!miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (miller->focused_column_index > 0) {
    miller->focused_column_index--;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_OUT_OF_BOUNDS;
}

/**
 * \brief Navigates focus to the column on the right (child level).
 * \param[in,out] miller The Miller columns widget.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_miller_columns_base_navigate_right(struct ui_miller_columns_base *miller) {
  if (!miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* We can only navigate right if a child column actually exists (was spawned
   * by a selection) */
  if (miller->focused_column_index < miller->active_column_count - 1) {
    miller->focused_column_index++;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_OUT_OF_BOUNDS;
}

/** \brief ui_miller_columns_base_get_column_count */
/**
 * \brief Gets the current number of active columns.
 * \param[in] miller The Miller columns widget.
 * \param[out] out_count Pointer to store the column count.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_miller_columns_base_get_column_count(
    const struct ui_miller_columns_base *miller, int *out_count) {
  if (!miller || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = miller->active_column_count;
  return UI_ERROR_NONE;
}

/** \brief ui_miller_columns_base_get_topology_changed_signal */
/**
 * \brief Gets the reactive signal that emits when the column topology changes.
 * \param[in,out] miller The Miller columns widget.
 * \param[out] out_signal Pointer to store the signal.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_miller_columns_base_get_topology_changed_signal(
    struct ui_miller_columns_base *miller, ui_signal_t **out_signal) {
  if (!miller || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = miller->topology_changed_signal;
  return UI_ERROR_NONE;
}
