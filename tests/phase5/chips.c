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

int M3_CALL m3_chip_test_set_fail_point(m3_u32 fail_point);
int M3_CALL m3_chip_test_set_color_fail_after(m3_u32 call_count);
int M3_CALL m3_chip_test_clear_fail_points(void);
int M3_CALL m3_chip_test_validate_color(const M3Color *color);
int M3_CALL m3_chip_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                   M3Scalar b, M3Scalar a);
int M3_CALL m3_chip_test_color_with_alpha(const M3Color *base, M3Scalar alpha,
                                          M3Color *out_color);
int M3_CALL m3_chip_test_validate_text_style(const M3TextStyle *style,
                                             M3Bool require_family);
int M3_CALL m3_chip_test_validate_layout(const M3ChipLayout *layout);
int M3_CALL m3_chip_test_validate_style(const M3ChipStyle *style,
                                        M3Bool require_family);
int M3_CALL m3_chip_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_chip_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_chip_test_validate_backend(const M3TextBackend *backend);
int M3_CALL m3_chip_test_metrics_update(M3Chip *chip);
int M3_CALL m3_chip_test_resolve_colors(const M3Chip *chip,
                                        M3Color *out_background,
                                        M3Color *out_text, M3Color *out_outline,
                                        M3Color *out_ripple);
int M3_CALL m3_chip_test_resolve_corner(const M3Chip *chip,
                                        M3Scalar *out_corner);
int M3_CALL m3_chip_test_compute_delete_bounds(M3Chip *chip,
                                               M3Rect *out_bounds);
int M3_CALL m3_chip_test_draw_delete_icon(const M3Gfx *gfx,
                                          const M3Rect *bounds, M3Color color,
                                          M3Scalar thickness);

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
  M3Handle last_font;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_line_x0;
  M3Scalar last_line_y0;
  M3Scalar last_line_x1;
  M3Scalar last_line_y1;
  M3Scalar last_line_thickness;
  M3Color last_line_color;
  M3Scalar last_text_x;
  M3Scalar last_text_y;
  m3_usize last_text_len;
  M3Color last_text_color;
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
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return M3_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestChipBackend *backend;

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

  backend = (TestChipBackend *)text;
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
  TestChipBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)text;
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
  TestChipBackend *backend;

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

  backend = (TestChipBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return M3_ERR_IO;
  }

  *out_width = (M3Scalar)(utf8_len * 10u);
  *out_height = 18.0f;
  *out_baseline = 13.0f;
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestChipBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestChipBackend *)text;
  backend->draw_text_calls += 1;
  if (backend->fail_draw_text) {
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
  TestChipBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1,
                              M3Scalar y1, M3Color color, M3Scalar thickness) {
  TestChipBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_push_clip(void *gfx, const M3Rect *rect) {
  TestChipBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestChipBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestChipBackend *)gfx;
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
                                              test_gfx_draw_line,
                                              NULL,
                                              test_gfx_push_clip,
                                              test_gfx_pop_clip,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL};

