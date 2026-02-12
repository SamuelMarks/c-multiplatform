#include "m3/m3_menu.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestMenuBackend {
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
  int negative_width;
  M3Handle last_font;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_text_x;
  M3Scalar last_text_y;
  m3_usize last_text_len;
  M3Color last_text_color;
} TestMenuBackend;

typedef struct TestActionState {
  int calls;
  m3_u32 last_action;
  m3_usize last_index;
  int fail_next;
} TestActionState;

static const M3TextVTable g_test_text_vtable;
static const M3TextVTable g_test_text_vtable_no_draw;
static const M3TextVTable g_test_text_vtable_no_measure;
static const M3TextVTable g_test_text_vtable_no_destroy;
static const M3TextVTable g_test_text_vtable_no_create;
static const M3GfxVTable g_test_gfx_vtable;
static const M3GfxVTable g_test_gfx_vtable_no_draw;

static int test_backend_init(TestMenuBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return M3_OK;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 m3_i32 size_px, m3_i32 weight, M3Bool italic,
                                 M3Handle *out_font) {
  TestMenuBackend *backend;

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

  backend = (TestMenuBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return M3_ERR_IO;
  }

  out_font->id = (m3_u32)backend->create_calls;
  out_font->generation = (m3_u32)backend->create_calls;
  backend->last_font = *out_font;
  return M3_OK;
}

static int test_text_destroy_font(void *text, M3Handle font) {
  TestMenuBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)text;
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
  TestMenuBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestMenuBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return M3_ERR_IO;
  }

  if (backend->negative_width) {
    *out_width = -1.0f;
  } else {
    *out_width = (M3Scalar)(utf8_len * 8u);
  }
  *out_height = 20.0f;
  *out_baseline = 14.0f;
  return M3_OK;
}

static int test_text_draw_text(void *text, M3Handle font, const char *utf8,
                               m3_usize utf8_len, M3Scalar x, M3Scalar y,
                               M3Color color) {
  TestMenuBackend *backend;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return M3_ERR_STATE;
  }

  backend = (TestMenuBackend *)text;
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
  TestMenuBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
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
  TestMenuBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestMenuBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_action(void *ctx, M3Menu *menu, m3_u32 action, m3_usize index) {
  TestActionState *state;

  M3_UNUSED(menu);

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = action;
  state->last_index = index;
  if (state->fail_next) {
    state->fail_next = 0;
    return M3_ERR_IO;
  }
  return M3_OK;
}

static void test_make_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                                    m3_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

static int test_menu_style_init_helper(M3MenuStyle *style) {
  int rc;

  rc = m3_menu_style_init(style);
  if (rc != M3_OK) {
    return rc;
  }

  style->text_style.utf8_family = "Test";
  return M3_OK;
}

