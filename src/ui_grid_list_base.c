/* clang-format off */
#include "ui_grid_list_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

struct ui_grid_list_base {
  int columns;
  struct ui_grid_list_item *items;
  size_t item_count;
  size_t item_capacity;
  struct ui_computed *data_signal;
};

ui_error_t ui_grid_list_base_create(struct ui_grid_list_base **out_grid_list,
                                    int columns) {
  struct ui_grid_list_base *gl;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_grid_list || columns < 1) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  gl = (struct ui_grid_list_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_grid_list_base));
  if (!gl) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  gl->columns = columns;
  gl->items = NULL;
  gl->item_count = 0;
  gl->item_capacity = 0;

  *out_grid_list = gl;

cleanup:
  return rc;
}

ui_error_t ui_grid_list_base_destroy(struct ui_grid_list_base *grid_list) {
  if (!grid_list) {
    return UI_ERROR_NONE;
  }
  if (grid_list->items) {
    C_MULTIPLATFORM_FREE(grid_list->items);
  }
  C_MULTIPLATFORM_FREE(grid_list);
  return UI_ERROR_NONE;
}

ui_error_t ui_grid_list_base_set_columns(struct ui_grid_list_base *grid_list,
                                         int columns) {
  if (!grid_list || columns < 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  grid_list->columns = columns;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_grid_list_base_get_columns(const struct ui_grid_list_base *grid_list,
                              int *out_columns) {
  if (!grid_list || !out_columns) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_columns = grid_list->columns;
  return UI_ERROR_NONE;
}

ui_error_t ui_grid_list_base_add_item(struct ui_grid_list_base *grid_list,
                                      int rowspan, int colspan) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t new_cap;
  struct ui_grid_list_item *new_items;

  if (!grid_list || rowspan < 1 || colspan < 1) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (grid_list->item_count == grid_list->item_capacity) {
    new_cap = grid_list->item_capacity == 0 ? 4 : grid_list->item_capacity * 2;
    new_items = (struct ui_grid_list_item *)C_MULTIPLATFORM_MALLOC(
        new_cap * sizeof(struct ui_grid_list_item));
    if (!new_items) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    if (grid_list->items) {
      size_t i;
      for (i = 0; i < grid_list->item_count; i++) {
        new_items[i] = grid_list->items[i];
      }
      C_MULTIPLATFORM_FREE(grid_list->items);
    }

    grid_list->items = new_items;
    grid_list->item_capacity = new_cap;
  }

  grid_list->items[grid_list->item_count].rowspan = rowspan;
  grid_list->items[grid_list->item_count].colspan = colspan;
  grid_list->item_count++;

cleanup:
  return rc;
}

/** \brief ui_error */
ui_error_t
ui_grid_list_base_get_item_count(const struct ui_grid_list_base *grid_list,
                                 size_t *out_count) {
  if (!grid_list || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_count = grid_list->item_count;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_grid_list_base_get_item(const struct ui_grid_list_base *grid_list,
                           size_t index,
                           const struct ui_grid_list_item **out_item) {
  if (!grid_list || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (index >= grid_list->item_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  *out_item = &grid_list->items[index];
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_grid_list_base_calculate_rows(const struct ui_grid_list_base *grid_list,
                                 int *out_rows) {
  ui_error_t rc = UI_ERROR_NONE;
  int *col_occupancy = NULL;
  int c;
  size_t i;
  int max_rows = 0;

  if (!grid_list || !out_rows) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  col_occupancy =
      (int *)C_MULTIPLATFORM_MALLOC(grid_list->columns * sizeof(int));
  if (!col_occupancy) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  for (c = 0; c < grid_list->columns; c++) {
    col_occupancy[c] = 0;
  }

  for (i = 0; i < grid_list->item_count; i++) {
    const struct ui_grid_list_item *item = &grid_list->items[i];
    int cspan = item->colspan;
    int rspan = item->rowspan;
    int r = 0;
    int found = 0;
    int found_c = 0;
    int found_r = 0;

    if (cspan > grid_list->columns) {
      cspan = grid_list->columns;
    }

    while (!found) {
      for (c = 0; c <= grid_list->columns - cspan; c++) {
        int valid = 1;
        int j;
        for (j = c; j < c + cspan; j++) {
          if (col_occupancy[j] > r) {
            valid = 0;
            break;
          }
        }
        if (valid) {
          found = 1;
          found_c = c;
          found_r = r;
          break;
        }
      }
      if (!found) {
        r++;
      }
    }

    for (c = found_c; c < found_c + cspan; c++) {
      col_occupancy[c] = found_r + rspan;
      if (col_occupancy[c] > max_rows) {
        max_rows = col_occupancy[c];
      }
    }
  }

  *out_rows = max_rows;

cleanup:
  if (col_occupancy) {
    C_MULTIPLATFORM_FREE(col_occupancy);
  }
  return rc;
}

ui_error_t ui_grid_list_base_bind_data(struct ui_grid_list_base *widget,
                                       struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
