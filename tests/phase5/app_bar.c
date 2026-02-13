#include "m3/m3_app_bar.h"
#include "test_utils.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_APP_BAR_TEST_FAIL_NONE 0u
#define M3_APP_BAR_TEST_FAIL_SHADOW_INIT 1u
#define M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT 2u
#define M3_APP_BAR_TEST_FAIL_COLOR_SET 3u
#define M3_APP_BAR_TEST_FAIL_SCROLL_INIT 4u
#endif

int CMP_CALL m3_app_bar_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL m3_app_bar_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_app_bar_test_clear_fail_points(void);
int CMP_CALL m3_app_bar_test_set_match_fail_after(cmp_u32 call_count);
int CMP_CALL m3_app_bar_test_set_color_error_after(cmp_u32 call_count);
int CMP_CALL m3_app_bar_test_set_collapse_fail(CMPBool enable);
int CMP_CALL m3_app_bar_test_call_color_should_fail(CMPBool *out_fail);
int CMP_CALL m3_app_bar_test_call_fail_point_match(cmp_u32 point,
                                                  CMPBool *out_match);
int CMP_CALL m3_app_bar_test_validate_color(const CMPColor *color);
int CMP_CALL m3_app_bar_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                      CMPScalar b, CMPScalar a);
int CMP_CALL m3_app_bar_test_validate_edges(const CMPLayoutEdges *edges);
int CMP_CALL m3_app_bar_test_validate_text_style(const CMPTextStyle *style,
                                                CMPBool require_family);
int CMP_CALL m3_app_bar_test_validate_style(const M3AppBarStyle *style,
                                           CMPBool require_family);
int CMP_CALL m3_app_bar_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_app_bar_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_app_bar_test_compute_collapse_range(const M3AppBarStyle *style,
                                                   CMPScalar *out_range);
int CMP_CALL m3_app_bar_test_compute_current_height(const M3AppBar *bar,
                                                   CMPScalar *out_height);
int CMP_CALL m3_app_bar_test_compute_content_bounds(const M3AppBar *bar,
                                                   CMPRect *out_bounds);
int CMP_CALL m3_app_bar_test_compute_title_position(const M3AppBar *bar,
                                                   const CMPTextMetrics *metrics,
                                                   CMPScalar *out_x,
                                                   CMPScalar *out_y);
int CMP_CALL m3_app_bar_test_measure_title(const M3AppBar *bar,
                                          CMPTextMetrics *out_metrics);
int CMP_CALL m3_app_bar_test_apply_scroll(M3AppBar *bar, CMPScalar delta,
                                         CMPScalar *out_consumed);

typedef struct TestAppBarBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int draw_rect_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_draw;
  int fail_draw_rect;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestAppBarBackend;

static int test_backend_init(TestAppBarBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return CMP_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                                 CMPHandle *out_font) {
  TestAppBarBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }
  if (italic != CMP_FALSE && italic != CMP_TRUE) {
    return CMP_ERR_RANGE;
  }

  backend = (TestAppBarBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return CMP_ERR_IO;
  }

  out_font->id = 1u;
  out_font->generation = 1u;
  backend->last_font = *out_font;
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  TestAppBarBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestAppBarBackend *)text;
  backend->destroy_calls += 1;
  if (backend->fail_destroy) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_text_measure_text(void *text, CMPHandle font, const char *utf8,
                                  cmp_usize utf8_len, CMPScalar *out_width,
                                  CMPScalar *out_height,
                                  CMPScalar *out_baseline) {
  TestAppBarBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestAppBarBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }

  *out_width = (CMPScalar)(utf8_len * 10u);
  *out_height = 20.0f;
  *out_baseline = 15.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestAppBarBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestAppBarBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw) {
    return CMP_ERR_IO;
  }

  backend->last_text_x = x;
  backend->last_text_y = y;
  backend->last_text_len = utf8_len;
  backend->last_text_color = color;
  return CMP_OK;
}

