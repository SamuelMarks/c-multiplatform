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
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_CONTENT_WIDTH_NEGATIVE 11u
#define M3_TAB_ROW_TEST_FAIL_STYLE_INIT_ERROR 12u
#define M3_TAB_ROW_TEST_FAIL_ANIM_INIT_ERROR 13u
#define M3_TAB_ROW_TEST_FAIL_CLAMP_SCROLL 14u
#define M3_TAB_ROW_TEST_FAIL_INDICATOR_START_WIDTH 15u
#define M3_TAB_ROW_TEST_FAIL_ANIM_INIT_WIDTH 16u
#define M3_TAB_ROW_TEST_FAIL_ANIM_INIT_WIDTH_ERROR 17u
#define M3_TAB_ROW_TEST_FAIL_LAYOUT_MODE_INVALID 18u

#define M3_SEGMENTED_TEST_FAIL_STYLE_INIT 1u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 2u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE 3u
#define M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE 4u
#define M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE 5u
#define M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE 6u
#define M3_SEGMENTED_TEST_FAIL_STYLE_INIT_ERROR 7u
#define M3_SEGMENTED_TEST_FAIL_EVENT_CLEAR_STATES 8u
#define M3_SEGMENTED_TEST_FAIL_OUTLINE_WIDTH_NEGATIVE 9u
#define M3_SEGMENTED_TEST_FAIL_EVENT_MODE_INVALID 10u

int CMP_CALL m3_tab_row_test_set_fail_point(cmp_u32 point);
int CMP_CALL m3_tab_row_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_tab_row_test_clear_fail_points(void);
int CMP_CALL m3_tab_row_test_validate_color(const CMPColor *color);
int CMP_CALL m3_tab_row_test_color_set(CMPColor *color, CMPScalar r,
                                       CMPScalar g, CMPScalar b, CMPScalar a);
int CMP_CALL m3_tab_row_test_color_with_alpha(const CMPColor *base,
                                              CMPScalar alpha,
                                              CMPColor *out_color);
int CMP_CALL m3_tab_row_test_validate_edges(const CMPLayoutEdges *edges);
int CMP_CALL m3_tab_row_test_validate_text_style(const CMPTextStyle *style,
                                                 CMPBool require_family);
int CMP_CALL m3_tab_row_test_validate_style(const M3TabRowStyle *style,
                                            CMPBool require_family);
int CMP_CALL m3_tab_row_test_validate_items(const M3TabItem *items,
                                            cmp_usize count);
int CMP_CALL m3_tab_row_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_tab_row_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_tab_row_test_validate_backend(const CMPTextBackend *backend);
int CMP_CALL m3_tab_row_test_measure_content(const M3TabRow *row, cmp_u32 mode,
                                             CMPScalar *out_width,
                                             CMPScalar *out_height);
int CMP_CALL m3_tab_row_test_color_should_fail_null(void);
int CMP_CALL m3_tab_row_test_fail_point_match_null(void);
int CMP_CALL m3_tab_row_test_force_color_error(CMPBool enable);
int CMP_CALL m3_tab_row_test_set_fail_point_error_after(cmp_u32 call_count);
int CMP_CALL m3_tab_row_test_set_value_fail_after(cmp_u32 call_count);
int CMP_CALL m3_tab_row_test_set_start_fail_after(cmp_u32 call_count);
int CMP_CALL m3_tab_row_test_measure_max_text(const M3TabRow *row,
                                              CMPScalar *out_width,
                                              CMPScalar *out_height,
                                              CMPScalar *out_baseline);
int CMP_CALL m3_tab_row_test_item_width(const M3TabRow *row,
                                        const M3TabItem *item,
                                        CMPScalar *out_width);
int CMP_CALL m3_tab_row_test_compute_layout(const M3TabRow *row,
                                            CMPScalar *out_content_width,
                                            CMPScalar *out_tab_width,
                                            CMPScalar *out_tab_height,
                                            cmp_u32 *out_mode);
int CMP_CALL m3_tab_row_test_compute_layout_null_out(const M3TabRow *row);
int CMP_CALL m3_tab_row_test_clamp_scroll(M3TabRow *row, cmp_u32 mode,
                                          CMPScalar content_width,
                                          CMPScalar available_width);
int CMP_CALL m3_tab_row_test_clamp_scroll_null_layout(M3TabRow *row);
int CMP_CALL m3_tab_row_test_indicator_target(
    const M3TabRow *row, cmp_u32 mode, CMPScalar tab_width, CMPScalar spacing,
    CMPScalar content_width, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_height, CMPScalar *out_pos, CMPScalar *out_width);
int CMP_CALL m3_tab_row_test_indicator_target_null_layout(const M3TabRow *row,
                                                          CMPScalar *out_pos,
                                                          CMPScalar *out_width);
int CMP_CALL m3_tab_row_test_sync_indicator(
    M3TabRow *row, cmp_u32 mode, CMPScalar tab_width, CMPScalar spacing,
    CMPScalar content_width, CMPScalar start_x, CMPScalar start_y,
    CMPScalar tab_height, CMPBool animate);
int CMP_CALL m3_tab_row_test_sync_indicator_null_layout(M3TabRow *row,
                                                        CMPBool animate);
int CMP_CALL m3_tab_row_test_item_rect(const M3TabRow *row, cmp_u32 mode,
                                       CMPScalar start_x, CMPScalar start_y,
                                       CMPScalar tab_width,
                                       CMPScalar tab_height, CMPScalar spacing,
                                       CMPScalar content_width,
                                       CMPScalar available_width,
                                       cmp_usize index, CMPRect *out_rect);
int CMP_CALL m3_tab_row_test_item_rect_null_layout(const M3TabRow *row,
                                                   cmp_usize index,
                                                   CMPRect *out_rect);
int CMP_CALL m3_tab_row_test_hit_test(const M3TabRow *row, cmp_u32 mode,
                                      CMPScalar start_x, CMPScalar start_y,
                                      CMPScalar tab_width, CMPScalar tab_height,
                                      CMPScalar spacing,
                                      CMPScalar content_width,
                                      CMPScalar available_width, cmp_i32 x,
                                      cmp_i32 y, cmp_usize *out_index);
int CMP_CALL m3_tab_row_test_hit_test_null_layout(const M3TabRow *row,
                                                  cmp_i32 x, cmp_i32 y,
                                                  cmp_usize *out_index);

int CMP_CALL m3_segmented_test_set_fail_point(cmp_u32 point);
int CMP_CALL m3_segmented_test_set_color_fail_after(cmp_u32 call_count);
int CMP_CALL m3_segmented_test_clear_fail_points(void);
int CMP_CALL m3_segmented_test_validate_color(const CMPColor *color);
int CMP_CALL m3_segmented_test_color_set(CMPColor *color, CMPScalar r,
                                         CMPScalar g, CMPScalar b, CMPScalar a);
int CMP_CALL m3_segmented_test_color_with_alpha(const CMPColor *base,
                                                CMPScalar alpha,
                                                CMPColor *out_color);
int CMP_CALL m3_segmented_test_validate_edges(const CMPLayoutEdges *edges);
int CMP_CALL m3_segmented_test_validate_text_style(const CMPTextStyle *style,
                                                   CMPBool require_family);
int CMP_CALL m3_segmented_test_validate_style(const M3SegmentedStyle *style,
                                              CMPBool require_family);
int CMP_CALL m3_segmented_test_validate_items(const M3SegmentedItem *items,
                                              cmp_usize count);
int CMP_CALL m3_segmented_test_validate_measure_spec(CMPMeasureSpec spec);
int CMP_CALL m3_segmented_test_validate_rect(const CMPRect *rect);
int CMP_CALL m3_segmented_test_validate_backend(const CMPTextBackend *backend);
int CMP_CALL m3_segmented_test_validate_mode(cmp_u32 mode);
int CMP_CALL m3_segmented_test_validate_selected_states(const CMPBool *states,
                                                        cmp_usize count);
int CMP_CALL
m3_segmented_test_measure_content(const M3SegmentedButtons *buttons,
                                  CMPScalar *out_width, CMPScalar *out_height);
int CMP_CALL m3_segmented_test_color_should_fail_null(void);
int CMP_CALL m3_segmented_test_fail_point_match_null(void);
int CMP_CALL m3_segmented_test_force_color_error(CMPBool enable);
int CMP_CALL m3_segmented_test_set_fail_point_error_after(cmp_u32 call_count);
int CMP_CALL m3_segmented_test_measure_max_text(
    const M3SegmentedButtons *buttons, CMPScalar *out_width,
    CMPScalar *out_height, CMPScalar *out_baseline);
int CMP_CALL m3_segmented_test_compute_layout(const M3SegmentedButtons *buttons,
                                              CMPScalar *out_content_width,
                                              CMPScalar *out_segment_width,
                                              CMPScalar *out_segment_height,
                                              CMPScalar *out_spacing);
int CMP_CALL
m3_segmented_test_compute_layout_null_out(const M3SegmentedButtons *buttons);
int CMP_CALL m3_segmented_test_hit_test(
    const M3SegmentedButtons *buttons, CMPScalar start_x, CMPScalar start_y,
    CMPScalar segment_width, CMPScalar segment_height, CMPScalar spacing,
    CMPScalar content_width, cmp_i32 x, cmp_i32 y, cmp_usize *out_index);
int CMP_CALL m3_segmented_test_hit_test_null_layout(
    const M3SegmentedButtons *buttons, cmp_i32 x, cmp_i32 y,
    cmp_usize *out_index);
int CMP_CALL m3_segmented_test_is_selected(const M3SegmentedButtons *buttons,
                                           cmp_usize index,
                                           CMPBool *out_selected);

