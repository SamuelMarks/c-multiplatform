#include "m3/m3_chip.h"
#include "test_utils.h"

#include <string.h>

#define M3_CHIP_TEST_FAIL_NONE 0u
#define M3_CHIP_TEST_FAIL_TEXT_STYLE_INIT 1u
#define M3_CHIP_TEST_FAIL_RIPPLE_INIT 2u
#define M3_CHIP_TEST_FAIL_RIPPLE_RADIUS 3u
#define M3_CHIP_TEST_FAIL_RIPPLE_START 4u
#define M3_CHIP_TEST_FAIL_RIPPLE_RELEASE 5u
#define M3_CHIP_TEST_FAIL_RESOLVE_COLORS 6u
#define M3_CHIP_TEST_FAIL_RESOLVE_CORNER 7u
#define M3_CHIP_TEST_FAIL_OUTLINE_WIDTH 8u
#define M3_CHIP_TEST_FAIL_DELETE_BOUNDS 9u

int CMP_CALL m3_chip_test_set_fail_point(cmp_u32 fail_point);
int CMP_CALL m3_chip_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_chip_test_clear_fail_points(void);
int CMP_CALL m3_chip_test_validate_color(const CMPColor *color);
int CMP_CALL m3_chip_test_color_set(CMPColor *color, CMPScalar r, CMPScalar g,
                                    CMPScalar b, CMPScalar a);
int CMP_CALL m3_chip_test_color_with_alpha(const CMPColor *base,
                                           CMPScalar alpha,
                                           CMPColor *out_color);
int CMP_CALL m3_chip_test_validate_text_style(const CMPTextStyle *style,
                                              CMPBool require_family);
int CMP_CALL m3_chip_test_validate_layout(const M3ChipLayout *layout);
int CMP_CALL m3_chip_test_validate_style(const M3ChipStyle *style,
                                         CMPBool require_family);
int CMP_CALL m3_chip_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_chip_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_chip_test_validate_backend(const CMPTextBackend *backend);
int CMP_CALL m3_chip_test_metrics_update(M3Chip *chip);
int CMP_CALL m3_chip_test_resolve_colors(const M3Chip *chip,
                                         CMPColor *out_background,
                                         CMPColor *out_text,
                                         CMPColor *out_outline,
                                         CMPColor *out_ripple);
int CMP_CALL m3_chip_test_resolve_corner(const M3Chip *chip,
                                         CMPScalar *out_corner);
int CMP_CALL m3_chip_test_compute_delete_bounds(M3Chip *chip,
                                                CMPRect *out_bounds);
int CMP_CALL m3_chip_test_draw_delete_icon(const CMPGfx *gfx,
                                           const CMPRect *bounds,
                                           CMPColor color, CMPScalar thickness);

typedef struct TestChipBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_text_calls;
  int draw_rect_calls;
  int draw_line_calls;
  int push_clip_calls;
  int pop_clip_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_draw_text;
  int fail_draw_rect;
  int fail_draw_line;
  int fail_push_clip;
  int fail_pop_clip;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_line_x0;
  CMPScalar last_line_y0;
  CMPScalar last_line_x1;
  CMPScalar last_line_y1;
  CMPScalar last_line_thickness;
  CMPColor last_line_color;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestChipBackend;

typedef struct ClickCounter {
  int calls;
  int fail;
} ClickCounter;

typedef struct DeleteCounter {
  int calls;
  int fail;
} DeleteCounter;

