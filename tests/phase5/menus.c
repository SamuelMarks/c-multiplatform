#include "m3/m3_menu.h"
#include "test_utils.h"

#include <string.h>

int CMP_CALL m3_menu_test_set_style_fail_text_style(CMPBool enable);
int CMP_CALL m3_menu_test_set_style_fail_shadow_init(CMPBool enable);
int CMP_CALL m3_menu_test_set_style_fail_background_color(CMPBool enable);
int CMP_CALL m3_menu_test_set_style_fail_disabled_color(CMPBool enable);

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
  int fail_measure_after;
  int negative_width;
  CMPHandle last_font;
  CMPRect last_rect;
  CMPColor last_rect_color;
  CMPScalar last_corner;
  CMPScalar last_text_x;
  CMPScalar last_text_y;
  cmp_usize last_text_len;
  CMPColor last_text_color;
} TestMenuBackend;

typedef struct TestActionState {
  int calls;
  cmp_u32 last_action;
  cmp_usize last_index;
  int fail_next;
} TestActionState;

static const CMPTextVTable g_test_text_vtable;
static const CMPTextVTable g_test_text_vtable_no_draw;
static const CMPTextVTable g_test_text_vtable_no_measure;
static const CMPTextVTable g_test_text_vtable_no_destroy;
static const CMPTextVTable g_test_text_vtable_no_create;
static const CMPGfxVTable g_test_gfx_vtable;
static const CMPGfxVTable g_test_gfx_vtable_no_draw;

static int test_backend_init(TestMenuBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  return CMP_OK;
}

static int cmp_near(CMPScalar a, CMPScalar b, CMPScalar tol) {
  CMPScalar diff;

  diff = a - b;
  if (diff < 0.0f) {
    diff = -diff;
  }
  return (diff <= tol) ? 1 : 0;
}

static int test_text_create_font(void *text, const char *utf8_family,
                                 cmp_i32 size_px, cmp_i32 weight,
                                 CMPBool italic, CMPHandle *out_font) {
  TestMenuBackend *backend;

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

  backend = (TestMenuBackend *)text;
  backend->create_calls += 1;
  if (backend->fail_create) {
    return CMP_ERR_IO;
  }

  out_font->id = (cmp_u32)backend->create_calls;
  out_font->generation = (cmp_u32)backend->create_calls;
  backend->last_font = *out_font;
  return CMP_OK;
}

static int test_text_destroy_font(void *text, CMPHandle font) {
  TestMenuBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)text;
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
  TestMenuBackend *backend;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestMenuBackend *)text;
  backend->measure_calls += 1;
  if (backend->fail_measure) {
    return CMP_ERR_IO;
  }
  if (backend->fail_measure_after > 0 &&
      backend->measure_calls >= backend->fail_measure_after) {
    return CMP_ERR_IO;
  }

  if (backend->negative_width) {
    *out_width = -1.0f;
  } else {
    *out_width = (CMPScalar)(utf8_len * 8u);
  }
  *out_height = 20.0f;
  *out_baseline = 14.0f;
  return CMP_OK;
}