typedef struct TestTabsBackend {
  int create_calls;
  int destroy_calls;
  int measure_calls;
  int draw_text_calls;
  int draw_rect_calls;
  int fail_create;
  int fail_destroy;
  int fail_measure;
  int fail_measure_at;
  int fail_draw_text;
  int fail_draw_rect;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestTabsBackend;

typedef struct TabSelectState {
  int calls;
  cmp_usize last_index;
  cmp_usize fail_index;
} TabSelectState;

typedef struct SegmentedSelectState {
  int calls;
  cmp_usize last_index;
  CMPBool last_selected;
  cmp_usize fail_index;
} SegmentedSelectState;

static int test_backend_init(TestTabsBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_measure_at = 0;
  return CMP_OK;
}

static int setup_text_backend(TestTabsBackend *state, CMPTextBackend *backend,
                              const CMPTextVTable *vtable) {
  if (state == NULL || backend == NULL || vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  backend->ctx = state;
  backend->vtable = vtable;
  return CMP_OK;
}

static int setup_gfx(TestTabsBackend *state, CMPGfx *gfx, CMPPaintContext *ctx,
                     const CMPGfxVTable *vtable,
                     const CMPTextVTable *text_vtable) {
  if (state == NULL || gfx == NULL || ctx == NULL || vtable == NULL ||
      text_vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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
  return CMP_OK;
}

static int tabs_near(CMPScalar a, CMPScalar b, CMPScalar tol,
                     CMPBool *out_near) {
  CMPScalar diff;

  if (out_near == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  *out_near = (diff <= tol) ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
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

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestTabsBackend *backend;

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

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

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

  backend = (TestTabsBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }
  if (backend->fail_measure_at > 0 &&
      backend->measure_calls == backend->fail_measure_at) {
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
  TestTabsBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestTabsBackend *)text;
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
  TestTabsBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestTabsBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int tab_on_select(void *ctx, struct M3TabRow *row, cmp_usize index) {
  TabSelectState *state;

  if (ctx == NULL || row == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TabSelectState *)ctx;
  state->calls += 1;
  state->last_index = index;
  if (index == state->fail_index) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int segmented_on_select(void *ctx, struct M3SegmentedButtons *buttons,
                               cmp_usize index, CMPBool selected) {
  SegmentedSelectState *state;

  if (ctx == NULL || buttons == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (SegmentedSelectState *)ctx;
  state->calls += 1;
  state->last_index = index;
  state->last_selected = selected;
  if (index == state->fail_index) {
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

static const CMPTextVTable g_test_text_vtable_no_measure = {
    test_text_create_font, test_text_destroy_font, NULL, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_destroy = {
    test_text_create_font, NULL, test_text_measure_text, test_text_draw_text};

static const CMPTextVTable g_test_text_vtable_no_create = {
    NULL, test_text_destroy_font, test_text_measure_text, test_text_draw_text};

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

static const CMPGfxVTable g_test_gfx_vtable_no_rect = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

static int test_tab_row_style_init(void) {
  M3TabRowStyle style;
  CMPBool near;
  cmp_u32 i;

  CMP_TEST_EXPECT(m3_tab_row_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  CMP_TEST_ASSERT(style.mode == M3_TAB_MODE_FIXED);
  CMP_TEST_OK(
      tabs_near(style.min_width, M3_TAB_DEFAULT_MIN_WIDTH, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(
      tabs_near(style.min_height, M3_TAB_DEFAULT_MIN_HEIGHT, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(
      tabs_near(style.padding_x, M3_TAB_DEFAULT_PADDING_X, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(
      tabs_near(style.padding_y, M3_TAB_DEFAULT_PADDING_Y, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.indicator_thickness,
                        M3_TAB_DEFAULT_INDICATOR_THICKNESS, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.indicator_anim_duration,
                        M3_TAB_DEFAULT_INDICATOR_DURATION, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_ASSERT(style.indicator_anim_easing ==
                  M3_TAB_DEFAULT_INDICATOR_EASING);
  CMP_TEST_ASSERT(style.text_style.size_px == 14);
  CMP_TEST_ASSERT(style.text_style.weight == 400);
  CMP_TEST_ASSERT(style.text_style.italic == CMP_FALSE);
  CMP_TEST_ASSERT(style.text_style.utf8_family == NULL);
  CMP_TEST_OK(tabs_near(style.background_color.a, 0.0f, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_STYLE_INIT));
  CMP_TEST_EXPECT(m3_tab_row_style_init(&style), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  for (i = 1u; i <= 5u; ++i) {
    CMP_TEST_OK(m3_tab_row_test_set_color_fail_after(i));
    CMP_TEST_EXPECT(m3_tab_row_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  }

  return CMP_OK;
}

static int test_tab_row_validation_helpers(void) {
  M3TabRowStyle style;
  M3TabRowStyle base_style;
  CMPTextStyle text_style;
  CMPColor color;
  CMPLayoutEdges edges;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3TabItem item;
  M3TabRow row;
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  M3TabItem items[2];
  CMPScalar width;
  CMPScalar height;

  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 1.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = -0.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = -0.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = -0.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_tab_row_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, -1.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f));

  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(NULL, 0.5f, &color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, -1.0f, &color),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_test_validate_text_style(&text_style, CMP_FALSE));
  text_style.utf8_family = "Sans";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 901;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 3;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.r = 0.0f;
  text_style.color.g = 2.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.g = 0.0f;
  text_style.color.b = -0.5f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.b = 0.0f;
  text_style.color.a = 2.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.a = 1.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  base_style = style;
  style.mode = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.spacing = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.min_height = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding_y = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.indicator_thickness = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.indicator_corner = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.indicator_anim_duration = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.indicator_anim_easing = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  style = base_style;
  style.selected_text_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.disabled_indicator_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.mode = M3_TAB_MODE_FIXED;
  CMP_TEST_OK(m3_tab_row_test_validate_style(&style, CMP_TRUE));
  style = base_style;
  style.indicator_anim_easing = CMP_ANIM_EASE_OUT;
  CMP_TEST_OK(m3_tab_row_test_validate_style(&style, CMP_TRUE));
  style = base_style;
  CMP_TEST_OK(m3_tab_row_test_validate_style(&style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_items(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_test_validate_items(NULL, 0));
  item.utf8_label = NULL;
  item.utf8_len = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_items(&item, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  item.utf8_len = 0;
  CMP_TEST_OK(m3_tab_row_test_validate_items(&item, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_tab_row_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.ctx = NULL;
  backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_backend(&backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(m3_tab_row_test_validate_backend(&backend));
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(NULL, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_measure_content(&row, 99u, &width, &height),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width,
                                              &height));
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                              &width, &height));
  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));

  return CMP_OK;
}

static int test_tab_row_internal_helpers(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[2];
  CMPColor color;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar content_width;
  CMPScalar tab_width;
  CMPScalar tab_height;
  cmp_u32 layout_mode;
  CMPRect rect;
  cmp_usize index;

  CMP_TEST_EXPECT(m3_tab_row_test_color_should_fail_null(),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_fail_point_match_null(),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_force_color_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_IO);

  color.r = 0.0f;
  color.g = 2.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color),
                  CMP_ERR_RANGE);
  color.g = 0.0f;
  CMP_TEST_OK(m3_tab_row_test_force_color_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 0.5f, &color),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_text_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(NULL, &width, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, NULL, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  row.items = NULL;
  row.item_count = 1;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  row.items = items;
  row.item_count = 2;
  CMP_TEST_OK(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline));

  CMP_TEST_EXPECT(m3_tab_row_test_item_width(NULL, &items[0], &width),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_item_width(&row, NULL, &width),
                  CMP_ERR_INVALID_ARGUMENT);
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], &width),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;
  row.style.padding_x = -10.0f;
  row.style.min_width = -100.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], &width),
                  CMP_ERR_RANGE);
  row.style.padding_x = style.padding_x;
  row.style.min_width = style.min_width;

  CMP_TEST_EXPECT(m3_tab_row_test_measure_content(&row, 99u, &width, &height),
                  CMP_ERR_RANGE);
  row.item_count = 0;
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width,
                                              &height));
  row.item_count = 2;
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                                  &width, &height),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;
  row.style.padding.left = -100.0f;
  row.item_count = 0;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_RANGE);
  row.style.padding.left = style.padding.left;
  row.item_count = 2;

  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(NULL, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, NULL, &tab_width,
                                                 &tab_height, &layout_mode),
                  CMP_ERR_INVALID_ARGUMENT);
  row.style.mode = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.style.mode = M3_TAB_MODE_FIXED;
  row.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.bounds.width = 100.0f;
  row.bounds.height = 50.0f;

  row.style.padding.left = 60.0f;
  row.style.padding.right = 60.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.style.padding.left = style.padding.left;
  row.style.padding.right = style.padding.right;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(3u));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  CMP_TEST_EXPECT(
      m3_tab_row_test_clamp_scroll(NULL, M3_TAB_MODE_SCROLLABLE, 10.0f, 5.0f),
      CMP_ERR_INVALID_ARGUMENT);
  row.scroll_offset = -1.0f;
  CMP_TEST_EXPECT(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_SCROLLABLE, 10.0f, 5.0f),
      CMP_ERR_RANGE);
  row.scroll_offset = 0.0f;
  CMP_TEST_OK(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_FIXED, 10.0f, 5.0f));

  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      NULL, M3_TAB_MODE_FIXED, 10.0f, 0.0f, 10.0f, 0.0f, 0.0f,
                      10.0f, &width, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, 99u, 10.0f, 0.0f,
                                                   10.0f, 0.0f, 0.0f, 10.0f,
                                                   &width, &height),
                  CMP_ERR_RANGE);
  row.selected_index = 1;
  row.item_count = 2;
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      &row, M3_TAB_MODE_SCROLLABLE, 10.0f, 0.0f, 20.0f, 0.0f,
                      0.0f, 10.0f, &width, &height),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      &row, M3_TAB_MODE_FIXED, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                      10.0f, &width, &height),
                  CMP_ERR_RANGE);
  row.selected_index = 0;
  row.item_count = 1;
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      &row, M3_TAB_MODE_SCROLLABLE, 10.0f, 0.0f, 20.0f, 0.0f,
                      0.0f, 10.0f, &width, &height),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;
  row.selected_index = 1;
  row.item_count = 2;

  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 (CMPBool)2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, 99u, 10.0f, 0.0f, 10.0f,
                                                 0.0f, 0.0f, 10.0f, CMP_FALSE),
                  CMP_ERR_RANGE);

  row.style.indicator_anim_duration = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_RANGE);
  row.style.indicator_anim_duration = 0.2f;
  row.style.indicator_anim_easing = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  row.style.indicator_anim_easing = M3_TAB_DEFAULT_INDICATOR_EASING;

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_INDICATOR_START));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_tab_row_test_set_value_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_value_fail_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 10.0f, 10.0f,
                                            0u, &rect),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 10.0f, 10.0f,
                                            0u, &rect),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(NULL, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 10.0f, 10.0f,
                                            0u, &rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 10.0f, 10.0f,
                                            5u, &rect),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                            0.0f, 10.0f, 10.0f, 0u, &rect),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_SCROLLABLE, 0.0f,
                                        0.0f, 0.0f, 10.0f, 5.0f, 200.0f, 200.0f,
                                        1u, &rect));

  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(NULL, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0,
                                           0, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                           0.0f, 10.0f, 10.0f, 0, 0, &index),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           0.0f, 10.0f, 0.0f, 0.0f, 10.0f, 0, 0,
                                           &index),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       10.0f, 10.0f, 0.0f, 20.0f, 10.0f, 1, 1,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_test_item_width(&row, &items[0], &width));
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f,
                                       0.0f, 10.0f, 5.0f, 200.0f, 200.0f,
                                       (cmp_i32)(width - 1.0f), 5, &index));
  CMP_TEST_ASSERT(index == 0u);
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f,
                                       0.0f, 10.0f, 5.0f, 200.0f, 200.0f,
                                       (cmp_i32)(width + 2.0f), 5, &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  content_width = width * (CMPScalar)row.item_count +
                  row.style.spacing * (CMPScalar)(row.item_count - 1u);
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(
      &row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f, 0.0f, 10.0f, row.style.spacing,
      content_width, 200.0f, (cmp_i32)(width + row.style.spacing + 1.0f), 5,
      &index));
  CMP_TEST_ASSERT(index == 1u);
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f,
                                           0.0f, 0.0f, 10.0f, 5.0f, 200.0f,
                                           200.0f, 1, 5, &index),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0,
                                           0, &index),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  {
    CMPBool changed;

    CMP_TEST_OK(cmp_anim_controller_start_timing(
        &row.indicator_pos_anim, 0.0f, 1.0f, 1.0f, CMP_ANIM_EASE_LINEAR));
    CMP_TEST_OK(cmp_anim_controller_start_timing(
        &row.indicator_width_anim, 0.0f, 2.0f, 1.0f, CMP_ANIM_EASE_LINEAR));
    row.indicator_pos = 10.0f;
    row.indicator_width = 10.0f;
    CMP_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
    CMP_TEST_ASSERT(changed == CMP_TRUE);
  }

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_segmented_internal_helpers(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedItem items[2];
  CMPColor color;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar content_width;
  CMPScalar segment_width;
  CMPScalar segment_height;
  CMPScalar spacing;
  cmp_usize index;
  cmp_usize saved_count;
  CMPBool selected;

  CMP_TEST_EXPECT(m3_segmented_test_color_should_fail_null(),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_fail_point_match_null(),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_force_color_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_IO);

  color.r = 0.0f;
  color.g = 2.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 0.5f, &color),
                  CMP_ERR_RANGE);
  color.g = 0.0f;
  CMP_TEST_OK(m3_segmented_test_force_color_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 0.5f, &color),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.outline_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_text_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  CMP_TEST_EXPECT(
      m3_segmented_test_measure_max_text(NULL, &width, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, NULL, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  buttons.items = NULL;
  buttons.item_count = 1;
  CMP_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  buttons.items = items;
  buttons.item_count = 2;
  CMP_TEST_OK(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline));

  CMP_TEST_EXPECT(m3_segmented_test_measure_content(NULL, &width, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.item_count = 0;
  CMP_TEST_OK(m3_segmented_test_measure_content(&buttons, &width, &height));
  buttons.item_count = 2;
  buttons.style.padding.left = -100.0f;
  buttons.item_count = 0;
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, &height),
                  CMP_ERR_RANGE);
  buttons.style.padding.left = style.padding.left;
  buttons.item_count = 2;

  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(NULL, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, NULL,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_RANGE);
  buttons.bounds.width = 100.0f;
  buttons.bounds.height = 40.0f;
  buttons.style.padding.left = 60.0f;
  buttons.style.padding.right = 60.0f;
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_RANGE);
  buttons.style.padding.left = style.padding.left;
  buttons.style.padding.right = style.padding.right;
  {
    CMPScalar height_backup;

    height_backup = buttons.bounds.height;
    buttons.bounds.height =
        buttons.style.padding.top + buttons.style.padding.bottom;
    CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                     &segment_width,
                                                     &segment_height, &spacing),
                    CMP_ERR_RANGE);
    buttons.bounds.height = height_backup;
  }
  {
    CMPScalar height_backup;

    height_backup = buttons.bounds.height;
    buttons.bounds.height =
        buttons.style.padding.top + buttons.style.padding.bottom + 1.0f;
    CMP_TEST_OK(m3_segmented_test_compute_layout(
        &buttons, &content_width, &segment_width, &segment_height, &spacing));
    buttons.bounds.height = height_backup;
  }

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(2u));
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(3u));
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  index = 0;
  CMP_TEST_EXPECT(m3_segmented_test_hit_test(NULL, 0.0f, 0.0f, 10.0f, 10.0f,
                                             0.0f, 10.0f, 0, 0, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  saved_count = buttons.item_count;
  buttons.item_count = 0;
  index = 1u;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         0.0f, 10.0f, 0, 0, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);
  buttons.item_count = saved_count;
  CMP_TEST_EXPECT(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 0.0f, 10.0f,
                                             0.0f, 10.0f, 0, 0, &index),
                  CMP_ERR_RANGE);
  index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         4.0f, 24.0f, 11, 5, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);
  index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         0.0f, 20.0f, 20, 5, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                             0.0f, 10.0f, 0, 0, &index),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  CMP_TEST_EXPECT(m3_segmented_test_is_selected(NULL, 0u, &selected),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.mode = M3_SEGMENTED_MODE_MULTI;
  CMP_TEST_EXPECT(m3_segmented_test_is_selected(&buttons, 0u, &selected),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;
  buttons.selected_index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_is_selected(&buttons, 0u, &selected));
  CMP_TEST_ASSERT(selected == CMP_FALSE);
  CMP_TEST_EXPECT(m3_segmented_test_is_selected(&buttons, 5u, &selected),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));
  return CMP_OK;
}

