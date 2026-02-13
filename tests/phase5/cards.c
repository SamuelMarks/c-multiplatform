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
#define M3_CARD_TEST_FAIL_MEASURE_CONTENT 10u
#define M3_CARD_TEST_FAIL_CORNER_RADIUS 11u

int CMP_CALL m3_card_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL m3_card_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_card_test_clear_fail_points(void);
int CMP_CALL m3_card_test_validate_color(const CMPColor *color);
int CMP_CALL m3_card_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a);
int CMP_CALL m3_card_test_color_with_alpha(const CMPColor *base,
                                           CMPScalar alpha,
                                           CMPColor *out_color);
int CMP_CALL m3_card_test_validate_edges(const CMPLayoutEdges *edges);
int CMP_CALL m3_card_test_validate_style(const M3CardStyle *style);
int CMP_CALL m3_card_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_card_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_card_test_measure_content(const M3CardStyle *style,
                                          CMPScalar *out_width,
                                          CMPScalar *out_height);
int CMP_CALL m3_card_test_compute_inner(const M3Card *card, CMPRect *out_inner,
                                        CMPScalar *out_corner);
int CMP_CALL m3_card_test_compute_content_bounds(const M3Card *card,
                                                 CMPRect *out_bounds);
int CMP_CALL m3_card_test_resolve_colors(const M3Card *card,
                                         CMPColor *out_background,
                                         CMPColor *out_outline,
                                         CMPColor *out_ripple);

typedef struct TestCardBackend {
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_draw_rect;
  int fail_draw_rect_after;
  int fail_push_clip;
  int fail_pop_clip;
  CMPRect last_rect;
  CMPColor last_color;
  CMPScalar last_corner;
} TestCardBackend;

typedef struct ClickCounter {
  int calls;
  int fail;
} ClickCounter;

static int test_backend_init(TestCardBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = CMP_OK;
  backend->fail_draw_rect_after = 0;
  backend->fail_push_clip = CMP_OK;
  backend->fail_pop_clip = CMP_OK;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestCardBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect != CMP_OK) {
    return backend->fail_draw_rect;
  }
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls >= backend->fail_draw_rect_after) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestCardBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip != CMP_OK) {
    return backend->fail_push_clip;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestCardBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestCardBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip != CMP_OK) {
    return backend->fail_pop_clip;
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {NULL,
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

static const CMPGfxVTable g_test_gfx_vtable_no_rect = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_clip = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                              cmp_i32 y) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return CMP_OK;
}

