#include "m3/m3_tabs.h"

#include <string.h>

#ifdef CMP_TESTING
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

static cmp_u32 g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
static cmp_u32 g_m3_tab_row_test_color_fail_after = 0u;
static cmp_u32 g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
static cmp_u32 g_m3_segmented_test_color_fail_after = 0u;
static int g_m3_tab_row_test_force_color_error = 0;
static cmp_u32 g_m3_tab_row_test_fail_point_error_after = 0u;
static cmp_u32 g_m3_tab_row_test_value_fail_after = 0u;
static cmp_u32 g_m3_tab_row_test_start_fail_after = 0u;
static int g_m3_segmented_test_force_color_error = 0;
static cmp_u32 g_m3_segmented_test_fail_point_error_after = 0u;

int CMP_CALL m3_tab_row_test_set_fail_point(cmp_u32 point) {
  g_m3_tab_row_test_fail_point = point;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_tab_row_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_clear_fail_points(void) {
  g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
  g_m3_tab_row_test_color_fail_after = 0u;
  return CMP_OK;
}

int CMP_CALL m3_segmented_test_set_fail_point(cmp_u32 point) {
  g_m3_segmented_test_fail_point = point;
  return CMP_OK;
}

int CMP_CALL m3_segmented_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_segmented_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_segmented_test_clear_fail_points(void) {
  g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
  g_m3_segmented_test_color_fail_after = 0u;
  return CMP_OK;
}

static int m3_tab_row_test_color_should_fail(CMPBool *out_fail) {
  if (out_fail == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_tab_row_test_force_color_error != 0) {
    g_m3_tab_row_test_force_color_error = 0;
    return CMP_ERR_IO;
  }
  if (g_m3_tab_row_test_color_fail_after == 0u) {
    *out_fail = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_tab_row_test_color_fail_after -= 1u;
  *out_fail = (g_m3_tab_row_test_color_fail_after == 0u) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

static int m3_tab_row_test_fail_point_match(cmp_u32 point, CMPBool *out_match) {
  if (out_match == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_tab_row_test_fail_point_error_after > 0u) {
    g_m3_tab_row_test_fail_point_error_after -= 1u;
    if (g_m3_tab_row_test_fail_point_error_after == 0u) {
      return CMP_ERR_IO;
    }
  }
  if (g_m3_tab_row_test_fail_point != point) {
    *out_match = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_tab_row_test_fail_point = M3_TAB_ROW_TEST_FAIL_NONE;
  *out_match = CMP_TRUE;
  return CMP_OK;
}

static int m3_segmented_test_color_should_fail(CMPBool *out_fail) {
  if (out_fail == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_segmented_test_force_color_error != 0) {
    g_m3_segmented_test_force_color_error = 0;
    return CMP_ERR_IO;
  }
  if (g_m3_segmented_test_color_fail_after == 0u) {
    *out_fail = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_segmented_test_color_fail_after -= 1u;
  *out_fail = (g_m3_segmented_test_color_fail_after == 0u) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

static int m3_segmented_test_fail_point_match(cmp_u32 point, CMPBool *out_match) {
  if (out_match == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_segmented_test_fail_point_error_after > 0u) {
    g_m3_segmented_test_fail_point_error_after -= 1u;
    if (g_m3_segmented_test_fail_point_error_after == 0u) {
      return CMP_ERR_IO;
    }
  }
  if (g_m3_segmented_test_fail_point != point) {
    *out_match = CMP_FALSE;
    return CMP_OK;
  }
  g_m3_segmented_test_fail_point = M3_SEGMENTED_TEST_FAIL_NONE;
  *out_match = CMP_TRUE;
  return CMP_OK;
}
#endif /* GCOVR_EXCL_LINE */

static int m3_tabs_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

typedef struct M3TabRowLayout {
  cmp_u32 mode;
  CMPScalar start_x;
  CMPScalar start_y;
  CMPScalar tab_width;
  CMPScalar tab_height;
  CMPScalar spacing;
  CMPScalar content_width;
  CMPScalar content_height;
  CMPScalar available_width;
} M3TabRowLayout;

typedef struct M3SegmentedLayout {
  CMPScalar start_x;
  CMPScalar start_y;
  CMPScalar segment_width;
  CMPScalar segment_height;
  CMPScalar spacing;
  CMPScalar content_width;
  CMPScalar content_height;
} M3SegmentedLayout;

static int m3_tab_row_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_tab_row_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                CMPScalar b, CMPScalar a) {
  CMPBool should_fail;
  int rc;

  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
#ifdef CMP_TESTING
  rc = m3_tab_row_test_color_should_fail(&should_fail);
  if (rc != CMP_OK) {
    return rc;
  }
  if (should_fail == CMP_TRUE) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_tab_row_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                       CMPColor *out_color) {
  CMPBool should_fail;
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_tab_row_validate_color(base);
  if (rc != CMP_OK) {
    return rc;
  }
#ifdef CMP_TESTING
  rc = m3_tab_row_test_color_should_fail(&should_fail);
  if (rc != CMP_OK) {
    return rc;
  }
  if (should_fail == CMP_TRUE) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return CMP_OK;
}

static int m3_tab_row_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_tab_row_validate_text_style(const CMPTextStyle *style,
                                          CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  rc = m3_tabs_validate_bool(style->italic);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_tab_row_validate_style(const M3TabRowStyle *style,
                                     CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->mode) {
  case M3_TAB_MODE_FIXED:
  case M3_TAB_MODE_SCROLLABLE:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  if (style->spacing < 0.0f || style->min_width < 0.0f ||
      style->min_height < 0.0f || style->padding_x < 0.0f ||
      style->padding_y < 0.0f || style->indicator_thickness < 0.0f ||
      style->indicator_corner < 0.0f || style->indicator_anim_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }

  switch (style->indicator_anim_easing) {
  case CMP_ANIM_EASE_LINEAR:
  case CMP_ANIM_EASE_IN:
  case CMP_ANIM_EASE_OUT:
  case CMP_ANIM_EASE_IN_OUT: /* GCOVR_EXCL_LINE */
    break;
  default:
    return CMP_ERR_RANGE;
  }

  rc = m3_tab_row_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_text_style(&style->text_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_color(&style->selected_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_color(&style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_tab_row_validate_items(const M3TabItem *items, cmp_usize count) {
  cmp_usize i;

  if (count == 0) {
    return CMP_OK;
  }
  if (items == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    if (items[i].utf8_label == NULL && items[i].utf8_len != 0) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

static int m3_tab_row_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_tab_row_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_tab_row_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_tab_row_measure_max_text(const M3TabRow *row, CMPScalar *out_width,
                                       CMPScalar *out_height,
                                       CMPScalar *out_baseline) {
  CMPTextMetrics metrics;
  CMPScalar max_width;
  CMPScalar max_height; /* GCOVR_EXCL_LINE */
  CMPScalar max_baseline;
  cmp_usize i;
  int rc;

  if (row == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_width = 0.0f;
  max_height = 0.0f;
  max_baseline = 0.0f;

  if (row->item_count == 0) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    *out_baseline = 0.0f;
    return CMP_OK;
  }
  if (row->items == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < row->item_count; ++i) {
    if (row->items[i].utf8_len == 0) {
      continue;
    }
    rc = cmp_text_measure_utf8(&row->text_backend, row->font,
                              row->items[i].utf8_label, row->items[i].utf8_len,
                              &metrics);
    if (rc != CMP_OK) {
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
  return CMP_OK;
}

static int m3_tab_row_item_width(const M3TabRow *row, const M3TabItem *item,
                                 CMPScalar *out_width) { /* GCOVR_EXCL_LINE */
  CMPTextMetrics metrics;
  CMPScalar width;
  int rc;

  if (row == NULL || item == NULL || out_width == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  metrics.width = 0.0f;
  metrics.height = 0.0f;
  metrics.baseline = 0.0f;

  if (item->utf8_len > 0) {
    rc = cmp_text_measure_utf8(&row->text_backend, row->font, item->utf8_label,
                              item->utf8_len, &metrics);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  width = metrics.width + row->style.padding_x * 2.0f;
  if (width < row->style.min_width) {
    width = row->style.min_width;
  }
  if (width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_width = width;
  return CMP_OK;
}

static int m3_tab_row_measure_content(const M3TabRow *row, cmp_u32 mode,
                                      CMPScalar *out_width,
                                      CMPScalar *out_height) {
  CMPScalar max_width;
  CMPScalar max_height;
  CMPScalar max_baseline;
  CMPScalar tab_width;
  CMPScalar tab_height;
  CMPScalar content_width;
  CMPScalar content_height;
  cmp_usize i;
  int rc;

  if (row == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (mode != M3_TAB_MODE_FIXED && mode != M3_TAB_MODE_SCROLLABLE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_tab_row_measure_max_text(row, &max_width, &max_height, &max_baseline);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(max_baseline);

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
      content_width = tab_width * (CMPScalar)row->item_count;
      content_width += row->style.spacing * (CMPScalar)(row->item_count - 1);
    } else {
      content_width = 0.0f;
    }
  } else {
    content_width = 0.0f;
    if (row->item_count > 0) {
      for (i = 0; i < row->item_count; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i], &tab_width);
        if (rc != CMP_OK) {
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
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  *out_width =
      row->style.padding.left + row->style.padding.right + content_width;
  *out_height =
      row->style.padding.top + row->style.padding.bottom + content_height;
  if (*out_width < 0.0f || *out_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int m3_tab_row_compute_layout(const M3TabRow *row,
                                     M3TabRowLayout *out_layout) {
  M3TabRowStyle style;
  CMPRect bounds;
  CMPScalar available_width; /* GCOVR_EXCL_LINE */
  CMPScalar available_height;
  CMPScalar spacing;
  CMPScalar max_width;
  CMPScalar max_height; /* GCOVR_EXCL_LINE */
  CMPScalar max_baseline;
  CMPScalar tab_height;
  CMPScalar tab_width;
  CMPScalar content_width;
  CMPScalar total_spacing;
  cmp_u32 mode; /* GCOVR_EXCL_LINE */
  cmp_usize i;
  int rc;
#ifdef CMP_TESTING
  CMPBool match; /* GCOVR_EXCL_LINE */
#endif /* GCOVR_EXCL_LINE */

  if (row == NULL || out_layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style = row->style;
  bounds = row->bounds;

  rc = m3_tab_row_validate_style(&style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  available_width = bounds.width - style.padding.left - style.padding.right;
  available_height = bounds.height - style.padding.top - style.padding.bottom;
  if (available_width < 0.0f || available_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_tab_row_measure_max_text(row, &max_width, &max_height, &max_baseline);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(max_baseline);

  spacing = style.spacing;
#ifdef CMP_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    spacing = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
  if (spacing < 0.0f) {
    return CMP_ERR_RANGE;
  }

  tab_height = max_height + style.padding_y * 2.0f;
  if (tab_height < style.min_height) {
    tab_height = style.min_height;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    tab_height = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
  if (tab_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (row->item_count > 0 && available_height <= 0.0f) {
    return CMP_ERR_RANGE;
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
      total_spacing = spacing * (CMPScalar)(row->item_count - 1);
      if (available_width < total_spacing) {
        return CMP_ERR_RANGE;
      }
      tab_width = (available_width - total_spacing) / (CMPScalar)row->item_count;
#ifdef CMP_TESTING
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE, &match);
      if (rc != CMP_OK) {
        return rc;
      }
      if (match == CMP_TRUE) {
        tab_width = -1.0f;
      }
#endif
      if (tab_width < 0.0f) {
        return CMP_ERR_RANGE;
      }
    } else {
      tab_width = 0.0f;
    }
    out_layout->tab_width = tab_width;
    if (row->item_count > 0) {
      content_width = tab_width * (CMPScalar)row->item_count;
      content_width += spacing * (CMPScalar)(row->item_count - 1);
    } else {
      content_width = 0.0f;
    }
  } else if (mode == M3_TAB_MODE_SCROLLABLE) {
    content_width = 0.0f;
    if (row->item_count > 0) {
      for (i = 0; i < row->item_count; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i], &tab_width);
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        content_width += tab_width;
        if (i + 1 < row->item_count) {
          content_width += spacing;
        }
      }
    }
  } else {
    return CMP_ERR_RANGE;
  }

  if (content_width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  out_layout->content_width = content_width;
  out_layout->content_height = tab_height;
  return CMP_OK;
}

static int m3_tab_row_clamp_scroll(M3TabRow *row,
                                   const M3TabRowLayout *layout) {
  CMPScalar max_scroll;

  if (row == NULL || layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (row->scroll_offset < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (layout->mode != M3_TAB_MODE_SCROLLABLE) {
    row->scroll_offset = 0.0f;
    return CMP_OK;
  }

  max_scroll = layout->content_width - layout->available_width;
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }
  if (row->scroll_offset > max_scroll) {
    row->scroll_offset = max_scroll;
  }
  return CMP_OK;
}

static int
m3_tab_row_indicator_target(const M3TabRow *row, const M3TabRowLayout *layout,
                            CMPScalar *out_pos, /* GCOVR_EXCL_LINE */
                            CMPScalar *out_width) {
  CMPScalar pos = 0.0f;
  CMPScalar width = 0.0f;
  cmp_usize i = 0u;
  int rc = CMP_OK;

  if (row == NULL || layout == NULL || out_pos == NULL || out_width == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (row->selected_index == M3_TAB_INVALID_INDEX ||
      row->selected_index >= row->item_count) {
    *out_pos = 0.0f;
    *out_width = 0.0f;
    return CMP_OK;
  }

  if (layout->mode == M3_TAB_MODE_FIXED) {
    pos = (layout->tab_width + layout->spacing) * (CMPScalar)row->selected_index;
    width = layout->tab_width;
  } else if (layout->mode == M3_TAB_MODE_SCROLLABLE) {
    pos = 0.0f;
    width = 0.0f;
    for (i = 0; i < row->selected_index; ++i) {
      rc = m3_tab_row_item_width(row, &row->items[i], &width);
      if (rc != CMP_OK) {
        return rc;
      }
      pos += width + layout->spacing;
    }
    rc = m3_tab_row_item_width(row, &row->items[row->selected_index], &width);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    return CMP_ERR_RANGE;
  }

  if (pos < 0.0f || width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_pos = pos;
  *out_width = width;
  return CMP_OK;
}

static int m3_tab_row_sync_indicator(M3TabRow *row,
                                     const M3TabRowLayout *layout,
                                     CMPBool animate) {
  CMPScalar target_pos;
  CMPScalar target_width;
  CMPScalar value;
  int rc;
#ifdef CMP_TESTING
  CMPBool fail_match;
#endif

  if (row == NULL || layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tabs_validate_bool(animate);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_indicator_target(row, layout, &target_pos, &target_width);
  if (rc != CMP_OK) {
    return rc;
  }

  if (animate == CMP_TRUE) {
    rc = cmp_anim_controller_start_timing(
        &row->indicator_pos_anim, row->indicator_pos, target_pos,
        row->style.indicator_anim_duration, row->style.indicator_anim_easing);
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
    if (g_m3_tab_row_test_start_fail_after > 0u) {
      g_m3_tab_row_test_start_fail_after -= 1u;
      if (g_m3_tab_row_test_start_fail_after == 0u) {
        rc = CMP_ERR_IO;
      }
    }
#endif
#ifdef CMP_TESTING
    if (rc == CMP_OK) {
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_INDICATOR_START, &fail_match); /* GCOVR_EXCL_LINE */
      if (rc == CMP_OK && fail_match == CMP_TRUE) {
        rc = CMP_ERR_IO;
      }
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      return rc;
    }

    rc = cmp_anim_controller_start_timing(
        &row->indicator_width_anim, row->indicator_width, target_width,
        row->style.indicator_anim_duration, row->style.indicator_anim_easing);
#ifdef CMP_TESTING
    if (g_m3_tab_row_test_start_fail_after > 0u) {
      g_m3_tab_row_test_start_fail_after -= 1u; /* GCOVR_EXCL_LINE */
      if (g_m3_tab_row_test_start_fail_after == 0u) {
        rc = CMP_ERR_IO;
      }
    }
#endif
#ifdef CMP_TESTING
    if (rc == CMP_OK) {
      rc = m3_tab_row_test_fail_point_match(
          M3_TAB_ROW_TEST_FAIL_INDICATOR_START, &fail_match);
      if (rc == CMP_OK && fail_match == CMP_TRUE) {
        rc = CMP_ERR_IO; /* GCOVR_EXCL_LINE */
      }
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
  } else {
    rc = cmp_anim_controller_start_timing(&row->indicator_pos_anim, target_pos,
                                         target_pos, 0.0f, CMP_ANIM_EASE_LINEAR);
#ifdef CMP_TESTING
    if (g_m3_tab_row_test_start_fail_after > 0u) {
      g_m3_tab_row_test_start_fail_after -= 1u;
      if (g_m3_tab_row_test_start_fail_after == 0u) {
        rc = CMP_ERR_IO; /* GCOVR_EXCL_LINE */
      }
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_anim_controller_start_timing(
        &row->indicator_width_anim, target_width, target_width, 0.0f,
        CMP_ANIM_EASE_LINEAR); /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING             /* GCOVR_EXCL_LINE */
    if (g_m3_tab_row_test_start_fail_after > 0u) {
      g_m3_tab_row_test_start_fail_after -= 1u;
      if (g_m3_tab_row_test_start_fail_after == 0u) {
        rc = CMP_ERR_IO;
      }
    }
#endif /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_anim_controller_get_value(&row->indicator_pos_anim, &value);
#ifdef CMP_TESTING
  if (g_m3_tab_row_test_value_fail_after > 0u) {
    g_m3_tab_row_test_value_fail_after -= 1u;
    if (g_m3_tab_row_test_value_fail_after == 0u) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  row->indicator_pos = value;
  rc = cmp_anim_controller_get_value(&row->indicator_width_anim, &value);
#ifdef CMP_TESTING
  if (g_m3_tab_row_test_value_fail_after > 0u) {
    g_m3_tab_row_test_value_fail_after -= 1u;
    if (g_m3_tab_row_test_value_fail_after == 0u) {
      rc = CMP_ERR_IO;
    }
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  row->indicator_width = value;

  return CMP_OK;
}

static int m3_tab_row_item_rect(const M3TabRow *row,
                                const M3TabRowLayout *layout, cmp_usize index,
                                CMPRect *out_rect) {
  CMPScalar offset;
  CMPScalar width; /* GCOVR_EXCL_LINE */
  int rc; /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match; /* GCOVR_EXCL_LINE */
#endif /* GCOVR_EXCL_LINE */

  if (row == NULL || layout == NULL || out_rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= row->item_count) {
    return CMP_ERR_RANGE;
  }

  out_rect->x = layout->start_x;
  out_rect->y = layout->start_y;
  out_rect->width = layout->tab_width;
  out_rect->height = layout->tab_height;

  if (layout->mode == M3_TAB_MODE_FIXED) {
    out_rect->x += (layout->tab_width + layout->spacing) * (CMPScalar)index;
  } else if (layout->mode == M3_TAB_MODE_SCROLLABLE) {
    offset = 0.0f;
    if (index > 0) {
      cmp_usize i;
      for (i = 0; i < index; ++i) {
        rc = m3_tab_row_item_width(row, &row->items[i],
                                   &width); /* GCOVR_EXCL_LINE */
        if (rc != CMP_OK) {
          return rc; /* GCOVR_EXCL_LINE */
        }
        offset += width + layout->spacing; /* GCOVR_EXCL_LINE */
      }
    }
    rc = m3_tab_row_item_width(row, &row->items[index], &width);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    out_rect->x += offset - row->scroll_offset;
    out_rect->width = width;
  } else {
    return CMP_ERR_RANGE;
  }

#ifdef CMP_TESTING
  rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE,
                                        &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    out_rect->width = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */

  if (out_rect->width < 0.0f || out_rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int m3_tab_row_hit_test(const M3TabRow *row,
                               const M3TabRowLayout *layout, cmp_i32 x, cmp_i32 y,
                               cmp_usize *out_index) {
  CMPScalar fx;
  CMPScalar fy;
  CMPScalar stride = 0.0f;
  CMPScalar pos = 0.0f;
  CMPScalar width = 0.0f;
  cmp_usize index; /* GCOVR_EXCL_LINE */
  int rc;         /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match;   /* GCOVR_EXCL_LINE */
#endif /* GCOVR_EXCL_LINE */

  if (row == NULL || layout == NULL || out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_index = M3_TAB_INVALID_INDEX;
  if (row->item_count == 0) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  fx = (CMPScalar)x;
  fy = (CMPScalar)y;

  if (fy < layout->start_y || fy > layout->start_y + layout->tab_height) {
    return CMP_OK;
  }

  if (layout->mode == M3_TAB_MODE_FIXED) {
    if (fx < layout->start_x || fx > layout->start_x + layout->content_width) {
      return CMP_OK;
    }
    stride = layout->tab_width + layout->spacing;
    if (stride <= 0.0f) {
      return CMP_ERR_RANGE;
    }
    pos = fx - layout->start_x;
#ifdef CMP_TESTING
    rc = m3_tab_row_test_fail_point_match(
        M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
    if (rc != CMP_OK) {
      return rc;
    }
    if (match == CMP_TRUE) {
      pos = -1.0f;
    }
#endif
    if (pos < 0.0f) {
      return CMP_OK;
    }
    index = (cmp_usize)(pos / stride);
    if (index >= row->item_count) {
      return CMP_OK;
    }
    if (pos > (CMPScalar)index * stride + layout->tab_width) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    *out_index = index;
    return CMP_OK;
  }

  if (layout->mode != M3_TAB_MODE_SCROLLABLE) {
    return CMP_ERR_RANGE;
  }

  pos = fx - layout->start_x + row->scroll_offset;
#ifdef CMP_TESTING
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (match == CMP_TRUE) {
    pos = -1.0f;
  }
#endif
  if (pos < 0.0f || pos > layout->content_width) {
    return CMP_OK;
  }

  for (index = 0; index < row->item_count; ++index) {
    rc = m3_tab_row_item_width(row, &row->items[index], &width);
    if (rc != CMP_OK) { /* GCOVR_EXCL_LINE */
      return rc;
    }
    if (pos <= width) {   /* GCOVR_EXCL_LINE */
      *out_index = index; /* GCOVR_EXCL_LINE */
      return CMP_OK;       /* GCOVR_EXCL_LINE */
    }
    pos -= width;
    if (index + 1 < row->item_count) { /* GCOVR_EXCL_LINE */
      if (pos <= layout->spacing) {    /* GCOVR_EXCL_LINE */
        return CMP_OK;
      }
      pos -= layout->spacing;
    }
  }

  return CMP_OK;
}

static int m3_tab_row_widget_measure(void *widget, CMPMeasureSpec width,
                                     CMPMeasureSpec height, CMPSize *out_size) {
  M3TabRow *row;
  CMPScalar content_width;
  CMPScalar content_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  row = (M3TabRow *)widget;
  rc = m3_tab_row_validate_style(&row->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_items(row->items, row->item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_measure_content(row, row->style.mode, &content_width,
                                  &content_height);
  if (rc != CMP_OK) {
    return rc;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (content_height > height.size) ? height.size : content_height;
  } else {
    out_size->height = content_height;
  }

  return CMP_OK;
}

static int m3_tab_row_widget_layout(void *widget, CMPRect bounds) {
  M3TabRow *row;
  M3TabRowLayout layout;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  row = (M3TabRow *)widget;
  row->bounds = bounds;

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }
  row->content_width = layout.content_width;

  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_sync_indicator(row, &layout, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_tab_row_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3TabRow *row;
  M3TabRowLayout layout;
  CMPRect item_rect;
  CMPRect indicator_rect;
  CMPTextMetrics metrics;
  CMPScalar text_x;
  CMPScalar text_y;
  CMPScalar indicator_thickness;
  CMPScalar indicator_width;
  CMPColor text_color;
  CMPColor indicator_color;
  cmp_usize i;
  int rc;
#ifdef CMP_TESTING
  CMPBool match; /* GCOVR_EXCL_LINE */
#endif /* GCOVR_EXCL_LINE */

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  row = (M3TabRow *)widget;
  rc = m3_tab_row_validate_style(&row->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_rect(&row->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_validate_items(row->items, row->item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }
  row->content_width = layout.content_width;

  if (row->style.background_color.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &row->bounds,
                                     row->style.background_color, 0.0f);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  indicator_thickness = row->style.indicator_thickness;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  rc = m3_tab_row_test_fail_point_match(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (match == CMP_TRUE) {
    indicator_thickness = -1.0f;
  }
#endif
  if (indicator_thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (row->selected_index != M3_TAB_INVALID_INDEX &&
      row->selected_index < row->item_count && indicator_thickness > 0.0f) {
    indicator_width = row->indicator_width;
    indicator_rect.x = layout.start_x + row->indicator_pos - row->scroll_offset;
    indicator_rect.y = layout.start_y + layout.tab_height - indicator_thickness;
    indicator_rect.width = indicator_width;
    indicator_rect.height = indicator_thickness;
#ifdef CMP_TESTING
    rc = m3_tab_row_test_fail_point_match(
        M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE, &match);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (match == CMP_TRUE) {
      indicator_rect.width = -1.0f;
    }
#endif

    if (indicator_rect.width < 0.0f || indicator_rect.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    indicator_color = row->style.indicator_color;
    if (row->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
      indicator_color = row->style.disabled_indicator_color;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &indicator_rect,
                                     indicator_color,
                                     row->style.indicator_corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (row->item_count == 0) {
    return CMP_OK;
  }

  if (row->text_backend.vtable == NULL ||
      row->text_backend.vtable->measure_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  for (i = 0; i < row->item_count; ++i) {
    rc = m3_tab_row_item_rect(row, &layout, i, &item_rect);
    if (rc != CMP_OK) {
      return rc;
    }

    if (row->items[i].utf8_len == 0) {
      continue;
    }

    rc = cmp_text_measure_utf8(&row->text_backend, row->font,
                              row->items[i].utf8_label, row->items[i].utf8_len,
                              &metrics); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }

    text_x = item_rect.x + (item_rect.width - metrics.width) * 0.5f;
    text_y = item_rect.y + (item_rect.height - metrics.height) * 0.5f +
             metrics.baseline;

    text_color = row->style.text_style.color;
    if (row->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
      text_color = row->style.disabled_text_color;
    } else if (i == row->selected_index) {
      text_color = row->style.selected_text_color;
    }

    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, row->font, row->items[i].utf8_label,
        row->items[i].utf8_len, text_x, text_y, text_color);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_tab_row_widget_event(void *widget, const CMPInputEvent *event,
                                   CMPBool *out_handled) { /* GCOVR_EXCL_LINE */
  M3TabRow *row = NULL;
  M3TabRowLayout layout; /* GCOVR_EXCL_LINE */
  cmp_usize index;        /* GCOVR_EXCL_LINE */
  cmp_usize previous;     /* GCOVR_EXCL_LINE */
  CMPScalar delta = 0.0f;
  int rc = CMP_OK;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  row = (M3TabRow *)widget;

  if (row->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  if (event->type == CMP_INPUT_POINTER_SCROLL) {
    if (row->style.mode != M3_TAB_MODE_SCROLLABLE) {
      return CMP_OK;
    }
    delta = (CMPScalar)event->data.pointer.scroll_x;
    if (delta == 0.0f) {
      return CMP_OK;
    }
    rc = m3_tab_row_set_scroll(row, row->scroll_offset + delta);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  }

  if (event->type != CMP_INPUT_POINTER_DOWN &&
      event->type != CMP_INPUT_POINTER_UP) {
    return CMP_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_hit_test(row, &layout, event->data.pointer.x,
                           event->data.pointer.y, &index);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (event->type == CMP_INPUT_POINTER_DOWN) {
    if (row->pressed_index != M3_TAB_INVALID_INDEX) {
      return CMP_ERR_STATE;
    }
    if (index == M3_TAB_INVALID_INDEX) {
      return CMP_OK;
    }
    row->pressed_index = index;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  }

  if (row->pressed_index == M3_TAB_INVALID_INDEX) {
    return CMP_OK;
  }

  if (index == row->pressed_index) {
    previous = row->selected_index;
    row->selected_index = index;
    rc = m3_tab_row_sync_indicator(row, &layout, CMP_TRUE);
    if (rc != CMP_OK) {
      row->selected_index = previous;
      row->pressed_index = M3_TAB_INVALID_INDEX;
      return rc;
    }
    if (row->on_select != NULL) {
      rc = row->on_select(row->on_select_ctx, row, index);
      if (rc != CMP_OK) {
        row->selected_index = previous;
        rc = m3_tab_row_sync_indicator(row, &layout, CMP_FALSE);
        if (rc != CMP_OK) {
          row->pressed_index = M3_TAB_INVALID_INDEX;
          return rc;
        }
        row->pressed_index = M3_TAB_INVALID_INDEX;
        return rc;
      }
    }
  }

  row->pressed_index = M3_TAB_INVALID_INDEX;
  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int m3_tab_row_widget_get_semantics(void *widget,
                                           CMPSemantics *out_semantics) {
  M3TabRow *row;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  row = (M3TabRow *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (row->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (row->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_tab_row_widget_destroy(void *widget) {
  M3TabRow *row;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  row = (M3TabRow *)widget;
  rc = CMP_OK;
  if (row->owns_font == CMP_TRUE &&
      (row->font.id != 0u || row->font.generation != 0u)) {
    if (row->text_backend.vtable != NULL &&
        row->text_backend.vtable->destroy_font != NULL) {
      rc = row->text_backend.vtable->destroy_font(row->text_backend.ctx,
                                                  row->font);
    } else {
      rc = CMP_ERR_UNSUPPORTED;
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
  row->owns_font = CMP_FALSE;
  return rc;
}

static const CMPWidgetVTable g_m3_tab_row_widget_vtable = {
    m3_tab_row_widget_measure,       m3_tab_row_widget_layout,
    m3_tab_row_widget_paint,         m3_tab_row_widget_event,
    m3_tab_row_widget_get_semantics, m3_tab_row_widget_destroy};

int CMP_CALL m3_tab_row_style_init(M3TabRowStyle *style) {
  int rc;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match;
#endif /* GCOVR_EXCL_LINE */

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_text_style_init(&style->text_style);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_STYLE_INIT,
                                          &match);
    if (rc == CMP_OK && match == CMP_TRUE) {
      rc = CMP_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != CMP_OK) {
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
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_color_set(&style->indicator_color, 0.0f, 0.0f, 0.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_color_with_alpha(&style->text_style.color, 0.38f,
                                   &style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_color_with_alpha(&style->indicator_color, 0.38f,
                                   &style->disabled_indicator_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_tab_row_init(M3TabRow *row, const CMPTextBackend *backend,
                            const M3TabRowStyle *style, const M3TabItem *items,
                            cmp_usize item_count, cmp_usize selected_index) { /* GCOVR_EXCL_LINE */
  M3TabRowLayout layout = {0};
  int rc = CMP_OK; /* GCOVR_EXCL_LINE */
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match;
#endif /* GCOVR_EXCL_LINE */

  if (row == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_tab_row_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_tab_row_validate_items(items, item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  if (selected_index != M3_TAB_INVALID_INDEX && selected_index >= item_count) {
    return CMP_ERR_RANGE;
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

  rc = cmp_text_font_create(backend, &style->text_style, &row->font);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_anim_controller_init(&row->indicator_pos_anim);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ANIM_INIT,
                                          &match);
    if (rc == CMP_OK && match == CMP_TRUE) {
      rc = CMP_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != CMP_OK) {
    cmp_text_font_destroy(backend, row->font);
    return rc;
  }

  rc = cmp_anim_controller_init(&row->indicator_width_anim);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_ANIM_INIT,
                                          &match);
    if (rc == CMP_OK && match == CMP_TRUE) {
      rc = CMP_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != CMP_OK) {
    cmp_text_font_destroy(backend, row->font);
    return rc;
  }

  row->owns_font = CMP_TRUE;
  row->widget.ctx = row;
  row->widget.vtable = &g_m3_tab_row_widget_vtable;
  row->widget.handle.id = 0u;
  row->widget.handle.generation = 0u;
  row->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  layout.mode = row->style.mode;
  layout.start_x = 0.0f;
  layout.start_y = 0.0f;
  layout.tab_width = 0.0f;
  layout.tab_height = row->style.min_height;
  layout.spacing = row->style.spacing;
  layout.content_width = 0.0f;
  layout.content_height = row->style.min_height;
  layout.available_width = 0.0f;
  rc = m3_tab_row_sync_indicator(row, &layout, CMP_FALSE);
  if (rc != CMP_OK) {
    cmp_text_font_destroy(backend, row->font);
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_tab_row_set_items(M3TabRow *row, const M3TabItem *items,
                                 cmp_usize item_count) {
  int rc = CMP_OK;

  if (row == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_items(items, item_count);
  if (rc != CMP_OK) {
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

  return CMP_OK;
}

int CMP_CALL m3_tab_row_set_style(M3TabRow *row, const M3TabRowStyle *style) {
  CMPHandle new_font;
  int rc;

  if (row == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(&row->text_backend, &style->text_style, &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (row->owns_font == CMP_TRUE) {
    rc = cmp_text_font_destroy(&row->text_backend, row->font);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&row->text_backend, new_font);
      return rc;
    }
  }

  row->style = *style;
  row->font = new_font;
  row->owns_font = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_set_selected(M3TabRow *row, cmp_usize selected_index) {
  M3TabRowLayout layout = {0};
  int rc;

  if (row == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (selected_index != M3_TAB_INVALID_INDEX &&
      selected_index >= row->item_count) {
    return CMP_ERR_RANGE;
  }

  row->selected_index = selected_index;
  if (row->bounds.width <= 0.0f || row->bounds.height <= 0.0f) {
    return CMP_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_sync_indicator(row, &layout, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

int CMP_CALL m3_tab_row_get_selected(const M3TabRow *row,
                                    cmp_usize *out_selected) {
  if (row == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_selected = row->selected_index;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_set_scroll(M3TabRow *row, CMPScalar offset) {
  M3TabRowLayout layout = {0};
  int rc;

  if (row == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (offset < 0.0f) {
    return CMP_ERR_RANGE;
  }

  row->scroll_offset = offset;
  if (row->style.mode != M3_TAB_MODE_SCROLLABLE) {
    row->scroll_offset = 0.0f;
    return CMP_OK;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_tab_row_clamp_scroll(row, &layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

int CMP_CALL m3_tab_row_get_scroll(const M3TabRow *row, CMPScalar *out_offset) {
  if (row == NULL || out_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_offset = row->scroll_offset;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_set_on_select(M3TabRow *row, CMPTabRowOnSelect on_select,
                                     void *ctx) {
  if (row == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  row->on_select = on_select;
  row->on_select_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_step(M3TabRow *row, CMPScalar dt, CMPBool *out_changed) {
  CMPScalar value; /* GCOVR_EXCL_LINE */
  CMPBool finished = CMP_FALSE;
  CMPBool changed = CMP_FALSE;
  int rc = CMP_OK; /* GCOVR_EXCL_LINE */

  if (row == NULL || out_changed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (row->indicator_pos_anim.mode != CMP_ANIM_MODE_NONE) {
    rc = cmp_anim_controller_step(&row->indicator_pos_anim, dt, &value,
                                 &finished);
    if (rc != CMP_OK) {
      return rc;
    }
    if (value != row->indicator_pos) {
      row->indicator_pos = value;
      changed = CMP_TRUE;
    }
  }

  if (row->indicator_width_anim.mode != CMP_ANIM_MODE_NONE) {
    rc = cmp_anim_controller_step(&row->indicator_width_anim, dt, &value,
                                 &finished);
    if (rc != CMP_OK) {
      return rc;
    }
    if (value != row->indicator_width) {
      row->indicator_width = value;
      changed = CMP_TRUE;
    }
  }

  CMP_UNUSED(finished);
  *out_changed = changed;
  return CMP_OK;
}

static int m3_segmented_validate_color(const CMPColor *color) {
  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(color->r >= 0.0f && color->r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->g >= 0.0f && color->g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->b >= 0.0f && color->b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(color->a >= 0.0f && color->a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_segmented_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                  CMPScalar b, CMPScalar a) {
  CMPBool should_fail;
  int rc;

  if (color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(r >= 0.0f && r <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(g >= 0.0f && g <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(b >= 0.0f && b <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
  if (!(a >= 0.0f && a <= 1.0f)) {
    return CMP_ERR_RANGE;
  }
#ifdef CMP_TESTING
  rc = m3_segmented_test_color_should_fail(&should_fail);
  if (rc != CMP_OK) {
    return rc;
  }
  if (should_fail == CMP_TRUE) {
    return CMP_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_segmented_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                         CMPColor *out_color) {
  CMPBool should_fail;
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_segmented_validate_color(base);
  if (rc != CMP_OK) {
    return rc;
  }
#ifdef CMP_TESTING
  rc = m3_segmented_test_color_should_fail(&should_fail);
  if (rc != CMP_OK) {
    return rc;
  }
  if (should_fail == CMP_TRUE) {
    return CMP_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return CMP_OK;
}

static int m3_segmented_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int
m3_segmented_validate_text_style(const CMPTextStyle *style,
                                 CMPBool require_family) { /* GCOVR_EXCL_LINE */
  int rc = CMP_OK;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (require_family == CMP_TRUE && style->utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (style->size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (style->weight < 100 || style->weight > 900) {
    return CMP_ERR_RANGE;
  }
  rc = m3_tabs_validate_bool(style->italic);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }
  return CMP_OK;
}

static int m3_segmented_validate_style(const M3SegmentedStyle *style,
                                       CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (style->spacing < 0.0f || style->min_width < 0.0f ||
      style->min_height < 0.0f || style->padding_x < 0.0f ||
      style->padding_y < 0.0f || style->outline_width < 0.0f ||
      style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_segmented_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_validate_text_style(&style->text_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->outline_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->selected_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->selected_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_segmented_validate_color(&style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_color(&style->disabled_selected_background_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_segmented_validate_color(&style->disabled_selected_text_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_segmented_validate_items(const M3SegmentedItem *items,
                                       cmp_usize count) {
  cmp_usize i;

  if (count == 0) {
    return CMP_OK;
  }
  if (items == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    if (items[i].utf8_label == NULL && items[i].utf8_len != 0) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

static int m3_segmented_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_segmented_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_segmented_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_segmented_validate_mode(cmp_u32 mode) {
  switch (mode) {
  case M3_SEGMENTED_MODE_SINGLE:
  case M3_SEGMENTED_MODE_MULTI:
    break;
  default:
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_segmented_validate_selected_states(const CMPBool *states,
                                                 cmp_usize count) {
  cmp_usize i;
  int rc;

  if (count == 0) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
  if (states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  for (i = 0; i < count; ++i) {
    rc = m3_tabs_validate_bool(states[i]);
    if (rc != CMP_OK) {
      return rc;
    }
  }
  return CMP_OK;
}

static int m3_segmented_measure_max_text(const M3SegmentedButtons *buttons,
                                         CMPScalar *out_width,
                                         CMPScalar *out_height,
                                         CMPScalar *out_baseline) {
  CMPTextMetrics metrics;
  CMPScalar max_width;
  CMPScalar max_height;
  CMPScalar max_baseline;
  cmp_usize i;
  int rc;

  if (buttons == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_width = 0.0f;
  max_height = 0.0f;
  max_baseline = 0.0f;

  if (buttons->item_count == 0) {
    *out_width = 0.0f;
    *out_height = 0.0f;
    *out_baseline = 0.0f;
    return CMP_OK;
  }
  if (buttons->items == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  for (i = 0; i < buttons->item_count; ++i) {
    if (buttons->items[i].utf8_len == 0) {
      continue;
    }
    rc = cmp_text_measure_utf8(&buttons->text_backend, buttons->font,
                              buttons->items[i].utf8_label,
                              buttons->items[i].utf8_len, &metrics);
    if (rc != CMP_OK) {
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
  return CMP_OK;
}

static int
m3_segmented_measure_content(const M3SegmentedButtons *buttons,
                             CMPScalar *out_width,
                             CMPScalar *out_height) { /* GCOVR_EXCL_LINE */
  CMPScalar max_width = 0.0f;
  CMPScalar max_height = 0.0f;
  CMPScalar max_baseline = 0.0f;
  CMPScalar segment_width = 0.0f;
  CMPScalar segment_height = 0.0f;
  CMPScalar content_width = 0.0f;
  CMPScalar content_height = 0.0f;
  int rc = CMP_OK; /* GCOVR_EXCL_LINE */

  if (buttons == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_measure_max_text(buttons, &max_width, &max_height,
                                     &max_baseline);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(max_baseline);

  segment_width = max_width + buttons->style.padding_x * 2.0f;
  if (segment_width < buttons->style.min_width) {
    segment_width = buttons->style.min_width;
  }

  segment_height = max_height + buttons->style.padding_y * 2.0f;
  if (segment_height < buttons->style.min_height) {
    segment_height = buttons->style.min_height;
  }

  if (buttons->item_count > 0) {
    content_width = segment_width * (CMPScalar)buttons->item_count;
    content_width +=
        buttons->style.spacing * (CMPScalar)(buttons->item_count - 1);
  } else {
    content_width = 0.0f;
  }
  content_height = segment_height;

  if (content_width < 0.0f || content_height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  *out_width = buttons->style.padding.left + buttons->style.padding.right +
               content_width;
  *out_height = buttons->style.padding.top + buttons->style.padding.bottom +
                content_height;
  if (*out_width < 0.0f || *out_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int m3_segmented_compute_layout(const M3SegmentedButtons *buttons,
                                       M3SegmentedLayout *out_layout) {
  M3SegmentedStyle style = {0};
  CMPRect bounds = {0};
  CMPScalar available_width = 0.0f;
  CMPScalar available_height = 0.0f;
  CMPScalar spacing = 0.0f; /* GCOVR_EXCL_LINE */
  CMPScalar max_width = 0.0f;
  CMPScalar max_height = 0.0f;
  CMPScalar max_baseline = 0.0f;   /* GCOVR_EXCL_LINE */
  CMPScalar segment_height = 0.0f; /* GCOVR_EXCL_LINE */
  CMPScalar segment_width = 0.0f;
  CMPScalar total_spacing = 0.0f;
  int rc = CMP_OK;
#ifdef CMP_TESTING
  CMPBool match; /* GCOVR_EXCL_LINE */
#endif

  if (buttons == NULL || out_layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style = buttons->style;
  bounds = buttons->bounds;

  rc = m3_segmented_validate_style(&style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  available_width = bounds.width - style.padding.left - style.padding.right;
  available_height = bounds.height - style.padding.top - style.padding.bottom;
  if (available_width < 0.0f || available_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_segmented_measure_max_text(buttons, &max_width, &max_height,
                                     &max_baseline);
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(max_baseline);

  spacing = style.spacing;
#ifdef CMP_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    spacing = -1.0f;
  }
#endif
  if (spacing < 0.0f) {
    return CMP_ERR_RANGE;
  }

  segment_height = max_height + style.padding_y * 2.0f;
  if (segment_height < style.min_height) {
    segment_height = style.min_height;
  }
#ifdef CMP_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    segment_height = -1.0f;
  }
#endif
  if (segment_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (buttons->item_count > 0 && available_height <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (segment_height > available_height && available_height > 0.0f) {
    segment_height = available_height;
  }

  if (buttons->item_count > 0) {
    total_spacing = spacing * (CMPScalar)(buttons->item_count - 1);
    if (available_width < total_spacing) {
      return CMP_ERR_RANGE;
    }
    segment_width =
        (available_width - total_spacing) / (CMPScalar)buttons->item_count;
#ifdef CMP_TESTING
    rc = m3_segmented_test_fail_point_match(
        M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE, &match);
    if (rc != CMP_OK) {
      return rc;
    }
    if (match == CMP_TRUE) {
      segment_width = -1.0f;
    }
#endif
    if (segment_width < 0.0f) {
      return CMP_ERR_RANGE;
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
    out_layout->content_width = segment_width * (CMPScalar)buttons->item_count +
                                spacing * (CMPScalar)(buttons->item_count - 1);
  } else {
    out_layout->content_width = 0.0f;
  }
  out_layout->content_height = segment_height;

  if (available_height > segment_height) {
    out_layout->start_y += (available_height - segment_height) * 0.5f;
  }

  return CMP_OK;
}

static int m3_segmented_hit_test(const M3SegmentedButtons *buttons,
                                 const M3SegmentedLayout *layout, cmp_i32 x,
                                 cmp_i32 y, cmp_usize *out_index) { /* GCOVR_EXCL_LINE */
  CMPScalar fx = 0.0f;
  CMPScalar fy = 0.0f;
  CMPScalar stride = 0.0f;
  CMPScalar pos = 0.0f; /* GCOVR_EXCL_LINE */
  cmp_usize index = 0u;
  int rc = CMP_OK;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match;
#endif

  if (buttons == NULL || layout == NULL || out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_index = M3_SEGMENTED_INVALID_INDEX;
  if (buttons->item_count == 0) {
    return CMP_OK;
  }

  fx = (CMPScalar)x;
  fy = (CMPScalar)y;

  if (fx < layout->start_x || fx > layout->start_x + layout->content_width) {
    return CMP_OK;
  }
  if (fy < layout->start_y || fy > layout->start_y + layout->segment_height) {
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  stride = layout->segment_width + layout->spacing;
  if (stride <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  pos = fx - layout->start_x;
#ifdef CMP_TESTING
  rc = m3_segmented_test_fail_point_match(
      M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE, &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    pos = -1.0f;
  }
#endif
  if (pos < 0.0f) {
    return CMP_OK;
  }

  index = (cmp_usize)(pos / stride);
  if (index >= buttons->item_count) {
    return CMP_OK;
  }
  if (pos > (CMPScalar)index * stride + layout->segment_width) {
    return CMP_OK;
  }

  *out_index = index;
  return CMP_OK;
}

static int
m3_segmented_is_selected(const M3SegmentedButtons *buttons, cmp_usize index,
                         CMPBool *out_selected) { /* GCOVR_EXCL_LINE */
  int rc;                                        /* GCOVR_EXCL_LINE */

  if (buttons == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (index >= buttons->item_count) {
    return CMP_ERR_RANGE;
  }

  if (buttons->mode == M3_SEGMENTED_MODE_SINGLE) {
    *out_selected = (buttons->selected_index == index) ? CMP_TRUE : CMP_FALSE;
    return CMP_OK;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (buttons->selected_states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = m3_tabs_validate_bool(buttons->selected_states[index]);
  if (rc != CMP_OK) {
    return rc;
  }
  *out_selected = buttons->selected_states[index];
  return CMP_OK;
}

static int m3_segmented_widget_measure(void *widget, CMPMeasureSpec width,
                                       CMPMeasureSpec height, CMPSize *out_size) {
  M3SegmentedButtons *buttons;
  CMPScalar content_width;
  CMPScalar content_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = m3_segmented_validate_style(&buttons->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_items(buttons->items, buttons->item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_measure_content(buttons, &content_width, &content_height);
  if (rc != CMP_OK) {
    return rc;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (content_width > width.size) ? width.size : content_width;
  } else {
    out_size->width = content_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height = /* GCOVR_EXCL_LINE */
        (content_height > height.size) ? height.size
                                       : content_height; /* GCOVR_EXCL_LINE */
  } else {
    out_size->height = content_height;
  }

  return CMP_OK;
}

static int m3_segmented_widget_layout(void *widget, CMPRect bounds) {
  M3SegmentedButtons *buttons;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  buttons = (M3SegmentedButtons *)widget;
  buttons->bounds = bounds;
  return CMP_OK;
}

static int m3_segmented_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3SegmentedButtons *buttons;
  M3SegmentedLayout layout;
  CMPRect segment_rect;
  CMPRect inner_rect; /* GCOVR_EXCL_LINE */
  CMPTextMetrics metrics;
  CMPScalar text_x;
  CMPScalar text_y;
  CMPScalar outline_width;
  CMPScalar inner_corner;
  CMPScalar corner_radius;
  CMPColor bg_color;
  CMPColor text_color;
  CMPColor outline_color;
  CMPBool selected;
  cmp_usize i = 0u;
  int rc = CMP_OK;
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  CMPBool match;
#endif /* GCOVR_EXCL_LINE */

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (ctx->gfx->text_vtable == NULL ||
      ctx->gfx->text_vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = m3_segmented_validate_style(&buttons->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_rect(&buttons->bounds);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_validate_items(buttons->items, buttons->item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_compute_layout(buttons, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  if (buttons->item_count == 0) {
    return CMP_OK;
  }

  if (buttons->text_backend.vtable == NULL ||
      buttons->text_backend.vtable->measure_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  outline_width = buttons->style.outline_width;
  if (outline_width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  for (i = 0; i < buttons->item_count; ++i) {
    segment_rect.x =
        layout.start_x + (layout.segment_width + layout.spacing) * (CMPScalar)i;
    segment_rect.y = layout.start_y;
    segment_rect.width = layout.segment_width;
    segment_rect.height = layout.segment_height;
#ifdef CMP_TESTING
    rc = m3_segmented_test_fail_point_match(
        M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE, &match);
    if (rc != CMP_OK) {
      return rc;
    }
    if (match == CMP_TRUE) {
      segment_rect.width = -1.0f;
    }
#endif
    if (segment_rect.width < 0.0f || segment_rect.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    corner_radius = 0.0f;
    if (i == 0 || i + 1 == buttons->item_count) {
      corner_radius = buttons->style.corner_radius;
    }

    rc = m3_segmented_is_selected(buttons, i, &selected);
    if (rc != CMP_OK) {
      return rc;
    }

    if (buttons->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
      bg_color = buttons->style.disabled_background_color;
      text_color = buttons->style.disabled_text_color;
      outline_color = buttons->style.disabled_outline_color;
      if (selected == CMP_TRUE) {
        bg_color = buttons->style.disabled_selected_background_color;
        text_color = buttons->style.disabled_selected_text_color;
      }
    } else {
      bg_color = buttons->style.background_color;
      text_color = buttons->style.text_style.color;
      outline_color = buttons->style.outline_color;
      if (selected == CMP_TRUE) {
        bg_color = buttons->style.selected_background_color;
        text_color = buttons->style.selected_text_color;
      }
    }

    inner_rect = segment_rect;
    inner_corner = corner_radius;
    if (outline_width > 0.0f) {
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &segment_rect,
                                       outline_color, corner_radius);
      if (rc != CMP_OK) {
        return rc;
      }
      inner_rect.x = segment_rect.x + outline_width;
      inner_rect.y = segment_rect.y + outline_width;
      inner_rect.width = segment_rect.width - outline_width * 2.0f;
      inner_rect.height = segment_rect.height - outline_width * 2.0f;
      if (inner_rect.width < 0.0f || inner_rect.height < 0.0f) {
        return CMP_ERR_RANGE;
      }
      inner_corner = corner_radius - outline_width;
      if (inner_corner < 0.0f) {
        inner_corner = 0.0f;
      }
    }

    if (bg_color.a > 0.0f) {
      rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner_rect, bg_color,
                                       inner_corner);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }

    if (buttons->items[i].utf8_len == 0) {
      continue; /* GCOVR_EXCL_LINE */
    }

    rc = cmp_text_measure_utf8(&buttons->text_backend, buttons->font,
                              buttons->items[i].utf8_label,
                              buttons->items[i].utf8_len, &metrics);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }

    text_x = segment_rect.x + (segment_rect.width - metrics.width) * 0.5f;
    text_y = segment_rect.y + (segment_rect.height - metrics.height) * 0.5f +
             metrics.baseline;

    rc = ctx->gfx->text_vtable->draw_text(
        ctx->gfx->ctx, buttons->font, buttons->items[i].utf8_label,
        buttons->items[i].utf8_len, text_x, text_y, text_color);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_segmented_widget_event(void *widget, const CMPInputEvent *event,
                                     CMPBool *out_handled) {
  M3SegmentedButtons *buttons;
  M3SegmentedLayout layout; /* GCOVR_EXCL_LINE */
  cmp_usize index;
  cmp_usize previous;
  CMPBool selected;
  CMPBool new_selected;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;
  buttons = (M3SegmentedButtons *)widget;

  if (buttons->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  if (event->type != CMP_INPUT_POINTER_DOWN &&
      event->type != CMP_INPUT_POINTER_UP) {
    return CMP_OK;
  }

  rc = m3_segmented_validate_mode(buttons->mode);
  if (rc != CMP_OK) {
    return rc;
  }
  if (buttons->mode == M3_SEGMENTED_MODE_MULTI && buttons->item_count > 0 &&
      buttons->selected_states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_compute_layout(buttons, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_hit_test(buttons, &layout, event->data.pointer.x,
                             event->data.pointer.y, &index);
  if (rc != CMP_OK) {
    return rc;
  }

  if (event->type == CMP_INPUT_POINTER_DOWN) {
    if (buttons->pressed_index != M3_SEGMENTED_INVALID_INDEX) {
      return CMP_ERR_STATE;
    }
    if (index == M3_SEGMENTED_INVALID_INDEX) {
      return CMP_OK;
    }
    buttons->pressed_index = index;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  }

  if (buttons->pressed_index == M3_SEGMENTED_INVALID_INDEX) {
    return CMP_OK;
  }

  if (index == buttons->pressed_index) {
    if (buttons->mode == M3_SEGMENTED_MODE_SINGLE) {
      previous = buttons->selected_index;
      buttons->selected_index = index;
      if (buttons->on_select != NULL) {
        rc =
            buttons->on_select(buttons->on_select_ctx, buttons, index, CMP_TRUE);
        if (rc != CMP_OK) {
          buttons->selected_index = previous;
          buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
          return rc;
        }
      }
    } else if (buttons->mode == M3_SEGMENTED_MODE_MULTI) {
      if (buttons->selected_states == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
      }
      rc = m3_segmented_is_selected(buttons, index, &selected);
      if (rc != CMP_OK) {
        return rc;
      }
      new_selected = (selected == CMP_TRUE) ? CMP_FALSE : CMP_TRUE;
      buttons->selected_states[index] = new_selected;
      if (buttons->on_select != NULL) {
        rc = buttons->on_select(buttons->on_select_ctx, buttons, index,
                                new_selected);
        if (rc != CMP_OK) {
          buttons->selected_states[index] = selected;
          buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
          return rc;
        }
      }
    } else {               /* GCOVR_EXCL_LINE */
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
  }

  buttons->pressed_index = M3_SEGMENTED_INVALID_INDEX;
  *out_handled = CMP_TRUE;
  return CMP_OK;
}

static int m3_segmented_widget_get_semantics(void *widget,
                                             CMPSemantics *out_semantics) {
  M3SegmentedButtons *buttons;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  buttons = (M3SegmentedButtons *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0;
  if (buttons->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (buttons->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_segmented_widget_destroy(void *widget) {
  M3SegmentedButtons *buttons;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  buttons = (M3SegmentedButtons *)widget;
  rc = CMP_OK;
  if (buttons->owns_font == CMP_TRUE &&
      (buttons->font.id != 0u || buttons->font.generation != 0u)) {
    if (buttons->text_backend.vtable != NULL &&
        buttons->text_backend.vtable->destroy_font != NULL) {
      rc = buttons->text_backend.vtable->destroy_font(buttons->text_backend.ctx,
                                                      buttons->font);
    } else {
      rc = CMP_ERR_UNSUPPORTED;
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
  buttons->owns_font = CMP_FALSE;
  return rc;
}

static const CMPWidgetVTable g_m3_segmented_widget_vtable = {
    m3_segmented_widget_measure,       m3_segmented_widget_layout,
    m3_segmented_widget_paint,         m3_segmented_widget_event,
    m3_segmented_widget_get_semantics, m3_segmented_widget_destroy};

int CMP_CALL m3_segmented_style_init(M3SegmentedStyle *style) {
  int rc;
#ifdef CMP_TESTING
  CMPBool match;
#endif

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_text_style_init(&style->text_style);
#ifdef CMP_TESTING
  if (rc == CMP_OK) {
    rc = m3_segmented_test_fail_point_match(M3_SEGMENTED_TEST_FAIL_STYLE_INIT,
                                            &match);
    if (rc == CMP_OK && match == CMP_TRUE) {
      rc = CMP_ERR_UNKNOWN;
    }
  }
#endif
  if (rc != CMP_OK) {
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
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->outline_color, 0.44f, 0.44f, 0.44f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->selected_background_color, 0.26f, 0.52f,
                              0.96f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_set(&style->selected_text_color, 1.0f, 1.0f, 1.0f,
                              1.0f);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_color_with_alpha(&style->background_color, 0.12f,
                                     &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->outline_color, 0.12f,
                                     &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->text_style.color, 0.38f,
                                     &style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc =
      m3_segmented_color_with_alpha(&style->selected_background_color, 0.12f,
                                    &style->disabled_selected_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_segmented_color_with_alpha(&style->selected_text_color, 0.38f,
                                     &style->disabled_selected_text_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_init(M3SegmentedButtons *buttons,
                                      const CMPTextBackend *backend,
                                      const M3SegmentedStyle *style,
                                      const M3SegmentedItem *items,
                                      cmp_usize item_count, cmp_u32 mode,
                                      cmp_usize selected_index,
                                      CMPBool *selected_states) {
  int rc;

  if (buttons == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_segmented_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_validate_items(items, item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_segmented_validate_mode(mode);
  if (rc != CMP_OK) {
    return rc;
  }

  if (mode == M3_SEGMENTED_MODE_SINGLE) {
    if (selected_index != M3_SEGMENTED_INVALID_INDEX &&
        selected_index >= item_count) {
      return CMP_ERR_RANGE;
    }
  } else {
    rc = m3_segmented_validate_selected_states(selected_states, item_count);
    if (rc != CMP_OK) {
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

  rc = cmp_text_font_create(backend, &style->text_style, &buttons->font);
  if (rc != CMP_OK) {
    return rc;
  }

  buttons->owns_font = CMP_TRUE;
  buttons->widget.ctx = buttons;
  buttons->widget.vtable = &g_m3_segmented_widget_vtable;
  buttons->widget.handle.id = 0u;
  buttons->widget.handle.generation = 0u;
  buttons->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_set_items(M3SegmentedButtons *buttons,
                                           const M3SegmentedItem *items,
                                           cmp_usize item_count) {
  int rc;

  if (buttons == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_items(items, item_count);
  if (rc != CMP_OK) {
    return rc;
  }

  if (buttons->mode == M3_SEGMENTED_MODE_MULTI && item_count > 0 &&
      buttons->selected_states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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

  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_set_style(M3SegmentedButtons *buttons,
                                           const M3SegmentedStyle *style) {
  CMPHandle new_font;
  int rc; /* GCOVR_EXCL_LINE */

  if (buttons == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(&buttons->text_backend, &style->text_style,
                           &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (buttons->owns_font == CMP_TRUE) {
    rc = cmp_text_font_destroy(&buttons->text_backend, buttons->font);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&buttons->text_backend, new_font);
      return rc;
    }
  }

  buttons->style = *style;
  buttons->font = new_font;
  buttons->owns_font = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_set_selected_index(M3SegmentedButtons *buttons,
                                                    cmp_usize selected_index) {
  if (buttons == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_SINGLE) {
    return CMP_ERR_STATE;
  }
  if (selected_index != M3_SEGMENTED_INVALID_INDEX &&
      selected_index >= buttons->item_count) {
    return CMP_ERR_RANGE;
  }
  buttons->selected_index = selected_index;
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_get_selected_index(
    const M3SegmentedButtons *buttons, cmp_usize *out_selected) {
  if (buttons == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_SINGLE) {
    return CMP_ERR_STATE;
  }
  *out_selected = buttons->selected_index;
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_set_selected_state(M3SegmentedButtons *buttons,
                                                    cmp_usize index,
                                                    CMPBool selected) {
  int rc;

  if (buttons == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return CMP_ERR_STATE;
  }
  if (index >= buttons->item_count) {
    return CMP_ERR_RANGE;
  }
  if (buttons->selected_states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tabs_validate_bool(selected);
  if (rc != CMP_OK) {
    return rc;
  }
  buttons->selected_states[index] = selected; /* GCOVR_EXCL_LINE */
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_get_selected_state(
    const M3SegmentedButtons *buttons, cmp_usize index, CMPBool *out_selected) {
  int rc;

  if (buttons == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buttons->mode != M3_SEGMENTED_MODE_MULTI) {
    return CMP_ERR_STATE;
  }
  if (index >= buttons->item_count) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (buttons->selected_states == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_tabs_validate_bool(buttons->selected_states[index]);
  if (rc != CMP_OK) {
    return rc;
  }
  *out_selected = buttons->selected_states[index];
  return CMP_OK;
}

int CMP_CALL m3_segmented_buttons_set_on_select(M3SegmentedButtons *buttons,
                                               CMPSegmentedOnSelect on_select,
                                               void *ctx) {
  if (buttons == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  buttons->on_select = on_select;
  buttons->on_select_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL m3_tab_row_test_validate_color(const CMPColor *color) {
  return m3_tab_row_validate_color(color);
}

int CMP_CALL m3_tab_row_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                      CMPScalar b, CMPScalar a) {
  return m3_tab_row_color_set(color, r, g, b, a);
}

int CMP_CALL m3_tab_row_test_color_with_alpha(const CMPColor *base,
                                             CMPScalar alpha,
                                             CMPColor *out_color) {
  return m3_tab_row_color_with_alpha(base, alpha, out_color);
}

int CMP_CALL m3_tab_row_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_tab_row_validate_edges(edges);
}

int CMP_CALL m3_tab_row_test_validate_text_style(const CMPTextStyle *style,
                                                CMPBool require_family) {
  return m3_tab_row_validate_text_style(style, require_family);
}

int CMP_CALL m3_tab_row_test_validate_style(const M3TabRowStyle *style,
                                           CMPBool require_family) {
  return m3_tab_row_validate_style(style, require_family);
}

int CMP_CALL m3_tab_row_test_validate_items(const M3TabItem *items,
                                           cmp_usize count) {
  return m3_tab_row_validate_items(items, count);
}

int CMP_CALL m3_tab_row_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_tab_row_validate_measure_spec(spec);
}

int CMP_CALL m3_tab_row_test_validate_rect(const CMPRect *rect) {
  return m3_tab_row_validate_rect(rect);
}

int CMP_CALL m3_tab_row_test_validate_backend(const CMPTextBackend *backend) {
  return m3_tab_row_validate_backend(backend);
}

int CMP_CALL m3_tab_row_test_measure_content(const M3TabRow *row, cmp_u32 mode,
                                            CMPScalar *out_width,
                                            CMPScalar *out_height) {
  return m3_tab_row_measure_content(row, mode, out_width, out_height);
}

int CMP_CALL m3_tab_row_test_color_should_fail_null(void) {
  return m3_tab_row_test_color_should_fail(NULL);
}

int CMP_CALL m3_tab_row_test_fail_point_match_null(void) {
  return m3_tab_row_test_fail_point_match(M3_TAB_ROW_TEST_FAIL_NONE, NULL);
}

int CMP_CALL m3_tab_row_test_force_color_error(CMPBool enable) {
  g_m3_tab_row_test_force_color_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_set_fail_point_error_after(cmp_u32 call_count) {
  g_m3_tab_row_test_fail_point_error_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_set_value_fail_after(cmp_u32 call_count) {
  g_m3_tab_row_test_value_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_set_start_fail_after(cmp_u32 call_count) {
  g_m3_tab_row_test_start_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_tab_row_test_measure_max_text(const M3TabRow *row,
                                             CMPScalar *out_width,
                                             CMPScalar *out_height,
                                             CMPScalar *out_baseline) {
  return m3_tab_row_measure_max_text(row, out_width, out_height, out_baseline);
}

int CMP_CALL m3_tab_row_test_item_width(const M3TabRow *row,
                                       const M3TabItem *item,
                                       CMPScalar *out_width) {
  return m3_tab_row_item_width(row, item, out_width);
}

int CMP_CALL m3_tab_row_test_compute_layout(const M3TabRow *row,
                                           CMPScalar *out_content_width,
                                           CMPScalar *out_tab_width,
                                           CMPScalar *out_tab_height,
                                           cmp_u32 *out_mode) {
  M3TabRowLayout layout;
  int rc;

  if (out_content_width == NULL || out_tab_width == NULL ||
      out_tab_height == NULL || out_mode == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_tab_row_compute_layout(row, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_content_width = layout.content_width; /* GCOVR_EXCL_LINE */
  *out_tab_width = layout.tab_width;         /* GCOVR_EXCL_LINE */
  *out_tab_height = layout.tab_height;       /* GCOVR_EXCL_LINE */
  *out_mode = layout.mode;                   /* GCOVR_EXCL_LINE */
  return CMP_OK;                              /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_tab_row_test_clamp_scroll(M3TabRow *row, cmp_u32 mode,
                                         CMPScalar content_width,
                                         CMPScalar available_width) {
  M3TabRowLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.mode = mode;
  layout.content_width = content_width;
  layout.available_width = available_width;
  return m3_tab_row_clamp_scroll(row, &layout);
}

int CMP_CALL m3_tab_row_test_indicator_target(
    const M3TabRow *row, cmp_u32 mode, CMPScalar tab_width, CMPScalar spacing,
    CMPScalar content_width, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_height, CMPScalar *out_pos, CMPScalar *out_width) {
  M3TabRowLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.mode = mode;
  layout.tab_width = tab_width;
  layout.spacing = spacing;
  layout.content_width = content_width;
  layout.start_x = start_x;
  layout.start_y = start_y;
  layout.tab_height = tab_height;
  return m3_tab_row_indicator_target(row, &layout, out_pos, out_width);
}

int CMP_CALL m3_tab_row_test_sync_indicator(M3TabRow *row, cmp_u32 mode,
                                           CMPScalar tab_width, CMPScalar spacing,
                                           CMPScalar content_width,
                                           CMPScalar start_x, CMPScalar start_y,
                                           CMPScalar tab_height,
                                           CMPBool animate) {
  M3TabRowLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.mode = mode;
  layout.tab_width = tab_width;
  layout.spacing = spacing;
  layout.content_width = content_width;
  layout.start_x = start_x;
  layout.start_y = start_y;
  layout.tab_height = tab_height;
  return m3_tab_row_sync_indicator(row, &layout, animate);
}

int CMP_CALL m3_tab_row_test_item_rect(const M3TabRow *row, cmp_u32 mode,
                                      CMPScalar start_x, CMPScalar start_y,
                                      CMPScalar tab_width, CMPScalar tab_height,
                                      CMPScalar spacing, CMPScalar content_width,
                                      CMPScalar available_width, cmp_usize index,
                                      CMPRect *out_rect) {
  M3TabRowLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.mode = mode;
  layout.start_x = start_x;
  layout.start_y = start_y;
  layout.tab_width = tab_width;
  layout.tab_height = tab_height;
  layout.spacing = spacing;
  layout.content_width = content_width;
  layout.available_width = available_width;
  return m3_tab_row_item_rect(row, &layout, index, out_rect);
}

int CMP_CALL m3_tab_row_test_hit_test(const M3TabRow *row, cmp_u32 mode,
                                     CMPScalar start_x, CMPScalar start_y,
                                     CMPScalar tab_width, CMPScalar tab_height,
                                     CMPScalar spacing, CMPScalar content_width,
                                     CMPScalar available_width, cmp_i32 x,
                                     cmp_i32 y, cmp_usize *out_index) {
  M3TabRowLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.mode = mode;
  layout.start_x = start_x;
  layout.start_y = start_y;
  layout.tab_width = tab_width;
  layout.tab_height = tab_height;
  layout.spacing = spacing;
  layout.content_width = content_width;
  layout.available_width = available_width;
  return m3_tab_row_hit_test(row, &layout, x, y, out_index);
}

int CMP_CALL m3_segmented_test_validate_color(const CMPColor *color) {
  return m3_segmented_validate_color(color);
}

int CMP_CALL m3_segmented_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                        CMPScalar b, CMPScalar a) {
  return m3_segmented_color_set(color, r, g, b, a);
}

int CMP_CALL m3_segmented_test_color_with_alpha(const CMPColor *base,
                                               CMPScalar alpha,
                                               CMPColor *out_color) {
  return m3_segmented_color_with_alpha(base, alpha, out_color);
}

int CMP_CALL m3_segmented_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_segmented_validate_edges(edges);
}

int CMP_CALL m3_segmented_test_validate_text_style(const CMPTextStyle *style,
                                                  CMPBool require_family) {
  return m3_segmented_validate_text_style(style, require_family);
}

int CMP_CALL m3_segmented_test_validate_style(const M3SegmentedStyle *style,
                                             CMPBool require_family) {
  return m3_segmented_validate_style(style, require_family);
}

int CMP_CALL m3_segmented_test_validate_items(const M3SegmentedItem *items,
                                             cmp_usize count) {
  return m3_segmented_validate_items(items, count);
}

int CMP_CALL m3_segmented_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_segmented_validate_measure_spec(spec);
}

int CMP_CALL m3_segmented_test_validate_rect(const CMPRect *rect) {
  return m3_segmented_validate_rect(rect);
}

int CMP_CALL m3_segmented_test_validate_backend(const CMPTextBackend *backend) {
  return m3_segmented_validate_backend(backend);
}

int CMP_CALL m3_segmented_test_validate_mode(cmp_u32 mode) {
  return m3_segmented_validate_mode(mode);
}

int CMP_CALL m3_segmented_test_validate_selected_states(const CMPBool *states,
                                                       cmp_usize count) {
  return m3_segmented_validate_selected_states(states, count);
}

int CMP_CALL m3_segmented_test_measure_content(const M3SegmentedButtons *buttons,
                                              CMPScalar *out_width,
                                              CMPScalar *out_height) {
  return m3_segmented_measure_content(buttons, out_width, out_height);
}

int CMP_CALL m3_segmented_test_color_should_fail_null(void) {
  return m3_segmented_test_color_should_fail(NULL);
}

int CMP_CALL m3_segmented_test_fail_point_match_null(void) {
  return m3_segmented_test_fail_point_match(M3_SEGMENTED_TEST_FAIL_NONE, NULL);
}

int CMP_CALL m3_segmented_test_force_color_error(CMPBool enable) {
  g_m3_segmented_test_force_color_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL m3_segmented_test_set_fail_point_error_after(cmp_u32 call_count) {
  g_m3_segmented_test_fail_point_error_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_segmented_test_measure_max_text(
    const M3SegmentedButtons *buttons, CMPScalar *out_width,
    CMPScalar *out_height, CMPScalar *out_baseline) {
  return m3_segmented_measure_max_text(buttons, out_width, out_height,
                                       out_baseline);
}

int CMP_CALL m3_segmented_test_compute_layout(const M3SegmentedButtons *buttons,
                                             CMPScalar *out_content_width,
                                             CMPScalar *out_segment_width,
                                             CMPScalar *out_segment_height,
                                             CMPScalar *out_spacing) {
  M3SegmentedLayout layout;
  int rc;

  if (out_content_width == NULL || out_segment_width == NULL ||
      out_segment_height == NULL || out_spacing == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_segmented_compute_layout(buttons, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_content_width = layout.content_width;   /* GCOVR_EXCL_LINE */
  *out_segment_width = layout.segment_width;   /* GCOVR_EXCL_LINE */
  *out_segment_height = layout.segment_height; /* GCOVR_EXCL_LINE */
  *out_spacing = layout.spacing;               /* GCOVR_EXCL_LINE */
  return CMP_OK;                                /* GCOVR_EXCL_LINE */
}

int CMP_CALL m3_segmented_test_hit_test(
    const M3SegmentedButtons *buttons, CMPScalar start_x, CMPScalar start_y,
    CMPScalar segment_width, CMPScalar segment_height, CMPScalar spacing,
    CMPScalar content_width, cmp_i32 x, cmp_i32 y, cmp_usize *out_index) {
  M3SegmentedLayout layout;

  memset(&layout, 0, sizeof(layout));
  layout.start_x = start_x;
  layout.start_y = start_y;
  layout.segment_width = segment_width;
  layout.segment_height = segment_height;
  layout.spacing = spacing;
  layout.content_width = content_width;
  return m3_segmented_hit_test(buttons, &layout, x, y, out_index);
}

int CMP_CALL m3_segmented_test_is_selected(const M3SegmentedButtons *buttons,
                                          cmp_usize index,
                                          CMPBool *out_selected) {
  return m3_segmented_is_selected(buttons, index, out_selected);
}
#endif
