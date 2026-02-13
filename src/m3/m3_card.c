#include "m3/m3_card.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_CARD_TEST_FAIL_NONE 0u
#define M3_CARD_TEST_FAIL_SHADOW_INIT 1u
#define M3_CARD_TEST_FAIL_RIPPLE_INIT 2u
#define M3_CARD_TEST_FAIL_SHADOW_SET 3u
#define M3_CARD_TEST_FAIL_RESOLVE_COLORS 4u
#define M3_CARD_TEST_FAIL_OUTLINE_WIDTH 5u
#define M3_CARD_TEST_FAIL_RIPPLE_RADIUS 6u
#define M3_CARD_TEST_FAIL_RIPPLE_START 7u
#define M3_CARD_TEST_FAIL_RIPPLE_RELEASE 8u
#define M3_CARD_TEST_FAIL_CONTENT_BOUNDS 9u
#define M3_CARD_TEST_FAIL_MEASURE_CONTENT 10u
#define M3_CARD_TEST_FAIL_CORNER_RADIUS 11u

static cmp_u32 g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
static cmp_u32 g_m3_card_test_color_fail_after = 0u;

int CMP_CALL m3_card_test_set_fail_point(cmp_u32 fail_point) {
  g_m3_card_test_fail_point = fail_point;
  return CMP_OK;
}

