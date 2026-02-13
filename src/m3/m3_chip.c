#include "m3/m3_chip.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_CHIP_TEST_FAIL_NONE 0u
#define M3_CHIP_TEST_FAIL_TEXT_STYLE_INIT 1u
#define M3_CHIP_TEST_FAIL_RIPPLE_INIT 2u
#define M3_CHIP_TEST_FAIL_RIPPLE_RADIUS 3u
#define M3_CHIP_TEST_FAIL_RIPPLE_START 4u
#define M3_CHIP_TEST_FAIL_RIPPLE_RELEASE 5u
#define M3_CHIP_TEST_FAIL_RESOLVE_COLORS 6u
#define M3_CHIP_TEST_FAIL_RESOLVE_CORNER 7u
#define M3_CHIP_TEST_FAIL_OUTLINE_WIDTH 8u
#define M3_CHIP_TEST_FAIL_DELETE_BOUNDS 9u

static cmp_u32 g_m3_chip_test_fail_point = M3_CHIP_TEST_FAIL_NONE;
static cmp_u32 g_m3_chip_test_color_fail_after = 0u;

int CMP_CALL m3_chip_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_chip_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_chip_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_chip_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_chip_test_clear_fail_points(void) {
  g_m3_chip_test_fail_point = M3_CHIP_TEST_FAIL_NONE;
  g_m3_chip_test_color_fail_after = 0u;
  return CMP_OK;
}

