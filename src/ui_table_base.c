/**
 * @file ui_table_base.c
 * @brief Implementation of the table base component.
 */

/* clang-format off */
#include "ui_table_base.h"
#include "ui_internal_mem.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
/** @brief Internal mock countdown for append child failures. */
extern int g_mock_append_child_fail_countdown;

/**
 * @brief Mock implementation of ui_dom_node_append_child.
 * @param parent Parent node.
 * @param child Child node.
 * @return Mocked error code.
 */
static ui_error_t mock_ui_dom_node_append_child(struct ui_dom_node *parent,
                                                struct ui_dom_node *child) {
  if (g_mock_append_child_fail_countdown == 0) {
    g_mock_append_child_fail_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  if (g_mock_append_child_fail_countdown > 0) {
    g_mock_append_child_fail_countdown--;
  }
  return ui_dom_node_append_child(parent, child);
}
/** @cond */
#define ui_dom_node_append_child mock_ui_dom_node_append_child
/** @endcond */
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * @struct ui_table_base
 * @brief Internal implementation of the table base component.
 */
struct ui_table_base {
  struct ui_table_model
      model; /**< Data model containing rows, columns, and rendering logic. */
  struct ui_table_column_config
      *col_configs; /**< Array of column configurations. */
  size_t num_cols;  /**< Number of columns. */
  struct ui_table_sort_config sort_config; /**< Active sort configuration. */
  struct ui_table_pagination_config
      pagination_config; /**< Active pagination configuration. */
  struct ui_selection_model
      *selection_model;            /**< Associated selection model. */
  struct ui_computed *data_signal; /**< Bound data signal. */
};

/**
 * @brief Creates a new table base component.
 * @param[out] out_table Pointer to store the created table.
 * @param[in] model The table data model.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_table_base_create(struct ui_table_base **out_table,
                                const struct ui_table_model *model) {
  struct ui_table_base *table;
  size_t num_cols;
  size_t i;
  ui_error_t rc;

  if (!out_table || !model || !model->get_row_count ||
      !model->get_column_count || !model->render_cell ||
      !model->render_header) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table = (struct ui_table_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_table_base));
  if (!table)
    return UI_ERROR_OUT_OF_MEMORY;

  table->model = *model;
  table->sort_config.active_column_index = 0;
  table->sort_config.direction = UI_TABLE_SORT_NONE;
  table->pagination_config.page_size =
      0; /* 0 means all items (no pagination) */
  table->pagination_config.current_page = 0;

  rc = ui_selection_model_create(&table->selection_model);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(table);
    return rc;
  }

  num_cols = model->get_column_count(model->user_data);
  table->num_cols = num_cols;

  if (num_cols > 0) {
    table->col_configs =
        (struct ui_table_column_config *)C_MULTIPLATFORM_MALLOC(
            (size_t)num_cols * sizeof(struct ui_table_column_config));
    if (!table->col_configs) {
      ui_selection_model_destroy(table->selection_model);
      C_MULTIPLATFORM_FREE(table);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < num_cols; i++) {
      table->col_configs[i].sizing = UI_TABLE_COLUMN_FLEX;
      table->col_configs[i].width = 1.0f;
      table->col_configs[i].min_width = 0.0f;
      table->col_configs[i].max_width = 0.0f; /* 0 = infinite */
    }
  } else {
    table->col_configs = NULL;
  }

  *out_table = table;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a table base component.
 * @param[in,out] table The table to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_table_base_destroy(struct ui_table_base *table) {
  if (!table)
    return UI_ERROR_NONE;
  ui_selection_model_destroy(table->selection_model);
  if (table->col_configs) {
    C_MULTIPLATFORM_FREE(table->col_configs);
  }
  C_MULTIPLATFORM_FREE(table);
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the selection model of the table.
 * @param[in,out] table The table.
 * @param[out] out_model Pointer to store the selection model.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_table_base_get_selection_model(struct ui_table_base *table,
                                  struct ui_selection_model **out_model) {
  if (!table || !out_model)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_model = table->selection_model;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the configuration for a specific column.
 * @param[in,out] table The table.
 * @param[in] col_index The index of the column to configure.
 * @param[in] config The column configuration.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_table_base_set_column_config(struct ui_table_base *table, size_t col_index,
                                const struct ui_table_column_config *config) {
  if (!table || !config)
    return UI_ERROR_INVALID_ARGUMENT;
  if (col_index >= table->num_cols)
    return UI_ERROR_OUT_OF_BOUNDS;

  table->col_configs[col_index] = *config;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the active sort configuration for the table.
 * @param[in,out] table The table.
 * @param[in] config The sort configuration.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_table_base_set_sort_config(struct ui_table_base *table,
                              const struct ui_table_sort_config *config) {
  if (!table || !config)
    return UI_ERROR_INVALID_ARGUMENT;
  if (config->direction != UI_TABLE_SORT_NONE &&
      config->active_column_index >= table->num_cols) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }
  table->sort_config = *config;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the pagination configuration for the table.
 * @param[in,out] table The table.
 * @param[in] config The pagination configuration.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_table_base_set_pagination_config(
    struct ui_table_base *table,
    const struct ui_table_pagination_config *config) {
  if (!table || !config)
    return UI_ERROR_INVALID_ARGUMENT;
  table->pagination_config = *config;
  return UI_ERROR_NONE;
}

/**
 * @brief Helper function to get ARIA string for sort direction.
 * @param dir The sort direction.
 * @return The corresponding ARIA string.
 */
static const char *get_aria_sort_string(enum ui_table_sort_direction dir) {
  switch (dir) {
  case UI_TABLE_SORT_ASCENDING:
    return "ascending";
  case UI_TABLE_SORT_DESCENDING:
    return "descending";
  default:
    return "none";
  }
}

/**
 * @brief Renders the table into a DOM container.
 * @param[in,out] table The table widget.
 * @param[in,out] container The DOM node to render into.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_table_base_render(struct ui_table_base *table,
                                struct ui_dom_node *container) {
  struct ui_dom_node *table_root = NULL;
  struct ui_dom_node *thead = NULL;
  struct ui_dom_node *header_row = NULL;
  struct ui_dom_node *tbody = NULL;
  ui_error_t rc;
  size_t i, j;
  size_t total_rows, total_cols;
  size_t start_row, end_row;
  char style_buf[256];

  if (!table || !container)
    return UI_ERROR_INVALID_ARGUMENT;

  total_rows = table->model.get_row_count(table->model.user_data);
  total_cols = table->num_cols;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &table_root);
  if (rc != UI_ERROR_NONE)
    return rc;

/** @cond */
#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
  /** @endcond */

  (void)UI_DOM_SET_ATTR_IGNORE(table_root, "role", "grid");

  /* Construct the table inside table_root.
     Eagerly appending ensures a single ui_dom_node_destroy on table_root
     will cascade properly upon any subsequent failure. */

  /* Thead */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &thead);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(table_root, thead);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(thead);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    goto cleanup;
  }
  (void)UI_DOM_SET_ATTR_IGNORE(thead, "role", "rowgroup");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &header_row);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(thead, header_row);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(header_row);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    goto cleanup;
  }
  (void)UI_DOM_SET_ATTR_IGNORE(header_row, "role", "row");

  for (j = 0; j < total_cols; j++) {
    struct ui_dom_node *header_cell;
    struct ui_table_column_config *cfg = &table->col_configs[j];

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &header_cell);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    rc = ui_dom_node_append_child(header_row, header_cell);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(header_cell);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      goto cleanup;
    }

    ui_dom_node_set_attribute(header_cell, "role", "columnheader");

    if (table->sort_config.active_column_index == j) {
      ui_dom_node_set_attribute(
          header_cell, "aria-sort",
          get_aria_sort_string(table->sort_config.direction));
    }

    if (cfg->sizing == UI_TABLE_COLUMN_FLEX) {
#if defined(_MSC_VER)
      sprintf_s(style_buf, sizeof(style_buf), "flex: %f;", cfg->width);
#else
      sprintf(style_buf, "flex: %f;", cfg->width);
#endif
    } else {
#if defined(_MSC_VER)
      sprintf_s(style_buf, sizeof(style_buf), "width: %fpx;", cfg->width);
#else
      sprintf(style_buf, "width: %fpx;", cfg->width);
#endif
    }
    ui_dom_node_set_attribute(header_cell, "style", style_buf);

    rc = table->model.render_header(j, header_cell, table->model.user_data);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }

  /* Tbody */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &tbody);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(table_root, tbody);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(tbody);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    goto cleanup;
  }
  (void)UI_DOM_SET_ATTR_IGNORE(tbody, "role", "rowgroup");

  start_row = 0;
  end_row = total_rows;

  if (table->pagination_config.page_size > 0) {
    start_row = table->pagination_config.current_page *
                table->pagination_config.page_size;
    end_row = start_row + table->pagination_config.page_size;
    if (end_row > total_rows)
      end_row = total_rows;
    if (start_row > total_rows)
      start_row = total_rows; /* Prevent out of bounds if page is too high */
  }

  for (i = start_row; i < end_row; i++) {
    struct ui_dom_node *row;
    int is_selected = 0;

    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &row);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
    rc = ui_dom_node_append_child(tbody, row);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(row);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      goto cleanup;
    }

    ui_dom_node_set_attribute(row, "role", "row");

    ui_selection_model_is_selected(table->selection_model, (void *)(size_t)i,
                                   &is_selected);
    if (is_selected) {
      ui_dom_node_set_attribute(row, "aria-selected", "true");
    }

    for (j = 0; j < total_cols; j++) {
      struct ui_dom_node *cell;
      struct ui_table_column_config *cfg = &table->col_configs[j];

      rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cell);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
      rc = ui_dom_node_append_child(row, cell);
      if (rc != UI_ERROR_NONE) {
        {
          ui_error_t rc_cleanup = ui_dom_node_destroy(cell);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
        goto cleanup;
      }

      ui_dom_node_set_attribute(cell, "role", "gridcell");

      if (cfg->sizing == UI_TABLE_COLUMN_FLEX) {
#if defined(_MSC_VER)
        sprintf_s(style_buf, sizeof(style_buf), "flex: %f;", cfg->width);
#else
        sprintf(style_buf, "flex: %f;", cfg->width);
#endif
      } else {
#if defined(_MSC_VER)
        sprintf_s(style_buf, sizeof(style_buf), "width: %fpx;", cfg->width);
#else
        sprintf(style_buf, "width: %fpx;", cfg->width);
#endif
      }
      ui_dom_node_set_attribute(cell, "style", style_buf);

      rc = table->model.render_cell(i, j, cell, table->model.user_data);
      if (rc != UI_ERROR_NONE)
        goto cleanup;
    }
  }

  rc = ui_dom_node_append_child(container, table_root);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  return UI_ERROR_NONE;

cleanup: {
  ui_error_t rc_cleanup = ui_dom_node_destroy(table_root);
  if (rc_cleanup != UI_ERROR_NONE) {
    (void)rc_cleanup; /* Avoid override */
  }
}
  return rc;
}

/**
 * @brief Binds a reactive data signal to the table.
 * @param[in,out] widget The table widget.
 * @param[in,out] signal The data signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_table_base_bind_data(struct ui_table_base *widget,
                                   struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
