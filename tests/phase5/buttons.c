#include "m3/m3_button.h"
#include "test_utils.h"

#include <string.h>

#define M3_BUTTON_TEST_FAIL_NONE 0u
#define M3_BUTTON_TEST_FAIL_TEXT_STYLE_INIT 1u
#define M3_BUTTON_TEST_FAIL_SHADOW_INIT 2u
#define M3_BUTTON_TEST_FAIL_RIPPLE_INIT 3u
#define M3_BUTTON_TEST_FAIL_RIPPLE_RADIUS 4u
#define M3_BUTTON_TEST_FAIL_RIPPLE_START 5u
#define M3_BUTTON_TEST_FAIL_SHADOW_SET 6u
#define M3_BUTTON_TEST_FAIL_RESOLVE_COLORS 7u
#define M3_BUTTON_TEST_FAIL_RESOLVE_CORNER 8u
#define M3_BUTTON_TEST_FAIL_OUTLINE_WIDTH 9u
#define M3_BUTTON_TEST_FAIL_RIPPLE_RELEASE 10u

int M3_CALL m3_button_test_set_fail_point(m3_u32 fail_point);
int M3_CALL m3_button_test_set_color_fail_after(m3_u32 call_count);
int M3_CALL m3_button_test_clear_fail_points(void);
int M3_CALL m3_button_test_validate_color(const M3Color *color);
int M3_CALL m3_button_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                     M3Scalar b, M3Scalar a);
int M3_CALL m3_button_test_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                            M3Color *out_color);
int M3_CALL m3_button_test_validate_text_style(const M3TextStyle *style,
                                               M3Bool require_family);
int M3_CALL m3_button_test_validate_style(const M3ButtonStyle *style,
                                          M3Bool require_family);
int M3_CALL m3_button_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_button_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_button_test_validate_backend(const M3TextBackend *backend);
int M3_CALL m3_button_test_metrics_update(M3Button *button);
int M3_CALL m3_button_test_resolve_colors(const M3Button *button,
                                          M3Color *out_background,
                                          M3Color *out_text,
                                          M3Color *out_outline,
                                          M3Color *out_ripple);
int M3_CALL m3_button_test_resolve_corner(const M3Button *button,
                                          M3Scalar *out_corner);

typedef struct TestButtonBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_calls;
  int draw_rect_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_draw;
  int fail_draw_rect;
  int fail_push_clip;
  int fail_pop_clip;
  M3Handle last_font;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_text_x;
  M3Scalar last_text_y;
  m3_usize last_text_len;
  M3Color last_text_color;
} TestButtonBackend;