static int test_card_on_click(void *ctx, struct M3Card *card) {
  ClickCounter *counter;

  if (ctx == NULL || card == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (ClickCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_card_helpers(void) {
  CMPColor color;
  CMPColor out_color;
  CMPLayoutEdges edges;
  M3CardStyle style;
  M3CardStyle bad_style;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3Card card;
  M3CardStyle card_style;
  CMPRect content;
  CMPRect inner;
  CMPColor background;
  CMPColor outline;
  CMPColor ripple;
  CMPScalar content_width;
  CMPScalar content_height;
  CMPScalar inner_corner;
  volatile const CMPColor *null_color;
  volatile const CMPLayoutEdges *null_edges;
  volatile const CMPRect *null_rect;

  null_color = NULL;
  null_edges = NULL;
  null_rect = NULL;

  CMP_TEST_EXPECT(m3_card_test_validate_color((const CMPColor *)null_color),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(m3_card_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(m3_card_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(m3_card_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 0.0f;
  CMP_TEST_OK(m3_card_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_card_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));
  CMP_TEST_ASSERT(cmp_near(color.r, 0.1f, 0.0001f));
  CMP_TEST_ASSERT(cmp_near(color.a, 0.4f, 0.0001f));

  CMP_TEST_OK(m3_card_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_card_test_color_set(&color, 0.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_card_test_color_with_alpha(NULL, 0.5f, &out_color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_color_with_alpha(&color, -0.1f, &out_color),
                  CMP_ERR_RANGE);
  color.r = -0.1f;
  CMP_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, &out_color),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_color_set(&color, 0.2f, 0.3f, 0.4f, 0.5f));
  CMP_TEST_OK(m3_card_test_color_with_alpha(&color, 0.5f, &out_color));
  CMP_TEST_ASSERT(cmp_near(out_color.a, 0.25f, 0.0001f));

  CMP_TEST_OK(m3_card_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_card_test_color_with_alpha(&color, 0.5f, &out_color),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_EXPECT(
      m3_card_test_validate_edges((const CMPLayoutEdges *)null_edges),
      CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_card_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_card_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_card_style_init_filled(&style));
  CMP_TEST_OK(m3_card_test_validate_style(&style));
  bad_style = style;
  bad_style.variant = 0u;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.outline_width = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_expand_duration = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.shadow_enabled = 2;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.outline_color.g = 2.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_color.b = 2.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.disabled_background_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.disabled_outline_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_style(&bad_style), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_card_test_measure_content(NULL, &content_width, &content_height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_measure_content(&style, NULL, &content_height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_measure_content(&style, &content_width, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.padding.left = -1.0f;
  bad_style.padding.right = 0.0f;
  bad_style.outline_width = 0.0f;
  CMP_TEST_EXPECT(
      m3_card_test_measure_content(&bad_style, &content_width, &content_height),
      CMP_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_OK(m3_card_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_card_test_validate_rect((const CMPRect *)null_rect),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 0.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_card_test_validate_rect(&rect));

  CMP_TEST_OK(m3_card_style_init_filled(&card_style));
  CMP_TEST_OK(m3_card_init(&card, &card_style));
  CMP_TEST_EXPECT(m3_card_test_compute_inner(NULL, &inner, &inner_corner),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, NULL, &inner_corner),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, &inner, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  card.bounds.x = 0.0f;
  card.bounds.y = 0.0f;
  card.bounds.width = 100.0f;
  card.bounds.height = 80.0f;
  card.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, &inner, &inner_corner),
                  CMP_ERR_RANGE);
  card.bounds.width = 100.0f;
  card.style.outline_width = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, &inner, &inner_corner),
                  CMP_ERR_RANGE);
  card.style.outline_width = card_style.outline_width;
  card.style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, &inner, &inner_corner),
                  CMP_ERR_RANGE);
  card.style.corner_radius = card_style.corner_radius;
  card.style.outline_width = 6.0f;
  card.bounds.width = 10.0f;
  card.bounds.height = 10.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_inner(&card, &inner, &inner_corner),
                  CMP_ERR_RANGE);
  card.style.outline_width = card_style.outline_width;
  card.bounds.width = 100.0f;
  card.bounds.height = 80.0f;
  CMP_TEST_OK(m3_card_test_compute_content_bounds(&card, &content));
  CMP_TEST_ASSERT(cmp_near(content.width, 68.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(content.height, 48.0f, 0.001f));

  bad_style = card.style;
  bad_style.variant = 0u;
  card.style = bad_style;
  CMP_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                  CMP_ERR_RANGE);
  card.style = card_style;

  card.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                  CMP_ERR_RANGE);
  card.bounds.width = 100.0f;

  card.style.corner_radius = 0.5f;
  card.style.outline_width = 1.0f;
  card.bounds.width = 120.0f;
  card.bounds.height = 90.0f;
  CMP_TEST_OK(m3_card_test_compute_content_bounds(&card, &content));
  card.style.corner_radius = card_style.corner_radius;
  card.style.outline_width = card_style.outline_width;

  card.bounds.width = 10.0f;
  card.bounds.height = 10.0f;
  CMP_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                  CMP_ERR_RANGE);
  card.bounds.width = 100.0f;
  card.bounds.height = 80.0f;

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_CONTENT_BOUNDS));
  CMP_TEST_EXPECT(m3_card_test_compute_content_bounds(&card, &content),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple));
  card.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple));
  card.widget.flags = 0u;

  CMP_TEST_EXPECT(
      m3_card_test_resolve_colors(NULL, &background, &outline, &ripple),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_test_resolve_colors(&card, NULL, &outline, &ripple),
                  CMP_ERR_INVALID_ARGUMENT);

  card.style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple),
      CMP_ERR_RANGE);
  card.style.background_color.r = 1.0f;

  card.style.outline_color.g = 2.0f;
  CMP_TEST_EXPECT(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple),
      CMP_ERR_RANGE);
  card.style.outline_color.g = card_style.outline_color.g;

  card.style.ripple_color.b = 2.0f;
  CMP_TEST_EXPECT(
      m3_card_test_resolve_colors(&card, &background, &outline, &ripple),
      CMP_ERR_RANGE);
  card.style.ripple_color.b = card_style.ripple_color.b;

  return 0;
}

