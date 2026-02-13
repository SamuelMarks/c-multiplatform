#include "m3/m3_navigation.h"
#include "test_utils.h"

#include <string.h>

#ifdef CMP_TESTING
#define M3_NAV_TEST_FAIL_RESOLVE_MODE 1u
#define M3_NAV_TEST_FAIL_LAYOUT_ITEM_WIDTH_NEGATIVE 2u
#define M3_NAV_TEST_FAIL_LAYOUT_MODE_INVALID 3u
#define M3_NAV_TEST_FAIL_INDICATOR_RECT_NEGATIVE 4u
#define M3_NAV_TEST_FAIL_ITEM_RECT_NEGATIVE 5u
#define M3_NAV_TEST_FAIL_STYLE_INIT 6u
#define M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE 7u
#define M3_NAV_TEST_FAIL_LAYOUT_SPACING_NEGATIVE 8u
#define M3_NAV_TEST_FAIL_LAYOUT_ITEM_HEIGHT_NEGATIVE 9u
#define M3_NAV_TEST_FAIL_MEASURE_CONTENT 10u
#define M3_NAV_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE 11u
#endif

typedef struct TestNavBackend {
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
} TestNavBackend;

typedef struct TestSelectState {
  int calls;
  cmp_usize last_index;
  int fail_index;
} TestSelectState;

