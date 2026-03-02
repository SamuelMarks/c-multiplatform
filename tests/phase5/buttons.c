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

int CMP_CALL m3_button_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL m3_button_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_button_test_clear_fail_points(void);
int CMP_CALL m3_button_test_validate_color(const CMPColor *color);
int CMP_CALL m3_button_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                      CMPScalar b, CMPScalar a);
int CMP_CALL m3_button_test_color_with_alpha(const CMPColor *base,
                                             CMPScalar alpha,
                                             CMPColor *out_color);
int CMP_CALL m3_button_test_validate_text_style(const CMPTextStyle *style,
                                                CMPBool require_family);
int CMP_CALL m3_button_test_validate_style(const M3ButtonStyle *style,
                                           CMPBool require_family);
int CMP_CALL m3_button_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_button_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_button_test_validate_backend(const CMPTextBackend *backend);
int CMP_CALL m3_button_test_metrics_update(M3Button *button);
int CMP_CALL m3_button_test_resolve_colors(const M3Button *button,
                                           CMPColor *out_background,
                                           CMPColor *out_text,
                                           CMPColor *out_outline,
                                           CMPColor *out_ripple);
int CMP_CALL m3_button_test_resolve_corner(const M3Button *button,
                                           CMPScalar *out_corner);

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
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestButtonBackend;

static void test_backend_init(TestButtonBackend *backend) {
  memset(backend, 0, sizeof(*backend));
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestButtonBackend *backend;

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

  backend = (TestButtonBackend *)text;
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
  TestButtonBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)text;
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
  TestButtonBackend *backend;

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

  backend = (TestButtonBackend *)text;
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
  TestButtonBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestButtonBackend *)text;
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
  TestButtonBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
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
  TestButtonBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestButtonBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestButtonBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static const CMPTextVTable g_test_text_vtable = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_draw = {
    test_text_create_font, test_text_destroy_font, test_text_measure_text,
    NULL};

static const CMPTextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

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

static const CMPGfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, test_gfx_push_clip, test_gfx_pop_clip,
    NULL, NULL, NULL, NULL, NULL};

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int cmp_color_near(CMPColor a, CMPColor b, CMPScalar tol) {
  if (!cmp_near(a.r, b.r, tol)) {
    return 0;
  }
  if (!cmp_near(a.g, b.g, tol)) {
    return 0;
  }
  if (!cmp_near(a.b, b.b, tol)) {
    return 0;
  }
  if (!cmp_near(a.a, b.a, tol)) {
    return 0;
  }
  return 1;
}