static int test_card_style_init_failures(void) {
  M3CardStyle style;
  cmp_u32 fail_after;

  for (fail_after = 1u; fail_after <= 11u; ++fail_after) {
    CMP_TEST_OK(m3_card_test_set_color_fail_after(fail_after));
    CMP_TEST_EXPECT(m3_card_style_init_elevated(&style), CMP_ERR_IO);
    CMP_TEST_OK(m3_card_test_clear_fail_points());
  }

  return 0;
}

static int test_card_style_init_filled_failures(void) {
  M3CardStyle style;
  cmp_u32 fail_after;

  for (fail_after = 6u; fail_after <= 10u; ++fail_after) {
    CMP_TEST_OK(m3_card_test_set_color_fail_after(fail_after));
    CMP_TEST_EXPECT(m3_card_style_init_filled(&style), CMP_ERR_IO);
    CMP_TEST_OK(m3_card_test_clear_fail_points());
  }

  return 0;
}

static int test_card_style_init_outlined_failures(void) {
  M3CardStyle style;
  cmp_u32 fail_after;

  for (fail_after = 6u; fail_after <= 10u; ++fail_after) {
    CMP_TEST_OK(m3_card_test_set_color_fail_after(fail_after));
    CMP_TEST_EXPECT(m3_card_style_init_outlined(&style), CMP_ERR_IO);
    CMP_TEST_OK(m3_card_test_clear_fail_points());
  }

  return 0;
}