static int test_text_draw_text(void *text, CMPHandle font, const char *utf8,
                               cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                               CMPColor color) {
  TestMenuBackend *backend;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (font.id == 0u && font.generation == 0u) {
    return CMP_ERR_STATE;
  }

  backend = (TestMenuBackend *)text;
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
  TestMenuBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
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
  TestMenuBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
  backend->push_clip_calls += 1;
  if (backend->fail_push_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_gfx_pop_clip(void *gfx) {
  TestMenuBackend *backend;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (TestMenuBackend *)gfx;
  backend->pop_clip_calls += 1;
  if (backend->fail_pop_clip) {
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static int test_action(void *ctx, M3Menu *menu, cmp_u32 action,
                       cmp_usize index) {
  TestActionState *state;

  CMP_UNUSED(menu);

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestActionState *)ctx;
  state->calls += 1;
  state->last_action = action;
  state->last_index = index;
  if (state->fail_next) {
    state->fail_next = 0;
    return CMP_ERR_IO;
  }
  return CMP_OK;
}

static void test_make_pointer_event(CMPInputEvent *event, cmp_u32 type,
                                    cmp_i32 x, cmp_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

static int test_menu_style_init_helper(M3MenuStyle *style) {
  int rc;

  rc = m3_menu_style_init(style);
  if (rc != CMP_OK) {
    return rc;
  }

  style->text_style.utf8_family = "Test";
  return CMP_OK;
}

static int test_menu_style_init_failures(void) {
  M3MenuStyle style;

  CMP_TEST_EXPECT(m3_menu_style_init(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_menu_test_set_style_fail_text_style(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_style_init(&style), CMP_ERR_IO);

  CMP_TEST_OK(m3_menu_test_set_style_fail_shadow_init(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_style_init(&style), CMP_ERR_IO);

  CMP_TEST_OK(m3_menu_test_set_style_fail_background_color(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_style_init(&style), CMP_ERR_IO);

  CMP_TEST_OK(m3_menu_test_set_style_fail_disabled_color(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_style_init(&style), CMP_ERR_IO);

  return CMP_OK;
}

static int test_menu_validation_helpers(void) {
  CMPColor color;
  CMPLayoutEdges edges;
  CMPTextStyle style;
  CMPMeasureSpec spec;
  CMPRect rect;
  M3MenuAnchor anchor;
  M3MenuPlacement placement;
  M3MenuStyle menu_style;
  M3MenuStyle bad_menu_style;
  M3Menu menu;
  M3MenuItem items[1];
  M3MenuItem menu_items[2];
  CMPRect overlay;
  CMPRect panel_bounds;
  CMPScalar text_width;
  CMPScalar panel_width;
  CMPScalar panel_height;
  CMPBool has_label;
  CMPBool inside;
  cmp_usize index;
  cmp_u32 direction;
  TestMenuBackend backend;
  int rc;

  CMP_TEST_EXPECT(m3_menu_test_validate_color(NULL), CMP_ERR_INVALID_ARGUMENT);
  color.r = -0.1f;
  color.g = 0.0f;
  color.b = 0.0f;
  color.a = 1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_color(&color), CMP_ERR_RANGE);
  color.r = 0.0f;
  color.g = 1.2f;
  CMP_TEST_EXPECT(m3_menu_test_validate_color(&color), CMP_ERR_RANGE);
  color.g = 0.0f;
  color.b = 1.2f;
  CMP_TEST_EXPECT(m3_menu_test_validate_color(&color), CMP_ERR_RANGE);
  color.b = 0.0f;
  color.a = 1.2f;
  CMP_TEST_EXPECT(m3_menu_test_validate_color(&color), CMP_ERR_RANGE);
  color.a = 1.0f;
  CMP_TEST_OK(m3_menu_test_validate_color(&color));

  CMP_TEST_EXPECT(m3_menu_test_color_set(NULL, 0.0f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_color_set(&color, -0.1f, 0.0f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, -0.1f, 0.0f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, 0.0f, -0.1f, 1.0f),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_menu_test_color_set(&color, 0.0f, 0.0f, 0.0f, -0.1f),
                  CMP_ERR_RANGE);
  CMP_TEST_OK(m3_menu_test_color_set(&color, 0.1f, 0.2f, 0.3f, 0.4f));

  CMP_TEST_EXPECT(m3_menu_test_validate_edges(NULL), CMP_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_edges(&edges), CMP_ERR_RANGE);
  edges.left = 1.0f;
  edges.top = 2.0f;
  edges.right = 3.0f;
  edges.bottom = 4.0f;
  CMP_TEST_OK(m3_menu_test_validate_edges(&edges));

  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(NULL, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  rc = cmp_text_style_init(&style);
  CMP_TEST_OK(rc);
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  style.utf8_family = "Test";
  style.size_px = 0;
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style.size_px = 12;
  style.weight = 99;
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style.weight = 901;
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style.weight = 400;
  style.italic = 2;
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style.italic = CMP_FALSE;
  style.color.r = 1.2f;
  CMP_TEST_EXPECT(m3_menu_test_validate_text_style(&style, CMP_TRUE),
                  CMP_ERR_RANGE);
  style.color.r = 0.0f;
  CMP_TEST_OK(m3_menu_test_validate_text_style(&style, CMP_TRUE));

  spec.mode = 99u;
  spec.size = 0.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_measure_spec(spec),
                  CMP_ERR_INVALID_ARGUMENT);
  spec.mode = CMP_MEASURE_EXACTLY;
  spec.size = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_measure_spec(spec), CMP_ERR_RANGE);
  spec.mode = CMP_MEASURE_AT_MOST;
  spec.size = 10.0f;
  CMP_TEST_OK(m3_menu_test_validate_measure_spec(spec));
  spec.mode = CMP_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  CMP_TEST_OK(m3_menu_test_validate_measure_spec(spec));

  CMP_TEST_EXPECT(m3_menu_test_validate_rect(NULL), CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_rect(&rect), CMP_ERR_RANGE);
  rect.width = 1.0f;
  CMP_TEST_OK(m3_menu_test_validate_rect(&rect));

  CMP_TEST_EXPECT(m3_menu_test_validate_anchor(NULL), CMP_ERR_INVALID_ARGUMENT);
  anchor.type = 99u;
  CMP_TEST_EXPECT(m3_menu_test_validate_anchor(&anchor), CMP_ERR_RANGE);
  anchor.type = M3_MENU_ANCHOR_RECT;
  anchor.rect.x = 0.0f;
  anchor.rect.y = 0.0f;
  anchor.rect.width = -1.0f;
  anchor.rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_anchor(&anchor), CMP_ERR_RANGE);
  anchor.type = M3_MENU_ANCHOR_RECT;
  anchor.rect.x = 1.0f;
  anchor.rect.y = 2.0f;
  anchor.rect.width = 3.0f;
  anchor.rect.height = 4.0f;
  CMP_TEST_OK(m3_menu_test_validate_anchor(&anchor));
  anchor.type = M3_MENU_ANCHOR_POINT;
  anchor.point.x = 0.0f;
  anchor.point.y = 0.0f;
  CMP_TEST_OK(m3_menu_test_validate_anchor(&anchor));

  CMP_TEST_EXPECT(m3_menu_test_validate_placement(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  placement.direction = 99u;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_EXPECT(m3_menu_test_validate_placement(&placement), CMP_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = 99u;
  CMP_TEST_EXPECT(m3_menu_test_validate_placement(&placement), CMP_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_END;
  CMP_TEST_OK(m3_menu_test_validate_placement(&placement));

  CMP_TEST_OK(m3_menu_test_validate_items(NULL, 0u));
  items[0].utf8_label = NULL;
  items[0].utf8_len = 1u;
  items[0].enabled = CMP_TRUE;
  CMP_TEST_EXPECT(m3_menu_test_validate_items(items, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  items[0].utf8_label = "A";
  items[0].utf8_len = 1u;
  items[0].enabled = 2;
  CMP_TEST_EXPECT(m3_menu_test_validate_items(items, 1u), CMP_ERR_RANGE);
  items[0].enabled = CMP_FALSE;
  CMP_TEST_OK(m3_menu_test_validate_items(items, 1u));

  rc = test_backend_init(&backend);
  CMP_TEST_OK(rc);
  rc = test_menu_style_init_helper(&menu_style);
  CMP_TEST_OK(rc);

  CMP_TEST_EXPECT(m3_menu_test_validate_style(NULL, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_menu_style = menu_style;
  bad_menu_style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.item_spacing = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.min_width = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.min_width = 10.0f;
  bad_menu_style.max_width = 5.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.corner_radius = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.anchor_gap = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.padding.left = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.text_style.utf8_family = NULL;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_menu_style = menu_style;
  bad_menu_style.background_color.r = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);
  bad_menu_style = menu_style;
  bad_menu_style.disabled_text_color.a = 2.0f;
  CMP_TEST_EXPECT(m3_menu_test_validate_style(&bad_menu_style, CMP_TRUE),
                  CMP_ERR_RANGE);

  memset(&menu, 0, sizeof(menu));
  menu.text_backend.ctx = &backend;
  menu.text_backend.vtable = &g_test_text_vtable;
  menu.style = menu_style;
  menu.items = menu_items;
  menu.item_count = 2u;
  menu.font.id = 1u;
  menu.font.generation = 1u;

  menu_items[0].utf8_label = "Alpha";
  menu_items[0].utf8_len = 5u;
  menu_items[0].enabled = CMP_TRUE;
  menu_items[1].utf8_label = NULL;
  menu_items[1].utf8_len = 0u;
  menu_items[1].enabled = CMP_TRUE;

  CMP_TEST_EXPECT(m3_menu_test_update_metrics(NULL, &text_width, &has_label),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, NULL, &has_label),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, &text_width, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  menu.items = NULL;
  menu.item_count = 1u;
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, &text_width, &has_label),
                  CMP_ERR_INVALID_ARGUMENT);
  menu.items = menu_items;
  menu.item_count = 2u;

  CMP_TEST_OK(m3_menu_test_update_metrics(&menu, &text_width, &has_label));
  CMP_TEST_ASSERT(has_label == CMP_TRUE);

  menu_items[0].utf8_label = NULL;
  menu_items[0].utf8_len = 0u;
  menu_items[1].utf8_label = NULL;
  menu_items[1].utf8_len = 0u;
  CMP_TEST_OK(m3_menu_test_update_metrics(&menu, &text_width, &has_label));
  CMP_TEST_ASSERT(has_label == CMP_FALSE);
  menu_items[0].utf8_label = "Alpha";
  menu_items[0].utf8_len = 5u;
  menu_items[1].utf8_label = NULL;
  menu_items[1].utf8_len = 0u;

  backend.fail_measure = 1;
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, &text_width, &has_label),
                  CMP_ERR_IO);
  backend.fail_measure = 0;

  backend.measure_calls = 0;
  backend.fail_measure_after = 2;
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, &text_width, &has_label),
                  CMP_ERR_IO);
  backend.fail_measure_after = 0;

  backend.measure_calls = 0;
  backend.negative_width = 1;
  menu_items[0].utf8_label = "A";
  menu_items[0].utf8_len = 1u;
  CMP_TEST_EXPECT(m3_menu_test_update_metrics(&menu, &text_width, &has_label),
                  CMP_ERR_RANGE);
  backend.negative_width = 0;

  CMP_TEST_EXPECT(m3_menu_test_compute_panel_size(NULL, &panel_width,
                                                  &panel_height, &has_label),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_menu_test_compute_panel_size(&menu, NULL, &panel_height, &has_label),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_menu_test_compute_panel_size(&menu, &panel_width, NULL, &has_label),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      m3_menu_test_compute_panel_size(&menu, &panel_width, &panel_height, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  menu.style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_size(&menu, &panel_width,
                                                  &panel_height, &has_label),
                  CMP_ERR_RANGE);
  menu.style = menu_style;

  menu.items = NULL;
  menu.item_count = 1u;
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_size(&menu, &panel_width,
                                                  &panel_height, &has_label),
                  CMP_ERR_INVALID_ARGUMENT);
  menu.items = menu_items;
  menu.item_count = 2u;

  menu.style.min_width = 0.0f;
  menu.style.max_width = 20.0f;
  CMP_TEST_OK(m3_menu_test_compute_panel_size(&menu, &panel_width,
                                              &panel_height, &has_label));
  CMP_TEST_ASSERT(cmp_near(panel_width, menu.style.max_width, 0.001f));
  menu.style.min_width = menu_style.min_width;
  menu.style.max_width = menu_style.max_width;

  CMP_TEST_OK(m3_menu_test_set_force_negative_panel(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_size(&menu, &panel_width,
                                                  &panel_height, &has_label),
                  CMP_ERR_RANGE);

  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 50.0f;
  overlay.height = 40.0f;
  menu.anchor.type = M3_MENU_ANCHOR_RECT;
  menu.anchor.rect.x = 5.0f;
  menu.anchor.rect.y = 5.0f;
  menu.anchor.rect.width = 10.0f;
  menu.anchor.rect.height = 10.0f;
  menu.placement.direction = M3_MENU_DIRECTION_DOWN;
  menu.placement.align = M3_MENU_ALIGN_START;

  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(
                      NULL, &overlay, 10.0f, 10.0f, &panel_bounds, &direction),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(&menu, NULL, 10.0f, 10.0f,
                                                    &panel_bounds, &direction),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(&menu, &overlay, 10.0f,
                                                    10.0f, NULL, &direction),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(&menu, &overlay, 10.0f,
                                                    10.0f, &panel_bounds, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  overlay.width = -1.0f;
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(
                      &menu, &overlay, 10.0f, 10.0f, &panel_bounds, &direction),
                  CMP_ERR_RANGE);
  overlay.width = 50.0f;

  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(
                      &menu, &overlay, -1.0f, 10.0f, &panel_bounds, &direction),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_menu_test_compute_panel_bounds(
                      &menu, &overlay, 10.0f, -1.0f, &panel_bounds, &direction),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_menu_test_compute_panel_bounds(&menu, &overlay, 60.0f, 60.0f,
                                                &panel_bounds, &direction));
  overlay.width = 10.0f;
  overlay.height = 10.0f;
  CMP_TEST_OK(m3_menu_test_set_force_bounds_overflow(CMP_TRUE));
  CMP_TEST_OK(m3_menu_test_compute_panel_bounds(&menu, &overlay, 60.0f, 60.0f,
                                                &panel_bounds, &direction));
  CMP_TEST_OK(m3_menu_test_compute_panel_bounds(&menu, &overlay, 60.0f, 60.0f,
                                                &panel_bounds, &direction));
  CMP_TEST_ASSERT(panel_bounds.x == overlay.x);
  CMP_TEST_ASSERT(panel_bounds.y == overlay.y);
  overlay.width = 1.0f;
  overlay.height = 1.0f;
  CMP_TEST_OK(m3_menu_test_compute_panel_bounds(&menu, &overlay, 20.0f, 20.0f,
                                                &panel_bounds, &direction));
  CMP_TEST_ASSERT(panel_bounds.x == overlay.x);
  CMP_TEST_ASSERT(panel_bounds.y == overlay.y);
  overlay.width = 50.0f;
  overlay.height = 40.0f;

  overlay.x = 0.0f;
  overlay.y = 0.0f;
  overlay.width = 200.0f;
  overlay.height = 200.0f;
  menu.anchor.type = M3_MENU_ANCHOR_RECT;
  menu.anchor.rect.x = 80.0f;
  menu.anchor.rect.y = 80.0f;
  menu.anchor.rect.width = 20.0f;
  menu.anchor.rect.height = 20.0f;
  menu.placement.direction = M3_MENU_DIRECTION_DOWN;
  menu.placement.align = M3_MENU_ALIGN_CENTER;
  CMP_TEST_OK(m3_menu_test_compute_panel_bounds(&menu, &overlay, 40.0f, 40.0f,
                                                &panel_bounds, &direction));
  CMP_TEST_ASSERT(panel_bounds.x > overlay.x);
  CMP_TEST_ASSERT(panel_bounds.y > overlay.y);

  menu.menu_bounds.x = 0.0f;
  menu.menu_bounds.y = 0.0f;
  menu.menu_bounds.width = 20.0f;
  menu.menu_bounds.height = 20.0f;
  CMP_TEST_EXPECT(m3_menu_test_hit_test(NULL, 0.0f, 0.0f, &inside, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_hit_test(&menu, 0.0f, 0.0f, NULL, &index),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_test_hit_test(&menu, 0.0f, 0.0f, &inside, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(m3_menu_test_set_force_hit_test_error(CMP_TRUE));
  CMP_TEST_EXPECT(m3_menu_test_hit_test(&menu, 1.0f, 1.0f, &inside, &index),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(m3_menu_test_hit_test(&menu, 100.0f, 100.0f, &inside, &index));
  CMP_TEST_ASSERT(inside == CMP_FALSE);
  CMP_TEST_ASSERT(index == M3_MENU_INVALID_INDEX);

  menu.style.padding.top = 10.0f;
  CMP_TEST_OK(m3_menu_test_hit_test(&menu, 1.0f, 0.0f, &inside, &index));
  CMP_TEST_ASSERT(inside == CMP_TRUE);
  menu.style.item_height = 10.0f;
  menu.style.item_spacing = 0.0f;
  menu.menu_bounds.height = 50.0f;
  CMP_TEST_OK(m3_menu_test_hit_test(&menu, 1.0f, 45.0f, &inside, &index));
  CMP_TEST_ASSERT(inside == CMP_TRUE);
  CMP_TEST_ASSERT(index == M3_MENU_INVALID_INDEX);
  menu.menu_bounds.height = 20.0f;

  menu.style.padding.top = 0.0f;
  menu.style.item_height = 10.0f;
  menu.style.item_spacing = 5.0f;
  menu.menu_bounds.height = 40.0f;
  CMP_TEST_OK(m3_menu_test_hit_test(&menu, 1.0f, 12.0f, &inside, &index));
  CMP_TEST_ASSERT(inside == CMP_TRUE);
  CMP_TEST_ASSERT(index == M3_MENU_INVALID_INDEX);
  CMP_TEST_OK(m3_menu_test_hit_test(&menu, 1.0f, 5.0f, &inside, &index));
  CMP_TEST_ASSERT(inside == CMP_TRUE);
  CMP_TEST_ASSERT(index == 0u);
  menu.style.padding.top = menu_style.padding.top;

  return 0;
}

static int test_menu_init_and_setters(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3MenuStyle bad_style;
  M3Menu menu;
  M3MenuItem items[2];
  M3MenuItem bad_items[1];
  CMPRect rect;
  M3MenuPlacement placement;
  CMPSemantics semantics;
  CMPBool open;
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);

  items[0].utf8_label = "One";
  items[0].utf8_len = 3u;
  items[0].enabled = CMP_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = CMP_TRUE;

  CMP_TEST_EXPECT(m3_menu_init(NULL, &backend, &style, items, 2u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_init(&menu, NULL, &style, items, 2u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, NULL, items, 2u),
                  CMP_ERR_INVALID_ARGUMENT);

  backend.vtable = NULL;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                  CMP_ERR_INVALID_ARGUMENT);

  backend.vtable = &g_test_text_vtable_no_create;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                  CMP_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_destroy;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                  CMP_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                  CMP_ERR_UNSUPPORTED);
  backend.vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u),
                  CMP_ERR_UNSUPPORTED);

  backend.vtable = &g_test_text_vtable;
  bad_style = style;
  bad_style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &bad_style, items, 2u),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, NULL, 2u),
                  CMP_ERR_INVALID_ARGUMENT);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_menu_init(&menu, &backend, &style, items, 2u), CMP_ERR_IO);
  backend_state.fail_create = 0;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));
  CMP_TEST_ASSERT(menu.widget.flags & CMP_WIDGET_FLAG_FOCUSABLE);
  CMP_TEST_ASSERT((menu.widget.flags & CMP_WIDGET_FLAG_HIDDEN) == 0u);
  CMP_TEST_ASSERT(menu.open == CMP_TRUE);

  CMP_TEST_EXPECT(menu.widget.vtable->get_semantics(NULL, &semantics),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(menu.widget.vtable->get_semantics(menu.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(menu.widget.vtable->get_semantics(menu.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.flags & CMP_SEMANTIC_FLAG_FOCUSABLE);
  menu.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  CMP_TEST_OK(menu.widget.vtable->get_semantics(menu.widget.ctx, &semantics));
  CMP_TEST_ASSERT(semantics.flags & CMP_SEMANTIC_FLAG_DISABLED);
  menu.widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_DISABLED;

  bad_items[0].utf8_label = NULL;
  bad_items[0].utf8_len = 1u;
  bad_items[0].enabled = CMP_TRUE;
  CMP_TEST_EXPECT(m3_menu_set_items(&menu, bad_items, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  bad_items[0].utf8_label = "A";
  bad_items[0].utf8_len = 1u;
  bad_items[0].enabled = 2;
  CMP_TEST_EXPECT(m3_menu_set_items(&menu, bad_items, 1u), CMP_ERR_RANGE);
  CMP_TEST_OK(m3_menu_set_items(&menu, items, 2u));
  CMP_TEST_ASSERT(menu.item_count == 2u);

  CMP_TEST_EXPECT(m3_menu_set_items(NULL, items, 2u), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_menu_set_style(NULL, &style), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_set_style(&menu, NULL), CMP_ERR_INVALID_ARGUMENT);
  bad_style = style;
  bad_style.item_height = 0.0f;
  CMP_TEST_EXPECT(m3_menu_set_style(&menu, &bad_style), CMP_ERR_RANGE);

  backend_state.fail_create = 1;
  CMP_TEST_EXPECT(m3_menu_set_style(&menu, &style), CMP_ERR_IO);
  backend_state.fail_create = 0;

  backend_state.fail_destroy = 1;
  CMP_TEST_EXPECT(m3_menu_set_style(&menu, &style), CMP_ERR_IO);
  CMP_TEST_ASSERT(backend_state.destroy_calls >= 2);
  backend_state.fail_destroy = 0;

  CMP_TEST_OK(m3_menu_set_style(&menu, &style));

  CMP_TEST_EXPECT(m3_menu_set_on_action(NULL, test_action, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(m3_menu_set_anchor_rect(NULL, &rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_set_anchor_rect(&menu, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 1.0f;
  CMP_TEST_EXPECT(m3_menu_set_anchor_rect(&menu, &rect), CMP_ERR_RANGE);
  rect.width = 10.0f;
  rect.height = 5.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &rect));

  CMP_TEST_EXPECT(m3_menu_set_anchor_point(NULL, 0.0f, 0.0f),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_menu_set_anchor_point(&menu, 4.0f, 5.0f));

  CMP_TEST_EXPECT(m3_menu_set_placement(NULL, &placement),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_set_placement(&menu, NULL), CMP_ERR_INVALID_ARGUMENT);
  placement.direction = 99u;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_EXPECT(m3_menu_set_placement(&menu, &placement), CMP_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = 99u;
  CMP_TEST_EXPECT(m3_menu_set_placement(&menu, &placement), CMP_ERR_RANGE);
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_END;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));

  CMP_TEST_EXPECT(m3_menu_set_open(NULL, CMP_TRUE), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_set_open(&menu, 2), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_menu_set_open(&menu, CMP_FALSE));
  CMP_TEST_ASSERT(menu.widget.flags & CMP_WIDGET_FLAG_HIDDEN);
  CMP_TEST_OK(m3_menu_set_open(&menu, CMP_TRUE));
  CMP_TEST_ASSERT((menu.widget.flags & CMP_WIDGET_FLAG_HIDDEN) == 0u);

  CMP_TEST_EXPECT(m3_menu_get_open(NULL, &open), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_open(&menu, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_menu_get_open(&menu, &open));
  CMP_TEST_ASSERT(open == CMP_TRUE);

  CMP_TEST_EXPECT(m3_menu_get_bounds(NULL, &rect), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_bounds(&menu, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_overlay_bounds(NULL, &rect),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_overlay_bounds(&menu, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_resolved_direction(NULL, &placement.direction),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_resolved_direction(&menu, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_measure_layout(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  CMPMeasureSpec width;
  CMPMeasureSpec height;
  CMPSize size;
  CMPRect bounds;
  CMPRect anchor_rect;
  CMPRect out_bounds;
  CMPRect overlay_bounds;
  M3MenuPlacement placement;
  cmp_u32 resolved;
  CMPScalar expected_width;
  CMPScalar expected_height;
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);
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
  items[0].enabled = CMP_TRUE;
  items[1].utf8_label = "BBBB";
  items[1].utf8_len = 4u;
  items[1].enabled = CMP_TRUE;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  width.mode = CMP_MEASURE_UNSPECIFIED;
  width.size = 0.0f;
  height.mode = CMP_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  CMP_TEST_EXPECT(menu.widget.vtable->measure(NULL, width, height, &size),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, NULL),
      CMP_ERR_INVALID_ARGUMENT);

  width.mode = 99u;
  width.size = 0.0f;
  height.mode = CMP_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  CMP_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      CMP_ERR_INVALID_ARGUMENT);

  width.mode = CMP_MEASURE_EXACTLY;
  width.size = 10.0f;
  height.mode = 99u;
  height.size = 0.0f;
  CMP_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      CMP_ERR_INVALID_ARGUMENT);

  width.mode = CMP_MEASURE_AT_MOST;
  width.size = -1.0f;
  height.mode = CMP_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  CMP_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      CMP_ERR_RANGE);

  width.mode = CMP_MEASURE_UNSPECIFIED;
  width.size = 0.0f;
  height.mode = CMP_MEASURE_UNSPECIFIED;
  height.size = 0.0f;
  CMP_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  expected_width = 32.0f;
  expected_height = 40.0f;
  CMP_TEST_ASSERT(size.width == expected_width);
  CMP_TEST_ASSERT(size.height == expected_height);

  CMP_TEST_OK(m3_menu_test_set_force_negative_panel(CMP_TRUE));
  CMP_TEST_EXPECT(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size),
      CMP_ERR_RANGE);

  width.mode = CMP_MEASURE_AT_MOST;
  width.size = 10.0f;
  height.mode = CMP_MEASURE_AT_MOST;
  height.size = 10.0f;
  CMP_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  CMP_TEST_ASSERT(size.width == 10.0f);
  CMP_TEST_ASSERT(size.height == 10.0f);

  width.mode = CMP_MEASURE_EXACTLY;
  width.size = 50.0f;
  height.mode = CMP_MEASURE_EXACTLY;
  height.size = 60.0f;
  CMP_TEST_OK(
      menu.widget.vtable->measure(menu.widget.ctx, width, height, &size));
  CMP_TEST_ASSERT(size.width == 50.0f);
  CMP_TEST_ASSERT(size.height == 60.0f);

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  CMP_TEST_EXPECT(menu.widget.vtable->layout(NULL, bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(m3_menu_test_set_force_negative_panel(CMP_TRUE));
  CMP_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                  CMP_ERR_RANGE);
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  CMP_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  bounds.width = 100.0f;
  bounds.height = 100.0f;
  menu.anchor.type = 99u;
  CMP_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  anchor_rect.x = 10.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 20.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));

  menu.placement.direction = 99u;
  menu.placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  menu.placement.direction = M3_MENU_DIRECTION_DOWN;
  menu.placement.align = 99u;
  CMP_TEST_EXPECT(menu.widget.vtable->layout(menu.widget.ctx, bounds),
                  CMP_ERR_RANGE);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_OK(m3_menu_get_overlay_bounds(&menu, &overlay_bounds));
  CMP_TEST_ASSERT(overlay_bounds.width == bounds.width);
  CMP_TEST_ASSERT(overlay_bounds.height == bounds.height);
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);
  CMP_TEST_ASSERT(out_bounds.x == 10.0f);
  CMP_TEST_ASSERT(out_bounds.y == 20.0f);

  placement.direction = M3_MENU_DIRECTION_UP;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.y = 60.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);
  CMP_TEST_ASSERT(out_bounds.y == 20.0f);

  anchor_rect.y = 5.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);
  CMP_TEST_ASSERT(out_bounds.y == 15.0f);

  anchor_rect.y = 80.0f;
  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);
  CMP_TEST_ASSERT(out_bounds.y == 40.0f);

  placement.direction = M3_MENU_DIRECTION_RIGHT;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 10.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);
  CMP_TEST_ASSERT(out_bounds.x == 20.0f);

  anchor_rect.x = 80.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);
  CMP_TEST_ASSERT(out_bounds.x == 48.0f);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_CENTER;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 50.0f;
  anchor_rect.y = 50.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);
  CMP_TEST_ASSERT(out_bounds.y == 35.0f);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_END;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 5.0f;
  anchor_rect.y = 50.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);
  CMP_TEST_ASSERT(out_bounds.y == 20.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_CENTER;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 0.0f;
  anchor_rect.y = 10.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(out_bounds.x == 0.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_END;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 50.0f;
  anchor_rect.width = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(out_bounds.x == 28.0f);

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  CMP_TEST_OK(m3_menu_set_anchor_point(&menu, 5.0f, 5.0f));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(out_bounds.x == 5.0f);

  menu.style.item_height = 60.0f;
  menu.style.item_spacing = 0.0f;
  menu.style.min_width = 100.0f;
  bounds.width = 50.0f;
  bounds.height = 50.0f;

  placement.direction = M3_MENU_DIRECTION_DOWN;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 10.0f;
  anchor_rect.y = 30.0f;
  anchor_rect.width = 10.0f;
  anchor_rect.height = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_UP);

  placement.direction = M3_MENU_DIRECTION_UP;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.y = 5.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_DOWN);

  placement.direction = M3_MENU_DIRECTION_RIGHT;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 30.0f;
  anchor_rect.y = 10.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_LEFT);

  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  anchor_rect.x = 5.0f;
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_resolved_direction(&menu, &resolved));
  CMP_TEST_ASSERT(resolved == M3_MENU_DIRECTION_RIGHT);

  anchor_rect.x = 110.0f;
  placement.direction = M3_MENU_DIRECTION_LEFT;
  placement.align = M3_MENU_ALIGN_START;
  CMP_TEST_OK(m3_menu_set_placement(&menu, &placement));
  CMP_TEST_OK(m3_menu_set_anchor_rect(&menu, &anchor_rect));
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(out_bounds.x == 0.0f);

  bounds.width = 20.0f;
  bounds.height = 30.0f;
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));
  CMP_TEST_OK(m3_menu_get_bounds(&menu, &out_bounds));
  CMP_TEST_ASSERT(out_bounds.width == 20.0f);
  CMP_TEST_ASSERT(out_bounds.height == 30.0f);

  CMP_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_item_bounds(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  CMPRect bounds;
  CMPRect item_bounds;
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);
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
  items[0].enabled = CMP_TRUE;
  items[1].utf8_label = "B";
  items[1].utf8_len = 1u;
  items[1].enabled = CMP_TRUE;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 50.0f;
  bounds.height = 50.0f;
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  CMP_TEST_EXPECT(m3_menu_get_item_bounds(NULL, 0u, &item_bounds),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 0u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 5u, &item_bounds),
                  CMP_ERR_RANGE);

  menu.style.item_height = 0.0f;
  menu.style.item_spacing = 0.0f;
  CMP_TEST_EXPECT(m3_menu_get_item_bounds(&menu, 0u, &item_bounds),
                  CMP_ERR_RANGE);
  menu.style.item_height = 10.0f;

  CMP_TEST_OK(m3_menu_get_item_bounds(&menu, 1u, &item_bounds));
  CMP_TEST_ASSERT(item_bounds.y ==
                  menu.menu_bounds.y + menu.style.padding.top + 10.0f);

  CMP_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_events(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  CMPRect bounds;
  CMPInputEvent event;
  CMPBool handled;
  TestActionState action;
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);
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
  items[0].enabled = CMP_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = CMP_FALSE;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  memset(&event, 0, sizeof(event));
  handled = CMP_FALSE;
  CMP_TEST_EXPECT(menu.widget.vtable->event(NULL, &event, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, NULL, &handled),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  bounds.x = 10.0f;
  bounds.y = 10.0f;
  bounds.width = 80.0f;
  bounds.height = 80.0f;
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  action.calls = 0;
  action.last_action = 0u;
  action.last_index = M3_MENU_INVALID_INDEX;
  action.fail_next = 0;
  CMP_TEST_OK(m3_menu_set_on_action(&menu, test_action, &action));

  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x - 5.0f),
                          (cmp_i32)(menu.menu_bounds.y - 5.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(action.calls == 1);
  CMP_TEST_ASSERT(action.last_action == M3_MENU_ACTION_DISMISS);
  CMP_TEST_ASSERT(action.last_index == M3_MENU_INVALID_INDEX);

  action.fail_next = 1;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x - 2.0f),
                          (cmp_i32)(menu.menu_bounds.y - 2.0f));
  handled = CMP_FALSE;
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                  CMP_ERR_IO);

  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(menu.pressed_index == 0u);

  test_make_pointer_event(&event, CMP_INPUT_POINTER_UP,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  CMP_TEST_ASSERT(action.last_action == M3_MENU_ACTION_SELECT);
  CMP_TEST_ASSERT(action.last_index == 0u);

  action.fail_next = 1;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  test_make_pointer_event(&event, CMP_INPUT_POINTER_UP,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                  CMP_ERR_IO);

  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 15.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(menu.pressed_index == M3_MENU_INVALID_INDEX);

  test_make_pointer_event(&event, CMP_INPUT_POINTER_UP,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 15.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));

  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 11.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(menu.pressed_index == M3_MENU_INVALID_INDEX);

  menu.item_count = 0u;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);
  menu.item_count = 2u;

  menu.style.item_height = 0.0f;
  menu.style.item_spacing = 0.0f;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  menu.style.item_height = 10.0f;

  test_make_pointer_event(&event, CMP_INPUT_POINTER_MOVE,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  test_make_pointer_event(&event, CMP_INPUT_POINTER_SCROLL,
                          (cmp_i32)(menu.menu_bounds.x + 1.0f),
                          (cmp_i32)(menu.menu_bounds.y + 1.0f));
  handled = CMP_FALSE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_TRUE);

  menu.widget.flags |= CMP_WIDGET_FLAG_HIDDEN;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0);
  handled = CMP_TRUE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  menu.widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_HIDDEN;

  menu.widget.flags |= CMP_WIDGET_FLAG_DISABLED;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0);
  handled = CMP_TRUE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);
  menu.widget.flags &= (cmp_u32)~CMP_WIDGET_FLAG_DISABLED;

  menu.menu_bounds.width = -1.0f;
  test_make_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0);
  handled = CMP_FALSE;
  CMP_TEST_EXPECT(menu.widget.vtable->event(menu.widget.ctx, &event, &handled),
                  CMP_ERR_RANGE);
  menu.menu_bounds.width = 10.0f;

  memset(&event, 0, sizeof(event));
  event.type = CMP_INPUT_KEY_DOWN;
  handled = CMP_TRUE;
  CMP_TEST_OK(menu.widget.vtable->event(menu.widget.ctx, &event, &handled));
  CMP_TEST_ASSERT(handled == CMP_FALSE);

  CMP_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_paint(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[2];
  M3MenuItem empty_items[1];
  CMPRect bounds;
  CMPPaintContext ctx;
  CMPGfx gfx;
  CMPGfx bad_gfx;
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);
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
  items[0].enabled = CMP_TRUE;
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;
  items[1].enabled = CMP_FALSE;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 2u));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 100.0f;
  bounds.height = 100.0f;
  CMP_TEST_OK(menu.widget.vtable->layout(menu.widget.ctx, bounds));

  gfx.ctx = &backend_state;
  gfx.vtable = &g_test_gfx_vtable;
  gfx.text_vtable = &g_test_text_vtable;

  ctx.gfx = &gfx;
  ctx.clip = bounds;
  ctx.dpi_scale = 1.0f;

  CMP_TEST_EXPECT(menu.widget.vtable->paint(NULL, &ctx),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  bad_gfx.ctx = &backend_state;
  bad_gfx.vtable = &g_test_gfx_vtable_no_draw;
  bad_gfx.text_vtable = &g_test_text_vtable;
  ctx.gfx = &bad_gfx;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);

  ctx.gfx = &gfx;
  menu.style.item_height = 0.0f;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  menu.style.item_height = 20.0f;

  menu.menu_bounds.width = -1.0f;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  menu.menu_bounds.width = 50.0f;

  menu.text_backend.vtable = &g_test_text_vtable_no_measure;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  menu.text_backend.vtable = &g_test_text_vtable;

  menu.font.id = 0u;
  menu.font.generation = 0u;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_STATE);
  menu.font.id = 1u;
  menu.font.generation = 1u;

  backend_state.negative_width = 1;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  backend_state.negative_width = 0;

  backend_state.fail_draw_rect = 1;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx), CMP_ERR_IO);
  backend_state.fail_draw_rect = 0;

  menu.style.shadow_enabled = CMP_TRUE;
  menu.style.shadow.layers = 0u;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_RANGE);
  menu.style.shadow.layers = 1u;

  ctx.gfx->text_vtable = &g_test_text_vtable_no_draw;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx),
                  CMP_ERR_UNSUPPORTED);
  ctx.gfx->text_vtable = &g_test_text_vtable;

  menu.style.disabled_text_color.r = 0.25f;
  menu.style.disabled_text_color.g = 0.5f;
  menu.style.disabled_text_color.b = 0.75f;
  menu.style.disabled_text_color.a = 1.0f;

  CMP_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  CMP_TEST_ASSERT(backend_state.last_text_color.r ==
                  menu.style.disabled_text_color.r);

  backend_state.fail_draw = 1;
  CMP_TEST_EXPECT(menu.widget.vtable->paint(menu.widget.ctx, &ctx), CMP_ERR_IO);
  backend_state.fail_draw = 0;

  items[1].utf8_label = NULL;
  items[1].utf8_len = 0u;
  CMP_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  items[1].utf8_label = "Two";
  items[1].utf8_len = 3u;

  menu.style.shadow.color.a = 0.5f;
  menu.style.shadow.layers = 1u;
  backend_state.push_clip_calls = 0;
  backend_state.pop_clip_calls = 0;
  CMP_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  CMP_TEST_ASSERT(backend_state.push_clip_calls > 0);
  CMP_TEST_ASSERT(backend_state.pop_clip_calls > 0);

  empty_items[0].utf8_label = NULL;
  empty_items[0].utf8_len = 0u;
  empty_items[0].enabled = CMP_TRUE;
  CMP_TEST_OK(m3_menu_set_items(&menu, empty_items, 1u));
  ctx.gfx->text_vtable = NULL;
  CMP_TEST_OK(menu.widget.vtable->paint(menu.widget.ctx, &ctx));
  ctx.gfx->text_vtable = &g_test_text_vtable;

  CMP_TEST_OK(menu.widget.vtable->destroy(menu.widget.ctx));
  return 0;
}

