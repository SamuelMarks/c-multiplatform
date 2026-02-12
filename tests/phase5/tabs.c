#include "m3/m3_tabs.h"
#include "test_utils.h"

#include <string.h>

#define M3_TAB_ROW_TEST_FAIL_STYLE_INIT 1u
#define M3_TAB_ROW_TEST_FAIL_ANIM_INIT 2u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_START 3u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 4u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE 5u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE 6u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE 7u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE 8u
#define M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE 9u
#define M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE 10u

#define M3_SEGMENTED_TEST_FAIL_STYLE_INIT 1u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 2u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE 3u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE 4u
#define M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE 5u
#define M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE 6u

int M3_CALL m3_tab_row_test_set_fail_point(m3_u32 point);
int M3_CALL m3_tab_row_test_set_color_fail_after(m3_u32 call_count);
int M3_CALL m3_tab_row_test_clear_fail_points(void);
int M3_CALL m3_tab_row_test_validate_color(const M3Color *color);
int M3_CALL m3_tab_row_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                      M3Scalar b, M3Scalar a);
int M3_CALL m3_tab_row_test_color_with_alpha(const M3Color *base,
                                             M3Scalar alpha,
                                             M3Color *out_color);
int M3_CALL m3_tab_row_test_validate_edges(const M3LayoutEdges *edges);
int M3_CALL m3_tab_row_test_validate_text_style(const M3TextStyle *style,
                                                M3Bool require_family);
int M3_CALL m3_tab_row_test_validate_style(const M3TabRowStyle *style,
                                           M3Bool require_family);
int M3_CALL m3_tab_row_test_validate_items(const M3TabItem *items,
                                           m3_usize count);
int M3_CALL m3_tab_row_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_tab_row_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_tab_row_test_validate_backend(const M3TextBackend *backend);
int M3_CALL m3_tab_row_test_measure_content(const M3TabRow *row, m3_u32 mode,
                                            M3Scalar *out_width,
                                            M3Scalar *out_height);
int M3_CALL m3_tab_row_test_color_should_fail_null(void);
int M3_CALL m3_tab_row_test_fail_point_match_null(void);
int M3_CALL m3_tab_row_test_force_color_error(M3Bool enable);
int M3_CALL m3_tab_row_test_set_fail_point_error_after(m3_u32 call_count);
int M3_CALL m3_tab_row_test_set_value_fail_after(m3_u32 call_count);
int M3_CALL m3_tab_row_test_set_start_fail_after(m3_u32 call_count);
int M3_CALL m3_tab_row_test_measure_max_text(const M3TabRow *row,
                                             M3Scalar *out_width,
                                             M3Scalar *out_height,
                                             M3Scalar *out_baseline);
int M3_CALL m3_tab_row_test_item_width(const M3TabRow *row,
                                       const M3TabItem *item,
                                       M3Scalar *out_width);
int M3_CALL m3_tab_row_test_compute_layout(const M3TabRow *row,
                                           M3Scalar *out_content_width,
                                           M3Scalar *out_tab_width,
                                           M3Scalar *out_tab_height,
                                           m3_u32 *out_mode);
int M3_CALL m3_tab_row_test_clamp_scroll(M3TabRow *row, m3_u32 mode,
                                         M3Scalar content_width,
                                         M3Scalar available_width);
int M3_CALL m3_tab_row_test_indicator_target(
    const M3TabRow *row, m3_u32 mode, M3Scalar tab_width, M3Scalar spacing,
    M3Scalar content_width, M3Scalar start_x, M3Scalar start_y,
    M3Scalar tab_height, M3Scalar *out_pos, M3Scalar *out_width);
int M3_CALL m3_tab_row_test_sync_indicator(M3TabRow *row, m3_u32 mode,
                                           M3Scalar tab_width, M3Scalar spacing,
                                           M3Scalar content_width,
                                           M3Scalar start_x, M3Scalar start_y,
                                           M3Scalar tab_height, M3Bool animate);
int M3_CALL m3_tab_row_test_item_rect(const M3TabRow *row, m3_u32 mode,
                                      M3Scalar start_x, M3Scalar start_y,
                                      M3Scalar tab_width, M3Scalar tab_height,
                                      M3Scalar spacing, M3Scalar content_width,
                                      M3Scalar available_width, m3_usize index,
                                      M3Rect *out_rect);
int M3_CALL m3_tab_row_test_hit_test(const M3TabRow *row, m3_u32 mode,
                                     M3Scalar start_x, M3Scalar start_y,
                                     M3Scalar tab_width, M3Scalar tab_height,
                                     M3Scalar spacing, M3Scalar content_width,
                                     M3Scalar available_width, m3_i32 x,
                                     m3_i32 y, m3_usize *out_index);

int M3_CALL m3_segmented_test_set_fail_point(m3_u32 point);
int M3_CALL m3_segmented_test_set_color_fail_after(m3_u32 call_count);
int M3_CALL m3_segmented_test_clear_fail_points(void);
int M3_CALL m3_segmented_test_validate_color(const M3Color *color);
int M3_CALL m3_segmented_test_color_set(M3Color *color, M3Scalar r, M3Scalar g,
                                        M3Scalar b, M3Scalar a);
int M3_CALL m3_segmented_test_color_with_alpha(const M3Color *base,
                                               M3Scalar alpha,
                                               M3Color *out_color);
int M3_CALL m3_segmented_test_validate_edges(const M3LayoutEdges *edges);
int M3_CALL m3_segmented_test_validate_text_style(const M3TextStyle *style,
                                                  M3Bool require_family);
int M3_CALL m3_segmented_test_validate_style(const M3SegmentedStyle *style,
                                             M3Bool require_family);
int M3_CALL m3_segmented_test_validate_items(const M3SegmentedItem *items,
                                             m3_usize count);
int M3_CALL m3_segmented_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_segmented_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_segmented_test_validate_backend(const M3TextBackend *backend);
int M3_CALL m3_segmented_test_validate_mode(m3_u32 mode);
int M3_CALL m3_segmented_test_validate_selected_states(const M3Bool *states,
                                                       m3_usize count);
int M3_CALL m3_segmented_test_measure_content(const M3SegmentedButtons *buttons,
                                              M3Scalar *out_width,
                                              M3Scalar *out_height);
