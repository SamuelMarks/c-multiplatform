#include "m3/m3_card.h"

#include <string.h>

#ifdef M3_TESTING
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

static m3_u32 g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
static m3_u32 g_m3_card_test_color_fail_after = 0u;

int M3_CALL m3_card_test_set_fail_point(m3_u32 fail_point) {
  g_m3_card_test_fail_point = fail_point;
  return M3_OK;
}

int M3_CALL m3_card_test_set_color_fail_after(m3_u32 call_count) {
  g_m3_card_test_color_fail_after = call_count;
  return M3_OK;
}

int M3_CALL m3_card_test_clear_fail_points(void) {
  g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
  g_m3_card_test_color_fail_after = 0u;
  return M3_OK;
}

static int m3_card_test_color_should_fail(void) {
  if (g_m3_card_test_color_fail_after == 0u) {
    return 0;
  }
  g_m3_card_test_color_fail_after -= 1u;
  return (g_m3_card_test_color_fail_after == 0u) ? 1 : 0;
}

static int m3_card_test_fail_point_match(m3_u32 point) {
  if (g_m3_card_test_fail_point != point) {
    return 0;
  }
  g_m3_card_test_fail_point = M3_CARD_TEST_FAIL_NONE;
  return 1;
}
#endif

static int m3_card_validate_color(const M3Color *color) {
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

static int m3_card_color_set(M3Color *color, M3Scalar r, M3Scalar g, M3Scalar b,
                             M3Scalar a) {
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
  if (m3_card_test_color_should_fail()) {
    return M3_ERR_IO;
  }
#endif
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return M3_OK;
}

static int m3_card_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                    M3Color *out_color) {
  int rc;

  if (base == NULL || out_color == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!(alpha >= 0.0f && alpha <= 1.0f)) {
    return M3_ERR_RANGE;
  }

  rc = m3_card_validate_color(base);
  if (rc != M3_OK) {
    return rc;
  }
#ifdef M3_TESTING
  if (m3_card_test_color_should_fail()) {
    return M3_ERR_IO;
  }
#endif

  *out_color = *base;
  out_color->a = out_color->a * alpha;
  return M3_OK;
}