static int test_menu_destroy(void) {
  TestMenuBackend backend_state;
  CMPTextBackend backend;
  M3MenuStyle style;
  M3Menu menu;
  M3MenuItem items[1];
  int rc;

  rc = test_backend_init(&backend_state);
  CMP_TEST_OK(rc);
  backend.ctx = &backend_state;
  backend.vtable = &g_test_text_vtable;

  rc = test_menu_style_init_helper(&style);
  CMP_TEST_OK(rc);

  items[0].utf8_label = "A";
  items[0].utf8_len = 1u;
  items[0].enabled = CMP_TRUE;

  CMP_TEST_OK(m3_menu_init(&menu, &backend, &style, items, 1u));

  CMP_TEST_EXPECT(menu.widget.vtable->destroy(NULL), CMP_ERR_INVALID_ARGUMENT);

  menu.text_backend.vtable = &g_test_text_vtable_no_destroy;
  menu.owns_font = CMP_TRUE;
  menu.font.id = 1u;
  menu.font.generation = 1u;
  CMP_TEST_EXPECT(menu.widget.vtable->destroy(menu.widget.ctx),
                  CMP_ERR_UNSUPPORTED);

  return 0;
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
    NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL};

int main(void) {
  CMP_TEST_OK(test_menu_validation_helpers());
  CMP_TEST_OK(test_menu_style_init_failures());
  CMP_TEST_OK(test_menu_init_and_setters());
  CMP_TEST_OK(test_menu_measure_layout());
  CMP_TEST_OK(test_menu_item_bounds());
  CMP_TEST_OK(test_menu_events());
  CMP_TEST_OK(test_menu_paint());
  CMP_TEST_OK(test_menu_destroy());
  return 0;
}