int M3_CALL m3_segmented_test_color_should_fail_null(void);
int M3_CALL m3_segmented_test_fail_point_match_null(void);
int M3_CALL m3_segmented_test_force_color_error(M3Bool enable);
int M3_CALL m3_segmented_test_set_fail_point_error_after(m3_u32 call_count);
int M3_CALL m3_segmented_test_measure_max_text(
    const M3SegmentedButtons *buttons, M3Scalar *out_width,
    M3Scalar *out_height, M3Scalar *out_baseline);
int M3_CALL m3_segmented_test_compute_layout(const M3SegmentedButtons *buttons,
                                             M3Scalar *out_content_width,
                                             M3Scalar *out_segment_width,
                                             M3Scalar *out_segment_height,
                                             M3Scalar *out_spacing);
int M3_CALL m3_segmented_test_hit_test(const M3SegmentedButtons *buttons,
                                       M3Scalar start_x, M3Scalar start_y,
                                       M3Scalar segment_width,
                                       M3Scalar segment_height,
                                       M3Scalar spacing, M3Scalar content_width,
                                       m3_i32 x, m3_i32 y, m3_usize *out_index);
int M3_CALL m3_segmented_test_is_selected(const M3SegmentedButtons *buttons,
                                          m3_usize index, M3Bool *out_selected);

typedef struct TestTabsBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_text_calls;
  int draw_rect_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_draw_text;
  int fail_draw_rect;
  M3Handle last_font;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_text_x;
  M3Scalar last_text_y;
  m3_usize last_text_len;
  M3Color last_text_color;
} TestTabsBackend;

typedef struct TabSelectState {
  int calls;
  m3_usize last_index;
  m3_usize fail_index;
} TabSelectState;

typedef struct SegmentedSelectState {
  int calls;
  m3_usize last_index;
  M3Bool last_selected;
  m3_usize fail_index;
} SegmentedSelectState;

static int test_backend_init(TestTabsBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return M3_OK;
}

