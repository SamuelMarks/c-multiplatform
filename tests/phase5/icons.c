#include "m3/m3_icon.h"
#include "test_utils.h"

#include <string.h>

#define M3_ICON_TEST_FAIL_NONE 0u
#define M3_ICON_TEST_FAIL_PATH_INIT 1u
#define M3_ICON_TEST_FAIL_PATH_APPEND 2u

typedef struct TestTextBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int fail_create;
  int fail_measure;
  int fail_draw;
  int fail_destroy;
  M3Handle last_font;
  m3_i32 last_size;
  m3_i32 last_weight;
  M3Bool last_italic;
  char last_family[64];
  M3Scalar last_x;
  M3Scalar last_y;
  m3_usize last_len;
  M3Color last_color;
  M3Scalar metric_width;
  M3Scalar metric_height;
  M3Scalar metric_baseline;
} TestTextBackend;

typedef struct TestGfxBackend {
  int draw_path_calls;
  int fail_draw_path;
  m3_usize cmd_count;
  M3PathCmd cmds[32];
  M3Color last_color;
} TestGfxBackend;

static void test_text_backend_init(TestTextBackend *backend) {
  memset(backend, 0, sizeof(*backend));
  backend->metric_width = 12.0f;
  backend->metric_height = 16.0f;
  backend->metric_baseline = 11.0f;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestTextBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return M3_ERR_RANGE;
  }

  backend = (TestTextBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return M3_ERR_IO;
  }

  backend->last_size = size_px;
  backend->last_weight = weight;
  backend->last_italic = italic;
  {
    size_t len = strlen(utf8_family);
    if (len >= sizeof(backend->last_family)) {
      len = sizeof(backend->last_family) - 1;
    }
    memcpy(backend->last_family, utf8_family, len);
    backend->last_family[len] = '\0';
  }

  out_font->id = 1u;
  out_font->generation = 1u;
  backend->last_font = *out_font;
  return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font) {
  TestTextBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTextBackend *)text;
  backend->destroy_calls += 1;
  if (backend->fail_destroy) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_text_measure_text(void *text, M3Handle font, const char *utf8,
                                  m3_usize utf8_len, M3Scalar *out_width,
                                  M3Scalar *out_height,
                                  M3Scalar *out_baseline) {
  TestTextBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestTextBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return M3_ERR_IO;
  }

  *out_width = backend->metric_width;
  *out_height = backend->metric_height;
  *out_baseline = backend->metric_baseline;
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestTextBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestTextBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw) {
    return M3_ERR_IO;
  }

  backend->last_x = x;
  backend->last_y = y;
  backend->last_len = utf8_len;
  backend->last_color = color;
  return M3_OK;
}

static const M3TextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const M3TextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font, NULL, test_text_measure_text, test_text_draw_text};

