#include "m3/m3_sheet.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_SHEET_TEST_FAIL_NONE 0u
#define M3_SHEET_TEST_FAIL_SHADOW_INIT 1u
#define M3_SHEET_TEST_FAIL_COLOR_SET 2u
#define M3_SHEET_TEST_FAIL_ANIM_INIT 3u
#define M3_SHEET_TEST_FAIL_ANIM_START 4u
#define M3_SHEET_TEST_FAIL_ANIM_TARGET 5u

static m3_u32 g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
static m3_u32 g_m3_sheet_test_color_fail_after = 0u;

int M3_CALL m3_sheet_test_set_fail_point(m3_u32 fail_point) {
  g_m3_sheet_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_sheet_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_sheet_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_sheet_test_clear_fail_points(void) {
  g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
  g_m3_sheet_test_color_fail_after = 0u;
  return M3_OK;
}

static int m3_sheet_test_color_should_fail(void) {
  if (g_m3_sheet_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_sheet_test_color_fail_after -= 1u;
  return (g_m3_sheet_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_sheet_test_fail_point_match(m3_u32 point) {
  if (g_m3_sheet_test_fail_point != point) {
    return 0;
  }
  g_m3_sheet_test_fail_point = M3_SHEET_TEST_FAIL_NONE;
  return 1;
}
#endif

static int m3_sheet_validate_color(const M3Color *color) {
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

static int m3_sheet_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                              M3Scalar b, M3Scalar a) {
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
  if (m3_sheet_test_color_should_fail()) {
    return M3_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_sheet_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->right < 0.0f || edges->top < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_sheet_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_sheet_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_sheet_validate_style(const M3SheetStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_SHEET_VARIANT_STANDARD:
  case M3_SHEET_VARIANT_MODAL:
    break;
  default:
    return M3_ERR_RANGE;
  }

  rc = m3_sheet_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  if (style->height < 0.0f || style->min_height < 0.0f ||
      style->max_height < 0.0f || style->max_width < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->max_height > 0.0f && style->max_height < style->min_height) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->dismiss_threshold < 0.0f || style->dismiss_threshold > 1.0f) {
    return M3_ERR_RANGE;
  }
  if (style->min_fling_velocity < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->spring_stiffness <= 0.0f || style->spring_damping < 0.0f ||
      style->spring_mass <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->spring_tolerance < 0.0f || style->spring_rest_velocity < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
    return M3_ERR_RANGE;
  }
  if (style->scrim_enabled != M3_FALSE && style->scrim_enabled != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_sheet_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_sheet_validate_color(&style->scrim_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_sheet_compute_preferred_height(const M3SheetStyle *style,
                                             M3Scalar *out_height) {
  M3Scalar height;

  if (style == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  height = style->height;
  if (height < style->min_height) {
    height = style->min_height;
  }
  if (style->max_height > 0.0f && height > style->max_height) {
    height = style->max_height;
  }
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_height = height;
  return M3_OK;
}

static int m3_sheet_compute_bounds(const M3SheetStyle *style,
                                   const M3Rect *overlay, M3Rect *out_bounds) {
  M3Scalar width;
  M3Scalar height;
  int rc;

  if (style == NULL || overlay == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_rect(overlay);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_compute_preferred_height(style, &height);
  if (rc != M3_OK) {
    return rc;
  }

  if (height > overlay->height) {
    height = overlay->height;
  }
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  width = overlay->width;
  if (style->max_width > 0.0f && width > style->max_width) {
    width = style->max_width;
  }
  if (width < 0.0f) {
    return M3_ERR_RANGE;
  }

  out_bounds->width = width;
  out_bounds->height = height;
  out_bounds->x = overlay->x + (overlay->width - width) * 0.5f;
  out_bounds->y = overlay->y + overlay->height - height;
  return M3_OK;
}

static int m3_sheet_apply_offset(M3Sheet *sheet, M3Scalar offset,
                                 M3Bool reset_velocity, M3Bool *out_changed) {
  M3Scalar height;
  M3Scalar clamped;
  M3Bool clamped_flag;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  clamped = offset;
  clamped_flag = M3_FALSE;
  if (clamped < 0.0f) {
    clamped = 0.0f;
    clamped_flag = M3_TRUE;
  }
  if (clamped > height) {
    clamped = height;
    clamped_flag = M3_TRUE;
  }

  if (out_changed != NULL) {
    *out_changed = (clamped != sheet->offset) ? M3_TRUE : M3_FALSE;
  }

  sheet->offset = clamped;
  sheet->anim.spring.position = clamped;
  sheet->anim.value = clamped;
  if (reset_velocity == M3_TRUE || clamped_flag == M3_TRUE) {
    sheet->anim.spring.velocity = 0.0f;
  }
  return M3_OK;
}

static int m3_sheet_current_bounds(const M3Sheet *sheet, M3Rect *out_bounds) {
  if (sheet == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_bounds = sheet->sheet_bounds;
  out_bounds->y += sheet->offset;
  return m3_sheet_validate_rect(out_bounds);
}

static int m3_sheet_compute_scrim_alpha(const M3Sheet *sheet,
                                        M3Scalar *out_alpha) {
  M3Scalar height;
  M3Scalar alpha;

  if (sheet == NULL || out_alpha == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (height <= 0.0f) {
    *out_alpha = 0.0f;
    return M3_OK;
  }

  alpha = 1.0f - sheet->offset / height;
  if (alpha < 0.0f) {
    alpha = 0.0f;
  }
  if (alpha > 1.0f) {
    alpha = 1.0f;
  }

  *out_alpha = alpha;
  return M3_OK;
}

static int m3_sheet_update_hidden(M3Sheet *sheet) {
  M3Scalar height;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (sheet->open == M3_TRUE) {
    sheet->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
    return M3_OK;
  }

  if (sheet->offset >= height && height > 0.0f) {
    sheet->widget.flags |= M3_WIDGET_FLAG_HIDDEN;
  } else if (height > 0.0f) {
    sheet->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
  }

  return M3_OK;
}

static int m3_sheet_start_animation(M3Sheet *sheet, M3Scalar target) {
  M3Scalar height;
  M3Scalar clamped;
  int rc;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  height = sheet->sheet_bounds.height;
  if (height < 0.0f) {
    return M3_ERR_RANGE;
  }

  clamped = target;
  if (clamped < 0.0f) {
    clamped = 0.0f;
  }
  if (clamped > height) {
    clamped = height;
  }

  rc = m3_anim_controller_set_spring_target(&sheet->anim, clamped);
#ifdef M3_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_TARGET)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_sheet_open(M3Sheet *sheet) {
  int rc;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  sheet->open = M3_TRUE;
  sheet->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
  rc = m3_sheet_start_animation(sheet, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_sheet_dismiss(M3Sheet *sheet) {
  int rc;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (sheet->open == M3_FALSE) {
    return M3_OK;
  }

  sheet->open = M3_FALSE;
  sheet->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
  if (sheet->on_action != NULL) {
    rc = sheet->on_action(sheet->on_action_ctx, sheet, M3_SHEET_ACTION_DISMISS);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_sheet_start_animation(sheet, sheet->sheet_bounds.height);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_sheet_hit_test(const M3Sheet *sheet, M3Scalar x, M3Scalar y,
                             M3Bool *out_inside) {
  M3Rect bounds;
  int rc;

  if (sheet == NULL || out_inside == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_current_bounds(sheet, &bounds);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_rect_contains_point(&bounds, x, y, out_inside);
}

#ifdef M3_TESTING
int M3_CALL m3_sheet_test_validate_color(const M3Color *color) {
  return m3_sheet_validate_color(color);
}

int M3_CALL m3_sheet_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                    M3Scalar b, M3Scalar a) {
  return m3_sheet_color_set(color, r, g, b, a);
}

int M3_CALL m3_sheet_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_sheet_validate_edges(edges);
}

int M3_CALL m3_sheet_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_sheet_validate_measure_spec(spec);
}

int M3_CALL m3_sheet_test_validate_rect(const M3Rect *rect) {
  return m3_sheet_validate_rect(rect);
}

int M3_CALL m3_sheet_test_validate_style(const M3SheetStyle *style) {
  return m3_sheet_validate_style(style);
}

int M3_CALL m3_sheet_test_apply_offset(M3Sheet *sheet, M3Scalar offset,
                                       M3Bool reset_velocity,
                                       M3Bool *out_changed) {
  return m3_sheet_apply_offset(sheet, offset, reset_velocity, out_changed);
}

int M3_CALL m3_sheet_test_compute_scrim_alpha(const M3Sheet *sheet,
                                              M3Scalar *out_alpha) {
  return m3_sheet_compute_scrim_alpha(sheet, out_alpha);
}
#endif

static int m3_sheet_style_init_base(M3SheetStyle *style, m3_u32 variant) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_shadow_init(&style->shadow);
#ifdef M3_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_SHADOW_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
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
  style->shadow_enabled = M3_FALSE;
  style->scrim_enabled = M3_FALSE;

  rc = m3_sheet_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_sheet_style_init_standard(M3SheetStyle *style) {
  int rc;

  rc = m3_sheet_style_init_base(style, M3_SHEET_VARIANT_STANDARD);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  style->shadow_enabled = M3_FALSE;
  style->scrim_enabled = M3_FALSE;
  return M3_OK;
}

int M3_CALL m3_sheet_style_init_modal(M3SheetStyle *style) {
  int rc;

  rc = m3_sheet_style_init_base(style, M3_SHEET_VARIANT_MODAL);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_sheet_color_set(&style->scrim_color, 0.0f, 0.0f, 0.0f, 0.32f);
  if (rc != M3_OK) {
    return rc;
  }

  style->shadow_enabled = M3_TRUE;
  style->scrim_enabled = M3_TRUE;
  return M3_OK;
}

static int m3_sheet_widget_measure(void *widget, M3MeasureSpec width,
                                   M3MeasureSpec height, M3Size *out_size) {
  M3Sheet *sheet;
  M3Scalar desired_height;
  M3Scalar desired_width;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_sheet_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_compute_preferred_height(&sheet->style, &desired_height);
  if (rc != M3_OK) {
    return rc;
  }

  desired_width = 0.0f;
  if (sheet->style.max_width > 0.0f) {
    desired_width = sheet->style.max_width;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    if (desired_width > 0.0f && desired_width < width.size) {
      out_size->width = desired_width;
    } else {
      out_size->width = width.size;
    }
  } else {
    out_size->width = desired_width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height =
        (desired_height > height.size) ? height.size : desired_height;
  } else {
    out_size->height = desired_height;
  }

  return M3_OK;
}

static int m3_sheet_widget_layout(void *widget, M3Rect bounds) {
  M3Sheet *sheet;
  M3Rect sheet_bounds;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_compute_bounds(&sheet->style, &bounds, &sheet_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  sheet->overlay_bounds = bounds;
  sheet->sheet_bounds = sheet_bounds;

  rc = m3_sheet_apply_offset(sheet, sheet->offset, M3_TRUE, NULL);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_sheet_update_hidden(sheet);
}

static int m3_sheet_widget_paint(void *widget, M3PaintContext *ctx) {
  M3Sheet *sheet;
  M3Rect sheet_bounds;
  const M3Rect *shadow_clip;
  M3Color scrim_color;
  M3Scalar scrim_alpha;
  M3Scalar corner_radius;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL || ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  sheet = (M3Sheet *)widget;

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_validate_rect(&sheet->overlay_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_current_bounds(sheet, &sheet_bounds);
  if (rc != M3_OK) {
    return rc;
  }

  if (sheet->style.scrim_enabled == M3_TRUE) {
    rc = m3_sheet_compute_scrim_alpha(sheet, &scrim_alpha);
    if (rc != M3_OK) {
      return rc;
    }

    if (scrim_alpha > 0.0f) {
      scrim_color = sheet->style.scrim_color;
      scrim_color.a = scrim_color.a * scrim_alpha;
      if (scrim_color.a > 0.0f) {
        rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &ctx->clip, scrim_color,
                                         0.0f);
        if (rc != M3_OK) {
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

  if (sheet->style.shadow_enabled == M3_TRUE) {
    rc = m3_shadow_paint(&sheet->style.shadow, ctx->gfx, &sheet_bounds,
                         shadow_clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  corner_radius = sheet->style.corner_radius;
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc =
      ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &sheet_bounds,
                                  sheet->style.background_color, corner_radius);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_sheet_widget_event(void *widget, const M3InputEvent *event,
                                 M3Bool *out_handled) {
  M3Sheet *sheet;
  M3Bool inside;
  M3Scalar new_offset;
  M3Scalar height;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  sheet = (M3Sheet *)widget;
  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  if (sheet->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }
  if (sheet->widget.flags & M3_WIDGET_FLAG_HIDDEN) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    rc = m3_sheet_hit_test(sheet, (M3Scalar)event->data.pointer.x,
                           (M3Scalar)event->data.pointer.y, &inside);
    if (rc != M3_OK) {
      return rc;
    }

    if (sheet->style.scrim_enabled == M3_TRUE) {
      if (inside == M3_FALSE) {
        rc = m3_sheet_dismiss(sheet);
        if (rc != M3_OK) {
          return rc;
        }
      }
      *out_handled = M3_TRUE;
      return M3_OK;
    }

    if (inside == M3_TRUE) {
      *out_handled = M3_TRUE;
    }
    return M3_OK;
  case M3_INPUT_POINTER_UP:
  case M3_INPUT_POINTER_MOVE:
  case M3_INPUT_POINTER_SCROLL:
    if (sheet->style.scrim_enabled == M3_TRUE) {
      *out_handled = M3_TRUE;
      return M3_OK;
    }
    rc = m3_sheet_hit_test(sheet, (M3Scalar)event->data.pointer.x,
                           (M3Scalar)event->data.pointer.y, &inside);
    if (rc != M3_OK) {
      return rc;
    }
    if (inside == M3_TRUE) {
      *out_handled = M3_TRUE;
    }
    return M3_OK;
  case M3_INPUT_GESTURE_DRAG_START:
    rc = m3_sheet_hit_test(sheet, event->data.gesture.start_x,
                           event->data.gesture.start_y, &inside);
    if (rc != M3_OK) {
      return rc;
    }
    if (inside == M3_FALSE) {
      return M3_OK;
    }
    if (sheet->dragging == M3_TRUE) {
      return M3_ERR_STATE;
    }
    sheet->dragging = M3_TRUE;
    sheet->drag_start_offset = sheet->offset;
    rc = m3_anim_controller_stop(&sheet->anim);
    if (rc != M3_OK) {
      return rc;
    }
    sheet->anim.spring.velocity = 0.0f;
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_GESTURE_DRAG_UPDATE:
    if (sheet->dragging == M3_FALSE) {
      return M3_OK;
    }
    height = sheet->sheet_bounds.height;
    if (height < 0.0f) {
      return M3_ERR_RANGE;
    }
    new_offset = sheet->drag_start_offset + event->data.gesture.total_y;
    rc = m3_sheet_apply_offset(sheet, new_offset, M3_TRUE, NULL);
    if (rc != M3_OK) {
      return rc;
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_GESTURE_DRAG_END:
    if (sheet->dragging == M3_FALSE) {
      return M3_OK;
    }
    sheet->dragging = M3_FALSE;
    height = sheet->sheet_bounds.height;
    if (height <= 0.0f) {
      return M3_ERR_RANGE;
    }
    sheet->anim.spring.velocity = event->data.gesture.velocity_y;
    if (event->data.gesture.velocity_y > sheet->style.min_fling_velocity ||
        sheet->offset >= height * sheet->style.dismiss_threshold) {
      rc = m3_sheet_dismiss(sheet);
      if (rc != M3_OK) {
        return rc;
      }
    } else {
      rc = m3_sheet_open(sheet);
      if (rc != M3_OK) {
        return rc;
      }
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_GESTURE_FLING:
    if (sheet->dragging == M3_FALSE) {
      return M3_OK;
    }
    sheet->dragging = M3_FALSE;
    height = sheet->sheet_bounds.height;
    if (height <= 0.0f) {
      return M3_ERR_RANGE;
    }
    sheet->anim.spring.velocity = event->data.gesture.velocity_y;
    if (event->data.gesture.velocity_y > sheet->style.min_fling_velocity) {
      rc = m3_sheet_dismiss(sheet);
      if (rc != M3_OK) {
        return rc;
      }
    } else {
      rc = m3_sheet_open(sheet);
      if (rc != M3_OK) {
        return rc;
      }
    }
    *out_handled = M3_TRUE;
    return M3_OK;
  default:
    return M3_OK;
  }
}

static int m3_sheet_widget_get_semantics(void *widget,
                                         M3Semantics *out_semantics) {
  M3Sheet *sheet;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  sheet = (M3Sheet *)widget;
  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  if (sheet->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (sheet->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_sheet_widget_destroy(void *widget) {
  M3Sheet *sheet;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
  sheet->dragging = M3_FALSE;
  sheet->open = M3_FALSE;
  sheet->anim.mode = M3_ANIM_MODE_NONE;
  sheet->anim.running = M3_FALSE;
  sheet->anim.value = 0.0f;
  memset(&sheet->anim.timing, 0, sizeof(sheet->anim.timing));
  memset(&sheet->anim.spring, 0, sizeof(sheet->anim.spring));
  sheet->on_action = NULL;
  sheet->on_action_ctx = NULL;
  return M3_OK;
}

static const M3WidgetVTable g_m3_sheet_widget_vtable = {
    m3_sheet_widget_measure,       m3_sheet_widget_layout,
    m3_sheet_widget_paint,         m3_sheet_widget_event,
    m3_sheet_widget_get_semantics, m3_sheet_widget_destroy};

int M3_CALL m3_sheet_init(M3Sheet *sheet, const M3SheetStyle *style) {
  int rc;

  if (sheet == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  memset(sheet, 0, sizeof(*sheet));
  sheet->style = *style;
  sheet->open = M3_TRUE;
  sheet->dragging = M3_FALSE;
  sheet->offset = 0.0f;
  sheet->drag_start_offset = 0.0f;

  rc = m3_anim_controller_init(&sheet->anim);
#ifdef M3_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_start_spring(
      &sheet->anim, 0.0f, 0.0f, style->spring_stiffness, style->spring_damping,
      style->spring_mass);
#ifdef M3_TESTING
  if (m3_sheet_test_fail_point_match(M3_SHEET_TEST_FAIL_ANIM_START)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_spring_set_tolerance(&sheet->anim.spring, style->spring_tolerance,
                               style->spring_rest_velocity);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_stop(&sheet->anim);
  if (rc != M3_OK) {
    return rc;
  }

  sheet->widget.ctx = sheet;
  sheet->widget.vtable = &g_m3_sheet_widget_vtable;
  sheet->widget.handle.id = 0u;
  sheet->widget.handle.generation = 0u;
  sheet->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_sheet_set_style(M3Sheet *sheet, const M3SheetStyle *style) {
  M3Bool running;
  M3Scalar target;
  int rc;

  if (sheet == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_anim_controller_is_running(&sheet->anim, &running);
  if (rc != M3_OK) {
    return rc;
  }

  target = sheet->open == M3_TRUE ? 0.0f : sheet->sheet_bounds.height;
  rc = m3_anim_controller_start_spring(
      &sheet->anim, sheet->offset, target, style->spring_stiffness,
      style->spring_damping, style->spring_mass);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_spring_set_tolerance(&sheet->anim.spring, style->spring_tolerance,
                               style->spring_rest_velocity);
  if (rc != M3_OK) {
    return rc;
  }

  if (running == M3_FALSE) {
    rc = m3_anim_controller_stop(&sheet->anim);
    if (rc != M3_OK) {
      return rc;
    }
  }

  sheet->style = *style;
  return M3_OK;
}

int M3_CALL m3_sheet_set_on_action(M3Sheet *sheet, M3SheetOnAction on_action,
                                   void *ctx) {
  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  sheet->on_action = on_action;
  sheet->on_action_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_sheet_set_open(M3Sheet *sheet, M3Bool open) {
  int rc;

  if (sheet == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (open != M3_FALSE && open != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  sheet->dragging = M3_FALSE;

  if (open == M3_TRUE) {
    return m3_sheet_open(sheet);
  }

  sheet->open = M3_FALSE;
  if (sheet->sheet_bounds.height <= 0.0f ||
      sheet->offset >= sheet->sheet_bounds.height) {
    sheet->widget.flags |= M3_WIDGET_FLAG_HIDDEN;
    rc = m3_anim_controller_stop(&sheet->anim);
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  }

  sheet->widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;
  rc = m3_sheet_start_animation(sheet, sheet->sheet_bounds.height);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_sheet_get_open(const M3Sheet *sheet, M3Bool *out_open) {
  if (sheet == NULL || out_open == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_open = sheet->open;
  return M3_OK;
}

int M3_CALL m3_sheet_step(M3Sheet *sheet, M3Scalar dt, M3Bool *out_changed) {
  M3Bool running;
  M3Bool finished;
  M3Bool changed;
  M3Scalar value;
  int rc;

  if (sheet == NULL || out_changed == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (dt < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_sheet_validate_style(&sheet->style);
  if (rc != M3_OK) {
    return rc;
  }

  *out_changed = M3_FALSE;

  rc = m3_anim_controller_is_running(&sheet->anim, &running);
  if (rc != M3_OK) {
    return rc;
  }

  if (running == M3_FALSE) {
    rc = m3_sheet_update_hidden(sheet);
    if (rc != M3_OK) {
      return rc;
    }
    return M3_OK;
  }

  rc = m3_anim_controller_step(&sheet->anim, dt, &value, &finished);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_sheet_apply_offset(sheet, value, M3_FALSE, &changed);
  if (rc != M3_OK) {
    return rc;
  }
  if (changed == M3_TRUE) {
    *out_changed = M3_TRUE;
  }

  if (finished == M3_TRUE) {
    rc = m3_sheet_update_hidden(sheet);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_sheet_get_bounds(const M3Sheet *sheet, M3Rect *out_bounds) {
  if (sheet == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  return m3_sheet_current_bounds(sheet, out_bounds);
}

int M3_CALL m3_sheet_get_content_bounds(const M3Sheet *sheet,
                                        M3Rect *out_bounds) {
  M3Rect bounds;
  int rc;

  if (sheet == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sheet_current_bounds(sheet, &bounds);
  if (rc != M3_OK) {
    return rc;
  }

  bounds.x += sheet->style.padding.left;
  bounds.y += sheet->style.padding.top;
  bounds.width -= sheet->style.padding.left + sheet->style.padding.right;
  bounds.height -= sheet->style.padding.top + sheet->style.padding.bottom;
  if (bounds.width < 0.0f) {
    bounds.width = 0.0f;
  }
  if (bounds.height < 0.0f) {
    bounds.height = 0.0f;
  }

  *out_bounds = bounds;
  return M3_OK;
}
