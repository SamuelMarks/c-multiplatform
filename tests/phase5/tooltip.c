#include "cmpc/cmp_api_gfx.h"
#include "cmpc/cmp_core.h"
#include "m3/m3_tooltip.h"
#include "test_utils.h"

#include <stdio.h>
#include <string.h>

typedef struct TestBackend {
  int draw_rect_calls;
  int draw_text_calls;
} TestBackend;

static void test_backend_init(TestBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

static int g_fail_draw_rect = 0;

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestBackend *backend = (TestBackend *)gfx;
  if (g_fail_draw_rect)
    return CMP_ERR_UNSUPPORTED;
  if (backend)
    backend->draw_rect_calls++;
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static int g_fail_create_font = 0;
static int g_fail_create_font_count = 0;
static int g_fail_destroy_font = 0;
static int g_fail_measure = 0;
static int g_fail_measure_count = 0;
static int g_fail_draw = 0;
static int g_fail_draw_count = 0;

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  if (g_fail_create_font)
    return CMP_ERR_UNSUPPORTED;
  if (g_fail_create_font_count > 0) {
    g_fail_create_font_count--;
    if (g_fail_create_font_count == 0)
      return CMP_ERR_UNSUPPORTED;
  }
  if (out_font) {
    out_font->id = 1u;
    out_font->generation = 1u;
  }
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  if (g_fail_destroy_font)
    return CMP_ERR_UNSUPPORTED;
  return CMP_OK;
}