static int setup_text_backend(TestTabsBackend *state, M3TextBackend *backend,
                              const M3TextVTable *vtable) {
  if (state == NULL || backend == NULL || vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  backend->ctx = state;
  backend->vtable = vtable;
  return M3_OK;
}

static int setup_gfx(TestTabsBackend *state, M3Gfx *gfx, M3PaintContext *ctx,
                     const M3GfxVTable *vtable,
                     const M3TextVTable *text_vtable) {
  if (state == NULL || gfx == NULL || ctx == NULL || vtable == NULL ||
      text_vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  gfx->ctx = state;
  gfx->vtable = vtable;
  gfx->text_vtable = text_vtable;
  ctx->gfx = gfx;
  ctx->clip.x = 0.0f;
  ctx->clip.y = 0.0f;
  ctx->clip.width = 1000.0f;
  ctx->clip.height = 1000.0f;
  ctx->dpi_scale = 1.0f;
  return M3_OK;
}

static int tabs_near(M3Scalar a, M3Scalar b, M3Scalar tol, M3Bool *out_near) {
  M3Scalar diff;

  if (out_near == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  *out_near = (diff <= tol) ? M3_TRUE : M3_FALSE;
  return M3_OK;
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

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestTabsBackend *backend;

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

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

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

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTabsBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int tab_on_select(void *ctx, struct M3TabRow *row, m3_usize index) {
  TabSelectState *state;

  if (ctx == NULL || row == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TabSelectState *)ctx;
  state->calls += 1;
  state->last_index = index;
  if (index == state->fail_index) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int segmented_on_select(void *ctx, struct M3SegmentedButtons *buttons,
                               m3_usize index, M3Bool selected) {
  SegmentedSelectState *state;

  if (ctx == NULL || buttons == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (SegmentedSelectState *)ctx;
  state->calls += 1;
  state->last_index = index;
  state->last_selected = selected;
  if (index == state->fail_index) {
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

static const M3TextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font, NULL, test_text_measure_text, test_text_draw_text};

static const M3TextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const M3GfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const M3GfxVTable g_test_gfx_vtable_no_rect = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static int test_tab_row_style_init(void) {
  M3TabRowStyle style;
  M3Bool near;
  m3_u32 i;

  M3_TEST_EXPECT(m3_tab_row_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tab_row_style_init(&style));
  M3_TEST_ASSERT(style.mode == M3_TAB_MODE_FIXED);
  M3_TEST_OK(
      tabs_near(style.min_width, M3_TAB_DEFAULT_MIN_WIDTH, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(
      tabs_near(style.min_height, M3_TAB_DEFAULT_MIN_HEIGHT, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(
      tabs_near(style.padding_x, M3_TAB_DEFAULT_PADDING_X, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(
      tabs_near(style.padding_y, M3_TAB_DEFAULT_PADDING_Y, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.indicator_thickness,
                       M3_TAB_DEFAULT_INDICATOR_THICKNESS, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.indicator_anim_duration,
                       M3_TAB_DEFAULT_INDICATOR_DURATION, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_ASSERT(style.indicator_anim_easing ==
                 M3_TAB_DEFAULT_INDICATOR_EASING);
  M3_TEST_ASSERT(style.text_style.size_px == 14);
  M3_TEST_ASSERT(style.text_style.weight == 400);
  M3_TEST_ASSERT(style.text_style.italic == M3_FALSE);
  M3_TEST_ASSERT(style.text_style.utf8_family == NULL);
  M3_TEST_OK(tabs_near(style.background_color.a, 0.0f, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);

  M3_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_STYLE_INIT));
  M3_TEST_EXPECT(m3_tab_row_style_init(&style), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  for (i = 1u; i <= 5u; ++i) {
    M3_TEST_OK(m3_tab_row_test_set_color_fail_after(i));
    M3_TEST_EXPECT(m3_tab_row_style_init(&style), M3_ERR_IO);
    M3_TEST_OK(m3_tab_row_test_clear_fail_points());
  }

  return M3_OK;
}

static int test_tab_row_validation_helpers(void) {
  M3TabRowStyle style;
  M3TabRowStyle base_style;
  M3TextStyle text_style;
  M3Color color;
  M3LayoutEdges edges;
  M3MeasureSpec spec;
  M3Rect rect;
  M3TabItem item;
  M3TabRow row;
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3TabItem items[2];
  M3Scalar width;
  M3Scalar height;

  M3_TEST_EXPECT(m3_tab_row_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.5f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.5f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.5f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_tab_row_test_validate_color(&color));

  M3_TEST_EXPECT(m3_tab_row_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_color_set(&color, -1.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f));

  M3_TEST_EXPECT(m3_tab_row_test_color_with_alpha(NULL, 0.5f, &color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, -1.0f, &color),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color));

  M3_TEST_EXPECT(m3_tab_row_test_validate_edges(NULL), M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 0.0f;
  edges.bottom = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.bottom = 0.0f;
  M3_TEST_OK(m3_tab_row_test_validate_edges(&edges));

  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tab_row_test_validate_text_style(&text_style, M3_FALSE));
  text_style.utf8_family = "Sans";
  text_style.size_px = 0;
  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 3;
  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.italic = M3_FALSE;
  text_style.color.a = 2.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.color.a = 1.0f;
  M3_TEST_OK(m3_tab_row_test_validate_text_style(&text_style, M3_TRUE));

  M3_TEST_EXPECT(m3_tab_row_test_validate_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  base_style = style;
  style.mode = 99u;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.spacing = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.min_width = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.min_height = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.padding_y = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.indicator_thickness = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.indicator_corner = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.indicator_anim_duration = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.indicator_anim_easing = 99u;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  style = base_style;
  style.selected_text_color.a = 2.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  style.disabled_indicator_color.a = 2.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  style = base_style;
  M3_TEST_OK(m3_tab_row_test_validate_style(&style, M3_TRUE));

  M3_TEST_EXPECT(m3_tab_row_test_validate_items(NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tab_row_test_validate_items(NULL, 0));
  item.utf8_label = NULL;
  item.utf8_len = 1;
  M3_TEST_EXPECT(m3_tab_row_test_validate_items(&item, 1),
                 M3_ERR_INVALID_ARGUMENT);
  item.utf8_len = 0;
  M3_TEST_OK(m3_tab_row_test_validate_items(&item, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  M3_TEST_OK(m3_tab_row_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_tab_row_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.height = 1.0f;
  M3_TEST_OK(m3_tab_row_test_validate_rect(&rect));

  M3_TEST_EXPECT(m3_tab_row_test_validate_backend(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  backend.ctx = NULL;
  backend.vtable = NULL;
  M3_TEST_EXPECT(m3_tab_row_test_validate_backend(&backend),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(m3_tab_row_test_validate_backend(&backend));
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  M3_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));
  M3_TEST_EXPECT(
      m3_tab_row_test_measure_content(NULL, M3_TAB_MODE_FIXED, &width, &height),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_measure_content(&row, 99u, &width, &height),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width,
                                             &height));
  M3_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                             &width, &height));
  M3_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));

  return M3_OK;
}

static int test_tab_row_internal_helpers(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[2];
  M3Color color;
  M3Scalar width;
  M3Scalar height;
  M3Scalar baseline;
  M3Scalar content_width;
  M3Scalar tab_width;
  M3Scalar tab_height;
  m3_u32 layout_mode;
  M3Rect rect;
  m3_usize index;

  M3_TEST_EXPECT(m3_tab_row_test_color_should_fail_null(),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_fail_point_match_null(),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_force_color_error(M3_TRUE));
  M3_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_IO);

  color.r = 0.0f;
  color.g = 2.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color),
                 M3_ERR_RANGE);
  color.g = 0.0f;
  M3_TEST_OK(m3_tab_row_test_force_color_error(M3_TRUE));
  M3_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color),
                 M3_ERR_IO);

  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.indicator_color.r = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_text_color.r = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_indicator_color.r = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_validate_style(&style, M3_TRUE), M3_ERR_RANGE);

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  M3_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  M3_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(NULL, &width, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, NULL, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  row.items = NULL;
  row.item_count = 1;
  M3_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  row.items = items;
  row.item_count = 2;
  M3_TEST_OK(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline));

  M3_TEST_EXPECT(m3_tab_row_test_item_width(NULL, &items[0], &width),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_item_width(&row, NULL, &width),
                 M3_ERR_INVALID_ARGUMENT);
  backend_state.fail_measure = 1;
  M3_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], &width),
                 M3_ERR_IO);
  backend_state.fail_measure = 0;
  row.style.padding_x = -10.0f;
  row.style.min_width = -100.0f;
  M3_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], &width),
                 M3_ERR_RANGE);
  row.style.padding_x = style.padding_x;
  row.style.min_width = style.min_width;

  M3_TEST_EXPECT(m3_tab_row_test_measure_content(&row, 99u, &width, &height),
                 M3_ERR_RANGE);
  row.item_count = 0;
  M3_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width,
                                             &height));
  row.item_count = 2;
  backend_state.fail_measure = 1;
  M3_TEST_EXPECT(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                                 &width, &height),
                 M3_ERR_IO);
  backend_state.fail_measure = 0;
  row.style.padding.left = -100.0f;
  row.item_count = 0;
  M3_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      M3_ERR_RANGE);
  row.style.padding.left = style.padding.left;
  row.item_count = 2;

  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(NULL, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, NULL, &tab_width,
                                                &tab_height, &layout_mode),
                 M3_ERR_INVALID_ARGUMENT);
  row.style.mode = 99u;
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_RANGE);
  row.style.mode = M3_TAB_MODE_FIXED;
  row.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_RANGE);
  row.bounds.width = 100.0f;
  row.bounds.height = 50.0f;

  row.style.padding.left = 60.0f;
  row.style.padding.right = 60.0f;
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_RANGE);
  row.style.padding.left = style.padding.left;
  row.style.padding.right = style.padding.right;

  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(2u));
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(3u));
  M3_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                &tab_width, &tab_height,
                                                &layout_mode),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  M3_TEST_EXPECT(
      m3_tab_row_test_clamp_scroll(NULL, M3_TAB_MODE_SCROLLABLE, 10.0f, 5.0f),
      M3_ERR_INVALID_ARGUMENT);
  row.scroll_offset = -1.0f;
  M3_TEST_EXPECT(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_SCROLLABLE, 10.0f, 5.0f),
      M3_ERR_RANGE);
  row.scroll_offset = 0.0f;
  M3_TEST_OK(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_FIXED, 10.0f, 5.0f));

  M3_TEST_EXPECT(m3_tab_row_test_indicator_target(NULL, M3_TAB_MODE_FIXED,
                                                  10.0f, 0.0f, 10.0f, 0.0f,
                                                  0.0f, 10.0f, &width, &height),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, 99u, 10.0f, 0.0f, 10.0f,
                                                  0.0f, 0.0f, 10.0f, &width,
                                                  &height),
                 M3_ERR_RANGE);
  row.selected_index = 1;
  row.item_count = 2;
  backend_state.fail_measure = 1;
  M3_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, M3_TAB_MODE_SCROLLABLE,
                                                  10.0f, 0.0f, 20.0f, 0.0f,
                                                  0.0f, 10.0f, &width, &height),
                 M3_ERR_IO);
  backend_state.fail_measure = 0;
  M3_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, M3_TAB_MODE_FIXED,
                                                  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                  10.0f, &width, &height),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                (M3Bool)2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, 99u, 10.0f, 0.0f, 10.0f,
                                                0.0f, 0.0f, 10.0f, M3_FALSE),
                 M3_ERR_RANGE);

  row.style.indicator_anim_duration = -1.0f;
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_TRUE),
                 M3_ERR_RANGE);
  row.style.indicator_anim_duration = 0.2f;
  row.style.indicator_anim_easing = 99u;
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  row.style.indicator_anim_easing = M3_TAB_DEFAULT_INDICATOR_EASING;

  M3_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_TRUE),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_start_fail_after(2u));
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_FALSE),
                 M3_ERR_IO);

  M3_TEST_OK(m3_tab_row_test_set_value_fail_after(1u));
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_FALSE),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_value_fail_after(2u));
  M3_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                M3_FALSE),
                 M3_ERR_IO);

  M3_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  M3_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0u,
                                           &rect),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_EXPECT(m3_tab_row_test_item_rect(NULL, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0u,
                                           &rect),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 5u,
                                           &rect),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_tab_row_test_item_rect(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                           0.0f, 10.0f, 10.0f, 0u, &rect),
                 M3_ERR_RANGE);

  index = M3_TAB_INVALID_INDEX;
  M3_TEST_EXPECT(m3_tab_row_test_hit_test(NULL, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                          10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0,
                                          0, &index),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_test_hit_test(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                          0.0f, 10.0f, 10.0f, 0, 0, &index),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  M3_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                          10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0,
                                          0, &index),
                 M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  {
    M3Bool changed;

    M3_TEST_OK(m3_anim_controller_start_timing(
        &row.indicator_pos_anim, 0.0f, 1.0f, 1.0f, M3_ANIM_EASE_LINEAR));
    M3_TEST_OK(m3_anim_controller_start_timing(
        &row.indicator_width_anim, 0.0f, 2.0f, 1.0f, M3_ANIM_EASE_LINEAR));
    row.indicator_pos = 10.0f;
    row.indicator_width = 10.0f;
    M3_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
    M3_TEST_ASSERT(changed == M3_TRUE);
  }

  M3_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return M3_OK;
}