static int test_backend_init(TestNavBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return CMP_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight, CMPBool italic,
                                 CMPHandle *out_font) {
  TestNavBackend *backend;

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

  backend = (TestNavBackend *)text;
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
  TestNavBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestNavBackend *)text;
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
  TestNavBackend *backend;

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

  backend = (TestNavBackend *)text;
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
  TestNavBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestNavBackend *)text;
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
  TestNavBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestNavBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect) {
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

static const CMPGfxVTable g_test_gfx_vtable = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

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

static int test_style_init(void) {
  M3NavigationStyle style;

  CMP_TEST_EXPECT(m3_navigation_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_style_init(&style));
  CMP_TEST_ASSERT(style.mode == M3_NAV_MODE_AUTO);
  CMP_TEST_ASSERT(cmp_near(style.bar_height, M3_NAV_DEFAULT_BAR_HEIGHT, 0.001f));
  CMP_TEST_ASSERT(cmp_near(style.rail_width, M3_NAV_DEFAULT_RAIL_WIDTH, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.drawer_width, M3_NAV_DEFAULT_DRAWER_WIDTH, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.item_height, M3_NAV_DEFAULT_ITEM_HEIGHT, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.item_min_width, M3_NAV_DEFAULT_ITEM_MIN_WIDTH, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.item_spacing, M3_NAV_DEFAULT_ITEM_SPACING, 0.001f));
  CMP_TEST_ASSERT(cmp_near(style.indicator_thickness,
                         M3_NAV_DEFAULT_INDICATOR_THICKNESS, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.indicator_corner, M3_NAV_DEFAULT_INDICATOR_CORNER, 0.001f));
  CMP_TEST_ASSERT(
      cmp_near(style.breakpoint_rail, M3_NAV_DEFAULT_BREAKPOINT_RAIL, 0.001f));
  CMP_TEST_ASSERT(cmp_near(style.breakpoint_drawer,
                         M3_NAV_DEFAULT_BREAKPOINT_DRAWER, 0.001f));
  CMP_TEST_ASSERT(style.text_style.size_px == 14);
  CMP_TEST_ASSERT(style.text_style.weight == 400);
  CMP_TEST_ASSERT(style.text_style.italic == CMP_FALSE);
  CMP_TEST_ASSERT(style.text_style.utf8_family == NULL);
  CMP_TEST_ASSERT(cmp_near(style.background_color.a, 0.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(style.selected_text_color.a, 1.0f, 0.001f));

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_STYLE_INIT));
  CMP_TEST_EXPECT(m3_navigation_style_init(&style), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif
  return CMP_OK;
}

static int test_validation_helpers(void) {
  M3NavigationStyle style;
  M3NavigationStyle base_style;
  CMPTextStyle text_style;
  CMPColor color;
  CMPLayoutEdges edges;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3NavigationItem item;
  cmp_u32 mode;
  CMPScalar width;
  CMPScalar height;

  CMP_TEST_EXPECT(m3_navigation_test_validate_color(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_navigation_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_navigation_test_validate_edges(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.right = 0.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 0.0f;
  edges.bottom = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.bottom = 0.0f;
  CMP_TEST_OK(m3_navigation_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(NULL, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_text_style_init(&text_style));
  text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_test_validate_text_style(&text_style, CMP_FALSE));
  text_style.utf8_family = "Test";
  text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.size_px = 12;
  text_style.weight = 50;
  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.weight = 400;
  text_style.italic = 2;
  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.italic = CMP_FALSE;
  text_style.color.r = -0.5f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE),
                 CMP_ERR_RANGE);
  text_style.color.r = 0.0f;
  CMP_TEST_OK(m3_navigation_test_validate_text_style(&text_style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_navigation_test_validate_style(NULL, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";
  base_style = style;
  style.mode = 99u;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.bar_height = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.rail_width = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.drawer_width = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.item_min_width = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.item_spacing = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.indicator_thickness = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.indicator_corner = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.breakpoint_rail = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.breakpoint_drawer = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.breakpoint_rail = 200.0f;
  style.breakpoint_drawer = 100.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.padding.top = 100.0f;
  style.padding.bottom = 100.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.padding.left = 50.0f;
  style.padding.right = 50.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_INVALID_ARGUMENT);
  style = base_style;
  style.text_style.size_px = 0;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.text_style.weight = 50;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.text_style.italic = 3;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.text_style.color.a = 2.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.selected_text_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.indicator_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  style.background_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_style(&style, CMP_TRUE),
                 CMP_ERR_RANGE);
  style = base_style;
  CMP_TEST_OK(m3_navigation_test_validate_style(&style, CMP_TRUE));

  CMP_TEST_EXPECT(m3_navigation_test_validate_items(NULL, 1),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_test_validate_items(NULL, 0));
  item.utf8_label = NULL;
  item.utf8_len = 1;
  CMP_TEST_EXPECT(m3_navigation_test_validate_items(&item, 1),
                 CMP_ERR_INVALID_ARGUMENT);
  item.utf8_len = 0;
  CMP_TEST_OK(m3_navigation_test_validate_items(&item, 1));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_measure_spec(spec),
                 CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_navigation_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_navigation_test_validate_rect(NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  rect.height = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.height = 1.0f;
  CMP_TEST_OK(m3_navigation_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_navigation_test_resolve_mode(NULL, 0.0f, &mode),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_test_resolve_mode(&base_style, -1.0f, &mode),
                 CMP_ERR_RANGE);
  base_style.mode = M3_NAV_MODE_BAR;
  CMP_TEST_OK(m3_navigation_test_resolve_mode(&base_style, 10.0f, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_BAR);
  base_style.mode = M3_NAV_MODE_AUTO;
  base_style.breakpoint_rail = 100.0f;
  base_style.breakpoint_drawer = 200.0f;
  CMP_TEST_OK(m3_navigation_test_resolve_mode(&base_style, 50.0f, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_BAR);
  CMP_TEST_OK(m3_navigation_test_resolve_mode(&base_style, 150.0f, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_RAIL);
  CMP_TEST_OK(m3_navigation_test_resolve_mode(&base_style, 250.0f, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_DRAWER);

  CMP_TEST_EXPECT(m3_navigation_test_measure_content(NULL, M3_NAV_MODE_BAR, 0,
                                                    &width, &height),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_test_measure_content(
                     &base_style, M3_NAV_MODE_BAR, 0, NULL, &height),
                 CMP_ERR_INVALID_ARGUMENT);
  base_style.item_spacing = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_measure_content(
                     &base_style, M3_NAV_MODE_BAR, 0, &width, &height),
                 CMP_ERR_RANGE);
  base_style = style;
  CMP_TEST_EXPECT(
      m3_navigation_test_measure_content(&base_style, 99u, 0, &width, &height),
      CMP_ERR_RANGE);
  base_style.bar_height = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_measure_content(
                     &base_style, M3_NAV_MODE_BAR, 0, &width, &height),
                 CMP_ERR_RANGE);
  base_style = style;
  CMP_TEST_OK(m3_navigation_test_measure_content(&base_style, M3_NAV_MODE_BAR, 0,
                                                &width, &height));
  CMP_TEST_OK(m3_navigation_test_measure_content(&base_style, M3_NAV_MODE_BAR, 2,
                                                &width, &height));
  CMP_TEST_OK(m3_navigation_test_measure_content(&base_style, M3_NAV_MODE_RAIL,
                                                2, &width, &height));

  return CMP_OK;
}

static int test_init_validation(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3NavigationStyle bad_style;
  M3Navigation nav;
  M3NavigationItem items[2];
  M3NavigationItem bad_items[1];
  CMPTextBackend bad_backend;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "Home";
  items[0].utf8_len = 4;
  items[1].utf8_label = "Settings";
  items[1].utf8_len = 8;

  CMP_TEST_EXPECT(m3_navigation_init(NULL, &backend, &style, items, 2, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_init(&nav, NULL, &style, items, 2, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, NULL, items, 2, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  bad_backend = backend;
  bad_backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &bad_backend, &style, items, 2, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  bad_backend = backend;
  bad_backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &bad_backend, &style, items, 2, 0),
                 CMP_ERR_UNSUPPORTED);
  bad_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &bad_backend, &style, items, 2, 0),
                 CMP_ERR_UNSUPPORTED);

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_RESOLVE_MODE));
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, items, 2, 0),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  bad_style = style;
  bad_style.item_height = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &bad_style, items, 2, 0),
                 CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, NULL, 1, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 1;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, bad_items, 1, 0),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, items, 2, 5),
                 CMP_ERR_RANGE);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_navigation_init(&nav, &backend, &style, items, 2, 0),
                 CMP_ERR_IO);
  backend_state.fail_create = 0;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));
  CMP_TEST_ASSERT(nav.widget.vtable != NULL);
  CMP_TEST_ASSERT((nav.widget.flags & CMP_WIDGET_FLAG_FOCUSABLE) != 0u);
  CMP_TEST_ASSERT(nav.selected_index == 0u);

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_setters_and_getters(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[2];
  M3NavigationItem new_items[1];
  cmp_usize selected;
  cmp_u32 mode;
  CMPSemantics semantics;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "One";
  items[0].utf8_len = 3;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));

  CMP_TEST_EXPECT(m3_navigation_get_selected(NULL, &selected),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_get_selected(&nav, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_get_selected(&nav, &selected));
  CMP_TEST_ASSERT(selected == 0u);

  CMP_TEST_EXPECT(nav.widget.vtable->get_semantics(NULL, &semantics),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(nav.widget.vtable->get_semantics(nav.widget.ctx, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  nav.widget.flags = CMP_WIDGET_FLAG_DISABLED | CMP_WIDGET_FLAG_FOCUSABLE;
  CMP_TEST_OK(nav.widget.vtable->get_semantics(nav.widget.ctx, &semantics));
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_DISABLED) != 0u);
  CMP_TEST_ASSERT((semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE) != 0u);
  nav.widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;

  CMP_TEST_EXPECT(m3_navigation_set_selected(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_set_selected(&nav, 5), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_set_selected(&nav, M3_NAV_INVALID_INDEX));
  CMP_TEST_OK(m3_navigation_get_selected(&nav, &selected));
  CMP_TEST_ASSERT(selected == M3_NAV_INVALID_INDEX);
  CMP_TEST_OK(m3_navigation_set_selected(&nav, 1));

  CMP_TEST_EXPECT(m3_navigation_set_on_select(NULL, NULL, NULL),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_set_on_select(&nav, NULL, NULL));

  CMP_TEST_EXPECT(m3_navigation_set_style(NULL, &style),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_set_style(&nav, NULL), CMP_ERR_INVALID_ARGUMENT);
  {
    M3NavigationStyle bad_style = style;
    bad_style.item_height = -1.0f;
    CMP_TEST_EXPECT(m3_navigation_set_style(&nav, &bad_style), CMP_ERR_RANGE);
  }

  CMP_TEST_EXPECT(m3_navigation_set_items(NULL, NULL, 0),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_navigation_set_items(&nav, NULL, 0));
  CMP_TEST_ASSERT(nav.item_count == 0u);
  CMP_TEST_ASSERT(nav.selected_index == M3_NAV_INVALID_INDEX);
  {
    M3NavigationItem bad_items[1];

    bad_items[0].utf8_label = NULL;
    bad_items[0].utf8_len = 1;
    CMP_TEST_EXPECT(m3_navigation_set_items(&nav, bad_items, 1),
                   CMP_ERR_INVALID_ARGUMENT);
  }

  new_items[0].utf8_label = "Only";
  new_items[0].utf8_len = 4;
  nav.selected_index = 2;
  nav.pressed_index = 2;
  CMP_TEST_OK(m3_navigation_set_items(&nav, new_items, 1));
  CMP_TEST_ASSERT(nav.selected_index == M3_NAV_INVALID_INDEX);
  CMP_TEST_ASSERT(nav.pressed_index == M3_NAV_INVALID_INDEX);

  nav.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_get_mode(&nav, &mode), CMP_ERR_RANGE);

  CMP_TEST_EXPECT(m3_navigation_get_mode(NULL, &mode), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_get_mode(&nav, NULL), CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_navigation_set_style(&nav, &style), CMP_ERR_IO);
  backend_state.fail_create = 0;
  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_navigation_set_style(&nav, &style), CMP_ERR_IO);
  backend_state.fail_destroy = 0;

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_measure_layout_mode(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[2];
  CMPMeasureSpec bad_spec;
  CMPMeasureSpec size_spec;
  CMPSize size;
  CMPRect bounds;
  cmp_u32 mode;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.item_min_width = 50.0f;
  style.item_spacing = 10.0f;
  style.bar_height = 60.0f;
  style.padding.left = 5.0f;
  style.padding.right = 5.0f;
  style.padding.top = 2.0f;
  style.padding.bottom = 2.0f;

  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));

  size_spec.mode = CMP_MEASURE_UNSPECIFIED;
  size_spec.size = 0.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(NULL, size_spec, size_spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, size_spec, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  bad_spec.mode = 99u;
  bad_spec.size = 0.0f;
  size_spec.mode = CMP_MEASURE_EXACTLY;
  size_spec.size = 10.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, bad_spec, size_spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);

  bad_spec.mode = CMP_MEASURE_EXACTLY;
  bad_spec.size = -5.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, bad_spec, size_spec, &size),
                 CMP_ERR_RANGE);

  bad_spec.mode = 99u;
  bad_spec.size = 0.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, bad_spec, &size),
                 CMP_ERR_INVALID_ARGUMENT);

  nav.style.item_height = -1.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size),
                 CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_RESOLVE_MODE));
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());

  CMP_TEST_OK(
      m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_MEASURE_CONTENT));
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  style.mode = M3_NAV_MODE_BAR;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  size_spec.mode = CMP_MEASURE_UNSPECIFIED;
  size_spec.size = 0.0f;
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 120.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 60.0f, 0.001f));

  size_spec.mode = CMP_MEASURE_EXACTLY;
  size_spec.size = 123.0f;
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 123.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 123.0f, 0.001f));

  size_spec.mode = CMP_MEASURE_AT_MOST;
  size_spec.size = 50.0f;
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 50.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 50.0f, 0.001f));

  style.mode = M3_NAV_MODE_AUTO;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  size_spec.mode = CMP_MEASURE_UNSPECIFIED;
  size_spec.size = 0.0f;
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));

  nav.style.item_spacing = -1.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size),
                 CMP_ERR_RANGE);
  nav.style.item_spacing = style.item_spacing;

  style.mode = M3_NAV_MODE_RAIL;
  style.rail_width = 72.0f;
  style.item_height = 40.0f;
  style.item_spacing = 5.0f;
  style.padding.top = 4.0f;
  style.padding.bottom = 4.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 72.0f, 0.001f));
  CMP_TEST_ASSERT(cmp_near(size.height, 93.0f, 0.001f));

  style.mode = M3_NAV_MODE_DRAWER;
  style.drawer_width = 300.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  CMP_TEST_OK(nav.widget.vtable->measure(&nav, size_spec, size_spec, &size));
  CMP_TEST_ASSERT(cmp_near(size.width, 300.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->layout(&nav, bounds), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(nav.widget.vtable->layout(NULL, bounds),
                 CMP_ERR_INVALID_ARGUMENT);

  nav.style.item_height = -1.0f;
  bounds.width = 80.0f;
  bounds.height = 60.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->layout(&nav, bounds), CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

  style.mode = M3_NAV_MODE_AUTO;
  style.breakpoint_rail = 100.0f;
  style.breakpoint_drawer = 200.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));

  bounds.width = 80.0f;
  bounds.height = 60.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
  CMP_TEST_OK(m3_navigation_get_mode(&nav, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_BAR);

  bounds.width = 150.0f;
  bounds.height = 60.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
  CMP_TEST_OK(m3_navigation_get_mode(&nav, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_RAIL);

  bounds.width = 250.0f;
  bounds.height = 60.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
  CMP_TEST_OK(m3_navigation_get_mode(&nav, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_DRAWER);

  style.mode = M3_NAV_MODE_BAR;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  CMP_TEST_OK(m3_navigation_get_mode(&nav, &mode));
  CMP_TEST_ASSERT(mode == M3_NAV_MODE_BAR);

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_compute_layout_and_hit_test(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[3];
  M3NavigationLayoutTest layout;
  cmp_usize index;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.item_spacing = 5.0f;
  style.item_height = 20.0f;
  style.padding.left = 0.0f;
  style.padding.right = 0.0f;
  style.padding.top = 0.0f;
  style.padding.bottom = 0.0f;

  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1;
  items[2].utf8_label = "C";
  items[2].utf8_len = 1;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 0));
  nav.bounds.x = 0.0f;
  nav.bounds.y = 0.0f;
  nav.bounds.width = 100.0f;
  nav.bounds.height = 60.0f;

  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(NULL, &layout),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  nav.style.item_height = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

  nav.bounds.width = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.bounds.width = 100.0f;

  nav.style.padding.left = 10.0f;
  nav.style.padding.right = 10.0f;
  nav.style.padding.top = 10.0f;
  nav.style.padding.bottom = 10.0f;
  nav.bounds.width = 5.0f;
  nav.bounds.height = 5.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.padding.left = style.padding.left;
  nav.style.padding.right = style.padding.right;
  nav.style.padding.top = style.padding.top;
  nav.style.padding.bottom = style.padding.bottom;
  nav.bounds.width = 100.0f;
  nav.bounds.height = 60.0f;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_RESOLVE_MODE));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_UNKNOWN);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());

  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_LAYOUT_SPACING_NEGATIVE));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  nav.style.item_spacing = -1.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.item_spacing = style.item_spacing;

  nav.style.mode = M3_NAV_MODE_BAR;
  nav.style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