static int test_button_on_click(void *ctx, struct M3Button *button) {
  int *count;

  if (button == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  count = (int *)ctx;
  if (count != NULL) {
    *count += 1;
  }
  return CMP_OK;
}

static int test_button_on_click_fail(void *ctx, struct M3Button *button) {
  CMP_UNUSED(ctx);
  CMP_UNUSED(button);
  return CMP_ERR_IO;
}

static void init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                               cmp_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

typedef int(CMP_CALL *CMPButtonStyleInitFn)(M3ButtonStyle *style);

static int test_style_init_color_fail(CMPButtonStyleInitFn init_fn,
                                      cmp_u32 call_count) {
  M3ButtonStyle tmp_style;
  cmp_u32 i;

  for (i = 1u; i <= call_count; ++i) {
    CMP_TEST_OK(m3_button_test_set_color_fail_after(i));
    CMP_TEST_EXPECT(init_fn(&tmp_style), CMP_ERR_IO);
    CMP_TEST_OK(m3_button_test_clear_fail_points());
  }

  return 0;
}

int main(void) {
  TestButtonBackend backend;
  CMPTextBackend text_backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3ButtonStyle style;
  M3ButtonStyle other_style;
  M3Button button;
  M3Button destroy_button;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  int clicks;
  CMPColor color;
  CMPScalar corner;
  M3ButtonStyle tmp_style;
  CMPTextStyle tmp_text_style;
  CMPColor resolved_bg;
  CMPColor resolved_text;
  CMPColor resolved_outline;
  CMPColor resolved_ripple;
  CMP_TEST_EXPECT(m3_button_style_init_filled(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_style_init_tonal(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_style_init_outlined(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_style_init_text(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_style_init_elevated(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_style_init_fab(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_button_style_init_filled(&style));
  CMP_TEST_ASSERT(style.variant == M3_BUTTON_VARIANT_FILLED);
  CMP_TEST_ASSERT(
      cmp_near(style.padding_x, M3_BUTTON_DEFAULT_PADDING_X, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.padding_y, M3_BUTTON_DEFAULT_PADDING_Y, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.corner_radius, M3_BUTTON_DEFAULT_CORNER_RADIUS, 0.001f));
  CMP_TEST_ASSERT(cmp_near(style.ripple_expand_duration,
                           M3_BUTTON_DEFAULT_RIPPLE_EXPAND, 0.001f));

  CMP_TEST_OK(m3_button_style_init_tonal(&other_style));
  CMP_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_TONAL);
  CMP_TEST_OK(m3_button_style_init_text(&other_style));
  CMP_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_TEXT);
  CMP_TEST_OK(m3_button_style_init_elevated(&other_style));
  CMP_TEST_ASSERT(other_style.variant == M3_BUTTON_VARIANT_ELEVATED);
  CMP_TEST_ASSERT(other_style.shadow_enabled == CMP_TRUE);

  CMP_TEST_OK(
      m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_TEXT_STYLE_INIT));
  CMP_TEST_EXPECT(m3_button_style_init_filled(&tmp_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_INIT));
  CMP_TEST_EXPECT(m3_button_style_init_filled(&tmp_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_SET));
  CMP_TEST_EXPECT(m3_button_style_init_elevated(&tmp_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_SHADOW_SET));
  CMP_TEST_EXPECT(m3_button_style_init_fab(&tmp_style), CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

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

  CMP_TEST_EXPECT(m3_button_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.5f;
  color.g = 0.5f;
  color.b = 0.5f;
  color.a = 0.5f;
  CMP_TEST_EXPECT(m3_button_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.5f;
  color.g = -0.5f;
  CMP_TEST_EXPECT(m3_button_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.5f;
  color.b = -0.5f;
  CMP_TEST_EXPECT(m3_button_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.5f;
  color.a = -0.5f;
  CMP_TEST_EXPECT(m3_button_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 0.5f;
  CMP_TEST_OK(m3_button_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_button_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_test_color_set(&color, -0.1f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, -0.1f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, 0.0f, -0.1f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_button_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_button_test_color_with_alpha(NULL, 0.5f, &color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_test_color_with_alpha(&color, -0.1f, &color),
                  CMP_ERR_RANGE);
  color.r = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_color_with_alpha(&color, 0.5f, &color),
                  CMP_ERR_RANGE);
  color.r = 0.5f;

  CMP_TEST_EXPECT(m3_button_test_validate_text_style(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&tmp_text_style));
  tmp_text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  tmp_text_style.utf8_family = "Test";
  tmp_text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  tmp_text_style.size_px = 14;
  tmp_text_style.weight = 99;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  tmp_text_style.weight = 901;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  tmp_text_style.weight = 400;
  tmp_text_style.italic = 2;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  tmp_text_style.italic = CMP_FALSE;
  tmp_text_style.color.r = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  tmp_text_style.color.r = 0.0f;
  CMP_TEST_OK(m3_button_test_validate_text_style(&tmp_text_style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_button_test_validate_style(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  CMP_TEST_OK(m3_button_test_validate_style(&tmp_style, CMP_FALSE));
  tmp_style.variant = 42u;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.outline_width = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.fab_diameter = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_fab(&tmp_style));
  tmp_style.fab_diameter = 0.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.ripple_expand_duration = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.shadow_enabled = 2;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.background_color.r = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.outline_color.g = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.ripple_color.b = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_background_color.a = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_text_color.r = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_style_init_filled(&tmp_style));
  tmp_style.disabled_outline_color.g = -0.2f;
  CMP_TEST_EXPECT(m3_button_test_validate_style(&tmp_style, CMP_FALSE),
                  CMP_ERR_RANGE);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_measure_spec(width_spec),
                  CMP_ERR_INVALID_ARGUMENT);
  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_measure_spec(width_spec),
                  CMP_ERR_RANGE);
  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 1.0f;
  CMP_TEST_OK(m3_button_test_validate_measure_spec(width_spec));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_test_validate_rect(&bounds), CMP_ERR_RANGE);
  bounds.width = 1.0f;
  bounds.height = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_validate_rect(&bounds), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_button_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  test_backend_init(&backend);
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_button_test_validate_backend(&text_backend));
  text_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_button_test_validate_backend(&text_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable;

  CMP_TEST_EXPECT(m3_button_init(NULL, &text_backend, &style, "OK", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_init(&button, NULL, &style, "OK", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, NULL, "OK", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_INVALID_ARGUMENT);
  style.text_style.utf8_family = "Test";

  backend.fail_create = 1;
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_IO);
  backend.fail_create = 0;

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_INIT));
  CMP_TEST_EXPECT(m3_button_init(&button, &text_backend, &style, "OK", 2),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(m3_button_init(&button, &text_backend, &style, "OK", 2));
  CMP_TEST_ASSERT(button.widget.ctx == &button);
  CMP_TEST_ASSERT(button.widget.vtable != NULL);
  CMP_TEST_ASSERT((button.widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) != 0);

  CMP_TEST_EXPECT(m3_button_test_metrics_update(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(
      m3_button_test_resolve_colors(NULL, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_button_test_resolve_colors(&button, &resolved_bg,
                                            &resolved_text, &resolved_outline,
                                            &resolved_ripple));
  CMP_TEST_ASSERT(
      cmp_color_near(resolved_bg, button.style.background_color, 0.001f));
  button.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(m3_button_test_resolve_colors(&button, &resolved_bg,
                                            &resolved_text, &resolved_outline,
                                            &resolved_ripple));
  CMP_TEST_ASSERT(cmp_color_near(
      resolved_bg, button.style.disabled_background_color, 0.001f));
  button.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;
  button.style.background_color.r = -0.1f;
  CMP_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      CMP_ERR_RANGE);
  button.style.background_color = style.background_color;
  button.style.text_style.color.r = -0.1f;
  CMP_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      CMP_ERR_RANGE);
  button.style.text_style.color = style.text_style.color;
  button.style.outline_color.g = -0.1f;
  CMP_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      CMP_ERR_RANGE);
  button.style.outline_color = style.outline_color;
  button.style.ripple_color.b = -0.1f;
  CMP_TEST_EXPECT(
      m3_button_test_resolve_colors(&button, &resolved_bg, &resolved_text,
                                    &resolved_outline, &resolved_ripple),
      CMP_ERR_RANGE);
  button.style.ripple_color = style.ripple_color;

  CMP_TEST_EXPECT(m3_button_test_resolve_corner(NULL, &corner),
                  CMP_ERR_INVALID_ARGUMENT);
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  button.bounds = bounds;
  CMP_TEST_EXPECT(m3_button_test_resolve_corner(&button, &corner),
                  CMP_ERR_RANGE);
  bounds.width = 40.0f;
  bounds.height = 20.0f;
  button.bounds = bounds;
  button.style.variant = M3_BUTTON_VARIANT_FAB;
  CMP_TEST_OK(m3_button_test_resolve_corner(&button, &corner));
  CMP_TEST_ASSERT(cmp_near(corner, 10.0f, 0.001f));
  button.style.variant = M3_BUTTON_VARIANT_FILLED;
  button.style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_button_test_resolve_corner(&button, &corner),
                  CMP_ERR_RANGE);
  button.style.corner_radius = style.corner_radius;

  CMP_TEST_EXPECT(m3_button_set_label(NULL, "Hi", 2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_button_set_label(&button, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_button_set_label(&button, "OK", 2));

  CMP_TEST_EXPECT(m3_button_set_on_click(NULL, test_button_on_click, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  clicks = 0;
  CMP_TEST_OK(m3_button_set_on_click(&button, test_button_on_click, &clicks));

  CMP_TEST_EXPECT(m3_button_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  tmp_style = style;
  tmp_style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_button_set_style(&button, &tmp_style),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.fail_create = 1;
  CMP_TEST_EXPECT(m3_button_set_style(&button, &style), CMP_ERR_IO);
  backend.fail_create = 0;
  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_button_set_style(&button, &style), CMP_ERR_IO);
  backend.fail_destroy = 0;

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(
      button.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  button.style.variant = 42u;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);
  button.style.variant = M3_BUTTON_VARIANT_FILLED;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 64.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 36.0f, 0.001f));

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 80.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  CMP_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 80.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 30.0f, 0.001f));

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 50.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  CMP_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 50.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 40.0f, 0.001f));

  backend.fail_measure = 1;
  button.metrics_valid = CMP_FALSE;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  CMP_TEST_EXPECT(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                                height_spec, &size),
                  CMP_ERR_IO);
  backend.fail_measure = 0;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(button.widget.vtable->layout(button.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  CMP_TEST_EXPECT(button.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);

  paint_ctx.gfx = &gfx;
  paint_ctx.clip = bounds;
  paint_ctx.dpi_scale = 1.0f;

  CMP_TEST_EXPECT(button.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable_no_draw;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  test_backend_init(&backend);
  gfx.ctx = &backend;
  text_backend.ctx = &backend;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 1);
  CMP_TEST_ASSERT(backend.draw_calls == 1);

  test_backend_init(&backend);
  backend.fail_draw_rect = 1;
  gfx.ctx = &backend;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  CMP_TEST_OK(
      m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RESOLVE_COLORS));
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(
      m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RESOLVE_CORNER));
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_OUTLINE_WIDTH));
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  button.style.variant = 42u;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  button.style.variant = M3_BUTTON_VARIANT_FILLED;

  button.bounds.width = -1.0f;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  button.bounds = bounds;

  backend.fail_measure = 1;
  button.metrics_valid = CMP_FALSE;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_measure = 0;

  button.style.shadow.color.a = 0.5f;
  button.style.shadow_enabled = CMP_TRUE;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  backend.fail_draw_rect = 1;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = 0;
  button.style.shadow_enabled = CMP_FALSE;
  button.style.shadow.color.a = 0.0f;

  button.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  text_backend.ctx = &backend;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(cmp_color_near(
      backend.last_rect_color, button.style.disabled_background_color, 0.001f));
  CMP_TEST_ASSERT(cmp_color_near(backend.last_text_color,
                                 button.style.disabled_text_color, 0.001f));
  button.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  button.ripple.state = 99u;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  button.ripple.state = CMP_RIPPLE_STATE_EXPANDING;
  button.ripple.radius = -1.0f;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  button.ripple.radius = 0.0f;
  button.ripple.opacity = 0.0f;
  button.ripple.state = CMP_RIPPLE_STATE_IDLE;

  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.pressed = CMP_FALSE;
  button.ripple.radius = 0.0f;
  button.ripple.opacity = 0.0f;
  button.ripple.state = CMP_RIPPLE_STATE_IDLE;

  button.utf8_label = NULL;
  button.utf8_len = 0;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.utf8_label = "OK";
  button.utf8_len = 2;

  bounds.width = 10.0f;
  bounds.height = 10.0f;
  button.style.padding_x = 12.0f;
  button.style.padding_y = 12.0f;
  CMP_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  paint_ctx.clip = bounds;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.style.padding_x = other_style.padding_x;
  button.style.padding_y = other_style.padding_y;
  bounds.width = 100.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  paint_ctx.clip = bounds;

  CMP_TEST_OK(m3_button_style_init_outlined(&other_style));
  other_style.text_style.utf8_family = "Test";
  other_style.background_color.a = 1.0f;
  CMP_TEST_OK(m3_button_set_style(&button, &other_style));
  CMP_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));
  test_backend_init(&backend);
  gfx.ctx = &backend;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend.draw_rect_calls == 2);

  button.style.outline_width = 2.0f;
  button.style.corner_radius = 1.0f;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  CMP_TEST_OK(button.widget.vtable->paint(button.widget.ctx, &paint_ctx));
  button.style.corner_radius = other_style.corner_radius;

  test_backend_init(&backend);
  backend.fail_draw_rect = 1;
  gfx.ctx = &backend;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  button.style.outline_width = 30.0f;
  test_backend_init(&backend);
  gfx.ctx = &backend;
  CMP_TEST_EXPECT(button.widget.vtable->paint(button.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  button.style.outline_width = other_style.outline_width;

  CMP_TEST_OK(m3_button_style_init_fab(&other_style));
  other_style.text_style.utf8_family = "Test";
  CMP_TEST_OK(m3_button_set_style(&button, &other_style));
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  CMP_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width >= other_style.fab_diameter);
  CMP_TEST_ASSERT(size.height >= other_style.fab_diameter);
  button.style.min_width = 0.0f;
  button.style.min_height = 0.0f;
  CMP_TEST_OK(button.widget.vtable->measure(button.widget.ctx, width_spec,
                                            height_spec, &size));
  CMP_TEST_ASSERT(size.width >= other_style.fab_diameter);
  CMP_TEST_ASSERT(size.height >= other_style.fab_diameter);
  button.style.min_width = other_style.min_width;
  button.style.min_height = other_style.min_height;

  button.metrics_valid = CMP_TRUE;
  CMP_TEST_OK(m3_button_test_metrics_update(&button));
  button.metrics_valid = CMP_FALSE;

  CMP_TEST_EXPECT(button.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(button.widget.vtable->event(button.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_button_style_init_filled(&other_style));
  other_style.text_style.utf8_family = "Test";
  CMP_TEST_OK(m3_button_set_style(&button, &other_style));
  CMP_TEST_OK(button.widget.vtable->layout(button.widget.ctx, bounds));

  button.style.ripple_expand_duration = -1.0f;
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  button.style.ripple_expand_duration = other_style.ripple_expand_duration;

  button.style.ripple_color.a = 2.0f;
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  button.style.ripple_color = other_style.ripple_color;

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_START));
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  button.bounds.width = -1.0f;
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  button.bounds = bounds;

  CMP_TEST_OK(m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_RADIUS));
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());

  button.pressed = CMP_TRUE;
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_STATE);
  button.pressed = CMP_FALSE;

  init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(button.pressed == CMP_TRUE);

  init_pointer_event(&event, 99u, 0, 0);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  button.style.ripple_fade_duration = -1.0f;
  init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  button.style.ripple_fade_duration = other_style.ripple_fade_duration;
  button.pressed = CMP_TRUE;

  CMP_TEST_OK(
      m3_button_test_set_fail_point(M3_BUTTON_TEST_FAIL_RIPPLE_RELEASE));
  init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_button_test_clear_fail_points());
  button.pressed = CMP_TRUE;

  clicks = 0;
  CMP_TEST_OK(m3_button_set_on_click(&button, test_button_on_click, &clicks));
  init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(clicks == 1);

  button.pressed = CMP_TRUE;
  CMP_TEST_OK(m3_button_set_on_click(&button, test_button_on_click_fail, NULL));
  init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6);
  CMP_TEST_EXPECT(
      button.widget.vtable->event(button.widget.ctx, &event, &handled),
      CMP_ERR_IO);

  button.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6);
  CMP_TEST_OK(button.widget.vtable->event(button.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  button.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMP_TEST_OK(
      button.widget.vtable->get_semantics(button.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_BUTTON);
  CMP_TEST_ASSERT(semantics.utf8_label == button.utf8_label);
  button.widget.flags |= (CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE);
  CMP_TEST_OK(
      button.widget.vtable->get_semantics(button.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0);
  button.widget.flags &=
      ~(CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE);

  CMP_TEST_EXPECT(button.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(button.widget.vtable->get_semantics(button.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(button.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_button_init(&destroy_button, &text_backend, &style, "OK", 2));
  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(
      destroy_button.widget.vtable->destroy(destroy_button.widget.ctx),
      CMP_ERR_IO);
  backend.fail_destroy = 0;

  CMP_TEST_OK(m3_button_init(&destroy_button, &text_backend, &style, "OK", 2));
  destroy_button.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(
      destroy_button.widget.vtable->destroy(destroy_button.widget.ctx),
      CMP_ERR_UNSUPPORTED);

  return 0;
}
