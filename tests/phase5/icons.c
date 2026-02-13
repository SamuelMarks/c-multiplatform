#include "cmpc/cmp_icon.h"
#include "test_utils.h"

#include <string.h>

#define CMP_ICON_TEST_FAIL_NONE 0u
#define CMP_ICON_TEST_FAIL_PATH_INIT 1u
#define CMP_ICON_TEST_FAIL_PATH_APPEND 2u

typedef struct TestTextBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int fail_create;
  int fail_measure;
  int fail_draw;
  int fail_destroy;
  CMPHandle last_font;
  cmp_i32 last_size;
  cmp_i32 last_weight;
  CMPBool last_italic;
  char last_family[64];
  CMPScalar last_x;
  CMPScalar last_y;
  cmp_usize last_len;
  CMPColor last_color;
  CMPScalar metric_width;
  CMPScalar metric_height;
  CMPScalar metric_baseline;
} TestTextBackend;

typedef struct TestGfxBackend {
  int draw_path_calls;
  int fail_draw_path;
  cmp_usize cmd_count;
  CMPPathCmd cmds[32];
  CMPColor last_color;
} TestGfxBackend;

static void test_text_backend_init(TestTextBackend *backend) {
  memset(backend, 0, sizeof(*backend));
  backend->metric_width = 12.0f;
  backend->metric_height = 16.0f;
  backend->metric_baseline = 11.0f;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestTextBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }

  backend = (TestTextBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return CMP_ERR_IO;
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
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  TestTextBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTextBackend *)text;
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
  TestTextBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestTextBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }

  *out_width = backend->metric_width;
  *out_height = backend->metric_height;
  *out_baseline = backend->metric_baseline;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestTextBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestTextBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw) {
    return CMP_ERR_IO;
  }

  backend->last_x = x;
  backend->last_y = y;
  backend->last_len = utf8_len;
  backend->last_color = color;
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const CMPTextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font, NULL, test_text_measure_text, test_text_draw_text};

