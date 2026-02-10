#include "m3/m3_selection.h"
#include "test_utils.h"

#include <string.h>

#ifdef M3_TESTING
#define M3_SELECTION_TEST_FAIL_CHECKBOX_RESOLVE_COLORS 1u
#define M3_SELECTION_TEST_FAIL_SWITCH_RESOLVE_COLORS 2u
#define M3_SELECTION_TEST_FAIL_RADIO_RESOLVE_COLORS 3u
#define M3_SELECTION_TEST_FAIL_CHECKBOX_CHECK_THICKNESS_NEGATIVE 4u
#define M3_SELECTION_TEST_FAIL_RADIO_DOT_RADIUS_NEGATIVE 5u
#define M3_SELECTION_TEST_FAIL_RADIO_INNER_CORNER_NEGATIVE 6u
#endif

typedef struct TestSelectionBackend {
  int draw_rect_calls;
  int draw_line_calls;
  int fail_draw_rect;
  int fail_draw_line;
  int fail_draw_rect_after;
  int fail_draw_line_after;
  M3Rect last_rect;
  M3Color last_rect_color;
  M3Scalar last_corner;
  M3Scalar last_line_x0;
  M3Scalar last_line_y0;
  M3Scalar last_line_x1;
  M3Scalar last_line_y1;
  M3Color last_line_color;
  M3Scalar last_line_thickness;
} TestSelectionBackend;

typedef struct ToggleCounter {
  int calls;
  M3Bool last_state;
  int fail;
} ToggleCounter;

static int test_backend_init(TestSelectionBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(backend, 0, sizeof(*backend));
  backend->fail_draw_rect = M3_OK;
  backend->fail_draw_line = M3_OK;
  backend->fail_draw_rect_after = 0;
  backend->fail_draw_line_after = 0;
  return M3_OK;
}

static int test_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                              M3Scalar corner_radius) {
  TestSelectionBackend *backend;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestSelectionBackend *)gfx;
  backend->draw_rect_calls += 1;
  backend->last_rect = *rect;
  backend->last_rect_color = color;
  backend->last_corner = corner_radius;
  if (backend->fail_draw_rect_after > 0 &&
      backend->draw_rect_calls == backend->fail_draw_rect_after) {
    return M3_ERR_IO;
  }
  if (backend->fail_draw_rect != M3_OK) {
    return backend->fail_draw_rect;
  }
  return M3_OK;
}

static int test_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1,
                              M3Scalar y1, M3Color color, M3Scalar thickness) {
  TestSelectionBackend *backend;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (TestSelectionBackend *)gfx;
  backend->draw_line_calls += 1;
  backend->last_line_x0 = x0;
  backend->last_line_y0 = y0;
  backend->last_line_x1 = x1;
  backend->last_line_y1 = y1;
  backend->last_line_color = color;
  backend->last_line_thickness = thickness;
  if (backend->fail_draw_line_after > 0 &&
      backend->draw_line_calls == backend->fail_draw_line_after) {
    return M3_ERR_IO;
  }
  if (backend->fail_draw_line != M3_OK) {
    return backend->fail_draw_line;
  }
  return M3_OK;
}

static const M3GfxVTable g_test_vtable = {NULL,
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
                                          NULL};

static const M3GfxVTable g_test_vtable_no_rect = {
    NULL, NULL, NULL, NULL, test_gfx_draw_line, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL};

static const M3GfxVTable g_test_vtable_no_line = {
    NULL, NULL, NULL, test_gfx_draw_rect, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL};

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