int CMP_CALL m3_card_test_set_color_fail_after(cmp_u32 call_count) {
  g_m3_card_test_color_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL m3_card_test_clear_fail_points(void) {
  g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
  g_m3_card_test_color_fail_after = 0u;
  return CMP_OK;
}

static int m3_card_test_color_should_fail(void) {
  if (g_m3_card_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_card_test_color_fail_after -= 1u;
  return (g_m3_card_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_card_test_fail_point_match(cmp_u32 point) {
  if (g_m3_card_test_fail_point != point) {
    return 0;
  }
  g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
  return 1;
}
#endif /* GCOVR_EXCL_LINE */

static int m3_card_validate_color(const CMPColor *color) {
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

static int m3_card_color_set(CMPColor *color, CMPScalar r, CMPScalar g, CMPScalar b,
                             CMPScalar a) {
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
  if (m3_card_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return CMP_OK;
}

static int m3_card_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                    CMPColor *out_color) {
  int rc;

  if (base == NULL || out_color == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return CMP_ERR_RANGE;
  }

  rc = m3_card_validate_color(base);
  if (rc != CMP_OK) {
    return rc;
  }
#ifdef CMP_TESTING
  if (m3_card_test_color_should_fail()) {
    return CMP_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return CMP_OK;
}

static int m3_card_validate_edges(const CMPLayoutEdges *edges) {
  if (edges == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_card_validate_style(const M3CardStyle *style) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_CARD_VARIANT_ELEVATED:
  case M3_CARD_VARIANT_FILLED:
  case M3_CARD_VARIANT_OUTLINED:
    break;
  default:
    return CMP_ERR_RANGE;
  }

  rc = m3_card_validate_edges(&style->padding);
  if (rc != CMP_OK) {
    return rc;
  }

  if (style->min_width < 0.0f || style->min_height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f || style->outline_width < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->ripple_expand_duration < 0.0f ||
      style->ripple_fade_duration < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (style->shadow_enabled != CMP_FALSE && style->shadow_enabled != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  rc = m3_card_validate_color(&style->background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->outline_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->ripple_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_card_validate_measure_spec(CMPMeasureSpec spec) {
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.mode != CMP_MEASURE_EXACTLY &&
      spec.mode != CMP_MEASURE_AT_MOST) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != CMP_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_card_validate_rect(const CMPRect *rect) {
  if (rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int m3_card_measure_content(const M3CardStyle *style,
                                   CMPScalar *out_width, CMPScalar *out_height) {
  CMPScalar required_width;
  CMPScalar required_height;

  if (style == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_MEASURE_CONTENT)) {
    return CMP_ERR_IO;
  }
#endif

  required_width =
      style->padding.left + style->padding.right + style->outline_width * 2.0f;
  required_height =
      style->padding.top + style->padding.bottom + style->outline_width * 2.0f;
  if (required_width < 0.0f || required_height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_width = style->min_width;
  if (*out_width < required_width) {
    *out_width = required_width;
  }
  *out_height = style->min_height;
  if (*out_height < required_height) {
    *out_height = required_height;
  }
  return CMP_OK;
}

static int m3_card_compute_inner(const M3Card *card, CMPRect *out_inner,
                                 CMPScalar *out_corner) {
  CMPRect inner;
  CMPScalar outline_width;
  CMPScalar corner;
  int rc;

  if (card == NULL || out_inner == NULL || out_corner == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_rect(&card->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  outline_width = card->style.outline_width;
  if (outline_width < 0.0f) {
    return CMP_ERR_RANGE;
  }
  corner = card->style.corner_radius;
  if (corner < 0.0f) {
    return CMP_ERR_RANGE;
  }

  inner = card->bounds;
  if (outline_width > 0.0f) {
    inner.x += outline_width;
    inner.y += outline_width;
    inner.width -= outline_width * 2.0f;
    inner.height -= outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return CMP_ERR_RANGE;
    }
    corner -= outline_width;
    if (corner < 0.0f) {
      corner = 0.0f;
    }
  }

  *out_inner = inner;
  *out_corner = corner;
  return CMP_OK;
}

static int m3_card_compute_content_bounds(const M3Card *card,
                                          CMPRect *out_bounds) {
  CMPRect inner;
  CMPScalar inner_corner;
  CMPScalar padding_width;
  CMPScalar padding_height;
  int rc;

  if (card == NULL || out_bounds == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(&card->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_compute_inner(card, &inner, &inner_corner);
  if (rc != CMP_OK) {
    return rc;
  }

  padding_width = card->style.padding.left + card->style.padding.right;
  padding_height = card->style.padding.top + card->style.padding.bottom;
  inner.x += card->style.padding.left;
  inner.y += card->style.padding.top;
  inner.width -= padding_width;
  inner.height -= padding_height;
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_CONTENT_BOUNDS)) {
    inner.width = -1.0f;
  }
#endif
  if (inner.width < 0.0f || inner.height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  *out_bounds = inner;
  return CMP_OK;
}

static int m3_card_resolve_colors(const M3Card *card, CMPColor *out_background,
                                  CMPColor *out_outline, CMPColor *out_ripple) {
  int rc;

  if (card == NULL || out_background == NULL || out_outline == NULL ||
      out_ripple == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (card->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    *out_background = card->style.disabled_background_color;
    *out_outline = card->style.disabled_outline_color;
    *out_ripple = card->style.disabled_outline_color;
  } else {
    *out_background = card->style.background_color;
    *out_outline = card->style.outline_color;
    *out_ripple = card->style.ripple_color;
  }

  rc = m3_card_validate_color(out_background);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(out_outline);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_color(out_ripple);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_card_style_init_base(M3CardStyle *style, cmp_u32 variant) {
  int rc;

  if (style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = cmp_shadow_init(&style->shadow);
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_SHADOW_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  style->variant = variant;
  style->padding.left = M3_CARD_DEFAULT_PADDING_X;
  style->padding.right = M3_CARD_DEFAULT_PADDING_X;
  style->padding.top = M3_CARD_DEFAULT_PADDING_Y;
  style->padding.bottom = M3_CARD_DEFAULT_PADDING_Y;
  style->min_width = M3_CARD_DEFAULT_MIN_WIDTH;
  style->min_height = M3_CARD_DEFAULT_MIN_HEIGHT;
  style->corner_radius = M3_CARD_DEFAULT_CORNER_RADIUS;
  style->outline_width = M3_CARD_DEFAULT_OUTLINE_WIDTH;
  style->ripple_expand_duration = M3_CARD_DEFAULT_RIPPLE_EXPAND;
  style->ripple_fade_duration = M3_CARD_DEFAULT_RIPPLE_FADE;
  style->shadow_enabled = CMP_FALSE;

  rc = m3_card_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->disabled_background_color, 0.0f, 0.0f, 0.0f,
                         0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc =
      m3_card_color_set(&style->disabled_outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_card_style_init_elevated(M3CardStyle *style) {
  CMPColor shadow_color;
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_ELEVATED);
  if (rc != CMP_OK) {
    return rc;
  }

  style->outline_width = 0.0f;

  rc = m3_card_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_color_set(&shadow_color, 0.0f, 0.0f, 0.0f, 0.3f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = cmp_shadow_set(&style->shadow, 0.0f, 2.0f, 6.0f, 0.0f,
                     style->corner_radius, 3, shadow_color);
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_SHADOW_SET)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  style->shadow_enabled = CMP_TRUE;

  return CMP_OK;
}

int CMP_CALL m3_card_style_init_filled(M3CardStyle *style) {
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_FILLED);
  if (rc != CMP_OK) {
    return rc;
  }

  style->outline_width = 0.0f;

  rc = m3_card_color_set(&style->background_color, 0.94f, 0.94f, 0.94f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL m3_card_style_init_outlined(M3CardStyle *style) {
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_OUTLINED);
  if (rc != CMP_OK) {
    return rc;
  }

  style->outline_width = M3_CARD_DEFAULT_OUTLINE_WIDTH;

  rc = m3_card_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.75f, 0.75f, 0.75f, 1.0f);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

static int m3_card_widget_measure(void *widget, CMPMeasureSpec width,
                                  CMPMeasureSpec height, CMPSize *out_size) {
  M3Card *card;
  CMPScalar desired_width;
  CMPScalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_measure_spec(width);
  if (rc != CMP_OK) {
    return rc;
  }
  rc = m3_card_validate_measure_spec(height);
  if (rc != CMP_OK) {
    return rc;
  }

  card = (M3Card *)widget;
  rc = m3_card_validate_style(&card->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_measure_content(&card->style, &desired_width, &desired_height);
  if (rc != CMP_OK) {
    return rc;
  }

  if (width.mode == CMP_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == CMP_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
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

static int m3_card_widget_layout(void *widget, CMPRect bounds) {
  M3Card *card;
  int rc;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_rect(&bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  card = (M3Card *)widget;
  card->bounds = bounds;
  return CMP_OK;
}

static int m3_card_widget_paint(void *widget, CMPPaintContext *ctx) {
  M3Card *card;
  CMPRect bounds;
  CMPRect inner;
  CMPColor background;
  CMPColor outline;
  CMPColor ripple_color;
  CMPShadow shadow;
  CMPScalar corner_radius;
  CMPScalar inner_corner;
  CMPScalar outline_width;
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

  card = (M3Card *)widget;

  rc = m3_card_validate_style(&card->style);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_validate_rect(&card->bounds);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = m3_card_resolve_colors(card, &background, &outline, &ripple_color);
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RESOLVE_COLORS)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  CMP_UNUSED(ripple_color);

  corner_radius = card->style.corner_radius;
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_CORNER_RADIUS)) {
    corner_radius = -1.0f;
  }
#endif
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  bounds = card->bounds;
  outline_width = card->style.outline_width;
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_OUTLINE_WIDTH)) {
    outline_width = -1.0f;
  }
#endif /* GCOVR_EXCL_LINE */
  if (outline_width < 0.0f) {
    return CMP_ERR_RANGE;
  }

  if (card->style.shadow_enabled == CMP_TRUE) {
    shadow = card->style.shadow;
    shadow.corner_radius = corner_radius;
    rc = cmp_shadow_paint(&shadow, ctx->gfx, &bounds, &ctx->clip);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if (outline_width > 0.0f) {
    inner.x = bounds.x + outline_width;
    inner.y = bounds.y + outline_width;
    inner.width = bounds.width - outline_width * 2.0f;
    inner.height = bounds.height - outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return CMP_ERR_RANGE;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline,
                                     corner_radius);
    if (rc != CMP_OK) {
      return rc;
    }

    inner_corner = corner_radius - outline_width;
    if (inner_corner < 0.0f) {
      inner_corner = 0.0f;
    }
  } else {
    inner = bounds;
    inner_corner = corner_radius;
  }

  if (background.a > 0.0f) {
    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &inner, background,
                                     inner_corner);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  if ((card->widget.flags & CMP_WIDGET_FLAG_DISABLED) == 0) {
    rc = cmp_ripple_is_active(&card->ripple, &ripple_active);
    if (rc != CMP_OK) {
      return rc;
    }
    if (ripple_active == CMP_TRUE) {
      rc = cmp_ripple_paint(&card->ripple, ctx->gfx, &inner, inner_corner);
      if (rc != CMP_OK) {
        return rc;
      }
    }
  }

  return CMP_OK;
}

static int m3_card_widget_event(void *widget, const CMPInputEvent *event,
                                CMPBool *out_handled) {
  M3Card *card;
  CMPScalar center_x;
  CMPScalar center_y;
  CMPScalar max_radius;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_handled = CMP_FALSE;

  card = (M3Card *)widget;

  if (card->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    return CMP_OK;
  }

  switch (event->type) {
  case CMP_INPUT_POINTER_DOWN:
    if (card->pressed == CMP_TRUE) {
      return CMP_ERR_STATE;
    }
    if (card->style.ripple_expand_duration < 0.0f) {
      return CMP_ERR_RANGE;
    }
    rc = m3_card_validate_color(&card->style.ripple_color);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = m3_card_validate_rect(&card->bounds);
    if (rc != CMP_OK) {
      return rc;
    }
    center_x = (CMPScalar)event->data.pointer.x;
    center_y = (CMPScalar)event->data.pointer.y;
    rc = cmp_ripple_compute_max_radius(&card->bounds, center_x, center_y,
                                      &max_radius);
#ifdef CMP_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_RADIUS)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_ripple_start(&card->ripple, center_x, center_y, max_radius,
                         card->style.ripple_expand_duration,
                         card->style.ripple_color);
#ifdef CMP_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_START)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK) {
      return rc;
    }
    card->pressed = CMP_TRUE;
    *out_handled = CMP_TRUE;
    return CMP_OK;
  case CMP_INPUT_POINTER_UP:
    if (card->pressed == CMP_FALSE) {
      return CMP_OK;
    }
    if (card->style.ripple_fade_duration < 0.0f) {
      return CMP_ERR_RANGE;
    }
    card->pressed = CMP_FALSE;
    rc = cmp_ripple_release(&card->ripple, card->style.ripple_fade_duration);
#ifdef CMP_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_RELEASE)) {
      rc = CMP_ERR_IO;
    }
#endif
    if (rc != CMP_OK && rc != CMP_ERR_STATE) {
      return rc;
    }
    if (card->on_click != NULL) {
      rc = card->on_click(card->on_click_ctx, card);
      if (rc != CMP_OK) {
        return rc;
      }
    }
    *out_handled = CMP_TRUE;
    return CMP_OK;
  default:
    return CMP_OK;
  }
}

static int m3_card_widget_get_semantics(void *widget,
                                        CMPSemantics *out_semantics) {
  M3Card *card;

  if (widget == NULL || out_semantics == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  card = (M3Card *)widget;
  out_semantics->role = CMP_SEMANTIC_BUTTON;
  out_semantics->flags = 0;
  if (card->widget.flags & CMP_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_DISABLED;
  }
  if (card->widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= CMP_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return CMP_OK;
}

static int m3_card_widget_destroy(void *widget) {
  M3Card *card;

  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  card = (M3Card *)widget;
  card->widget.ctx = NULL;
  card->widget.vtable = NULL;
  card->widget.handle.id = 0u;
  card->widget.handle.generation = 0u;
  card->widget.flags = 0u;
  memset(&card->style, 0, sizeof(card->style));
  card->bounds.x = 0.0f;
  card->bounds.y = 0.0f;
  card->bounds.width = 0.0f;
  card->bounds.height = 0.0f;
  card->pressed = CMP_FALSE;
  card->on_click = NULL;
  card->on_click_ctx = NULL;
  memset(&card->ripple, 0, sizeof(card->ripple));
  return CMP_OK;
}

static const CMPWidgetVTable g_m3_card_widget_vtable = {
    m3_card_widget_measure,       m3_card_widget_layout,
    m3_card_widget_paint,         m3_card_widget_event,
    m3_card_widget_get_semantics, m3_card_widget_destroy};

int CMP_CALL m3_card_init(M3Card *card, const M3CardStyle *style) {
  int rc;

  if (card == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(card, 0, sizeof(*card));
  card->style = *style;
  card->pressed = CMP_FALSE;

  rc = cmp_ripple_init(&card->ripple);
#ifdef CMP_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_INIT)) {
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }

  card->widget.ctx = card;
  card->widget.vtable = &g_m3_card_widget_vtable;
  card->widget.handle.id = 0u;
  card->widget.handle.generation = 0u;
  card->widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  return CMP_OK;
}

int CMP_CALL m3_card_set_style(M3Card *card, const M3CardStyle *style) {
  int rc;

  if (card == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(style);
  if (rc != CMP_OK) {
    return rc;
  }

  card->style = *style;
  return CMP_OK;
}

int CMP_CALL m3_card_set_on_click(M3Card *card, CMPCardOnClick on_click,
                                 void *ctx) {
  if (card == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  card->on_click = on_click;
  card->on_click_ctx = ctx;
  return CMP_OK;
}

int CMP_CALL m3_card_get_content_bounds(const M3Card *card, CMPRect *out_bounds) {
  return m3_card_compute_content_bounds(card, out_bounds);
}

#ifdef CMP_TESTING
int CMP_CALL m3_card_test_validate_color(const CMPColor *color) {
  return m3_card_validate_color(color);
}

int CMP_CALL m3_card_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                   CMPScalar b, CMPScalar a) {
  return m3_card_color_set(color, r, g, b, a);
}

int CMP_CALL m3_card_test_color_with_alpha(const CMPColor *base, CMPScalar alpha,
                                          CMPColor *out_color) {
  return m3_card_color_with_alpha(base, alpha, out_color);
}

int CMP_CALL m3_card_test_validate_edges(const CMPLayoutEdges *edges) {
  return m3_card_validate_edges(edges);
}

int CMP_CALL m3_card_test_validate_style(const M3CardStyle *style) {
  return m3_card_validate_style(style);
}

int CMP_CALL m3_card_test_validate_measure_spec(CMPMeasureSpec spec) {
  return m3_card_validate_measure_spec(spec);
}

int CMP_CALL m3_card_test_validate_rect(const CMPRect *rect) {
  return m3_card_validate_rect(rect);
}

int CMP_CALL m3_card_test_measure_content(const M3CardStyle *style,
                                         CMPScalar *out_width,
                                         CMPScalar *out_height) {
  return m3_card_measure_content(style, out_width, out_height);
}

int CMP_CALL m3_card_test_compute_inner(const M3Card *card, CMPRect *out_inner,
                                       CMPScalar *out_corner) {
  return m3_card_compute_inner(card, out_inner, out_corner);
}

int CMP_CALL m3_card_test_compute_content_bounds(const M3Card *card,
                                                CMPRect *out_bounds) {
  return m3_card_compute_content_bounds(card, out_bounds);
}

int CMP_CALL m3_card_test_resolve_colors(const M3Card *card,
                                        CMPColor *out_background,
                                        CMPColor *out_outline,
                                        CMPColor *out_ripple) {
  return m3_card_resolve_colors(card, out_background, out_outline, out_ripple);
}
#endif
