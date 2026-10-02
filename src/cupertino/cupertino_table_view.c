/**
 * @file cupertino_table_view.c
 * @brief Implementation of Cupertino Table View component.
 */

/* clang-format off */
#include "cupertino/cupertino_table_view.h"
#include "ui_engine.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_table_view_mock_list_base_destroy_fail = 0;

static ui_error_t mock_ui_list_base_destroy(struct ui_list_base *list) {
  if (g_cupertino_table_view_mock_list_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_list_base_destroy(list);
}
#undef ui_list_base_destroy
/** @cond */
#define ui_list_base_destroy mock_ui_list_base_destroy
/** @endcond */
#endif

ui_error_t
cupertino_table_view_create(struct ui_engine *engine,
                            const struct cupertino_table_view_descriptor *desc,
                            struct cupertino_table_view **out_table) {
  struct cupertino_table_view *view;
  ui_error_t rc;

  if (engine == NULL || desc == NULL || out_table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_table_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_table_view));
  if (view == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->style = desc->style;
  view->edit_mode = desc->edit_mode;
  view->is_rtl = desc->is_rtl;
  view->swiped_section = -1;
  view->swiped_row = -1;
  view->index_scrub.selected_index = -1;
  view->reorder_from_section = -1;
  view->reorder_from_row = -1;
  view->reorder_to_section = -1;
  view->reorder_to_row = -1;

  rc = ui_list_base_create(&view->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  *out_table = view;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_destroy(struct cupertino_table_view *table) {
  ui_error_t rc;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->base != NULL) {
    rc = ui_list_base_destroy(table->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(table);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_add_section(struct cupertino_table_view *table,
                                 struct cupertino_list_section *section) {
  if (table == NULL || section == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->section_count >= CUPERTINO_TABLE_VIEW_MAX_SECTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->sections[table->section_count] = section;
  table->section_count++;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_get_section_count(const struct cupertino_table_view *table,
                                       size_t *out_count) {
  if (table == NULL || out_count == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = table->section_count;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_get_section(const struct cupertino_table_view *table,
                                 size_t index,
                                 struct cupertino_list_section **out_section) {
  if (table == NULL || out_section == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= table->section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_section = table->sections[index];
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_set_edit_mode(struct cupertino_table_view *table,
                                   int edit_mode) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table->edit_mode = edit_mode;
  if (!edit_mode && table->reorder_active) {
    table->reorder_active = 0;
    table->reorder_from_section = -1;
    table->reorder_from_row = -1;
    table->reorder_to_section = -1;
    table->reorder_to_row = -1;
  }
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_get_edit_mode(const struct cupertino_table_view *table,
                                   int *out_edit_mode) {
  if (table == NULL || out_edit_mode == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_edit_mode = table->edit_mode;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_add_leading_swipe_action(
    struct cupertino_table_view *table,
    const struct cupertino_swipe_action *action) {
  if (table == NULL || action == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->leading_action_count >= CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->leading_actions[table->leading_action_count] = *action;
  table->leading_action_count++;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_add_trailing_swipe_action(
    struct cupertino_table_view *table,
    const struct cupertino_swipe_action *action) {
  if (table == NULL || action == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->trailing_action_count >= CUPERTINO_TABLE_VIEW_MAX_SWIPE_ACTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->trailing_actions[table->trailing_action_count] = *action;
  table->trailing_action_count++;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_clear_swipe_actions(struct cupertino_table_view *table) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table->leading_action_count = 0;
  table->trailing_action_count = 0;
  table->swiped_section = -1;
  table->swiped_row = -1;
  table->swipe_offset = 0.0f;
  table->full_swipe_triggered = 0;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_swipe_pan(struct cupertino_table_view *table,
                                          int section, int row, float offset,
                                          float row_width) {
  if (table == NULL || row_width <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table->swiped_section = section;
  table->swiped_row = row;
  table->swipe_offset = offset;

  if ((offset > 0.0f && offset > (row_width * 0.6f)) ||
      (offset < 0.0f && (-offset) > (row_width * 0.6f))) {
    table->full_swipe_triggered = 1;
  } else {
    table->full_swipe_triggered = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_swipe_release(struct cupertino_table_view *table,
                                   float row_width, int *out_action_triggered) {
  int triggered;

  if (table == NULL || out_action_triggered == NULL || row_width <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  triggered = 0;
  if (table->full_swipe_triggered) {
    if (table->swipe_offset > 0.0f && table->leading_action_count > 0) {
      if (table->leading_actions[0].handler != NULL) {
        table->leading_actions[0].handler(table->leading_actions[0].user_data);
      }
      triggered = 1;
    } else if (table->swipe_offset < 0.0f && table->trailing_action_count > 0) {
      if (table->trailing_actions[0].handler != NULL) {
        table->trailing_actions[0].handler(
            table->trailing_actions[0].user_data);
      }
      triggered = 1;
    }
  }

  /* Reset swipe state with spring recoil */
  table->swiped_section = -1;
  table->swiped_row = -1;
  table->swipe_offset = 0.0f;
  table->full_swipe_triggered = 0;

  *out_action_triggered = triggered;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_set_index_titles(struct cupertino_table_view *table,
                                      const char *const *titles, size_t count) {
  size_t i;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (count > CUPERTINO_TABLE_VIEW_MAX_INDEX_TITLES) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->index_scrub.count = count;
  for (i = 0; i < count; i++) {
    if (titles != NULL && titles[i] != NULL) {
#if defined(_MSC_VER)
      strncpy_s(table->index_scrub.titles[i],
                sizeof(table->index_scrub.titles[i]), titles[i], _TRUNCATE);
#else
      strncpy(table->index_scrub.titles[i], titles[i],
              sizeof(table->index_scrub.titles[i]) - 1);
      table->index_scrub.titles[i][sizeof(table->index_scrub.titles[i]) - 1] =
          '\0';
#endif
    } else {
      table->index_scrub.titles[i][0] = '\0';
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_scrub_index(struct cupertino_table_view *table,
                                            float touch_y, float bar_height,
                                            int *out_selected_section) {
  float item_height;
  int idx;

  if (table == NULL || out_selected_section == NULL || bar_height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (table->index_scrub.count == 0) {
    *out_selected_section = -1;
    return UI_ERROR_NONE;
  }

  item_height = bar_height / (float)table->index_scrub.count;
  if (touch_y < 0.0f) {
    idx = 0;
  } else {
    idx = (int)(touch_y / item_height);
    if ((size_t)idx >= table->index_scrub.count) {
      idx = (int)table->index_scrub.count - 1;
    }
  }

  table->index_scrub.is_active = 1;
  table->index_scrub.touch_y = touch_y;
  table->index_scrub.selected_index = idx;
  table->index_scrub.show_magnifying_bubble = 1;

  *out_selected_section = idx;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_scrub_release(struct cupertino_table_view *table) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table->index_scrub.is_active = 0;
  table->index_scrub.show_magnifying_bubble = 0;
  table->index_scrub.selected_index = -1;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_reorder_begin(struct cupertino_table_view *table,
                                   int section, int row) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!table->edit_mode) {
    return UI_ERROR_UNSUPPORTED;
  }

  if (section < 0 || (size_t)section >= table->section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (row < 0 || (size_t)row >= table->sections[section]->tile_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->reorder_active = 1;
  table->reorder_from_section = section;
  table->reorder_from_row = row;
  table->reorder_to_section = section;
  table->reorder_to_row = row;

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_reorder_update(struct cupertino_table_view *table,
                                    int to_section, int to_row) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!table->reorder_active) {
    return UI_ERROR_UNSUPPORTED;
  }

  if (to_section < 0 || (size_t)to_section >= table->section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (to_row < 0 || (size_t)to_row > table->sections[to_section]->tile_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  table->reorder_to_section = to_section;
  table->reorder_to_row = to_row;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_reorder_commit(struct cupertino_table_view *table) {
  struct cupertino_list_section *src_sec;
  struct cupertino_list_section *dst_sec;
  struct cupertino_list_tile *moved_tile;
  size_t i;

  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!table->reorder_active) {
    return UI_ERROR_UNSUPPORTED;
  }

  src_sec = table->sections[table->reorder_from_section];
  dst_sec = table->sections[table->reorder_to_section];

  if ((size_t)table->reorder_from_row >= src_sec->tile_count) {
    table->reorder_active = 0;
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  moved_tile = src_sec->tiles[table->reorder_from_row];

  /* Remove from source section */
  for (i = (size_t)table->reorder_from_row; i + 1 < src_sec->tile_count; i++) {
    src_sec->tiles[i] = src_sec->tiles[i + 1];
  }
  src_sec->tile_count--;

  /* Insert into destination section */
  if (dst_sec->tile_count < CUPERTINO_LIST_SECTION_MAX_TILES) {
    size_t target = (size_t)table->reorder_to_row;
    if (target > dst_sec->tile_count) {
      target = dst_sec->tile_count;
    }
    for (i = dst_sec->tile_count; i > target; i--) {
      dst_sec->tiles[i] = dst_sec->tiles[i - 1];
    }
    dst_sec->tiles[target] = moved_tile;
    dst_sec->tile_count++;
  }

  table->reorder_active = 0;
  table->reorder_from_section = -1;
  table->reorder_from_row = -1;
  table->reorder_to_section = -1;
  table->reorder_to_row = -1;

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_table_view_reorder_cancel(struct cupertino_table_view *table) {
  if (table == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  table->reorder_active = 0;
  table->reorder_from_section = -1;
  table->reorder_from_row = -1;
  table->reorder_to_section = -1;
  table->reorder_to_row = -1;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_table_view_get_separator_layout(
    const struct cupertino_table_view *table, size_t section, size_t row,
    float *out_inset, int *out_hidden) {
  struct cupertino_list_section *sec;
  struct cupertino_list_tile *tile;

  if (table == NULL || out_inset == NULL || out_hidden == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section >= table->section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  sec = table->sections[section];
  if (row >= sec->tile_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  /* Inset Grouped style: last cell hides separator */
  if (table->style == CUPERTINO_TABLE_VIEW_INSET_GROUPED &&
      row == sec->tile_count - 1) {
    *out_hidden = 1;
    *out_inset = 0.0f;
    return UI_ERROR_NONE;
  }

  *out_hidden = 0;
  tile = sec->tiles[row];
  if (tile != NULL && tile->leading_icon[0] != '\0') {
    *out_inset = CUPERTINO_LIST_SECTION_ICON_INSET;
  } else {
    *out_inset = CUPERTINO_LIST_SECTION_DEFAULT_INSET;
  }

  return UI_ERROR_NONE;
}