static int test_text_measure_text(void *text, CMPHandle font, const char *utf8,
                                  cmp_usize utf8_len, cmp_u32 base_direction,
                                  CMPScalar *out_width, CMPScalar *out_height,
                                  CMPScalar *out_baseline) {
  if (g_fail_measure)
    return CMP_ERR_UNSUPPORTED;
  if (g_fail_measure_count > 0) {
    g_fail_measure_count--;
    if (g_fail_measure_count == 0)
      return CMP_ERR_UNSUPPORTED;
  }
  if (out_width)
    *out_width = 10.0f;
  if (out_height)
    *out_height = 10.0f;
  if (out_baseline)
    *out_baseline = 8.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, cmp_u32 base_direction,
                               CMPScalar x, CMPScalar y, CMPColor color) {
  TestBackend *backend = (TestBackend *)text;
  if (g_fail_draw)
    return CMP_ERR_UNSUPPORTED;
  if (g_fail_draw_count > 0) {
    g_fail_draw_count--;
    if (g_fail_draw_count == 0)
      return CMP_ERR_UNSUPPORTED;
  }
  if (backend)
    backend->draw_text_calls++;
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {test_text_create_font,
                                                 test_text_destroy_font,
                                                 test_text_measure_text,
                                                 test_text_draw_text,
                                                 NULL,
                                                 NULL,
                                                 NULL};

static int test_tooltip(void) {
  CMPTooltipStyle plain_style;
  CMPTooltipStyle rich_style;
  M3Tooltip tooltip;
  CMPTooltipAnchor anchor;
  CMPTooltipPlacement placement;
  CMPRect overlay = {0.0f, 0.0f, 800.0f, 600.0f};
  CMPMeasureSpec width_spec = {0, 0};
  CMPMeasureSpec height_spec = {0, 0};
  CMPSize size;
  CMPRect bounds = {0.0f, 0.0f, 100.0f, 100.0f};
  CMPPaintContext ctx;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;

  CMPTextBackend text_backend;
  CMPGfx gfx;
  TestBackend backend;

  test_backend_init(&backend);
  memset(&gfx, 0, sizeof(gfx));
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;
  gfx.ctx = &backend;

  memset(&text_backend, 0, sizeof(text_backend));
  text_backend.vtable = &g_test_text_vtable;
  text_backend.ctx = &backend;

  memset(&ctx, 0, sizeof(ctx));
  ctx.gfx = &gfx;
  ctx.clip = overlay;
  ctx.dpi_scale = 1.0f;

  CMP_TEST_EXPECT(m3_tooltip_style_init_plain(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tooltip_style_init_plain(&plain_style));

  CMP_TEST_EXPECT(m3_tooltip_style_init_rich(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tooltip_style_init_rich(&rich_style));

  anchor.type = CMP_TOOLTIP_ANCHOR_POINT;
  anchor.point.x = 400.0f;
  anchor.point.y = 300.0f;

  placement.direction = CMP_TOOLTIP_DIRECTION_UP;
  placement.align = CMP_TOOLTIP_ALIGN_CENTER;

  CMP_TEST_EXPECT(m3_tooltip_init(NULL, &text_backend, &plain_style, &anchor,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, NULL, &plain_style, &anchor,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, NULL, &anchor,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &plain_style, NULL,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &plain_style,
                                  &anchor, NULL, overlay, "Hello", 5u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &plain_style,
                                  &anchor, &placement, overlay, NULL, 5u),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Init plain tooltip */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));

  /* Measure plain */
  CMP_TEST_EXPECT(
      tooltip.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                                 height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                             height_spec, &size));

  /* Layout plain */
  CMP_TEST_EXPECT(tooltip.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds));

  /* Paint plain */
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx));

  /* Paint plain with 0 bounds */
  tooltip.bounds.width = 0.0f;
  CMP_TEST_OK(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx));
  tooltip.bounds.width = bounds.width;

  /* Event plain */
  CMP_TEST_EXPECT(tooltip.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      tooltip.widget.vtable->event(tooltip.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      tooltip.widget.vtable->event(tooltip.widget.ctx, &event, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      tooltip.widget.vtable->event(tooltip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  /* Semantics plain */
  CMP_TEST_EXPECT(tooltip.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      tooltip.widget.vtable->get_semantics(tooltip.widget.ctx, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      tooltip.widget.vtable->get_semantics(tooltip.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_TEXT);
  CMP_TEST_ASSERT(semantics.utf8_label != NULL);

  /* Metrics coverages */
  CMP_TEST_EXPECT(m3_tooltip_test_metrics_update(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Destroy plain */
  CMP_TEST_EXPECT(tooltip.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(tooltip.widget.vtable->destroy(tooltip.widget.ctx));

  /* Init rich tooltip */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                              &placement, overlay, "Rich Body", 9u));
  CMP_TEST_EXPECT(m3_tooltip_set_title(NULL, "Rich Title", 10u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tooltip_set_title(&tooltip, "Rich Title", 10u));

  /* Set overlay */
  CMP_TEST_EXPECT(m3_tooltip_set_overlay(NULL, overlay),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tooltip_set_overlay(&tooltip, overlay));

  CMP_TEST_OK(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                             height_spec, &size));
  CMP_TEST_OK(tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds));
  CMP_TEST_OK(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx));

  CMP_TEST_OK(tooltip.widget.vtable->destroy(tooltip.widget.ctx));

  /* Test unsupported */
  CMPGfx bad_gfx;
  CMPPaintContext bad_ctx;
  memset(&bad_gfx, 0, sizeof(bad_gfx));
  memset(&bad_ctx, 0, sizeof(bad_ctx));
  bad_ctx.gfx = &bad_gfx;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &bad_ctx),
                  CMP_ERR_UNSUPPORTED);
  bad_gfx.vtable = &g_test_gfx_vtable;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &bad_ctx),
                  CMP_ERR_UNSUPPORTED);

  /* Test fail create font (plain) */
  g_fail_create_font = 1;
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &plain_style,
                                  &anchor, &placement, overlay, "Hello", 5u),
                  CMP_ERR_UNSUPPORTED);
  g_fail_create_font = 0;

  /* Test fail create font (rich) */
  g_fail_create_font = 1;
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_UNSUPPORTED);
  g_fail_create_font = 0;

  /* Test fail create font (rich body - 2nd font) */
  g_fail_create_font_count = 1;
  CMP_TEST_EXPECT(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                                  &placement, overlay, "Hello", 5u),
                  CMP_ERR_UNSUPPORTED);
  g_fail_create_font_count = 0;

  /* Test measure fail plain */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  g_fail_measure = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds),
                  CMP_ERR_UNSUPPORTED);
  g_fail_measure = 0;

  /* Test draw fail plain */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec, height_spec,
                                 &size);
  tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds);
  g_fail_draw = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  g_fail_draw = 0;

  /* Test measure fail rich title */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  m3_tooltip_set_title(&tooltip, "Title", 5u);
  g_fail_measure = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_UNSUPPORTED);
  g_fail_measure = 0;

  /* Test measure fail rich body */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  m3_tooltip_set_title(&tooltip, "Title", 5u);
  g_fail_measure_count = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_UNSUPPORTED);
  g_fail_measure_count = 0;

  /* Test draw fail rich body */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  m3_tooltip_set_title(&tooltip, "Title", 5u);
  tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec, height_spec,
                                 &size);
  tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds);
  g_fail_draw_count = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  g_fail_draw_count = 0;

  /* Test draw fail rich title */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &rich_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  m3_tooltip_set_title(&tooltip, "Title", 5u);
  tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec, height_spec,
                                 &size);
  tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds);
  g_fail_draw = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  g_fail_draw = 0;

  /* Test invalid style in measure/layout to fail
   * cmp_tooltip_compute_content_size */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  tooltip.style.padding.left = -10.0f; /* Invalid padding */
  CMP_TEST_EXPECT(tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  tooltip.style.padding.left = plain_style.padding.left; /* Restore */

  /* Test draw rect fail */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  tooltip.widget.vtable->measure(tooltip.widget.ctx, width_spec, height_spec,
                                 &size);
  tooltip.widget.vtable->layout(tooltip.widget.ctx, bounds);
  g_fail_draw_rect = 1;
  CMP_TEST_EXPECT(tooltip.widget.vtable->paint(tooltip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  g_fail_draw_rect = 0;

  /* Force destroy to fail to hit cleanup_rc branch */
  CMP_TEST_OK(m3_tooltip_init(&tooltip, &text_backend, &plain_style, &anchor,
                              &placement, overlay, "Hello", 5u));
  g_fail_destroy_font = 1;
  tooltip.widget.vtable->destroy(tooltip.widget.ctx);
  g_fail_destroy_font = 0;

  return 0;
}

int main(void) {
  if (test_tooltip() != 0) {
    return 1;
  }
  return 0;
}