static int test_segmented_internal_helpers(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedItem items[2];
  M3Color color;
  M3Scalar width;
  M3Scalar height;
  M3Scalar baseline;
  M3Scalar content_width;
  M3Scalar segment_width;
  M3Scalar segment_height;
  M3Scalar spacing;
  m3_usize index;
  M3Bool selected;

  M3_TEST_EXPECT(m3_segmented_test_color_should_fail_null(),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_test_fail_point_match_null(),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_force_color_error(M3_TRUE));
  M3_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_IO);

  color.r = 0.0f;
  color.g = 2.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 0.5f, &color),
                 M3_ERR_RANGE);
  color.g = 0.0f;
  M3_TEST_OK(m3_segmented_test_force_color_error(M3_TRUE));
  M3_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 0.5f, &color),
                 M3_ERR_IO);

  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.r = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.outline_color.r = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_text_color.r = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  M3_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                       M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  M3_TEST_EXPECT(
      m3_segmented_test_measure_max_text(NULL, &width, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, NULL, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  buttons.items = NULL;
  buttons.item_count = 1;
  M3_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline),
      M3_ERR_INVALID_ARGUMENT);
  buttons.items = items;
  buttons.item_count = 2;
  M3_TEST_OK(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline));

  M3_TEST_EXPECT(m3_segmented_test_measure_content(NULL, &width, &height),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.item_count = 0;
  M3_TEST_OK(m3_segmented_test_measure_content(&buttons, &width, &height));
  buttons.item_count = 2;
  buttons.style.padding.left = -100.0f;
  buttons.item_count = 0;
  M3_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, &height),
                 M3_ERR_RANGE);
  buttons.style.padding.left = style.padding.left;
  buttons.item_count = 2;

  M3_TEST_EXPECT(m3_segmented_test_compute_layout(NULL, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(
                     &buttons, NULL, &segment_width, &segment_height, &spacing),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.bounds.width = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_RANGE);
  buttons.bounds.width = 100.0f;
  buttons.bounds.height = 40.0f;
  buttons.style.padding.left = 60.0f;
  buttons.style.padding.right = 60.0f;
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_RANGE);
  buttons.style.padding.left = style.padding.left;
  buttons.style.padding.right = style.padding.right;

  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_IO);
  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(2u));
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_IO);
  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(3u));
  M3_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                  &segment_width,
                                                  &segment_height, &spacing),
                 M3_ERR_IO);
  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  index = 0;
  M3_TEST_EXPECT(m3_segmented_test_hit_test(NULL, 0.0f, 0.0f, 10.0f, 10.0f,
                                            0.0f, 10.0f, 0, 0, &index),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  M3_TEST_EXPECT(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                            0.0f, 10.0f, 0, 0, &index),
                 M3_ERR_IO);
  M3_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  M3_TEST_EXPECT(m3_segmented_test_is_selected(NULL, 0u, &selected),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.mode = M3_SEGMENTED_MODE_MULTI;
  M3_TEST_EXPECT(m3_segmented_test_is_selected(&buttons, 0u, &selected),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;
  buttons.selected_index = M3_SEGMENTED_INVALID_INDEX;
  M3_TEST_OK(m3_segmented_test_is_selected(&buttons, 0u, &selected));
  M3_TEST_ASSERT(selected == M3_FALSE);

  M3_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));
  return M3_OK;
}