static int test_backend_init(TestChipBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return CMP_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestChipBackend *backend;

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

  backend = (TestChipBackend *)text;
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
  TestChipBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)text;
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
  TestChipBackend *backend;

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

  backend = (TestChipBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }

  *out_width = (CMPScalar)(utf8_len * 10u);
  *out_height = 18.0f;
  *out_baseline = 13.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestChipBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestChipBackend *)text;
  backend->draw_text_calls += 1;
  if (backend->fail_draw_text) {
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
  TestChipBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                              CMPScalar x1, CMPScalar y1, CMPColor color,
                              CMPScalar thickness) {
  TestChipBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
  backend->draw_line_calls += 1;
  backend->last_line_x0 = x0;
  backend->last_line_y0 = y0;
  backend->last_line_x1 = x1;
  backend->last_line_y1 = y1;
  backend->last_line_thickness = thickness;
  backend->last_line_color = color;
  if (backend->fail_draw_line) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_push_clip(void *gfx, const CMPRect *rect) {
  TestChipBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestChipBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
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
                                               test_gfx_draw_line,
                                               NULL,
                                               test_gfx_push_clip,
                                               test_gfx_pop_clip,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_rect = {NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       test_gfx_draw_line,
                                                       NULL,
                                                       test_gfx_push_clip,
                                                       test_gfx_pop_clip,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_line = {NULL,
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

static const CMPGfxVTable g_test_gfx_vtable_no_clip = {NULL,
                                                       NULL,
                                                       NULL,
                                                       test_gfx_draw_rect,
                                                       test_gfx_draw_line,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL};

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

static int test_chip_on_click(void *ctx, struct M3Chip *chip) {
  ClickCounter *counter;

  if (ctx == NULL || chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (ClickCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_chip_on_delete(void *ctx, struct M3Chip *chip) {
  DeleteCounter *counter;

  if (ctx == NULL || chip == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  counter = (DeleteCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_chip_helpers(void) {
  CMPColor color;
  CMPColor out_color;
  CMPTextStyle text_style;
  M3ChipLayout layout;
  M3ChipStyle style;
  M3ChipStyle bad_style;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3Chip chip;
  TestChipBackend backend;
  CMPTextBackend text_backend;
  CMPColor background;
  CMPColor text_color;
  CMPColor outline;
  CMPColor ripple;
  CMPScalar corner;
  CMPRect delete_bounds;
  volatile const CMPColor *null_color;
  volatile const CMPTextStyle *null_style;
  volatile const M3ChipLayout *null_layout;
  volatile const M3ChipStyle *null_chip_style;
  volatile const CMPRect *null_rect;
  volatile const CMPTextBackend *null_backend;

  null_color = NULL;
  null_style = NULL;
  null_layout = NULL;
  null_chip_style = NULL;
  null_rect = NULL;
  null_backend = NULL;

  CMP_TEST_EXPECT(m3_chip_test_validate_color((const CMPColor *)null_color),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 1.2f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = -0.1f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = -0.1f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 1.2f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 2.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = -0.1f;
  CMP_TEST_EXPECT(m3_chip_test_validate_color(&color), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_chip_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_color_set(&color, -1.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, -1.0f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, -1.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, 0.0f, 2.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_chip_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(NULL, 0.5f, &out_color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, -0.1f, &out_color),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 1.5f, &out_color),
                  CMP_ERR_RANGE);
  color.r = 2.0f;
  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, &out_color),
                  CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_OK(m3_chip_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, &out_color),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(
                      (const CMPTextStyle *)null_style, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  memset(&text_style, 0, sizeof(text_style));
  text_style.size_px = 12;
  text_style.weight = 400;
  text_style.italic = CMP_FALSE;
  text_style.color.r = 0.0f;
  text_style.color.g = 0.0f;
  text_style.color.b = 0.0f;
  text_style.color.a = 1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 901;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.italic = CMP_FALSE;
  text_style.color.r = 2.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.r = 0.0f;
  text_style.color.g = -0.5f;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.g = 0.0f;
  text_style.color.b = 1.5f;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.b = 0.0f;
  text_style.color.a = -0.5f;
  CMP_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.a = 1.0f;
  CMP_TEST_OK(m3_chip_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_EXPECT(
      m3_chip_test_validate_layout((const M3ChipLayout *)null_layout),
      CMP_ERR_INVALID_ARGUMENT);
  memset(&layout, 0, sizeof(layout));
  layout.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_layout(&layout), CMP_ERR_RANGE);
  layout.padding_x = 0.0f;
  layout.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_layout(&layout), CMP_ERR_RANGE);
  layout.min_width = 0.0f;
  layout.icon_size = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_layout(&layout), CMP_ERR_RANGE);
  layout.icon_size = 10.0f;
  layout.icon_gap = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_layout(&layout), CMP_ERR_RANGE);
  layout.icon_gap = 0.0f;
  layout.delete_icon_thickness = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_layout(&layout), CMP_ERR_RANGE);
  layout.delete_icon_thickness = 1.0f;
  CMP_TEST_OK(m3_chip_test_validate_layout(&layout));

  CMP_TEST_EXPECT(m3_chip_test_validate_style(
                      (const M3ChipStyle *)null_chip_style, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_style_init_assist(&style));
  style.text_style.utf8_family = "Test";
  CMP_TEST_OK(m3_chip_test_validate_style(&style, CMP_TRUE));
  bad_style = style;
  bad_style.variant = 0;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_style = style;
  bad_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_expand_duration = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_style = style;
  bad_style.layout.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_style = style;
  bad_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_style = style;
  bad_style.background_color.r = 2.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, CMP_TRUE),
                  CMP_ERR_RANGE);

  spec.mode = 999u;
  spec.size = 10.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_measure_spec(spec), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_chip_test_validate_rect((const CMPRect *)null_rect),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 10.0f;
  CMP_TEST_EXPECT(m3_chip_test_validate_rect(&rect), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_chip_test_validate_backend((const CMPTextBackend *)null_backend),
      CMP_ERR_INVALID_ARGUMENT);
  memset(&text_backend, 0, sizeof(text_backend));
  CMP_TEST_EXPECT(m3_chip_test_validate_backend(&text_backend),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&chip, 0, sizeof(chip));
  chip.style = style;
  chip.widget.flags = 0;
  chip.selected = CMP_TRUE;
  CMP_TEST_OK(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                          &outline, &ripple));
  CMP_TEST_ASSERT(
      cmp_near(background.r, style.selected_background_color.r, 0.001f));
  CMP_TEST_ASSERT(cmp_near(text_color.r, style.selected_text_color.r, 0.001f));
  chip.widget.flags = CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                          &outline, &ripple));
  CMP_TEST_ASSERT(
      cmp_near(background.a, style.disabled_background_color.a, 0.001f));
  chip.widget.flags = 0;
  chip.selected = CMP_FALSE;
  chip.style.background_color.r = 2.0f;
  CMP_TEST_EXPECT(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                              &outline, &ripple),
                  CMP_ERR_RANGE);
  chip.style.background_color = style.background_color;

  chip.bounds.x = 0.0f;
  chip.bounds.y = 0.0f;
  chip.bounds.width = 20.0f;
  chip.bounds.height = 10.0f;
  chip.style.corner_radius = 20.0f;
  CMP_TEST_OK(m3_chip_test_resolve_corner(&chip, &corner));
  CMP_TEST_ASSERT(cmp_near(corner, 5.0f, 0.001f));
  chip.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_chip_test_resolve_corner(&chip, &corner), CMP_ERR_RANGE);
  chip.bounds.width = 20.0f;

  chip.metrics.width = 10.0f;
  chip.metrics.height = 18.0f;
  chip.metrics.baseline = 13.0f;
  chip.metrics_valid = CMP_TRUE;
  chip.bounds.x = 0.0f;
  chip.bounds.y = 0.0f;
  chip.bounds.width = 120.0f;
  chip.bounds.height = 32.0f;
  chip.show_delete = CMP_FALSE;
  CMP_TEST_EXPECT(m3_chip_test_compute_delete_bounds(NULL, &delete_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  CMP_TEST_ASSERT(delete_bounds.width == 0.0f);
  chip.show_delete = CMP_TRUE;
  CMP_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  CMP_TEST_ASSERT(delete_bounds.width > 0.0f);
  chip.bounds.width = 10.0f;
  chip.bounds.height = 10.0f;
  CMP_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  chip.bounds.width = 120.0f;
  chip.bounds.height = 32.0f;
  chip.style.layout.icon_size = 0.0f;
  CMP_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                  CMP_ERR_RANGE);
  chip.style.layout.icon_size = style.layout.icon_size;
  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_DELETE_BOUNDS));
  CMP_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  chip.dense = 2;
  CMP_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  chip.dense = CMP_FALSE;

  CMP_TEST_EXPECT(m3_chip_test_metrics_update(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(test_backend_init(&backend));
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;
  chip.text_backend = text_backend;
  chip.font.id = 1u;
  chip.font.generation = 1u;
  chip.utf8_label = "A";
  chip.utf8_len = 1;
  chip.metrics_valid = CMP_FALSE;
  CMP_TEST_OK(m3_chip_test_metrics_update(&chip));
  CMP_TEST_ASSERT(chip.metrics_valid == CMP_TRUE);
  chip.metrics_valid = CMP_FALSE;
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(m3_chip_test_metrics_update(&chip), CMP_ERR_IO);
  backend.fail_measure = 0;
  chip.metrics_valid = CMP_TRUE;
  CMP_TEST_OK(m3_chip_test_metrics_update(&chip));

  return 0;
}

static int test_chip_draw_delete(void) {
  TestChipBackend backend;
  CMPGfx gfx;
  CMPRect bounds;
  CMPColor color;

  CMP_TEST_OK(test_backend_init(&backend));
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 10.0f;
  bounds.height = 10.0f;
  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;

  CMP_TEST_EXPECT(m3_chip_test_draw_delete_icon(NULL, &bounds, color, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, NULL, color, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, -1.0f),
                  CMP_ERR_RANGE);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  bounds.width = 0.0f;
  CMP_TEST_OK(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f));
  bounds.width = 10.0f;

  backend.fail_draw_line = 1;
  CMP_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f),
                  CMP_ERR_IO);
  backend.fail_draw_line = 0;

  CMP_TEST_OK(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f));
  return 0;
}

