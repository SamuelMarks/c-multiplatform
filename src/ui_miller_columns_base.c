/* clang-format off */
#include "ui_miller_columns_base.h"
#include "ui_arena.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#define UI_MILLER_MAX_DEPTH 32

struct ui_miller_columns_base {
  struct ui_arena *arena;
  struct ui_tree_model tree_model;
  void *model_user_data;

  struct ui_miller_column_state columns[UI_MILLER_MAX_DEPTH];
  int active_column_count;
  int focused_column_index;

  ui_signal_t *topology_changed_signal;
};

static enum ui_error topology_equality(union ui_signal_payload a,
                                       union ui_signal_payload b,
                                       ui_bool_t *out_equal) {
  if (out_equal)
    *out_equal = (a.int_val == b.int_val) ? UI_TRUE : UI_FALSE;
  return UI_ERROR_NONE;
}

/** \brief ui_miller_columns_base_create */
enum ui_error ui_miller_columns_base_create(
    struct ui_arena *arena, const struct ui_tree_model *tree_model,
    void *model_user_data, struct ui_miller_columns_base **out_miller) {
  enum ui_error err;
  void *ptr;
  union ui_signal_payload initial_payload;

  if (!arena || !tree_model || !out_miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_arena_alloc(arena, sizeof(struct ui_miller_columns_base), 8, &ptr);
  if (err != UI_ERROR_NONE) {
    return err;
  }

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
  (void)ui_signal_create(
      arena, initial_payload, UI_SIGNAL_TYPE_INT32, topology_equality, NULL,
      UI_SIGNAL_MODE_SINGLE_THREADED, &(*out_miller)->topology_changed_signal);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_miller_columns_base_destroy(struct ui_miller_columns_base *miller) {
  if (!miller) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (miller->topology_changed_signal) {
    ui_signal_destroy(miller->topology_changed_signal);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
  (void)ui_signal_set(miller->topology_changed_signal, payload);

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
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
enum ui_error ui_miller_columns_base_get_column_count(
    const struct ui_miller_columns_base *miller, int *out_count) {
  if (!miller || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = miller->active_column_count;
  return UI_ERROR_NONE;
}

/** \brief ui_miller_columns_base_get_topology_changed_signal */
enum ui_error ui_miller_columns_base_get_topology_changed_signal(
    struct ui_miller_columns_base *miller, ui_signal_t **out_signal) {
  if (!miller || !out_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_signal = miller->topology_changed_signal;
  return UI_ERROR_NONE;
}