static int test_tab_row_init_and_setters(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3TextBackend bad_backend;
  M3TabRowStyle style;
  M3TabRowStyle bad_style;
  M3TabRow row;
  M3TabRow row2;
  M3TabRow row3;
  M3TabItem items[2];
  M3TabItem bad_items[1];
  M3Bool changed;
  m3_usize selected;
  M3Scalar scroll;

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));

  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "Alpha";
  items[0].utf8_len = 5;
  items[1].utf8_label = "Beta";
  items[1].utf8_len = 4;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 3;

  M3_TEST_EXPECT(m3_tab_row_init(NULL, &backend, &style, items, 2, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_init(&row, NULL, &style, items, 2, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_init(&row, &backend, NULL, items, 2, 0),
                 M3_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = NULL;
  bad_backend.vtable = NULL;
  M3_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                 M3_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = &backend_state;
  bad_backend.vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                 M3_ERR_UNSUPPORTED);

  bad_backend.vtable = &g_test_text_vtable_no_destroy;
  M3_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                 M3_ERR_UNSUPPORTED);

  bad_backend.vtable = &g_test_text_vtable_no_create;
  M3_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                 M3_ERR_UNSUPPORTED);

  bad_style = style;
  bad_style.mode = 99u;
  M3_TEST_EXPECT(m3_tab_row_init(&row, &backend, &bad_style, items, 2, 0),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_tab_row_init(&row, &backend, &style, bad_items, 1, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_init(&row, &backend, &style, items, 2, 5),
                 M3_ERR_RANGE);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_tab_row_init(&row3, &backend, &style, items, 2, 0),
                 M3_ERR_IO);
  backend_state.fail_create = 0;

  M3_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ANIM_INIT));
  M3_TEST_EXPECT(m3_tab_row_init(&row2, &backend, &style, items, 2, 0),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  M3_TEST_EXPECT(m3_tab_row_set_items(NULL, items, 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_set_style(NULL, &style), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_set_style(&row, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_set_selected(NULL, 0), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_get_selected(NULL, &selected),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_get_selected(&row, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_set_scroll(NULL, 0.0f), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_get_scroll(NULL, &scroll), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_get_scroll(&row, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_set_on_select(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_step(NULL, 0.0f, &changed),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_tab_row_step(&row, -1.0f, &changed), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_tab_row_step(&row, 0.0f, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_tab_row_set_selected(&row, 1));
  M3_TEST_EXPECT(m3_tab_row_set_selected(&row, 5), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_get_selected(&row, &selected));
  M3_TEST_ASSERT(selected == 1);

  M3_TEST_EXPECT(m3_tab_row_set_items(&row, bad_items, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_tab_row_set_items(&row, items, 2));
  row.selected_index = 1;
  row.pressed_index = 1;
  M3_TEST_OK(m3_tab_row_set_items(&row, NULL, 0));
  M3_TEST_ASSERT(row.selected_index == M3_TAB_INVALID_INDEX);
  M3_TEST_ASSERT(row.pressed_index == M3_TAB_INVALID_INDEX);
  M3_TEST_OK(m3_tab_row_set_items(&row, items, 2));

  bad_style = style;
  bad_style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_tab_row_set_style(&row, &bad_style),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_tab_row_set_scroll(&row, -1.0f), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_set_scroll(&row, 20.0f));
  M3_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  M3_TEST_ASSERT(scroll == 0.0f);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_tab_row_set_style(&row, &style), M3_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  M3_TEST_EXPECT(m3_tab_row_set_style(&row, &style), M3_ERR_IO);
  backend_state.fail_destroy = 0;
  M3_TEST_OK(m3_tab_row_set_style(&row, &style));

  M3_TEST_OK(m3_tab_row_set_on_select(&row, NULL, NULL));

  M3_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));

  M3_TEST_OK(m3_tab_row_init(&row2, &backend, &style, items, 2, 0));
  row2.text_backend.vtable = &g_test_text_vtable_no_destroy;
  M3_TEST_EXPECT(row2.widget.vtable->destroy(row2.widget.ctx),
                 M3_ERR_UNSUPPORTED);

  return M3_OK;
}

static int test_tab_row_widget_fixed(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[3];
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  M3Semantics semantics;
  TabSelectState select_state;
  M3Bool changed;
  M3Scalar saved_spacing;
  M3Scalar saved_scroll;
  M3Rect saved_bounds;
  const M3TabItem *saved_items;
  m3_usize saved_count;

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                       &g_test_text_vtable));

  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.a = 1.0f;

  items[0].utf8_label = "Home";
  items[0].utf8_len = 4;
  items[1].utf8_label = "Library";
  items[1].utf8_len = 7;
  items[2].utf8_label = "Settings";
  items[2].utf8_len = 8;

  M3_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 3, 0));

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      row.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      row.widget.vtable->measure(row.widget.ctx, width_spec, height_spec, NULL),
      M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_RANGE);

  saved_spacing = row.style.spacing;
  row.style.spacing = -1.0f;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 1;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  row.items = saved_items;
  row.item_count = saved_count;

  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                            height_spec, &size),
                 M3_ERR_UNSUPPORTED);
  row.text_backend.vtable = &g_test_text_vtable;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec, height_spec,
                                        &size));
  M3_TEST_ASSERT(size.width > 0.0f);
  M3_TEST_ASSERT(size.height > 0.0f);

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 320.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 100.0f;
  M3_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec, height_spec,
                                        &size));
  M3_TEST_ASSERT(size.width == 320.0f);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 200.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  M3_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec, height_spec,
                                        &size));
  M3_TEST_ASSERT(size.width <= 200.0f);
  M3_TEST_ASSERT(size.height == 40.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = 0.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);

  saved_spacing = row.style.spacing;
  row.style.spacing = 1000.0f;
  bounds.width = 10.0f;
  bounds.height = 40.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_scroll = row.scroll_offset;
  row.scroll_offset = -1.0f;
  bounds.width = 100.0f;
  bounds.height = 40.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  row.scroll_offset = saved_scroll;

  bounds.width = 320.0f;
  bounds.height = 60.0f;
  M3_TEST_EXPECT(row.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  M3_TEST_EXPECT(row.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(row.widget.vtable->get_semantics(row.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  row.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  row.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(row.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  saved_bounds = row.bounds;
  row.bounds.width = -1.0f;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  row.bounds = saved_bounds;

  saved_spacing = row.style.spacing;
  row.style.spacing = -1.0f;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 1;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  row.items = saved_items;
  row.item_count = saved_count;

  saved_scroll = row.scroll_offset;
  row.scroll_offset = -1.0f;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  row.scroll_offset = saved_scroll;

  M3_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend_state.draw_rect_calls > 0);
  M3_TEST_ASSERT(backend_state.draw_text_calls > 0);

  backend_state.fail_draw_rect = 1;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_measure = 1;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw_text = 1;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_draw_text = 0;

  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  row.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 0;
  M3_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));
  M3_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.items = saved_items;
  row.item_count = saved_count;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  row.text_backend.vtable = &g_test_text_vtable;

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0));
  M3_TEST_EXPECT(row.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  row.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  row.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = (m3_usize) ~(m3_usize)0;
  M3_TEST_OK(m3_tab_row_set_on_select(&row, tab_on_select, &select_state));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 500, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                 M3_ERR_STATE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(select_state.calls == 1);

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 500, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 120, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 20;
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_DOWN, 0, 0));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(m3_tab_row_set_selected(&row, M3_TAB_INVALID_INDEX));
  row.indicator_pos_anim.mode = M3_ANIM_MODE_NONE;
  row.indicator_width_anim.mode = M3_ANIM_MODE_NONE;
  M3_TEST_OK(m3_tab_row_step(&row, 0.0f, &changed));
  M3_TEST_ASSERT(changed == M3_FALSE);

  row.style.indicator_anim_duration = 0.5f;
  M3_TEST_OK(m3_tab_row_set_selected(&row, 2));
  M3_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
  M3_TEST_ASSERT(changed == M3_TRUE);

  M3_TEST_EXPECT(row.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return M3_OK;
}