static int test_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color,
                              CMPScalar corner_radius) {
  TestAppBarBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestAppBarBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

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
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_style(M3AppBarStyle *style) {
  int rc;

  rc = m3_app_bar_style_init_small(style);
  if (rc != CMP_OK) {
    return rc;
  }
  style->title_style.utf8_family = "Test";
  style->title_style.size_px = 20;
  style->title_style.weight = 500;
  style->title_style.italic = CMP_FALSE;
  style->title_style.color.r = 0.1f;
  style->title_style.color.g = 0.2f;
  style->title_style.color.b = 0.3f;
  style->title_style.color.a = 1.0f;
  return CMP_OK;
}

static int test_app_bar_helpers(void) {
  CMPColor color;
  CMPLayoutEdges edges;
  CMPTextStyle text_style;
  M3AppBarStyle style;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3AppBar bar;
  CMPScalar height;
  CMPRect content;
  CMPTextMetrics metrics;
  CMPScalar pos_x;
  CMPScalar pos_y;

  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(NULL, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_app_bar_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);

  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_color(&color), CMP_ERR_RANGE);

  color.r = 0.0f;
  color.g = -0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_color(&color), CMP_ERR_RANGE);

  color.g = 0.0f;
  color.b = -0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_color(&color), CMP_ERR_RANGE);

  color.b = 0.0f;
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_color(&color), CMP_ERR_RANGE);

  color.a = 1.0f;
  CMP_TEST_OK(m3_app_bar_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_app_bar_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                 CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_app_bar_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

#ifdef CMP_TESTING
  CMP_TEST_EXPECT(m3_app_bar_test_call_color_should_fail(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_app_bar_test_call_fail_point_match(M3_APP_BAR_TEST_FAIL_NONE, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_app_bar_test_set_color_error_after(2));
  CMP_TEST_OK(m3_app_bar_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  CMP_TEST_OK(m3_app_bar_test_set_color_error_after(1));
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  CMP_TEST_OK(m3_app_bar_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_app_bar_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f),
                 CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
#endif

  CMP_TEST_EXPECT(m3_app_bar_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_app_bar_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(NULL, CMP_FALSE),
                 CMP_ERR_INVALID_ARGUMENT);
  memset(&text_style, 0, sizeof(text_style));
  text_style.utf8_family = NULL;
  text_style.size_px = 12;
  text_style.weight = 400;
  text_style.italic = CMP_FALSE;
  text_style.color = color;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);

  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 99;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.color.r = 0.1f;
  CMP_TEST_OK(m3_app_bar_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_OK(m3_app_bar_style_init_small(&style));
  CMP_TEST_OK(m3_app_bar_test_validate_style(&style, CMP_FALSE));
  style.variant = 99u;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.variant = M3_APP_BAR_VARIANT_SMALL;
  style.collapsed_height = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.collapsed_height = M3_APP_BAR_DEFAULT_SMALL_HEIGHT;
  style.expanded_height = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.expanded_height = style.collapsed_height - 1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.expanded_height = M3_APP_BAR_DEFAULT_SMALL_HEIGHT;
  style.padding.top = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.padding.top = M3_APP_BAR_DEFAULT_PADDING_Y;
  style.padding.bottom = style.collapsed_height;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.padding.bottom = M3_APP_BAR_DEFAULT_PADDING_Y;
  style.shadow_enabled = 2;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.shadow_enabled = CMP_FALSE;
  style.background_color.r = -0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_FALSE),
                 CMP_ERR_RANGE);
  style.background_color.r = 0.1f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  style.title_style.utf8_family = "Test";
  CMP_TEST_OK(m3_app_bar_test_validate_style(&style, CMP_TRUE));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_measure_spec(spec),
                 CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.size = 10.0f;
  CMP_TEST_OK(m3_app_bar_test_validate_measure_spec(spec));

  rect.width = -1.0f;
  rect.height = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_app_bar_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_app_bar_test_compute_collapse_range(NULL, &height),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&bar, 0, sizeof(bar));
  bar.style = style;
  bar.collapse_offset = -1.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_current_height(NULL, &height),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_compute_current_height(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_compute_current_height(&bar, &height),
                 CMP_ERR_RANGE);
  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  bar.collapse_offset = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_current_height(&bar, &height),
                 CMP_ERR_RANGE);
  bar.style = style;
  bar.collapse_offset = 0.0f;
  CMP_TEST_OK(m3_app_bar_test_compute_current_height(&bar, &height));
  bar.style.collapsed_height = -1.0f;
  bar.style.expanded_height = -1.0f;
  bar.collapse_offset = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_current_height(&bar, &height),
                 CMP_ERR_RANGE);
  bar.style = style;

  bar.bounds.x = 0.0f;
  bar.bounds.y = 0.0f;
  bar.bounds.width = 200.0f;
  bar.bounds.height = 200.0f;
  bar.style.padding.left = 10.0f;
  bar.style.padding.right = 10.0f;
  bar.style.padding.top = 10.0f;
  bar.style.padding.bottom = 10.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_content_bounds(NULL, &content),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_compute_content_bounds(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_compute_content_bounds(&bar, &content));
  CMP_TEST_ASSERT(cmp_near(content.width, 180.0f, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(content.height, bar.style.expanded_height - 20.0f, 0.001f));
  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  bar.collapse_offset = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_content_bounds(&bar, &content),
                 CMP_ERR_RANGE);
  bar.style = style;
  bar.bounds.width = 5.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_compute_content_bounds(&bar, &content),
                 CMP_ERR_RANGE);

  metrics.width = 20.0f;
  metrics.height = 10.0f;
  metrics.baseline = 7.0f;
  bar.bounds.width = 200.0f;
  bar.bounds.height = 200.0f;
  bar.style.variant = M3_APP_BAR_VARIANT_CENTER;
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(NULL, &metrics, &pos_x, &pos_y),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, NULL, &pos_x, &pos_y),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, &metrics, NULL, &pos_y),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));
  CMP_TEST_ASSERT(pos_x > bar.bounds.x);
  bar.style.variant = M3_APP_BAR_VARIANT_SMALL;
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));
  CMP_TEST_ASSERT(cmp_near(pos_x, bar.bounds.x + bar.style.padding.left, 0.001f));

  bar.style.variant = M3_APP_BAR_VARIANT_MEDIUM;
  bar.style.expanded_height = 112.0f;
  bar.style.collapsed_height = 64.0f;
  bar.collapse_offset = 24.0f;
  CMP_TEST_OK(m3_app_bar_test_set_collapse_fail(CMP_TRUE));
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y),
      CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));

  bar.collapse_offset = -10.0f;
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));
  bar.collapse_offset = 200.0f;
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));
  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y),
      CMP_ERR_RANGE);

  bar.style.expanded_height = 64.0f;
  bar.style.collapsed_height = 64.0f;
  bar.collapse_offset = 0.0f;
  CMP_TEST_OK(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y));

  bar.bounds.width = 10.0f;
  bar.style.padding.left = 8.0f;
  bar.style.padding.right = 8.0f;
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y),
      CMP_ERR_RANGE);

  bar.bounds.width = 200.0f;
  bar.style.padding.left = 10.0f;
  bar.style.padding.right = 10.0f;
  bar.style.padding.top = 40.0f;
  bar.style.padding.bottom = 40.0f;
  bar.style.collapsed_height = 60.0f;
  bar.style.expanded_height = 60.0f;
  CMP_TEST_EXPECT(
      m3_app_bar_test_compute_title_position(&bar, &metrics, &pos_x, &pos_y),
      CMP_ERR_RANGE);

  bar.style = style;
  bar.style.expanded_height = 112.0f;
  bar.style.collapsed_height = 64.0f;
  bar.collapse_offset = 20.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_measure_title(NULL, &metrics),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_measure_title(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_measure_title(&bar, &metrics));
  CMP_TEST_OK(m3_app_bar_test_apply_scroll(&bar, 0.0f, &height));
  CMP_TEST_ASSERT(cmp_near(height, 0.0f, 0.001f));
  CMP_TEST_OK(m3_app_bar_test_apply_scroll(&bar, 10.0f, &height));
  CMP_TEST_ASSERT(cmp_near(height, 10.0f, 0.001f));
  CMP_TEST_OK(m3_app_bar_test_apply_scroll(&bar, -5.0f, &height));
  CMP_TEST_ASSERT(cmp_near(height, -5.0f, 0.001f));
  bar.collapse_offset = 5.0f;
  CMP_TEST_OK(m3_app_bar_test_apply_scroll(&bar, -10.0f, &height));
  CMP_TEST_ASSERT(cmp_near(height, -5.0f, 0.001f));
  CMP_TEST_EXPECT(m3_app_bar_test_apply_scroll(NULL, 1.0f, &height),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_test_apply_scroll(&bar, 1.0f, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  bar.collapse_offset = 1000.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_apply_scroll(&bar, 1.0f, &height),
                 CMP_ERR_RANGE);
  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  bar.collapse_offset = 0.0f;
  CMP_TEST_EXPECT(m3_app_bar_test_apply_scroll(&bar, 1.0f, &height),
                 CMP_ERR_RANGE);

  return 0;
}