static int test_gfx_draw_path(void *gfx, const M3Path *path, M3Color color) {
  TestGfxBackend *backend;
  m3_usize copy_count;

  if (gfx == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->draw_path_calls += 1;
  backend->last_color = color;
  if (backend->fail_draw_path) {
    return M3_ERR_IO;
  }

  backend->cmd_count = path->count;
  copy_count = path->count;
  if (copy_count > M3_COUNTOF(backend->cmds)) {
    copy_count = M3_COUNTOF(backend->cmds);
  }
  if (copy_count > 0 && path->commands == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (copy_count > 0) {
    memcpy(backend->cmds, path->commands,
           copy_count * sizeof(backend->cmds[0]));
  }
  return M3_OK;
}

static const M3GfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, NULL, NULL, test_gfx_draw_path, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_gfx_vtable_no_path = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static void test_gfx_backend_init(TestGfxBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

int main(void) {
  M3IconStyle style;
  M3IconStyle bad_style;
  M3IconSvg svg;
  M3IconSvg svg_bad;
  M3IconMetrics metrics;
  M3Gfx gfx;
  TestTextBackend text_backend;
  TestGfxBackend gfx_backend;
  M3Rect bounds;
  M3Color color;
  const char *name;
  m3_usize name_len;
  M3Scalar expected_x;
  M3Scalar expected_y;

  M3_TEST_EXPECT(m3_icon_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_icon_style_init(&style));
  M3_TEST_ASSERT(style.utf8_family != NULL);
  M3_TEST_ASSERT(strcmp(style.utf8_family, M3_ICON_DEFAULT_FAMILY) == 0);
  M3_TEST_ASSERT(style.size_px == M3_ICON_DEFAULT_SIZE_PX);
  M3_TEST_ASSERT(style.weight == M3_ICON_DEFAULT_WEIGHT);
  M3_TEST_ASSERT(style.italic == M3_FALSE);
  M3_TEST_ASSERT(style.color.r == 0.0f);
  M3_TEST_ASSERT(style.color.g == 0.0f);
  M3_TEST_ASSERT(style.color.b == 0.0f);
  M3_TEST_ASSERT(style.color.a == 1.0f);

  M3_TEST_EXPECT(m3_icon_svg_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_icon_svg_init(&svg));
  M3_TEST_ASSERT(svg.utf8_path == NULL);
  M3_TEST_ASSERT(svg.viewbox_x == 0.0f);
  M3_TEST_ASSERT(svg.viewbox_y == 0.0f);
  M3_TEST_ASSERT(svg.viewbox_width == 24.0f);
  M3_TEST_ASSERT(svg.viewbox_height == 24.0f);

  M3_TEST_EXPECT(m3_icon_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_EXPECT(m3_icon_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  M3_TEST_EXPECT(m3_icon_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  M3_TEST_EXPECT(m3_icon_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  M3_TEST_EXPECT(m3_icon_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_icon_test_validate_color(&color));

  M3_TEST_EXPECT(m3_icon_test_validate_style(NULL), M3_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_icon_test_validate_style(&bad_style),
                 M3_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.size_px = 0;
  M3_TEST_EXPECT(m3_icon_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.weight = 50;
  M3_TEST_EXPECT(m3_icon_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.italic = 2;
  M3_TEST_EXPECT(m3_icon_test_validate_style(&bad_style), M3_ERR_RANGE);
  bad_style = style;
  bad_style.color.r = -0.1f;
  M3_TEST_EXPECT(m3_icon_test_validate_style(&bad_style), M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_icon_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(m3_icon_test_validate_rect(&bounds), M3_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  M3_TEST_EXPECT(m3_icon_test_validate_rect(&bounds), M3_ERR_RANGE);
  bounds.height = 10.0f;
  M3_TEST_OK(m3_icon_test_validate_rect(&bounds));

  M3_TEST_EXPECT(m3_icon_test_validate_svg(NULL), M3_ERR_INVALID_ARGUMENT);
  svg_bad = svg;
  svg_bad.utf8_path = NULL;
  M3_TEST_EXPECT(m3_icon_test_validate_svg(&svg_bad), M3_ERR_INVALID_ARGUMENT);
  svg_bad = svg;
  svg_bad.utf8_path = "M0 0";
  svg_bad.viewbox_width = 0.0f;
  M3_TEST_EXPECT(m3_icon_test_validate_svg(&svg_bad), M3_ERR_RANGE);
  svg_bad.viewbox_width = 24.0f;
  svg_bad.viewbox_height = 0.0f;
  M3_TEST_EXPECT(m3_icon_test_validate_svg(&svg_bad), M3_ERR_RANGE);

  gfx.ctx = NULL;
  gfx.vtable = NULL;
  gfx.text_vtable = NULL;
  M3_TEST_OK(m3_icon_test_set_cstr_limit(3));
  M3_TEST_EXPECT(m3_icon_measure_cstr(&gfx, &style, "four", &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_icon_test_set_cstr_limit(0));

  test_text_backend_init(&text_backend);
  gfx.ctx = &text_backend;
  gfx.vtable = NULL;
  gfx.text_vtable = &g_test_text_vtable;
  name = "home";
  name_len = 4u;
  M3_TEST_OK(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                  M3_ICON_RENDER_AUTO, &metrics));
  M3_TEST_ASSERT(metrics.width == text_backend.metric_width);
  M3_TEST_ASSERT(metrics.height == text_backend.metric_height);
  M3_TEST_ASSERT(metrics.baseline == text_backend.metric_baseline);
  M3_TEST_ASSERT(text_backend.create_calls == 1);
  M3_TEST_ASSERT(text_backend.measure_calls == 1);
  M3_TEST_ASSERT(text_backend.destroy_calls == 1);

  M3_TEST_OK(m3_icon_measure_cstr(&gfx, &style, name, &svg, M3_ICON_RENDER_AUTO,
                                  &metrics));
  M3_TEST_ASSERT(text_backend.create_calls == 2);

  bounds.x = 5.0f;
  bounds.y = 10.0f;
  bounds.width = 30.0f;
  bounds.height = 20.0f;
  style.color.r = 0.1f;
  style.color.g = 0.2f;
  style.color.b = 0.3f;
  style.color.a = 0.4f;
  M3_TEST_OK(m3_icon_draw_utf8(&gfx, &bounds, &style, name, name_len, &svg,
                               M3_ICON_RENDER_AUTO));
  expected_x = bounds.x + (bounds.width - metrics.width) * 0.5f;
  expected_y =
      bounds.y + (bounds.height - metrics.height) * 0.5f + metrics.baseline;
  M3_TEST_ASSERT(m3_near(text_backend.last_x, expected_x, 0.001f));
  M3_TEST_ASSERT(m3_near(text_backend.last_y, expected_y, 0.001f));
  M3_TEST_ASSERT(text_backend.last_len == name_len);
  M3_TEST_ASSERT(text_backend.last_color.r == style.color.r);
  M3_TEST_ASSERT(text_backend.last_color.g == style.color.g);
  M3_TEST_ASSERT(text_backend.last_color.b == style.color.b);
  M3_TEST_ASSERT(text_backend.last_color.a == style.color.a);

  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_UNSUPPORTED);

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, name, name_len, &svg,
                                   M3_ICON_RENDER_FONT),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable_no_measure;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable_no_destroy;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_UNSUPPORTED);

  test_text_backend_init(&text_backend);
  gfx.text_vtable = &g_test_text_vtable;
  gfx.ctx = &text_backend;
  text_backend.fail_create = 1;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_IO);
  text_backend.fail_create = 0;
  text_backend.fail_measure = 1;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_IO);
  M3_TEST_ASSERT(text_backend.destroy_calls == 1);
  text_backend.fail_measure = 0;
  text_backend.fail_draw = 1;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, name, name_len, &svg,
                                   M3_ICON_RENDER_FONT),
                 M3_ERR_IO);
  M3_TEST_ASSERT(text_backend.destroy_calls == 2);
  text_backend.fail_draw = 0;
  text_backend.fail_destroy = 1;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_IO);
  text_backend.fail_destroy = 0;
  text_backend.metric_width = -1.0f;
  M3_TEST_EXPECT(m3_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                      M3_ICON_RENDER_FONT, &metrics),
                 M3_ERR_RANGE);
  text_backend.metric_width = 12.0f;
  text_backend.metric_height = -1.0f;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, name, name_len, &svg,
                                   M3_ICON_RENDER_FONT),
                 M3_ERR_RANGE);
  text_backend.metric_height = 16.0f;

  test_gfx_backend_init(&gfx_backend);
  gfx.ctx = &gfx_backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;
  svg.utf8_path = "M0 0 L10 0 L10 10 Z";
  svg.viewbox_x = 0.0f;
  svg.viewbox_y = 0.0f;
  svg.viewbox_width = 10.0f;
  svg.viewbox_height = 10.0f;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 20.0f;
  bounds.height = 10.0f;
  M3_TEST_OK(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                               M3_ICON_RENDER_AUTO));
  M3_TEST_ASSERT(gfx_backend.draw_path_calls == 1);
  M3_TEST_ASSERT(gfx_backend.cmd_count == 4u);
  M3_TEST_ASSERT(gfx_backend.cmds[0].type == M3_PATH_CMD_MOVE_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[0].data.move_to.x, 5.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[0].data.move_to.y, 0.0f, 0.001f));
  M3_TEST_ASSERT(gfx_backend.cmds[1].type == M3_PATH_CMD_LINE_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[1].data.line_to.x, 15.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[1].data.line_to.y, 0.0f, 0.001f));

  svg.utf8_path = "M0 0 C0 0 10 0 10 10 S20 20 30 10 Q40 0 50 10 T60 10 Z";
  svg.viewbox_width = 60.0f;
  svg.viewbox_height = 20.0f;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 60.0f;
  bounds.height = 20.0f;
  M3_TEST_OK(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                               M3_ICON_RENDER_SVG));
  M3_TEST_ASSERT(gfx_backend.cmd_count == 6u);
  M3_TEST_ASSERT(gfx_backend.cmds[1].type == M3_PATH_CMD_CUBIC_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[1].data.cubic_to.cx2, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[1].data.cubic_to.cy2, 0.0f, 0.001f));
  M3_TEST_ASSERT(gfx_backend.cmds[2].type == M3_PATH_CMD_CUBIC_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[2].data.cubic_to.cx1, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[2].data.cubic_to.cy1, 20.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[2].data.cubic_to.cx2, 20.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[2].data.cubic_to.cy2, 20.0f, 0.001f));
  M3_TEST_ASSERT(gfx_backend.cmds[3].type == M3_PATH_CMD_QUAD_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[3].data.quad_to.cx, 40.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[3].data.quad_to.cy, 0.0f, 0.001f));
  M3_TEST_ASSERT(gfx_backend.cmds[4].type == M3_PATH_CMD_QUAD_TO);
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[4].data.quad_to.cx, 60.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(gfx_backend.cmds[4].data.quad_to.cy, 20.0f, 0.001f));

  svg.utf8_path = "m5 5 l5 0 h5 v5 c0 5 5 5 5 0 s5 -5 10 0 q5 5 10 0 t10 0 z";
  svg.viewbox_width = 40.0f;
  svg.viewbox_height = 20.0f;
  bounds.width = 40.0f;
  bounds.height = 20.0f;
  M3_TEST_OK(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                               M3_ICON_RENDER_SVG));

  svg.utf8_path = "M0 0 A10 10 0 0 0 10 10";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_UNSUPPORTED);

  svg.utf8_path = "M0 0 L10";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_CORRUPT);

  svg.utf8_path = "M0 0";
  svg.viewbox_width = 0.0f;
  svg.viewbox_height = 20.0f;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_icon_test_set_fail_point(M3_ICON_TEST_FAIL_PATH_INIT));
  svg.viewbox_width = 20.0f;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_IO);
  M3_TEST_OK(m3_icon_test_clear_fail_points());

  M3_TEST_OK(m3_icon_test_set_fail_point(M3_ICON_TEST_FAIL_PATH_APPEND));
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_IO);
  M3_TEST_OK(m3_icon_test_clear_fail_points());

  gfx.vtable = &g_test_gfx_vtable_no_path;
  M3_TEST_EXPECT(m3_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                   M3_ICON_RENDER_SVG),
                 M3_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_gfx_vtable;
  svg.utf8_path = "M0 0 L10 10";
  svg.viewbox_width = 10.0f;
  svg.viewbox_height = 20.0f;
  style.size_px = 20;
  M3_TEST_OK(m3_icon_measure_utf8(&gfx, &style, NULL, 0u, &svg,
                                  M3_ICON_RENDER_SVG, &metrics));
  M3_TEST_ASSERT(m3_near(metrics.width, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(metrics.height, 20.0f, 0.001f));

  return 0;
}