static const M3GfxVTable g_test_gfx_vtable_no_rect = {NULL,
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

static const M3GfxVTable g_test_gfx_vtable_no_line = {NULL,
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

static const M3GfxVTable g_test_gfx_vtable_no_clip = {NULL,
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

static int m3_near(M3Scalar a, M3Scalar b, M3Scalar tol) {
  M3Scalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                              m3_i32 y) {
  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return M3_OK;
}

static int test_chip_on_click(void *ctx, struct M3Chip *chip) {
  ClickCounter *counter;

  if (ctx == NULL || chip == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ClickCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_chip_on_delete(void *ctx, struct M3Chip *chip) {
  DeleteCounter *counter;

  if (ctx == NULL || chip == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (DeleteCounter *)ctx;
  counter->calls += 1;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_chip_helpers(void) {
  M3Color color;
  M3Color out_color;
  M3TextStyle text_style;
  M3ChipLayout layout;
  M3ChipStyle style;
  M3ChipStyle bad_style;
  M3MeasureSpec spec;
  M3Rect rect;
  M3Chip chip;
  TestChipBackend backend;
  M3TextBackend text_backend;
  M3Color background;
  M3Color text_color;
  M3Color outline;
  M3Color ripple;
  M3Scalar corner;
  M3Rect delete_bounds;
  volatile const M3Color *null_color;
  volatile const M3TextStyle *null_style;
  volatile const M3ChipLayout *null_layout;
  volatile const M3ChipStyle *null_chip_style;
  volatile const M3Rect *null_rect;
  volatile const M3TextBackend *null_backend;

  null_color = NULL;
  null_style = NULL;
  null_layout = NULL;
  null_chip_style = NULL;
  null_rect = NULL;
  null_backend = NULL;

  M3_TEST_EXPECT(m3_chip_test_validate_color((const M3Color *)null_color),
                 M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  M3_TEST_EXPECT(m3_chip_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = -0.1f;
  M3_TEST_EXPECT(m3_chip_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 2.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_color(&color), M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_chip_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_test_color_set(&color, -1.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, -1.0f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, -1.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, 0.0f, 2.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_chip_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_chip_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(NULL, 0.5f, &out_color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, -0.1f, &out_color),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 1.5f, &out_color),
                 M3_ERR_RANGE);
  color.r = 2.0f;
  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, &out_color),
                 M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_OK(m3_chip_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_chip_test_color_with_alpha(&color, 0.5f, &out_color),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_EXPECT(m3_chip_test_validate_text_style(
                     (const M3TextStyle *)null_style, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  memset(&text_style, 0, sizeof(text_style));
  text_style.size_px = 12;
  text_style.weight = 400;
  text_style.italic = M3_FALSE;
  text_style.color.r = 0.0f;
  text_style.color.g = 0.0f;
  text_style.color.b = 0.0f;
  text_style.color.a = 1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  M3_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  M3_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  M3_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.italic = M3_FALSE;
  text_style.color.r = 2.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.color.r = 0.0f;
  M3_TEST_OK(m3_chip_test_validate_text_style(&text_style, M3_TRUE));

  M3_TEST_EXPECT(
      m3_chip_test_validate_layout((const M3ChipLayout *)null_layout),
      M3_ERR_INVALID_ARGUMENT);
  memset(&layout, 0, sizeof(layout));
  layout.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_layout(&layout), M3_ERR_RANGE);
  layout.padding_x = 0.0f;
  layout.min_width = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_layout(&layout), M3_ERR_RANGE);
  layout.min_width = 0.0f;
  layout.icon_size = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_layout(&layout), M3_ERR_RANGE);
  layout.icon_size = 10.0f;
  layout.icon_gap = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_layout(&layout), M3_ERR_RANGE);
  layout.icon_gap = 0.0f;
  layout.delete_icon_thickness = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_layout(&layout), M3_ERR_RANGE);
  layout.delete_icon_thickness = 1.0f;
  M3_TEST_OK(m3_chip_test_validate_layout(&layout));

  M3_TEST_EXPECT(m3_chip_test_validate_style(
                     (const M3ChipStyle *)null_chip_style, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_style_init_assist(&style));
  style.text_style.utf8_family = "Test";
  M3_TEST_OK(m3_chip_test_validate_style(&style, M3_TRUE));
  bad_style = style;
  bad_style.variant = 0;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);
  bad_style = style;
  bad_style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);
  bad_style = style;
  bad_style.ripple_expand_duration = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);
  bad_style = style;
  bad_style.layout.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);
  bad_style = style;
  bad_style.text_style.size_px = 0;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);
  bad_style = style;
  bad_style.background_color.r = 2.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_style(&bad_style, M3_TRUE),
                 M3_ERR_RANGE);

  spec.mode = 999u;
  spec.size = 10.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_measure_spec(spec), M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_chip_test_validate_rect((const M3Rect *)null_rect),
                 M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 10.0f;
  M3_TEST_EXPECT(m3_chip_test_validate_rect(&rect), M3_ERR_RANGE);

  M3_TEST_EXPECT(
      m3_chip_test_validate_backend((const M3TextBackend *)null_backend),
      M3_ERR_INVALID_ARGUMENT);
  memset(&text_backend, 0, sizeof(text_backend));
  M3_TEST_EXPECT(m3_chip_test_validate_backend(&text_backend),
                 M3_ERR_INVALID_ARGUMENT);

  memset(&chip, 0, sizeof(chip));
  chip.style = style;
  chip.widget.flags = 0;
  chip.selected = M3_TRUE;
  M3_TEST_OK(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                         &outline, &ripple));
  M3_TEST_ASSERT(
      m3_near(background.r, style.selected_background_color.r, 0.001f));
  M3_TEST_ASSERT(m3_near(text_color.r, style.selected_text_color.r, 0.001f));
  chip.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                         &outline, &ripple));
  M3_TEST_ASSERT(
      m3_near(background.a, style.disabled_background_color.a, 0.001f));
  chip.widget.flags = 0;
  chip.selected = M3_FALSE;
  chip.style.background_color.r = 2.0f;
  M3_TEST_EXPECT(m3_chip_test_resolve_colors(&chip, &background, &text_color,
                                             &outline, &ripple),
                 M3_ERR_RANGE);
  chip.style.background_color = style.background_color;

  chip.bounds.x = 0.0f;
  chip.bounds.y = 0.0f;
  chip.bounds.width = 20.0f;
  chip.bounds.height = 10.0f;
  chip.style.corner_radius = 20.0f;
  M3_TEST_OK(m3_chip_test_resolve_corner(&chip, &corner));
  M3_TEST_ASSERT(m3_near(corner, 5.0f, 0.001f));
  chip.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_chip_test_resolve_corner(&chip, &corner), M3_ERR_RANGE);
  chip.bounds.width = 20.0f;

  chip.metrics.width = 10.0f;
  chip.metrics.height = 18.0f;
  chip.metrics.baseline = 13.0f;
  chip.metrics_valid = M3_TRUE;
  chip.bounds.x = 0.0f;
  chip.bounds.y = 0.0f;
  chip.bounds.width = 120.0f;
  chip.bounds.height = 32.0f;
  chip.show_delete = M3_FALSE;
  M3_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  M3_TEST_ASSERT(delete_bounds.width == 0.0f);
  chip.show_delete = M3_TRUE;
  M3_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  M3_TEST_ASSERT(delete_bounds.width > 0.0f);
  chip.bounds.width = 10.0f;
  chip.bounds.height = 10.0f;
  M3_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  chip.bounds.width = 120.0f;
  chip.bounds.height = 32.0f;
  chip.style.layout.icon_size = 0.0f;
  M3_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                 M3_ERR_RANGE);
  chip.style.layout.icon_size = style.layout.icon_size;
  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_DELETE_BOUNDS));
  M3_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  chip.dense = 2;
  M3_TEST_EXPECT(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds),
                 M3_ERR_INVALID_ARGUMENT);
  chip.dense = M3_FALSE;

  M3_TEST_EXPECT(m3_chip_test_metrics_update(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(test_backend_init(&backend));
  text_backend.ctx = &backend;
  text_backend.vtable = &g_test_text_vtable;
  chip.text_backend = text_backend;
  chip.font.id = 1u;
  chip.font.generation = 1u;
  chip.utf8_label = "A";
  chip.utf8_len = 1;
  chip.metrics_valid = M3_FALSE;
  M3_TEST_OK(m3_chip_test_metrics_update(&chip));
  M3_TEST_ASSERT(chip.metrics_valid == M3_TRUE);
  chip.metrics_valid = M3_FALSE;
  backend.fail_measure = 1;
  M3_TEST_EXPECT(m3_chip_test_metrics_update(&chip), M3_ERR_IO);
  backend.fail_measure = 0;
  chip.metrics_valid = M3_TRUE;
  M3_TEST_OK(m3_chip_test_metrics_update(&chip));

  return 0;
}

static int test_chip_draw_delete(void) {
  TestChipBackend backend;
  M3Gfx gfx;
  M3Rect bounds;
  M3Color color;

  M3_TEST_OK(test_backend_init(&backend));
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

  M3_TEST_EXPECT(m3_chip_test_draw_delete_icon(NULL, &bounds, color, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, NULL, color, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, -1.0f),
                 M3_ERR_RANGE);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  bounds.width = 0.0f;
  M3_TEST_OK(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f));
  bounds.width = 10.0f;

  backend.fail_draw_line = 1;
  M3_TEST_EXPECT(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f),
                 M3_ERR_IO);
  backend.fail_draw_line = 0;

  M3_TEST_OK(m3_chip_test_draw_delete_icon(&gfx, &bounds, color, 1.0f));
  return 0;
}