static int test_tab_row_widget_scrollable(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[3];
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  M3Scalar content_width;
  M3Scalar content_height;
  M3Scalar max_scroll;
  M3Scalar scroll;
  M3Bool near;

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                       &g_test_text_vtable));

  M3_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.mode = M3_TAB_MODE_SCROLLABLE;

  items[0].utf8_label = "Short";
  items[0].utf8_len = 5;
  items[1].utf8_label = "VeryLongLabel";
  items[1].utf8_len = 13;
  items[2].utf8_label = "Mid";
  items[2].utf8_len = 3;

  M3_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 3, 0));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 120.0f;
  bounds.height = 50.0f;
  M3_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  bounds.width = 1000.0f;
  bounds.height = 50.0f;
  M3_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));
  M3_TEST_OK(m3_tab_row_set_scroll(&row, 25.0f));
  M3_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  M3_TEST_ASSERT(scroll == 0.0f);
  bounds.width = 120.0f;
  bounds.height = 50.0f;
  M3_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  M3_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                             &content_width, &content_height));
  max_scroll =
      content_width - (bounds.width - style.padding.left - style.padding.right);
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }

  M3_TEST_EXPECT(m3_tab_row_set_scroll(&row, -1.0f), M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_set_scroll(&row, max_scroll + 50.0f));
  M3_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  M3_TEST_OK(tabs_near(scroll, max_scroll, 0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 10;
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 0;
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 10;
  M3_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_DOWN, 0, 0));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  style.mode = M3_TAB_MODE_FIXED;
  M3_TEST_OK(m3_tab_row_set_style(&row, &style));
  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());
  style.mode = M3_TAB_MODE_SCROLLABLE;
  M3_TEST_OK(m3_tab_row_set_style(&row, &style));

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_INDICATOR_START));
  M3_TEST_EXPECT(m3_tab_row_set_selected(&row, 1), M3_ERR_IO);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  M3_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  M3_TEST_OK(m3_tab_row_test_clear_fail_points());

  M3_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return M3_OK;
}