static int test_menu_validation_helpers(void) {
  M3Color color;
  M3LayoutEdges edges;
  M3TextStyle style;
  M3MeasureSpec spec;
  M3Rect rect;
  M3MenuAnchor anchor;
  M3MenuPlacement placement;
  M3MenuItem items[1];
  int rc;

  M3_TEST_EXPECT(m3_menu_test_validate_color(NULL), M3_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_color(&color), M3_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  M3_TEST_EXPECT(m3_menu_test_validate_color(&color), M3_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  M3_TEST_EXPECT(m3_menu_test_validate_color(&color), M3_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  M3_TEST_EXPECT(m3_menu_test_validate_color(&color), M3_ERR_RANGE);
  color.a = 1.0f;
  M3_TEST_OK(m3_menu_test_validate_color(&color));

  M3_TEST_EXPECT(m3_menu_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_test_color_set(&color, -0.1f, 0.0f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, -0.1f, 0.0f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, 0.0f, -0.1f, 1.0f),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_menu_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

  M3_TEST_EXPECT(m3_menu_test_validate_edges(NULL), M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 1.0f;
  edges.top = 2.0f;
  edges.right = 3.0f;
  edges.bottom = 4.0f;
  M3_TEST_OK(m3_menu_test_validate_edges(&edges));

  M3_TEST_EXPECT(m3_menu_test_validate_text_style(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  rc = m3_text_style_init(&style);
  M3_TEST_OK(rc);
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  style.utf8_family = "Test";
  style.size_px = 0;
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style.size_px = 12;
  style.weight = 99;
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style.weight = 901;
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style.weight = 400;
  style.italic = 2;
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style.italic = M3_FALSE;
  style.color.r = 1.2f;
  M3_TEST_EXPECT(m3_menu_test_validate_text_style(&style, M3_TRUE),
                 M3_ERR_RANGE);
  style.color.r = 0.0f;
  M3_TEST_OK(m3_menu_test_validate_text_style(&style, M3_TRUE));

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = 10.0f;
  M3_TEST_OK(m3_menu_test_validate_measure_spec(spec));
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  M3_TEST_OK(m3_menu_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_menu_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 1.0f;
  M3_TEST_OK(m3_menu_test_validate_rect(&rect));

  M3_TEST_EXPECT(m3_menu_test_validate_anchor(NULL), M3_ERR_INVALID_ARGUMENT);
  anchor.type = 99u;
  M3_TEST_EXPECT(m3_menu_test_validate_anchor(&anchor), M3_ERR_RANGE);
  anchor.type = M3_MENU_ANCHOR_RECT;
  anchor.rect.x = 0.0f;
  anchor.rect.y = 0.0f;
  anchor.rect.width = -1.0f;
  anchor.rect.height = 1.0f;
  M3_TEST_EXPECT(m3_menu_test_validate_anchor(&anchor), M3_ERR_RANGE);
  anchor.type = M3_MENU_ANCHOR_POINT;
  anchor.point.x = 0.0f;
  anchor.point.y = 0.0f;
  M3_TEST_OK(m3_menu_test_validate_anchor(&anchor));

  M3_TEST_EXPECT(m3_menu_test_validate_placement(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  placement.direction = 99u;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_EXPECT(m3_menu_test_validate_placement(&placement), M3_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = 99u;
  M3_TEST_EXPECT(m3_menu_test_validate_placement(&placement), M3_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_END;
  M3_TEST_OK(m3_menu_test_validate_placement(&placement));

  M3_TEST_OK(m3_menu_test_validate_items(NULL, 0u));
  items[0].utf8_label = NULL;
  items[0].utf8_len = 1u;
  items[0].enabled = M3_TRUE;
  M3_TEST_EXPECT(m3_menu_test_validate_items(items, 1u),
                 M3_ERR_INVALID_ARGUMENT);
  items[0].utf8_label = "A";
  items[0].utf8_len = 1u;
  items[0].enabled = 2;
  M3_TEST_EXPECT(m3_menu_test_validate_items(items, 1u), M3_ERR_RANGE);
  items[0].enabled = M3_FALSE;
  M3_TEST_OK(m3_menu_test_validate_items(items, 1u));

  return 0;
}

static int test_menu_init_and_setters(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3MenuStyle bad_style;
  M3Menu menu;
  M3MenuItem items[2];
  M3MenuItem bad_items[1];
  M3Rect rect;
  M3MenuPlacement placement;
  M3Semantics semantics;
  M3Bool open;
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);

  items[0].utf8_label = "One";
  items[0].utf8_len = 3u;
  items[0].enabled = M3_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = M3_TRUE;

  M3_TEST_EXPECT(m3_menu_init(NULL, &backend, &style, items, 2u),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_init(&menu, NULL, &style, items, 2u),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, NULL, items, 2u),
                 M3_ERR_INVALID_ARGUMENT);

  backend.vtable = NULL;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                 M3_ERR_INVALID_ARGUMENT);

  backend.vtable = &g_test_text_vtable_no_create;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                 M3_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_destroy;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                 M3_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_measure;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                 M3_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                 M3_ERR_UNSUPPORTED);

  backend.vtable = &g_test_text_vtable;
  bad_style = style;
  bad_style.item_height = 0.0f;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &bad_style, items, 2u),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, NULL, 2u),
                 M3_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u), M3_ERR_IO);
  backend_state.fail_create = 0;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));
  M3_TEST_ASSERT(menu.widget.flags & M3_WIDGET_FLAG_FOCUSABLE);
  M3_TEST_ASSERT((menu.widget.flags & M3_WIDGET_FLAG_HIDDEN) == 0u);
  M3_TEST_ASSERT(menu.open == M3_TRUE);

  M3_TEST_EXPECT(menu.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(menu.widget.vtable->get_semantics(menu.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(menu.widget.vtable->get_semantics(menu.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.flags & M3_SEMANTIC_FLAG_FOCUSABLE);
  menu.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(menu.widget.vtable->get_semantics(menu.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.flags & M3_SEMANTIC_FLAG_DISABLED);
  menu.widget.flags &= (m3_u32)~M3_WIDGET_FLAG_DISABLED;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 1u;
  bad_items[0].enabled = M3_TRUE;
  M3_TEST_EXPECT(m3_menu_set_items(&menu, bad_items, 1u),
                 M3_ERR_INVALID_ARGUMENT);
  bad_items[0].utf8_label = "A";
  bad_items[0].utf8_len = 1u;
  bad_items[0].enabled = 2;
  M3_TEST_EXPECT(m3_menu_set_items(&menu, bad_items, 1u), M3_ERR_RANGE);
  M3_TEST_OK(m3_menu_set_items(&menu, items, 2u));
  M3_TEST_ASSERT(menu.item_count == 2u);

  M3_TEST_EXPECT(m3_menu_set_items(NULL, items, 2u), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_menu_set_style(NULL, &style), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_set_style(&menu, NULL), M3_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.item_height = 0.0f;
  M3_TEST_EXPECT(m3_menu_set_style(&menu, &bad_style), M3_ERR_RANGE);

  backend_state.fail_create = 1;
  M3_TEST_EXPECT(m3_menu_set_style(&menu, &style), M3_ERR_IO);
  backend_state.fail_create = 0;

  backend_state.fail_destroy = 1;
  M3_TEST_EXPECT(m3_menu_set_style(&menu, &style), M3_ERR_IO);
  M3_TEST_ASSERT(backend_state.destroy_calls >= 2);
  backend_state.fail_destroy = 0;

  M3_TEST_OK(m3_menu_set_style(&menu, &style));

  M3_TEST_EXPECT(m3_menu_set_anchor_rect(NULL, &rect), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_set_anchor_rect(&menu, NULL), M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  M3_TEST_EXPECT(m3_menu_set_anchor_rect(&menu, &rect), M3_ERR_RANGE);
  rect.width = 10.0f;
  rect.height = 5.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &rect));

  M3_TEST_EXPECT(m3_menu_set_anchor_point(NULL, 0.0f, 0.0f),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_menu_set_anchor_point(&menu, 4.0f, 5.0f));

  M3_TEST_EXPECT(m3_menu_set_placement(NULL, &placement),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_set_placement(&menu, NULL), M3_ERR_INVALID_ARGUMENT);
  placement.direction = 99u;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_EXPECT(m3_menu_set_placement(&menu, &placement), M3_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = 99u;
  M3_TEST_EXPECT(m3_menu_set_placement(&menu, &placement), M3_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_END;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));

  M3_TEST_EXPECT(m3_menu_set_open(NULL, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_set_open(&menu, 2), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_menu_set_open(&menu, M3_FALSE));
  M3_TEST_ASSERT(menu.widget.flags & M3_WIDGET_FLAG_HIDDEN);
  M3_TEST_OK(m3_menu_set_open(&menu, M3_TRUE));
  M3_TEST_ASSERT((menu.widget.flags & M3_WIDGET_FLAG_HIDDEN) == 0u);

  M3_TEST_EXPECT(m3_menu_get_open(NULL, &open), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_open(&menu, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_menu_get_open(&menu, &open));
  M3_TEST_ASSERT(open == M3_TRUE);

  M3_TEST_EXPECT(m3_menu_get_bounds(NULL, &rect), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_bounds(&menu, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_overlay_bounds(NULL, &rect),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_overlay_bounds(&menu, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_resolved_direction(NULL, &placement.direction),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_resolved_direction(&menu, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_measure_layout(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  M3MeasureSpec width;
  M3MeasureSpec height;
  M3Size size;
  M3Rect bounds;
  M3Rect anchor_rect;
  M3Rect out_bounds;
  M3Rect overlay_bounds;
  M3MenuPlacement placement;
  m3_u32 resolved;
  M3Scalar expected_width;
  M3Scalar expected_height;
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);
  style.padding.left = 0.0f;
  style.padding.right = 0.0f;
  style.padding.top = 0.0f;
  style.padding.bottom = 0.0f;
  style.min_width = 0.0f;
  style.max_width = 0.0f;
  style.item_height = 20.0f;
  style.item_spacing = 0.0f;
  style.anchor_gap = 0.0f;

  items[0].utf8_label = "AA";
  items[0].utf8_len = 2u;
  items[0].enabled = M3_TRUE;
  items[1].utf8_label = "BBBB";
  items[1].utf8_len = 4u;
  items[1].enabled = M3_TRUE;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  width.mode = 99u;
  width.size = 0.0f;
  height.mode = M3_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  M3_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      M3_ERR_INVALID_ARGUMENT);

  width.mode = M3_MEASURE_AT_MOST;
  width.size = -1.0f;
  height.mode = M3_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  M3_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      M3_ERR_RANGE);

  width.mode = M3_MEASURE_UNSPECIFIED;
  width.size = 0.0f;
  height.mode = M3_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  M3_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  expected_width = 32.0f;
  expected_height = 40.0f;
  M3_TEST_ASSERT(size.width == expected_width);
  M3_TEST_ASSERT(size.height == expected_height);

  width.mode = M3_MEASURE_AT_MOST;
  width.size = 10.0f;
  height.mode = M3_MEASURE_AT_MOST;
  height.size = 10.0f;
  M3_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  M3_TEST_ASSERT(size.width == 10.0f);
  M3_TEST_ASSERT(size.height == 10.0f);

  width.mode = M3_MEASURE_EXACTLY;
  width.size = 50.0f;
  height.mode = M3_MEASURE_EXACTLY;
  height.size = 60.0f;
  M3_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  M3_TEST_ASSERT(size.width == 50.0f);
  M3_TEST_ASSERT(size.height == 60.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 100.0f;
  menu.anchor.type = 99u;
  M3_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                 M3_ERR_RANGE);

  anchor_rect.x = 10.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 20.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));

  menu.placement.direction = 99u;
  menu.placement.align = M3_MENU_ALIGN_START;
  M3_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                 M3_ERR_RANGE);

  menu.placement.direction = M3_MENU_DIRECTION_DOWN;
  menu.placement.align = 99u;
  M3_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                 M3_ERR_RANGE);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_OK(m3_menu_get_overlay_bounds(&menu, &overlay_bounds));
  M3_TEST_ASSERT(overlay_bounds.width == bounds.width);
  M3_TEST_ASSERT(overlay_bounds.height == bounds.height);
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);
  M3_TEST_ASSERT(out_bounds.x == 10.0f);
  M3_TEST_ASSERT(out_bounds.y == 20.0f);

  placement.direction = M3_MENU_DIRECTION_UP;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.y = 60.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);
  M3_TEST_ASSERT(out_bounds.y == 20.0f);

  anchor_rect.y = 5.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);
  M3_TEST_ASSERT(out_bounds.y == 15.0f);

  anchor_rect.y = 80.0f;
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);
  M3_TEST_ASSERT(out_bounds.y == 40.0f);

  placement.direction = M3_MENU_DIRECTION_RIGHT;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 10.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);
  M3_TEST_ASSERT(out_bounds.x == 20.0f);

  anchor_rect.x = 80.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);
  M3_TEST_ASSERT(out_bounds.x == 48.0f);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_CENTER;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 50.0f;
  anchor_rect.y = 50.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);
  M3_TEST_ASSERT(out_bounds.y == 35.0f);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_END;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 5.0f;
  anchor_rect.y = 50.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);
  M3_TEST_ASSERT(out_bounds.y == 20.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_CENTER;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 0.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(out_bounds.x == 0.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_END;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 50.0f;
  anchor_rect.width = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(out_bounds.x == 28.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  M3_TEST_OK(m3_menu_set_anchor_point(&menu, 5.0f, 5.0f));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(out_bounds.x == 5.0f);

  menu.style.item_height = 60.0f;
  menu.style.item_spacing = 0.0f;
  menu.style.min_width = 100.0f;
  bounds.width = 50.0f;
  bounds.height = 50.0f;

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 10.0f;
  anchor_rect.y = 30.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);

  placement.direction = M3_MENU_DIRECTION_UP;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.y = 5.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);

  placement.direction = M3_MENU_DIRECTION_RIGHT;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 30.0f;
  anchor_rect.y = 10.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 5.0f;
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  M3_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);

  anchor_rect.x = 110.0f;
  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_START;
  M3_TEST_OK(m3_menu_set_placement(&menu, &placement));
  M3_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(out_bounds.x == 0.0f);

  bounds.width = 20.0f;
  bounds.height = 30.0f;
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  M3_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  M3_TEST_ASSERT(out_bounds.width == 20.0f);
  M3_TEST_ASSERT(out_bounds.height == 30.0f);

  M3_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_item_bounds(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  M3Rect bounds;
  M3Rect item_bounds;
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);
  style.padding.left = 2.0f;
  style.padding.right = 2.0f;
  style.padding.top = 1.0f;
  style.padding.bottom = 1.0f;
  style.item_height = 10.0f;
  style.item_spacing = 0.0f;
  style.min_width = 0.0f;
  style.max_width = 0.0f;
  style.anchor_gap = 0.0f;

  items[0].utf8_label = "A";
  items[0].utf8_len = 1u;
  items[0].enabled = M3_TRUE;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1u;
  items[1].enabled = M3_TRUE;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 50.0f;
  bounds.height = 50.0f;
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  M3_TEST_EXPECT(m3_menu_get_item_bounds(NULL, 0u, &item_bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 0u, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 5u, &item_bounds),
                 M3_ERR_RANGE);

  menu.style.item_height = 0.0f;
  menu.style.item_spacing = 0.0f;
  M3_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 0u, &item_bounds),
                 M3_ERR_RANGE);
  menu.style.item_height = 10.0f;

  M3_TEST_OK(m3_menu_get_item_bounds(&menu, 1u, &item_bounds));
  M3_TEST_ASSERT(item_bounds.y ==
                 menu.menu_bounds.y + menu.style.padding.top + 10.0f);

  M3_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_events(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  TestActionState action;
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);
  style.padding.left = 0.0f;
  style.padding.right = 0.0f;
  style.padding.top = 0.0f;
  style.padding.bottom = 0.0f;
  style.item_height = 10.0f;
  style.item_spacing = 4.0f;
  style.min_width = 0.0f;
  style.max_width = 0.0f;
  style.anchor_gap = 0.0f;

  items[0].utf8_label = "One";
  items[0].utf8_len = 3u;
  items[0].enabled = M3_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = M3_FALSE;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  bounds.x = 10.0f;
  bounds.y = 10.0f;
  bounds.width = 80.0f;
  bounds.height = 80.0f;
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  action.calls = 0;
  action.last_action = 0u;
  action.last_index = M3_MENU_INVALID_INDEX;
  action.fail_next = 0;
  M3_TEST_OK(m3_menu_set_on_action(&menu, test_action, &action));

  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x - 5.0f),
                          (m3_i32)(menu.menu_bounds.y - 5.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(action.calls == 1);
  M3_TEST_ASSERT(action.last_action == M3_MENU_ACTION_DISMISS);
  M3_TEST_ASSERT(action.last_index == M3_MENU_INVALID_INDEX);

  action.fail_next = 1;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x - 2.0f),
                          (m3_i32)(menu.menu_bounds.y - 2.0f));
  handled = M3_FALSE;
  M3_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                 M3_ERR_IO);

  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(menu.pressed_index == 0u);

  test_make_pointer_event(&event, M3_INPUT_POINTER_UP,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(action.last_action == M3_MENU_ACTION_SELECT);
  M3_TEST_ASSERT(action.last_index == 0u);

  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 15.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(menu.pressed_index == M3_MENU_INVALID_INDEX);

  test_make_pointer_event(&event, M3_INPUT_POINTER_UP,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 15.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));

  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 11.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(menu.pressed_index == M3_MENU_INVALID_INDEX);

  menu.item_count = 0u;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  menu.item_count = 2u;

  menu.style.item_height = 0.0f;
  menu.style.item_spacing = 0.0f;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  menu.style.item_height = 10.0f;

  test_make_pointer_event(&event, M3_INPUT_POINTER_MOVE,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  test_make_pointer_event(&event, M3_INPUT_POINTER_SCROLL,
                          (m3_i32)(menu.menu_bounds.x + 1.0f),
                          (m3_i32)(menu.menu_bounds.y + 1.0f));
  handled = M3_FALSE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  menu.widget.flags |= M3_WIDGET_FLAG_HIDDEN;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0);
  handled = M3_TRUE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  menu.widget.flags &= (m3_u32)~M3_WIDGET_FLAG_HIDDEN;

  menu.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0);
  handled = M3_TRUE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  menu.widget.flags &= (m3_u32)~M3_WIDGET_FLAG_DISABLED;

  menu.menu_bounds.width = -1.0f;
  test_make_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0);
  handled = M3_FALSE;
  M3_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                 M3_ERR_RANGE);
  menu.menu_bounds.width = 10.0f;

  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_KEY_DOWN;
  handled = M3_TRUE;
  M3_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_paint(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  M3MenuItem empty_items[1];
  M3Rect bounds;
  M3PaintContext ctx;
  M3Gfx gfx;
  M3Gfx bad_gfx;
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);
  style.padding.left = 0.0f;
  style.padding.right = 0.0f;
  style.padding.top = 0.0f;
  style.padding.bottom = 0.0f;
  style.item_height = 20.0f;
  style.item_spacing = 0.0f;
  style.min_width = 0.0f;
  style.max_width = 0.0f;
  style.anchor_gap = 0.0f;

  items[0].utf8_label = "One";
  items[0].utf8_len = 3u;
  items[0].enabled = M3_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = M3_FALSE;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  M3_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  gfx.ctx = &backend_state;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;

  M3_TEST_EXPECT(menu.widget.vtable->paint(NULL, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  bad_gfx.ctx = &backend_state;
  bad_gfx.vtable = &g_test_gfx_vtable_no_draw;
  bad_gfx.text_vtable = &g_test_text_vtable;
  ctx.gfx = &bad_gfx;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);

  ctx.gfx = &gfx;
  menu.style.item_height = 0.0f;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  menu.style.item_height = 20.0f;

  menu.menu_bounds.width = -1.0f;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  menu.menu_bounds.width = 50.0f;

  menu.text_backend.vtable = &g_test_text_vtable_no_measure;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  menu.text_backend.vtable = &g_test_text_vtable;

  menu.font.id = 0u;
  menu.font.generation = 0u;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_STATE);
  menu.font.id = 1u;
  menu.font.generation = 1u;

  backend_state.negative_width = 1;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  backend_state.negative_width = 0;

  backend_state.fail_draw_rect = 1;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx), M3_ERR_IO);
  backend_state.fail_draw_rect = 0;

  menu.style.shadow_enabled = M3_TRUE;
  menu.style.shadow.layers = 0u;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_RANGE);
  menu.style.shadow.layers = 1u;

  ctx.gfx->text_vtable = &g_test_text_vtable_no_draw;
  M3_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  ctx.gfx->text_vtable = &g_test_text_vtable;

  menu.style.disabled_text_color.r = 0.25f;
  menu.style.disabled_text_color.g = 0.5f;
  menu.style.disabled_text_color.b = 0.75f;
  menu.style.disabled_text_color.a = 1.0f;

  M3_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  M3_TEST_ASSERT(backend_state.last_text_color.r ==
                 menu.style.disabled_text_color.r);

  menu.style.shadow.color.a = 0.5f;
  menu.style.shadow.layers = 1u;
  backend_state.push_clip_calls = 0;
  backend_state.pop_clip_calls = 0;
  M3_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  M3_TEST_ASSERT(backend_state.push_clip_calls > 0);
  M3_TEST_ASSERT(backend_state.pop_clip_calls > 0);

  empty_items[0].utf8_label = NULL;
  empty_items[0].utf8_len = 0u;
  empty_items[0].enabled = M3_TRUE;
  M3_TEST_OK(m3_menu_set_items(&menu, empty_items, 1u));
  ctx.gfx->text_vtable = NULL;
  M3_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  ctx.gfx->text_vtable = &g_test_text_vtable;

  M3_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_destroy(void) {
  TestMenuBackend backend_state;
  M3TextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[1];
  int rc;

  rc = test_backend_init(&backend_state);
  M3_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  M3_TEST_OK(rc);

  items[0].utf8_label = "A";
  items[0].utf8_len = 1u;
  items[0].enabled = M3_TRUE;

  M3_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 1u));

  M3_TEST_EXPECT(menu.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);

  menu.text_backend.vtable = &g_test_text_vtable_no_destroy;
  menu.owns_font = M3_TRUE;
  menu.font.id = 1u;
  menu.font.generation = 1u;
  M3_TEST_EXPECT(menu.widget.vtable->destroy(menu.widget.ctx),
                 M3_ERR_UNSUPPORTED);

  return 0;
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

static const M3GfxVTable g_test_gfx_vtable = {NULL,
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

static const M3GfxVTable g_test_gfx_vtable_no_draw = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

int main(void) {
  M3_TEST_OK(test_menu_validation_helpers());
  M3_TEST_OK(test_menu_init_and_setters());
  M3_TEST_OK(test_menu_measure_layout());
  M3_TEST_OK(test_menu_item_bounds());
  M3_TEST_OK(test_menu_events());
  M3_TEST_OK(test_menu_paint());
  M3_TEST_OK(test_menu_destroy());
  return 0;
}