#ifdef CMP_TESTING
  nav.style.mode = M3_NAV_MODE_BAR;
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_LAYOUT_ITEM_HEIGHT_NEGATIVE));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  nav.bounds.height = nav.style.padding.top + nav.style.padding.bottom;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);

  nav.bounds.height = 20.0f;
  nav.style.item_height = 50.0f;
  CMP_TEST_OK(m3_navigation_test_compute_layout(&nav, &layout));
  CMP_TEST_ASSERT(cmp_near(layout.item_height, 20.0f, 0.001f));
  nav.style.item_height = style.item_height;
  nav.bounds.height = 60.0f;

  nav.item_count = 3;
  nav.style.item_spacing = 10.0f;
  nav.bounds.width = 15.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.item_spacing = style.item_spacing;
  nav.bounds.width = 100.0f;
  nav.item_count = 2;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_LAYOUT_ITEM_WIDTH_NEGATIVE));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  nav.style.mode = M3_NAV_MODE_RAIL;
  nav.style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

#ifdef CMP_TESTING
  nav.style.mode = M3_NAV_MODE_RAIL;
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_LAYOUT_ITEM_HEIGHT_NEGATIVE));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  nav.bounds.width = nav.style.padding.left + nav.style.padding.right;
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  nav.bounds.width = 100.0f;

  nav.item_count = 0;
  CMP_TEST_OK(m3_navigation_test_compute_layout(&nav, &layout));
  CMP_TEST_ASSERT(cmp_near(layout.content_height, 0.0f, 0.001f));
  nav.item_count = 2;