static int test_app_bar_style_init(void) {
  M3AppBarStyle style;

  CMP_TEST_EXPECT(m3_app_bar_style_init_small(NULL), CMP_ERR_INVALID_ARGUMENT);

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_app_bar_test_set_match_fail_after(1));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_set_match_fail_after(2));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_set_match_fail_after(3));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
#endif

  CMP_TEST_OK(m3_app_bar_style_init_small(&style));
  CMP_TEST_ASSERT(style.variant == M3_APP_BAR_VARIANT_SMALL);
  CMP_TEST_ASSERT(
      cmp_near(style.collapsed_height, M3_APP_BAR_DEFAULT_SMALL_HEIGHT, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.expanded_height, M3_APP_BAR_DEFAULT_SMALL_HEIGHT, 0.001f));

  CMP_TEST_OK(m3_app_bar_style_init_center(&style));
  CMP_TEST_ASSERT(style.variant == M3_APP_BAR_VARIANT_CENTER);

  CMP_TEST_OK(m3_app_bar_style_init_medium(&style));
  CMP_TEST_ASSERT(style.variant == M3_APP_BAR_VARIANT_MEDIUM);
  CMP_TEST_ASSERT(
      cmp_near(style.expanded_height, M3_APP_BAR_DEFAULT_MEDIUM_HEIGHT, 0.001f));

  CMP_TEST_OK(m3_app_bar_style_init_large(&style));
  CMP_TEST_ASSERT(style.variant == M3_APP_BAR_VARIANT_LARGE);
  CMP_TEST_ASSERT(
      cmp_near(style.expanded_height, M3_APP_BAR_DEFAULT_LARGE_HEIGHT, 0.001f));

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_app_bar_test_set_fail_point(M3_APP_BAR_TEST_FAIL_SHADOW_INIT));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  CMP_TEST_OK(
      m3_app_bar_test_set_fail_point(M3_APP_BAR_TEST_FAIL_TEXT_STYLE_INIT));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  CMP_TEST_OK(m3_app_bar_test_set_fail_point(M3_APP_BAR_TEST_FAIL_COLOR_SET));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  CMP_TEST_OK(m3_app_bar_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_app_bar_style_init_small(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
#endif

  return 0;
}

static int test_app_bar_init_and_setters(void) {
  TestAppBarBackend backend_state;
  CMPTextBackend backend;
  M3AppBarStyle style;
  M3AppBarStyle new_style;
  M3AppBar bar;
  CMPScalar offset;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(init_style(&style));
  style.title_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, NULL, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  style.title_style.utf8_family = "Test";

  backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4),
                 CMP_ERR_INVALID_ARGUMENT);
  backend.vtable = &g_test_text_vtable;

  backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4),
                 CMP_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_EXPECT(m3_app_bar_init(NULL, &backend, &style, NULL, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, NULL, &style, NULL, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, NULL, NULL, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, NULL, 4),
                 CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4), CMP_ERR_IO);
  backend_state.fail_create = 0;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_app_bar_test_set_fail_point(M3_APP_BAR_TEST_FAIL_SCROLL_INIT));
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());

  backend_state.fail_destroy = 1;
  CMP_TEST_OK(m3_app_bar_test_set_fail_point(M3_APP_BAR_TEST_FAIL_SCROLL_INIT));
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4), CMP_ERR_IO);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
  backend_state.fail_destroy = 0;

  CMP_TEST_OK(m3_app_bar_test_set_match_fail_after(1));
  CMP_TEST_EXPECT(m3_app_bar_init(&bar, &backend, &style, "Test", 4),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
#endif

  CMP_TEST_OK(m3_app_bar_init(&bar, &backend, &style, "Test", 4));
  CMP_TEST_ASSERT(bar.widget.vtable != NULL);
  CMP_TEST_ASSERT(bar.scroll_parent.vtable != NULL);

  CMP_TEST_EXPECT(m3_app_bar_set_title(NULL, NULL, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_set_title(&bar, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_set_title(&bar, NULL, 0));
  CMP_TEST_OK(m3_app_bar_set_title(&bar, "New", 3));

  CMP_TEST_EXPECT(m3_app_bar_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_set_style(&bar, NULL), CMP_ERR_INVALID_ARGUMENT);

  new_style = style;
  new_style.title_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_app_bar_set_style(&bar, &new_style),
                 CMP_ERR_INVALID_ARGUMENT);
  new_style.title_style.utf8_family = "Test";
  new_style.title_style.size_px = 24;
  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_app_bar_set_style(&bar, &new_style), CMP_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_app_bar_set_style(&bar, &new_style), CMP_ERR_IO);
  backend_state.fail_destroy = 0;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_app_bar_test_set_collapse_fail(CMP_TRUE));
  CMP_TEST_EXPECT(m3_app_bar_set_style(&bar, &new_style), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_app_bar_test_clear_fail_points());
