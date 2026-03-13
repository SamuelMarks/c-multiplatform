/* clang-format off */
#include "m3/m3_badge.h"
#include "test_utils.h"
/* clang-format on */

static int g_fail_create_font = 0;
static int g_fail_font_metrics = 0;

static int mock_create_font(void *text, const char *utf8_family,
                            cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                            CMPHandle *out_font) {
  if (g_fail_create_font)
    return CMP_ERR_IO;
  if (out_font) {
    out_font->id = 1;
    out_font->generation = 1;
  }
  return CMP_OK;
}
static int mock_destroy_font(void *text, CMPHandle font) { return CMP_OK; }
static int mock_measure_text(void *text, CMPHandle font, const char *utf8,
                             cmp_usize utf8_len, cmp_u32 base_direction,
                             CMPScalar *out_width, CMPScalar *out_height,
                             CMPScalar *out_baseline) {
  if (g_fail_font_metrics && utf8 == NULL)
    return CMP_ERR_IO;
  if (out_width)
    *out_width = 10.0f;
  if (out_height)
    *out_height = 10.0f;
  if (out_baseline)
    *out_baseline = 8.0f;
  return CMP_OK;
}
static int mock_draw_text(void *text, CMPHandle font, const char *utf8,
                          cmp_usize utf8_len, cmp_u32 base_direction,
                          CMPScalar x, CMPScalar y, CMPColor color) {
  return CMP_OK;
}
static const CMPTextVTable mock_text_vtable = {mock_create_font,
                                               mock_destroy_font,
                                               mock_measure_text,
                                               mock_draw_text,
                                               NULL,
                                               NULL,
                                               NULL};

static int mock_draw_rect(void *ctx, const CMPRect *rect, CMPColor color,
                          CMPScalar radius) {
  return CMP_OK;
}
static const CMPGfxVTable g_test_gfx_vtable = {NULL, NULL, NULL, mock_draw_rect,
                                               NULL, NULL, NULL, NULL};

static void cmp_test_text_backend_init(CMPTextBackend *backend) {
  backend->ctx = NULL;
  backend->vtable = &mock_text_vtable;
}

static int test_badge_style(void) {
  M3BadgeStyle style;

  CMP_TEST_EXPECT(m3_badge_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_badge_style_init(&style));

  CMP_TEST_EXPECT(style.dot_diameter == M3_BADGE_DEFAULT_DOT_DIAMETER
                      ? CMP_OK
                      : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(
      style.min_width == M3_BADGE_DEFAULT_MIN_WIDTH ? CMP_OK : CMP_ERR_UNKNOWN,
      CMP_OK);
  CMP_TEST_EXPECT(style.height == M3_BADGE_DEFAULT_HEIGHT ? CMP_OK
                                                          : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(
      style.padding_x == M3_BADGE_DEFAULT_PADDING_X ? CMP_OK : CMP_ERR_UNKNOWN,
      CMP_OK);

  return 0;
}

static int test_badge_init(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPTextBackend backend;

  m3_badge_style_init(&style);
  cmp_test_text_backend_init(&backend);

  CMP_TEST_EXPECT(m3_badge_init(NULL, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, NULL, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  style.dot_diameter = -1.0f;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  style.dot_diameter = 6.0f;

  style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  style.min_width = 16.0f;

  style.height = -1.0f;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  style.height = 16.0f;

  style.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  style.padding_x = 4.0f;

  CMP_TEST_OK(m3_badge_init(&badge, &backend, &style, NULL, 0));

  g_fail_create_font = 1;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, "1", 1), CMP_ERR_IO);
  g_fail_create_font = 0;

  g_fail_font_metrics = 1;
  CMP_TEST_EXPECT(m3_badge_init(&badge, &backend, &style, "1", 1), CMP_ERR_IO);
  g_fail_font_metrics = 0;

  CMP_TEST_OK(m3_badge_init(&badge, &backend, &style, "1", 1));

  CMP_TEST_EXPECT(m3_badge_set_label(NULL, "2", 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_set_label(&badge, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_badge_set_label(&badge, "2", 1));

  CMP_TEST_EXPECT(m3_badge_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, NULL), CMP_ERR_INVALID_ARGUMENT);
  style.height = -1.0f;
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, &style), CMP_ERR_INVALID_ARGUMENT);
  style.height = 16.0f;
  CMP_TEST_OK(m3_badge_set_style(&badge, &style));

  g_fail_create_font = 1;
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, &style), CMP_ERR_IO);
  g_fail_create_font = 0;

  g_fail_font_metrics = 1;
  CMP_TEST_EXPECT(m3_badge_set_style(&badge, &style), CMP_ERR_IO);
  g_fail_font_metrics = 0;

  return 0;
}