#ifdef CMP_TESTING
  CMP_TEST_OK(
      m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_LAYOUT_MODE_INVALID));
  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, &layout),
                 CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  CMP_TEST_EXPECT(m3_navigation_test_compute_layout(&nav, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  layout.mode = M3_NAV_MODE_BAR;
  layout.start_x = 0.0f;
  layout.start_y = 0.0f;
  layout.item_width = 10.0f;
  layout.item_height = 10.0f;
  layout.spacing = 0.0f;
  layout.content_width = 100.0f;
  layout.content_height = 10.0f;

  CMP_TEST_EXPECT(m3_navigation_test_hit_test(NULL, &layout, 0, 0, &index),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_test_hit_test(&nav, NULL, 0, 0, &index),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_navigation_test_hit_test(&nav, &layout, 0, 0, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  nav.item_count = 0;
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 0, 0, &index));
  nav.item_count = 1;

  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, -5, 5, &index));
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 50, &index));

  layout.item_width = 0.0f;
  layout.spacing = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index),
                 CMP_ERR_RANGE);
  layout.item_width = 10.0f;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index));
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  nav.item_count = 1;
  layout.content_width = 100.0f;
  layout.item_width = 10.0f;
  layout.spacing = 0.0f;
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 50, 5, &index));

  nav.item_count = 2;
  layout.item_width = 10.0f;
  layout.spacing = 5.0f;
  layout.content_width = 100.0f;
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 12, 5, &index));

  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index));
  CMP_TEST_ASSERT(index == 0u);

  layout.mode = M3_NAV_MODE_RAIL;
  layout.start_x = 0.0f;
  layout.start_y = 0.0f;
  layout.item_width = 10.0f;
  layout.item_height = 10.0f;
  layout.spacing = 0.0f;
  layout.content_width = 10.0f;
  layout.content_height = 20.0f;
  nav.item_count = 2;

  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, -5, 5, &index));
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 50, &index));

  layout.item_height = 0.0f;
  layout.spacing = 0.0f;
  CMP_TEST_EXPECT(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index),
                 CMP_ERR_RANGE);
  layout.item_height = 10.0f;

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_HIT_TEST_POS_NEGATIVE));
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index));
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  layout.content_height = 100.0f;
  nav.item_count = 1;
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 50, &index));

  nav.item_count = 2;
  layout.item_height = 10.0f;
  layout.spacing = 5.0f;
  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 12, &index));

  CMP_TEST_OK(m3_navigation_test_hit_test(&nav, &layout, 5, 5, &index));
  CMP_TEST_ASSERT(index == 0u);

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_on_select(void *ctx, M3Navigation *nav, cmp_usize index) {
  TestSelectState *state;

  if (ctx == NULL || nav == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  state = (TestSelectState *)ctx;
  state->calls += 1;
  state->last_index = index;
  if (state->fail_index >= 0 && (cmp_usize)state->fail_index == index) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_event_selection(void) {
  TestNavBackend backend_state;
  TestSelectState select_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[3];
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.mode = M3_NAV_MODE_BAR;
  style.item_spacing = 0.0f;
  style.padding.left = 0.0f;
  style.padding.right = 0.0f;
  style.padding.top = 0.0f;
  style.padding.bottom = 0.0f;

  items[0].utf8_label = "A";
  items[0].utf8_len = 1;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1;
  items[2].utf8_label = "C";
  items[2].utf8_len = 1;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 3, 0));

  select_state.calls = 0;
  select_state.last_index = M3_NAV_INVALID_INDEX;
  select_state.fail_index = -1;
  CMP_TEST_OK(m3_navigation_set_on_select(&nav, test_on_select, &select_state));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 300.0f;
  bounds.height = 60.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));

  CMP_TEST_EXPECT(nav.widget.vtable->event(NULL, &event, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(nav.widget.vtable->event(nav.widget.ctx, NULL, &handled),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(nav.widget.vtable->event(nav.widget.ctx, &event, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 10;
  event.data.pointer.y = 10;
  nav.style.item_height = -1.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled),
                 CMP_ERR_RANGE);
  nav.style.item_height = style.item_height;

  nav.bounds.width = 0.0f;
  nav.bounds.height = 60.0f;
  event.data.pointer.x = 0;
  event.data.pointer.y = 10;
  CMP_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled),
                 CMP_ERR_RANGE);
  nav.bounds.width = bounds.width;
  nav.bounds.height = bounds.height;

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = -10;
  event.data.pointer.y = -10;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  CMP_TEST_ASSERT(nav.pressed_index == M3_NAV_INVALID_INDEX);

  event.type = CMP_INPUT_POINTER_UP;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 10;
  event.data.pointer.y = 10;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(nav.pressed_index == 0u);

  CMP_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled),
                 CMP_ERR_STATE);

  event.type = CMP_INPUT_POINTER_UP;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(nav.selected_index == 0u);
  CMP_TEST_ASSERT(select_state.calls == 1);
  CMP_TEST_ASSERT(select_state.last_index == 0u);

  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 150;
  event.data.pointer.y = 10;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  event.type = CMP_INPUT_POINTER_UP;
  event.data.pointer.x = 290;
  event.data.pointer.y = 10;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(nav.selected_index == 0u);

  select_state.fail_index = 1;
  event.type = CMP_INPUT_POINTER_DOWN;
  event.data.pointer.x = 150;
  event.data.pointer.y = 10;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  event.type = CMP_INPUT_POINTER_UP;
  CMP_TEST_EXPECT(nav.widget.vtable->event(&nav, &event, &handled), CMP_ERR_IO);
  CMP_TEST_ASSERT(nav.selected_index == 0u);

  nav.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  event.type = CMP_INPUT_POINTER_DOWN;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  nav.widget.flags &= ~CMP_WIDGET_FLAG_DISABLED;

  event.type = CMP_INPUT_KEY_DOWN;
  CMP_TEST_OK(nav.widget.vtable->event(&nav, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_paint(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[2];
  M3NavigationItem bad_items[1];
  CMPPaintContext paint_ctx;
  CMPGfx gfx;
  CMPRect bounds;
  CMPRect clip;
  int rc;

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";
  style.mode = M3_NAV_MODE_BAR;
  style.background_color.a = 1.0f;
  style.indicator_thickness = 2.0f;
  style.selected_text_color.r = 1.0f;
  style.selected_text_color.g = 0.0f;
  style.selected_text_color.b = 0.0f;
  style.selected_text_color.a = 1.0f;

  items[0].utf8_label = "X";
  items[0].utf8_len = 1;
  items[1].utf8_label = "Y";
  items[1].utf8_len = 1;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 2, 1));

  gfx.ctx = &backend_state;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  clip.x = 0.0f;
  clip.y = 0.0f;
  clip.width = 200.0f;
  clip.height = 80.0f;
  paint_ctx.gfx = &gfx;
  paint_ctx.clip = clip;
  paint_ctx.dpi_scale = 1.0f;

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 80.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));

  CMP_TEST_EXPECT(nav.widget.vtable->paint(NULL, &paint_ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(nav.widget.vtable->paint(nav.widget.ctx, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  gfx.vtable = &g_test_gfx_vtable_no_rect;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_gfx_vtable;

  rc = nav.widget.vtable->paint(&nav, &paint_ctx);
  CMP_TEST_OK(rc);
  CMP_TEST_ASSERT(backend_state.draw_rect_calls == 2);
  CMP_TEST_ASSERT(backend_state.draw_calls == 2);
  CMP_TEST_ASSERT(cmp_near(backend_state.last_text_color.r, 1.0f, 0.001f));

#ifdef CMP_TESTING
  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_INDICATOR_RECT_NEGATIVE));
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());

  CMP_TEST_OK(m3_navigation_test_set_fail_point(
      M3_NAV_TEST_FAIL_INDICATOR_THICKNESS_NEGATIVE));
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());

  CMP_TEST_OK(
      m3_navigation_test_set_fail_point(M3_NAV_TEST_FAIL_ITEM_RECT_NEGATIVE));
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_navigation_test_clear_fail_points());
#endif

  items[0].utf8_len = 0;
  CMP_TEST_OK(nav.widget.vtable->paint(&nav, &paint_ctx));
  items[0].utf8_len = 1;

  style.background_color.a = 0.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  backend_state.draw_rect_calls = 0;
  backend_state.draw_calls = 0;
  CMP_TEST_OK(nav.widget.vtable->paint(&nav, &paint_ctx));
  CMP_TEST_ASSERT(backend_state.draw_rect_calls == 1);
  CMP_TEST_ASSERT(backend_state.draw_calls == 2);

  nav.bounds.width = -1.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);
  nav.bounds.width = bounds.width;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 1;
  nav.items = bad_items;
  nav.item_count = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  nav.items = items;
  nav.item_count = 2;

  nav.style.padding.left = 10.0f;
  nav.style.padding.right = 10.0f;
  nav.bounds.width = 10.0f;
  nav.bounds.height = bounds.height;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);
  nav.style.padding.left = style.padding.left;
  nav.style.padding.right = style.padding.right;
  nav.bounds.width = bounds.width;

  gfx.vtable = NULL;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_gfx_vtable;

  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = NULL;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);
  gfx.text_vtable = &g_test_text_vtable;

  nav.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx),
                 CMP_ERR_UNSUPPORTED);
  nav.text_backend.vtable = &g_test_text_vtable;

  nav.items = NULL;
  nav.item_count = 0;
  nav.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_OK(nav.widget.vtable->paint(&nav, &paint_ctx));
  nav.text_backend.vtable = &g_test_text_vtable;
  nav.items = items;
  nav.item_count = 2;

  style.mode = M3_NAV_MODE_RAIL;
  style.background_color.a = 0.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  bounds.width = 80.0f;
  bounds.height = 120.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));
  CMP_TEST_OK(nav.widget.vtable->paint(&nav, &paint_ctx));
  style.mode = M3_NAV_MODE_BAR;
  style.background_color.a = 0.0f;
  CMP_TEST_OK(m3_navigation_set_style(&nav, &style));
  bounds.width = 200.0f;
  bounds.height = 80.0f;
  CMP_TEST_OK(nav.widget.vtable->layout(&nav, bounds));

  backend_state.fail_measure = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_IO);
  backend_state.fail_measure = 0;

  backend_state.fail_draw = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_IO);
  backend_state.fail_draw = 0;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  nav.style.background_color.a = 1.0f;
  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;
  nav.style.background_color.a = 0.0f;

  nav.style.indicator_thickness = -1.0f;
  CMP_TEST_EXPECT(nav.widget.vtable->paint(&nav, &paint_ctx), CMP_ERR_RANGE);

  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));
  return CMP_OK;
}