static int test_tab_row_init_and_setters(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPTextBackend bad_backend;
  M3TabRowStyle style;
  M3TabRowStyle bad_style;
  M3TabRow row;
  M3TabRow row2;
  M3TabRow row3;
  M3TabItem items[2];
  M3TabItem bad_items[1];
  CMPBool changed;
  cmp_usize selected;
  CMPScalar scroll;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "Alpha";
  items[0].utf8_len = 5;
  items[1].utf8_label = "Beta";
  items[1].utf8_len = 4;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 3;

  CMP_TEST_EXPECT(m3_tab_row_init(NULL, &backend, &style, items, 2, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_init(&row, NULL, &style, items, 2, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &backend, NULL, items, 2, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = NULL;
  bad_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = &backend_state;
  bad_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                  CMP_ERR_UNSUPPORTED);

  bad_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                  CMP_ERR_UNSUPPORTED);

  bad_backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &bad_backend, &style, items, 2, 0),
                  CMP_ERR_UNSUPPORTED);

  bad_style = style;
  bad_style.mode = 99u;
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &backend, &bad_style, items, 2, 0),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_tab_row_init(&row, &backend, &style, bad_items, 1, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_init(&row, &backend, &style, items, 2, 5),
                  CMP_ERR_RANGE);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_tab_row_init(&row3, &backend, &style, items, 2, 0),
                  CMP_ERR_IO);
  backend_state.fail_create = 0;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ANIM_INIT));
  CMP_TEST_EXPECT(m3_tab_row_init(&row2, &backend, &style, items, 2, 0),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_init(&row2, &backend, &style, items, 2, 0),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  CMP_TEST_EXPECT(m3_tab_row_set_items(NULL, items, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_set_style(&row, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_set_selected(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_get_selected(NULL, &selected),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_get_selected(&row, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_set_scroll(NULL, 0.0f), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_get_scroll(NULL, &scroll),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_get_scroll(&row, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_set_on_select(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_step(NULL, 0.0f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_step(&row, -1.0f, &changed), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_step(&row, 0.0f, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_tab_row_set_selected(&row, 1));
  row.bounds.width = 200.0f;
  row.bounds.height = 40.0f;
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_set_selected(&row, 0), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  row.bounds.width = 0.0f;
  row.bounds.height = 0.0f;
  CMP_TEST_OK(m3_tab_row_set_selected(&row, 1));
  CMP_TEST_EXPECT(m3_tab_row_set_selected(&row, 5), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_get_selected(&row, &selected));
  CMP_TEST_ASSERT(selected == 1);

  CMP_TEST_EXPECT(m3_tab_row_set_items(&row, bad_items, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_set_items(&row, items, 2));
  row.selected_index = 1;
  row.pressed_index = 1;
  CMP_TEST_OK(m3_tab_row_set_items(&row, NULL, 0));
  CMP_TEST_ASSERT(row.selected_index == M3_TAB_INVALID_INDEX);
  CMP_TEST_ASSERT(row.pressed_index == M3_TAB_INVALID_INDEX);
  CMP_TEST_OK(m3_tab_row_set_items(&row, items, 2));

  bad_style = style;
  bad_style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_tab_row_set_style(&row, &bad_style),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_tab_row_set_scroll(&row, -1.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_set_scroll(&row, 20.0f));
  CMP_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  CMP_TEST_ASSERT(scroll == 0.0f);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_tab_row_set_style(&row, &style), CMP_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_tab_row_set_style(&row, &style), CMP_ERR_IO);
  backend_state.fail_destroy = 0;
  CMP_TEST_OK(m3_tab_row_set_style(&row, &style));

  CMP_TEST_OK(m3_tab_row_set_on_select(&row, NULL, NULL));

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));

  CMP_TEST_OK(m3_tab_row_init(&row2, &backend, &style, items, 2, 0));
  row2.text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(row2.widget.vtable->destroy(row2.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  return CMP_OK;
}

static int test_tab_row_widget_fixed(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[3];
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  TabSelectState select_state;
  CMPBool changed;
  CMPScalar saved_spacing;
  CMPScalar saved_scroll;
  CMPRect saved_bounds;
  const M3TabItem *saved_items;
  const char *saved_label;
  cmp_usize saved_count;
  cmp_usize saved_len;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.background_color.a = 1.0f;

  items[0].utf8_label = "Home";
  items[0].utf8_len = 4;
  items[1].utf8_label = "Library";
  items[1].utf8_len = 7;
  items[2].utf8_label = "Settings";
  items[2].utf8_len = 8;

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 3, 0));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(
      row.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      row.widget.vtable->measure(row.widget.ctx, width_spec, height_spec, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_RANGE);

  saved_spacing = row.style.spacing;
  row.style.spacing = -1.0f;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 1;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  row.items = saved_items;
  row.item_count = saved_count;

  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                             height_spec, &size),
                  CMP_ERR_UNSUPPORTED);
  row.text_backend.vtable = &g_test_text_vtable;

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                         height_spec, &size));
  CMP_TEST_ASSERT(size.width > 0.0f);
  CMP_TEST_ASSERT(size.height > 0.0f);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 320.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 100.0f;
  CMP_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                         height_spec, &size));
  CMP_TEST_ASSERT(size.width == 320.0f);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 200.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  CMP_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                         height_spec, &size));
  CMP_TEST_ASSERT(size.width <= 200.0f);
  CMP_TEST_ASSERT(size.height == 40.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = 0.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  saved_spacing = row.style.spacing;
  row.style.spacing = 1000.0f;
  bounds.width = 10.0f;
  bounds.height = 40.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_scroll = row.scroll_offset;
  row.scroll_offset = -1.0f;
  bounds.width = 100.0f;
  bounds.height = 40.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  row.scroll_offset = saved_scroll;

  bounds.width = 320.0f;
  bounds.height = 60.0f;
  CMP_TEST_EXPECT(row.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  CMP_TEST_EXPECT(row.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(row.widget.vtable->get_semantics(row.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  row.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  row.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMP_TEST_EXPECT(row.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  saved_bounds = row.bounds;
  row.bounds.width = -1.0f;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  row.bounds = saved_bounds;

  saved_spacing = row.style.spacing;
  row.style.spacing = -1.0f;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  row.style.spacing = saved_spacing;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  row.items = saved_items;
  row.item_count = saved_count;

  saved_scroll = row.scroll_offset;
  row.scroll_offset = -1.0f;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  row.scroll_offset = saved_scroll;

  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend_state.draw_rect_calls > 0);
  CMP_TEST_ASSERT(backend_state.draw_text_calls > 0);

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw_text = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_text = 0;

  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  {
    const char *saved_label0;
    const char *saved_label1;
    const char *saved_label2;
    cmp_usize saved_len0;
    cmp_usize saved_len1;
    cmp_usize saved_len2;

    saved_label0 = items[0].utf8_label;
    saved_len0 = items[0].utf8_len;
    saved_label1 = items[1].utf8_label;
    saved_len1 = items[1].utf8_len;
    saved_label2 = items[2].utf8_label;
    saved_len2 = items[2].utf8_len;
    items[0].utf8_label = NULL;
    items[0].utf8_len = 0;
    items[1].utf8_label = NULL;
    items[1].utf8_len = 0;
    items[2].utf8_label = NULL;
    items[2].utf8_len = 0;
    row.text_backend.vtable = NULL;
    CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                    CMP_ERR_UNSUPPORTED);
    row.text_backend.vtable = &g_test_text_vtable;
    items[0].utf8_label = saved_label0;
    items[0].utf8_len = saved_len0;
    items[1].utf8_label = saved_label1;
    items[1].utf8_len = saved_len1;
    items[2].utf8_label = saved_label2;
    items[2].utf8_len = saved_len2;
  }

  row.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  saved_items = row.items;
  saved_count = row.item_count;
  row.items = NULL;
  row.item_count = 0;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.items = saved_items;
  row.item_count = saved_count;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  row.text_backend.vtable = &g_test_text_vtable;

  saved_label = items[1].utf8_label;
  saved_len = items[1].utf8_len;
  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  items[1].utf8_label = saved_label;
  items[1].utf8_len = saved_len;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0));
  CMP_TEST_EXPECT(row.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  row.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  row.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  saved_scroll = row.scroll_offset;
  row.scroll_offset = -1.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  row.scroll_offset = saved_scroll;

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_tab_row_set_on_select(&row, tab_on_select, &select_state));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 500, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_STATE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(select_state.calls == 1);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 500, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 120, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 20;
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_KEY_DOWN, 0, 0));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(m3_tab_row_set_selected(&row, M3_TAB_INVALID_INDEX));
  row.indicator_pos_anim.mode = CMP_ANIM_MODE_NONE;
  row.indicator_width_anim.mode = CMP_ANIM_MODE_NONE;
  CMP_TEST_OK(m3_tab_row_step(&row, 0.0f, &changed));
  CMP_TEST_ASSERT(changed == CMP_FALSE);

  row.style.indicator_anim_duration = 0.5f;
  CMP_TEST_OK(m3_tab_row_set_selected(&row, 2));
  CMP_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_TRUE);

  CMP_TEST_EXPECT(row.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_tab_row_widget_scrollable(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[3];
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  CMPScalar content_width;
  CMPScalar content_height;
  CMPScalar max_scroll;
  CMPScalar scroll;
  CMPBool near;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.mode = M3_TAB_MODE_SCROLLABLE;

  items[0].utf8_label = "Short";
  items[0].utf8_len = 5;
  items[1].utf8_label = "VeryLongLabel";
  items[1].utf8_len = 13;
  items[2].utf8_label = "Mid";
  items[2].utf8_len = 3;

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 3, 0));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 120.0f;
  bounds.height = 50.0f;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  bounds.width = 1000.0f;
  bounds.height = 50.0f;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));
  CMP_TEST_OK(m3_tab_row_set_scroll(&row, 25.0f));
  CMP_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  CMP_TEST_ASSERT(scroll == 0.0f);
  bounds.width = 120.0f;
  bounds.height = 50.0f;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                              &content_width, &content_height));
  max_scroll =
      content_width - (bounds.width - style.padding.left - style.padding.right);
  if (max_scroll < 0.0f) {
    max_scroll = 0.0f;
  }

  CMP_TEST_EXPECT(m3_tab_row_set_scroll(&row, -1.0f), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_set_scroll(&row, max_scroll + 50.0f));
  CMP_TEST_OK(m3_tab_row_get_scroll(&row, &scroll));
  CMP_TEST_OK(tabs_near(scroll, max_scroll, 0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 10;
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 0;
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_SCROLL, 0, 0));
  event.data.pointer.scroll_x = 10;
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_KEY_DOWN, 0, 0));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  style.mode = M3_TAB_MODE_FIXED;
  CMP_TEST_OK(m3_tab_row_set_style(&row, &style));
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  style.mode = M3_TAB_MODE_SCROLLABLE;
  CMP_TEST_OK(m3_tab_row_set_style(&row, &style));

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_INDICATOR_START));
  CMP_TEST_EXPECT(m3_tab_row_set_selected(&row, 1), CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_tab_row_branch_sweep(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabRow destroy_row;
  M3TabRow destroy_row2;
  M3TabRow destroy_row3;
  M3TabRow destroy_row4;
  M3TabRow destroy_row5;
  M3TabItem items[2];
  CMPColor color;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar content_width;
  CMPScalar tab_width;
  CMPScalar tab_height;
  CMPScalar hit_x;
  CMPScalar hit_y;
  CMPScalar start_x;
  CMPScalar start_y;
  CMPScalar available_width;
  CMPScalar available_height;
  cmp_u32 layout_mode;
  CMPRect rect;
  CMPRect bounds;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  cmp_usize index;
  TabSelectState select_state;
  CMPBool changed;
  M3TabRowStyle saved_style;
  CMPRect saved_bounds;
  cmp_usize saved_item_count;
  const M3TabItem *saved_items;
  cmp_usize saved_selected;
  cmp_u32 saved_flags;
  CMPTextBackend saved_text_backend;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  saved_style = row.style;
  saved_bounds = row.bounds;
  saved_item_count = row.item_count;
  saved_items = row.items;
  saved_selected = row.selected_index;
  saved_flags = row.widget.flags;
  saved_text_backend = row.text_backend;

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_STYLE_INIT_ERROR));
  CMP_TEST_EXPECT(m3_tab_row_style_init(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_STYLE_INIT));
  CMP_TEST_EXPECT(m3_tab_row_style_init(&style), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";

  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_color_with_alpha(&color, 1.5f, &color),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, &width, NULL, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_measure_max_text(&row, &width, &height, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  row.item_count = 0;
  CMP_TEST_OK(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline));
  row.item_count = saved_item_count;

  CMP_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(NULL, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, NULL, &height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  row.style.min_height = 200.0f;
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width,
                                              &height));
  row.style = saved_style;

  row.item_count = 0;
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                              &width, &height));
  row.item_count = saved_item_count;

  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                              &width, &height));

  backend_state.measure_calls = 0;
  backend_state.fail_measure_at = 3;
  CMP_TEST_EXPECT(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                                  &width, &height),
                  CMP_ERR_IO);
  backend_state.fail_measure_at = 0;

  row.style.spacing = -1000.0f;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_RANGE);
  row.style = saved_style;

  row.style.padding_y = -10.0f;
  row.style.min_height = -100.0f;
  row.item_count = 0;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_RANGE);
  row.style = saved_style;
  row.item_count = saved_item_count;

  row.style.padding.top = -100.0f;
  row.style.padding.bottom = 0.0f;
  row.style.padding.left = 0.0f;
  row.style.padding.right = 0.0f;
  row.item_count = 0;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_RANGE);
  row.style = saved_style;
  row.item_count = saved_item_count;

  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout_null_out(&row),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, NULL, &tab_width,
                                                 &tab_height, &layout_mode),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width, NULL,
                                                 &tab_height, &layout_mode),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(
                      &row, &content_width, &tab_width, NULL, &layout_mode),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  row.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.bounds = saved_bounds;

  row.bounds.width = 100.0f;
  row.bounds.height = 10.0f;
  row.style.padding.top = 10.0f;
  row.style.padding.bottom = 10.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.style = saved_style;
  row.bounds = saved_bounds;

  row.style.mode = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  row.style = saved_style;

  row.bounds.width = 200.0f;
  row.bounds.height = 40.0f;
  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_LAYOUT_MODE_INVALID));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  row.bounds = saved_bounds;

  row.style.min_height = 200.0f;
  row.bounds.width = 200.0f;
  row.bounds.height = 300.0f;
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));
  CMP_TEST_ASSERT(tab_height == row.style.min_height);
  row.style = saved_style;

  row.style.min_height = 50.0f;
  row.bounds.height = 20.0f;
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));
  CMP_TEST_ASSERT(tab_height == (row.bounds.height - row.style.padding.top -
                                 row.style.padding.bottom));
  row.style = saved_style;
  row.bounds = saved_bounds;

  row.style.mode = M3_TAB_MODE_SCROLLABLE;
  row.item_count = 0;
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));
  row.item_count = saved_item_count;
  row.bounds.width = 200.0f;
  row.bounds.height = 40.0f;

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.measure_calls = 0;
  backend_state.fail_measure_at = 3;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_IO);
  backend_state.fail_measure_at = 0;

  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_CONTENT_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_tab_row_test_clamp_scroll_null_layout(&row),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_CLAMP_SCROLL));
  CMP_TEST_EXPECT(m3_tab_row_set_scroll(&row, 1.0f), CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_EXPECT(
      m3_tab_row_test_indicator_target_null_layout(&row, &width, &height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, M3_TAB_MODE_FIXED,
                                                   10.0f, 0.0f, 10.0f, 0.0f,
                                                   0.0f, 10.0f, NULL, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, M3_TAB_MODE_FIXED,
                                                   10.0f, 0.0f, 10.0f, 0.0f,
                                                   0.0f, 10.0f, &width, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      &row, M3_TAB_MODE_FIXED, -1.0f, 0.0f, 10.0f, 0.0f, 0.0f,
                      10.0f, &width, &height),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(NULL, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator_null_layout(&row, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(2u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_START_WIDTH));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_EXPECT(m3_tab_row_test_item_rect_null_layout(&row, 0u, &rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 10.0f, 10.0f,
                                            0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_SCROLLABLE, 0.0f,
                                            0.0f, 0.0f, 10.0f, 5.0f, 200.0f,
                                            200.0f, 1u, &rect),
                  CMP_ERR_IO);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_SCROLLABLE, 0.0f,
                                            0.0f, 0.0f, 10.0f, 5.0f, 200.0f,
                                            200.0f, 0u, &rect),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, -1.0f, 0.0f, 10.0f, 10.0f,
                                            0u, &rect),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_tab_row_test_hit_test_null_layout(&row, 0, 0, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0,
                                           0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  row.item_count = 0;
  index = 123u;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0, 0,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  row.item_count = saved_item_count;

  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 0, -5,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);

  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       10.0f, 10.0f, 0.0f, 10.0f, 10.0f, 50, 5,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);

  row.item_count = 1;
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       1.0f, 10.0f, 0.0f, 200.0f, 200.0f, 150,
                                       5, &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  row.item_count = saved_item_count;

  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                       10.0f, 10.0f, 5.0f, 25.0f, 25.0f, 12, 5,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);

  row.scroll_offset = -5.0f;
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f,
                                       0.0f, 10.0f, 5.0f, 20.0f, 200.0f, 0, 5,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  row.scroll_offset = 0.0f;

  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f,
                                       0.0f, 10.0f, 5.0f, 20.0f, 200.0f, 50, 5,
                                       &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_SCROLLABLE, 0.0f,
                                           0.0f, 0.0f, 10.0f, 5.0f, 20.0f,
                                           200.0f, 1, 5, &index),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  row.style.spacing = 5.0f;
  CMP_TEST_OK(m3_tab_row_test_item_width(&row, &items[0], &width));
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(
      &row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f, 0.0f, 10.0f, row.style.spacing,
      width * 2.0f + row.style.spacing, 200.0f,
      (cmp_i32)(width + row.style.spacing * 0.5f), 5, &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);

  row.style = saved_style;

  row.style.mode = M3_TAB_MODE_SCROLLABLE;
  row.style.spacing = 0.0f;
  row.style.min_width = 200.0f;
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));
  row.style.min_width = 0.0f;
  index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_hit_test(
      &row, M3_TAB_MODE_SCROLLABLE, 0.0f, 0.0f, 0.0f, tab_height, 0.0f,
      content_width, 200.0f, (cmp_i32)(content_width - 1.0f), 1, &index));
  CMP_TEST_ASSERT(index == M3_TAB_INVALID_INDEX);
  row.style = saved_style;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10000.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 5.0f;
  CMP_TEST_OK(row.widget.vtable->measure(row.widget.ctx, width_spec,
                                         height_spec, &size));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_EXPECT(row.widget.vtable->layout(row.widget.ctx, bounds),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  row.widget.flags = 0u;
  CMP_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) == 0u);

  row.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  CMP_TEST_OK(row.widget.vtable->get_semantics(row.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
  row.widget.flags = saved_flags;

  row.bounds = bounds;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(4u));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(5u));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  row.style.background_color.a = 0.0f;
  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;
  row.style = saved_style;

  row.selected_index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.selected_index = saved_selected;
  row.style.indicator_thickness = 0.0f;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.style = saved_style;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  items[0].utf8_len = 0;
  items[1].utf8_len = 0;
  row.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  row.text_backend = saved_text_backend;
  items[0].utf8_len = 3;
  items[1].utf8_len = 3;

  backend_state.measure_calls = 0;
  backend_state.fail_measure_at = 3;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_measure_at = 0;

  bounds.width = 200.0f;
  bounds.height = 40.0f;
  row.bounds = bounds;

  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));
  start_x = row.bounds.x + row.style.padding.left;
  start_y = row.bounds.y + row.style.padding.top;
  available_width =
      row.bounds.width - row.style.padding.left - row.style.padding.right;
  available_height =
      row.bounds.height - row.style.padding.top - row.style.padding.bottom;
  if (available_height > tab_height) {
    start_y += (available_height - tab_height) * 0.5f;
  }
  hit_x = start_x + 1.0f;
  hit_y = start_y + 1.0f;

  CMP_TEST_OK(m3_tab_row_test_hit_test(&row, row.style.mode, start_x, start_y,
                                       tab_width, tab_height, row.style.spacing,
                                       content_width, available_width,
                                       (cmp_i32)hit_x, (cmp_i32)hit_y, &index));
  CMP_TEST_ASSERT(index != M3_TAB_INVALID_INDEX);

  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(4u));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_fail_point_error_after(0u));

  row.on_select = NULL;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_tab_row_set_on_select(&row, tab_on_select, &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(select_state.calls > 0);

  select_state.fail_index = index;
  CMP_TEST_OK(m3_tab_row_set_on_select(&row, tab_on_select, &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(3u));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(row.widget.vtable->event(row.widget.ctx, &event, &handled),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));

  row.pressed_index = 10u;
  CMP_TEST_OK(m3_tab_row_set_items(&row, items, 1u));
  CMP_TEST_ASSERT(row.pressed_index == M3_TAB_INVALID_INDEX);
  CMP_TEST_OK(m3_tab_row_set_items(&row, saved_items, saved_item_count));

  row.owns_font = CMP_FALSE;
  CMP_TEST_OK(m3_tab_row_set_style(&row, &saved_style));
  row.owns_font = CMP_TRUE;

  row.bounds.width = 0.0f;
  row.bounds.height = 20.0f;
  CMP_TEST_OK(m3_tab_row_set_selected(&row, 0));
  row.bounds.width = 20.0f;
  row.bounds.height = 0.0f;
  CMP_TEST_OK(m3_tab_row_set_selected(&row, 0));
  row.bounds = saved_bounds;

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ANIM_INIT_ERROR));
  CMP_TEST_EXPECT(m3_tab_row_init(&destroy_row, &backend, &style, items, 2, 0),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ANIM_INIT));
  CMP_TEST_EXPECT(m3_tab_row_init(&destroy_row, &backend, &style, items, 2, 0),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_ANIM_INIT_WIDTH_ERROR));
  CMP_TEST_EXPECT(m3_tab_row_init(&destroy_row, &backend, &style, items, 2, 0),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_ANIM_INIT_WIDTH));
  CMP_TEST_EXPECT(m3_tab_row_init(&destroy_row, &backend, &style, items, 2, 0),
                  CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_init(&destroy_row, &backend, &style, items, 2,
                              M3_TAB_INVALID_INDEX));
  CMP_TEST_OK(destroy_row.widget.vtable->destroy(destroy_row.widget.ctx));

  {
    CMPTextBackend bad_backend = backend;

    bad_backend.vtable = &g_test_text_vtable_no_measure;
    CMP_TEST_EXPECT(
        m3_tab_row_init(&destroy_row, &bad_backend, &style, items, 2, 0),
        CMP_ERR_UNSUPPORTED);
  }

  CMP_TEST_OK(m3_tab_row_init(&destroy_row, &backend, &style, items, 2, 0));
  destroy_row.owns_font = CMP_FALSE;
  CMP_TEST_OK(destroy_row.widget.vtable->destroy(destroy_row.widget.ctx));

  CMP_TEST_OK(m3_tab_row_init(&destroy_row2, &backend, &style, items, 2, 0));
  destroy_row2.font.id = 0u;
  destroy_row2.font.generation = 0u;
  CMP_TEST_OK(destroy_row2.widget.vtable->destroy(destroy_row2.widget.ctx));

  CMP_TEST_OK(m3_tab_row_init(&destroy_row4, &backend, &style, items, 2, 0));
  destroy_row4.font.id = 0u;
  destroy_row4.font.generation = 1u;
  CMP_TEST_OK(destroy_row4.widget.vtable->destroy(destroy_row4.widget.ctx));

  CMP_TEST_OK(m3_tab_row_init(&destroy_row5, &backend, &style, items, 2, 0));
  destroy_row5.font.id = 1u;
  destroy_row5.font.generation = 0u;
  CMP_TEST_OK(destroy_row5.widget.vtable->destroy(destroy_row5.widget.ctx));

  CMP_TEST_OK(m3_tab_row_init(&destroy_row3, &backend, &style, items, 2, 0));
  destroy_row3.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(destroy_row3.widget.vtable->destroy(destroy_row3.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  row.indicator_pos_anim.mode = (cmp_u32)999;
  row.indicator_pos_anim.running = CMP_TRUE;
  CMP_TEST_EXPECT(m3_tab_row_step(&row, 0.1f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_start_timing(
      &row.indicator_pos_anim, 1.0f, 1.0f, 1.0f, CMP_ANIM_EASE_LINEAR));
  row.indicator_pos_anim.mode = CMP_ANIM_MODE_TIMING;
  row.indicator_pos_anim.running = CMP_TRUE;
  row.indicator_pos = 1.0f;
  CMP_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_FALSE);

  row.indicator_width_anim.mode = (cmp_u32)999;
  row.indicator_width_anim.running = CMP_TRUE;
  CMP_TEST_EXPECT(m3_tab_row_step(&row, 0.1f, &changed),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_anim_controller_start_timing(
      &row.indicator_width_anim, 2.0f, 2.0f, 1.0f, CMP_ANIM_EASE_LINEAR));
  row.indicator_width_anim.mode = CMP_ANIM_MODE_TIMING;
  row.indicator_width_anim.running = CMP_TRUE;
  row.indicator_width = 2.0f;
  CMP_TEST_OK(m3_tab_row_step(&row, 0.1f, &changed));
  CMP_TEST_ASSERT(changed == CMP_FALSE);

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_tab_row_extra_coverage(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRowStyle bad_style;
  M3TabRow row;
  M3TabItem items[2];
  CMPRect bounds;
  CMPScalar width;
  CMPScalar height;
  CMPScalar content_width;
  CMPScalar tab_width;
  CMPScalar tab_height;
  cmp_u32 layout_mode;
  M3TabRowStyle saved_style;
  CMPTextBackend saved_text_backend;
  cmp_u32 saved_flags;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1;

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  saved_style = row.style;
  saved_text_backend = row.text_backend;
  saved_flags = row.widget.flags;

  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_style = saved_style;
  bad_style.mode = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.indicator_corner = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.indicator_anim_easing = 99u;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.selected_text_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);
  bad_style = saved_style;
  bad_style.indicator_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_style(&bad_style, CMP_FALSE),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(NULL, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_tab_row_test_measure_content(&row, 99u, &width, &height),
                  CMP_ERR_RANGE);
  row.items = NULL;
  row.item_count = 1;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_INVALID_ARGUMENT);
  row.items = items;
  row.item_count = 0;
  row.style.padding.left = -10.0f;
  row.style.padding.right = 0.0f;
  row.style.padding.top = 0.0f;
  row.style.padding.bottom = 0.0f;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_content(&row, M3_TAB_MODE_FIXED, &width, &height),
      CMP_ERR_RANGE);
  row.style = saved_style;
  row.item_count = 2;
  row.bounds.x = 0.0f;
  row.bounds.y = 0.0f;
  row.bounds.width = 100.0f;
  row.bounds.height = 30.0f;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_TAB_HEIGHT_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_LAYOUT_CONTENT_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(&row, &content_width,
                                                 &tab_width, &tab_height,
                                                 &layout_mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &layout_mode));

  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, 99u, 10.0f, 0.0f, 10.0f,
                                                 0.0f, 0.0f, 10.0f, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                             0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                             CMP_TRUE));
  CMP_TEST_OK(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                             0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                             CMP_FALSE));
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_FALSE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_start_fail_after(0u));
  CMP_TEST_OK(m3_tab_row_test_set_value_fail_after(1u));
  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 10.0f, 0.0f, 0.0f, 10.0f,
                                                 CMP_TRUE),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_tab_row_test_set_value_fail_after(0u));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 30.0f;
  row.bounds = bounds;
  row.indicator_width = 10.0f;
  row.indicator_pos = 0.0f;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  row.style = saved_style;
  row.style.mode = 99u;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  row.style = saved_style;

  row.bounds.width = -1.0f;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  row.bounds = bounds;

  row.items = NULL;
  row.item_count = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  row.items = items;
  row.item_count = 2;

  row.style.background_color.a = 0.0f;
  row.item_count = 0;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.item_count = 2;
  row.style.background_color.a = saved_style.background_color.a;

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  CMP_TEST_OK(m3_tab_row_test_set_fail_point(
      M3_TAB_ROW_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  row.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  row.text_backend = saved_text_backend;

  items[0].utf8_len = 0;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  items[0].utf8_len = 1;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_draw_text = 1;
  CMP_TEST_EXPECT(row.widget.vtable->paint(row.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_text = 0;

  row.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(row.widget.vtable->paint(row.widget.ctx, &paint_ctx));
  row.widget.flags = saved_flags;

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_tab_row_branch_edges(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPTextBackend bad_backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3TabRowStyle style;
  M3TabRow row;
  M3TabItem items[2];
  M3TabItem bad_items[1];
  CMPLayoutEdges edges;
  CMPMeasureSpec spec;
  CMPRect bounds;
  CMPRect rect;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar content_width;
  CMPScalar tab_width;
  CMPScalar tab_height;
  CMPScalar pos;
  CMPScalar indicator_width;
  cmp_u32 mode;
  cmp_usize index;
  CMPInputEvent event;
  CMPBool handled;
  TabSelectState select_state;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_tab_row_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1;

  CMP_TEST_OK(m3_tab_row_init(&row, &backend, &style, items, 2, 0));

  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_edges(&edges));

  CMP_TEST_OK(m3_tab_row_test_validate_items(NULL, 0));
  CMP_TEST_EXPECT(m3_tab_row_test_validate_items(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 1;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_items(bad_items, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_items[0].utf8_len = 0;
  CMP_TEST_OK(m3_tab_row_test_validate_items(bad_items, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_tab_row_test_validate_measure_spec(spec));

  bad_backend = backend;
  bad_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_tab_row_test_validate_backend(&bad_backend),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_tab_row_test_validate_backend(&backend));

  row.items = NULL;
  row.item_count = 1;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  row.items = items;
  row.item_count = 2;
  items[0].utf8_len = 0;
  items[1].utf8_len = 0;
  CMP_TEST_OK(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline));
  items[0].utf8_len = 1;
  items[1].utf8_len = 1;
  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(
      m3_tab_row_test_measure_max_text(&row, &width, &height, &baseline),
      CMP_ERR_IO);
  backend_state.fail_measure = 0;

  items[0].utf8_len = 0;
  CMP_TEST_OK(m3_tab_row_test_item_width(&row, &items[0], &width));
  row.style.padding_x = -1.0f;
  row.style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_item_width(&row, &items[0], &width),
                  CMP_ERR_RANGE);
  row.style.min_width = style.min_width;
  row.style.padding_x = style.padding_x;

  row.item_count = 0;
  CMP_TEST_OK(m3_tab_row_test_measure_content(&row, M3_TAB_MODE_SCROLLABLE,
                                              &width, &height));
  row.item_count = 2;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 10.0f;
  bounds.height = 20.0f;
  row.bounds = bounds;
  row.style.spacing = 20.0f;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(
                      &row, &content_width, &tab_width, &tab_height, &mode),
                  CMP_ERR_RANGE);
  row.style.spacing = style.spacing;

  row.style.padding.top = 5.0f;
  row.style.padding.bottom = 5.0f;
  bounds.width = 100.0f;
  bounds.height = 5.0f;
  row.bounds = bounds;
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(
                      &row, &content_width, &tab_width, &tab_height, &mode),
                  CMP_ERR_RANGE);
  row.style.padding = style.padding;
  bounds.height = 30.0f;
  row.bounds = bounds;

  CMP_TEST_OK(
      m3_tab_row_test_set_fail_point(M3_TAB_ROW_TEST_FAIL_LAYOUT_MODE_INVALID));
  CMP_TEST_EXPECT(m3_tab_row_test_compute_layout(
                      &row, &content_width, &tab_width, &tab_height, &mode),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_tab_row_test_clear_fail_points());

  row.style.mode = M3_TAB_MODE_SCROLLABLE;
  row.item_count = 0;
  CMP_TEST_OK(m3_tab_row_test_compute_layout(&row, &content_width, &tab_width,
                                             &tab_height, &mode));
  row.item_count = 2;
  row.style.mode = style.mode;

  row.scroll_offset = -1.0f;
  CMP_TEST_EXPECT(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_SCROLLABLE, 50.0f, 10.0f),
      CMP_ERR_RANGE);
  row.scroll_offset = 10.0f;
  CMP_TEST_OK(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_FIXED, 50.0f, 10.0f));
  CMP_TEST_ASSERT(row.scroll_offset == 0.0f);
  row.scroll_offset = 10.0f;
  CMP_TEST_OK(
      m3_tab_row_test_clamp_scroll(&row, M3_TAB_MODE_SCROLLABLE, 20.0f, 40.0f));
  CMP_TEST_ASSERT(row.scroll_offset == 0.0f);

  row.selected_index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_test_indicator_target(&row, M3_TAB_MODE_FIXED, 10.0f,
                                               0.0f, 20.0f, 0.0f, 0.0f, 10.0f,
                                               &pos, &indicator_width));
  CMP_TEST_ASSERT(pos == 0.0f);
  CMP_TEST_ASSERT(indicator_width == 0.0f);
  row.selected_index = 0;
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(&row, 99u, 10.0f, 0.0f,
                                                   20.0f, 0.0f, 0.0f, 10.0f,
                                                   &pos, &indicator_width),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_indicator_target(
                      &row, M3_TAB_MODE_FIXED, -1.0f, 0.0f, 20.0f, 0.0f, 0.0f,
                      10.0f, &pos, &indicator_width),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_tab_row_test_sync_indicator(&row, M3_TAB_MODE_FIXED, 10.0f,
                                                 0.0f, 20.0f, 0.0f, 0.0f, 10.0f,
                                                 (CMPBool)2),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            10.0f, 10.0f, 0.0f, 20.0f, 20.0f, 5,
                                            &rect),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                            0.0f, 20.0f, 20.0f, 0, &rect),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_item_rect(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                            -1.0f, 10.0f, 0.0f, 20.0f, 20.0f, 0,
                                            &rect),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, 99u, 0.0f, 0.0f, 10.0f, 10.0f,
                                           0.0f, 20.0f, 20.0f, 0, 0, &index),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_tab_row_test_hit_test(&row, M3_TAB_MODE_FIXED, 0.0f, 0.0f,
                                           0.0f, 10.0f, 0.0f, 20.0f, 20.0f, 1,
                                           1, &index),
                  CMP_ERR_RANGE);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 30.0f;
  CMP_TEST_OK(row.widget.vtable->layout(row.widget.ctx, bounds));

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = M3_TAB_INVALID_INDEX;
  CMP_TEST_OK(m3_tab_row_set_on_select(&row, tab_on_select, &select_state));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(row.pressed_index != M3_TAB_INVALID_INDEX);
  select_state.fail_index = row.pressed_index;
  if (row.selected_index == row.pressed_index) {
    row.selected_index = (row.selected_index == 0u) ? 1u : 0u;
  }
  index = row.selected_index;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 5));
  CMP_TEST_OK(row.widget.vtable->event(row.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(select_state.calls > 0);
  CMP_TEST_ASSERT(row.selected_index == index);

  CMP_TEST_OK(row.widget.vtable->destroy(row.widget.ctx));
  return CMP_OK;
}

static int test_segmented_style_init(void) {
  M3SegmentedStyle style;
  CMPBool near;
  cmp_u32 i;

  CMP_TEST_EXPECT(m3_segmented_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  CMP_TEST_OK(tabs_near(style.min_width, M3_SEGMENTED_DEFAULT_MIN_WIDTH, 0.001f,
                        &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.min_height, M3_SEGMENTED_DEFAULT_MIN_HEIGHT,
                        0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.padding_x, M3_SEGMENTED_DEFAULT_PADDING_X, 0.001f,
                        &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.padding_y, M3_SEGMENTED_DEFAULT_PADDING_Y, 0.001f,
                        &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.outline_width, M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH,
                        0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);
  CMP_TEST_OK(tabs_near(style.corner_radius, M3_SEGMENTED_DEFAULT_CORNER_RADIUS,
                        0.001f, &near));
  CMP_TEST_ASSERT(near == CMP_TRUE);

  CMP_TEST_OK(
      m3_segmented_test_set_fail_point(M3_SEGMENTED_TEST_FAIL_STYLE_INIT));
  CMP_TEST_EXPECT(m3_segmented_style_init(&style), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  for (i = 1u; i <= 9u; ++i) {
    CMP_TEST_OK(m3_segmented_test_set_color_fail_after(i));
    CMP_TEST_EXPECT(m3_segmented_style_init(&style), CMP_ERR_IO);
    CMP_TEST_OK(m3_segmented_test_clear_fail_points());
  }

  return CMP_OK;
}

static int test_segmented_validation_helpers(void) {
  M3SegmentedStyle style;
  M3SegmentedStyle base_style;
  CMPTextStyle text_style;
  CMPLayoutEdges edges;
  CMPColor color;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3SegmentedItem item;
  CMPBool states[2];
  M3SegmentedButtons buttons;
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  M3SegmentedItem items[2];
  CMPScalar width;
  CMPScalar height;

  CMP_TEST_EXPECT(m3_segmented_test_validate_color(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 1.1f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.1f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.1f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.1f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_segmented_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_segmented_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, -1.0f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.0f));

  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(NULL, 0.5f, &color),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, -1.0f, &color),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_color_with_alpha(&color, 0.5f, &color));

  CMP_TEST_EXPECT(m3_segmented_test_validate_edges(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.right = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.right = 0.0f;
  edges.top = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.top = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_segmented_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_test_validate_text_style(&text_style, CMP_FALSE));
  text_style.utf8_family = "Sans";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 901;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.r = 0.0f;
  text_style.color.g = 1.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.g = 0.0f;
  text_style.color.b = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.b = 0.0f;
  text_style.color.a = 2.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.a = -0.5f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  text_style.color.a = 1.0f;
  CMP_TEST_OK(m3_segmented_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_segmented_test_validate_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  base_style = style;
  style.spacing = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.min_height = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding_x = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding_y = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.outline_width = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  style = base_style;
  style.selected_text_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style = base_style;
  CMP_TEST_OK(m3_segmented_test_validate_style(&style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_segmented_test_validate_items(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_test_validate_items(NULL, 0));
  item.utf8_label = NULL;
  item.utf8_len = 1;
  CMP_TEST_EXPECT(m3_segmented_test_validate_items(&item, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  item.utf8_len = 0;
  CMP_TEST_OK(m3_segmented_test_validate_items(&item, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_segmented_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_segmented_test_validate_rect(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_segmented_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_segmented_test_validate_backend(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  backend.ctx = NULL;
  backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_segmented_test_validate_backend(&backend),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_segmented_test_validate_mode(99u), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_validate_mode(M3_SEGMENTED_MODE_SINGLE));

  CMP_TEST_EXPECT(m3_segmented_test_validate_selected_states(NULL, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  states[0] = (CMPBool)2;
  CMP_TEST_EXPECT(m3_segmented_test_validate_selected_states(states, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  states[0] = CMP_FALSE;
  CMP_TEST_OK(m3_segmented_test_validate_selected_states(states, 1));

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(m3_segmented_test_validate_backend(&backend));
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  states[0] = CMP_TRUE;
  states[1] = CMP_FALSE;
  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_MULTI,
                                        M3_SEGMENTED_INVALID_INDEX, states));
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(NULL, &width, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_test_measure_content(&buttons, &width, &height));
  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  return CMP_OK;
}

static int test_segmented_init_and_setters(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPTextBackend bad_backend;
  M3SegmentedStyle style;
  M3SegmentedStyle bad_style;
  M3SegmentedButtons buttons;
  M3SegmentedButtons buttons2;
  M3SegmentedButtons buttons3;
  M3SegmentedItem items[2];
  M3SegmentedItem bad_items[1];
  CMPBool states[2];
  cmp_usize selected;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 2;

  states[0] = CMP_FALSE;
  states[1] = CMP_TRUE;

  CMP_TEST_EXPECT(m3_segmented_buttons_init(NULL, &backend, &style, items, 2,
                                            M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, NULL, &style, items, 2,
                                            M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, NULL, items, 2,
                                            M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = NULL;
  bad_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &bad_backend, &style,
                                            items, 2, M3_SEGMENTED_MODE_SINGLE,
                                            0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_backend.ctx = &backend_state;
  bad_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &bad_backend, &style,
                                            items, 2, M3_SEGMENTED_MODE_SINGLE,
                                            0, NULL),
                  CMP_ERR_UNSUPPORTED);

  bad_style = style;
  bad_style.spacing = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &bad_style,
                                            items, 2, M3_SEGMENTED_MODE_SINGLE,
                                            0, NULL),
                  CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style,
                                            bad_items, 1,
                                            M3_SEGMENTED_MODE_SINGLE, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items,
                                            2, 99u, 0, NULL),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items,
                                            2, M3_SEGMENTED_MODE_SINGLE, 5,
                                            NULL),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons, &backend, &style, items,
                                            2, M3_SEGMENTED_MODE_MULTI,
                                            M3_SEGMENTED_INVALID_INDEX, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_segmented_buttons_init(&buttons3, &backend, &style, items,
                                            2, M3_SEGMENTED_MODE_SINGLE, 0,
                                            NULL),
                  CMP_ERR_IO);
  backend_state.fail_create = 0;

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  CMP_TEST_EXPECT(m3_segmented_buttons_set_items(NULL, items, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_set_style(NULL, &style),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_set_selected_index(NULL, 0),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_get_selected_index(NULL, &selected),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_get_selected_index(&buttons, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_set_selected_state(NULL, 0, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_get_selected_state(NULL, 0, &states[0]),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_get_selected_state(&buttons, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_buttons_set_on_select(NULL, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_segmented_buttons_get_selected_index(&buttons, &selected));
  CMP_TEST_ASSERT(selected == 0);
  CMP_TEST_OK(m3_segmented_buttons_set_selected_index(&buttons, 1));
  CMP_TEST_EXPECT(m3_segmented_buttons_set_selected_index(&buttons, 5),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_segmented_buttons_set_selected_state(&buttons, 0, CMP_TRUE),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&buttons, 0, &states[0]),
      CMP_ERR_STATE);

  bad_style = style;
  bad_style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &bad_style),
                  CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &style), CMP_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_segmented_buttons_set_style(&buttons, &style), CMP_ERR_IO);
  backend_state.fail_destroy = 0;
  CMP_TEST_OK(m3_segmented_buttons_set_style(&buttons, &style));

  CMP_TEST_EXPECT(m3_segmented_buttons_set_items(&buttons, bad_items, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_buttons_set_items(&buttons, items, 2));
  buttons.selected_index = 1;
  buttons.pressed_index = 1;
  CMP_TEST_OK(m3_segmented_buttons_set_items(&buttons, NULL, 0));
  CMP_TEST_ASSERT(buttons.selected_index == M3_SEGMENTED_INVALID_INDEX);
  CMP_TEST_ASSERT(buttons.pressed_index == M3_SEGMENTED_INVALID_INDEX);
  CMP_TEST_OK(m3_segmented_buttons_set_items(&buttons, items, 2));

  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons2, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_MULTI,
                                        M3_SEGMENTED_INVALID_INDEX, states));
  CMP_TEST_EXPECT(m3_segmented_buttons_set_selected_index(&buttons2, 0),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(m3_segmented_buttons_get_selected_index(&buttons2, &selected),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      m3_segmented_buttons_set_selected_state(&buttons2, 5, CMP_TRUE),
      CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      m3_segmented_buttons_set_selected_state(&buttons2, 0, (CMPBool)3),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_buttons_set_selected_state(&buttons2, 1, CMP_TRUE));
  CMP_TEST_ASSERT(states[1] == CMP_TRUE);
  buttons2.selected_states = NULL;
  CMP_TEST_EXPECT(
      m3_segmented_buttons_set_selected_state(&buttons2, 0, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  buttons2.selected_states = states;
  states[0] = (CMPBool)2;
  CMP_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&buttons2, 0, &states[0]),
      CMP_ERR_INVALID_ARGUMENT);
  states[0] = CMP_TRUE;
  CMP_TEST_OK(
      m3_segmented_buttons_get_selected_state(&buttons2, 0, &states[0]));

  buttons2.selected_states = NULL;
  CMP_TEST_EXPECT(m3_segmented_buttons_set_items(&buttons2, items, 2),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(buttons2.widget.vtable->destroy(buttons2.widget.ctx));

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons3, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));
  buttons3.text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(buttons3.widget.vtable->destroy(buttons3.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  return CMP_OK;
}

static int test_segmented_widget_flow(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedButtons multi_buttons;
  M3SegmentedItem items[3];
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  SegmentedSelectState select_state;
  CMPBool states[3];
  CMPSemantics semantics;
  CMPScalar saved_outline;
  CMPScalar saved_spacing;
  CMPRect saved_bounds;
  const M3SegmentedItem *saved_items;
  cmp_usize saved_count;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  items[2].utf8_label = "Three";
  items[2].utf8_len = 5;

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 3,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(
      buttons.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_RANGE);

  saved_spacing = buttons.style.spacing;
  buttons.style.spacing = -1.0f;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_RANGE);
  buttons.style.spacing = saved_spacing;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 1;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                                 height_spec, &size),
                  CMP_ERR_UNSUPPORTED);
  buttons.text_backend.vtable = &g_test_text_vtable;

  width_spec.mode = CMP_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = CMP_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  CMP_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                             height_spec, &size));
  CMP_TEST_ASSERT(size.width > 0.0f);
  CMP_TEST_ASSERT(size.height > 0.0f);

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 200.0f;
  height_spec.mode = CMP_MEASURE_EXACTLY;
  height_spec.size = 40.0f;
  CMP_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                             height_spec, &size));
  CMP_TEST_ASSERT(size.width <= 200.0f);
  CMP_TEST_ASSERT(size.height == 40.0f);

  width_spec.mode = CMP_MEASURE_EXACTLY;
  width_spec.size = 200.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 30.0f;
  CMP_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                             height_spec, &size));
  CMP_TEST_ASSERT(size.height <= 30.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->layout(buttons.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 10.0f;
  bounds.height = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->layout(buttons.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 300.0f;
  bounds.height = 50.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(buttons.widget.vtable->layout(buttons.widget.ctx, bounds));

  CMP_TEST_EXPECT(buttons.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.role == CMP_SEMANTIC_NONE);
  buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  buttons.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMP_TEST_EXPECT(buttons.widget.vtable->paint(NULL, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  saved_bounds = buttons.bounds;
  buttons.bounds.width = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.bounds = saved_bounds;

  saved_spacing = buttons.style.spacing;
  buttons.style.spacing = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style.spacing = saved_spacing;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  CMP_TEST_ASSERT(backend_state.draw_rect_calls > 0);
  CMP_TEST_ASSERT(backend_state.draw_text_calls > 0);

  saved_outline = buttons.style.outline_width;
  buttons.style.outline_width = 1000.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style.outline_width = saved_outline;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw_text = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_text = 0;

  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  saved_items = buttons.items;
  saved_count = buttons.item_count;
  buttons.items = NULL;
  buttons.item_count = 0;
  CMP_TEST_OK(buttons.widget.vtable->layout(buttons.widget.ctx, bounds));
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.items = saved_items;
  buttons.item_count = saved_count;

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  buttons.text_backend.vtable = &g_test_text_vtable;

  buttons.style.outline_width = 0.0f;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.style.outline_width = M3_SEGMENTED_DEFAULT_OUTLINE_WIDTH;

  buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0));
  CMP_TEST_EXPECT(buttons.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, NULL, &handled),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  buttons.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.last_selected = CMP_FALSE;
  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                 &select_state));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_KEY_DOWN, 0, 0));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 1000, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_STATE);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  select_state.fail_index = 1;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 120, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 120, 10));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 1000, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_LAYOUT_SEGMENT_HEIGHT_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_EXPECT(buttons.widget.vtable->destroy(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));

  states[0] = CMP_FALSE;
  states[1] = CMP_TRUE;
  states[2] = CMP_FALSE;
  CMP_TEST_OK(m3_segmented_buttons_init(&multi_buttons, &backend, &style, items,
                                        3, M3_SEGMENTED_MODE_MULTI,
                                        M3_SEGMENTED_INVALID_INDEX, states));
  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 300.0f;
  bounds.height = 50.0f;
  CMP_TEST_OK(
      multi_buttons.widget.vtable->layout(multi_buttons.widget.ctx, bounds));
  states[0] = (CMPBool)2;
  CMP_TEST_EXPECT(
      multi_buttons.widget.vtable->paint(multi_buttons.widget.ctx, &paint_ctx),
      CMP_ERR_INVALID_ARGUMENT);
  states[0] = CMP_FALSE;
  select_state.fail_index = 2;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(
      &multi_buttons, segmented_on_select, &select_state));

  states[0] = (CMPBool)2;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  multi_buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;
  states[0] = CMP_FALSE;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 220, 10));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 220, 10));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_IO);

  multi_buttons.selected_states = NULL;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);

  multi_buttons.selected_states = states;
  multi_buttons.mode = 99u;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_RANGE);

  multi_buttons.mode = M3_SEGMENTED_MODE_MULTI;
  multi_buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(multi_buttons.widget.vtable->destroy(multi_buttons.widget.ctx));
  return CMP_OK;
}