static int test_chip_style_init(void) {
  M3ChipStyle style;

  CMP_TEST_OK(m3_chip_style_init_assist(&style));
  CMP_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_ASSIST);
  CMP_TEST_OK(m3_chip_style_init_filter(&style));
  CMP_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_FILTER);
  CMP_TEST_OK(m3_chip_style_init_input(&style));
  CMP_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_INPUT);
  CMP_TEST_OK(m3_chip_style_init_suggestion(&style));
  CMP_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_SUGGESTION);

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_TEXT_STYLE_INIT));
  CMP_TEST_EXPECT(m3_chip_style_init_assist(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_test_set_color_fail_after(1));
  CMP_TEST_EXPECT(m3_chip_style_init_assist(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  return 0;
}

static int test_chip_widget(void) {
  TestChipBackend backend;
  CMPTextBackend text_backend;
  CMPGfx gfx;
  CMPPaintContext ctx;
  M3ChipStyle style;
  M3ChipStyle other_style;
  M3Chip chip;
  M3Chip destroy_chip;
  M3ChipLayout dense_backup;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  ClickCounter clicks;
  DeleteCounter deletes;
  CMPRect delete_bounds;
  CMPScalar expected_width;
  CMPScalar expected_height;

  CMP_TEST_OK(test_backend_init(&backend));
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;
  gfx.ctx = &backend;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  ctx.gfx = &gfx;
  ctx.clip.x = 0.0f;
  ctx.clip.y = 0.0f;
  ctx.clip.width = 200.0f;
  ctx.clip.height = 50.0f;
  ctx.dpi_scale = 1.0f;

  CMP_TEST_OK(m3_chip_style_init_filter(&style));
  style.text_style.utf8_family = "Test";
  other_style = style;
  other_style.text_style.size_px = 16;

  CMP_TEST_EXPECT(m3_chip_init(NULL, &text_backend, &style, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_init(&chip, NULL, &style, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, NULL, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  text_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                  CMP_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                  CMP_ERR_INVALID_ARGUMENT);
  style.text_style.utf8_family = "Test";

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_INIT));
  CMP_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_init(&chip, &text_backend, &style, "Chip", 4));
  CMP_TEST_ASSERT(chip.widget.ctx == &chip);
  CMP_TEST_ASSERT(chip.widget.vtable != NULL);
  CMP_TEST_ASSERT((chip.widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) != 0);

  CMP_TEST_EXPECT(m3_chip_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_set_style(&chip, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_chip_set_label(NULL, "A", 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_set_label(&chip, NULL, 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_label(&chip, "New", 3));

  backend.fail_create = 1;
  CMP_TEST_EXPECT(m3_chip_set_style(&chip, &other_style), CMP_ERR_IO);
  backend.fail_create = 0;

  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_chip_set_style(&chip, &other_style), CMP_ERR_IO);
  backend.fail_destroy = 0;

  CMP_TEST_OK(m3_chip_set_style(&chip, &other_style));

  CMP_TEST_EXPECT(m3_chip_set_selected(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_set_selected(&chip, 2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_selected(&chip, CMP_TRUE));
  CMP_TEST_EXPECT(m3_chip_get_selected(NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_get_selected(&chip, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_get_selected(&chip, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(m3_chip_set_dense(NULL, CMP_TRUE), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_set_dense(&chip, 2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_dense(&chip, CMP_TRUE));
  CMP_TEST_EXPECT(m3_chip_get_dense(NULL, &handled), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_get_dense(&chip, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_get_dense(&chip, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(m3_chip_set_show_delete(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_set_show_delete(&chip, 2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_show_delete(&chip, CMP_TRUE));
  CMP_TEST_EXPECT(m3_chip_get_show_delete(NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_chip_get_show_delete(&chip, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_get_show_delete(&chip, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(m3_chip_set_on_click(NULL, test_chip_on_click, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_on_click(&chip, test_chip_on_click, NULL));
  CMP_TEST_EXPECT(m3_chip_set_on_delete(NULL, test_chip_on_delete, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_on_delete(&chip, test_chip_on_delete, NULL));

  width_spec.mode = 999u;
  width_spec.size = 10.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                              height_spec, &size),
                  CMP_ERR_RANGE);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                          height_spec, &size));
  expected_width = (CMPScalar)(chip.utf8_len * 10u) +
                   chip.style.dense_layout.padding_x * 2.0f +
                   chip.style.dense_layout.icon_gap +
                   chip.style.dense_layout.icon_size;
  expected_height = chip.style.dense_layout.min_height;
  if (expected_height <
      chip.metrics.height + chip.style.dense_layout.padding_y * 2.0f) {
    expected_height =
        chip.metrics.height + chip.style.dense_layout.padding_y * 2.0f;
  }
  if (expected_height < chip.style.dense_layout.icon_size +
                            chip.style.dense_layout.padding_y * 2.0f) {
    expected_height = chip.style.dense_layout.icon_size +
                      chip.style.dense_layout.padding_y * 2.0f;
  }
  CMP_TEST_ASSERT(cmp_near(size.width, expected_width, 0.01f));
  CMP_TEST_ASSERT(cmp_near(size.height, expected_height, 0.01f));

  dense_backup = chip.style.dense_layout;
  chip.style.dense_layout.min_height = 1.0f;
  chip.style.dense_layout.icon_size = 40.0f;
  chip.style.dense_layout.padding_y = 6.0f;
  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 20.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 20.0f, 0.01f));
  CMP_TEST_ASSERT(size.height >= chip.style.dense_layout.icon_size);
  chip.style.dense_layout = dense_backup;

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 50.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 20.0f;
  CMP_TEST_OK(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                          height_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 50.0f, 0.01f));
  CMP_TEST_ASSERT(size.height <= 20.0f + 0.01f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(chip.widget.vtable->layout(chip.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 120.0f;
  bounds.height = 32.0f;
  CMP_TEST_OK(chip.widget.vtable->layout(chip.widget.ctx, bounds));

  CMP_TEST_EXPECT(chip.widget.vtable->paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  chip.metrics_valid = CMP_FALSE;
  backend.fail_measure = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_measure = 0;

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RESOLVE_COLORS));
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RESOLVE_CORNER));
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_OUTLINE_WIDTH));
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  chip.style.outline_width = 100.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  chip.style.outline_width = other_style.outline_width;

  chip.style.outline_width = 0.0f;
  CMP_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.style.outline_width = other_style.outline_width;

  backend.fail_draw_rect = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_rect = 0;

  backend.fail_draw_text = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_text = 0;

  gfx.vtable = &g_test_gfx_vtable_no_line;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  backend.fail_draw_line = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), CMP_ERR_IO);
  backend.fail_draw_line = 0;

  chip.ripple.state = CMP_RIPPLE_STATE_EXPANDING;
  gfx.vtable = &g_test_gfx_vtable_no_clip;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;
  CMP_TEST_OK(cmp_ripple_start(&chip.ripple, 5.0f, 5.0f, 12.0f, 0.1f,
                               chip.style.ripple_color));
  CMP_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.ripple.state = CMP_RIPPLE_STATE_IDLE;

  CMP_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));

  CMP_TEST_OK(m3_chip_set_label(&chip, NULL, 0));
  CMP_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.metrics_valid = CMP_TRUE;
  chip.utf8_label = NULL;
  chip.utf8_len = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_set_label(&chip, "Chip", 4));

  CMP_TEST_EXPECT(chip.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_STATE);

  chip.pressed = CMP_FALSE;
  chip.style.ripple_expand_duration = -1.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  chip.style.ripple_expand_duration = other_style.ripple_expand_duration;

  chip.style.ripple_color.r = -1.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  chip.style.ripple_color = other_style.ripple_color;

  chip.bounds.width = -1.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  chip.bounds.width = 120.0f;

  chip.style.dense_layout.icon_size = 0.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  chip.style.dense_layout.icon_size = other_style.dense_layout.icon_size;

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_DELETE_BOUNDS));
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_RADIUS));
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_START));
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6));
  chip.pressed = CMP_FALSE;
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  chip.pressed = CMP_TRUE;
  chip.style.ripple_fade_duration = -1.0f;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  chip.style.ripple_fade_duration = other_style.ripple_fade_duration;
  chip.pressed = CMP_TRUE;

  CMP_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_RELEASE));
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_chip_test_clear_fail_points());
  chip.pressed = CMP_TRUE;

  clicks.calls = 0;
  clicks.fail = 0;
  CMP_TEST_OK(m3_chip_set_on_click(&chip, test_chip_on_click, &clicks));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(clicks.calls == 1);

  chip.pressed = CMP_TRUE;
  clicks.fail = 1;
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  clicks.fail = 0;

  chip.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  chip.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMP_TEST_OK(m3_chip_set_show_delete(&chip, CMP_TRUE));
  CMP_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  deletes.calls = 0;
  deletes.fail = 0;
  CMP_TEST_OK(m3_chip_set_on_delete(&chip, test_chip_on_delete, &deletes));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                                 (cmp_i32)(delete_bounds.x + 1.0f),
                                 (cmp_i32)(delete_bounds.y + 1.0f)));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP,
                                 (cmp_i32)(delete_bounds.x + 1.0f),
                                 (cmp_i32)(delete_bounds.y + 1.0f)));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(deletes.calls == 1);
  deletes.fail = 1;
  chip.pressed = CMP_TRUE;
  chip.pressed_delete = CMP_TRUE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6));
  CMP_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  deletes.fail = 0;

  chip.style.variant = M3_CHIP_VARIANT_FILTER;
  chip.selected = CMP_FALSE;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 6));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 6));
  CMP_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(chip.selected == CMP_TRUE);

  CMP_TEST_OK(chip.widget.vtable->get_semantics(chip.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_BUTTON);
  chip.widget.flags |= (CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE);
  CMP_TEST_OK(chip.widget.vtable->get_semantics(chip.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_SELECTED) != 0);
  chip.widget.flags &= ~(CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE);

  CMP_TEST_EXPECT(chip.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(chip.widget.vtable->get_semantics(chip.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(chip.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_chip_init(&destroy_chip, &text_backend, &style, "OK", 2));
  backend.fail_destroy = 1;
  CMP_TEST_EXPECT(destroy_chip.widget.vtable->destroy(destroy_chip.widget.ctx),
                  CMP_ERR_IO);
  backend.fail_destroy = 0;

  CMP_TEST_OK(m3_chip_init(&destroy_chip, &text_backend, &style, "OK", 2));
  destroy_chip.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(destroy_chip.widget.vtable->destroy(destroy_chip.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_chip_helpers() == 0);
  CMP_TEST_ASSERT(test_chip_draw_delete() == 0);
  CMP_TEST_ASSERT(test_chip_style_init() == 0);
  CMP_TEST_ASSERT(test_chip_widget() == 0);
  return 0;
}