int main(void) {
  TestCardBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3CardStyle elevated_style;
  M3CardStyle filled_style;
  M3CardStyle outlined_style;
  M3CardStyle bad_style;
  M3Card card;
  M3Card paint_card;
  M3Card shadow_card;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPRect content_bounds;
  CMPSemantics semantics;
  CMPBool handled;
  ClickCounter counter;
  CMPInputEvent event;

  CMP_TEST_OK(test_card_helpers());
  CMP_TEST_OK(test_card_style_init_failures());
  CMP_TEST_OK(test_card_style_init_filled_failures());
  CMP_TEST_OK(test_card_style_init_outlined_failures());

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 200.0f;
  paint_ctx.clip.height = 200.0f;

  CMP_TEST_EXPECT(m3_card_style_init_elevated(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_style_init_filled(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_style_init_outlined(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_SHADOW_INIT));
  CMP_TEST_EXPECT(m3_card_style_init_elevated(&elevated_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_card_style_init_filled(&filled_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_SHADOW_SET));
  CMP_TEST_EXPECT(m3_card_style_init_elevated(&elevated_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_style_init_elevated(&elevated_style));
  CMP_TEST_OK(m3_card_style_init_filled(&filled_style));
  CMP_TEST_OK(m3_card_style_init_outlined(&outlined_style));
  CMP_TEST_ASSERT(elevated_style.variant == M3_CARD_VARIANT_ELEVATED);
  CMP_TEST_ASSERT(cmp_near(elevated_style.corner_radius,
                           M3_CARD_DEFAULT_CORNER_RADIUS, 0.001f));
  CMP_TEST_ASSERT(elevated_style.shadow_enabled == CMP_TRUE);
  CMP_TEST_ASSERT(filled_style.outline_width == 0.0f);
  CMP_TEST_ASSERT(outlined_style.outline_width ==
                  M3_CARD_DEFAULT_OUTLINE_WIDTH);

  CMP_TEST_EXPECT(m3_card_init(NULL, &filled_style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_init(&card, NULL), CMP_ERR_INVALID_ARGUMENT);
  bad_style = filled_style;
  bad_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_card_init(&card, &bad_style), CMP_ERR_RANGE);

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_INIT));
  CMP_TEST_EXPECT(m3_card_init(&card, &filled_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_init(&card, &filled_style));
  CMP_TEST_ASSERT(card.widget.ctx == &card);
  CMP_TEST_ASSERT(card.widget.vtable != NULL);

  CMP_TEST_EXPECT(m3_card_set_style(NULL, &filled_style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_set_style(&card, NULL), CMP_ERR_INVALID_ARGUMENT);
  bad_style = filled_style;
  bad_style.outline_width = -1.0f;
  CMP_TEST_EXPECT(m3_card_set_style(&card, &bad_style), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_set_style(&card, &outlined_style));

  counter.calls = 0;
  counter.fail = 0;
  CMP_TEST_EXPECT(m3_card_set_on_click(NULL, test_card_on_click, &counter),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_card_set_on_click(&card, test_card_on_click, &counter));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 80.0f;
  card.bounds = bounds;

  CMP_TEST_EXPECT(m3_card_get_content_bounds(NULL, &content_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_card_get_content_bounds(&card, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_card_set_style(&card, &filled_style));
  CMP_TEST_OK(m3_card_get_content_bounds(&card, &content_bounds));
  CMP_TEST_ASSERT(cmp_near(content_bounds.width, 68.0f, 0.001f));

  bad_style = filled_style;
  bad_style.padding.left = 80.0f;
  bad_style.padding.right = 80.0f;
  CMP_TEST_OK(m3_card_set_style(&card, &bad_style));
  CMP_TEST_EXPECT(m3_card_get_content_bounds(&card, &content_bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_set_style(&card, &filled_style));

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_CONTENT_BOUNDS));
  CMP_TEST_EXPECT(m3_card_get_content_bounds(&card, &content_bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, M3_CARD_DEFAULT_MIN_WIDTH, 0.001f));

  bad_style = filled_style;
  bad_style.min_width = 10.0f;
  bad_style.min_height = 5.0f;
  bad_style.padding.left = 40.0f;
  bad_style.padding.right = 40.0f;
  bad_style.padding.top = 10.0f;
  bad_style.padding.bottom = 10.0f;
  CMP_TEST_OK(m3_card_set_style(&card, &bad_style));
  CMP_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 80.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 20.0f, 0.001f));
  CMP_TEST_OK(m3_card_set_style(&card, &filled_style));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 120.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 50.0f;
  CMP_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 120.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 50.0f, 0.001f));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 30.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 44.0f;
  CMP_TEST_OK(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 30.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 44.0f, 0.001f));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  CMP_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 10.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  card.style.outline_width = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_RANGE);
  card.style = filled_style;

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_MEASURE_CONTENT));
  CMP_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_EXPECT(
      card.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(card.widget.vtable->measure(card.widget.ctx, width_spec,
                                              height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  bounds.width = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->layout(card.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 100.0f;
  bounds.height = 80.0f;
  CMP_TEST_OK(card.widget.vtable->layout(card.widget.ctx, bounds));

  CMP_TEST_EXPECT(card.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(card.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RESOLVE_COLORS));
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_OUTLINE_WIDTH));
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  card.style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  card.style = filled_style;

  card.bounds.width = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  card.bounds = bounds;

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_CORNER_RADIUS));
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  card.style = outlined_style;
  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;
  card.style = filled_style;

  card.style.corner_radius = 5.0f;
  card.style.outline_width = 20.0f;
  CMP_TEST_OK(card.widget.vtable->paint(card.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(cmp_near(backend.last_corner, 0.0f, 0.001f));
  card.style = filled_style;

  card.ripple.state = 99;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  card.ripple.state = CMP_RIPPLE_STATE_IDLE;

  backend.fail_draw_rect = CMP_ERR_IO;
  CMP_TEST_EXPECT(card.widget.vtable->paint(card.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = CMP_OK;

  CMP_TEST_OK(m3_card_init(&shadow_card, &elevated_style));
  shadow_card.bounds = bounds;
  gfx.vtable = &g_test_gfx_vtable_no_clip;
  CMP_TEST_EXPECT(
      shadow_card.widget.vtable->paint(shadow_card.widget.ctx, &paint_ctx),
      CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;
  CMP_TEST_OK(
      shadow_card.widget.vtable->paint(shadow_card.widget.ctx, &paint_ctx));

  CMP_TEST_OK(m3_card_init(&paint_card, &filled_style));
  paint_card.bounds = bounds;
  CMP_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));
  paint_card.ripple.state = CMP_RIPPLE_STATE_EXPANDING;
  paint_card.ripple.opacity = 0.5f;
  paint_card.ripple.radius = 8.0f;
  paint_card.ripple.center_x = bounds.x + 10.0f;
  paint_card.ripple.center_y = bounds.y + 10.0f;
  paint_card.ripple.color = paint_card.style.ripple_color;
  CMP_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));
  paint_card.ripple.state = CMP_RIPPLE_STATE_IDLE;
  paint_card.ripple.opacity = 0.0f;
  paint_card.ripple.radius = 0.0f;

  CMP_TEST_OK(m3_card_init(&paint_card, &outlined_style));
  paint_card.bounds = bounds;
  CMP_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));

  paint_card.style.outline_width = 60.0f;
  CMP_TEST_EXPECT(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx),
      CMP_ERR_RANGE);
  paint_card.style.outline_width = outlined_style.outline_width;

  paint_card.style.background_color.a = 0.0f;
  CMP_TEST_OK(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  handled = CMP_FALSE;
  CMP_TEST_OK(
      paint_card.widget.vtable->event(paint_card.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  gfx.vtable = &g_test_gfx_vtable_no_clip;
  CMP_TEST_EXPECT(
      paint_card.widget.vtable->paint(paint_card.widget.ctx, &paint_ctx),
      CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  CMP_TEST_OK(m3_card_init(&card, &filled_style));
  card.bounds = bounds;
  CMP_TEST_EXPECT(card.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  card.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  handled = CMP_TRUE;
  CMP_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  card.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  card.pressed = CMP_TRUE;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_STATE);
  card.pressed = CMP_FALSE;

  card.style.ripple_expand_duration = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  card.style.ripple_expand_duration = filled_style.ripple_expand_duration;

  card.style.ripple_color.r = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  card.style.ripple_color.r = filled_style.ripple_color.r;

  card.bounds.width = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  card.bounds.width = bounds.width;

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_RADIUS));
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_START));
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  CMP_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_KEY_DOWN, 0, 0));
  CMP_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  card.pressed = CMP_FALSE;
  handled = CMP_TRUE;
  CMP_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  card.pressed = CMP_TRUE;
  card.style.ripple_fade_duration = -1.0f;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  card.style.ripple_fade_duration = filled_style.ripple_fade_duration;

  card.pressed = CMP_TRUE;
  CMP_TEST_OK(m3_card_test_set_fail_point(M3_CARD_TEST_FAIL_RIPPLE_RELEASE));
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_card_test_clear_fail_points());

  counter.calls = 0;
  counter.fail = 0;
  CMP_TEST_OK(m3_card_set_on_click(&card, test_card_on_click, &counter));
  card.pressed = CMP_TRUE;
  card.ripple.state = CMP_RIPPLE_STATE_IDLE;
  CMP_TEST_OK(card.widget.vtable->event(card.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(counter.calls == 1);

  counter.fail = 1;
  card.pressed = CMP_TRUE;
  card.ripple.state = CMP_RIPPLE_STATE_IDLE;
  CMP_TEST_EXPECT(card.widget.vtable->event(card.widget.ctx, &event, &handled),
                  CMP_ERR_IO);

  CMP_TEST_EXPECT(card.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(card.widget.vtable->get_semantics(card.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(card.widget.vtable->get_semantics(card.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_BUTTON);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);

  card.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(card.widget.vtable->get_semantics(card.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);

  CMP_TEST_EXPECT(card.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(card.widget.vtable->destroy(card.widget.ctx));
  CMP_TEST_ASSERT(card.widget.vtable == NULL);

  return 0;
}