static int test_segmented_style_init(void) {
  M3SegmentedStyle style;
  M3Bool near;
  m3_u32 i;

  M3_TEST_EXPECT(m3_segmented_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_segmented_style_init(&style));
  M3_TEST_OK(tabs_near(style.min_width, M3_SEGMENTED_DEFAULT_MIN_WIDTH, 0.001f,
                       &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.min_height, M3_SEGMENTED_DEFAULT_MIN_HEIGHT,
                       0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.padding_x, M3_SEGMENTED_DEFAULT_PADDING_X, 0.001f,
                       &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.padding_y, M3_SEGMENTED_DEFAULT_PADDING_Y, 0.001f,
                       &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.outline_width, M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH,
                       0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);
  M3_TEST_OK(tabs_near(style.corner_radius, M3_SEGMENTED_DEFAULT_CORNER_RADIUS,
                       0.001f, &near));
  M3_TEST_ASSERT(near == M3_TRUE);

  M3_TEST_OK(
      m3_segmented_test_set_fail_point(M3_SEGMENTED_TEST_FAIL_STYLE_INIT));
  M3_TEST_EXPECT(m3_segmented_style_init(&style), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  for (i = 1u; i <= 9u; ++i) {
    M3_TEST_OK(m3_segmented_test_set_color_fail_after(i));
    M3_TEST_EXPECT(m3_segmented_style_init(&style), M3_ERR_IO);
    M3_TEST_OK(m3_segmented_test_clear_fail_points());
  }

  return M3_OK;
}

static int test_segmented_validation_helpers(void) {
  M3SegmentedStyle style;
  M3SegmentedStyle base_style;
  M3TextStyle text_style;
  M3LayoutEdges edges;
  M3Color color;
  M3MeasureSpec spec;
  M3Rect rect;
  M3SegmentedItem item;
  M3Bool states[2];
  M3SegmentedButtons buttons;
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3SegmentedItem items[2];
  M3Scalar width;
  M3Scalar height;

  M3_TEST_EXPECT(m3_segmented_test_validate_color(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.1f;
  M3_TEST_EXPECT(m3_segmented_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  M3_TEST_EXPECT(m3_segmented_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.1f;
  M3_TEST_EXPECT(m3_segmented_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_segmented_test_validate_color(&color));

  M3_TEST_EXPECT(m3_segmented_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_test_color_set(&color, -1.0f, 0.0f, 0.0f, 0.0f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f));

  M3_TEST_EXPECT(m3_segmented_test_color_with_alpha(NULL, 0.5f, &color),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, -1.0f, &color),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_color_with_alpha(&color, 0.5f, &color));

  M3_TEST_EXPECT(m3_segmented_test_validate_edges(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 0.0f;
  edges.bottom = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.bottom = 0.0f;
  M3_TEST_OK(m3_segmented_test_validate_edges(&edges));

  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.utf8_family = "Sans";
  text_style.size_px = 0;
  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  text_style.italic = M3_FALSE;
  text_style.color.a = 2.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, M3_TRUE),
                 M3_ERR_RANGE);
  text_style.color.a = 1.0f;
  M3_TEST_OK(m3_segmented_test_validate_text_style(&text_style, M3_TRUE));

  M3_TEST_EXPECT(m3_segmented_test_validate_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  base_style = style;
  style.spacing = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.min_width = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.min_height = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.padding_x = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.padding_y = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.outline_width = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.corner_radius = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.padding.left = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  style = base_style;
  style.selected_text_color.a = 2.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style = base_style;
  M3_TEST_OK(m3_segmented_test_validate_style(&style, M3_TRUE));

  M3_TEST_EXPECT(m3_segmented_test_validate_items(NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_segmented_test_validate_items(NULL, 0));
  item.utf8_label = NULL;
  item.utf8_len = 1;
  M3_TEST_EXPECT(m3_segmented_test_validate_items(&item, 1),
                 M3_ERR_INVALID_ARGUMENT);
  item.utf8_len = 0;
  M3_TEST_OK(m3_segmented_test_validate_items(&item, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  M3_TEST_OK(m3_segmented_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_segmented_test_validate_rect(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  M3_TEST_EXPECT(m3_segmented_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.height = 1.0f;
  M3_TEST_OK(m3_segmented_test_validate_rect(&rect));

  M3_TEST_EXPECT(m3_segmented_test_validate_backend(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  backend.ctx = NULL;
  backend.vtable = NULL;
  M3_TEST_EXPECT(m3_segmented_test_validate_backend(&backend),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_segmented_test_validate_mode(99u), M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_validate_mode(M3_SEGMENTED_MODE_SINGLE));

  M3_TEST_EXPECT(m3_segmented_test_validate_selected_states(NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  states[0] = (M3Bool)2;
  M3_TEST_EXPECT(m3_segmented_test_validate_selected_states(states, 1),
                 M3_ERR_INVALID_ARGUMENT);
  states[0] = M3_FALSE;
  M3_TEST_OK(m3_segmented_test_validate_selected_states(states, 1));

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(m3_segmented_test_validate_backend(&backend));
  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  states[0] = M3_TRUE;
  states[1] = M3_FALSE;
  M3_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                       M3_SEGMENTED_MODE_MULTI,
                                       M3_SEGMENTED_INVALID_INDEX, states));
  M3_TEST_EXPECT(m3_segmented_test_measure_content(NULL, &width, &height),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_segmented_test_measure_content(&buttons, &width, &height));
  M3_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  return M3_OK;
}

static int test_segmented_init_and_setters(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3TextBackend bad_backend;
  M3SegmentedStyle style;
  M3SegmentedStyle bad_style;
  M3SegmentedButtons buttons;
  M3SegmentedButtons buttons2;
  M3SegmentedButtons buttons3;
  M3SegmentedItem items[2];
  M3SegmentedItem bad_items[1];
  M3Bool states[2];
  m3_usize selected;

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));

  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 2;

  states[0] = M3_FALSE;
  states[1] = M3_TRUE;

  M3_TEST_EXPECT(m3_segmented_buttons_init(NULL, &backend, &style, items, 2,
                                           M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, NULL, &style, items, 2,
                                           M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, NULL, items, 2,
                                           M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = NULL;
  bad_backend.vtable = NULL;
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &bad_backend, &style,
                                           items, 2, M3_SEGMENTED_MODE_SINGLE,
                                           0, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = &backend_state;
  bad_backend.vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &bad_backend, &style,
                                           items, 2, M3_SEGMENTED_MODE_SINGLE,
                                           0, NULL),
                 M3_ERR_UNSUPPORTED);

  bad_style = style;
  bad_style.spacing = -1.0f;
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &bad_style,
                                           items, 2, M3_SEGMENTED_MODE_SINGLE,
                                           0, NULL),
                 M3_ERR_RANGE);

  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style,
                                           bad_items, 1,
                                           M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                           99u, 0, NULL),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                           M3_SEGMENTED_MODE_SINGLE, 5, NULL),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                           M3_SEGMENTED_MODE_MULTI,
                                           M3_SEGMENTED_INVALID_INDEX, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_segmented_buttons_init(&buttons3, &backend, &style, items,
                                           2, M3_SEGMENTED_MODE_SINGLE, 0,
                                           NULL),
                 M3_ERR_IO);
  backend_state.fail_create = 0;

  M3_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                       M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  M3_TEST_EXPECT(m3_segmented_buttons_set_items(NULL, items, 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_set_style(NULL, &style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_index(NULL, 0),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_get_selected_index(NULL, &selected),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_get_selected_index(&buttons, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_state(NULL, 0, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_get_selected_state(NULL, 0, &states[0]),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_get_selected_state(&buttons, 0, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_segmented_buttons_set_on_select(NULL, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_segmented_buttons_get_selected_index(&buttons, &selected));
  M3_TEST_ASSERT(selected == 0);
  M3_TEST_OK(m3_segmented_buttons_set_selected_index(&buttons, 1));
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_index(&buttons, 5),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_state(&buttons, 0, M3_TRUE),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&buttons, 0, &states[0]),
      M3_ERR_STATE);

  bad_style = style;
  bad_style.text_style.utf8_family = NULL;
  M3_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &bad_style),
                 M3_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &style), M3_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  M3_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &style), M3_ERR_IO);
  backend_state.fail_destroy = 0;
  M3_TEST_OK(m3_segmented_buttons_set_style(&buttons, &style));

  M3_TEST_EXPECT(m3_segmented_buttons_set_items(&buttons, bad_items, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_segmented_buttons_set_items(&buttons, items, 2));
  buttons.selected_index = 1;
  buttons.pressed_index = 1;
  M3_TEST_OK(m3_segmented_buttons_set_items(&buttons, NULL, 0));
  M3_TEST_ASSERT(buttons.selected_index == M3_SEGMENTED_INVALID_INDEX);
  M3_TEST_ASSERT(buttons.pressed_index == M3_SEGMENTED_INVALID_INDEX);
  M3_TEST_OK(m3_segmented_buttons_set_items(&buttons, items, 2));

  M3_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  M3_TEST_OK(m3_segmented_buttons_init(&buttons2, &backend, &style, items, 2,
                                       M3_SEGMENTED_MODE_MULTI,
                                       M3_SEGMENTED_INVALID_INDEX, states));
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_index(&buttons2, 0),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_segmented_buttons_get_selected_index(&buttons2, &selected),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_segmented_buttons_set_selected_state(&buttons2, 5, M3_TRUE),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_segmented_buttons_set_selected_state(&buttons2, 0, (M3Bool)3),
      M3_ERR_INVALID_ARGUMENT);
  states[0] = (M3Bool)2;
  M3_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&buttons2, 0, &states[0]),
      M3_ERR_INVALID_ARGUMENT);
  states[0] = M3_TRUE;
  M3_TEST_OK(m3_segmented_buttons_get_selected_state(&buttons2, 0, &states[0]));

  buttons2.selected_states = NULL;
  M3_TEST_EXPECT(m3_segmented_buttons_set_items(&buttons2, items, 2),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(buttons2.widget.vtable->destroy(buttons2.widget.ctx));

  M3_TEST_OK(m3_segmented_buttons_init(&buttons3, &backend, &style, items, 2,
                                       M3_SEGMENTED_MODE_SINGLE, 0, NULL));
  buttons3.text_backend.vtable = &g_test_text_vtable_no_destroy;
  M3_TEST_EXPECT(buttons3.widget.vtable->destroy(buttons3.widget.ctx),
                 M3_ERR_UNSUPPORTED);

  return M3_OK;
}

static int test_segmented_widget_flow(void) {
  TestTabsBackend backend_state;
  M3TextBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedButtons multi_buttons;
  M3SegmentedItem items[3];
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  SegmentedSelectState select_state;
  M3Bool states[3];
  M3Semantics semantics;
  M3Scalar saved_spacing;
  M3Rect saved_bounds;
  const M3SegmentedItem *saved_items;
  m3_usize saved_count;

  M3_TEST_OK(test_backend_init(&backend_state));
  M3_TEST_OK(setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  M3_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                       &g_test_text_vtable));

  M3_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  items[2].utf8_label = "Three";
  items[2].utf8_len = 5;

  M3_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 3,
                                       M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      buttons.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_RANGE);

  saved_spacing = buttons.style.spacing;
  buttons.style.spacing = -1.0f;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_RANGE);
  buttons.style.spacing = saved_spacing;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 1;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                height_spec, &size),
                 M3_ERR_UNSUPPORTED);
  buttons.text_backend.vtable = &g_test_text_vtable;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                            height_spec, &size));
  M3_TEST_ASSERT(size.width > 0.0f);
  M3_TEST_ASSERT(size.height > 0.0f);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 200.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  M3_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                            height_spec, &size));
  M3_TEST_ASSERT(size.width <= 200.0f);
  M3_TEST_ASSERT(size.height == 40.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->layout(buttons.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->layout(buttons.widget.ctx, bounds),
                 M3_ERR_RANGE);
  bounds.width = 300.0f;
  bounds.height = 50.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(buttons.widget.vtable->layout(buttons.widget.ctx, bounds));

  M3_TEST_EXPECT(buttons.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(buttons.widget.vtable->get_semantics(buttons.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  buttons.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  buttons.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(buttons.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  saved_bounds = buttons.bounds;
  buttons.bounds.width = -1.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  buttons.bounds = saved_bounds;

  saved_spacing = buttons.style.spacing;
  buttons.style.spacing = -1.0f;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  buttons.style.spacing = saved_spacing;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 1;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  M3_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend_state.draw_rect_calls > 0);
  M3_TEST_ASSERT(backend_state.draw_text_calls > 0);

  backend_state.fail_draw_rect = 1;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_measure = 1;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw_text = 1;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend_state.fail_draw_text = 0;

  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 0;
  M3_TEST_OK(buttons.widget.vtable->layout(buttons.widget.ctx, bounds));
  M3_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  buttons.text_backend.vtable = &g_test_text_vtable;

  buttons.style.outline_width = 0.0f;
  M3_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.style.outline_width = M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH;

  buttons.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0));
  M3_TEST_EXPECT(buttons.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, NULL, &handled),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(buttons.widget.vtable->event(buttons.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  buttons.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  buttons.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.last_selected = M3_FALSE;
  select_state.fail_index = (m3_usize) ~(m3_usize)0;
  M3_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                &select_state));

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_DOWN, 0, 0));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 1000, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      M3_ERR_STATE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  select_state.fail_index = 1;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 120, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 120, 10));
  M3_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      M3_ERR_IO);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 1000, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE));
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE));
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE));
  M3_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  M3_TEST_OK(m3_segmented_test_clear_fail_points());

  M3_TEST_EXPECT(buttons.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  states[0] = M3_FALSE;
  states[1] = M3_TRUE;
  states[2] = M3_FALSE;
  M3_TEST_OK(m3_segmented_buttons_init(&multi_buttons, &backend, &style, items,
                                       3, M3_SEGMENTED_MODE_MULTI,
                                       M3_SEGMENTED_INVALID_INDEX, states));
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 300.0f;
  bounds.height = 50.0f;
  M3_TEST_OK(
      multi_buttons.widget.vtable->layout(multi_buttons.widget.ctx, bounds));
  select_state.fail_index = 2;
  M3_TEST_OK(m3_segmented_buttons_set_on_select(
      &multi_buttons, segmented_on_select, &select_state));

  states[0] = (M3Bool)2;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10));
  M3_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                    &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  multi_buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;
  states[0] = M3_FALSE;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 220, 10));
  M3_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 220, 10));
  M3_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                    &event, &handled),
                 M3_ERR_IO);

  multi_buttons.selected_states = NULL;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                    &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);

  multi_buttons.selected_states = states;
  multi_buttons.mode = 99u;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                    &event, &handled),
                 M3_ERR_RANGE);

  multi_buttons.mode = M3_SEGMENTED_MODE_MULTI;
  multi_buttons.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10));
  M3_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(multi_buttons.widget.vtable->destroy(multi_buttons.widget.ctx));
  return M3_OK;
}

int main(void) {
  int step;

  step = 1;
  if (test_tab_row_style_init() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_validation_helpers() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_internal_helpers() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_init_and_setters() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_widget_fixed() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_widget_scrollable() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_style_init() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_validation_helpers() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_internal_helpers() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_init_and_setters() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_widget_flow() != M3_OK) {
    fprintf(stderr, "m3_phase5_tabs step %d\n", step);
    return 1;
  }

  return 0;
}