static int test_badge_widget(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPTextBackend backend;
  CMPSize size;
  CMPRect bounds = {0, 0, 100, 100};
  CMPInputEvent event = {0};
  CMPBool handled;
  CMPSemantics semantics;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};
  CMPMeasureSpec exact100 = {CMP_MEASURE_EXACTLY, 100.0f};

  m3_badge_style_init(&style);
  cmp_test_text_backend_init(&backend);
  m3_badge_init(&badge, &backend, &style, "3", 1);

  /* Measure */
  CMP_TEST_EXPECT(badge.widget.vtable->measure(NULL, unspec, unspec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->measure(&badge, unspec, unspec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Force text width to be very small to test min_width */
  badge.style.min_width = 100.0f;
  badge.style.padding_x = 0.0f;
  CMP_TEST_OK(badge.widget.vtable->measure(&badge, unspec, unspec, &size));
  CMP_TEST_EXPECT(
      size.width == badge.style.min_width ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  badge.style.min_width = M3_BADGE_DEFAULT_MIN_WIDTH;
  badge.style.padding_x = M3_BADGE_DEFAULT_PADDING_X;

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, unspec, unspec, &size));
  CMP_TEST_EXPECT(size.width >= style.min_width ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(size.height == style.height ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, exact100, exact100, &size));
  CMP_TEST_EXPECT(size.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);
  CMP_TEST_EXPECT(size.height == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Layout */
  CMP_TEST_EXPECT(badge.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->layout(&badge, bounds));
  CMP_TEST_EXPECT(badge.bounds.width == 100.0f ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  /* Paint */
  {
    CMPPaintContext paint_ctx = {0};
    CMPGfx mock_gfx = {0};
    mock_gfx.vtable = &g_test_gfx_vtable;
    mock_gfx.text_vtable = &mock_text_vtable;
    paint_ctx.gfx = &mock_gfx;

    CMP_TEST_EXPECT(badge.widget.vtable->paint(NULL, &paint_ctx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(badge.widget.vtable->paint(&badge, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));

    /* Missing gfx->vtable */
    mock_gfx.vtable = NULL;
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
    mock_gfx.vtable = &g_test_gfx_vtable;

    /* Missing draw_rect */
    {
      CMPGfxVTable no_draw_rect_vtable = {NULL, NULL, NULL, NULL,
                                          NULL, NULL, NULL, NULL};
      mock_gfx.vtable = &no_draw_rect_vtable;
      CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
      mock_gfx.vtable = &g_test_gfx_vtable;
    }

    /* Missing text_vtable->draw_text */
    {
      CMPTextVTable no_draw_text_vtable = {mock_create_font,
                                           mock_destroy_font,
                                           mock_measure_text,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL};
      mock_gfx.text_vtable = &no_draw_text_vtable;
      CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
      mock_gfx.text_vtable = &mock_text_vtable;
    }

    /* Alpha 0 background */
    badge.style.background_color.a = 0.0f;
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
    badge.style.background_color.a = 1.0f;

    /* Alpha 0 text */
    badge.style.text_color.a = 0.0f;
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
    badge.style.text_color.a = 1.0f;

    /* Missing text vtable */
    mock_gfx.text_vtable = NULL;
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
    mock_gfx.text_vtable = &mock_text_vtable;

    /* Missing gfx */
    paint_ctx.gfx = NULL;
    CMP_TEST_OK(badge.widget.vtable->paint(&badge, &paint_ctx));
  }

  /* Event */
  CMP_TEST_EXPECT(badge.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->event(&badge, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->event(&badge, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->event(&badge, &event, &handled));
  CMP_TEST_EXPECT(handled == CMP_FALSE ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Semantics */
  CMP_TEST_EXPECT(badge.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(badge.widget.vtable->get_semantics(&badge, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->get_semantics(&badge, &semantics));
  CMP_TEST_EXPECT(
      semantics.role == CMP_SEMANTIC_TEXT ? CMP_OK : CMP_ERR_UNKNOWN, CMP_OK);

  /* Destroy */
  CMP_TEST_EXPECT(badge.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(badge.widget.vtable->destroy(&badge));

  return 0;
}

static int test_badge_dot(void) {
  M3Badge badge;
  M3BadgeStyle style;
  CMPSize size;
  CMPMeasureSpec unspec = {CMP_MEASURE_UNSPECIFIED, 0.0f};

  m3_badge_style_init(&style);
  m3_badge_init(&badge, NULL, &style, NULL, 0);

  CMP_TEST_OK(badge.widget.vtable->measure(&badge, unspec, unspec, &size));
  CMP_TEST_EXPECT(size.width == style.dot_diameter ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);
  CMP_TEST_EXPECT(size.height == style.dot_diameter ? CMP_OK : CMP_ERR_UNKNOWN,
                  CMP_OK);

  CMP_TEST_OK(badge.widget.vtable->destroy(&badge));
  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_badge_style() == 0);
  CMP_TEST_ASSERT(test_badge_init() == 0);
  CMP_TEST_ASSERT(test_badge_widget() == 0);
  CMP_TEST_ASSERT(test_badge_dot() == 0);
  return 0;
}