#endif

  CMP_TEST_OK(m3_app_bar_set_style(&bar, &new_style));

  CMP_TEST_EXPECT(m3_app_bar_set_collapse_offset(NULL, 0.0f),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_set_collapse_offset(&bar, -1.0f), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_app_bar_set_collapse_offset(&bar, 1.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_app_bar_set_collapse_offset(&bar, 0.0f));
  CMP_TEST_EXPECT(m3_app_bar_get_collapse_offset(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_get_collapse_offset(&bar, &offset));
  CMP_TEST_ASSERT(cmp_near(offset, 0.0f, 0.001f));

  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  CMP_TEST_EXPECT(m3_app_bar_set_collapse_offset(&bar, 0.0f), CMP_ERR_RANGE);
  bar.style.expanded_height = new_style.expanded_height;
  bar.style.collapsed_height = new_style.collapsed_height;

  bar.collapse_offset = -1.0f;
  CMP_TEST_OK(m3_app_bar_set_style(&bar, &new_style));
  CMP_TEST_OK(m3_app_bar_get_collapse_offset(&bar, &offset));
  CMP_TEST_ASSERT(cmp_near(offset, 0.0f, 0.001f));

  CMP_TEST_OK(m3_app_bar_style_init_large(&new_style));
  new_style.title_style.utf8_family = "Test";
  CMP_TEST_OK(m3_app_bar_set_style(&bar, &new_style));
  CMP_TEST_OK(m3_app_bar_set_collapse_offset(&bar, 88.0f));
  CMP_TEST_OK(m3_app_bar_style_init_small(&new_style));
  new_style.title_style.utf8_family = "Test";
  CMP_TEST_OK(m3_app_bar_set_style(&bar, &new_style));
  CMP_TEST_OK(m3_app_bar_get_collapse_offset(&bar, &offset));
  CMP_TEST_ASSERT(cmp_near(offset, 0.0f, 0.001f));

  CMP_TEST_OK(bar.widget.vtable->destroy(&bar));
  return 0;
}

static int test_app_bar_scroll(void) {
  TestAppBarBackend backend_state;
  CMPTextBackend backend;
  M3AppBarStyle style;
  M3AppBar bar;
  CMPScrollDelta delta;
  CMPScrollDelta consumed;
  CMPScrollDelta child_consumed;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_app_bar_style_init_medium(&style));
  style.title_style.utf8_family = "Test";
  style.title_style.size_px = 20;
  style.title_style.weight = 500;
  CMP_TEST_OK(m3_app_bar_init(&bar, &backend, &style, "Test", 4));

  delta.x = 0.0f;
  delta.y = 20.0f;
  CMP_TEST_OK(bar.scroll_parent.vtable->pre_scroll(bar.scroll_parent.ctx, &delta,
                                                  &consumed));
  CMP_TEST_ASSERT(cmp_near(consumed.y, 20.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bar.collapse_offset, 20.0f, 0.001f));

  delta.y = 40.0f;
  CMP_TEST_OK(bar.scroll_parent.vtable->pre_scroll(bar.scroll_parent.ctx, &delta,
                                                  &consumed));
  CMP_TEST_ASSERT(cmp_near(consumed.y, 28.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bar.collapse_offset, 48.0f, 0.001f));

  delta.y = -5.0f;
  CMP_TEST_OK(bar.scroll_parent.vtable->pre_scroll(bar.scroll_parent.ctx, &delta,
                                                  &consumed));
  CMP_TEST_ASSERT(cmp_near(consumed.y, 0.0f, 0.001f));

  delta.y = -10.0f;
  child_consumed.x = 0.0f;
  child_consumed.y = 0.0f;
  CMP_TEST_OK(bar.scroll_parent.vtable->post_scroll(
      bar.scroll_parent.ctx, &delta, &child_consumed, &consumed));
  CMP_TEST_ASSERT(cmp_near(consumed.y, -10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(bar.collapse_offset, 38.0f, 0.001f));

  delta.y = 10.0f;
  CMP_TEST_OK(bar.scroll_parent.vtable->post_scroll(
      bar.scroll_parent.ctx, &delta, &child_consumed, &consumed));
  CMP_TEST_ASSERT(cmp_near(consumed.y, 0.0f, 0.001f));

  CMP_TEST_EXPECT(bar.scroll_parent.vtable->pre_scroll(NULL, &delta, &consumed),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(bar.scroll_parent.vtable->post_scroll(
                     bar.scroll_parent.ctx, NULL, &child_consumed, &consumed),
                 CMP_ERR_INVALID_ARGUMENT);

  bar.style.expanded_height = 10.0f;
  bar.style.collapsed_height = 20.0f;
  delta.y = -5.0f;
  CMP_TEST_EXPECT(bar.scroll_parent.vtable->post_scroll(
                     bar.scroll_parent.ctx, &delta, &child_consumed, &consumed),
                 CMP_ERR_RANGE);
  bar.style.expanded_height = style.expanded_height;
  bar.style.collapsed_height = style.collapsed_height;

  bar.collapse_offset = -1.0f;
  delta.y = 5.0f;
  CMP_TEST_EXPECT(bar.scroll_parent.vtable->pre_scroll(bar.scroll_parent.ctx,
                                                      &delta, &consumed),
                 CMP_ERR_RANGE);

  CMP_TEST_OK(bar.widget.vtable->destroy(&bar));
  return 0;
}

static int test_app_bar_widget(void) {
  TestAppBarBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  M3AppBarStyle style;
  M3AppBar bar;
  CMPMeasureSpec spec;
  CMPSize size;
  CMPRect bounds;
  CMPSemantics semantics;
  CMPInputEvent event;
  CMPBool handled;
  int rc;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  gfx.ctx = &backend_state;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  ctx.gfx = &gfx;
  ctx.clip.x = 0.0f;
  ctx.clip.y = 0.0f;
  ctx.clip.width = 200.0f;
  ctx.clip.height = 200.0f;
  ctx.dpi_scale = 1.0f;

  CMP_TEST_OK(init_style(&style));
  CMP_TEST_OK(m3_app_bar_init(&bar, &backend, &style, "Title", 5));

  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(NULL, spec, spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(bar.widget.vtable->measure(&bar, spec, spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.height, bar.style.expanded_height, 0.001f));
  CMP_TEST_ASSERT(cmp_near(
      size.width, bar.style.padding.left + bar.style.padding.right + 50.0f,
      0.001f));

  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 123.0f;
  CMP_TEST_OK(bar.widget.vtable->measure(&bar, spec, spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 123.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 123.0f, 0.001f));

  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = 30.0f;
  CMP_TEST_OK(bar.widget.vtable->measure(&bar, spec, spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 30.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 30.0f, 0.001f));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = 10.0f;
  {
    CMPMeasureSpec bad_spec = spec;
    bad_spec.mode = 99u;
    CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, bad_spec, &size),
                   CMP_ERR_INVALID_ARGUMENT);
  }

  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, &size),
                 CMP_ERR_RANGE);

  bar.collapse_offset =
      (bar.style.expanded_height - bar.style.collapsed_height) + 1.0f;
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, &size),
                 CMP_ERR_RANGE);
  bar.collapse_offset = 0.0f;

  bar.style.collapsed_height = -1.0f;
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, &size),
                 CMP_ERR_RANGE);
  bar.style.collapsed_height = style.collapsed_height;

  backend_state.fail_measure = 1;
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->measure(&bar, spec, spec, &size),
                 CMP_ERR_IO);
  backend_state.fail_measure = 0;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->layout(&bar, bounds), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(bar.widget.vtable->layout(NULL, bounds),
                 CMP_ERR_INVALID_ARGUMENT);

  bounds.width = 200.0f;
  bounds.height = 100.0f;
  CMP_TEST_OK(bar.widget.vtable->layout(&bar, bounds));

  CMP_TEST_EXPECT(m3_app_bar_get_height(NULL, &size.height),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_get_height(&bar, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_get_height(&bar, &size.height));

  CMP_TEST_EXPECT(m3_app_bar_get_content_bounds(NULL, &bounds),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_app_bar_get_content_bounds(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_app_bar_get_content_bounds(&bar, &bounds));

  CMP_TEST_EXPECT(bar.widget.vtable->paint(NULL, &ctx), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, NULL), CMP_ERR_INVALID_ARGUMENT);

  ctx.gfx = NULL;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_INVALID_ARGUMENT);
  ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  bar.style.collapsed_height = 0.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_RANGE);
  bar.style.collapsed_height = style.collapsed_height;

  bar.collapse_offset =
      (bar.style.expanded_height - bar.style.collapsed_height) + 1.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_RANGE);
  bar.collapse_offset = 0.0f;

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_IO);
  backend_state.fail_measure = 0;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  bar.bounds.width = -1.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_RANGE);
  bar.bounds.width = bounds.width;

  bar.style.shadow_enabled = CMP_TRUE;
  bar.style.shadow.layers = 0u;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_RANGE);
  bar.style.shadow.layers = 1u;
  CMP_TEST_OK(bar.widget.vtable->paint(&bar, &ctx));
  bar.style.shadow_enabled = CMP_FALSE;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_draw = 1;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_IO);
  backend_state.fail_draw = 0;

  bar.bounds.width = 10.0f;
  bar.style.padding.left = 8.0f;
  bar.style.padding.right = 8.0f;
  CMP_TEST_EXPECT(bar.widget.vtable->paint(&bar, &ctx), CMP_ERR_RANGE);
  bar.bounds.width = 200.0f;
  bar.style.padding.left = M3_APP_BAR_DEFAULT_PADDING_X;
  bar.style.padding.right = M3_APP_BAR_DEFAULT_PADDING_X;

  bar.title_len = 0u;
  CMP_TEST_OK(bar.widget.vtable->paint(&bar, &ctx));
  bar.utf8_title = "Title";
  bar.title_len = 5u;

  CMP_TEST_OK(bar.widget.vtable->paint(&bar, &ctx));
  CMP_TEST_ASSERT(backend_state.draw_rect_calls > 0);
  CMP_TEST_ASSERT(backend_state.draw_calls > 0);

  CMP_TEST_EXPECT(bar.widget.vtable->event(NULL, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  memset(&event, 0, sizeof(event));
  CMP_TEST_OK(bar.widget.vtable->event(&bar, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(NULL, &semantics),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(bar.widget.vtable->get_semantics(&bar, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  bar.widget.flags = CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE;
  CMP_TEST_OK(bar.widget.vtable->get_semantics(&bar, &semantics));
  CMP_TEST_ASSERT(semantics.flags & CMP_SEMANTIC_FLAG_DISABLED);
  CMP_TEST_ASSERT(semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE);
  CMP_TEST_ASSERT(semantics.utf8_label == bar.utf8_title);

  CMP_TEST_EXPECT(bar.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  backend_state.fail_destroy = 1;
  rc = bar.widget.vtable->destroy(&bar);
  CMP_TEST_ASSERT(rc == CMP_ERR_IO);
  backend_state.fail_destroy = 0;

  CMP_TEST_OK(m3_app_bar_init(&bar, &backend, &style, "Title", 5));
  CMP_TEST_OK(bar.widget.vtable->destroy(&bar));

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_app_bar_helpers() == 0);
  CMP_TEST_ASSERT(test_app_bar_style_init() == 0);
  CMP_TEST_ASSERT(test_app_bar_init_and_setters() == 0);
  CMP_TEST_ASSERT(test_app_bar_scroll() == 0);
  CMP_TEST_ASSERT(test_app_bar_widget() == 0);
  return 0;
}
