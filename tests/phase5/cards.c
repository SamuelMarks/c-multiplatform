#include "m3/m3_card.h"
#include "test_utils.h"

#include <string.h>

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

int M3_CALL m3_card_test_set_fail_point(m3_u32 fail_point);
int M3_CALL m3_card_test_set_color_fail_after(m3_u32 call_count);
int M3_CALL m3_card_test_clear_fail_points(void);
int M3_CALL m3_card_test_validate_color(const M3Color *color);
int M3_CALL m3_card_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                   M3Scalar b, M3Scalar a);
int M3_CALL m3_card_test_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                          M3Color *out_color);
int M3_CALL m3_card_test_validate_edges(const M3LayoutEdges *edges);
int M3_CALL m3_card_test_validate_style(const M3CardStyle *style);
int M3_CALL m3_card_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_card_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_card_test_compute_content_bounds(const M3Card *card,
                                                M3Rect *out_bounds);
int M3_CALL m3_card_test_resolve_colors(const M3Card *card,
                                        M3Color *out_background,
                                        M3Color *out_outline,
                                        M3Color *out_ripple);

typedef struct TestCardBackend {
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_draw_rect;
  int fail_draw_rect_after;
  int fail_push_clip;
  int fail_pop_clip;
  M3Rect last_rect;
  M3Color last_color;
  M3Scalar last_corner;
} TestCardBackend;

typedef struct ClickCounter {
  int calls;
  int fail;
} ClickCounter;

static int test_backend_init(TestCardBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = M3_OK;
  backend->fail_draw_rect_after = 0;
  backend->fail_push_clip = M3_OK;
  backend->fail_pop_clip = M3_OK;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestCardBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != M3_OK) {
    return backend->fail_draw_rect;
  }
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls >= backend->fail_draw_rect_after) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_push_clip(void *gfx, const M3Rect *rect) {
  TestCardBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip != M3_OK) {
    return backend->fail_push_clip;
  }
  return M3_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestCardBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip != M3_OK) {
    return backend->fail_pop_clip;
  }
  return M3_OK;
}

static const M3GfxVTable g_test_gfx_vtable = {NULL,
                                              NULL,
                                              NULL,
                                              test_gfx_draw_rect,
                                              NULL,
                                              NULL,
                                              test_gfx_push_clip,
                                              test_gfx_pop_clip,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL};

static const M3GfxVTable g_test_gfx_vtable_no_rect = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_gfx_vtable_no_clip = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                              m3_i32 y) {
  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return M3_OK;
}