static int test_destroy(void) {
  TestNavBackend backend_state;
  CMPTextBackend backend;
  M3NavigationStyle style;
  M3Navigation nav;
  M3NavigationItem items[1];

  CMP_TEST_OK(test_backend_init(&backend_state));
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  CMP_TEST_OK(m3_navigation_style_init(&style));
  style.text_style.utf8_family = "Sans";

  items[0].utf8_label = "Z";
  items[0].utf8_len = 1;

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 1, 0));
  CMP_TEST_EXPECT(nav.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(nav.widget.vtable->destroy(&nav));

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 1, 0));
  nav.text_backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(nav.widget.vtable->destroy(&nav), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(m3_navigation_init(&nav, &backend, &style, items, 1, 0));
  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(nav.widget.vtable->destroy(&nav), CMP_ERR_IO);
  return CMP_OK;
}

int main(void) {
  CMP_TEST_OK(test_style_init());
  CMP_TEST_OK(test_validation_helpers());
  CMP_TEST_OK(test_init_validation());
  CMP_TEST_OK(test_setters_and_getters());
  CMP_TEST_OK(test_measure_layout_mode());
  CMP_TEST_OK(test_compute_layout_and_hit_test());
  CMP_TEST_OK(test_event_selection());
  CMP_TEST_OK(test_paint());
  CMP_TEST_OK(test_destroy());
  return 0;
}