static int test_gfx_draw_path(void *gfx, const CMPPath *path, CMPColor color) {
  TestGfxBackend *backend;
  cmp_usize copy_count;

  if (gfx == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestGfxBackend *)gfx;
  backend->draw_path_calls += 1;
  backend->last_color = color;
  if (backend->fail_draw_path) {
    return CMP_ERR_IO;
  }

  backend->cmd_count = path->count;
  copy_count = path->count;
  if (copy_count > CMP_COUNTOF(backend->cmds)) {
    copy_count = CMP_COUNTOF(backend->cmds);
  }
  if (copy_count > 0 && path->commands == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (copy_count > 0) {
    memcpy(backend->cmds, path->commands,
           copy_count * sizeof(backend->cmds[0]));
  }
  return CMP_OK;
}

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, NULL, NULL, test_gfx_draw_path, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_path = {
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

static void test_gfx_backend_init(TestGfxBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

int main(void) {
  CMPIconStyle style;
  CMPIconStyle bad_style;
  CMPIconSvg svg;
  CMPIconSvg svg_bad;
  CMPIconMetrics metrics;
  CMPGfx gfx;
  TestTextBackend text_backend;
  TestGfxBackend gfx_backend;
  CMPRect bounds;
  CMPColor color;
  const char *name;
  cmp_usize name_len;
  CMPScalar expected_x;
  CMPScalar expected_y;

  CMP_TEST_EXPECT(cmp_icon_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_icon_style_init(&style));
  CMP_TEST_ASSERT(style.utf8_family != NULL);
  CMP_TEST_ASSERT(strcmp(style.utf8_family, CMP_ICON_DEFAULT_FAMILY) == 0);
  CMP_TEST_ASSERT(style.size_px == CMP_ICON_DEFAULT_SIZE_PX);
  CMP_TEST_ASSERT(style.weight == CMP_ICON_DEFAULT_WEIGHT);
  CMP_TEST_ASSERT(style.italic == CMP_FALSE);
  CMP_TEST_ASSERT(style.color.r == 0.0f);
  CMP_TEST_ASSERT(style.color.g == 0.0f);
  CMP_TEST_ASSERT(style.color.b == 0.0f);
  CMP_TEST_ASSERT(style.color.a == 1.0f);

  CMP_TEST_EXPECT(cmp_icon_svg_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_icon_svg_init(&svg));
  CMP_TEST_ASSERT(svg.utf8_path == NULL);
  CMP_TEST_ASSERT(svg.viewbox_x == 0.0f);
  CMP_TEST_ASSERT(svg.viewbox_y == 0.0f);
  CMP_TEST_ASSERT(svg.viewbox_width == 24.0f);
  CMP_TEST_ASSERT(svg.viewbox_height == 24.0f);

  CMP_TEST_EXPECT(cmp_icon_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(cmp_icon_test_validate_color(&color));

  CMP_TEST_EXPECT(cmp_icon_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_icon_test_validate_style(&bad_style),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_icon_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.weight = 50;
  CMP_TEST_EXPECT(cmp_icon_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.italic = 2;
  CMP_TEST_EXPECT(cmp_icon_test_validate_style(&bad_style), CMP_ERR_RANGE);
  bad_style = style;
  bad_style.color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_style(&bad_style), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(cmp_icon_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.height = 10.0f;
  CMP_TEST_OK(cmp_icon_test_validate_rect(&bounds));

  CMP_TEST_EXPECT(cmp_icon_test_validate_svg(NULL), CMP_ERR_INVALID_ARGUMENT);
  svg_bad = svg;
  svg_bad.utf8_path = NULL;
  CMP_TEST_EXPECT(cmp_icon_test_validate_svg(&svg_bad),
                  CMP_ERR_INVALID_ARGUMENT);
  svg_bad = svg;
  svg_bad.utf8_path = "M0 0";
  svg_bad.viewbox_width = 0.0f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_svg(&svg_bad), CMP_ERR_RANGE);
  svg_bad.viewbox_width = 24.0f;
  svg_bad.viewbox_height = 0.0f;
  CMP_TEST_EXPECT(cmp_icon_test_validate_svg(&svg_bad), CMP_ERR_RANGE);

  gfx.ctx = NULL;
  gfx.vtable = NULL;
  gfx.text_vtable = NULL;
  CMP_TEST_OK(cmp_icon_test_set_cstr_limit(3));
  CMP_TEST_EXPECT(cmp_icon_measure_cstr(&gfx, &style, "four", &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_icon_test_set_cstr_limit(0));

  test_text_backend_init(&text_backend);
  gfx.ctx = &text_backend;
  gfx.vtable = NULL;
  gfx.text_vtable = &g_test_text_vtable;
  name = "home";
  name_len = 4u;
  CMP_TEST_OK(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                    CMP_ICON_RENDER_AUTO, &metrics));
  CMP_TEST_ASSERT(metrics.width == text_backend.metric_width);
  CMP_TEST_ASSERT(metrics.height == text_backend.metric_height);
  CMP_TEST_ASSERT(metrics.baseline == text_backend.metric_baseline);
  CMP_TEST_ASSERT(text_backend.create_calls == 1);
  CMP_TEST_ASSERT(text_backend.measure_calls == 1);
  CMP_TEST_ASSERT(text_backend.destroy_calls == 1);

  CMP_TEST_OK(cmp_icon_measure_cstr(&gfx, &style, name, &svg,
                                    CMP_ICON_RENDER_AUTO, &metrics));
  CMP_TEST_ASSERT(text_backend.create_calls == 2);

  bounds.x = 5.0f;
  bounds.y = 10.0f;
  bounds.width = 30.0f;
  bounds.height = 20.0f;
  style.color.r = 0.1f;
  style.color.g = 0.2f;
  style.color.b = 0.3f;
  style.color.a = 0.4f;
  CMP_TEST_OK(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len, &svg,
                                 CMP_ICON_RENDER_AUTO));
  expected_x = bounds.x + (bounds.width - metrics.width) * 0.5f;
  expected_y =
      bounds.y + (bounds.height - metrics.height) * 0.5f + metrics.baseline;
  CMP_TEST_ASSERT(cmp_near(text_backend.last_x, expected_x, 0.001f));
  CMP_TEST_ASSERT(cmp_near(text_backend.last_y, expected_y, 0.001f));
  CMP_TEST_ASSERT(text_backend.last_len == name_len);
  CMP_TEST_ASSERT(text_backend.last_color.r == style.color.r);
  CMP_TEST_ASSERT(text_backend.last_color.g == style.color.g);
  CMP_TEST_ASSERT(text_backend.last_color.b == style.color.b);
  CMP_TEST_ASSERT(text_backend.last_color.a == style.color.a);

  CMP_TEST_EXPECT(cmp_icon_draw_cstr(&gfx, &bounds, &style, NULL, &svg,
                                     CMP_ICON_RENDER_FONT),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_icon_test_set_cstr_limit(3));
  CMP_TEST_EXPECT(cmp_icon_draw_cstr(&gfx, &bounds, &style, "four", &svg,
                                     CMP_ICON_RENDER_FONT),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_icon_test_set_cstr_limit(0));
  CMP_TEST_OK(cmp_icon_draw_cstr(&gfx, &bounds, &style, name, &svg,
                                 CMP_ICON_RENDER_FONT));

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_UNSUPPORTED);

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len,
                                     &svg, CMP_ICON_RENDER_FONT),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_UNSUPPORTED);

  test_text_backend_init(&text_backend);
  gfx.text_vtable = &g_test_text_vtable;
  gfx.ctx = &text_backend;
  text_backend.fail_create = 1;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_IO);
  text_backend.fail_create = 0;
  text_backend.fail_measure = 1;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_IO);
  CMP_TEST_ASSERT(text_backend.destroy_calls == 1);
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len,
                                     &svg, CMP_ICON_RENDER_FONT),
                  CMP_ERR_IO);
  CMP_TEST_ASSERT(text_backend.destroy_calls == 2);
  text_backend.fail_measure = 0;
  text_backend.fail_draw = 1;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len,
                                     &svg, CMP_ICON_RENDER_FONT),
                  CMP_ERR_IO);
  CMP_TEST_ASSERT(text_backend.destroy_calls == 3);
  text_backend.fail_draw = 0;
  text_backend.fail_destroy = 1;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_IO);
  text_backend.fail_destroy = 0;
  text_backend.metric_width = -1.0f;
  CMP_TEST_EXPECT(cmp_icon_measure_utf8(&gfx, &style, name, name_len, &svg,
                                        CMP_ICON_RENDER_FONT, &metrics),
                  CMP_ERR_RANGE);
  text_backend.metric_width = 12.0f;
  text_backend.metric_height = -1.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len,
                                     &svg, CMP_ICON_RENDER_FONT),
                  CMP_ERR_RANGE);
  text_backend.metric_height = 16.0f;

  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, name, name_len,
                                     &svg, CMP_ICON_RENDER_SVG + 1u),
                  CMP_ERR_INVALID_ARGUMENT);

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
  CMP_TEST_OK(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                 CMP_ICON_RENDER_AUTO));
  CMP_TEST_ASSERT(gfx_backend.draw_path_calls == 1);
  CMP_TEST_ASSERT(gfx_backend.cmd_count == 4u);
  CMP_TEST_ASSERT(gfx_backend.cmds[0].type == CMP_PATH_CMD_MOVE_TO);
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[0].data.move_to.x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[0].data.move_to.y, 0.0f, 0.001f));
  CMP_TEST_ASSERT(gfx_backend.cmds[1].type == CMP_PATH_CMD_LINE_TO);
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[1].data.line_to.x, 15.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[1].data.line_to.y, 0.0f, 0.001f));

  svg.utf8_path = "M0 0 C0 0 10 0 10 10 S20 20 30 10 Q40 0 50 10 T60 10 Z";
  svg.viewbox_width = 60.0f;
  svg.viewbox_height = 20.0f;
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 60.0f;
  bounds.height = 20.0f;
  CMP_TEST_OK(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                 CMP_ICON_RENDER_SVG));
  CMP_TEST_ASSERT(gfx_backend.cmd_count == 6u);
  CMP_TEST_ASSERT(gfx_backend.cmds[1].type == CMP_PATH_CMD_CUBIC_TO);
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[1].data.cubic_to.cx2, 10.0f, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[1].data.cubic_to.cy2, 0.0f, 0.001f));
  CMP_TEST_ASSERT(gfx_backend.cmds[2].type == CMP_PATH_CMD_CUBIC_TO);
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[2].data.cubic_to.cx1, 10.0f, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[2].data.cubic_to.cy1, 20.0f, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[2].data.cubic_to.cx2, 20.0f, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(gfx_backend.cmds[2].data.cubic_to.cy2, 20.0f, 0.001f));
  CMP_TEST_ASSERT(gfx_backend.cmds[3].type == CMP_PATH_CMD_QUAD_TO);
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[3].data.quad_to.cx, 40.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[3].data.quad_to.cy, 0.0f, 0.001f));
  CMP_TEST_ASSERT(gfx_backend.cmds[4].type == CMP_PATH_CMD_QUAD_TO);
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[4].data.quad_to.cx, 60.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(gfx_backend.cmds[4].data.quad_to.cy, 20.0f, 0.001f));

  svg.utf8_path = "m5 5 l5 0 h5 v5 c0 5 5 5 5 0 s5 -5 10 0 q5 5 10 0 t10 0 z";
  svg.viewbox_width = 40.0f;
  svg.viewbox_height = 20.0f;
  bounds.width = 40.0f;
  bounds.height = 20.0f;
  CMP_TEST_OK(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                 CMP_ICON_RENDER_SVG));

  svg.utf8_path = "M0 0 A10 10 0 0 0 10 10";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_UNSUPPORTED);

  svg.utf8_path = "0 0 L10 10";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_CORRUPT);

  svg.utf8_path = "M0 0 L10";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_CORRUPT);

  svg.utf8_path = "M0 0";
  svg.viewbox_width = 0.0f;
  svg.viewbox_height = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_icon_test_set_fail_point(CMP_ICON_TEST_FAIL_PATH_INIT));
  svg.viewbox_width = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_icon_test_clear_fail_points());

  CMP_TEST_OK(cmp_icon_test_set_fail_point(CMP_ICON_TEST_FAIL_PATH_APPEND));
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_IO);
  CMP_TEST_OK(cmp_icon_test_clear_fail_points());

  svg.utf8_path = "M1e309 0";
  svg.viewbox_width = 20.0f;
  svg.viewbox_height = 20.0f;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_RANGE);

  svg.utf8_path = "Mnan 0";
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_RANGE);

  gfx.vtable = &g_test_gfx_vtable_no_path;
  CMP_TEST_EXPECT(cmp_icon_draw_utf8(&gfx, &bounds, &style, NULL, 0u, &svg,
                                     CMP_ICON_RENDER_SVG),
                  CMP_ERR_UNSUPPORTED);

  gfx.vtable = &g_test_gfx_vtable;
  svg.utf8_path = "M0 0 L10 10";
  svg.viewbox_width = 10.0f;
  svg.viewbox_height = 20.0f;
  style.size_px = 20;
  CMP_TEST_OK(cmp_icon_measure_utf8(&gfx, &style, NULL, 0u, &svg,
                                    CMP_ICON_RENDER_SVG, &metrics));
  CMP_TEST_ASSERT(cmp_near(metrics.width, 10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(metrics.height, 20.0f, 0.001f));

  return 0;
}