static int test_card_on_click(void *ctx, struct M3Card *card) {
  ClickCounter *counter;

  if (ctx == NULL || card == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ClickCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_card_helpers(void) {
  M3Color color;
  M3Color out_color;
  M3LayoutEdges edges;
  M3CardStyle style;
  M3CardStyle bad_style;
  M3MeasureSpec spec;
  M3Rect rect;
  M3Card card;
  M3CardStyle card_style;
  M3Rect content;
  M3Color background;
  M3Color outline;
  M3Color ripple;
  volatile const M3Color *null_color;
  volatile const M3LayoutEdges *null_edges;
  volatile const M3Rect *null_rect;

  null_color = NULL;
  null_edges = NULL;
  null_rect = NULL;

  M3_TEST_EXPECT(m3_card_test_validate_color((const M3Color *)null_color),
                 M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_card_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  M3_TEST_EXPECT(m3_card_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  M3_TEST_EXPECT(m3_card_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  M3_TEST_EXPECT(m3_card_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 0.0f;
  M3_TEST_OK(m3_card_test_validate_color(&color));

  M3_TEST_EXPECT(m3_card_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));
  M3_TEST_ASSERT(m3_near(color.r, 0.1f, 0.0001f));
  M3_TEST_ASSERT(m3_near(color.a, 0.4f, 0.0001f));

  M3_TEST_OK(m3_card_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_EXPECT(m3_card_test_color_with_alpha(NULL, 0.5f, &out_color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_test_color_with_alpha(&color, -0.1f, &out_color),
                 M3_ERR_RANGE);
  color.r = -0.1f;
  M3_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, &out_color),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_test_color_set(&color, 0.2f, 0.3f, 0.4f, 0.5f));
  M3_TEST_OK(m3_card_test_color_with_alpha(&color, 0.5f, &out_color));
  M3_TEST_ASSERT(m3_near(out_color.a, 0.25f, 0.0001f));

  M3_TEST_OK(m3_card_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, &out_color),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_EXPECT(m3_card_test_validate_edges((const M3LayoutEdges *)null_edges),
                 M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_card_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 0.0f;
  edges.top = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.top = 0.0f;
  edges.right = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.right = 0.0f;
  edges.bottom = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.bottom = 0.0f;
  M3_TEST_OK(m3_card_test_validate_edges(&edges));

  M3_TEST_OK(m3_card_style_init_filled(&style));
  M3_TEST_OK(m3_card_test_validate_style(&style));
  bad_style = style;
  bad_style.variant = 0u;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.min_width = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.outline_width = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_expand_duration = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.shadow_enabled = 2;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.outline_color.g = 2.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_color.b = 2.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.disabled_background_color.a = 2.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.disabled_outline_color.r = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_style(&bad_style), M3_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_card_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  M3_TEST_OK(m3_card_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_card_test_validate_rect((const M3Rect *)null_rect),
                 M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 0.0f;
  M3_TEST_EXPECT(m3_card_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  M3_TEST_EXPECT(m3_card_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.height = 1.0f;
  M3_TEST_OK(m3_card_test_validate_rect(&rect));

  M3_TEST_OK(m3_card_style_init_filled(&card_style));
  M3_TEST_OK(m3_card_init(&card, &card_style));
  card.bounds.x = 0.0f;
  card.bounds.y = 0.0f;
  card.bounds.width = 100.0f;
  card.bounds.height = 80.0f;
  M3_TEST_OK(m3_card_test_compute_content_bounds(&card, &content));
  M3_TEST_ASSERT(m3_near(content.width, 68.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(content.height, 48.0f, 0.001f));

  card.style.corner_radius = 0.5f;
  card.style.outline_width = 1.0f;
  card.bounds.width = 120.0f;
  card.bounds.height = 90.0f;
  M3_TEST_OK(m3_card_test_compute_content_bounds(&card, &content));
  card.style.corner_radius = card_style.corner_radius;
  card.style.outline_width = card_style.outline_width;

  card.bounds.width = 10.0f;
  card.bounds.height = 10.0f;
  M3_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                 M3_ERR_RANGE);
  card.bounds.width = 100.0f;
  card.bounds.height = 80.0f;

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_CONTENT_BOUNDS));
  M3_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple));
  card.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple));
  card.widget.flags = 0u;

  card.style.background_color.r = -1.0f;
  M3_TEST_EXPECT(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple),
      M3_ERR_RANGE);
  card.style.background_color.r = 1.0f;

  return 0;
}

int main(void) {
  TestCardBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3CardStyle elevated_style;
  M3CardStyle filled_style;
  M3CardStyle outlined_style;
  M3CardStyle bad_style;
  M3Card card;
  M3Card paint_card;
  M3Card shadow_card;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3Rect content_bounds;
  M3Semantics semantics;
  M3Bool handled;
  ClickCounter counter;
  M3InputEvent event;

  M3_TEST_OK(test_card_helpers());

  M3_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 200.0f;
  paint_ctx.clip.height = 200.0f;

  M3_TEST_EXPECT(m3_card_style_init_elevated(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_style_init_filled(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_style_init_outlined(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_SHADOW_INIT));
  M3_TEST_EXPECT(m3_card_style_init_elevated(&elevated_style), M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_card_style_init_filled(&filled_style), M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_SHADOW_SET));
  M3_TEST_EXPECT(m3_card_style_init_elevated(&elevated_style), M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_style_init_elevated(&elevated_style));
  M3_TEST_OK(m3_card_style_init_filled(&filled_style));
  M3_TEST_OK(m3_card_style_init_outlined(&outlined_style));
  M3_TEST_ASSERT(elevated_style.variant == M3_CARD_VARIANT_ELEVATED);
  M3_TEST_ASSERT(m3_near(elevated_style.corner_radius,
                         M3_CARD_DEFAULT_CORNER_RADIUS, 0.001f));
  M3_TEST_ASSERT(elevated_style.shadow_enabled == M3_TRUE);
  M3_TEST_ASSERT(filled_style.outline_width == 0.0f);
  M3_TEST_ASSERT(outlined_style.outline_width == M3_CARD_DEFAULT_OUTLINE_WIDTH);

  M3_TEST_EXPECT(m3_card_init(NULL, &filled_style), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_init(&card, NULL), M3_ERR_INVALID_ARGUMENT);
  bad_style = filled_style;
  bad_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_card_init(&card, &bad_style), M3_ERR_RANGE);

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_INIT));
  M3_TEST_EXPECT(m3_card_init(&card, &filled_style), M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_init(&card, &filled_style));
  M3_TEST_ASSERT(card.widget.ctx == &card);
  M3_TEST_ASSERT(card.widget.vtable != NULL);

  M3_TEST_EXPECT(m3_card_set_style(NULL, &filled_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_set_style(&card, NULL), M3_ERR_INVALID_ARGUMENT);
  bad_style = filled_style;
  bad_style.outline_width = -1.0f;
  M3_TEST_EXPECT(m3_card_set_style(&card, &bad_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_card_set_style(&card, &outlined_style));

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_EXPECT(m3_card_set_on_click(NULL, test_card_on_click, &counter),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_card_set_on_click(&card, test_card_on_click, &counter));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 80.0f;
  card.bounds = bounds;

  M3_TEST_EXPECT(m3_card_get_content_bounds(NULL, &content_bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_card_get_content_bounds(&card, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_card_set_style(&card, &filled_style));
  M3_TEST_OK(m3_card_get_content_bounds(&card, &content_bounds));
  M3_TEST_ASSERT(m3_near(content_bounds.width, 68.0f, 0.001f));

  bad_style = filled_style;
  bad_style.padding.left = 80.0f;
  bad_style.padding.right = 80.0f;
  M3_TEST_OK(m3_card_set_style(&card, &bad_style));
  M3_TEST_EXPECT(m3_card_get_content_bounds(&card, &content_bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_set_style(&card, &filled_style));

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_CONTENT_BOUNDS));
  M3_TEST_EXPECT(m3_card_get_content_bounds(&card, &content_bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                         height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, M3_CARD_DEFAULT_MIN_WIDTH, 0.001f));

  bad_style = filled_style;
  bad_style.min_width = 10.0f;
  bad_style.min_height = 5.0f;
  bad_style.padding.left = 40.0f;
  bad_style.padding.right = 40.0f;
  bad_style.padding.top = 10.0f;
  bad_style.padding.bottom = 10.0f;
  M3_TEST_OK(m3_card_set_style(&card, &bad_style));
  M3_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                         height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 80.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 20.0f, 0.001f));
  M3_TEST_OK(m3_card_set_style(&card, &filled_style));

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 120.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 50.0f;
  M3_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                         height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 120.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 50.0f, 0.001f));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 30.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 44.0f;
  M3_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                         height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 30.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 44.0f, 0.001f));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  M3_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                             height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(
      card.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                             height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  bounds.width = -1.0f;
  M3_TEST_EXPECT(card.widget.vtable->layout(card.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 100.0f;
  bounds.height = 80.0f;
  M3_TEST_OK(card.widget.vtable->layout(card.widget.ctx, bounds));

  M3_TEST_EXPECT(card.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(card.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RESOLVE_COLORS));
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_OUTLINE_WIDTH));
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = M3_OK;

  M3_TEST_OK(m3_card_init(&shadow_card, &elevated_style));
  shadow_card.bounds = bounds;
  gfx.vtable = &g_test_gfx_vtable_no_clip;
  M3_TEST_EXPECT(
      shadow_card.widget.vtable->paint(shadow_card.widget.ctx, &paint_ctx),
      M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  M3_TEST_OK(m3_card_init(&paint_card, &filled_style));
  paint_card.bounds = bounds;
  M3_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));

  M3_TEST_OK(m3_card_init(&paint_card, &outlined_style));
  paint_card.bounds = bounds;
  M3_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));

  paint_card.style.outline_width = 60.0f;
  M3_TEST_EXPECT(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx),
      M3_ERR_RANGE);
  paint_card.style.outline_width = outlined_style.outline_width;

  paint_card.style.background_color.a = 0.0f;
  M3_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  handled = M3_FALSE;
  M3_TEST_OK(
      paint_card.widget.vtable->event(paint_card.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  gfx.vtable = &g_test_gfx_vtable_no_clip;
  M3_TEST_EXPECT(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx),
      M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  M3_TEST_OK(m3_card_init(&card, &filled_style));
  card.bounds = bounds;
  M3_TEST_EXPECT(card.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  card.widget.flags = M3_WIDGET_FLAG_DISABLED;
  handled = M3_TRUE;
  M3_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  card.widget.flags = M3_WIDGET_FLAG_FOCUSABLE;

  card.pressed = M3_TRUE;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_STATE);
  card.pressed = M3_FALSE;

  card.style.ripple_expand_duration = -1.0f;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  card.style.ripple_expand_duration = filled_style.ripple_expand_duration;

  card.style.ripple_color.r = -1.0f;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  card.style.ripple_color.r = filled_style.ripple_color.r;

  card.bounds.width = -1.0f;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  card.bounds.width = bounds.width;

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_RADIUS));
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_START));
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  M3_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_DOWN, 0, 0));
  M3_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  card.pressed = M3_FALSE;
  handled = M3_TRUE;
  M3_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  card.pressed = M3_TRUE;
  card.style.ripple_fade_duration = -1.0f;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  card.style.ripple_fade_duration = filled_style.ripple_fade_duration;

  card.pressed = M3_TRUE;
  M3_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_RELEASE));
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_card_test_clear_fail_points());

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_card_set_on_click(&card, test_card_on_click, &counter));
  card.pressed = M3_TRUE;
  card.ripple.state = M3_RIPPLE_STATE_IDLE;
  M3_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(counter.calls == 1);

  counter.fail = 1;
  card.pressed = M3_TRUE;
  card.ripple.state = M3_RIPPLE_STATE_IDLE;
  M3_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                 M3_ERR_IO);

  M3_TEST_EXPECT(card.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(card.widget.vtable->get_semantics(card.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(card.widget.vtable->get_semantics(card.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_BUTTON);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_FOCUSABLE) != 0u);

  card.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(card.widget.vtable->get_semantics(card.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);

  M3_TEST_EXPECT(card.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(card.widget.vtable->destroy(card.widget.ctx));
  M3_TEST_ASSERT(card.widget.vtable == NULL);

  return 0;
}
