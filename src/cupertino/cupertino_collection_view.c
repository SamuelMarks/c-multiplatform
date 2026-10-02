/**
 * @file cupertino_collection_view.c
 * @brief Implementation of Cupertino Collection View & Compositional Layout.
 */

/* clang-format off */
#include "cupertino/cupertino_collection_view.h"
#include "ui_internal_mem.h"
#include "ui_engine.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_collection_view_mock_grid_set_columns_fail = 0;
int g_cupertino_collection_view_mock_grid_destroy_fail = 0;
int g_cupertino_collection_view_mock_grid_add_item_fail = 0;

static ui_error_t
mock_grid_list_base_set_columns(struct ui_grid_list_base *grid_list,
                                int columns) {
  if (g_cupertino_collection_view_mock_grid_set_columns_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_grid_list_base_set_columns(grid_list, columns);
}
#undef ui_grid_list_base_set_columns
/** @cond */
#define ui_grid_list_base_set_columns mock_grid_list_base_set_columns
/** @endcond */

static ui_error_t
mock_grid_list_base_destroy(struct ui_grid_list_base *grid_list) {
  if (g_cupertino_collection_view_mock_grid_destroy_fail) {
    ui_grid_list_base_destroy(grid_list);
    return UI_ERROR_UNKNOWN;
  }
  return ui_grid_list_base_destroy(grid_list);
}
#undef ui_grid_list_base_destroy
/** @cond */
#define ui_grid_list_base_destroy mock_grid_list_base_destroy
/** @endcond */

static ui_error_t
mock_grid_list_base_add_item(struct ui_grid_list_base *grid_list, int rowspan,
                             int colspan) {
  if (g_cupertino_collection_view_mock_grid_add_item_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  return ui_grid_list_base_add_item(grid_list, rowspan, colspan);
}
#undef ui_grid_list_base_add_item
/** @cond */
#define ui_grid_list_base_add_item mock_grid_list_base_add_item
/** @endcond */
#endif

static int calculate_active_columns(float viewport_width, int compact,
                                    int regular, int wide) {
  if (viewport_width < 600.0f) {
    return compact;
  } else if (viewport_width <= 1024.0f) {
    return regular;
  } else {
    return wide;
  }
}

ui_error_t cupertino_collection_view_create(
    struct ui_engine *engine,
    const struct cupertino_collection_view_descriptor *desc,
    struct cupertino_collection_view **out_view) {
  struct cupertino_collection_view *view;
  int initial_columns;
  ui_error_t rc;

  if (engine == NULL || desc == NULL || out_view == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view = (struct cupertino_collection_view *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_collection_view));
  if (view == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(view, 0, sizeof(*view));
  view->viewport_width =
      desc->initial_width > 0.0f ? desc->initial_width : 375.0f;
  view->viewport_height =
      desc->initial_height > 0.0f ? desc->initial_height : 667.0f;
  view->is_rtl = desc->is_rtl;

  view->layout.compact_column_count = 1;
  view->layout.regular_column_count = 2;
  view->layout.wide_column_count = 4;
  initial_columns = calculate_active_columns(
      view->viewport_width, view->layout.compact_column_count,
      view->layout.regular_column_count, view->layout.wide_column_count);
  view->layout.active_column_count = initial_columns;

  rc = ui_grid_list_base_create(&view->base, initial_columns);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(view);
    return rc;
  }

  *out_view = view;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_collection_view_destroy(struct cupertino_collection_view *view) {
  ui_error_t rc;

  if (view == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->base != NULL) {
    rc = ui_grid_list_base_destroy(view->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(view);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_collection_view_set_breakpoints(
    struct cupertino_collection_view *view, int compact, int regular,
    int wide) {
  int new_cols;
  ui_error_t rc;

  if (view == NULL || compact < 1 || regular < 1 || wide < 1) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view->layout.compact_column_count = compact;
  view->layout.regular_column_count = regular;
  view->layout.wide_column_count = wide;

  new_cols =
      calculate_active_columns(view->viewport_width, compact, regular, wide);
  if (new_cols != view->layout.active_column_count) {
    view->layout.active_column_count = new_cols;
    if (view->base != NULL) {
      rc = ui_grid_list_base_set_columns(view->base, new_cols);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_collection_view_update_viewport(
    struct cupertino_collection_view *view, float width, float height) {
  int new_cols;
  ui_error_t rc;

  if (view == NULL || width <= 0.0f || height <= 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  view->viewport_width = width;
  view->viewport_height = height;

  new_cols = calculate_active_columns(width, view->layout.compact_column_count,
                                      view->layout.regular_column_count,
                                      view->layout.wide_column_count);

  if (new_cols != view->layout.active_column_count) {
    view->layout.active_column_count = new_cols;
    if (view->base != NULL) {
      rc = ui_grid_list_base_set_columns(view->base, new_cols);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_collection_view_add_section(
    struct cupertino_collection_view *view,
    const struct cupertino_collection_section *section,
    size_t *out_section_index) {
  size_t idx;

  if (view == NULL || section == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (view->layout.section_count >= CUPERTINO_COLLECTION_VIEW_MAX_SECTIONS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  idx = view->layout.section_count;
  view->layout.sections[idx] = *section;
  view->layout.section_count++;

  if (out_section_index != NULL) {
    *out_section_index = idx;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_collection_view_add_group(
    struct cupertino_collection_view *view, size_t section_index,
    const struct cupertino_collection_group *group) {
  struct cupertino_collection_section *sec;
  size_t g_idx;
  size_t i;
  ui_error_t rc;

  if (view == NULL || group == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section_index >= view->layout.section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  sec = &view->layout.sections[section_index];
  if (sec->group_count >= CUPERTINO_COLLECTION_VIEW_MAX_GROUPS_PER_SECTION) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  g_idx = sec->group_count;
  sec->groups[g_idx] = *group;
  sec->group_count++;

  /* Mirror items into underlying CDK grid primitive */
  if (view->base != NULL) {
    for (i = 0; i < group->item_count; i++) {
      rc = ui_grid_list_base_add_item(view->base, 1, 1);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_collection_view_scroll_orthogonal(
    struct cupertino_collection_view *view, size_t section_index,
    float delta_x) {
  struct cupertino_collection_section *sec;

  if (view == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section_index >= view->layout.section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  sec = &view->layout.sections[section_index];
  if (sec->orthogonal_behavior == CUPERTINO_COLLECTION_ORTHOGONAL_NONE) {
    return UI_ERROR_UNSUPPORTED;
  }

  sec->orthogonal_scroll_offset += delta_x;
  if (sec->orthogonal_scroll_offset < 0.0f) {
    sec->orthogonal_scroll_offset = 0.0f;
  }

  return UI_ERROR_NONE;
}

static float resolve_dimension(const struct cupertino_collection_dimension *dim,
                               float parent_size) {
  switch (dim->type) {
  case CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_WIDTH:
  case CUPERTINO_COLLECTION_DIMENSION_FRACTIONAL_HEIGHT:
    return dim->value * parent_size;
  case CUPERTINO_COLLECTION_DIMENSION_ABSOLUTE:
  case CUPERTINO_COLLECTION_DIMENSION_ESTIMATED:
  default:
    return dim->value;
  }
}

ui_error_t cupertino_collection_view_get_item_layout(
    const struct cupertino_collection_view *view, size_t section_index,
    size_t group_index, size_t item_index, float *out_x, float *out_y,
    float *out_width, float *out_height) {
  const struct cupertino_collection_section *sec;
  const struct cupertino_collection_group *grp;
  const struct cupertino_collection_item *itm;
  float group_width, group_height;
  float item_w, item_h;
  float x = 0.0f, y = 0.0f;
  size_t s, g, i;

  if (view == NULL || out_x == NULL || out_y == NULL || out_width == NULL ||
      out_height == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (section_index >= view->layout.section_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  sec = &view->layout.sections[section_index];
  if (group_index >= sec->group_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  grp = &sec->groups[group_index];
  if (item_index >= grp->item_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  itm = &grp->items[item_index];

  /* Calculate Y offset by summing previous sections and groups */
  for (s = 0; s < section_index; s++) {
    const struct cupertino_collection_section *ps = &view->layout.sections[s];
    y += ps->content_insets.top + ps->content_insets.bottom;
    for (g = 0; g < ps->group_count; g++) {
      float gh = resolve_dimension(&ps->groups[g].layout_size.height,
                                   view->viewport_height);
      y += gh + ps->inter_group_spacing;
    }
  }

  /* Current section top inset */
  y += sec->content_insets.top;
  for (g = 0; g < group_index; g++) {
    float gh = resolve_dimension(&sec->groups[g].layout_size.height,
                                 view->viewport_height);
    y += gh + sec->inter_group_spacing;
  }

  /* Resolve group sizes */
  group_width =
      resolve_dimension(&grp->layout_size.width, view->viewport_width);
  group_height =
      resolve_dimension(&grp->layout_size.height, view->viewport_height);

  /* Horizontal layout within group */
  x = sec->content_insets.leading;
  if (sec->orthogonal_behavior != CUPERTINO_COLLECTION_ORTHOGONAL_NONE) {
    x -= sec->orthogonal_scroll_offset;
  }

  for (i = 0; i < item_index; i++) {
    if (grp->direction == CUPERTINO_COLLECTION_GROUP_HORIZONTAL) {
      float iw =
          resolve_dimension(&grp->items[i].layout_size.width, group_width);
      x += iw + grp->inter_item_spacing;
    } else {
      float ih =
          resolve_dimension(&grp->items[i].layout_size.height, group_height);
      y += ih + grp->inter_item_spacing;
    }
  }

  item_w = resolve_dimension(&itm->layout_size.width, group_width);
  item_h = resolve_dimension(&itm->layout_size.height, group_height);

  /* Inset adjustments */
  x += itm->content_insets.leading;
  y += itm->content_insets.top;
  item_w -= (itm->content_insets.leading + itm->content_insets.trailing);
  item_h -= (itm->content_insets.top + itm->content_insets.bottom);

  *out_x = x;
  *out_y = y;
  *out_width = item_w;
  *out_height = item_h;

  return UI_ERROR_NONE;
}
