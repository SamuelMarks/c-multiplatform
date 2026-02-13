#include "m3/m3_sheet.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_SHEET_TEST_FAIL_NONE 0u
#define M3_SHEET_TEST_FAIL_SHADOW_INIT 1u
#define M3_SHEET_TEST_FAIL_COLOR_SET 2u
#define M3_SHEET_TEST_FAIL_ANIM_INIT 3u
#define M3_SHEET_TEST_FAIL_ANIM_START 4u
#define M3_SHEET_TEST_FAIL_ANIM_TARGET 5u
#define M3_SHEET_TEST_FAIL_APPLY_OFFSET 6u
#define M3_SHEET_TEST_FAIL_SCRIM_ALPHA 7u

static cmp_u32 g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
static cmp_u32 g_m3_sheet_test_color_fail_after = 0u;

int CMP_CALL m3_sheet_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_sheet_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_sheet_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_sheet_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_sheet_test_clear_fail_points(void) {
  g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
  g_m3_sheet_test_color_fail_after = 0u;
  return CMP_OK;
}

static int m3_sheet_test_color_should_fail(void) {
  if (g_m3_sheet_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_sheet_test_color_fail_after -= 1u;
  return (g_m3_sheet_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_sheet_test_fail_point_match(cmp_u32 point) {
  if (g_m3_sheet_test_fail_point != point) {
    return 0;
  }
  g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
  return 1;
}
#endif

static int m3_sheet_validate_color(const CMPColor *color) {
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

static int m3_sheet_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
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
  if (m3_sheet_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_sheet_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_sheet_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_sheet_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_sheet_validate_style(const M3SheetStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_SHEET_VARIANT_STANDARD:
  case M3_SHEET_VARIANT_MODAL:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  rc = m3_sheet_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  if (style->height < 0.0f || style->min_height < 0.0f ||
      style->max_height < 0.0f || style->max_width < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->max_height > 0.0f && style->max_height < style->min_height) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->dismiss_threshold < 0.0f || style->dismiss_threshold > 1.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->min_fling_velocity < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->spring_stiffness <= 0.0f || style->spring_damping < 0.0f ||
      style->spring_mass <= 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->spring_tolerance < 0.0f || style->spring_rest_velocity < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->shadow_enabled != CMP_FALSE && style->shadow_enabled != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }
  if (style->scrim_enabled != CMP_FALSE && style->scrim_enabled != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_sheet_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_sheet_validate_color(&style->scrim_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_sheet_compute_preferred_height(const M3SheetStyle *style,
                                             CMPScalar *out_height) {
  CMPScalar height;

  if (style == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  height = style->height;
  if (height < style->min_height) {
    height = style->min_height; /* GCOVR_EXCL_LINE */
  }
  if (style->max_height > 0.0f && height > style->max_height) {
    height = style->max_height; /* GCOVR_EXCL_LINE */
  }
  if (height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  *out_height = height;
  return CMP_OK;
}

static int m3_sheet_compute_bounds(const M3SheetStyle *style,
                                   const CMPRect *overlay, CMPRect *out_bounds) {
  CMPScalar width;
  CMPScalar height;
  int rc;

  if (style == NULL || overlay == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_validate_rect(overlay);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_compute_preferred_height(style, &height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (height > overlay->height) {
    height = overlay->height; /* GCOVR_EXCL_LINE */
  }
  if (height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  width = overlay->width;
  if (style->max_width > 0.0f && width > style->max_width) {
    width = style->max_width;
  }
  if (width < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  out_bounds->width = width;
  out_bounds->height = height;
  out_bounds->x = overlay->x + (overlay->width - width) * 0.5f;
  out_bounds->y = overlay->y + overlay->height - height;
  return CMP_OK;
}

static int m3_sheet_apply_offset(M3Sheet *sheet, CMPScalar offset,
                                 CMPBool reset_velocity, CMPBool *out_changed) {
  CMPScalar height;
  CMPScalar clamped;
  CMPBool clamped_flag;

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_APPLY_OFFSET)) {
    return CMP_ERR_UNKNOWN;
  }
#endif

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  clamped = offset;
  clamped_flag = CMP_FALSE;
  if (clamped < 0.0f) {
    clamped = 0.0f;
    clamped_flag = CMP_TRUE;
  }
  if (clamped > height) {
    clamped = height;
    clamped_flag = CMP_TRUE;
  }

  if (out_changed != NULL) {
    *out_changed = (clamped != sheet->offset) ? CMP_TRUE : CMP_FALSE;
  }

  sheet->offset = clamped;
  sheet->anim.spring.position = clamped;
  sheet->anim.value = clamped;
  if (reset_velocity == CMP_TRUE || clamped_flag == CMP_TRUE) {
    sheet->anim.spring.velocity = 0.0f;
  }
  return CMP_OK;
}

static int m3_sheet_current_bounds(const M3Sheet *sheet, CMPRect *out_bounds) {
  if (sheet == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  *out_bounds = sheet->sheet_bounds;
  out_bounds->y += sheet->offset;
  return m3_sheet_validate_rect(out_bounds);
}

static int m3_sheet_compute_scrim_alpha(const M3Sheet *sheet,
                                        CMPScalar *out_alpha) {
  CMPScalar height;
  CMPScalar alpha; /* GCOVR_EXCL_LINE */

  if (sheet == NULL || out_alpha == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_SCRIM_ALPHA)) {
    return CMP_ERR_UNKNOWN;
  }
#endif /* GCOVR_EXCL_LINE */

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (height <= 0.0f) {
    *out_alpha = 0.0f;
    return CMP_OK;
  }

  alpha = 1.0f - sheet->offset / height;
  if (alpha < 0.0f) {
    alpha = 0.0f;
  }
  if (alpha > 1.0f) {
    alpha = 1.0f;
  }

  *out_alpha = alpha;
  return CMP_OK;
}

static int m3_sheet_update_hidden(M3Sheet *sheet) {
  CMPScalar height;

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  if (sheet->open == CMP_TRUE) {
    sheet->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;
    return CMP_OK;
  }

  if (sheet->offset >= height && height > 0.0f) {
    sheet->widget.flags |= CMP_WIDGET_FLAG_HIDDEN;
  } else if (height > 0.0f) {
    sheet->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_sheet_start_animation(M3Sheet *sheet, CMPScalar target) {
  CMPScalar height;
  CMPScalar clamped; /* GCOVR_EXCL_LINE */
  int rc;           /* GCOVR_EXCL_LINE */

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  clamped = target;
  if (clamped < 0.0f) {
    clamped = 0.0f; /* GCOVR_EXCL_LINE */
  }
  if (clamped > height) {
    clamped = height; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_anim_controller_set_spring_target(&sheet->anim, clamped);
#ifdef CMP_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_TARGET)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_sheet_open(M3Sheet *sheet) {
  int rc;

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  sheet->open = CMP_TRUE;
  sheet->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;
  rc = m3_sheet_start_animation(sheet, 0.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_sheet_dismiss(M3Sheet *sheet) {
  int rc;

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  if (sheet->open == CMP_FALSE) {
    return CMP_OK;
  }

  sheet->open = CMP_FALSE;
  sheet->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;
  if (sheet->on_action != NULL) {
    rc = sheet->on_action(sheet->on_action_ctx, sheet, M3_SHEET_ACTION_DISMISS);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = m3_sheet_start_animation(sheet, sheet->sheet_bounds.height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

static int m3_sheet_hit_test(const M3Sheet *sheet, CMPScalar x, CMPScalar y,
                             CMPBool *out_inside) {
  CMPRect bounds;
  int rc;

  if (sheet == NULL || out_inside == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_current_bounds(sheet, &bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return cmp_rect_contains_point(&bounds, x, y, out_inside);
}

#ifdef CMP_TESTING
int CMP_CALL m3_sheet_test_validate_color(const CMPColor *color) {
  return m3_sheet_validate_color(color);
}

int CMP_CALL m3_sheet_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a) {
  return m3_sheet_color_set(color, r, g, b, a);
}

int CMP_CALL m3_sheet_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_sheet_validate_edges(edges);
}

int CMP_CALL m3_sheet_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_sheet_validate_measure_spec(spec);
}

int CMP_CALL m3_sheet_test_validate_rect(const CMPRect *rect) {
  return m3_sheet_validate_rect(rect);
}

int CMP_CALL m3_sheet_test_validate_style(const M3SheetStyle *style) {
  return m3_sheet_validate_style(style);
}

int CMP_CALL m3_sheet_test_apply_offset(
    M3Sheet *sheet, CMPScalar offset, CMPBool reset_velocity,
    CMPBool *out_changed) { /* GCOVR_EXCL_LINE */
  return m3_sheet_apply_offset(sheet, offset, reset_velocity, out_changed);
}

int CMP_CALL m3_sheet_test_compute_scrim_alpha(const M3Sheet *sheet,
                                              CMPScalar *out_alpha) {
  return m3_sheet_compute_scrim_alpha(sheet, out_alpha);
}
#endif

static int m3_sheet_style_init_base(M3SheetStyle *style, cmp_u32 variant) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_shadow_init(&style->shadow);
#ifdef CMP_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_SHADOW_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  style->variant = variant;
  style->padding.left = M3_SHEET_DEFAULT_PADDING_X;
  style->padding.right = M3_SHEET_DEFAULT_PADDING_X;
  style->padding.top = M3_SHEET_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_SHEET_DEFAULT_PADDING_Y;
  style->height = M3_SHEET_DEFAULT_HEIGHT;
  style->min_height = M3_SHEET_DEFAULT_MIN_HEIGHT;
  style->max_height = M3_SHEET_DEFAULT_MAX_HEIGHT;
  style->max_width = M3_SHEET_DEFAULT_MAX_WIDTH;
  style->corner_radius = M3_SHEET_DEFAULT_CORNER_RADIUS;
  style->dismiss_threshold = M3_SHEET_DEFAULT_DISMISS_THRESHOLD;
  style->min_fling_velocity = M3_SHEET_DEFAULT_MIN_FLING_VELOCITY;
  style->spring_stiffness = M3_SHEET_DEFAULT_SPRING_STIFFNESS;
  style->spring_damping = M3_SHEET_DEFAULT_SPRING_DAMPING;
  style->spring_mass = M3_SHEET_DEFAULT_SPRING_MASS;
  style->spring_tolerance = M3_SHEET_DEFAULT_SPRING_TOLERANCE;
  style->spring_rest_velocity = M3_SHEET_DEFAULT_SPRING_REST_VELOCITY;
  style->shadow_enabled = CMP_FALSE;
  style->scrim_enabled = CMP_FALSE;

  rc = m3_sheet_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  return CMP_OK;
}

int CMP_CALL m3_sheet_style_init_standard(M3SheetStyle *style) {
  int rc;

  rc = m3_sheet_style_init_base(style, M3_SHEET_VARIANT_STANDARD);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_sheet_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->shadow_enabled = CMP_FALSE;
  style->scrim_enabled = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL m3_sheet_style_init_modal(M3SheetStyle *style) {
  int rc;

  rc = m3_sheet_style_init_base(style, M3_SHEET_VARIANT_MODAL);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_sheet_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.32f);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  style->shadow_enabled = CMP_TRUE;
  style->scrim_enabled = CMP_TRUE;
  return CMP_OK;
}

static int m3_sheet_widget_measure(void *widget, CMPMeasureSpec width,
                                   CMPMeasureSpec height, CMPSize *out_size) {
  M3Sheet *sheet;
  CMPScalar desired_height; /* GCOVR_EXCL_LINE */
  CMPScalar desired_width;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_sheet_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_compute_preferred_height(&sheet->style, &desired_height);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  desired_width = 0.0f;
  if (sheet->style.max_width > 0.0f) {
    desired_width = sheet->style.max_width;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    if (desired_width > 0.0f && desired_width < width.size) {
      out_size->width = desired_width; /* GCOVR_EXCL_LINE */
    } else {                           /* GCOVR_EXCL_LINE */
      out_size->width = width.size;
    }
  } else {
    out_size->width = desired_width;
  }

  if (height.mode == CMP_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == CMP_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return CMP_OK;
}

static int m3_sheet_widget_layout(void *widget, CMPRect bounds) {
  M3Sheet *sheet;
  CMPRect sheet_bounds;
  int rc; /* GCOVR_EXCL_LINE */

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = m3_sheet_compute_bounds(&sheet->style, &bounds, &sheet_bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  sheet->overlay_bounds = bounds;
  sheet->sheet_bounds = sheet_bounds;

  rc = m3_sheet_apply_offset(sheet, sheet->offset, CMP_TRUE, NULL);
  if (rc != CMP_OK) {
    return rc;
  }

  return m3_sheet_update_hidden(sheet);
}

static int m3_sheet_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Sheet *sheet;
  CMPRect sheet_bounds;
  const CMPRect *shadow_clip;
  CMPColor scrim_color;
  CMPScalar scrim_alpha;
  CMPScalar corner_radius; /* GCOVR_EXCL_LINE */
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  sheet = (M3Sheet *)widget;

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_sheet_validate_rect(&sheet->overlay_bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_sheet_current_bounds(sheet, &sheet_bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  if (sheet->style.scrim_enabled == CMP_TRUE) {
    rc = m3_sheet_compute_scrim_alpha(sheet, &scrim_alpha);
    if (rc != CMP_OK) {
      return rc;
    }

    if (scrim_alpha > 0.0f) {
      scrim_color = sheet->style.scrim_color;
      scrim_color.a = scrim_color.a * scrim_alpha;
      if (scrim_color.a > 0.0f) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ctx->clip, scrim_color,
                                         0.0f);
        if (rc != CMP_OK) {
          return rc;
        }
      }
    }
  }

  shadow_clip = NULL;
  if (ctx->gfx->vtable->push_clip != NULL &&
      ctx->gfx->vtable->pop_clip != NULL) {
    shadow_clip = &ctx->clip;
  }

  if (sheet->style.shadow_enabled == CMP_TRUE) {
    rc = cmp_shadow_paint(&sheet->style.shadow, ctx->gfx, &sheet_bounds,
                         shadow_clip);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  corner_radius = sheet->style.corner_radius;
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
  }

  rc =
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sheet_bounds,
                                  sheet->style.background_color, corner_radius);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_sheet_widget_event(void *widget, const CMPInputEvent *event,
                                 CMPBool *out_handled) {
  M3Sheet *sheet;
  CMPBool inside;
  CMPScalar new_offset;
  CMPScalar height;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc;
  }

  if (sheet->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }
  if (sheet->widget.flags & CMP_WIDGET_FLAG_HIDDEN) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    rc = m3_sheet_hit_test(sheet, (CMPScalar)event->data.pointer.x,
                           (CMPScalar)event->data.pointer.y, &inside);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }

    if (sheet->style.scrim_enabled == CMP_TRUE) {
      if (inside == CMP_FALSE) {
        rc = m3_sheet_dismiss(sheet);
        if (rc != CMP_OK) {
          return rc;
        }
      }
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }

    if (inside == CMP_TRUE) {
      *out_handled = CMP_TRUE; /* GCOVR_EXCL_LINE */
    }
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:   /* GCOVR_EXCL_LINE */
  case CMP_INPUT_POINTER_MOVE: /* GCOVR_EXCL_LINE */
  case CMP_INPUT_POINTER_SCROLL:
    if (sheet->style.scrim_enabled == CMP_TRUE) {
      *out_handled = CMP_TRUE;
      return CMP_OK;
    }
    rc = m3_sheet_hit_test(sheet, (CMPScalar)event->data.pointer.x,
                           (CMPScalar)event->data.pointer.y, &inside);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (inside == CMP_TRUE) {
      *out_handled = CMP_TRUE;
    }
    return CMP_OK;
  case CMP_INPUT_GESTURE_DRAG_START:
    rc = m3_sheet_hit_test(sheet, event->data.gesture.start_x,
                           event->data.gesture.start_y, &inside);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    if (inside == CMP_FALSE) {
      return CMP_OK;
    }
    if (sheet->dragging == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    sheet->dragging = CMP_TRUE;
    sheet->drag_start_offset = sheet->offset;
    rc = cmp_anim_controller_stop(&sheet->anim);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    sheet->anim.spring.velocity = 0.0f;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_GESTURE_DRAG_UPDATE:
    if (sheet->dragging == CMP_FALSE) {
      return CMP_OK;
    }
    height = sheet->sheet_bounds.height;
    if (height < 0.0f) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    new_offset = sheet->drag_start_offset + event->data.gesture.total_y;
    rc = m3_sheet_apply_offset(sheet, new_offset, CMP_TRUE, NULL);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_GESTURE_DRAG_END: /* GCOVR_EXCL_LINE */
    if (sheet->dragging == CMP_FALSE) {
      return CMP_OK;
    }
    sheet->dragging = CMP_FALSE;
    height = sheet->sheet_bounds.height;
    if (height <= 0.0f) {
      return CMP_ERR_RANGE;
    }
    sheet->anim.spring.velocity = event->data.gesture.velocity_y;
    if (event->data.gesture.velocity_y > sheet->style.min_fling_velocity ||
        sheet->offset >= height * sheet->style.dismiss_threshold) {
      rc = m3_sheet_dismiss(sheet);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    } else {
      rc = m3_sheet_open(sheet);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_GESTURE_FLING:
    if (sheet->dragging == CMP_FALSE) {
      return CMP_OK; /* GCOVR_EXCL_LINE */
    }
    sheet->dragging = CMP_FALSE;
    height = sheet->sheet_bounds.height;
    if (height <= 0.0f) {
      return CMP_ERR_RANGE; /* GCOVR_EXCL_LINE */
    }
    sheet->anim.spring.velocity = event->data.gesture.velocity_y;
    if (event->data.gesture.velocity_y > sheet->style.min_fling_velocity) {
      rc = m3_sheet_dismiss(sheet);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    } else {
      rc = m3_sheet_open(sheet);
      if (rc != CMP_OK) {
        return rc; /* GCOVR_EXCL_LINE */
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:        /* GCOVR_EXCL_LINE */
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }
}

static int m3_sheet_widget_get_semantics(
    void *widget, CMPSemantics *out_semantics) { /* GCOVR_EXCL_LINE */
  M3Sheet *sheet;                               /* GCOVR_EXCL_LINE */

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sheet = (M3Sheet *)widget;
  out_semantics->role = CMP_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (sheet->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (sheet->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_sheet_widget_destroy(void *widget) {
  M3Sheet *sheet; /* GCOVR_EXCL_LINE */

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sheet = (M3Sheet *)widget;
  sheet->widget.ctx = NULL;
  sheet->widget.vtable = NULL;
  sheet->widget.handle.id = 0u;
  sheet->widget.handle.generation = 0u;
  sheet->widget.flags = 0u;
  memset(&sheet->style, 0, sizeof(sheet->style));
  sheet->overlay_bounds.x = 0.0f;
  sheet->overlay_bounds.y = 0.0f;
  sheet->overlay_bounds.width = 0.0f;
  sheet->overlay_bounds.height = 0.0f;
  sheet->sheet_bounds = sheet->overlay_bounds;
  sheet->offset = 0.0f;
  sheet->drag_start_offset = 0.0f;
  sheet->dragging = CMP_FALSE;
  sheet->open = CMP_FALSE;
  sheet->anim.mode = CMP_ANIM_MODE_NONE;
  sheet->anim.running = CMP_FALSE;
  sheet->anim.value = 0.0f;
  memset(&sheet->anim.timing, 0, sizeof(sheet->anim.timing));
  memset(&sheet->anim.spring, 0, sizeof(sheet->anim.spring));
  sheet->on_action = NULL;
  sheet->on_action_ctx = NULL;
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_sheet_widget_vtable = {
    m3_sheet_widget_measure,       m3_sheet_widget_layout,
    m3_sheet_widget_paint,         m3_sheet_widget_event,
    m3_sheet_widget_get_semantics, m3_sheet_widget_destroy};

int CMP_CALL m3_sheet_init(M3Sheet *sheet, const M3SheetStyle *style) {
  int rc;

  if (sheet == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(sheet, 0, sizeof(*sheet));
  sheet->style = *style;
  sheet->open = CMP_TRUE;
  sheet->dragging = CMP_FALSE;
  sheet->offset = 0.0f;
  sheet->drag_start_offset = 0.0f;

  rc = cmp_anim_controller_init(&sheet->anim);
#ifdef CMP_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_anim_controller_start_spring(
      &sheet->anim, 0.0f, 0.0f, style->spring_stiffness, style->spring_damping,
      style->spring_mass);
#ifdef CMP_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_START)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_spring_set_tolerance(&sheet->anim.spring, style->spring_tolerance,
                               style->spring_rest_velocity);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_anim_controller_stop(&sheet->anim);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  sheet->widget.ctx = sheet;
  sheet->widget.vtable = &g_m3_sheet_widget_vtable;
  sheet->widget.handle.id = 0u;
  sheet->widget.handle.generation = 0u;
  sheet->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_sheet_set_style(M3Sheet *sheet, const M3SheetStyle *style) {
  CMPBool running;
  CMPScalar target; /* GCOVR_EXCL_LINE */
  int rc;

  if (sheet == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_anim_controller_is_running(&sheet->anim, &running);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  target = sheet->open == CMP_TRUE ? 0.0f : sheet->sheet_bounds.height;
  rc = cmp_anim_controller_start_spring(
      &sheet->anim, sheet->offset, target, style->spring_stiffness,
      style->spring_damping, style->spring_mass);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  rc = cmp_spring_set_tolerance(&sheet->anim.spring, style->spring_tolerance,
                               style->spring_rest_velocity);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (running == CMP_FALSE) {
    rc = cmp_anim_controller_stop(&sheet->anim);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
  }

  sheet->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_sheet_set_on_action(M3Sheet *sheet, CMPSheetOnAction on_action,
                                   void *ctx) {
  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sheet->on_action = on_action;
  sheet->on_action_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_sheet_set_open(M3Sheet *sheet, CMPBool open) {
  int rc;

  if (sheet == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (open != CMP_FALSE && open != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  sheet->dragging = CMP_FALSE;

  if (open == CMP_TRUE) {
    return m3_sheet_open(sheet);
  }

  sheet->open = CMP_FALSE;
  if (sheet->sheet_bounds.height <= 0.0f ||
      sheet->offset >= sheet->sheet_bounds.height) {
    sheet->widget.flags |= CMP_WIDGET_FLAG_HIDDEN; /* GCOVR_EXCL_LINE */
    rc = cmp_anim_controller_stop(&sheet->anim);   /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {                            /* GCOVR_EXCL_LINE */
      return rc;                                  /* GCOVR_EXCL_LINE */
    }
    return CMP_OK; /* GCOVR_EXCL_LINE */
  }

  sheet->widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;
  rc = m3_sheet_start_animation(sheet, sheet->sheet_bounds.height);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_sheet_get_open(const M3Sheet *sheet, CMPBool *out_open) {
  if (sheet == NULL || out_open == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_open = sheet->open;
  return CMP_OK;
}

int CMP_CALL m3_sheet_step(M3Sheet *sheet, CMPScalar dt, CMPBool *out_changed) {
  CMPBool running;
  CMPBool finished;
  CMPBool changed;
  CMPScalar value;
  int rc;

  if (sheet == NULL || out_changed == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != CMP_OK) {
    return rc;
  }

  *out_changed = CMP_FALSE;

  rc = cmp_anim_controller_is_running(&sheet->anim, &running);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  if (running == CMP_FALSE) {
    rc = m3_sheet_update_hidden(sheet);
    if (rc != CMP_OK) {
      return rc; /* GCOVR_EXCL_LINE */
    }
    return CMP_OK;
  }

  rc = cmp_anim_controller_step(&sheet->anim, dt, &value, &finished);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_sheet_apply_offset(sheet, value, CMP_FALSE, &changed);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }
  if (changed == CMP_TRUE) {
    *out_changed = CMP_TRUE;
  }

  if (finished == CMP_TRUE) {
    rc = m3_sheet_update_hidden(sheet); /* GCOVR_EXCL_LINE */
    if (rc != CMP_OK) {                  /* GCOVR_EXCL_LINE */
      return rc;                        /* GCOVR_EXCL_LINE */
    }
  }

  return CMP_OK;
}

int CMP_CALL m3_sheet_get_bounds(const M3Sheet *sheet, CMPRect *out_bounds) {
  if (sheet == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return m3_sheet_current_bounds(sheet, out_bounds);
}

int CMP_CALL m3_sheet_get_content_bounds(const M3Sheet *sheet,
                                        CMPRect *out_bounds) {
  CMPRect bounds;
  int rc;

  if (sheet == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_current_bounds(sheet, &bounds);
  if (rc != CMP_OK) {
    return rc; /* GCOVR_EXCL_LINE */
  }

  bounds.x += sheet->style.padding.left;
  bounds.y += sheet->style.padding.top;
  bounds.width -= sheet->style.padding.left + sheet->style.padding.right;
  bounds.height -= sheet->style.padding.top + sheet->style.padding.bottom;
  if (bounds.width < 0.0f) {
    bounds.width = 0.0f; /* GCOVR_EXCL_LINE */
  }
  if (bounds.height < 0.0f) {
    bounds.height = 0.0f; /* GCOVR_EXCL_LINE */
  }

  *out_bounds = bounds;
  return CMP_OK;
}