static void test_backend_init(TestButtonBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestButtonBackend *backend;

  if (text == NULL || utf8_family == NULL || out_font == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return M3_ERR_RANGE;
  }
  if (italic != M3_FALSE && italic != M3_TRUE) {
    return M3_ERR_RANGE;
  }

  backend = (TestButtonBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return M3_ERR_IO;
  }

  out_font->id = 1u;
  out_font->generation = 1u;
  backend->last_font = *out_font;
  return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font) {
  TestButtonBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)text;
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
  TestButtonBackend *backend;

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

  backend = (TestButtonBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return M3_ERR_IO;
  }

  *out_width = (M3Scalar)(utf8_len * 10u);
  *out_height = 20.0f;
  *out_baseline = 15.0f;
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestButtonBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestButtonBackend *)text;
  backend->draw_calls += 1;
  if (backend->fail_draw) {
    return M3_ERR_IO;
  }

  backend->last_text_x = x;
  backend->last_text_y = y;
  backend->last_text_len = utf8_len;
  backend->last_text_color = color;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestButtonBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_push_clip(void *gfx, const M3Rect *rect) {
  TestButtonBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestButtonBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static const M3TextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const M3TextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const M3GfxVTable g_test_gfx_vtable = {NULL,
                                              NULL,
                                              NULL,
                                              test_gfx_draw_rect,
                                              NULL,
                                              test_gfx_push_clip,
                                              test_gfx_pop_clip,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL};

static const M3GfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int m3_color_near(M3Color a, M3Color b, M3Scalar tol) {
  if (!m3_near(a.r, b.r, tol)) {
    return 0;
  }
  if (!m3_near(a.g, b.g, tol)) {
    return 0;
  }
  if (!m3_near(a.b, b.b, tol)) {
    return 0;
  }
  if (!m3_near(a.a, b.a, tol)) {
    return 0;
  }
  return 1;
}

static int test_button_on_click(void *ctx, struct M3Button *button) {
  int *count;

  if (button == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  count = (int *)ctx;
  if (count != NULL) {
    *count += 1;
  }
  return M3_OK;
}

static int test_button_on_click_fail(void *ctx, struct M3Button *button) {
  M3_UNUSED(ctx);
  M3_UNUSED(button);
  return M3_ERR_IO;
}

static void init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                               m3_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

typedef int(M3_CALL *M3ButtonStyleInitFn)(M3ButtonStyle *style);

static int test_style_init_color_fail(M3ButtonStyleInitFn init_fn,
                                      m3_u32 call_count) {
  M3ButtonStyle tmp_style;
  m3_u32 i;

  for (i = 1u; i <= call_count; ++i) {
    M3_TEST_OK(m3_button_test_set_color_fail_after(i));
    M3_TEST_EXPECT(init_fn(&tmp_style), M3_ERR_IO);
    M3_TEST_OK(m3_button_test_clear_fail_points());
  }

  return 0;
}

int main(void) {
  TestButtonBackend backend;
  M3TextBackend text_backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3ButtonStyle style;
  M3ButtonStyle other_style;
  M3Button button;
  M3Button destroy_button;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  M3Semantics semantics;
  int clicks;
  M3Color color;
  M3Scalar corner;
  M3ButtonStyle tmp_style;
  M3TextStyle tmp_text_style;
  M3Color resolved_bg;
  M3Color resolved_text;
  M3Color resolved_outline;
  M3Color resolved_ripple;
  M3_TEST_EXPECT(m3_button_style_init_filled(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_style_init_tonal(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_style_init_outlined(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_style_init_text(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_style_init_elevated(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_style_init_fab(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_button_style_init_filled(&style));
  M3_TEST_ASSERT(style.variant == M3_BUTTON_VARIANT_FILLED);
  M3_TEST_ASSERT(m3_near(style.padding_x, M3_BUTTON_DEFAULT_PADDING_X, 0.001f));
  M3_TEST_ASSERT(m3_near(style.padding_y, M3_BUTTON_DEFAULT_PADDING_Y, 0.001f));
  M3_TEST_ASSERT(
      m3_near(style.corner_radius, M3_BUTTON_DEFAULT_CORNER_RADIUS, 0.001f));
  M3_TEST_ASSERT(m3_near(style.ripple_expand_duration,
                         M3_BUTTON_DEFAULT_RIPPLE_EXPAND, 0.001f));

  M3_TEST_OK(m3_button_style_init_tonal(&other_style));
  M3_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_TONAL);
  M3_TEST_OK(m3_button_style_init_text(&other_style));
  M3_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_TEXT);
  M3_TEST_OK(m3_button_style_init_elevated(&other_style));
  M3_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_ELEVATED);
  M3_TEST_ASSERT(other_style.shadow_enabled == M3_TRUE);

  M3_TEST_OK(
      m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_TEXT_STYLE_INIT));
  M3_TEST_EXPECT(m3_button_style_init_filled(&tmp_style), M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_INIT));
  M3_TEST_EXPECT(m3_button_style_init_filled(&tmp_style), M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_SET));
  M3_TEST_EXPECT(m3_button_style_init_elevated(&tmp_style), M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_SET));
  M3_TEST_EXPECT(m3_button_style_init_fab(&tmp_style), M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  if (test_style_init_color_fail(m3_button_style_init_filled, 13u)) {
    return 1;
  }
  if (test_style_init_color_fail(m3_button_style_init_tonal, 13u)) {
    return 1;
  }
  if (test_style_init_color_fail(m3_button_style_init_outlined, 13u)) {
    return 1;
  }
  if (test_style_init_color_fail(m3_button_style_init_text, 13u)) {
    return 1;
  }
  if (test_style_init_color_fail(m3_button_style_init_elevated, 14u)) {
    return 1;
  }
  if (test_style_init_color_fail(m3_button_style_init_fab, 14u)) {
    return 1;
  }

  M3_TEST_EXPECT(m3_button_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.5f;
  color.g = 0.5f;
  color.b = 0.5f;
  color.a = 0.5f;
  M3_TEST_EXPECT(m3_button_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.5f;
  color.g = -0.5f;
  M3_TEST_EXPECT(m3_button_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.5f;
  color.b = -0.5f;
  M3_TEST_EXPECT(m3_button_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.5f;
  color.a = -0.5f;
  M3_TEST_EXPECT(m3_button_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 0.5f;
  M3_TEST_OK(m3_button_test_validate_color(&color));

  M3_TEST_EXPECT(m3_button_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_test_color_set(&color, -0.1f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, -0.1f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, 0.0f, -0.1f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_button_test_color_with_alpha(NULL, 0.5f, &color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_test_color_with_alpha(&color, -0.1f, &color),
                 M3_ERR_RANGE);
  color.r = -1.0f;
  M3_TEST_EXPECT(m3_button_test_color_with_alpha(&color, 0.5f, &color),
                 M3_ERR_RANGE);
  color.r = 0.5f;

  M3_TEST_EXPECT(m3_button_test_validate_text_style(NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_text_style_init(&tmp_text_style));
  tmp_text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  tmp_text_style.utf8_family = "Test";
  tmp_text_style.size_px = 0;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_RANGE);
  tmp_text_style.size_px = 14;
  tmp_text_style.weight = 99;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_RANGE);
  tmp_text_style.weight = 901;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_RANGE);
  tmp_text_style.weight = 400;
  tmp_text_style.italic = 2;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_RANGE);
  tmp_text_style.italic = M3_FALSE;
  tmp_text_style.color.r = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE),
                 M3_ERR_RANGE);
  tmp_text_style.color.r = 0.0f;
  M3_TEST_OK(m3_button_test_validate_text_style(&tmp_text_style, M3_TRUE));

  M3_TEST_EXPECT(m3_button_test_validate_style(NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  M3_TEST_OK(m3_button_test_validate_style(&tmp_style, M3_FALSE));
  tmp_style.variant = 42u;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.min_width = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.outline_width = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.fab_diameter = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_fab(&tmp_style));
  tmp_style.fab_diameter = 0.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.ripple_expand_duration = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.shadow_enabled = 2;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.text_style.size_px = 0;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.background_color.r = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.outline_color.g = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.ripple_color.b = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_background_color.a = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_text_color.r = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_outline_color.g = -0.2f;
  M3_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, M3_FALSE),
                 M3_ERR_RANGE);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  M3_TEST_EXPECT(m3_button_test_validate_measure_spec(width_spec),
                 M3_ERR_INVALID_ARGUMENT);
  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_measure_spec(width_spec),
                 M3_ERR_RANGE);
  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 1.0f;
  M3_TEST_OK(m3_button_test_validate_measure_spec(width_spec));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_test_validate_rect(&bounds), M3_ERR_RANGE);
  bounds.width = 1.0f;
  bounds.height = -1.0f;
  M3_TEST_EXPECT(m3_button_test_validate_rect(&bounds), M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_button_test_validate_backend(NULL),
                 M3_ERR_INVALID_ARGUMENT);

  test_backend_init(&backend);
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  M3_TEST_OK(m3_button_test_validate_backend(&text_backend));
  text_backend.vtable = NULL;
  M3_TEST_EXPECT(m3_button_test_validate_backend(&text_backend),
                 M3_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable;

  M3_TEST_EXPECT(m3_button_init(NULL, &text_backend, &style, "OK", 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_init(&button, NULL, &style, "OK", 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, NULL, "OK", 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);

  text_backend.vtable = NULL;
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_create;
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_INVALID_ARGUMENT);
  style.text_style.utf8_family = "Test";

  backend.fail_create = 1;
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_IO);
  backend.fail_create = 0;

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_INIT));
  M3_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                 M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_init(&button, &text_backend, &style, "OK", 2));
  M3_TEST_ASSERT(button.widget.ctx == &button);
  M3_TEST_ASSERT(button.widget.vtable != NULL);
  M3_TEST_ASSERT((button.widget.flags & M3_WIDGET_FLAG_FOCUSABLE) != 0);

  M3_TEST_EXPECT(m3_button_test_metrics_update(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(
      m3_button_test_resolve_colors(NULL, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_button_test_resolve_colors(&button, &resolved_bg,
                                           &resolved_text, &resolved_outline,
                                           &resolved_ripple));
  M3_TEST_ASSERT(
      m3_color_near(resolved_bg, button.style.background_color, 0.001f));
  button.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(m3_button_test_resolve_colors(&button, &resolved_bg,
                                           &resolved_text, &resolved_outline,
                                           &resolved_ripple));
  M3_TEST_ASSERT(m3_color_near(resolved_bg,
                               button.style.disabled_background_color, 0.001f));
  button.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;
  button.style.background_color.r = -0.1f;
  M3_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      M3_ERR_RANGE);
  button.style.background_color = style.background_color;
  button.style.text_style.color.r = -0.1f;
  M3_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      M3_ERR_RANGE);
  button.style.text_style.color = style.text_style.color;
  button.style.outline_color.g = -0.1f;
  M3_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      M3_ERR_RANGE);
  button.style.outline_color = style.outline_color;
  button.style.ripple_color.b = -0.1f;
  M3_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      M3_ERR_RANGE);
  button.style.ripple_color = style.ripple_color;

  M3_TEST_EXPECT(m3_button_test_resolve_corner(NULL, &corner),
                 M3_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  button.bounds = bounds;
  M3_TEST_EXPECT(m3_button_test_resolve_corner(&button, &corner), M3_ERR_RANGE);
  bounds.width = 40.0f;
  bounds.height = 20.0f;
  button.bounds = bounds;
  button.style.variant = M3_BUTTON_VARIANT_FAB;
  M3_TEST_OK(m3_button_test_resolve_corner(&button, &corner));
  M3_TEST_ASSERT(m3_near(corner, 10.0f, 0.001f));
  button.style.variant = M3_BUTTON_VARIANT_FILLED;
  button.style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_button_test_resolve_corner(&button, &corner), M3_ERR_RANGE);
  button.style.corner_radius = style.corner_radius;

  M3_TEST_EXPECT(m3_button_set_label(NULL, "Hi", 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_button_set_label(&button, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_button_set_label(&button, "OK", 2));

  M3_TEST_EXPECT(m3_button_set_on_click(NULL, test_button_on_click, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  clicks = 0;
  M3_TEST_OK(m3_button_set_on_click(&button, test_button_on_click, &clicks));

  M3_TEST_EXPECT(m3_button_set_style(NULL, &style), M3_ERR_INVALID_ARGUMENT);
  tmp_style = style;
  tmp_style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_button_set_style(&button, &tmp_style),
                 M3_ERR_INVALID_ARGUMENT);
  backend.fail_create = 1;
  M3_TEST_EXPECT(m3_button_set_style(&button, &style), M3_ERR_IO);
  backend.fail_create = 0;
  backend.fail_destroy = 1;
  M3_TEST_EXPECT(m3_button_set_style(&button, &style), M3_ERR_IO);
  backend.fail_destroy = 0;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      button.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  button.style.variant = 42u;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);
  button.style.variant = M3_BUTTON_VARIANT_FILLED;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 64.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 36.0f, 0.001f));

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 80.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  M3_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 80.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 30.0f, 0.001f));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 50.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  M3_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 50.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 40.0f, 0.001f));

  backend.fail_measure = 1;
  button.metrics_valid = M3_FALSE;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  M3_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                               height_spec, &size),
                 M3_ERR_IO);
  backend.fail_measure = 0;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(button.widget.vtable->layout(button.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 40.0f;
  M3_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  M3_TEST_EXPECT(button.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);

  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;

  M3_TEST_EXPECT(button.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable_no_draw;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  test_backend_init(&backend);
  gfx.ctx = &backend;
  text_backend.ctx = &backend;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 1);
  M3_TEST_ASSERT(backend.draw_calls == 1);

  test_backend_init(&backend);
  backend.fail_draw_rect = 1;
  gfx.ctx = &backend;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = 0;

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RESOLVE_COLORS));
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RESOLVE_CORNER));
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_OUTLINE_WIDTH));
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  button.style.variant = 42u;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  button.style.variant = M3_BUTTON_VARIANT_FILLED;

  button.bounds.width = -1.0f;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  button.bounds = bounds;

  backend.fail_measure = 1;
  button.metrics_valid = M3_FALSE;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_measure = 0;

  button.style.shadow.color.a = 0.5f;
  button.style.shadow_enabled = M3_TRUE;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  backend.fail_draw_rect = 1;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = 0;
  button.style.shadow_enabled = M3_FALSE;
  button.style.shadow.color.a = 0.0f;

  button.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  text_backend.ctx = &backend;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               button.style.disabled_background_color, 0.001f));
  M3_TEST_ASSERT(m3_color_near(backend.last_text_color,
                               button.style.disabled_text_color, 0.001f));
  button.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  button.ripple.state = 99u;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  button.ripple.state = M3_RIPPLE_STATE_EXPANDING;
  button.ripple.radius = -1.0f;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  button.ripple.radius = 0.0f;
  button.ripple.opacity = 0.0f;
  button.ripple.state = M3_RIPPLE_STATE_IDLE;

  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.pressed = M3_FALSE;
  button.ripple.radius = 0.0f;
  button.ripple.opacity = 0.0f;
  button.ripple.state = M3_RIPPLE_STATE_IDLE;

  button.utf8_label = NULL;
  button.utf8_len = 0;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.utf8_label = "OK";
  button.utf8_len = 2;

  bounds.width = 10.0f;
  bounds.height = 10.0f;
  button.style.padding_x = 12.0f;
  button.style.padding_y = 12.0f;
  M3_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  paint_ctx.clip = bounds;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.style.padding_x = other_style.padding_x;
  button.style.padding_y = other_style.padding_y;
  bounds.width = 100.0f;
  bounds.height = 40.0f;
  M3_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  paint_ctx.clip = bounds;

  M3_TEST_OK(m3_button_style_init_outlined(&other_style));
  other_style.text_style.utf8_family = "Test";
  other_style.background_color.a = 1.0f;
  M3_TEST_OK(m3_button_set_style(&button, &other_style));
  M3_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  test_backend_init(&backend);
  gfx.ctx = &backend;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);

  button.style.outline_width = 2.0f;
  button.style.corner_radius = 1.0f;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  M3_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.style.corner_radius = other_style.corner_radius;

  test_backend_init(&backend);
  backend.fail_draw_rect = 1;
  gfx.ctx = &backend;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect = 0;

  button.style.outline_width = 30.0f;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  M3_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  button.style.outline_width = other_style.outline_width;

  M3_TEST_OK(m3_button_style_init_fab(&other_style));
  other_style.text_style.utf8_family = "Test";
  M3_TEST_OK(m3_button_set_style(&button, &other_style));
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  M3_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(size.width >= other_style.fab_diameter);
  M3_TEST_ASSERT(size.height >= other_style.fab_diameter);
  button.style.min_width = 0.0f;
  button.style.min_height = 0.0f;
  M3_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                           height_spec, &size));
  M3_TEST_ASSERT(size.width >= other_style.fab_diameter);
  M3_TEST_ASSERT(size.height >= other_style.fab_diameter);
  button.style.min_width = other_style.min_width;
  button.style.min_height = other_style.min_height;

  button.metrics_valid = M3_TRUE;
  M3_TEST_OK(m3_button_test_metrics_update(&button));
  button.metrics_valid = M3_FALSE;

  M3_TEST_EXPECT(button.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(button.widget.vtable->event(button.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(button.widget.vtable->event(button.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_button_style_init_filled(&other_style));
  other_style.text_style.utf8_family = "Test";
  M3_TEST_OK(m3_button_set_style(&button, &other_style));
  M3_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));

  button.style.ripple_expand_duration = -1.0f;
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  button.style.ripple_expand_duration = other_style.ripple_expand_duration;

  button.style.ripple_color.a = 2.0f;
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  button.style.ripple_color = other_style.ripple_color;

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_START));
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  button.bounds.width = -1.0f;
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  button.bounds = bounds;

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_RADIUS));
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());

  button.pressed = M3_TRUE;
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_STATE);
  button.pressed = M3_FALSE;

  init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(button.pressed == M3_TRUE);

  init_pointer_event(&event, 99u, 0, 0);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  button.style.ripple_fade_duration = -1.0f;
  init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  button.style.ripple_fade_duration = other_style.ripple_fade_duration;
  button.pressed = M3_TRUE;

  M3_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_RELEASE));
  init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_OK(m3_button_test_clear_fail_points());
  button.pressed = M3_TRUE;

  clicks = 0;
  M3_TEST_OK(m3_button_set_on_click(&button, test_button_on_click, &clicks));
  init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(clicks == 1);

  button.pressed = M3_TRUE;
  M3_TEST_OK(m3_button_set_on_click(&button, test_button_on_click_fail, NULL));
  init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6);
  M3_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      M3_ERR_IO);

  button.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6);
  M3_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  button.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(
      button.widget.vtable->get_semantics(button.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_BUTTON);
  M3_TEST_ASSERT(semantics.utf8_label == button.utf8_label);
  button.widget.flags |= (M3_WIDGET_FLAG_DISABLED | M3_WIDGET_FLAG_FOCUSABLE);
  M3_TEST_OK(
      button.widget.vtable->get_semantics(button.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_FOCUSABLE) != 0);
  button.widget.flags &= ~(M3_WIDGET_FLAG_DISABLED | M3_WIDGET_FLAG_FOCUSABLE);

  M3_TEST_EXPECT(button.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(button.widget.vtable->get_semantics(button.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(button.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_button_init(&destroy_button, &text_backend, &style, "OK", 2));
  backend.fail_destroy = 1;
  M3_TEST_EXPECT(
      destroy_button.widget.vtable->destroy(destroy_button.widget.ctx),
      M3_ERR_IO);
  backend.fail_destroy = 0;

  M3_TEST_OK(m3_button_init(&destroy_button, &text_backend, &style, "OK", 2));
  destroy_button.text_backend.vtable = NULL;
  M3_TEST_EXPECT(
      destroy_button.widget.vtable->destroy(destroy_button.widget.ctx),
      M3_ERR_UNSUPPORTED);

  return 0;
}