static int test_segmented_branch_sweep(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedButtons multi_buttons;
  M3SegmentedButtons destroy_buttons;
  M3SegmentedButtons destroy_buttons2;
  M3SegmentedButtons destroy_buttons3;
  M3SegmentedItem items[2];
  CMPBool states[2];
  CMPColor color;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPScalar content_width;
  CMPScalar segment_width;
  CMPScalar segment_height;
  CMPScalar spacing;
  CMPScalar start_x;
  CMPScalar start_y;
  CMPScalar hit_x;
  CMPScalar hit_y;
  CMPRect bounds;
  CMPMeasureSpec width_spec;
  CMPMeasureSpec height_spec;
  CMPSize size;
  CMPInputEvent event;
  CMPBool handled;
  CMPSemantics semantics;
  cmp_usize index;
  SegmentedSelectState select_state;
  CMPBool selected;
  CMPTextBackend saved_text_backend;
  cmp_u32 saved_flags;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  states[0] = CMP_FALSE;
  states[1] = CMP_TRUE;

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));
  CMP_TEST_OK(m3_segmented_buttons_init(&multi_buttons, &backend, &style, items,
                                        2, M3_SEGMENTED_MODE_MULTI, 0, states));

  saved_text_backend = buttons.text_backend;
  saved_flags = buttons.widget.flags;

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_STYLE_INIT_ERROR));
  CMP_TEST_EXPECT(m3_segmented_style_init(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());
  CMP_TEST_OK(m3_segmented_test_set_color_fail_after(9u));
  CMP_TEST_EXPECT(m3_segmented_style_init(&style), CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_color_fail_after(0u));
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  buttons.style = style;
  multi_buttons.style = style;

  color.r = 0.0f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, -0.1f, 0.0f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, -0.1f, 0.0f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, -0.1f, 0.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_set(&color, 0.0f, 0.0f, 0.0f, 1.5f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 0.5f, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_color_with_alpha(&color, 1.5f, &color),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.selected_background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_outline_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_selected_background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.disabled_selected_text_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_validate_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  buttons.style = style;
  multi_buttons.style = style;

  CMP_TEST_OK(m3_segmented_test_validate_selected_states(NULL, 0));

  CMP_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, &width, NULL, &baseline),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, NULL),
      CMP_ERR_INVALID_ARGUMENT);
  buttons.item_count = 0;
  CMP_TEST_OK(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline));
  buttons.item_count = 2;

  CMP_TEST_EXPECT(m3_segmented_test_measure_content(NULL, &width, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, NULL, &height),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  buttons.style.spacing = -1000.0f;
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, &height),
                  CMP_ERR_RANGE);
  buttons.style = style;

  buttons.style.padding_y = -10.0f;
  buttons.style.min_height = -100.0f;
  buttons.item_count = 0;
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, &height),
                  CMP_ERR_RANGE);
  buttons.style = style;
  buttons.item_count = 2;

  buttons.style.padding.top = -100.0f;
  buttons.style.padding.bottom = 0.0f;
  buttons.style.padding.left = 0.0f;
  buttons.style.padding.right = 0.0f;
  buttons.item_count = 0;
  CMP_TEST_EXPECT(m3_segmented_test_measure_content(&buttons, &width, &height),
                  CMP_ERR_RANGE);
  buttons.style = style;
  buttons.item_count = 2;

  CMP_TEST_EXPECT(m3_segmented_test_compute_layout_null_out(&buttons),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(
                      &buttons, &content_width, &segment_width, NULL, &spacing),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  buttons.style.spacing = -1.0f;
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_RANGE);
  buttons.style = style;

  buttons.bounds.width = 100.0f;
  buttons.bounds.height = 10.0f;
  buttons.style.padding.top = 10.0f;
  buttons.style.padding.bottom = 10.0f;
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_RANGE);
  buttons.style = style;

  buttons.style.min_height = 200.0f;
  buttons.bounds.width = 200.0f;
  buttons.bounds.height = 300.0f;
  CMP_TEST_OK(m3_segmented_test_compute_layout(
      &buttons, &content_width, &segment_width, &segment_height, &spacing));
  CMP_TEST_ASSERT(segment_height == buttons.style.min_height);
  buttons.style = style;

  buttons.style.min_height = 50.0f;
  buttons.bounds.height = 20.0f;
  CMP_TEST_OK(m3_segmented_test_compute_layout(
      &buttons, &content_width, &segment_width, &segment_height, &spacing));
  CMP_TEST_ASSERT(segment_height ==
                  (buttons.bounds.height - buttons.style.padding.top -
                   buttons.style.padding.bottom));
  buttons.style = style;

  buttons.style.spacing = 200.0f;
  buttons.bounds.width = 100.0f;
  CMP_TEST_EXPECT(m3_segmented_test_compute_layout(&buttons, &content_width,
                                                   &segment_width,
                                                   &segment_height, &spacing),
                  CMP_ERR_RANGE);
  buttons.style = style;

  CMP_TEST_EXPECT(
      m3_segmented_test_hit_test_null_layout(&buttons, 0, 0, &index),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                             0.0f, 10.0f, 0, 0, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         0.0f, 10.0f, 0, -5, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);

  index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         0.0f, 10.0f, 50, 5, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);

  index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                         0.0f, 20.0f, 20, 5, &index));
  CMP_TEST_ASSERT(index == M3_SEGMENTED_INVALID_INDEX);

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  CMP_TEST_EXPECT(m3_segmented_test_hit_test(&buttons, 0.0f, 0.0f, 10.0f, 10.0f,
                                             0.0f, 10.0f, 1, 5, &index),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  CMP_TEST_EXPECT(m3_segmented_test_is_selected(&buttons, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.mode = 99u;
  CMP_TEST_EXPECT(m3_segmented_test_is_selected(&buttons, 0u, &selected),
                  CMP_ERR_RANGE);
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;

  width_spec.mode = CMP_MEASURE_AT_MOST;
  width_spec.size = 10000.0f;
  height_spec.mode = CMP_MEASURE_AT_MOST;
  height_spec.size = 5.0f;
  CMP_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                             height_spec, &size));
  height_spec.size = 1000.0f;
  CMP_TEST_OK(buttons.widget.vtable->measure(buttons.widget.ctx, width_spec,
                                             height_spec, &size));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 40.0f;
  CMP_TEST_OK(buttons.widget.vtable->layout(buttons.widget.ctx, bounds));
  CMP_TEST_OK(
      multi_buttons.widget.vtable->layout(multi_buttons.widget.ctx, bounds));
  CMP_TEST_OK(m3_segmented_test_compute_layout(
      &buttons, &content_width, &segment_width, &segment_height, &spacing));
  start_x = bounds.x + buttons.style.padding.left;
  start_y = bounds.y + buttons.style.padding.top;
  hit_x = start_x + segment_width * 0.5f;
  hit_y = start_y + segment_height * 0.5f;

  items[0].utf8_len = 0;
  items[1].utf8_len = 0;
  buttons.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  buttons.text_backend = saved_text_backend;
  items[0].utf8_len = 3;
  items[1].utf8_len = 3;

  buttons.style.outline_width = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style = style;

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_OUTLINE_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(4u));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(5u));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  buttons.style.outline_width = 1000.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style = style;

  buttons.style.outline_width = 0.0f;
  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;
  buttons.style = style;

  items[1].utf8_label = NULL;
  items[1].utf8_len = 0;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  backend_state.measure_calls = 0;
  backend_state.fail_measure_at = 2;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_measure_at = 0;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  buttons.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
  CMP_TEST_OK(
      buttons.widget.vtable->get_semantics(buttons.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
  buttons.widget.flags = saved_flags;

  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(4u));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));

  multi_buttons.selected_states = NULL;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  multi_buttons.selected_states = states;

  buttons.mode = 99u;
  buttons.pressed_index = 0u;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;
  buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_EVENT_MODE_INVALID));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;
  buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;

  buttons.on_select = NULL;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                 &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(select_state.calls > 0);

  select_state.fail_index = 0u;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                 &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);

  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(
      &multi_buttons, segmented_on_select, &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_EVENT_CLEAR_STATES));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());
  multi_buttons.selected_states = states;
  multi_buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;

  select_state.calls = 0;
  select_state.fail_index = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(
      &multi_buttons, segmented_on_select, &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_ASSERT(select_state.calls > 0);

  select_state.fail_index = 0u;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(
      &multi_buttons, segmented_on_select, &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_OK(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                 &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, (cmp_i32)hit_x,
                                 (cmp_i32)hit_y));
  CMP_TEST_EXPECT(multi_buttons.widget.vtable->event(multi_buttons.widget.ctx,
                                                     &event, &handled),
                  CMP_ERR_IO);

  CMP_TEST_OK(m3_segmented_buttons_set_items(&multi_buttons, items, 2));
  multi_buttons.selected_states = NULL;
  CMP_TEST_EXPECT(m3_segmented_buttons_set_items(&multi_buttons, items, 2),
                  CMP_ERR_INVALID_ARGUMENT);
  multi_buttons.selected_states = states;
  multi_buttons.pressed_index = 10u;
  CMP_TEST_OK(m3_segmented_buttons_set_items(&multi_buttons, items, 1u));
  CMP_TEST_ASSERT(multi_buttons.pressed_index == M3_SEGMENTED_INVALID_INDEX);

  buttons.owns_font = CMP_FALSE;
  CMP_TEST_OK(m3_segmented_buttons_set_style(&buttons, &style));
  buttons.owns_font = CMP_TRUE;

  CMP_TEST_OK(m3_segmented_buttons_set_selected_index(
      &buttons, M3_SEGMENTED_INVALID_INDEX));

  CMP_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&multi_buttons, 10u, &selected),
      CMP_ERR_RANGE);
  multi_buttons.selected_states = NULL;
  CMP_TEST_EXPECT(
      m3_segmented_buttons_get_selected_state(&multi_buttons, 0u, &selected),
      CMP_ERR_INVALID_ARGUMENT);
  multi_buttons.selected_states = states;

  CMP_TEST_OK(m3_segmented_buttons_init(&destroy_buttons, &backend, &style,
                                        items, 2, M3_SEGMENTED_MODE_SINGLE,
                                        M3_SEGMENTED_INVALID_INDEX, NULL));
  CMP_TEST_OK(
      destroy_buttons.widget.vtable->destroy(destroy_buttons.widget.ctx));

  {
    CMPTextBackend bad_backend = backend;

    bad_backend.vtable = &g_test_text_vtable_no_measure;
    CMP_TEST_EXPECT(
        m3_segmented_buttons_init(&destroy_buttons, &bad_backend, &style, items,
                                  2, M3_SEGMENTED_MODE_SINGLE, 0, NULL),
        CMP_ERR_UNSUPPORTED);
  }

  CMP_TEST_OK(m3_segmented_buttons_init(&destroy_buttons, &backend, &style,
                                        items, 2, M3_SEGMENTED_MODE_SINGLE, 0,
                                        NULL));
  destroy_buttons.owns_font = CMP_FALSE;
  CMP_TEST_OK(
      destroy_buttons.widget.vtable->destroy(destroy_buttons.widget.ctx));

  CMP_TEST_OK(m3_segmented_buttons_init(&destroy_buttons2, &backend, &style,
                                        items, 2, M3_SEGMENTED_MODE_SINGLE, 0,
                                        NULL));
  destroy_buttons2.font.id = 0u;
  destroy_buttons2.font.generation = 0u;
  CMP_TEST_OK(
      destroy_buttons2.widget.vtable->destroy(destroy_buttons2.widget.ctx));

  CMP_TEST_OK(m3_segmented_buttons_init(&destroy_buttons3, &backend, &style,
                                        items, 2, M3_SEGMENTED_MODE_SINGLE, 0,
                                        NULL));
  destroy_buttons3.text_backend.vtable = NULL;
  CMP_TEST_EXPECT(
      destroy_buttons3.widget.vtable->destroy(destroy_buttons3.widget.ctx),
      CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));
  CMP_TEST_OK(multi_buttons.widget.vtable->destroy(multi_buttons.widget.ctx));
  return CMP_OK;
}