static int m3_card_validate_edges(const M3LayoutEdges *edges) {
  if (edges == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (edges->left < 0.0f || edges->top < 0.0f || edges->right < 0.0f ||
      edges->bottom < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_card_validate_style(const M3CardStyle *style) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (style->variant) {
  case M3_CARD_VARIANT_ELEVATED:
  case M3_CARD_VARIANT_FILLED:
  case M3_CARD_VARIANT_OUTLINED:
    break;
  default:
    return M3_ERR_RANGE;
  }

  rc = m3_card_validate_edges(&style->padding);
  if (rc != M3_OK) {
    return rc;
  }

  if (style->min_width < 0.0f || style->min_height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->corner_radius < 0.0f || style->outline_width < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->ripple_expand_duration < 0.0f ||
      style->ripple_fade_duration < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (style->shadow_enabled != M3_FALSE && style->shadow_enabled != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  rc = m3_card_validate_color(&style->background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->outline_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->ripple_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(&style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_card_validate_measure_spec(M3MeasureSpec spec) {
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.mode != M3_MEASURE_EXACTLY &&
      spec.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (spec.mode != M3_MEASURE_UNSPECIFIED && spec.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_card_validate_rect(const M3Rect *rect) {
  if (rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_card_measure_content(const M3CardStyle *style,
                                   M3Scalar *out_width, M3Scalar *out_height) {
  M3Scalar required_width;
  M3Scalar required_height;

  if (style == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  required_width =
      style->padding.left + style->padding.right + style->outline_width * 2.0f;
  required_height =
      style->padding.top + style->padding.bottom + style->outline_width * 2.0f;
  if (required_width < 0.0f || required_height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_width = style->min_width;
  if (*out_width < required_width) {
    *out_width = required_width;
  }
  *out_height = style->min_height;
  if (*out_height < required_height) {
    *out_height = required_height;
  }
  return M3_OK;
}

static int m3_card_compute_inner(const M3Card *card, M3Rect *out_inner,
                                 M3Scalar *out_corner) {
  M3Rect inner;
  M3Scalar outline_width;
  M3Scalar corner;
  int rc;

  if (card == NULL || out_inner == NULL || out_corner == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_rect(&card->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  outline_width = card->style.outline_width;
  if (outline_width < 0.0f) {
    return M3_ERR_RANGE;
  }
  corner = card->style.corner_radius;
  if (corner < 0.0f) {
    return M3_ERR_RANGE;
  }

  inner = card->bounds;
  if (outline_width > 0.0f) {
    inner.x += outline_width;
    inner.y += outline_width;
    inner.width -= outline_width * 2.0f;
    inner.height -= outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return M3_ERR_RANGE;
    }
    corner -= outline_width;
    if (corner < 0.0f) {
      corner = 0.0f;
    }
  }

  *out_inner = inner;
  *out_corner = corner;
  return M3_OK;
}

static int m3_card_compute_content_bounds(const M3Card *card,
                                          M3Rect *out_bounds) {
  M3Rect inner;
  M3Scalar inner_corner;
  M3Scalar padding_width;
  M3Scalar padding_height;
  int rc;

  if (card == NULL || out_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(&card->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_compute_inner(card, &inner, &inner_corner);
  if (rc != M3_OK) {
    return rc;
  }

  padding_width = card->style.padding.left + card->style.padding.right;
  padding_height = card->style.padding.top + card->style.padding.bottom;
  inner.x += card->style.padding.left;
  inner.y += card->style.padding.top;
  inner.width -= padding_width;
  inner.height -= padding_height;
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_CONTENT_BOUNDS)) {
    inner.width = -1.0f;
  }
#endif
  if (inner.width < 0.0f || inner.height < 0.0f) {
    return M3_ERR_RANGE;
  }

  *out_bounds = inner;
  return M3_OK;
}

static int m3_card_resolve_colors(const M3Card *card, M3Color *out_background,
                                  M3Color *out_outline, M3Color *out_ripple) {
  int rc;

  if (card == NULL || out_background == NULL || out_outline == NULL ||
      out_ripple == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (card->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    *out_background = card->style.disabled_background_color;
    *out_outline = card->style.disabled_outline_color;
    *out_ripple = card->style.disabled_outline_color;
  } else {
    *out_background = card->style.background_color;
    *out_outline = card->style.outline_color;
    *out_ripple = card->style.ripple_color;
  }

  rc = m3_card_validate_color(out_background);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(out_outline);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_color(out_ripple);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_card_style_init_base(M3CardStyle *style, m3_u32 variant) {
  int rc;

  if (style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(style, 0, sizeof(*style));

  rc = m3_shadow_init(&style->shadow);
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_SHADOW_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
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
  style->shadow_enabled = M3_FALSE;

  rc = m3_card_color_set(&style->background_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->disabled_background_color, 0.0f, 0.0f, 0.0f,
                         0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc =
      m3_card_color_set(&style->disabled_outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_card_style_init_elevated(M3CardStyle *style) {
  M3Color shadow_color;
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_ELEVATED);
  if (rc != M3_OK) {
    return rc;
  }

  style->outline_width = 0.0f;

  rc = m3_card_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_color_set(&shadow_color, 0.0f, 0.0f, 0.0f, 0.3f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_shadow_set(&style->shadow, 0.0f, 2.0f, 6.0f, 0.0f,
                     style->corner_radius, 3, shadow_color);
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_SHADOW_SET)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  style->shadow_enabled = M3_TRUE;

  return M3_OK;
}

int M3_CALL m3_card_style_init_filled(M3CardStyle *style) {
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_FILLED);
  if (rc != M3_OK) {
    return rc;
  }

  style->outline_width = 0.0f;

  rc = m3_card_color_set(&style->background_color, 0.94f, 0.94f, 0.94f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.0f, 0.0f, 0.0f, 0.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

int M3_CALL m3_card_style_init_outlined(M3CardStyle *style) {
  int rc;

  rc = m3_card_style_init_base(style, M3_CARD_VARIANT_OUTLINED);
  if (rc != M3_OK) {
    return rc;
  }

  style->outline_width = M3_CARD_DEFAULT_OUTLINE_WIDTH;

  rc = m3_card_color_set(&style->background_color, 1.0f, 1.0f, 1.0f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->outline_color, 0.75f, 0.75f, 0.75f, 1.0f);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_set(&style->ripple_color, 0.0f, 0.0f, 0.0f, 0.12f);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_color_with_alpha(&style->background_color, 0.12f,
                                &style->disabled_background_color);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_color_with_alpha(&style->outline_color, 0.12f,
                                &style->disabled_outline_color);
  if (rc != M3_OK) {
    return rc;
  }

  return M3_OK;
}

static int m3_card_widget_measure(void *widget, M3MeasureSpec width,
                                  M3MeasureSpec height, M3Size *out_size) {
  M3Card *card;
  M3Scalar desired_width;
  M3Scalar desired_height;
  int rc;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_measure_spec(width);
  if (rc != M3_OK) {
    return rc;
  }
  rc = m3_card_validate_measure_spec(height);
  if (rc != M3_OK) {
    return rc;
  }

  card = (M3Card *)widget;
  rc = m3_card_validate_style(&card->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_measure_content(&card->style, &desired_width, &desired_height);
  if (rc != M3_OK) {
    return rc;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (desired_width > width.size) ? width.size : desired_width;
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

static int m3_card_widget_layout(void *widget, M3Rect bounds) {
  M3Card *card;
  int rc;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_rect(&bounds);
  if (rc != M3_OK) {
    return rc;
  }

  card = (M3Card *)widget;
  card->bounds = bounds;
  return M3_OK;
}

static int m3_card_widget_paint(void *widget, M3PaintContext *ctx) {
  M3Card *card;
  M3Rect bounds;
  M3Rect inner;
  M3Color background;
  M3Color outline;
  M3Color ripple_color;
  M3Shadow shadow;
  M3Scalar corner_radius;
  M3Scalar inner_corner;
  M3Scalar outline_width;
  M3Bool ripple_active;
  int rc;

  if (widget == NULL || ctx == NULL || ctx->gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (ctx->gfx->vtable->draw_rect == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  card = (M3Card *)widget;

  rc = m3_card_validate_style(&card->style);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_validate_rect(&card->bounds);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_card_resolve_colors(card, &background, &outline, &ripple_color);
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RESOLVE_COLORS)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }
  M3_UNUSED(ripple_color);

  corner_radius = card->style.corner_radius;
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  bounds = card->bounds;
  outline_width = card->style.outline_width;
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_OUTLINE_WIDTH)) {
    outline_width = -1.0f;
  }
#endif
  if (outline_width < 0.0f) {
    return M3_ERR_RANGE;
  }

  if (card->style.shadow_enabled == M3_TRUE) {
    shadow = card->style.shadow;
    shadow.corner_radius = corner_radius;
    rc = m3_shadow_paint(&shadow, ctx->gfx, &bounds, &ctx->clip);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (outline_width > 0.0f) {
    inner.x = bounds.x + outline_width;
    inner.y = bounds.y + outline_width;
    inner.width = bounds.width - outline_width * 2.0f;
    inner.height = bounds.height - outline_width * 2.0f;
    if (inner.width < 0.0f || inner.height < 0.0f) {
      return M3_ERR_RANGE;
    }

    rc = ctx->gfx->vtable->draw_rect(ctx->gfx->ctx, &bounds, outline,
                                     corner_radius);
    if (rc != M3_OK) {
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
    if (rc != M3_OK) {
      return rc;
    }
  }

  if ((card->widget.flags & M3_WIDGET_FLAG_DISABLED) == 0) {
    rc = m3_ripple_is_active(&card->ripple, &ripple_active);
    if (rc != M3_OK) {
      return rc;
    }
    if (ripple_active == M3_TRUE) {
      rc = m3_ripple_paint(&card->ripple, ctx->gfx, &inner, inner_corner);
      if (rc != M3_OK) {
        return rc;
      }
    }
  }

  return M3_OK;
}

static int m3_card_widget_event(void *widget, const M3InputEvent *event,
                                M3Bool *out_handled) {
  M3Card *card;
  M3Scalar center_x;
  M3Scalar center_y;
  M3Scalar max_radius;
  int rc;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_handled = M3_FALSE;

  card = (M3Card *)widget;

  if (card->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    return M3_OK;
  }

  switch (event->type) {
  case M3_INPUT_POINTER_DOWN:
    if (card->pressed == M3_TRUE) {
      return M3_ERR_STATE;
    }
    if (card->style.ripple_expand_duration < 0.0f) {
      return M3_ERR_RANGE;
    }
    rc = m3_card_validate_color(&card->style.ripple_color);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_card_validate_rect(&card->bounds);
    if (rc != M3_OK) {
      return rc;
    }
    center_x = (M3Scalar)event->data.pointer.x;
    center_y = (M3Scalar)event->data.pointer.y;
    rc = m3_ripple_compute_max_radius(&card->bounds, center_x, center_y,
                                      &max_radius);
#ifdef M3_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_RADIUS)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_ripple_start(&card->ripple, center_x, center_y, max_radius,
                         card->style.ripple_expand_duration,
                         card->style.ripple_color);
#ifdef M3_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_START)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK) {
      return rc;
    }
    card->pressed = M3_TRUE;
    *out_handled = M3_TRUE;
    return M3_OK;
  case M3_INPUT_POINTER_UP:
    if (card->pressed == M3_FALSE) {
      return M3_OK;
    }
    if (card->style.ripple_fade_duration < 0.0f) {
      return M3_ERR_RANGE;
    }
    card->pressed = M3_FALSE;
    rc = m3_ripple_release(&card->ripple, card->style.ripple_fade_duration);
#ifdef M3_TESTING
    if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_RELEASE)) {
      rc = M3_ERR_IO;
    }
#endif
    if (rc != M3_OK && rc != M3_ERR_STATE) {
      return rc;
    }
    if (card->on_click != NULL) {
      rc = card->on_click(card->on_click_ctx, card);
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

static int m3_card_widget_get_semantics(void *widget,
                                        M3Semantics *out_semantics) {
  M3Card *card;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  card = (M3Card *)widget;
  out_semantics->role = M3_SEMANTIC_BUTTON;
  out_semantics->flags = 0;
  if (card->widget.flags & M3_WIDGET_FLAG_DISABLED) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_DISABLED;
  }
  if (card->widget.flags & M3_WIDGET_FLAG_FOCUSABLE) {
    out_semantics->flags |= M3_SEMANTIC_FLAG_FOCUSABLE;
  }
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int m3_card_widget_destroy(void *widget) {
  M3Card *card;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
  card->pressed = M3_FALSE;
  card->on_click = NULL;
  card->on_click_ctx = NULL;
  memset(&card->ripple, 0, sizeof(card->ripple));
  return M3_OK;
}

static const M3WidgetVTable g_m3_card_widget_vtable = {
    m3_card_widget_measure,       m3_card_widget_layout,
    m3_card_widget_paint,         m3_card_widget_event,
    m3_card_widget_get_semantics, m3_card_widget_destroy};

int M3_CALL m3_card_init(M3Card *card, const M3CardStyle *style) {
  int rc;

  if (card == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  memset(card, 0, sizeof(*card));
  card->style = *style;
  card->pressed = M3_FALSE;

  rc = m3_ripple_init(&card->ripple);
#ifdef M3_TESTING
  if (m3_card_test_fail_point_match(M3_CARD_TEST_FAIL_RIPPLE_INIT)) {
    rc = M3_ERR_IO;
  }
#endif
  if (rc != M3_OK) {
    return rc;
  }

  card->widget.ctx = card;
  card->widget.vtable = &g_m3_card_widget_vtable;
  card->widget.handle.id = 0u;
  card->widget.handle.generation = 0u;
  card->widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
  return M3_OK;
}

int M3_CALL m3_card_set_style(M3Card *card, const M3CardStyle *style) {
  int rc;

  if (card == NULL || style == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_card_validate_style(style);
  if (rc != M3_OK) {
    return rc;
  }

  card->style = *style;
  return M3_OK;
}

int M3_CALL m3_card_set_on_click(M3Card *card, M3CardOnClick on_click,
                                 void *ctx) {
  if (card == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  card->on_click = on_click;
  card->on_click_ctx = ctx;
  return M3_OK;
}

int M3_CALL m3_card_get_content_bounds(const M3Card *card, M3Rect *out_bounds) {
  return m3_card_compute_content_bounds(card, out_bounds);
}

#ifdef M3_TESTING
int M3_CALL m3_card_test_validate_color(const M3Color *color) {
  return m3_card_validate_color(color);
}

int M3_CALL m3_card_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                   M3Scalar b, M3Scalar a) {
  return m3_card_color_set(color, r, g, b, a);
}

int M3_CALL m3_card_test_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                          M3Color *out_color) {
  return m3_card_color_with_alpha(base, alpha, out_color);
}

int M3_CALL m3_card_test_validate_edges(const M3LayoutEdges *edges) {
  return m3_card_validate_edges(edges);
}

int M3_CALL m3_card_test_validate_style(const M3CardStyle *style) {
  return m3_card_validate_style(style);
}

int M3_CALL m3_card_test_validate_measure_spec(M3MeasureSpec spec) {
  return m3_card_validate_measure_spec(spec);
}

int M3_CALL m3_card_test_validate_rect(const M3Rect *rect) {
  return m3_card_validate_rect(rect);
}

int M3_CALL m3_card_test_compute_content_bounds(const M3Card *card,
                                                M3Rect *out_bounds) {
  return m3_card_compute_content_bounds(card, out_bounds);
}

int M3_CALL m3_card_test_resolve_colors(const M3Card *card,
                                        M3Color *out_background,
                                        M3Color *out_outline,
                                        M3Color *out_ripple) {
  return m3_card_resolve_colors(card, out_background, out_outline, out_ripple);
}
#endif
