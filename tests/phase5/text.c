#include "cmpc/cmp_text.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestTextBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int fail_create;
  int fail_measure;
  int fail_draw;
  int negative_metrics;
  CMPHandle last_font;
  cmp_i32 last_size;
  cmp_i32 last_weight;
  CMPBool last_italic;
  char last_family[64];
  CMPScalar last_x;
  CMPScalar last_y;
  cmp_usize last_len;
  CMPColor last_color;
} TestTextBackend;

static void test_text_backend_init(TestTextBackend *backend) {
  memset(backend, 0, sizeof(*backend));
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
    size_t family_len = strlen(utf8_family);
    if (family_len >= sizeof(backend->last_family)) {
      family_len = sizeof(backend->last_family) - 1;
    }
    memcpy(backend->last_family, utf8_family, family_len);
    backend->last_family[family_len] = '\0';
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

  if (backend->negative_metrics) {
    *out_width = -1.0f;
    *out_height = 20.0f;
    *out_baseline = 15.0f;
  } else {
    *out_width = (CMPScalar)(utf8_len * 10u);
    *out_height = 20.0f;
    *out_baseline = 15.0f;
  }
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

static const CMPTextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

int main(void) {
  TestTextBackend backend;
  CMPTextBackend text_backend;
  CMPTextBackend alt_backend;
  CMPTextBackend invalid_backend;
  CMPGfx gfx;
  CMPTextStyle style;
  CMPTextStyle alt_style;
  CMPTextMetrics metrics;
  CMPTextWidget widget;
  CMPTextWidget temp_widget;
  CMPPaintContext paint_ctx;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPSemantics semantics;
  CMPHandle font;
  CMPHandle saved_font;
  CMPColor color;
  cmp_usize cstr_len;
  const CMPWidgetVTable *text_vtable;
  CMPInputEvent event;
  CMPBool handled;
  int rc;

  test_text_backend_init(&backend);
  memset(&event, 0, sizeof(event));
  gfx.ctx = &backend;
  gfx.vtable = NULL;
  gfx.text_vtable = &g_test_text_vtable;

  CMP_TEST_EXPECT(cmp_text_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&style));
  CMP_TEST_ASSERT(style.size_px == 14);
  CMP_TEST_ASSERT(style.weight == 400);
  CMP_TEST_ASSERT(style.italic == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_text_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  color = style.color;
  color.r = -0.1f;
  CMP_TEST_EXPECT(cmp_text_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(cmp_text_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  CMP_TEST_EXPECT(cmp_text_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = -0.2f;
  CMP_TEST_EXPECT(cmp_text_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(cmp_text_test_validate_color(&color));

  CMP_TEST_EXPECT(cmp_text_test_validate_style(NULL), CMP_ERR_INVALID_ARGUMENT);
  alt_style = style;
  alt_style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_text_test_validate_style(&alt_style),
                  CMP_ERR_INVALID_ARGUMENT);
  alt_style.utf8_family = "Test";
  alt_style.size_px = 0;
  CMP_TEST_EXPECT(cmp_text_test_validate_style(&alt_style), CMP_ERR_RANGE);
  alt_style.size_px = 16;
  alt_style.weight = 99;
  CMP_TEST_EXPECT(cmp_text_test_validate_style(&alt_style), CMP_ERR_RANGE);
  alt_style.weight = 400;
  alt_style.italic = 2;
  CMP_TEST_EXPECT(cmp_text_test_validate_style(&alt_style), CMP_ERR_RANGE);
  alt_style.italic = CMP_FALSE;
  alt_style.color.a = 1.5f;
  CMP_TEST_EXPECT(cmp_text_test_validate_style(&alt_style), CMP_ERR_RANGE);
  alt_style.color.a = 1.0f;
  CMP_TEST_OK(cmp_text_test_validate_style(&alt_style));

  CMP_TEST_EXPECT(cmp_text_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  alt_backend.ctx = &backend;
  alt_backend.vtable = NULL;
  CMP_TEST_EXPECT(cmp_text_test_validate_backend(&alt_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  alt_backend.vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_test_validate_backend(&alt_backend));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  CMP_TEST_EXPECT(cmp_text_test_validate_measure_spec(width_spec),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(cmp_text_test_validate_measure_spec(width_spec),
                  CMP_ERR_RANGE);
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = -1.0f;
  CMP_TEST_OK(cmp_text_test_validate_measure_spec(width_spec));

  CMP_TEST_EXPECT(cmp_text_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 5.0f;
  CMP_TEST_EXPECT(cmp_text_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.width = 1.0f;
  bounds.height = -2.0f;
  CMP_TEST_EXPECT(cmp_text_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.height = 2.0f;
  CMP_TEST_OK(cmp_text_test_validate_rect(&bounds));

  CMP_TEST_EXPECT(cmp_text_backend_from_gfx(NULL, &text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(cmp_text_backend_from_gfx(&gfx, &text_backend),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;
  CMP_TEST_OK(cmp_text_backend_from_gfx(&gfx, &text_backend));

  CMP_TEST_EXPECT(cmp_text_font_create(NULL, &style, &font),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, NULL, &font),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  memset(&invalid_backend, 0, sizeof(invalid_backend));
  CMP_TEST_EXPECT(cmp_text_font_create(&invalid_backend, &style, &font),
                  CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, &font),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  style.utf8_family = NULL;
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, &font),
                  CMP_ERR_INVALID_ARGUMENT);
  style.utf8_family = "Test";
  style.size_px = 0;
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, &font),
                  CMP_ERR_RANGE);
  style.size_px = 16;
  style.weight = 50;
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, &font),
                  CMP_ERR_RANGE);
  style.weight = 400;
  style.color.a = 1.2f;
  CMP_TEST_EXPECT(cmp_text_font_create(&text_backend, &style, &font),
                  CMP_ERR_RANGE);
  style.color.a = 1.0f;
  CMP_TEST_OK(cmp_text_font_create(&text_backend, &style, &font));
  CMP_TEST_ASSERT(backend.create_calls == 1);
  saved_font = font;

  CMP_TEST_EXPECT(cmp_text_font_destroy(NULL, font), CMP_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(cmp_text_font_destroy(&text_backend, saved_font),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;
  font.id = 0u;
  font.generation = 0u;
  CMP_TEST_OK(cmp_text_font_destroy(&text_backend, font));
  font = saved_font;

  CMP_TEST_EXPECT(cmp_text_measure_utf8(NULL, font, "abc", 3, &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_text_measure_utf8(&invalid_backend, font, "abc", 3, &metrics),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_measure_utf8(&text_backend, font, NULL, 3, &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_measure_utf8(&text_backend, font, "abc", 3, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(
      cmp_text_measure_utf8(&text_backend, font, "abc", 3, &metrics),
      CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  backend.fail_measure = 1;
  CMP_TEST_EXPECT(
      cmp_text_measure_utf8(&text_backend, font, "abc", 3, &metrics),
      CMP_ERR_IO);
  backend.fail_measure = 0;

  backend.negative_metrics = 1;
  CMP_TEST_EXPECT(
      cmp_text_measure_utf8(&text_backend, font, "abc", 3, &metrics),
      CMP_ERR_RANGE);
  backend.negative_metrics = 0;

  CMP_TEST_OK(cmp_text_measure_utf8(&text_backend, font, "abc", 3, &metrics));
  CMP_TEST_ASSERT(cmp_near(metrics.width, 30.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(metrics.height, 20.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(metrics.baseline, 15.0f, 0.001f));

  CMP_TEST_EXPECT(cmp_text_measure_cstr(NULL, font, "abc", &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_measure_cstr(&text_backend, font, NULL, &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_test_set_cstr_limit(2));
  CMP_TEST_EXPECT(cmp_text_measure_cstr(&text_backend, font, "abcd", &metrics),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_test_set_cstr_limit(0));

  CMP_TEST_OK(cmp_text_font_metrics(&text_backend, font, &metrics));
  CMP_TEST_ASSERT(cmp_near(metrics.width, 0.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(metrics.height, 20.0f, 0.001f));

  CMP_TEST_OK(cmp_text_measure_cstr(&text_backend, font, "", &metrics));
  CMP_TEST_ASSERT(cmp_near(metrics.width, 0.0f, 0.001f));

  CMP_TEST_EXPECT(cmp_text_test_cstrlen(NULL, &cstr_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_test_cstrlen("", NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_test_set_cstr_limit(2));
  CMP_TEST_EXPECT(cmp_text_test_cstrlen("abc", &cstr_len), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_text_test_set_cstr_limit(0));
  CMP_TEST_OK(cmp_text_test_cstrlen("abc", &cstr_len));
  CMP_TEST_ASSERT(cstr_len == 3u);

  CMP_TEST_OK(cmp_text_font_destroy(&text_backend, font));
  CMP_TEST_ASSERT(backend.destroy_calls == 1);

  CMP_TEST_EXPECT(cmp_text_widget_init(NULL, &text_backend, &style, "Hi", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, NULL, &style, "Hi", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, NULL, "Hi", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, NULL, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_text_widget_init(&widget, &invalid_backend, &style, "Hi", 2),
      CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  alt_style = style;
  alt_style.size_px = 0;
  CMP_TEST_EXPECT(
      cmp_text_widget_init(&widget, &text_backend, &alt_style, "Hi", 2),
      CMP_ERR_RANGE);
  backend.fail_create = 1;
  CMP_TEST_EXPECT(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2),
                  CMP_ERR_IO);
  backend.fail_create = 0;

  CMP_TEST_OK(cmp_text_widget_init(&widget, &text_backend, &style, "Hi", 2));
  text_vtable = widget.widget.vtable;
  CMP_TEST_ASSERT(text_vtable != NULL);

  temp_widget = widget;
  temp_widget.metrics_valid = CMP_TRUE;
  CMP_TEST_OK(cmp_text_test_metrics_update(&temp_widget));
  CMP_TEST_EXPECT(cmp_text_test_metrics_update(NULL), CMP_ERR_INVALID_ARGUMENT);

  temp_widget = widget;
  temp_widget.metrics_valid = CMP_FALSE;
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(cmp_text_test_metrics_update(&temp_widget), CMP_ERR_IO);
  backend.fail_measure = 0;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 10.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 20.0f, 0.001f));

  CMP_TEST_OK(cmp_text_widget_set_text(&widget, "Hello", 5));
  CMP_TEST_EXPECT(cmp_text_widget_set_text(NULL, "Hi", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_set_text(&widget, NULL, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 12.0f;
  CMP_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 50.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 12.0f, 0.001f));
  CMP_TEST_OK(cmp_text_widget_set_text(&widget, NULL, 0));
  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 42.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  CMP_TEST_OK(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 42.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 10.0f, 0.001f));

  bounds.x = 5.0f;
  bounds.y = 7.0f;
  bounds.width = 100.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(widget.widget.vtable->layout(widget.widget.ctx, bounds));

  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;
  CMP_TEST_OK(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_calls == 1);
  CMP_TEST_ASSERT(cmp_near(backend.last_x, 5.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(backend.last_y, 22.0f, 0.001f));

  CMP_TEST_OK(
      widget.widget.vtable->get_semantics(widget.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_TEXT);
  CMP_TEST_ASSERT(semantics.utf8_label == widget.utf8);

  CMP_TEST_EXPECT(
      widget.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = 42u;
  width_spec.size = 10.0f;
  CMP_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = -5.0f;
  CMP_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 10.0f;
  widget.metrics_valid = CMP_FALSE;
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(widget.widget.vtable->measure(widget.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_IO);
  backend.fail_measure = 0;

  bounds.width = -1.0f;
  CMP_TEST_EXPECT(widget.widget.vtable->layout(widget.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 100.0f;
  CMP_TEST_EXPECT(widget.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(widget.widget.vtable->layout(widget.widget.ctx, bounds));

  CMP_TEST_EXPECT(widget.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;
  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_measure = 0;
  backend.fail_draw = 1;
  CMP_TEST_EXPECT(widget.widget.vtable->paint(widget.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw = 0;

  CMP_TEST_EXPECT(widget.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      widget.widget.vtable->event(widget.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget.widget.vtable->event(widget.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(widget.widget.vtable->event(widget.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_EXPECT(widget.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(widget.widget.vtable->get_semantics(widget.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(widget.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  temp_widget = widget;
  temp_widget.backend.vtable = &g_test_text_vtable_no_destroy;
  temp_widget.owns_font = CMP_TRUE;
  temp_widget.font.id = 1u;
  temp_widget.font.generation = 1u;
  rc = text_vtable->destroy(&temp_widget);
  CMP_TEST_EXPECT(rc, CMP_ERR_UNSUPPORTED);
  CMP_TEST_ASSERT(temp_widget.font.id == 0u);

  temp_widget = widget;
  temp_widget.owns_font = CMP_FALSE;
  temp_widget.font.id = 1u;
  temp_widget.font.generation = 1u;
  CMP_TEST_OK(text_vtable->destroy(&temp_widget));

  alt_style = style;
  alt_style.weight = 50;
  CMP_TEST_EXPECT(cmp_text_widget_set_style(&widget, &alt_style),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_text_widget_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_set_style(&widget, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.fail_create = 1;
  CMP_TEST_EXPECT(cmp_text_widget_set_style(&widget, &style), CMP_ERR_IO);
  backend.fail_create = 0;
  CMP_TEST_OK(cmp_text_widget_set_style(&widget, &style));

  CMP_TEST_EXPECT(cmp_text_widget_get_metrics(NULL, &metrics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_text_widget_get_metrics(&widget, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(cmp_text_widget_get_metrics(&widget, &metrics), CMP_ERR_IO);
  backend.fail_measure = 0;
  CMP_TEST_OK(cmp_text_widget_get_metrics(&widget, &metrics));

  rc = widget.widget.vtable->destroy(widget.widget.ctx);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(backend.destroy_calls >= 1);

  return 0;
}