static int test_checkbox_on_change(void *ctx, M3Checkbox *checkbox,
                                   M3Bool checked) {
  ToggleCounter *counter;

  if (ctx == NULL || checkbox == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ToggleCounter *)ctx;
  counter->calls += 1;
  counter->last_state = checked;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_switch_on_change(void *ctx, M3Switch *widget, M3Bool on) {
  ToggleCounter *counter;

  if (ctx == NULL || widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ToggleCounter *)ctx;
  counter->calls += 1;
  counter->last_state = on;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_radio_on_change(void *ctx, M3Radio *radio, M3Bool selected) {
  ToggleCounter *counter;

  if (ctx == NULL || radio == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  counter = (ToggleCounter *)ctx;
  counter->calls += 1;
  counter->last_state = selected;
  if (counter->fail) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

int main(void) {
  TestSelectionBackend backend;
  M3Gfx gfx;
  M3PaintContext paint_ctx;
  M3CheckboxStyle checkbox_style;
  M3CheckboxStyle other_checkbox_style;
  M3Checkbox checkbox;
  M3SwitchStyle switch_style;
  M3SwitchStyle other_switch_style;
  M3Switch sw;
  M3RadioStyle radio_style;
  M3RadioStyle other_radio_style;
  M3Radio radio;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;
  M3Bool state;
  M3Semantics semantics;
  ToggleCounter counter;

  gfx.ctx = &backend;
  gfx.vtable = &g_test_vtable;
  gfx.text_vtable = NULL;

  paint_ctx.gfx = &gfx;
  paint_ctx.dpi_scale = 1.0f;
  paint_ctx.clip.x = 0.0f;
  paint_ctx.clip.y = 0.0f;
  paint_ctx.clip.width = 100.0f;
  paint_ctx.clip.height = 100.0f;

  M3_TEST_EXPECT(m3_checkbox_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_checkbox_style_init(&checkbox_style));
  M3_TEST_ASSERT(
      m3_near(checkbox_style.size, M3_CHECKBOX_DEFAULT_SIZE, 0.001f));

  M3_TEST_EXPECT(m3_checkbox_init(NULL, &checkbox_style, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_init(&checkbox, NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_init(&checkbox, &checkbox_style, 2),
                 M3_ERR_INVALID_ARGUMENT);

  other_checkbox_style = checkbox_style;
  other_checkbox_style.size = 0.0f;
  M3_TEST_EXPECT(m3_checkbox_init(&checkbox, &other_checkbox_style, M3_FALSE),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_checkbox_init(&checkbox, &checkbox_style, M3_FALSE));
  M3_TEST_ASSERT(checkbox.widget.ctx == &checkbox);
  M3_TEST_ASSERT(checkbox.widget.vtable != NULL);
  M3_TEST_EXPECT(m3_checkbox_set_style(NULL, &checkbox_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_set_style(&checkbox, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_checkbox_set_checked(NULL, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_set_checked(&checkbox, 2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_TRUE));
  M3_TEST_OK(m3_checkbox_get_checked(&checkbox, &state));
  M3_TEST_ASSERT(state == M3_TRUE);
  M3_TEST_EXPECT(m3_checkbox_get_checked(NULL, &state),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_get_checked(&checkbox, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_checkbox_set_label(NULL, "A", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_checkbox_set_label(&checkbox, NULL, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_checkbox_set_label(&checkbox, "Check", 5));

  other_checkbox_style = checkbox_style;
  other_checkbox_style.checked.fill.a = 2.0f;
  M3_TEST_EXPECT(m3_checkbox_set_style(&checkbox, &other_checkbox_style),
                 M3_ERR_RANGE);
  other_checkbox_style = checkbox_style;
  other_checkbox_style.border_width = -1.0f;
  M3_TEST_EXPECT(m3_checkbox_set_style(&checkbox, &other_checkbox_style),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_checkbox_set_style(&checkbox, &checkbox_style));

#ifdef M3_TESTING
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_CHECKBOX_RESOLVE_COLORS));
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_selection_test_clear_fail_points());
#endif

  M3_TEST_EXPECT(m3_checkbox_set_on_change(NULL, test_checkbox_on_change, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  counter.calls = 0;
  counter.last_state = M3_FALSE;
  counter.fail = 0;
  M3_TEST_OK(
      m3_checkbox_set_on_change(&checkbox, test_checkbox_on_change, &counter));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      checkbox.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(checkbox.widget.vtable->measure(checkbox.widget.ctx,
                                                 width_spec, height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(checkbox.widget.vtable->measure(
                     checkbox.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  checkbox.style.size = -1.0f;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->measure(
                     checkbox.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);
  checkbox.style.size = checkbox_style.size;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->measure(
                     checkbox.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  height_spec.mode = M3_MEASURE_UNSPECIFIED;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->measure(
                     checkbox.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(checkbox.widget.vtable->measure(checkbox.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, checkbox_style.size, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, checkbox_style.size, 0.001f));

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 12.0f;
  M3_TEST_OK(checkbox.widget.vtable->measure(checkbox.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, 10.0f, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, 12.0f, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->layout(checkbox.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 20.0f;
  bounds.height = 20.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(checkbox.widget.vtable->layout(checkbox.widget.ctx, bounds));

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  gfx.vtable = NULL;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable_no_line;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

  M3_TEST_OK(m3_checkbox_set_style(&checkbox, &checkbox_style));
  checkbox.style.border_width = 15.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  checkbox.style.border_width = checkbox_style.border_width;

  checkbox.style.corner_radius = 0.5f;
  checkbox.style.border_width = 2.0f;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx));
  checkbox.style.corner_radius = checkbox_style.corner_radius;
  checkbox.style.border_width = checkbox_style.border_width;

  checkbox.bounds.width = 2.0f;
  checkbox.bounds.height = 2.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  checkbox.bounds = bounds;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 2;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

#ifdef M3_TESTING
  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_TRUE));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_CHECKBOX_CHECK_THICKNESS_NEGATIVE));
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_selection_test_clear_fail_points());
#endif

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_line_after = 2;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_line_after = 0;

  checkbox.bounds.width = -1.0f;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  checkbox.bounds = bounds;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(test_backend_init(&backend));

  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_FALSE));
  M3_TEST_OK(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);
  M3_TEST_ASSERT(backend.draw_line_calls == 0);

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_TRUE));
  M3_TEST_OK(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);
  M3_TEST_ASSERT(backend.draw_line_calls == 2);
  M3_TEST_ASSERT(m3_color_near(backend.last_line_color,
                               checkbox_style.checked.mark, 0.001f));

  checkbox.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               checkbox_style.disabled_checked.fill, 0.001f));
  checkbox.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_FALSE));
  checkbox.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               checkbox_style.disabled_unchecked.fill, 0.001f));
  checkbox.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_TRUE));
  backend.fail_draw_line = M3_ERR_IO;
  M3_TEST_EXPECT(checkbox.widget.vtable->paint(checkbox.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_line = M3_OK;

  M3_TEST_EXPECT(checkbox.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      checkbox.widget.vtable->event(checkbox.widget.ctx, NULL, &handled),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, NULL),
      M3_ERR_INVALID_ARGUMENT);

  checkbox.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  checkbox.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  checkbox.checked = 2;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled),
      M3_ERR_INVALID_ARGUMENT);
  checkbox.checked = M3_FALSE;

  checkbox.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled),
      M3_ERR_STATE);
  checkbox.pressed = M3_FALSE;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_UP, 0, 0));
  M3_TEST_OK(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_checkbox_set_checked(&checkbox, M3_FALSE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 1);
  M3_TEST_ASSERT(counter.last_state == M3_TRUE);

  counter.fail = 1;
  checkbox.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_EXPECT(
      checkbox.widget.vtable->event(checkbox.widget.ctx, &event, &handled),
      M3_ERR_IO);
  M3_TEST_ASSERT(checkbox.checked == M3_TRUE);
  counter.fail = 0;

  M3_TEST_EXPECT(checkbox.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      checkbox.widget.vtable->get_semantics(checkbox.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_CHECKBOX);
  M3_TEST_ASSERT(semantics.utf8_label == checkbox.utf8_label);

  checkbox.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      checkbox.widget.vtable->get_semantics(checkbox.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  checkbox.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(checkbox.widget.vtable->destroy(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(checkbox.widget.vtable->destroy(checkbox.widget.ctx));

  M3_TEST_EXPECT(m3_switch_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_switch_style_init(&switch_style));
  M3_TEST_ASSERT(
      m3_near(switch_style.track_width, M3_SWITCH_DEFAULT_TRACK_WIDTH, 0.001f));

  M3_TEST_EXPECT(m3_switch_init(NULL, &switch_style, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_init(&sw, NULL, M3_FALSE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_init(&sw, &switch_style, 2),
                 M3_ERR_INVALID_ARGUMENT);

  other_switch_style = switch_style;
  other_switch_style.track_width = 10.0f;
  other_switch_style.track_height = 20.0f;
  M3_TEST_EXPECT(m3_switch_init(&sw, &other_switch_style, M3_FALSE),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_switch_init(&sw, &switch_style, M3_FALSE));
  M3_TEST_ASSERT(sw.widget.ctx == &sw);
  M3_TEST_EXPECT(m3_switch_set_style(NULL, &switch_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_set_style(&sw, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_switch_set_on(NULL, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_set_on(&sw, 3), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_switch_set_on(&sw, M3_TRUE));
  M3_TEST_OK(m3_switch_get_on(&sw, &state));
  M3_TEST_ASSERT(state == M3_TRUE);
  M3_TEST_EXPECT(m3_switch_get_on(NULL, &state), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_get_on(&sw, NULL), M3_ERR_INVALID_ARGUMENT);

  other_switch_style = switch_style;
  other_switch_style.thumb_inset = -1.0f;
  M3_TEST_EXPECT(m3_switch_set_style(&sw, &other_switch_style), M3_ERR_RANGE);
  other_switch_style = switch_style;
  other_switch_style.on.track.a = 2.0f;
  M3_TEST_EXPECT(m3_switch_set_style(&sw, &other_switch_style), M3_ERR_RANGE);
  other_switch_style = switch_style;
  other_switch_style.track_height = 20.0f;
  other_switch_style.track_width = 20.0f;
  other_switch_style.thumb_inset = 10.0f;
  M3_TEST_EXPECT(m3_switch_set_style(&sw, &other_switch_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_switch_set_style(&sw, &switch_style));

  M3_TEST_EXPECT(m3_switch_set_label(NULL, "S", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_switch_set_label(&sw, NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_switch_set_label(&sw, "Switch", 6));

  M3_TEST_EXPECT(m3_switch_set_on_change(NULL, test_switch_on_change, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_switch_set_on_change(&sw, test_switch_on_change, &counter));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);

  sw.style.track_width = -1.0f;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, &size),
      M3_ERR_RANGE);
  sw.style.track_width = switch_style.track_width;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  height_spec.mode = M3_MEASURE_UNSPECIFIED;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, &size),
      M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(
      sw.widget.vtable->measure(sw.widget.ctx, width_spec, height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, switch_style.track_width, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, switch_style.track_height, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(sw.widget.vtable->layout(sw.widget.ctx, bounds), M3_ERR_RANGE);

  bounds.width = switch_style.track_width;
  bounds.height = switch_style.track_height;
  M3_TEST_EXPECT(sw.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(sw.widget.vtable->layout(sw.widget.ctx, bounds));

  M3_TEST_EXPECT(sw.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

#ifdef M3_TESTING
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_SWITCH_RESOLVE_COLORS));
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_selection_test_clear_fail_points());
#endif

  M3_TEST_OK(m3_switch_set_style(&sw, &switch_style));
  sw.style.thumb_inset = 30.0f;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  sw.style.thumb_inset = switch_style.thumb_inset;

  sw.bounds.height = 2.0f;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  sw.bounds = bounds;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx), M3_ERR_IO);
  M3_TEST_OK(test_backend_init(&backend));

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 2;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx), M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

  sw.bounds.width = -1.0f;
  M3_TEST_EXPECT(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  sw.bounds = bounds;

  M3_TEST_OK(test_backend_init(&backend));
  sw.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  sw.on = M3_FALSE;
  M3_TEST_OK(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx));
  sw.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_switch_set_on(&sw, M3_FALSE));
  M3_TEST_OK(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);
  M3_TEST_ASSERT(
      m3_color_near(backend.last_rect_color, switch_style.off.thumb, 0.001f));

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_switch_set_on(&sw, M3_TRUE));
  M3_TEST_OK(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);
  M3_TEST_ASSERT(
      m3_color_near(backend.last_rect_color, switch_style.on.thumb, 0.001f));

  sw.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               switch_style.disabled_off.thumb, 0.001f));
  sw.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  sw.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_switch_set_on(&sw, M3_TRUE));
  M3_TEST_OK(sw.widget.vtable->paint(sw.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               switch_style.disabled_on.thumb, 0.001f));
  sw.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(sw.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(sw.widget.vtable->event(sw.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(sw.widget.vtable->event(sw.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  sw.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(sw.widget.vtable->event(sw.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  sw.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  sw.on = 2;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(sw.widget.vtable->event(sw.widget.ctx, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  sw.on = M3_FALSE;

  sw.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(sw.widget.vtable->event(sw.widget.ctx, &event, &handled),
                 M3_ERR_STATE);
  sw.pressed = M3_FALSE;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_UP, 0, 0));
  M3_TEST_OK(sw.widget.vtable->event(sw.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(sw.widget.vtable->event(sw.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_switch_set_on(&sw, M3_FALSE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(sw.widget.vtable->event(sw.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(sw.widget.vtable->event(sw.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 1);
  M3_TEST_ASSERT(counter.last_state == M3_TRUE);

  counter.fail = 1;
  sw.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_EXPECT(sw.widget.vtable->event(sw.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_ASSERT(sw.on == M3_TRUE);
  counter.fail = 0;

  M3_TEST_EXPECT(sw.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(sw.widget.vtable->get_semantics(sw.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_SWITCH);

  sw.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(sw.widget.vtable->get_semantics(sw.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  sw.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(sw.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(sw.widget.vtable->destroy(sw.widget.ctx));

  M3_TEST_EXPECT(m3_radio_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_radio_style_init(&radio_style));
  M3_TEST_ASSERT(m3_near(radio_style.size, M3_RADIO_DEFAULT_SIZE, 0.001f));

  M3_TEST_EXPECT(m3_radio_init(NULL, &radio_style, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_init(&radio, NULL, M3_FALSE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_init(&radio, &radio_style, 2),
                 M3_ERR_INVALID_ARGUMENT);

  other_radio_style = radio_style;
  other_radio_style.dot_radius = 20.0f;
  M3_TEST_EXPECT(m3_radio_init(&radio, &other_radio_style, M3_FALSE),
                 M3_ERR_RANGE);

  M3_TEST_OK(m3_radio_init(&radio, &radio_style, M3_FALSE));
  M3_TEST_ASSERT(radio.widget.ctx == &radio);
  M3_TEST_EXPECT(m3_radio_set_style(NULL, &radio_style),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_set_style(&radio, NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_radio_set_selected(NULL, M3_TRUE), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_set_selected(&radio, 3), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_TRUE));
  M3_TEST_OK(m3_radio_get_selected(&radio, &state));
  M3_TEST_ASSERT(state == M3_TRUE);
  M3_TEST_EXPECT(m3_radio_get_selected(NULL, &state), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_get_selected(&radio, NULL), M3_ERR_INVALID_ARGUMENT);

  other_radio_style = radio_style;
  other_radio_style.checked.mark.a = 2.0f;
  M3_TEST_EXPECT(m3_radio_set_style(&radio, &other_radio_style), M3_ERR_RANGE);
  other_radio_style = radio_style;
  other_radio_style.border_width = -1.0f;
  M3_TEST_EXPECT(m3_radio_set_style(&radio, &other_radio_style), M3_ERR_RANGE);
  M3_TEST_OK(m3_radio_set_style(&radio, &radio_style));

  M3_TEST_EXPECT(m3_radio_set_label(NULL, "R", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_radio_set_label(&radio, NULL, 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_radio_set_label(&radio, "Radio", 5));

  M3_TEST_EXPECT(m3_radio_set_on_change(NULL, test_radio_on_change, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_radio_set_on_change(&radio, test_radio_on_change, &counter));

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(
      radio.widget.vtable->measure(NULL, width_spec, height_spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                              height_spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                              height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  radio.style.size = -1.0f;
  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                              height_spec, &size),
                 M3_ERR_RANGE);
  radio.style.size = radio_style.size;

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = 99u;
  height_spec.size = 0.0f;
  M3_TEST_EXPECT(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                              height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  height_spec.mode = M3_MEASURE_UNSPECIFIED;

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = -1.0f;
  M3_TEST_EXPECT(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                              height_spec, &size),
                 M3_ERR_RANGE);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(radio.widget.vtable->measure(radio.widget.ctx, width_spec,
                                          height_spec, &size));
  M3_TEST_ASSERT(m3_near(size.width, radio_style.size, 0.001f));
  M3_TEST_ASSERT(m3_near(size.height, radio_style.size, 0.001f));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = -1.0f;
  bounds.height = 10.0f;
  M3_TEST_EXPECT(radio.widget.vtable->layout(radio.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = radio_style.size;
  bounds.height = radio_style.size;
  M3_TEST_EXPECT(radio.widget.vtable->layout(NULL, bounds),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(radio.widget.vtable->layout(radio.widget.ctx, bounds));

  M3_TEST_EXPECT(radio.widget.vtable->paint(NULL, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = NULL;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_INVALID_ARGUMENT);
  paint_ctx.gfx = &gfx;

  gfx.vtable = &g_test_vtable_no_rect;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_UNSUPPORTED);
  gfx.vtable = &g_test_vtable;

#ifdef M3_TESTING
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_RADIO_RESOLVE_COLORS));
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_selection_test_clear_fail_points());
#endif

  M3_TEST_OK(m3_radio_set_style(&radio, &radio_style));
  radio.style.border_width = 15.0f;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  radio.style.border_width = radio_style.border_width;

  radio.bounds.width = -1.0f;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  radio.bounds = bounds;

  radio.style.border_width = 8.0f;
  radio.bounds.width = 20.0f;
  radio.bounds.height = 10.0f;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  radio.style.border_width = radio_style.border_width;
  radio.bounds = bounds;

  radio.style.border_width = 8.0f;
  radio.bounds.width = 10.0f;
  radio.bounds.height = 10.0f;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  radio.style.border_width = radio_style.border_width;
  radio.bounds = bounds;

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 2;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

#ifdef M3_TESTING
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_TRUE));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_RADIO_DOT_RADIUS_NEGATIVE));
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_selection_test_clear_fail_points());

  M3_TEST_OK(m3_radio_set_selected(&radio, M3_FALSE));
  M3_TEST_OK(m3_selection_test_set_fail_point(
      M3_SELECTION_TEST_FAIL_RADIO_INNER_CORNER_NEGATIVE));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  M3_TEST_OK(m3_selection_test_clear_fail_points());
#endif

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect = M3_ERR_IO;
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  M3_TEST_OK(test_backend_init(&backend));

  M3_TEST_OK(test_backend_init(&backend));
  backend.fail_draw_rect_after = 3;
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_TRUE));
  M3_TEST_EXPECT(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx),
                 M3_ERR_IO);
  backend.fail_draw_rect_after = 0;

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_FALSE));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 2);

  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_TRUE));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(backend.draw_rect_calls == 3);

  radio.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_TRUE));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               radio_style.disabled_checked.mark, 0.001f));
  M3_TEST_OK(test_backend_init(&backend));
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_FALSE));
  M3_TEST_OK(radio.widget.vtable->paint(radio.widget.ctx, &paint_ctx));
  M3_TEST_ASSERT(m3_color_near(backend.last_rect_color,
                               radio_style.disabled_unchecked.fill, 0.001f));
  radio.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(radio.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(radio.widget.vtable->event(radio.widget.ctx, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(radio.widget.vtable->event(radio.widget.ctx, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  radio.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  radio.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  radio.selected = 2;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(radio.widget.vtable->event(radio.widget.ctx, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  radio.selected = M3_FALSE;

  radio.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_EXPECT(radio.widget.vtable->event(radio.widget.ctx, &event, &handled),
                 M3_ERR_STATE);
  radio.pressed = M3_FALSE;

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_KEY_UP, 0, 0));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);

  counter.calls = 0;
  counter.fail = 0;
  M3_TEST_OK(m3_radio_set_selected(&radio, M3_FALSE));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(counter.calls == 1);

  counter.calls = 0;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_OK(radio.widget.vtable->event(radio.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(counter.calls == 0);

  counter.fail = 1;
  radio.selected = M3_FALSE;
  radio.pressed = M3_TRUE;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 5, 6));
  M3_TEST_EXPECT(radio.widget.vtable->event(radio.widget.ctx, &event, &handled),
                 M3_ERR_IO);
  M3_TEST_ASSERT(radio.selected == M3_FALSE);
  counter.fail = 0;

  M3_TEST_EXPECT(radio.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(radio.widget.vtable->get_semantics(radio.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_RADIO);

  radio.selected = M3_TRUE;
  radio.widget.flags |= M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(radio.widget.vtable->get_semantics(radio.widget.ctx, &semantics));
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_SELECTED) != 0u);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  radio.widget.flags &= ~M3_WIDGET_FLAG_DISABLED;

  M3_TEST_EXPECT(radio.widget.vtable->destroy(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(radio.widget.vtable->destroy(radio.widget.ctx));

  return 0;
}
