#include "m3/m3_tabs.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_TAB_ROW_TEST_FAIL_NONE 0u
#define M3_TAB_ROW_TEST_FAIL_STYLE_INIT 1u
#define M3_TAB_ROW_TEST_FAIL_ANIM_INIT 2u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_START 3u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 4u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE 5u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE 6u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE 7u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE 8u
#define M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE 9u
#define M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE 10u

#define M3_SEGMENTED_TEST_FAIL_NONE 0u
#define M3_SEGMENTED_TEST_FAIL_STYLE_INIT 1u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 2u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE 3u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE 4u
#define M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE 5u
#define M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE 6u

static m3_u32 g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
static m3_u32 g_m3_tab_row_test_color_fail_after = 0u;
static m3_u32 g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
static m3_u32 g_m3_segmented_test_color_fail_after = 0u;

int M3_CALL m3_tab_row_test_set_fail_point(m3_u32 point) {
  g_m3_tab_row_test_fail_point = point;
  return M3_OK;
}

int M3_CALL m3_tab_row_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_tab_row_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_tab_row_test_clear_fail_points(void) {
  g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
  g_m3_tab_row_test_color_fail_after = 0u;
  return M3_OK;
}

int M3_CALL m3_segmented_test_set_fail_point(m3_u32 point) {
  g_m3_segmented_test_fail_point = point;
  return M3_OK;
}

int M3_CALL m3_segmented_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_segmented_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_segmented_test_clear_fail_points(void) {
  g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
  g_m3_segmented_test_color_fail_after = 0u;
  return M3_OK;
}