static int test_chip_style_init(void) {
  M3ChipStyle style;

  M3_TEST_OK(m3_chip_style_init_assist(&style));
  M3_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_ASSIST);
  M3_TEST_OK(m3_chip_style_init_filter(&style));
  M3_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_FILTER);
  M3_TEST_OK(m3_chip_style_init_input(&style));
  M3_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_INPUT);
  M3_TEST_OK(m3_chip_style_init_suggestion(&style));
  M3_TEST_ASSERT(style.variant == M3_CHIP_VARIANT_SUGGESTION);

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_TEXT_STYLE_INIT));
  M3_TEST_EXPECT(m3_chip_style_init_assist(&style), M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_test_set_color_fail_after(1));
  M3_TEST_EXPECT(m3_chip_style_init_assist(&style), M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  return 0;
}

static int test_chip_widget(void) {
  TestChipBackend backend;
  M3TextBackend text_backend;
  M3Gfx gfx;
  M3PaintContext ctx;
  M3ChipStyle style;
  M3ChipStyle other_style;
  M3Chip chip;
  M3Chip destroy_chip;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  M3Semantics semantics;
  ClickCounter clicks;
  DeleteCounter deletes;
  M3Rect delete_bounds;
  M3Scalar expected_width;
  M3Scalar expected_height;

  M3_TEST_OK(test_backend_init(&backend));
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

  M3_TEST_OK(m3_chip_style_init_filter(&style));
  style.text_style.utf8_family = "Test";
  other_style = style;
  other_style.text_style.size_px = 16;

  M3_TEST_EXPECT(m3_chip_init(NULL, &text_backend, &style, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_init(&chip, NULL, &style, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, NULL, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);

  text_backend.vtable = NULL;
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  text_backend.vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                 M3_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable_no_create;
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                 M3_ERR_UNSUPPORTED);
  text_backend.vtable = &g_test_text_vtable;

  style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1),
                 M3_ERR_INVALID_ARGUMENT);
  style.text_style.utf8_family = "Test";

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_INIT));
  M3_TEST_EXPECT(m3_chip_init(&chip, &text_backend, &style, "A", 1), M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_init(&chip, &text_backend, &style, "Chip", 4));
  M3_TEST_ASSERT(chip.widget.ctx == &chip);
  M3_TEST_ASSERT(chip.widget.vtable != NULL);
  M3_TEST_ASSERT((chip.widget.flags & M3_WIDGET_FLAG_FOCUSABLE) != 0);

  M3_TEST_EXPECT(m3_chip_set_label(NULL, "A", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_set_label(&chip, NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_label(&chip, "New", 3));

  backend.fail_create = 1;
  M3_TEST_EXPECT(m3_chip_set_style(&chip, &other_style), M3_ERR_IO);
  backend.fail_create = 0;

  backend.fail_destroy = 1;
  M3_TEST_EXPECT(m3_chip_set_style(&chip, &other_style), M3_ERR_IO);
  backend.fail_destroy = 0;

  M3_TEST_OK(m3_chip_set_style(&chip, &other_style));

  M3_TEST_EXPECT(m3_chip_set_selected(NULL, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_set_selected(&chip, 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_selected(&chip, M3_TRUE));
  M3_TEST_EXPECT(m3_chip_get_selected(NULL, &handled), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_get_selected(&chip, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_get_selected(&chip, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(m3_chip_set_dense(NULL, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_set_dense(&chip, 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_dense(&chip, M3_TRUE));
  M3_TEST_EXPECT(m3_chip_get_dense(NULL, &handled), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_get_dense(&chip, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_get_dense(&chip, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(m3_chip_set_show_delete(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_set_show_delete(&chip, 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_show_delete(&chip, M3_TRUE));
  M3_TEST_EXPECT(m3_chip_get_show_delete(NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_chip_get_show_delete(&chip, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_get_show_delete(&chip, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(m3_chip_set_on_click(NULL, test_chip_on_click, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_on_click(&chip, test_chip_on_click, NULL));
  M3_TEST_EXPECT(m3_chip_set_on_delete(NULL, test_chip_on_delete, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_on_delete(&chip, test_chip_on_delete, NULL));

  width_spec.mode = 999u;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                             height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                             height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                         height_spec, &size));
  expected_width = (M3Scalar)(chip.utf8_len * 10u) +
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
  M3_TEST_ASSERT(m3_near(size.width, expected_width, 0.01f));
  M3_TEST_ASSERT(m3_near(size.height, expected_height, 0.01f));

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 50.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 20.0f;
  M3_TEST_OK(chip.widget.vtable->measure(chip.widget.ctx, width_spec,
                                         height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 50.0f, 0.01f));
  M3_TEST_ASSERT(size.height <= 20.0f + 0.01f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(chip.widget.vtable->layout(chip.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 120.0f;
  bounds.height = 32.0f;
  M3_TEST_OK(chip.widget.vtable->layout(chip.widget.ctx, bounds));

  M3_TEST_EXPECT(chip.widget.vtable->paint(NULL, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  chip.metrics_valid = M3_FALSE;
  backend.fail_measure = 1;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  backend.fail_measure = 0;

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RESOLVE_COLORS));
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RESOLVE_CORNER));
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_OUTLINE_WIDTH));
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  chip.style.outline_width = 100.0f;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  chip.style.outline_width = other_style.outline_width;

  chip.style.outline_width = 0.0f;
  M3_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.style.outline_width = other_style.outline_width;

  backend.fail_draw_rect = 1;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  backend.fail_draw_rect = 0;

  backend.fail_draw_text = 1;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  backend.fail_draw_text = 0;

  gfx.vtable = &g_test_gfx_vtable_no_line;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  backend.fail_draw_line = 1;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx), M3_ERR_IO);
  backend.fail_draw_line = 0;

  chip.ripple.state = M3_RIPPLE_STATE_EXPANDING;
  gfx.vtable = &g_test_gfx_vtable_no_clip;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;
  M3_TEST_OK(m3_ripple_start(&chip.ripple, 5.0f, 5.0f, 12.0f, 0.1f,
                             chip.style.ripple_color));
  M3_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.ripple.state = M3_RIPPLE_STATE_IDLE;

  M3_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));

  M3_TEST_OK(m3_chip_set_label(&chip, NULL, 0));
  M3_TEST_OK(chip.widget.vtable->paint(chip.widget.ctx, &ctx));
  chip.metrics_valid = M3_TRUE;
  chip.utf8_label = NULL;
  chip.utf8_len = 1;
  M3_TEST_EXPECT(chip.widget.vtable->paint(chip.widget.ctx, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_set_label(&chip, "Chip", 4));

  M3_TEST_EXPECT(chip.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_STATE);

  chip.pressed = M3_FALSE;
  chip.style.ripple_expand_duration = -1.0f;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  chip.style.ripple_expand_duration = other_style.ripple_expand_duration;

  chip.style.ripple_color.r = -1.0f;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  chip.style.ripple_color = other_style.ripple_color;

  chip.bounds.width = -1.0f;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  chip.bounds.width = 120.0f;

  chip.style.dense_layout.icon_size = 0.0f;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  chip.style.dense_layout.icon_size = other_style.dense_layout.icon_size;

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_DELETE_BOUNDS));
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_RADIUS));
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_START));
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  chip.pressed = M3_FALSE;
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  chip.pressed = M3_TRUE;
  chip.style.ripple_fade_duration = -1.0f;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  chip.style.ripple_fade_duration = other_style.ripple_fade_duration;
  chip.pressed = M3_TRUE;

  M3_TEST_OK(m3_chip_test_set_fail_point(M3_CHIP_TEST_FAIL_RIPPLE_RELEASE));
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_OK(m3_chip_test_clear_fail_points());
  chip.pressed = M3_TRUE;

  clicks.calls = 0;
  clicks.fail = 0;
  M3_TEST_OK(m3_chip_set_on_click(&chip, test_chip_on_click, &clicks));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(clicks.calls == 1);

  chip.pressed = M3_TRUE;
  clicks.fail = 1;
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  clicks.fail = 0;

  chip.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  chip.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(m3_chip_set_show_delete(&chip, M3_TRUE));
  M3_TEST_OK(m3_chip_test_compute_delete_bounds(&chip, &delete_bounds));
  deletes.calls = 0;
  deletes.fail = 0;
  M3_TEST_OK(m3_chip_set_on_delete(&chip, test_chip_on_delete, &deletes));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(delete_bounds.x + 1.0f),
                                (m3_i32)(delete_bounds.y + 1.0f)));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP,
                                (m3_i32)(delete_bounds.x + 1.0f),
                                (m3_i32)(delete_bounds.y + 1.0f)));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(deletes.calls == 1);
  deletes.fail = 1;
  chip.pressed = M3_TRUE;
  chip.pressed_delete = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_EXPECT(chip.widget.vtable->event(chip.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  deletes.fail = 0;

  chip.style.variant = M3_CHIP_VARIANT_FILTER;
  chip.selected = M3_FALSE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(chip.widget.vtable->event(chip.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(chip.selected == M3_TRUE);

  M3_TEST_OK(chip.widget.vtable->get_semantics(chip.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_BUTTON);
  chip.widget.flags |= (M3_WIDGET_FLAG_DISABLED | M3_WIDGET_FLAG_FOCUSABLE);
  M3_TEST_OK(chip.widget.vtable->get_semantics(chip.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_FOCUSABLE) != 0);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_SELECTED) != 0);
  chip.widget.flags &= ~(M3_WIDGET_FLAG_DISABLED | M3_WIDGET_FLAG_FOCUSABLE);

  M3_TEST_EXPECT(chip.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(chip.widget.vtable->get_semantics(chip.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(chip.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_chip_init(&destroy_chip, &text_backend, &style, "OK", 2));
  backend.fail_destroy = 1;
  M3_TEST_EXPECT(destroy_chip.widget.vtable->destroy(destroy_chip.widget.ctx),
                 M3_ERR_IO);
  backend.fail_destroy = 0;

  M3_TEST_OK(m3_chip_init(&destroy_chip, &text_backend, &style, "OK", 2));
  destroy_chip.text_backend.vtable = NULL;
  M3_TEST_EXPECT(destroy_chip.widget.vtable->destroy(destroy_chip.widget.ctx),
                 M3_ERR_UNSUPPORTED);

  return 0;
}

int main(void) {
  M3_TEST_ASSERT(test_chip_helpers() == 0);
  M3_TEST_ASSERT(test_chip_draw_delete() == 0);
  M3_TEST_ASSERT(test_chip_style_init() == 0);
  M3_TEST_ASSERT(test_chip_widget() == 0);
  return 0;
}