static int m3_chip_test_color_should_fail(void) {
  if (g_m3_chip_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_chip_test_color_fail_after -= 1u;
  return (g_m3_chip_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_chip_test_fail_point_match(cmp_u32 point) {
  if (g_m3_chip_test_fail_point != point) {
    return 0;
  }
  g_m3_chip_test_fail_point = M3_CHIP_TEST_FAIL_NONE;
  return 1;
}
#endif

static int m3_chip_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_chip_validate_color(const CMPColor *color) {
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

static int m3_chip_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                             CMPScalar b, CMPScalar a) {
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
  if (m3_chip_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_chip_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                    CMPColor *out_color) {
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_chip_validate_color(base);
  if (rc != CMP_OK) {
    return rc;
  }
#ifdef CMP_TESTING
  if (m3_chip_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return CMP_OK;
}

static int m3_chip_validate_text_style(const CMPTextStyle *style,
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
  if (style->italic != CMP_FALSE && style->italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_chip_validate_color(&style->color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_chip_validate_layout(const M3ChipLayout *layout) {
  if (layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (layout->padding_x < 0.0f || layout->padding_y < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (layout->min_width < 0.0f || layout->min_height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (layout->icon_size < 0.0f || layout->icon_gap < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (layout->delete_icon_thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_chip_validate_style(const M3ChipStyle *style,
                                  CMPBool require_family) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_CHIP_VARIANT_ASSIST:
  case M3_CHIP_VARIANT_FILTER:
  case M3_CHIP_VARIANT_INPUT:
  case M3_CHIP_VARIANT_SUGGESTION:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  if (style->outline_width < 0.0f || style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->ripple_expand_duration < 0.0f ||
      style->ripple_fade_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_chip_validate_layout(&style->layout);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_chip_validate_layout(&style->dense_layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_text_style(&style->text_style, require_family);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_chip_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_chip_validate_color(&style->outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->selected_background_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->selected_outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->selected_text_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(&style->ripple_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_chip_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED &&
      spec.mode != CMP_MEASURE_EXACTLY && spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_chip_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_chip_validate_backend(const CMPTextBackend *backend) {
  if (backend == NULL || backend->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int m3_chip_metrics_update(M3Chip *chip) {
  int rc;

  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (chip->metrics_valid == CMP_TRUE) {
    return CMP_OK;
  }

  rc = cmp_text_measure_utf8(&chip->text_backend, chip->font, chip->utf8_label,
                             chip->utf8_len, &chip->metrics);
  if (rc != CMP_OK) {
    return rc;
  }

  chip->metrics_valid = CMP_TRUE;
  return CMP_OK;
}

static int m3_chip_get_layout(const M3Chip *chip, M3ChipLayout *out_layout) {
  int rc;

  if (chip == NULL || out_layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_bool(chip->dense);
  if (rc != CMP_OK) {
    return rc;
  }

  if (chip->dense == CMP_TRUE) {
    *out_layout = chip->style.dense_layout;
  } else {
    *out_layout = chip->style.layout;
  }
  return CMP_OK;
}

static int m3_chip_resolve_colors(const M3Chip *chip, CMPColor *out_background,
                                  CMPColor *out_text, CMPColor *out_outline,
                                  CMPColor *out_ripple) {
  int rc;

  if (chip == NULL || out_background == NULL || out_text == NULL ||
      out_outline == NULL || out_ripple == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (chip->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    *out_background = chip->style.disabled_background_color;
    *out_text = chip->style.disabled_text_color;
    *out_outline = chip->style.disabled_outline_color;
    *out_ripple = chip->style.disabled_text_color;
  } else if (chip->selected == CMP_TRUE) {
    *out_background = chip->style.selected_background_color;
    *out_text = chip->style.selected_text_color;
    *out_outline = chip->style.selected_outline_color;
    *out_ripple = chip->style.ripple_color;
  } else {
    *out_background = chip->style.background_color;
    *out_text = chip->style.text_style.color;
    *out_outline = chip->style.outline_color;
    *out_ripple = chip->style.ripple_color;
  }

  rc = m3_chip_validate_color(out_background);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_chip_validate_color(out_text);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(out_outline);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_validate_color(out_ripple);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_chip_resolve_corner(const M3Chip *chip, CMPScalar *out_corner) {
  CMPScalar corner;
  CMPScalar min_side;

  if (chip == NULL || out_corner == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }
  if (chip->style.corner_radius < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (chip->bounds.width < 0.0f || chip->bounds.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  corner = chip->style.corner_radius;
  min_side = (chip->bounds.width < chip->bounds.height) ? chip->bounds.width
                                                        : chip->bounds.height;
  if (min_side < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (corner > min_side * 0.5f) {
    corner = min_side * 0.5f;
  }
  *out_corner = corner;
  return CMP_OK;
}

static int m3_chip_compute_content_layout(M3Chip *chip,
                                          const M3ChipLayout *layout,
                                          CMPScalar *out_text_x,
                                          CMPScalar *out_text_y,
                                          CMPRect *out_delete_bounds) {
  CMPTextMetrics metrics;
  CMPRect bounds;
  CMPScalar available_width;
  CMPScalar available_height;
  CMPScalar content_width;
  CMPScalar content_x;
  CMPScalar text_x;
  CMPScalar text_y;
  CMPScalar icon_x;
  CMPScalar icon_y;
  int rc;

  if (chip == NULL || layout == NULL || out_text_x == NULL ||
      out_text_y == NULL || out_delete_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  out_delete_bounds->x = 0.0f;
  out_delete_bounds->y = 0.0f;
  out_delete_bounds->width = 0.0f;
  out_delete_bounds->height = 0.0f;

  rc = m3_chip_validate_layout(layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_rect(&chip->bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_metrics_update(chip);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  metrics = chip->metrics;
  bounds = chip->bounds;
  available_width = bounds.width - layout->padding_x * 2.0f;
  available_height = bounds.height - layout->padding_y * 2.0f;
  if (available_width < 0.0f) {
    available_width = 0.0f;
  }
  if (available_height < 0.0f) {
    available_height = 0.0f;
  }

  content_width = metrics.width;
  if (chip->show_delete == CMP_TRUE) {
    if (layout->icon_size <= 0.0f) {
      return CMP_ERR_RANGE;
    }
    content_width += layout->icon_gap + layout->icon_size;
  }

  if (available_width <= content_width) {
    content_x = bounds.x + layout->padding_x;
  } else {
    content_x =
        bounds.x + layout->padding_x + (available_width - content_width) * 0.5f;
  }

  if (available_height <= metrics.height) {
    text_y = bounds.y + layout->padding_y + metrics.baseline;
  } else {
    text_y = bounds.y + layout->padding_y +
             (available_height - metrics.height) * 0.5f + metrics.baseline;
  }

  text_x = content_x;
  *out_text_x = text_x;
  *out_text_y = text_y;

  if (chip->show_delete == CMP_TRUE) {
    icon_x = content_x + metrics.width + layout->icon_gap;
    if (available_height >= layout->icon_size) {
      icon_y = bounds.y + layout->padding_y +
               (available_height - layout->icon_size) * 0.5f;
    } else {
      icon_y = bounds.y + layout->padding_y;
    }
    out_delete_bounds->x = icon_x;
    out_delete_bounds->y = icon_y;
    out_delete_bounds->width = layout->icon_size;
    out_delete_bounds->height = layout->icon_size;
  }

#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_DELETE_BOUNDS)) {
    return CMP_ERR_IO;
  }
#endif

  return CMP_OK;
}

static int m3_chip_draw_delete_icon(const CMPGfx *gfx, const CMPRect *bounds,
                                    CMPColor color, CMPScalar thickness) {
  CMPScalar inset;
  CMPScalar max_inset;
  CMPScalar x0;
  CMPScalar y0;
  CMPScalar x1;
  CMPScalar y1;
  int rc;

  if (gfx == NULL || bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (gfx->vtable == NULL || gfx->vtable->draw_line == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (bounds->width < 0.0f || bounds->height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (bounds->width == 0.0f || bounds->height == 0.0f) {
    return CMP_OK;
  }

  inset = thickness;
  if (inset < 0.0f) {
    inset = 0.0f; /* GCOVR_EXCL_LINE */
  }
  max_inset = bounds->width * 0.5f;
  if (bounds->height * 0.5f < max_inset) {
    max_inset = bounds->height * 0.5f; /* GCOVR_EXCL_LINE */
  }
  if (inset > max_inset) {
    inset = max_inset; /* GCOVR_EXCL_LINE */
  }

  x0 = bounds->x + inset;
  y0 = bounds->y + inset;
  x1 = bounds->x + bounds->width - inset;
  y1 = bounds->y + bounds->height - inset;

  rc = gfx->vtable->draw_line(gfx->ctx, x0, y0, x1, y1, color, thickness);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = gfx->vtable->draw_line(gfx->ctx, x0, y1, x1, y0, color, thickness);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  return CMP_OK;
}

static int m3_chip_style_init_layout(M3ChipLayout *layout, CMPScalar padding_x,
                                     CMPScalar padding_y, CMPScalar min_width,
                                     CMPScalar min_height, CMPScalar icon_size,
                                     CMPScalar icon_gap,
                                     CMPScalar delete_thickness) {
  int rc;

  if (layout == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  layout->padding_x = padding_x;
  layout->padding_y = padding_y;
  layout->min_width = min_width;
  layout->min_height = min_height;
  layout->icon_size = icon_size;
  layout->icon_gap = icon_gap;
  layout->delete_icon_thickness = delete_thickness;

  rc = m3_chip_validate_layout(layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_chip_style_init_base(M3ChipStyle *style, cmp_u32 variant) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  memset(style, 0, sizeof(*style));
  style->variant = variant;

  rc = cmp_text_style_init(&style->text_style);
#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_TEXT_STYLE_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  style->outline_width = M3_CHIP_DEFAULT_OUTLINE_WIDTH;
  style->corner_radius = M3_CHIP_DEFAULT_CORNER_RADIUS;
  style->ripple_expand_duration = M3_CHIP_DEFAULT_RIPPLE_EXPAND;
  style->ripple_fade_duration = M3_CHIP_DEFAULT_RIPPLE_FADE;

  rc = m3_chip_style_init_layout(
      &style->layout, M3_CHIP_DEFAULT_PADDING_X, M3_CHIP_DEFAULT_PADDING_Y,
      M3_CHIP_DEFAULT_MIN_WIDTH, M3_CHIP_DEFAULT_MIN_HEIGHT,
      M3_CHIP_DEFAULT_ICON_SIZE, M3_CHIP_DEFAULT_ICON_GAP,
      M3_CHIP_DEFAULT_DELETE_THICKNESS);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_style_init_layout(
      &style->dense_layout, M3_CHIP_DENSE_PADDING_X, M3_CHIP_DENSE_PADDING_Y,
      M3_CHIP_DENSE_MIN_WIDTH, M3_CHIP_DENSE_MIN_HEIGHT,
      M3_CHIP_DENSE_ICON_SIZE, M3_CHIP_DENSE_ICON_GAP,
      M3_CHIP_DENSE_DELETE_THICKNESS);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_color_set(&style->text_style.color, 0.0f, 0.0f, 0.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_chip_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->outline_color, 0.7f, 0.7f, 0.7f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->selected_background_color, 0.9f, 0.9f, 0.9f,
                         1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc =
      m3_chip_color_set(&style->selected_outline_color, 0.6f, 0.6f, 0.6f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->selected_text_color, 0.0f, 0.0f, 0.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_with_alpha(&style->text_style.color, 0.38f,
                                &style->disabled_text_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_chip_widget_measure(void *widget, CMPMeasureSpec width,
                                  CMPMeasureSpec height, CMPSize *out_size) {
  M3Chip *chip;
  M3ChipLayout layout;
  CMPTextMetrics metrics;
  CMPScalar desired_width;
  CMPScalar desired_height;
  CMPScalar min_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_chip_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  chip = (M3Chip *)widget;
  rc = m3_chip_validate_style(&chip->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_get_layout(chip, &layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_metrics_update(chip);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  metrics = chip->metrics;
  desired_width = metrics.width + layout.padding_x * 2.0f;
  if (chip->show_delete == CMP_TRUE) {
    desired_width += layout.icon_gap + layout.icon_size;
  }
  if (desired_width < layout.min_width) {
    desired_width = layout.min_width; /* GCOVR_EXCL_LINE */
  }

  desired_height = metrics.height + layout.padding_y * 2.0f;
  min_height = layout.min_height;
  if (chip->show_delete == CMP_TRUE) {
    if (layout.icon_size + layout.padding_y * 2.0f > min_height) {
      min_height =
          layout.icon_size + layout.padding_y * 2.0f; /* GCOVR_EXCL_LINE */
    }
  }
  if (desired_height < min_height) {
    desired_height = min_height; /* GCOVR_EXCL_LINE */
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size)
                          ? width.size
                          : desired_width; /* GCOVR_EXCL_LINE */
  } else {                                 /* GCOVR_EXCL_LINE */
    out_size->width = desired_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size; /* GCOVR_EXCL_LINE */
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return CMP_OK;
}

static int m3_chip_widget_layout(void *widget, CMPRect bounds) {
  M3Chip *chip;
  int rc = CMP_OK;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  chip = (M3Chip *)widget;
  chip->bounds = bounds;
  return CMP_OK;
}

static int m3_chip_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Chip *chip;
  M3ChipLayout layout;
  CMPRect bounds;
  CMPRect inner;
  CMPRect delete_bounds;
  CMPColor background;
  CMPColor text_color;
  CMPColor outline;
  CMPColor ripple_color;
  CMPScalar corner_radius;
  CMPScalar inner_corner;
  CMPScalar outline_width;
  CMPScalar text_x;
  CMPScalar text_y;
  CMPBool ripple_active;
  int rc;

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

  chip = (M3Chip *)widget;

  rc = m3_chip_validate_style(&chip->style, CMP_FALSE);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_rect(&chip->bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_get_layout(chip, &layout);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_metrics_update(chip);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_chip_resolve_colors(chip, &background, &text_color, &outline,
                              &ripple_color);
#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RESOLVE_COLORS)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_chip_resolve_corner(chip, &corner_radius);
#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RESOLVE_CORNER)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  bounds = chip->bounds;
  outline_width = chip->style.outline_width;
#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_OUTLINE_WIDTH)) {
    outline_width = -1.0f;
  }
#endif
  if (outline_width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (outline_width > 0.0f) {
    inner.x = bounds.x + outline_width;
    inner.y = bounds.y + outline_width;
    inner.width = bounds.width - outline_width * 2.0f;
    inner.height = bounds.height - outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return CMP_ERR_RANGE;
    }
    inner_corner = corner_radius - outline_width;
    if (inner_corner < 0.0f) {
      inner_corner = 0.0f; /* GCOVR_EXCL_LINE */
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline,
                                     corner_radius);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, background,
                                     inner_corner);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
  } else {
    inner = bounds;
    inner_corner = corner_radius;
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, background,
                                     inner_corner);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
  }

  rc = cmp_ripple_is_active(&chip->ripple, &ripple_active);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (ripple_active == CMP_TRUE) {
    chip->ripple.color = ripple_color;
    rc = cmp_ripple_paint(&chip->ripple, ctx->gfx, &ctx->clip, inner_corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_chip_compute_content_layout(chip, &layout, &text_x, &text_y,
                                      &delete_bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (chip->utf8_label != NULL || chip->utf8_len == 0) {
    if (chip->utf8_label != NULL && chip->utf8_len != 0) {
      rc = ctx->gfx->text_vtable->draw_text(ctx->gfx->ctx, chip->font,
                                            chip->utf8_label, chip->utf8_len,
                                            text_x, text_y, text_color);
      if (rc != CMP_OK) {
        return rc;
      }
    }
  } else {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (chip->show_delete == CMP_TRUE) {
    rc = m3_chip_draw_delete_icon(ctx->gfx, &delete_bounds, text_color,
                                  layout.delete_icon_thickness);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

static int m3_chip_widget_event(void *widget, const CMPInputEvent *event,
                                CMPBool *out_handled) {
  M3Chip *chip;
  M3ChipLayout layout;
  CMPRect delete_bounds;
  CMPScalar center_x;
  CMPScalar center_y;
  CMPScalar max_radius;
  CMPBool hit_delete;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  chip = (M3Chip *)widget;

  if (chip->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (chip->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    if (chip->style.ripple_expand_duration < 0.0f) {
      return CMP_ERR_RANGE;
    }
    rc = m3_chip_validate_color(&chip->style.ripple_color);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_chip_validate_rect(&chip->bounds);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_chip_get_layout(chip, &layout);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    hit_delete = CMP_FALSE;
    if (chip->show_delete == CMP_TRUE) {
      rc = m3_chip_compute_content_layout(chip, &layout, &center_x, &center_y,
                                          &delete_bounds);
      if (rc != CMP_OK) {
        return rc;
      }
      if (event->data.pointer.x >= delete_bounds.x &&
          event->data.pointer.x <= delete_bounds.x + delete_bounds.width &&
          event->data.pointer.y >= delete_bounds.y &&
          event->data.pointer.y <= delete_bounds.y + delete_bounds.height) {
        hit_delete = CMP_TRUE;
      }
    }

    center_x = (CMPScalar)event->data.pointer.x;
    center_y = (CMPScalar)event->data.pointer.y;
    rc = cmp_ripple_compute_max_radius(&chip->bounds, center_x, center_y,
                                       &max_radius);
#ifdef CMP_TESTING
    if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RIPPLE_RADIUS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_ripple_start(&chip->ripple, center_x, center_y, max_radius,
                          chip->style.ripple_expand_duration,
                          chip->style.ripple_color);
#ifdef CMP_TESTING
    if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RIPPLE_START)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    chip->pressed = CMP_TRUE;
    chip->pressed_delete = hit_delete;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (chip->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    if (chip->style.ripple_fade_duration < 0.0f) {
      return CMP_ERR_RANGE;
    }
    chip->pressed = CMP_FALSE;
    rc = cmp_ripple_release(&chip->ripple, chip->style.ripple_fade_duration);
#ifdef CMP_TESTING
    if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RIPPLE_RELEASE)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK && rc != CMP_ERR_STATE) {
      return rc;
    }
    if (chip->pressed_delete == CMP_TRUE) {
      chip->pressed_delete = CMP_FALSE;
      if (chip->on_delete != NULL) {
        rc = chip->on_delete(chip->on_delete_ctx, chip);
        if (rc != CMP_OK) {
          return rc;
        }
      }
    } else {
      if (chip->style.variant == M3_CHIP_VARIANT_FILTER) {
        chip->selected = (chip->selected == CMP_TRUE) ? CMP_FALSE : CMP_TRUE;
      }
      if (chip->on_click != NULL) {
        rc = chip->on_click(chip->on_click_ctx, chip);
        if (rc != CMP_OK) {
          return rc;
        }
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
}

static int m3_chip_widget_get_semantics(void *widget,
                                        CMPSemantics *out_semantics) {
  M3Chip *chip;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  chip = (M3Chip *)widget;
  out_semantics->role = CMP_SEMANTIC_BUTTON;
  out_semantics->flags = 0;
  if (chip->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (chip->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  if (chip->selected == CMP_TRUE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_SELECTED;
  }
  out_semantics->utf8_label = chip->utf8_label;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_chip_widget_destroy(void *widget) {
  M3Chip *chip;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  chip = (M3Chip *)widget;
  rc = CMP_OK;
  if (chip->owns_font == CMP_TRUE &&
      (chip->font.id != 0u || chip->font.generation != 0u)) {
    if (chip->text_backend.vtable != NULL &&
        chip->text_backend.vtable->destroy_font != NULL) {
      rc = chip->text_backend.vtable->destroy_font(chip->text_backend.ctx,
                                                   chip->font);
    } else {
      rc = CMP_ERR_UNSUPPORTED;
    }
  }

  chip->font.id = 0u;
  chip->font.generation = 0u;
  chip->utf8_label = NULL;
  chip->utf8_len = 0;
  chip->metrics_valid = CMP_FALSE;
  chip->owns_font = CMP_FALSE;
  chip->pressed = CMP_FALSE;
  chip->pressed_delete = CMP_FALSE;
  chip->selected = CMP_FALSE;
  chip->dense = CMP_FALSE;
  chip->show_delete = CMP_FALSE;
  chip->text_backend.ctx = NULL;
  chip->text_backend.vtable = NULL;
  chip->widget.ctx = NULL;
  chip->widget.vtable = NULL;
  chip->on_click = NULL;
  chip->on_click_ctx = NULL;
  chip->on_delete = NULL;
  chip->on_delete_ctx = NULL;
  return rc;
}

static const CMPWidgetVTable g_m3_chip_widget_vtable = {
    m3_chip_widget_measure,       m3_chip_widget_layout,
    m3_chip_widget_paint,         m3_chip_widget_event,
    m3_chip_widget_get_semantics, m3_chip_widget_destroy};

int CMP_CALL m3_chip_style_init_assist(M3ChipStyle *style) {
  int rc;

  rc = m3_chip_style_init_base(style, M3_CHIP_VARIANT_ASSIST);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_chip_style_init_filter(M3ChipStyle *style) {
  int rc;

  rc = m3_chip_style_init_base(style, M3_CHIP_VARIANT_FILTER);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_color_set(&style->selected_background_color, 0.85f, 0.92f, 1.0f,
                         1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->selected_outline_color, 0.35f, 0.55f, 0.85f,
                         1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->selected_text_color, 0.1f, 0.2f, 0.45f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_chip_style_init_input(M3ChipStyle *style) {
  int rc;

  rc = m3_chip_style_init_base(style, M3_CHIP_VARIANT_INPUT);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->outline_width = 0.0f;
  rc = m3_chip_color_set(&style->background_color, 0.95f, 0.95f, 0.95f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_chip_style_init_suggestion(M3ChipStyle *style) {
  int rc;

  rc = m3_chip_style_init_base(style, M3_CHIP_VARIANT_SUGGESTION);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_color_set(&style->outline_color, 0.8f, 0.8f, 0.8f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_chip_color_set(&style->selected_background_color, 0.92f, 0.92f, 0.92f,
                         1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc =
      m3_chip_color_set(&style->selected_outline_color, 0.7f, 0.7f, 0.7f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_chip_init(M3Chip *chip, const CMPTextBackend *backend,
                          const M3ChipStyle *style, const char *utf8_label,
                          cmp_usize utf8_len) {
  int rc;

  if (chip == NULL || backend == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_chip_validate_backend(backend);
  if (rc != CMP_OK) {
    return rc;
  }
  if (backend->vtable->create_font == NULL ||
      backend->vtable->destroy_font == NULL ||
      backend->vtable->measure_text == NULL ||
      backend->vtable->draw_text == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = m3_chip_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(chip, 0, sizeof(*chip));
  chip->text_backend = *backend;
  chip->style = *style;
  chip->utf8_label = utf8_label;
  chip->utf8_len = utf8_len;
  chip->metrics_valid = CMP_FALSE;
  chip->pressed = CMP_FALSE;
  chip->pressed_delete = CMP_FALSE;
  chip->owns_font = CMP_TRUE;
  chip->selected = CMP_FALSE;
  chip->dense = CMP_FALSE;
  chip->show_delete = CMP_FALSE;

  rc = cmp_ripple_init(&chip->ripple);
#ifdef CMP_TESTING
  if (m3_chip_test_fail_point_match(M3_CHIP_TEST_FAIL_RIPPLE_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_text_font_create(backend, &style->text_style, &chip->font);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  chip->widget.ctx = chip;
  chip->widget.vtable = &g_m3_chip_widget_vtable;
  chip->widget.handle.id = 0u;
  chip->widget.handle.generation = 0u;
  chip->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_label(M3Chip *chip, const char *utf8_label,
                               cmp_usize utf8_len) {
  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_label == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  chip->utf8_label = utf8_label;
  chip->utf8_len = utf8_len;
  chip->metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_style(M3Chip *chip, const M3ChipStyle *style) {
  CMPHandle new_font;
  int rc = CMP_OK;

  if (chip == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_validate_style(style, CMP_TRUE);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_text_font_create(&chip->text_backend, &style->text_style, &new_font);
  if (rc != CMP_OK) {
    return rc;
  }

  if (chip->owns_font == CMP_TRUE) {
    rc = cmp_text_font_destroy(&chip->text_backend, chip->font);
    if (rc != CMP_OK) {
      cmp_text_font_destroy(&chip->text_backend, new_font);
      return rc;
    }
  }

  chip->style = *style;
  chip->font = new_font;
  chip->owns_font = CMP_TRUE;
  chip->metrics_valid = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_selected(M3Chip *chip, CMPBool selected) {
  int rc;

  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_chip_validate_bool(selected);
  if (rc != CMP_OK) {
    return rc;
  }

  chip->selected = selected;
  return CMP_OK;
}

int CMP_CALL m3_chip_get_selected(const M3Chip *chip, CMPBool *out_selected) {
  if (chip == NULL || out_selected == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_selected = chip->selected;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_dense(M3Chip *chip, CMPBool dense) {
  int rc;

  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_chip_validate_bool(dense);
  if (rc != CMP_OK) {
    return rc;
  }

  chip->dense = dense;
  return CMP_OK;
}

int CMP_CALL m3_chip_get_dense(const M3Chip *chip, CMPBool *out_dense) {
  if (chip == NULL || out_dense == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_dense = chip->dense;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_show_delete(M3Chip *chip, CMPBool show_delete) {
  int rc;

  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_chip_validate_bool(show_delete);
  if (rc != CMP_OK) {
    return rc;
  }

  chip->show_delete = show_delete;
  chip->pressed_delete = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_chip_get_show_delete(const M3Chip *chip,
                                     CMPBool *out_show_delete) {
  if (chip == NULL || out_show_delete == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_show_delete = chip->show_delete;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_on_click(M3Chip *chip, CMPChipOnClick on_click,
                                  void *ctx) {
  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  chip->on_click = on_click;
  chip->on_click_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_chip_set_on_delete(M3Chip *chip, CMPChipOnDelete on_delete,
                                   void *ctx) {
  if (chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  chip->on_delete = on_delete;
  chip->on_delete_ctx = ctx;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL m3_chip_test_validate_color(const CMPColor *color) {
  return m3_chip_validate_color(color);
}

int CMP_CALL m3_chip_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a) {
  return m3_chip_color_set(color, r, g, b, a);
}

int CMP_CALL m3_chip_test_color_with_alpha(const CMPColor *base,
                                           CMPScalar alpha,
                                           CMPColor *out_color) {
  return m3_chip_color_with_alpha(base, alpha, out_color);
}

int CMP_CALL m3_chip_test_validate_text_style(const CMPTextStyle *style,
                                              CMPBool require_family) {
  return m3_chip_validate_text_style(style, require_family);
}

int CMP_CALL m3_chip_test_validate_layout(const M3ChipLayout *layout) {
  return m3_chip_validate_layout(layout);
}

int CMP_CALL m3_chip_test_validate_style(const M3ChipStyle *style,
                                         CMPBool require_family) {
  return m3_chip_validate_style(style, require_family);
}

int CMP_CALL m3_chip_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_chip_validate_measure_spec(spec);
}

int CMP_CALL m3_chip_test_validate_rect(const CMPRect *rect) {
  return m3_chip_validate_rect(rect);
}

int CMP_CALL m3_chip_test_validate_backend(const CMPTextBackend *backend) {
  return m3_chip_validate_backend(backend);
}

int CMP_CALL m3_chip_test_metrics_update(M3Chip *chip) {
  return m3_chip_metrics_update(chip);
}

int CMP_CALL m3_chip_test_resolve_colors(const M3Chip *chip,
                                         CMPColor *out_background,
                                         CMPColor *out_text,
                                         CMPColor *out_outline,
                                         CMPColor *out_ripple) {
  return m3_chip_resolve_colors(chip, out_background, out_text, out_outline,
                                out_ripple);
}

int CMP_CALL m3_chip_test_resolve_corner(const M3Chip *chip,
                                         CMPScalar *out_corner) {
  return m3_chip_resolve_corner(chip, out_corner);
}

int CMP_CALL m3_chip_test_compute_delete_bounds(M3Chip *chip,
                                                CMPRect *out_bounds) {
  M3ChipLayout layout;
  int rc;
  CMPScalar text_x;
  CMPScalar text_y = 0.0f;

  if (chip == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_chip_get_layout(chip, &layout);
  if (rc != CMP_OK) {
    return rc;
  }

  return m3_chip_compute_content_layout(chip, &layout, &text_x, &text_y,
                                        out_bounds);
}

int CMP_CALL m3_chip_test_draw_delete_icon(const CMPGfx *gfx,
                                           const CMPRect *bounds,
                                           CMPColor color,
                                           CMPScalar thickness) {
  return m3_chip_draw_delete_icon(gfx, bounds, color, thickness);
}
#endif