static int m3_tab_row_test_color_should_fail(M3Bool *out_fail) {
  if (out_fail == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_tab_row_test_color_fail_after == 0u) {
    *out_fail = M3_FALSE;
    return M3_OK;
  }
  g_m3_tab_row_test_color_fail_after -= 1u;
  *out_fail = (g_m3_tab_row_test_color_fail_after == 0u) ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

static int m3_tab_row_test_fail_point_match(m3_u32 point, M3Bool *out_match) {
  if (out_match == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_tab_row_test_fail_point != point) {
    *out_match = M3_FALSE;
    return M3_OK;
  }
  g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
  *out_match = M3_TRUE;
  return M3_OK;
}

static int m3_segmented_test_color_should_fail(M3Bool *out_fail) {
  if (out_fail == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_segmented_test_color_fail_after == 0u) {
    *out_fail = M3_FALSE;
    return M3_OK;
  }
  g_m3_segmented_test_color_fail_after -= 1u;
  *out_fail = (g_m3_segmented_test_color_fail_after == 0u) ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

static int m3_segmented_test_fail_point_match(m3_u32 point, M3Bool *out_match) {
  if (out_match == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_segmented_test_fail_point != point) {
    *out_match = M3_FALSE;
    return M3_OK;
  }
  g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
  *out_match = M3_TRUE;
  return M3_OK;
}
#endif

static int m3_tabs_validate_bool(M3Bool value) {
  if (value != M3_FALSE && value != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

typedef struct M3TabRowLayout {
  m3_u32 mode;
  M3Scalar start_x;
  M3Scalar start_y;
  M3Scalar tab_width;
  M3Scalar tab_height;
  M3Scalar spacing;
  M3Scalar content_width;
  M3Scalar content_height;
  M3Scalar available_width;
} M3TabRowLayout;

typedef struct M3SegmentedLayout {
  M3Scalar start_x;
  M3Scalar start_y;
  M3Scalar segment_width;
  M3Scalar segment_height;
  M3Scalar spacing;
  M3Scalar content_width;
  M3Scalar content_height;
} M3SegmentedLayout;

static int m3_tab_row_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_tab_row_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                M3Scalar b, M3Scalar a) {
  M3Bool should_fail;
  int rc;

  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
#ifdef M3_TESTING
  rc = m3_tab_row_test_color_should_fail(&should_fail);
  if (rc != M3_OK) {
    return rc;
  }
  if (should_fail == M3_TRUE) {
    return M3_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_tab_row_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                       M3Color *out_color) {
  M3Bool should_fail;
  int rc;

  if (base == NULL || out_color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return M3_ERR_RANGE;
  }

  rc = m3_tab_row_validate_color(base);
  if (rc != M3_OK) {
    return rc;
  }
#ifdef M3_TESTING
  rc = m3_tab_row_test_color_should_fail(&should_fail);
  if (rc != M3_OK) {
    return rc;
  }
  if (should_fail == M3_TRUE) {
    return M3_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return M3_OK;
}

static int m3_tab_row_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_tab_row_validate_text_style(const M3TextStyle *style,
                                          M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (require_family == M3_TRUE && style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  rc = m3_tabs_validate_bool(style->italic);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_tab_row_validate_style(const M3TabRowStyle *style,
                                     M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->mode) {
  case M3_TAB_MODE_FIXED:
  case M3_TAB_MODE_SCROLLABLE:
    break;
  default:
    return M3_ERR_RANGE;
  }

  if (style->spacing < 0.0f || style->min_width < 0.0f ||
      style->min_height < 0.0f || style->padding_x < 0.0f ||
      style->padding_y < 0.0f || style->indicator_thickness < 0.0f ||
      style->indicator_corner < 0.0f || style->indicator_anim_duration < 0.0f) {
    return M3_ERR_RANGE;
  }

  switch (style->indicator_anim_easing) {
  case M3_ANIM_EASE_LINEAR:
  case M3_ANIM_EASE_IN:
  case M3_ANIM_EASE_OUT:
  case M3_ANIM_EASE_IN_OUT:
    break;
  default:
    return M3_ERR_RANGE;
  }

  rc = m3_tab_row_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_text_style(&style->text_style, require_family);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_color(&style->selected_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->indicator_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_tab_row_validate_items(const M3TabItem *items, m3_usize count) {
  m3_usize i;

  if (count == 0) {
    return M3_OK;
  }
  if (items == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    if (items[i].utf8_label == NULL && items[i].utf8_len != 0) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  return M3_OK;
}

static int m3_tab_row_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_tab_row_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_tab_row_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_tab_row_measure_max_text(const M3TabRow *row, M3Scalar *out_width,
                                       M3Scalar *out_height,
                                       M3Scalar *out_baseline) {
  M3TextMetrics metrics;
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  m3_usize i;
  int rc;

  if (row == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_width = 0.0f;
  max_height = 0.0f;
  max_baseline = 0.0f;

  if (row->item_count == 0) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    *out_baseline = 0.0f;
    return M3_OK;
  }
  if (row->items == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < row->item_count; ++i) {
    if (row->items[i].utf8_len == 0) {
      continue;
    }
    rc = m3_text_measure_utf8(&row->text_backend, row->font,
                              row->items[i].utf8_label, row->items[i].utf8_len,
                              &metrics);
    if (rc != M3_OK) {
      return rc;
    }
    if (metrics.width > max_width) {
      max_width = metrics.width;
    }
    if (metrics.height > max_height) {
      max_height = metrics.height;
    }
    if (metrics.baseline > max_baseline) {
      max_baseline = metrics.baseline;
    }
  }

  *out_width = max_width;
  *out_height = max_height;
  *out_baseline = max_baseline;
  return M3_OK;
}

static int m3_tab_row_item_width(const M3TabRow *row, const M3TabItem *item,
                                 M3Scalar *out_width) {
  M3TextMetrics metrics;
  M3Scalar width;
  int rc;

  if (row == NULL || item == NULL || out_width == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  metrics.width = 0.0f;
  metrics.height = 0.0f;
  metrics.baseline = 0.0f;

  if (item->utf8_len > 0) {
    rc = m3_text_measure_utf8(&row->text_backend, row->font, item->utf8_label,
                              item->utf8_len, &metrics);
    if (rc != M3_OK) {
      return rc;
    }
  }

  width = metrics.width + row->style.padding_x * 2.0f;
  if (width < row->style.min_width) {
    width = row->style.min_width;
  }
  if (width < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_width = width;
  return M3_OK;
}

static int m3_tab_row_measure_content(const M3TabRow *row, m3_u32 mode,
                                      M3Scalar *out_width,
                                      M3Scalar *out_height) {
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  M3Scalar tab_width;
  M3Scalar tab_height;
  M3Scalar content_width;
  M3Scalar content_height;
  m3_usize i;
  int rc;

  if (row == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (mode != M3_TAB_MODE_FIXED && mode != M3_TAB_MODE_SCROLLABLE) {
    return M3_ERR_RANGE;
  }

  rc = m3_tab_row_measure_max_text(row, &max_width, &max_height, &max_baseline);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(max_baseline);

  tab_height = max_height + row->style.padding_y * 2.0f;
  if (tab_height < row->style.min_height) {
    tab_height = row->style.min_height;
  }

  if (mode == M3_TAB_MODE_FIXED) {
    tab_width = max_width + row->style.padding_x * 2.0f;
    if (tab_width < row->style.min_width) {
      tab_width = row->style.min_width;
    }
    if (row->item_count > 0) {
      content_width = tab_width * (M3Scalar)row->item_count;
      content_width += row->style.spacing * (M3Scalar)(row->item_count - 1);
    } else {
      content_width = 0.0f;
    }
  } else {
    content_width = 0.0f;
    if (row->item_count > 0) {
      for (i = 0; i < row->item_count; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i], &tab_width);
        if (rc != M3_OK) {
          return rc;
        }
        content_width += tab_width;
        if (i + 1 < row->item_count) {
          content_width += row->style.spacing;
        }
      }
    }
  }

  content_height = tab_height;
  if (content_width < 0.0f || content_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_width =
      row->style.padding.left + row->style.padding.right + content_width;
  *out_height =
      row->style.padding.top + row->style.padding.bottom + content_height;
  if (*out_width < 0.0f || *out_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_tab_row_compute_layout(const M3TabRow *row,
                                     M3TabRowLayout *out_layout) {
  M3TabRowStyle style;
  M3Rect bounds;
  M3Scalar available_width;
  M3Scalar available_height;
  M3Scalar spacing;
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  M3Scalar tab_height;
  M3Scalar tab_width;
  M3Scalar content_width;
  M3Scalar total_spacing;
  m3_u32 mode;
  m3_usize i;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (row == NULL || out_layout == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style = row->style;
  bounds = row->bounds;

  rc = m3_tab_row_validate_style(&style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  available_width = bounds.width - style.padding.left - style.padding.right;
  available_height = bounds.height - style.padding.top - style.padding.bottom;
  if (available_width < 0.0f || available_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_tab_row_measure_max_text(row, &max_width, &max_height, &max_baseline);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(max_baseline);

  spacing = style.spacing;
#ifdef M3_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    spacing = -1.0f;
  }
#endif
  if (spacing < 0.0f) {
    return M3_ERR_RANGE;
  }

  tab_height = max_height + style.padding_y * 2.0f;
  if (tab_height < style.min_height) {
    tab_height = style.min_height;
  }
#ifdef M3_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    tab_height = -1.0f;
  }
#endif
  if (tab_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (row->item_count > 0 && available_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (tab_height > available_height && available_height > 0.0f) {
    tab_height = available_height;
  }

  mode = style.mode;
  memset(out_layout, 0, sizeof(*out_layout));
  out_layout->mode = mode;
  out_layout->spacing = spacing;
  out_layout->start_x = bounds.x + style.padding.left;
  out_layout->start_y = bounds.y + style.padding.top;
  out_layout->tab_height = tab_height;
  out_layout->available_width = available_width;

  if (available_height > tab_height) {
    out_layout->start_y += (available_height - tab_height) * 0.5f;
  }

  if (mode == M3_TAB_MODE_FIXED) {
    if (row->item_count > 0) {
      total_spacing = spacing * (M3Scalar)(row->item_count - 1);
      if (available_width < total_spacing) {
        return M3_ERR_RANGE;
      }
      tab_width = (available_width - total_spacing) / (M3Scalar)row->item_count;
#ifdef M3_TESTING
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE, &match);
      if (rc != M3_OK) {
        return rc;
      }
      if (match == M3_TRUE) {
        tab_width = -1.0f;
      }
#endif
      if (tab_width < 0.0f) {
        return M3_ERR_RANGE;
      }
    } else {
      tab_width = 0.0f;
    }
    out_layout->tab_width = tab_width;
    if (row->item_count > 0) {
      content_width = tab_width * (M3Scalar)row->item_count;
      content_width += spacing * (M3Scalar)(row->item_count - 1);
    } else {
      content_width = 0.0f;
    }
  } else if (mode == M3_TAB_MODE_SCROLLABLE) {
    content_width = 0.0f;
    if (row->item_count > 0) {
      for (i = 0; i < row->item_count; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i], &tab_width);
        if (rc != M3_OK) {
          return rc;
        }
        content_width += tab_width;
        if (i + 1 < row->item_count) {
          content_width += spacing;
        }
      }
    }
  } else {
    return M3_ERR_RANGE;
  }

  if (content_width < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_layout->content_width = content_width;
  out_layout->content_height = tab_height;
  return M3_OK;
}

static int m3_tab_row_clamp_scroll(M3TabRow *row,
                                   const M3TabRowLayout *layout) {
  M3Scalar max_scroll;

  if (row == NULL || layout == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (row->scroll_offset < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (layout->mode != M3_TAB_MODE_SCROLLABLE) {
    row->scroll_offset = 0.0f;
    return M3_OK;
  }

  max_scroll = layout->content_width - layout->available_width;
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }
  if (row->scroll_offset > max_scroll) {
    row->scroll_offset = max_scroll;
  }
  return M3_OK;
}

static int m3_tab_row_indicator_target(const M3TabRow *row,
                                       const M3TabRowLayout *layout,
                                       M3Scalar *out_pos, M3Scalar *out_width) {
  M3Scalar pos;
  M3Scalar width;
  m3_usize i;
  int rc;

  if (row == NULL || layout == NULL || out_pos == NULL || out_width == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (row->selected_index == M3_TAB_INVALID_INDEX ||
      row->selected_index >= row->item_count) {
    *out_pos = 0.0f;
    *out_width = 0.0f;
    return M3_OK;
  }

  if (layout->mode == M3_TAB_MODE_FIXED) {
    pos = (layout->tab_width + layout->spacing) * (M3Scalar)row->selected_index;
    width = layout->tab_width;
  } else if (layout->mode == M3_TAB_MODE_SCROLLABLE) {
    pos = 0.0f;
    width = 0.0f;
    for (i = 0; i < row->selected_index; ++i) {
      rc = m3_tab_row_item_width(row, &row->items[i], &width);
      if (rc != M3_OK) {
        return rc;
      }
      pos += width + layout->spacing;
    }
    rc = m3_tab_row_item_width(row, &row->items[row->selected_index], &width);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    return M3_ERR_RANGE;
  }

  if (pos < 0.0f || width < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_pos = pos;
  *out_width = width;
  return M3_OK;
}

static int m3_tab_row_sync_indicator(M3TabRow *row,
                                     const M3TabRowLayout *layout,
                                     M3Bool animate) {
  M3Scalar target_pos;
  M3Scalar target_width;
  M3Scalar value;
  int rc;
#ifdef M3_TESTING
  M3Bool fail_match;
#endif

  if (row == NULL || layout == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tabs_validate_bool(animate);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_indicator_target(row, layout, &target_pos, &target_width);
  if (rc != M3_OK) {
    return rc;
  }

  if (animate == M3_TRUE) {
    rc = m3_anim_controller_start_timing(
        &row->indicator_pos_anim, row->indicator_pos, target_pos,
        row->style.indicator_anim_duration, row->style.indicator_anim_easing);
#ifdef M3_TESTING
    if (rc == M3_OK) {
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_INDICATOR_START, &fail_match);
      if (rc == M3_OK && fail_match == M3_TRUE) {
        rc = M3_ERR_IO;
      }
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }

    rc = m3_anim_controller_start_timing(
        &row->indicator_width_anim, row->indicator_width, target_width,
        row->style.indicator_anim_duration, row->style.indicator_anim_easing);
#ifdef M3_TESTING
    if (rc == M3_OK) {
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_INDICATOR_START, &fail_match);
      if (rc == M3_OK && fail_match == M3_TRUE) {
        rc = M3_ERR_IO;
      }
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    rc = m3_anim_controller_start_timing(&row->indicator_pos_anim, target_pos,
                                         target_pos, 0.0f, M3_ANIM_EASE_LINEAR);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_anim_controller_start_timing(&row->indicator_width_anim,
                                         target_width, target_width, 0.0f,
                                         M3_ANIM_EASE_LINEAR);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_anim_controller_get_value(&row->indicator_pos_anim, &value);
  if (rc != M3_OK) {
    return rc;
  }
  row->indicator_pos = value;
  rc = m3_anim_controller_get_value(&row->indicator_width_anim, &value);
  if (rc != M3_OK) {
    return rc;
  }
  row->indicator_width = value;

  return M3_OK;
}

static int m3_tab_row_item_rect(const M3TabRow *row,
                                const M3TabRowLayout *layout, m3_usize index,
                                M3Rect *out_rect) {
  M3Scalar offset;
  M3Scalar width;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (row == NULL || layout == NULL || out_rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= row->item_count) {
    return M3_ERR_RANGE;
  }

  out_rect->x = layout->start_x;
  out_rect->y = layout->start_y;
  out_rect->width = layout->tab_width;
  out_rect->height = layout->tab_height;

  if (layout->mode == M3_TAB_MODE_FIXED) {
    out_rect->x += (layout->tab_width + layout->spacing) * (M3Scalar)index;
  } else if (layout->mode == M3_TAB_MODE_SCROLLABLE) {
    offset = 0.0f;
    if (index > 0) {
      m3_usize i;
      for (i = 0; i < index; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i], &width);
        if (rc != M3_OK) {
          return rc;
        }
        offset += width + layout->spacing;
      }
    }
    rc = m3_tab_row_item_width(row, &row->items[index], &width);
    if (rc != M3_OK) {
      return rc;
    }
    out_rect->x += offset - row->scroll_offset;
    out_rect->width = width;
  } else {
    return M3_ERR_RANGE;
  }

#ifdef M3_TESTING
  rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE,
                                        &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    out_rect->width = -1.0f;
  }
#endif

  if (out_rect->width < 0.0f || out_rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_tab_row_hit_test(const M3TabRow *row,
                               const M3TabRowLayout *layout, m3_i32 x, m3_i32 y,
                               m3_usize *out_index) {
  M3Scalar fx;
  M3Scalar fy;
  M3Scalar stride;
  M3Scalar pos;
  M3Scalar width;
  m3_usize index;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (row == NULL || layout == NULL || out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_index = M3_TAB_INVALID_INDEX;
  if (row->item_count == 0) {
    return M3_OK;
  }

  fx = (M3Scalar)x;
  fy = (M3Scalar)y;

  if (fy < layout->start_y || fy > layout->start_y + layout->tab_height) {
    return M3_OK;
  }

  if (layout->mode == M3_TAB_MODE_FIXED) {
    if (fx < layout->start_x || fx > layout->start_x + layout->content_width) {
      return M3_OK;
    }
    stride = layout->tab_width + layout->spacing;
    if (stride <= 0.0f) {
      return M3_ERR_RANGE;
    }
    pos = fx - layout->start_x;
#ifdef M3_TESTING
    rc = m3_tab_row_test_fail_point_match(
        M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
    if (rc != M3_OK) {
      return rc;
    }
    if (match == M3_TRUE) {
      pos = -1.0f;
    }
#endif
    if (pos < 0.0f) {
      return M3_OK;
    }
    index = (m3_usize)(pos / stride);
    if (index >= row->item_count) {
      return M3_OK;
    }
    if (pos > (M3Scalar)index * stride + layout->tab_width) {
      return M3_OK;
    }
    *out_index = index;
    return M3_OK;
  }

  if (layout->mode != M3_TAB_MODE_SCROLLABLE) {
    return M3_ERR_RANGE;
  }

  pos = fx - layout->start_x + row->scroll_offset;
#ifdef M3_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    pos = -1.0f;
  }
#endif
  if (pos < 0.0f || pos > layout->content_width) {
    return M3_OK;
  }

  for (index = 0; index < row->item_count; ++index) {
    rc = m3_tab_row_item_width(row, &row->items[index], &width);
    if (rc != M3_OK) {
      return rc;
    }
    if (pos <= width) {
      *out_index = index;
      return M3_OK;
    }
    pos -= width;
    if (index + 1 < row->item_count) {
      if (pos <= layout->spacing) {
        return M3_OK;
      }
      pos -= layout->spacing;
    }
  }

  return M3_OK;
}

static int m3_tab_row_widget_measure(void *widget, M3MeasureSpec width,
                                     M3MeasureSpec height, M3Size *out_size) {
  M3TabRow *row;
  M3Scalar content_width;
  M3Scalar content_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  row = (M3TabRow *)widget;
  rc = m3_tab_row_validate_style(&row->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_items(row->items, row->item_count);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_measure_content(row, row->style.mode, &content_width,
                                  &content_height);
  if (rc != M3_OK) {
    return rc;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return M3_OK;
}

static int m3_tab_row_widget_layout(void *widget, M3Rect bounds) {
  M3TabRow *row;
  M3TabRowLayout layout;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  row = (M3TabRow *)widget;
  row->bounds = bounds;

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  row->content_width = layout.content_width;

  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_sync_indicator(row, &layout, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_tab_row_widget_paint(void *widget, M3PaintContext *ctx) {
  M3TabRow *row;
  M3TabRowLayout layout;
  M3Rect item_rect;
  M3Rect indicator_rect;
  M3TextMetrics metrics;
  M3Scalar text_x;
  M3Scalar text_y;
  M3Scalar indicator_thickness;
  M3Scalar indicator_width;
  M3Color text_color;
  M3Color indicator_color;
  m3_usize i;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  row = (M3TabRow *)widget;
  rc = m3_tab_row_validate_style(&row->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_rect(&row->bounds);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_items(row->items, row->item_count);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  row->content_width = layout.content_width;

  if (row->style.background_color.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &row->bounds,
                                     row->style.background_color, 0.0f);
    if (rc != M3_OK) {
      return rc;
    }
  }

  indicator_thickness = row->style.indicator_thickness;
#ifdef M3_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    indicator_thickness = -1.0f;
  }
#endif
  if (indicator_thickness < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (row->selected_index != M3_TAB_INVALID_INDEX &&
      row->selected_index < row->item_count && indicator_thickness > 0.0f) {
    indicator_width = row->indicator_width;
    indicator_rect.x = layout.start_x + row->indicator_pos - row->scroll_offset;
    indicator_rect.y = layout.start_y + layout.tab_height - indicator_thickness;
    indicator_rect.width = indicator_width;
    indicator_rect.height = indicator_thickness;
#ifdef M3_TESTING
    rc = m3_tab_row_test_fail_point_match(
        M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE, &match);
    if (rc != M3_OK) {
      return rc;
    }
    if (match == M3_TRUE) {
      indicator_rect.width = -1.0f;
    }
#endif

    if (indicator_rect.width < 0.0f || indicator_rect.height < 0.0f) {
      return M3_ERR_RANGE;
    }

    indicator_color = row->style.indicator_color;
    if (row->widget.flags & M3_WIDGET_FLAG_DISABLED) {
      indicator_color = row->style.disabled_indicator_color;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &indicator_rect,
                                     indicator_color,
                                     row->style.indicator_corner);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (row->item_count == 0) {
    return M3_OK;
  }

  if (row->text_backend.vtable == NULL ||
      row->text_backend.vtable->measure_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  for (i = 0; i < row->item_count; ++i) {
    rc = m3_tab_row_item_rect(row, &layout, i, &item_rect);
    if (rc != M3_OK) {
      return rc;
    }

    if (row->items[i].utf8_len == 0) {
      continue;
    }

    rc = m3_text_measure_utf8(&row->text_backend, row->font,
                              row->items[i].utf8_label, row->items[i].utf8_len,
                              &metrics);
    if (rc != M3_OK) {
      return rc;
    }

    text_x = item_rect.x + (item_rect.width - metrics.width) * 0.5f;
    text_y = item_rect.y + (item_rect.height - metrics.height) * 0.5f +
             metrics.baseline;

    text_color = row->style.text_style.color;
    if (row->widget.flags & M3_WIDGET_FLAG_DISABLED) {
      text_color = row->style.disabled_text_color;
    } else if (i == row->selected_index) {
      text_color = row->style.selected_text_color;
    }

    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, row->font, row->items[i].utf8_label,
        row->items[i].utf8_len, text_x, text_y, text_color);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_tab_row_widget_event(void *widget, const M3InputEvent *event,
                                   M3Bool *out_handled) {
  M3TabRow *row;
  M3TabRowLayout layout;
  m3_usize index;
  m3_usize previous;
  M3Scalar delta;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  row = (M3TabRow *)widget;

  if (row->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  if (event->type == M3_INPUT_POINTER_SCROLL) {
    if (row->style.mode != M3_TAB_MODE_SCROLLABLE) {
      return M3_OK;
    }
    delta = (M3Scalar)event->data.pointer.scroll_x;
    if (delta == 0.0f) {
      return M3_OK;
    }
    rc = m3_tab_row_set_scroll(row, row->scroll_offset + delta);
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  }

  if (event->type != M3_INPUT_POINTER_DOWN &&
      event->type != M3_INPUT_POINTER_UP) {
    return M3_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_hit_test(row, &layout, event->data.pointer.x,
                           event->data.pointer.y, &index);
  if (rc != M3_OK) {
    return rc;
  }

  if (event->type == M3_INPUT_POINTER_DOWN) {
    if (row->pressed_index != M3_TAB_INVALID_INDEX) {
      return M3_ERR_STATE;
    }
    if (index == M3_TAB_INVALID_INDEX) {
      return M3_OK;
    }
    row->pressed_index = index;
    *out_handled = M3_TRUE;
    return M3_OK;
  }

  if (row->pressed_index == M3_TAB_INVALID_INDEX) {
    return M3_OK;
  }

  if (index == row->pressed_index) {
    previous = row->selected_index;
    row->selected_index = index;
    rc = m3_tab_row_sync_indicator(row, &layout, M3_TRUE);
    if (rc != M3_OK) {
      row->selected_index = previous;
      row->pressed_index = M3_TAB_INVALID_INDEX;
      return rc;
    }
    if (row->on_select != NULL) {
      rc = row->on_select(row->on_select_ctx, row, index);
      if (rc != M3_OK) {
        row->selected_index = previous;
        rc = m3_tab_row_sync_indicator(row, &layout, M3_FALSE);
        if (rc != M3_OK) {
          row->pressed_index = M3_TAB_INVALID_INDEX;
          return rc;
        }
        row->pressed_index = M3_TAB_INVALID_INDEX;
        return rc;
      }
    }
  }

  row->pressed_index = M3_TAB_INVALID_INDEX;
  *out_handled = M3_TRUE;
  return M3_OK;
}

static int m3_tab_row_widget_get_semantics(void *widget,
                                           M3Semantics *out_semantics) {
  M3TabRow *row;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  row = (M3TabRow *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (row->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (row->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_tab_row_widget_destroy(void *widget) {
  M3TabRow *row;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  row = (M3TabRow *)widget;
  rc = M3_OK;
  if (row->owns_font == M3_TRUE &&
      (row->font.id != 0u || row->font.generation != 0u)) {
    if (row->text_backend.vtable != NULL &&
        row->text_backend.vtable->destroy_font != NULL) {
      rc = row->text_backend.vtable->destroy_font(row->text_backend.ctx,
                                                  row->font);
    } else {
      rc = M3_ERR_UNSUPPORTED;
    }
  }

  row->font.id = 0u;
  row->font.generation = 0u;
  row->items = NULL;
  row->item_count = 0;
  row->selected_index = M3_TAB_INVALID_INDEX;
  row->pressed_index = M3_TAB_INVALID_INDEX;
  row->bounds.x = 0.0f;
  row->bounds.y = 0.0f;
  row->bounds.width = 0.0f;
  row->bounds.height = 0.0f;
  row->scroll_offset = 0.0f;
  row->content_width = 0.0f;
  row->indicator_pos = 0.0f;
  row->indicator_width = 0.0f;
  row->text_backend.ctx = NULL;
  row->text_backend.vtable = NULL;
  row->widget.ctx = NULL;
  row->widget.vtable = NULL;
  row->on_select = NULL;
  row->on_select_ctx = NULL;
  row->owns_font = M3_FALSE;
  return rc;
}

static const M3WidgetVTable g_m3_tab_row_widget_vtable = {
    m3_tab_row_widget_measure,       m3_tab_row_widget_layout,
    m3_tab_row_widget_paint,         m3_tab_row_widget_event,
    m3_tab_row_widget_get_semantics, m3_tab_row_widget_destroy};

int M3_CALL m3_tab_row_style_init(M3TabRowStyle *style) {
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_text_style_init(&style->text_style);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_STYLE_INIT,
                                          &match);
    if (rc == M3_OK && match == M3_TRUE) {
      rc = M3_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  style->mode = M3_TAB_MODE_FIXED;
  style->spacing = M3_TAB_DEFAULT_SPACING;
  style->min_width = M3_TAB_DEFAULT_MIN_WIDTH;
  style->min_height = M3_TAB_DEFAULT_MIN_HEIGHT;
  style->padding_x = M3_TAB_DEFAULT_PADDING_X;
  style->padding_y = M3_TAB_DEFAULT_PADDING_Y;
  style->indicator_thickness = M3_TAB_DEFAULT_INDICATOR_THICKNESS;
  style->indicator_corner = M3_TAB_DEFAULT_INDICATOR_CORNER;
  style->indicator_anim_duration = M3_TAB_DEFAULT_INDICATOR_DURATION;
  style->indicator_anim_easing = M3_TAB_DEFAULT_INDICATOR_EASING;

  rc =
      m3_tab_row_color_set(&style->selected_text_color, 0.0f, 0.0f, 0.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_color_set(&style->indicator_color, 0.0f, 0.0f, 0.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_color_with_alpha(&style->text_style.color, 0.38f,
                                   &style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_color_with_alpha(&style->indicator_color, 0.38f,
                                   &style->disabled_indicator_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_tab_row_init(M3TabRow *row, const M3TextBackend *backend,
                            const M3TabRowStyle *style, const M3TabItem *items,
                            m3_usize item_count, m3_usize selected_index) {
  M3TabRowLayout layout;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (row == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_tab_row_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  if (selected_index != M3_TAB_INVALID_INDEX && selected_index >= item_count) {
    return M3_ERR_RANGE;
  }

  memset(row, 0, sizeof(*row));
  row->text_backend = *backend;
  row->style = *style;
  row->items = items;
  row->item_count = item_count;
  row->selected_index = selected_index;
  row->pressed_index = M3_TAB_INVALID_INDEX;
  row->bounds.x = 0.0f;
  row->bounds.y = 0.0f;
  row->bounds.width = 0.0f;
  row->bounds.height = 0.0f;
  row->scroll_offset = 0.0f;
  row->content_width = 0.0f;

  rc = m3_text_font_create(backend, &style->text_style, &row->font);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_init(&row->indicator_pos_anim);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ANIM_INIT,
                                          &match);
    if (rc == M3_OK && match == M3_TRUE) {
      rc = M3_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != M3_OK) {
    m3_text_font_destroy(backend, row->font);
    return rc;
  }

  rc = m3_anim_controller_init(&row->indicator_width_anim);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ANIM_INIT,
                                          &match);
    if (rc == M3_OK && match == M3_TRUE) {
      rc = M3_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != M3_OK) {
    m3_text_font_destroy(backend, row->font);
    return rc;
  }

  row->owns_font = M3_TRUE;
  row->widget.ctx = row;
  row->widget.vtable = &g_m3_tab_row_widget_vtable;
  row->widget.handle.id = 0u;
  row->widget.handle.generation = 0u;
  row->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

  layout.mode = row->style.mode;
  layout.start_x = 0.0f;
  layout.start_y = 0.0f;
  layout.tab_width = 0.0f;
  layout.tab_height = row->style.min_height;
  layout.spacing = row->style.spacing;
  layout.content_width = 0.0f;
  layout.content_height = row->style.min_height;
  layout.available_width = 0.0f;
  rc = m3_tab_row_sync_indicator(row, &layout, M3_FALSE);
  if (rc != M3_OK) {
    m3_text_font_destroy(backend, row->font);
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_tab_row_set_items(M3TabRow *row, const M3TabItem *items,
                                 m3_usize item_count) {
  int rc;

  if (row == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  row->items = items;
  row->item_count = item_count;
  if (row->selected_index != M3_TAB_INVALID_INDEX &&
      row->selected_index >= item_count) {
    row->selected_index = M3_TAB_INVALID_INDEX;
  }
  if (row->pressed_index != M3_TAB_INVALID_INDEX &&
      row->pressed_index >= item_count) {
    row->pressed_index = M3_TAB_INVALID_INDEX;
  }

  return M3_OK;
}

int M3_CALL m3_tab_row_set_style(M3TabRow *row, const M3TabRowStyle *style) {
  M3Handle new_font;
  int rc;

  if (row == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_font_create(&row->text_backend, &style->text_style, &new_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (row->owns_font == M3_TRUE) {
    rc = m3_text_font_destroy(&row->text_backend, row->font);
    if (rc != M3_OK) {
      m3_text_font_destroy(&row->text_backend, new_font);
      return rc;
    }
  }

  row->style = *style;
  row->font = new_font;
  row->owns_font = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_tab_row_set_selected(M3TabRow *row, m3_usize selected_index) {
  M3TabRowLayout layout;
  int rc;

  if (row == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (selected_index != M3_TAB_INVALID_INDEX &&
      selected_index >= row->item_count) {
    return M3_ERR_RANGE;
  }

  row->selected_index = selected_index;
  if (row->bounds.width <= 0.0f || row->bounds.height <= 0.0f) {
    return M3_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_sync_indicator(row, &layout, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_tab_row_get_selected(const M3TabRow *row,
                                    m3_usize *out_selected) {
  if (row == NULL || out_selected == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_selected = row->selected_index;
  return M3_OK;
}

int M3_CALL m3_tab_row_set_scroll(M3TabRow *row, M3Scalar offset) {
  M3TabRowLayout layout;
  int rc;

  if (row == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (offset < 0.0f) {
    return M3_ERR_RANGE;
  }

  row->scroll_offset = offset;
  if (row->style.mode != M3_TAB_MODE_SCROLLABLE) {
    row->scroll_offset = 0.0f;
    return M3_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

int M3_CALL m3_tab_row_get_scroll(const M3TabRow *row, M3Scalar *out_offset) {
  if (row == NULL || out_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_offset = row->scroll_offset;
  return M3_OK;
}

int M3_CALL m3_tab_row_set_on_select(M3TabRow *row, M3TabRowOnSelect on_select,
                                     void *ctx) {
  if (row == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  row->on_select = on_select;
  row->on_select_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_tab_row_step(M3TabRow *row, M3Scalar dt, M3Bool *out_changed) {
  M3Scalar value;
  M3Bool finished;
  M3Bool changed;
  int rc;

  if (row == NULL || out_changed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return M3_ERR_RANGE;
  }

  changed = M3_FALSE;

  if (row->indicator_pos_anim.mode != M3_ANIM_MODE_NONE) {
    rc = m3_anim_controller_step(&row->indicator_pos_anim, dt, &value,
                                 &finished);
    if (rc != M3_OK) {
      return rc;
    }
    if (value != row->indicator_pos) {
      row->indicator_pos = value;
      changed = M3_TRUE;
    }
  }

  if (row->indicator_width_anim.mode != M3_ANIM_MODE_NONE) {
    rc = m3_anim_controller_step(&row->indicator_width_anim, dt, &value,
                                 &finished);
    if (rc != M3_OK) {
      return rc;
    }
    if (value != row->indicator_width) {
      row->indicator_width = value;
      changed = M3_TRUE;
    }
  }

  M3_UNUSED(finished);
  *out_changed = changed;
  return M3_OK;
}

static int m3_segmented_validate_color(const M3Color *color) {
  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_segmented_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                  M3Scalar b, M3Scalar a) {
  M3Bool should_fail;
  int rc;

  if (color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return M3_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return M3_ERR_RANGE;
  }
#ifdef M3_TESTING
  rc = m3_segmented_test_color_should_fail(&should_fail);
  if (rc != M3_OK) {
    return rc;
  }
  if (should_fail == M3_TRUE) {
    return M3_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_segmented_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                         M3Color *out_color) {
  M3Bool should_fail;
  int rc;

  if (base == NULL || out_color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return M3_ERR_RANGE;
  }

  rc = m3_segmented_validate_color(base);
  if (rc != M3_OK) {
    return rc;
  }
#ifdef M3_TESTING
  rc = m3_segmented_test_color_should_fail(&should_fail);
  if (rc != M3_OK) {
    return rc;
  }
  if (should_fail == M3_TRUE) {
    return M3_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return M3_OK;
}

static int m3_segmented_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_segmented_validate_text_style(const M3TextStyle *style,
                                            M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (require_family == M3_TRUE && style->utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return M3_ERR_RANGE;
  }
  rc = m3_tabs_validate_bool(style->italic);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_validate_color(&style->color);
  if (rc != M3_OK) {
    return rc;
  }
  return M3_OK;
}

static int m3_segmented_validate_style(const M3SegmentedStyle *style,
                                       M3Bool require_family) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (style->spacing < 0.0f || style->min_width < 0.0f ||
      style->min_height < 0.0f || style->padding_x < 0.0f ||
      style->padding_y < 0.0f || style->outline_width < 0.0f ||
      style->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_segmented_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_validate_text_style(&style->text_style, require_family);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->selected_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->selected_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_selected_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_selected_text_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_segmented_validate_items(const M3SegmentedItem *items,
                                       m3_usize count) {
  m3_usize i;

  if (count == 0) {
    return M3_OK;
  }
  if (items == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    if (items[i].utf8_label == NULL && items[i].utf8_len != 0) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  return M3_OK;
}

static int m3_segmented_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_segmented_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_segmented_validate_backend(const M3TextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_segmented_validate_mode(m3_u32 mode) {
  switch (mode) {
  case M3_SEGMENTED_MODE_SINGLE:
  case M3_SEGMENTED_MODE_MULTI:
    break;
  default:
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_segmented_validate_selected_states(const M3Bool *states,
                                                 m3_usize count) {
  m3_usize i;
  int rc;

  if (count == 0) {
    return M3_OK;
  }
  if (states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    rc = m3_tabs_validate_bool(states[i]);
    if (rc != M3_OK) {
      return rc;
    }
  }
  return M3_OK;
}

static int m3_segmented_measure_max_text(const M3SegmentedButtons *buttons,
                                         M3Scalar *out_width,
                                         M3Scalar *out_height,
                                         M3Scalar *out_baseline) {
  M3TextMetrics metrics;
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  m3_usize i;
  int rc;

  if (buttons == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_width = 0.0f;
  max_height = 0.0f;
  max_baseline = 0.0f;

  if (buttons->item_count == 0) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    *out_baseline = 0.0f;
    return M3_OK;
  }
  if (buttons->items == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < buttons->item_count; ++i) {
    if (buttons->items[i].utf8_len == 0) {
      continue;
    }
    rc = m3_text_measure_utf8(&buttons->text_backend, buttons->font,
                              buttons->items[i].utf8_label,
                              buttons->items[i].utf8_len, &metrics);
    if (rc != M3_OK) {
      return rc;
    }
    if (metrics.width > max_width) {
      max_width = metrics.width;
    }
    if (metrics.height > max_height) {
      max_height = metrics.height;
    }
    if (metrics.baseline > max_baseline) {
      max_baseline = metrics.baseline;
    }
  }

  *out_width = max_width;
  *out_height = max_height;
  *out_baseline = max_baseline;
  return M3_OK;
}

static int m3_segmented_measure_content(const M3SegmentedButtons *buttons,
                                        M3Scalar *out_width,
                                        M3Scalar *out_height) {
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  M3Scalar segment_width;
  M3Scalar segment_height;
  M3Scalar content_width;
  M3Scalar content_height;
  int rc;

  if (buttons == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_measure_max_text(buttons, &max_width, &max_height,
                                     &max_baseline);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(max_baseline);

  segment_width = max_width + buttons->style.padding_x * 2.0f;
  if (segment_width < buttons->style.min_width) {
    segment_width = buttons->style.min_width;
  }

  segment_height = max_height + buttons->style.padding_y * 2.0f;
  if (segment_height < buttons->style.min_height) {
    segment_height = buttons->style.min_height;
  }

  if (buttons->item_count > 0) {
    content_width = segment_width * (M3Scalar)buttons->item_count;
    content_width +=
        buttons->style.spacing * (M3Scalar)(buttons->item_count - 1);
  } else {
    content_width = 0.0f;
  }
  content_height = segment_height;

  if (content_width < 0.0f || content_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_width = buttons->style.padding.left + buttons->style.padding.right +
               content_width;
  *out_height = buttons->style.padding.top + buttons->style.padding.bottom +
                content_height;
  if (*out_width < 0.0f || *out_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_segmented_compute_layout(const M3SegmentedButtons *buttons,
                                       M3SegmentedLayout *out_layout) {
  M3SegmentedStyle style;
  M3Rect bounds;
  M3Scalar available_width;
  M3Scalar available_height;
  M3Scalar spacing;
  M3Scalar max_width;
  M3Scalar max_height;
  M3Scalar max_baseline;
  M3Scalar segment_height;
  M3Scalar segment_width;
  M3Scalar total_spacing;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (buttons == NULL || out_layout == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  style = buttons->style;
  bounds = buttons->bounds;

  rc = m3_segmented_validate_style(&style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  available_width = bounds.width - style.padding.left - style.padding.right;
  available_height = bounds.height - style.padding.top - style.padding.bottom;
  if (available_width < 0.0f || available_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_segmented_measure_max_text(buttons, &max_width, &max_height,
                                     &max_baseline);
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(max_baseline);

  spacing = style.spacing;
#ifdef M3_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    spacing = -1.0f;
  }
#endif
  if (spacing < 0.0f) {
    return M3_ERR_RANGE;
  }

  segment_height = max_height + style.padding_y * 2.0f;
  if (segment_height < style.min_height) {
    segment_height = style.min_height;
  }
#ifdef M3_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    segment_height = -1.0f;
  }
#endif
  if (segment_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (buttons->item_count > 0 && available_height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (segment_height > available_height && available_height > 0.0f) {
    segment_height = available_height;
  }

  if (buttons->item_count > 0) {
    total_spacing = spacing * (M3Scalar)(buttons->item_count - 1);
    if (available_width < total_spacing) {
      return M3_ERR_RANGE;
    }
    segment_width =
        (available_width - total_spacing) / (M3Scalar)buttons->item_count;
#ifdef M3_TESTING
    rc = m3_segmented_test_fail_point_match(
        M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE, &match);
    if (rc != M3_OK) {
      return rc;
    }
    if (match == M3_TRUE) {
      segment_width = -1.0f;
    }
#endif
    if (segment_width < 0.0f) {
      return M3_ERR_RANGE;
    }
  } else {
    segment_width = 0.0f;
  }

  memset(out_layout, 0, sizeof(*out_layout));
  out_layout->start_x = bounds.x + style.padding.left;
  out_layout->start_y = bounds.y + style.padding.top;
  out_layout->segment_width = segment_width;
  out_layout->segment_height = segment_height;
  out_layout->spacing = spacing;
  if (buttons->item_count > 0) {
    out_layout->content_width = segment_width * (M3Scalar)buttons->item_count +
                                spacing * (M3Scalar)(buttons->item_count - 1);
  } else {
    out_layout->content_width = 0.0f;
  }
  out_layout->content_height = segment_height;

  if (available_height > segment_height) {
    out_layout->start_y += (available_height - segment_height) * 0.5f;
  }

  return M3_OK;
}

static int m3_segmented_hit_test(const M3SegmentedButtons *buttons,
                                 const M3SegmentedLayout *layout, m3_i32 x,
                                 m3_i32 y, m3_usize *out_index) {
  M3Scalar fx;
  M3Scalar fy;
  M3Scalar stride;
  M3Scalar pos;
  m3_usize index;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (buttons == NULL || layout == NULL || out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_index = M3_SEGMENTED_INVALID_INDEX;
  if (buttons->item_count == 0) {
    return M3_OK;
  }

  fx = (M3Scalar)x;
  fy = (M3Scalar)y;

  if (fx < layout->start_x || fx > layout->start_x + layout->content_width) {
    return M3_OK;
  }
  if (fy < layout->start_y || fy > layout->start_y + layout->segment_height) {
    return M3_OK;
  }

  stride = layout->segment_width + layout->spacing;
  if (stride <= 0.0f) {
    return M3_ERR_RANGE;
  }

  pos = fx - layout->start_x;
#ifdef M3_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    pos = -1.0f;
  }
#endif
  if (pos < 0.0f) {
    return M3_OK;
  }

  index = (m3_usize)(pos / stride);
  if (index >= buttons->item_count) {
    return M3_OK;
  }
  if (pos > (M3Scalar)index * stride + layout->segment_width) {
    return M3_OK;
  }

  *out_index = index;
  return M3_OK;
}

static int m3_segmented_is_selected(const M3SegmentedButtons *buttons,
                                    m3_usize index, M3Bool *out_selected) {
  int rc;

  if (buttons == NULL || out_selected == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (index >= buttons->item_count) {
    return M3_ERR_RANGE;
  }

  if (buttons->mode == M3_SEGMENTED_MODE_SINGLE) {
    *out_selected = (buttons->selected_index == index) ? M3_TRUE : M3_FALSE;
    return M3_OK;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return M3_ERR_RANGE;
  }
  if (buttons->selected_states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  rc = m3_tabs_validate_bool(buttons->selected_states[index]);
  if (rc != M3_OK) {
    return rc;
  }
  *out_selected = buttons->selected_states[index];
  return M3_OK;
}

static int m3_segmented_widget_measure(void *widget, M3MeasureSpec width,
                                       M3MeasureSpec height, M3Size *out_size) {
  M3SegmentedButtons *buttons;
  M3Scalar content_width;
  M3Scalar content_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = m3_segmented_validate_style(&buttons->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_items(buttons->items, buttons->item_count);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_measure_content(buttons, &content_width, &content_height);
  if (rc != M3_OK) {
    return rc;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return M3_OK;
}

static int m3_segmented_widget_layout(void *widget, M3Rect bounds) {
  M3SegmentedButtons *buttons;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  buttons = (M3SegmentedButtons *)widget;
  buttons->bounds = bounds;
  return M3_OK;
}

static int m3_segmented_widget_paint(void *widget, M3PaintContext *ctx) {
  M3SegmentedButtons *buttons;
  M3SegmentedLayout layout;
  M3Rect segment_rect;
  M3Rect inner_rect;
  M3TextMetrics metrics;
  M3Scalar text_x;
  M3Scalar text_y;
  M3Scalar outline_width;
  M3Scalar inner_corner;
  M3Scalar corner_radius;
  M3Color bg_color;
  M3Color text_color;
  M3Color outline_color;
  M3Bool selected;
  m3_usize i;
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = m3_segmented_validate_style(&buttons->style, M3_FALSE);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_rect(&buttons->bounds);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_validate_items(buttons->items, buttons->item_count);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_compute_layout(buttons, &layout);
  if (rc != M3_OK) {
    return rc;
  }

  if (buttons->item_count == 0) {
    return M3_OK;
  }

  if (buttons->text_backend.vtable == NULL ||
      buttons->text_backend.vtable->measure_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  outline_width = buttons->style.outline_width;
  if (outline_width < 0.0f) {
    return M3_ERR_RANGE;
  }

  for (i = 0; i < buttons->item_count; ++i) {
    segment_rect.x =
        layout.start_x + (layout.segment_width + layout.spacing) * (M3Scalar)i;
    segment_rect.y = layout.start_y;
    segment_rect.width = layout.segment_width;
    segment_rect.height = layout.segment_height;
#ifdef M3_TESTING
    rc = m3_segmented_test_fail_point_match(
        M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE, &match);
    if (rc != M3_OK) {
      return rc;
    }
    if (match == M3_TRUE) {
      segment_rect.width = -1.0f;
    }
#endif
    if (segment_rect.width < 0.0f || segment_rect.height < 0.0f) {
      return M3_ERR_RANGE;
    }

    corner_radius = 0.0f;
    if (i == 0 || i + 1 == buttons->item_count) {
      corner_radius = buttons->style.corner_radius;
    }

    rc = m3_segmented_is_selected(buttons, i, &selected);
    if (rc != M3_OK) {
      return rc;
    }

    if (buttons->widget.flags & M3_WIDGET_FLAG_DISABLED) {
      bg_color = buttons->style.disabled_background_color;
      text_color = buttons->style.disabled_text_color;
      outline_color = buttons->style.disabled_outline_color;
      if (selected == M3_TRUE) {
        bg_color = buttons->style.disabled_selected_background_color;
        text_color = buttons->style.disabled_selected_text_color;
      }
    } else {
      bg_color = buttons->style.background_color;
      text_color = buttons->style.text_style.color;
      outline_color = buttons->style.outline_color;
      if (selected == M3_TRUE) {
        bg_color = buttons->style.selected_background_color;
        text_color = buttons->style.selected_text_color;
      }
    }

    inner_rect = segment_rect;
    inner_corner = corner_radius;
    if (outline_width > 0.0f) {
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &segment_rect,
                                       outline_color, corner_radius);
      if (rc != M3_OK) {
        return rc;
      }
      inner_rect.x = segment_rect.x + outline_width;
      inner_rect.y = segment_rect.y + outline_width;
      inner_rect.width = segment_rect.width - outline_width * 2.0f;
      inner_rect.height = segment_rect.height - outline_width * 2.0f;
      if (inner_rect.width < 0.0f || inner_rect.height < 0.0f) {
        return M3_ERR_RANGE;
      }
      inner_corner = corner_radius - outline_width;
      if (inner_corner < 0.0f) {
        inner_corner = 0.0f;
      }
    }

    if (bg_color.a > 0.0f) {
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner_rect, bg_color,
                                       inner_corner);
      if (rc != M3_OK) {
        return rc;
      }
    }

    if (buttons->items[i].utf8_len == 0) {
      continue;
    }

    rc = m3_text_measure_utf8(&buttons->text_backend, buttons->font,
                              buttons->items[i].utf8_label,
                              buttons->items[i].utf8_len, &metrics);
    if (rc != M3_OK) {
      return rc;
    }

    text_x = segment_rect.x + (segment_rect.width - metrics.width) * 0.5f;
    text_y = segment_rect.y + (segment_rect.height - metrics.height) * 0.5f +
             metrics.baseline;

    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, buttons->font, buttons->items[i].utf8_label,
        buttons->items[i].utf8_len, text_x, text_y, text_color);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

static int m3_segmented_widget_event(void *widget, const M3InputEvent *event,
                                     M3Bool *out_handled) {
  M3SegmentedButtons *buttons;
  M3SegmentedLayout layout;
  m3_usize index;
  m3_usize previous;
  M3Bool selected;
  M3Bool new_selected;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;
  buttons = (M3SegmentedButtons *)widget;

  if (buttons->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  if (event->type != M3_INPUT_POINTER_DOWN &&
      event->type != M3_INPUT_POINTER_UP) {
    return M3_OK;
  }

  rc = m3_segmented_validate_mode(buttons->mode);
  if (rc != M3_OK) {
    return rc;
  }
  if (buttons->mode == M3_SEGMENTED_MODE_MULTI && buttons->item_count > 0 &&
      buttons->selected_states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_compute_layout(buttons, &layout);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_hit_test(buttons, &layout, event->data.pointer.x,
                             event->data.pointer.y, &index);
  if (rc != M3_OK) {
    return rc;
  }

  if (event->type == M3_INPUT_POINTER_DOWN) {
    if (buttons->pressed_index != M3_SEGMENTED_INVALID_INDEX) {
      return M3_ERR_STATE;
    }
    if (index == M3_SEGMENTED_INVALID_INDEX) {
      return M3_OK;
    }
    buttons->pressed_index = index;
    *out_handled = M3_TRUE;
    return M3_OK;
  }

  if (buttons->pressed_index == M3_SEGMENTED_INVALID_INDEX) {
    return M3_OK;
  }

  if (index == buttons->pressed_index) {
    if (buttons->mode == M3_SEGMENTED_MODE_SINGLE) {
      previous = buttons->selected_index;
      buttons->selected_index = index;
      if (buttons->on_select != NULL) {
        rc =
            buttons->on_select(buttons->on_select_ctx, buttons, index, M3_TRUE);
        if (rc != M3_OK) {
          buttons->selected_index = previous;
          buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
          return rc;
        }
      }
    } else if (buttons->mode == M3_SEGMENTED_MODE_MULTI) {
      if (buttons->selected_states == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
      }
      rc = m3_segmented_is_selected(buttons, index, &selected);
      if (rc != M3_OK) {
        return rc;
      }
      new_selected = (selected == M3_TRUE) ? M3_FALSE : M3_TRUE;
      buttons->selected_states[index] = new_selected;
      if (buttons->on_select != NULL) {
        rc = buttons->on_select(buttons->on_select_ctx, buttons, index,
                                new_selected);
        if (rc != M3_OK) {
          buttons->selected_states[index] = selected;
          buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
          return rc;
        }
      }
    } else {
      return M3_ERR_RANGE;
    }
  }

  buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
  *out_handled = M3_TRUE;
  return M3_OK;
}

static int m3_segmented_widget_get_semantics(void *widget,
                                             M3Semantics *out_semantics) {
  M3SegmentedButtons *buttons;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  buttons = (M3SegmentedButtons *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (buttons->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (buttons->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_segmented_widget_destroy(void *widget) {
  M3SegmentedButtons *buttons;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = M3_OK;
  if (buttons->owns_font == M3_TRUE &&
      (buttons->font.id != 0u || buttons->font.generation != 0u)) {
    if (buttons->text_backend.vtable != NULL &&
        buttons->text_backend.vtable->destroy_font != NULL) {
      rc = buttons->text_backend.vtable->destroy_font(buttons->text_backend.ctx,
                                                      buttons->font);
    } else {
      rc = M3_ERR_UNSUPPORTED;
    }
  }

  buttons->font.id = 0u;
  buttons->font.generation = 0u;
  buttons->items = NULL;
  buttons->item_count = 0;
  buttons->selected_index = M3_SEGMENTED_INVALID_INDEX;
  buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
  buttons->selected_states = NULL;
  buttons->bounds.x = 0.0f;
  buttons->bounds.y = 0.0f;
  buttons->bounds.width = 0.0f;
  buttons->bounds.height = 0.0f;
  buttons->text_backend.ctx = NULL;
  buttons->text_backend.vtable = NULL;
  buttons->widget.ctx = NULL;
  buttons->widget.vtable = NULL;
  buttons->on_select = NULL;
  buttons->on_select_ctx = NULL;
  buttons->owns_font = M3_FALSE;
  return rc;
}

static const M3WidgetVTable g_m3_segmented_widget_vtable = {
    m3_segmented_widget_measure,       m3_segmented_widget_layout,
    m3_segmented_widget_paint,         m3_segmented_widget_event,
    m3_segmented_widget_get_semantics, m3_segmented_widget_destroy};

int M3_CALL m3_segmented_style_init(M3SegmentedStyle *style) {
  int rc;
#ifdef M3_TESTING
  M3Bool match;
#endif

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_text_style_init(&style->text_style);
#ifdef M3_TESTING
  if (rc == M3_OK) {
    rc = m3_segmented_test_fail_point_match(M3_SEGMENTED_TEST_FAIL_STYLE_INIT,
                                            &match);
    if (rc == M3_OK && match == M3_TRUE) {
      rc = M3_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  style->spacing = M3_SEGMENTED_DEFAULT_SPACING;
  style->min_width = M3_SEGMENTED_DEFAULT_MIN_WIDTH;
  style->min_height = M3_SEGMENTED_DEFAULT_MIN_HEIGHT;
  style->padding_x = M3_SEGMENTED_DEFAULT_PADDING_X;
  style->padding_y = M3_SEGMENTED_DEFAULT_PADDING_Y;
  style->outline_width = M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH;
  style->corner_radius = M3_SEGMENTED_DEFAULT_CORNER_RADIUS;

  rc = m3_segmented_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->outline_color, 0.44f, 0.44f, 0.44f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->selected_background_color, 0.26f, 0.52f,
                              0.96f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->selected_text_color, 1.0f, 1.0f, 1.0f,
                              1.0f);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_color_with_alpha(&style->background_color, 0.12f,
                                     &style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->outline_color, 0.12f,
                                     &style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->text_style.color, 0.38f,
                                     &style->disabled_text_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc =
      m3_segmented_color_with_alpha(&style->selected_background_color, 0.12f,
                                    &style->disabled_selected_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->selected_text_color, 0.38f,
                                     &style->disabled_selected_text_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_segmented_buttons_init(M3SegmentedButtons *buttons,
                                      const M3TextBackend *backend,
                                      const M3SegmentedStyle *style,
                                      const M3SegmentedItem *items,
                                      m3_usize item_count, m3_u32 mode,
                                      m3_usize selected_index,
                                      M3Bool *selected_states) {
  int rc;

  if (buttons == NULL || backend == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_backend(backend);
  if (rc != M3_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_segmented_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_segmented_validate_mode(mode);
  if (rc != M3_OK) {
    return rc;
  }

  if (mode == M3_SEGMENTED_MODE_SINGLE) {
    if (selected_index != M3_SEGMENTED_INVALID_INDEX &&
        selected_index >= item_count) {
      return M3_ERR_RANGE;
    }
  } else {
    rc = m3_segmented_validate_selected_states(selected_states, item_count);
    if (rc != M3_OK) {
      return rc;
    }
  }

  memset(buttons, 0, sizeof(*buttons));
  buttons->text_backend = *backend;
  buttons->style = *style;
  buttons->items = items;
  buttons->item_count = item_count;
  buttons->mode = mode;
  buttons->selected_index = selected_index;
  buttons->selected_states = selected_states;
  buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
  buttons->bounds.x = 0.0f;
  buttons->bounds.y = 0.0f;
  buttons->bounds.width = 0.0f;
  buttons->bounds.height = 0.0f;

  rc = m3_text_font_create(backend, &style->text_style, &buttons->font);
  if (rc != M3_OK) {
    return rc;
  }

  buttons->owns_font = M3_TRUE;
  buttons->widget.ctx = buttons;
  buttons->widget.vtable = &g_m3_segmented_widget_vtable;
  buttons->widget.handle.id = 0u;
  buttons->widget.handle.generation = 0u;
  buttons->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_set_items(M3SegmentedButtons *buttons,
                                           const M3SegmentedItem *items,
                                           m3_usize item_count) {
  int rc;

  if (buttons == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_items(items, item_count);
  if (rc != M3_OK) {
    return rc;
  }

  if (buttons->mode == M3_SEGMENTED_MODE_MULTI && item_count > 0 &&
      buttons->selected_states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  buttons->items = items;
  buttons->item_count = item_count;
  if (buttons->selected_index != M3_SEGMENTED_INVALID_INDEX &&
      buttons->selected_index >= item_count) {
    buttons->selected_index = M3_SEGMENTED_INVALID_INDEX;
  }
  if (buttons->pressed_index != M3_SEGMENTED_INVALID_INDEX &&
      buttons->pressed_index >= item_count) {
    buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
  }

  return M3_OK;
}

int M3_CALL m3_segmented_buttons_set_style(M3SegmentedButtons *buttons,
                                           const M3SegmentedStyle *style) {
  M3Handle new_font;
  int rc;

  if (buttons == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_style(style, M3_TRUE);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_text_font_create(&buttons->text_backend, &style->text_style,
                           &new_font);
  if (rc != M3_OK) {
    return rc;
  }

  if (buttons->owns_font == M3_TRUE) {
    rc = m3_text_font_destroy(&buttons->text_backend, buttons->font);
    if (rc != M3_OK) {
      m3_text_font_destroy(&buttons->text_backend, new_font);
      return rc;
    }
  }

  buttons->style = *style;
  buttons->font = new_font;
  buttons->owns_font = M3_TRUE;
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_set_selected_index(M3SegmentedButtons *buttons,
                                                    m3_usize selected_index) {
  if (buttons == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_SINGLE) {
    return M3_ERR_STATE;
  }
  if (selected_index != M3_SEGMENTED_INVALID_INDEX &&
      selected_index >= buttons->item_count) {
    return M3_ERR_RANGE;
  }
  buttons->selected_index = selected_index;
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_get_selected_index(
    const M3SegmentedButtons *buttons, m3_usize *out_selected) {
  if (buttons == NULL || out_selected == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_SINGLE) {
    return M3_ERR_STATE;
  }
  *out_selected = buttons->selected_index;
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_set_selected_state(M3SegmentedButtons *buttons,
                                                    m3_usize index,
                                                    M3Bool selected) {
  int rc;

  if (buttons == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return M3_ERR_STATE;
  }
  if (index >= buttons->item_count) {
    return M3_ERR_RANGE;
  }
  if (buttons->selected_states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tabs_validate_bool(selected);
  if (rc != M3_OK) {
    return rc;
  }
  buttons->selected_states[index] = selected;
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_get_selected_state(
    const M3SegmentedButtons *buttons, m3_usize index, M3Bool *out_selected) {
  int rc;

  if (buttons == NULL || out_selected == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return M3_ERR_STATE;
  }
  if (index >= buttons->item_count) {
    return M3_ERR_RANGE;
  }
  if (buttons->selected_states == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tabs_validate_bool(buttons->selected_states[index]);
  if (rc != M3_OK) {
    return rc;
  }
  *out_selected = buttons->selected_states[index];
  return M3_OK;
}

int M3_CALL m3_segmented_buttons_set_on_select(M3SegmentedButtons *buttons,
                                               M3SegmentedOnSelect on_select,
                                               void *ctx) {
  if (buttons == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  buttons->on_select = on_select;
  buttons->on_select_ctx = ctx;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_tab_row_test_validate_color(const M3Color *color) {
  return m3_tab_row_validate_color(color);
}

int M3_CALL m3_tab_row_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                      M3Scalar b, M3Scalar a) {
  return m3_tab_row_color_set(color, r, g, b, a);
}

int M3_CALL m3_tab_row_test_color_with_alpha(const M3Color *base,
                                             M3Scalar alpha,
                                             M3Color *out_color) {
  return m3_tab_row_color_with_alpha(base, alpha, out_color);
}

int M3_CALL m3_tab_row_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_tab_row_validate_edges(edges);
}

int M3_CALL m3_tab_row_test_validate_text_style(const M3TextStyle *style,
                                                M3Bool require_family) {
  return m3_tab_row_validate_text_style(style, require_family);
}

int M3_CALL m3_tab_row_test_validate_style(const M3TabRowStyle *style,
                                           M3Bool require_family) {
  return m3_tab_row_validate_style(style, require_family);
}

int M3_CALL m3_tab_row_test_validate_items(const M3TabItem *items,
                                           m3_usize count) {
  return m3_tab_row_validate_items(items, count);
}

int M3_CALL m3_tab_row_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_tab_row_validate_measure_spec(spec);
}

int M3_CALL m3_tab_row_test_validate_rect(const M3Rect *rect) {
  return m3_tab_row_validate_rect(rect);
}

int M3_CALL m3_tab_row_test_validate_backend(const M3TextBackend *backend) {
  return m3_tab_row_validate_backend(backend);
}

int M3_CALL m3_tab_row_test_measure_content(const M3TabRow *row, m3_u32 mode,
                                            M3Scalar *out_width,
                                            M3Scalar *out_height) {
  return m3_tab_row_measure_content(row, mode, out_width, out_height);
}

int M3_CALL m3_segmented_test_validate_color(const M3Color *color) {
  return m3_segmented_validate_color(color);
}

int M3_CALL m3_segmented_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                        M3Scalar b, M3Scalar a) {
  return m3_segmented_color_set(color, r, g, b, a);
}

int M3_CALL m3_segmented_test_color_with_alpha(const M3Color *base,
                                               M3Scalar alpha,
                                               M3Color *out_color) {
  return m3_segmented_color_with_alpha(base, alpha, out_color);
}

int M3_CALL m3_segmented_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_segmented_validate_edges(edges);
}

int M3_CALL m3_segmented_test_validate_text_style(const M3TextStyle *style,
                                                  M3Bool require_family) {
  return m3_segmented_validate_text_style(style, require_family);
}

int M3_CALL m3_segmented_test_validate_style(const M3SegmentedStyle *style,
                                             M3Bool require_family) {
  return m3_segmented_validate_style(style, require_family);
}

int M3_CALL m3_segmented_test_validate_items(const M3SegmentedItem *items,
                                             m3_usize count) {
  return m3_segmented_validate_items(items, count);
}

int M3_CALL m3_segmented_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_segmented_validate_measure_spec(spec);
}

int M3_CALL m3_segmented_test_validate_rect(const M3Rect *rect) {
  return m3_segmented_validate_rect(rect);
}

int M3_CALL m3_segmented_test_validate_backend(const M3TextBackend *backend) {
  return m3_segmented_validate_backend(backend);
}

int M3_CALL m3_segmented_test_validate_mode(m3_u32 mode) {
  return m3_segmented_validate_mode(mode);
}

int M3_CALL m3_segmented_test_validate_selected_states(const M3Bool *states,
                                                       m3_usize count) {
  return m3_segmented_validate_selected_states(states, count);
}

int M3_CALL m3_segmented_test_measure_content(const M3SegmentedButtons *buttons,
                                              M3Scalar *out_width,
                                              M3Scalar *out_height) {
  return m3_segmented_measure_content(buttons, out_width, out_height);
}
#endif