static int test_segmented_extra_coverage(void) {
  TestTabsBackend backend_state;
  CMPTextBackend backend;
  CMPGfx gfx;
  CMPPaintContext paint_ctx;
  M3SegmentedStyle style;
  M3SegmentedButtons buttons;
  M3SegmentedItem items[2];
  CMPBool states[2];
  CMPInputEvent event;
  CMPBool handled;
  CMPScalar width;
  CMPScalar height;
  CMPScalar baseline;
  CMPTextBackend saved_text_backend;
  M3SegmentedStyle saved_style;
  CMPRect saved_bounds;
  SegmentedSelectState select_state;
  cmp_u32 saved_flags;

  CMP_TEST_OK(test_backend_init(&backend_state));
  CMP_TEST_OK(
      setup_text_backend(&backend_state, &backend, &g_test_text_vtable));
  CMP_TEST_OK(setup_gfx(&backend_state, &gfx, &paint_ctx, &g_test_gfx_vtable,
                        &g_test_text_vtable));

  CMP_TEST_OK(m3_segmented_style_init(&style));
  style.text_style.utf8_family = "Sans";
  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;
  states[0] = CMP_FALSE;
  states[1] = CMP_TRUE;

  CMP_TEST_OK(m3_segmented_buttons_init(&buttons, &backend, &style, items, 2,
                                        M3_SEGMENTED_MODE_SINGLE, 0, NULL));
  saved_style = buttons.style;
  saved_text_backend = buttons.text_backend;
  saved_flags = buttons.widget.flags;
  buttons.bounds.width = 200.0f;
  buttons.bounds.height = 40.0f;
  saved_bounds = buttons.bounds;

  CMP_TEST_OK(
      m3_segmented_test_measure_max_text(&buttons, &width, &height, &baseline));

  paint_ctx.gfx = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  buttons.style.spacing = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style = saved_style;

  buttons.bounds.width = -1.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.bounds = saved_bounds;

  buttons.items = NULL;
  buttons.item_count = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  buttons.items = items;
  buttons.item_count = 2;

  buttons.bounds.width = 0.0f;
  buttons.bounds.height = 10.0f;
  buttons.style.padding.left = 20.0f;
  buttons.style.padding.right = 20.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style = saved_style;
  buttons.bounds = saved_bounds;

  buttons.item_count = 0;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.item_count = 2;

  buttons.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_UNSUPPORTED);
  buttons.text_backend = saved_text_backend;

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_OUTLINE_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  buttons.style.outline_width = 1000.0f;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_RANGE);
  buttons.style = saved_style;

  buttons.style.background_color.a = 0.0f;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.style.background_color.a = saved_style.background_color.a;

  items[0].utf8_len = 0;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  items[0].utf8_len = 3;

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw_text = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_text = 0;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx),
                  CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(buttons.widget.vtable->paint(buttons.widget.ctx, &paint_ctx));
  buttons.widget.flags = saved_flags;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  buttons.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  buttons.widget.flags = saved_flags;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 5, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));

  buttons.bounds.width = 0.0f;
  buttons.bounds.height = 10.0f;
  buttons.style.padding.left = 20.0f;
  buttons.style.padding.right = 20.0f;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_RANGE);
  buttons.style = saved_style;
  buttons.bounds = saved_bounds;

  CMP_TEST_OK(m3_segmented_test_set_fail_point(
      M3_SEGMENTED_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(1u));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_test_set_fail_point_error_after(0u));
  CMP_TEST_OK(m3_segmented_test_clear_fail_points());

  buttons.pressed_index = 0;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_STATE);
  buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 500, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));

  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  buttons.pressed_index = M3_SEGMENTED_INVALID_INDEX;
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));

  select_state.calls = 0;
  select_state.last_index = 0;
  select_state.fail_index = 0;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                 &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 5, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 5, 5));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, NULL, NULL));

  buttons.mode = M3_SEGMENTED_MODE_MULTI;
  buttons.selected_states = states;
  select_state.fail_index = 1;
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, segmented_on_select,
                                                 &select_state));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 150, 5));
  CMP_TEST_OK(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled));
  CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 150, 5));
  CMP_TEST_EXPECT(
      buttons.widget.vtable->event(buttons.widget.ctx, &event, &handled),
      CMP_ERR_IO);
  CMP_TEST_OK(m3_segmented_buttons_set_on_select(&buttons, NULL, NULL));
  buttons.mode = M3_SEGMENTED_MODE_SINGLE;
  buttons.selected_states = NULL;

  CMP_TEST_OK(buttons.widget.vtable->destroy(buttons.widget.ctx));
  return CMP_OK;
}

int main(void) {
  int step;

  step = 1;
  if (test_tab_row_style_init() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_validation_helpers() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_internal_helpers() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_init_and_setters() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_widget_fixed() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_widget_scrollable() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_branch_sweep() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_extra_coverage() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_tab_row_branch_edges() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_style_init() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_validation_helpers() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_internal_helpers() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_init_and_setters() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_widget_flow() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_branch_sweep() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }
  step += 1;
  if (test_segmented_extra_coverage() != CMP_OK) {
    fprintf(stderr, "cmp_phase5_tabs step %d\n", step);
    return 1;
  }

  return 0;
}
