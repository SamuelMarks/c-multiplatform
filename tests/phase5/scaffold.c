#include "m3/m3_scaffold.h"
#include "test_utils.h"

#include <string.h>

int M3_CALL m3_scaffold_test_validate_edges(const M3LayoutEdges *edges);
int M3_CALL m3_scaffold_test_validate_style(const M3ScaffoldStyle *style);
int M3_CALL m3_scaffold_test_validate_measure_spec(M3MeasureSpec spec);
int M3_CALL m3_scaffold_test_validate_rect(const M3Rect *rect);
int M3_CALL m3_scaffold_test_compute_safe_bounds(const M3Rect *bounds,
                                                 const M3LayoutEdges *safe_area,
                                                 M3Rect *out_bounds);
int M3_CALL m3_scaffold_test_compute_fab_target(const M3ScaffoldStyle *style,
                                                M3Scalar snackbar_height,
                                                M3Scalar *out_offset);
int M3_CALL m3_scaffold_test_widget_is_visible(const M3Widget *widget,
                                               M3Bool *out_visible);
int M3_CALL m3_scaffold_test_measure_child(M3Widget *child, M3MeasureSpec width,
                                           M3MeasureSpec height,
                                           M3Size *out_size);
int M3_CALL m3_scaffold_test_layout_child(M3Widget *child,
                                          const M3Rect *bounds);
int M3_CALL m3_scaffold_test_paint_child(M3Widget *child, M3PaintContext *ctx);
int M3_CALL m3_scaffold_test_event_child(M3Widget *child,
                                         const M3InputEvent *event,
                                         M3Bool *out_handled);
int M3_CALL m3_scaffold_test_event_get_position(const M3InputEvent *event,
                                                M3Bool *out_has_pos,
                                                M3Scalar *out_x,
                                                M3Scalar *out_y);
int M3_CALL m3_scaffold_test_child_hit(M3Widget *child, const M3Rect *bounds,
                                       M3Scalar x, M3Scalar y, M3Bool *out_hit);
int M3_CALL m3_scaffold_test_apply_measure_spec(M3Scalar desired,
                                                M3MeasureSpec spec,
                                                M3Scalar *out_value);

typedef struct TestScaffoldWidget {
  M3Widget widget;
  M3Size desired_size;
  int measure_calls;
  int layout_calls;
  int paint_calls;
  int event_calls;
  int semantics_calls;
  int destroy_calls;
  int fail_measure;
  int fail_layout;
  int fail_paint;
  int fail_event;
  int fail_semantics;
  int fail_destroy;
  int handle_events;
  M3Rect last_bounds;
} TestScaffoldWidget;

static int test_widget_measure(void *widget, M3MeasureSpec width,
                               M3MeasureSpec height, M3Size *out_size) {
  TestScaffoldWidget *state;

  if (widget == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (width.mode != M3_MEASURE_UNSPECIFIED &&
      width.mode != M3_MEASURE_EXACTLY && width.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (height.mode != M3_MEASURE_UNSPECIFIED &&
      height.mode != M3_MEASURE_EXACTLY && height.mode != M3_MEASURE_AT_MOST) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width.mode != M3_MEASURE_UNSPECIFIED && width.size < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (height.mode != M3_MEASURE_UNSPECIFIED && height.size < 0.0f) {
    return M3_ERR_RANGE;
  }

  state = (TestScaffoldWidget *)widget;
  state->measure_calls += 1;
  if (state->fail_measure) {
    return M3_ERR_IO;
  }

  if (width.mode == M3_MEASURE_EXACTLY) {
    out_size->width = width.size;
  } else if (width.mode == M3_MEASURE_AT_MOST) {
    out_size->width = (state->desired_size.width > width.size)
                          ? width.size
                          : state->desired_size.width;
  } else {
    out_size->width = state->desired_size.width;
  }

  if (height.mode == M3_MEASURE_EXACTLY) {
    out_size->height = height.size;
  } else if (height.mode == M3_MEASURE_AT_MOST) {
    out_size->height = (state->desired_size.height > height.size)
                           ? height.size
                           : state->desired_size.height;
  } else {
    out_size->height = state->desired_size.height;
  }

  return M3_OK;
}

static int test_widget_layout(void *widget, M3Rect bounds) {
  TestScaffoldWidget *state;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestScaffoldWidget *)widget;
  state->layout_calls += 1;
  if (state->fail_layout) {
    return M3_ERR_IO;
  }

  state->last_bounds = bounds;
  return M3_OK;
}

static int test_widget_paint(void *widget, M3PaintContext *ctx) {
  TestScaffoldWidget *state;

  if (widget == NULL || ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestScaffoldWidget *)widget;
  state->paint_calls += 1;
  if (state->fail_paint) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static int test_widget_event(void *widget, const M3InputEvent *event,
                             M3Bool *out_handled) {
  TestScaffoldWidget *state;

  if (widget == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestScaffoldWidget *)widget;
  state->event_calls += 1;
  if (state->fail_event) {
    return M3_ERR_IO;
  }

  *out_handled = state->handle_events ? M3_TRUE : M3_FALSE;
  return M3_OK;
}

static int test_widget_get_semantics(void *widget, M3Semantics *out_semantics) {
  TestScaffoldWidget *state;

  if (widget == NULL || out_semantics == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestScaffoldWidget *)widget;
  state->semantics_calls += 1;
  if (state->fail_semantics) {
    return M3_ERR_IO;
  }

  out_semantics->role = M3_SEMANTIC_NONE;
  out_semantics->flags = 0u;
  out_semantics->utf8_label = NULL;
  out_semantics->utf8_hint = NULL;
  out_semantics->utf8_value = NULL;
  return M3_OK;
}

static int test_widget_destroy(void *widget) {
  TestScaffoldWidget *state;

  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestScaffoldWidget *)widget;
  state->destroy_calls += 1;
  if (state->fail_destroy) {
    return M3_ERR_IO;
  }
  return M3_OK;
}

static const M3WidgetVTable g_test_widget_vtable = {
    test_widget_measure, test_widget_layout,        test_widget_paint,
    test_widget_event,   test_widget_get_semantics, test_widget_destroy};
static const M3WidgetVTable g_test_widget_vtable_no_measure = {
    NULL,
    test_widget_layout,
    test_widget_paint,
    test_widget_event,
    test_widget_get_semantics,
    test_widget_destroy};
static const M3WidgetVTable g_test_widget_vtable_no_layout = {
    test_widget_measure,       NULL,
    test_widget_paint,         test_widget_event,
    test_widget_get_semantics, test_widget_destroy};
static const M3WidgetVTable g_test_widget_vtable_no_paint = {
    test_widget_measure, test_widget_layout,        NULL,
    test_widget_event,   test_widget_get_semantics, test_widget_destroy};
static const M3WidgetVTable g_test_widget_vtable_no_event = {
    test_widget_measure,       test_widget_layout, test_widget_paint, NULL,
    test_widget_get_semantics, test_widget_destroy};

static int test_widget_init(TestScaffoldWidget *state, M3Scalar width,
                            M3Scalar height) {
  if (state == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(state, 0, sizeof(*state));
  state->desired_size.width = width;
  state->desired_size.height = height;
  state->widget.ctx = state;
  state->widget.vtable = &g_test_widget_vtable;
  state->widget.handle.id = 0u;
  state->widget.handle.generation = 0u;
  state->widget.flags = 0u;
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

static int test_scaffold_helpers(void) {
  M3LayoutEdges edges;
  M3ScaffoldStyle style;
  M3MeasureSpec spec;
  M3Rect rect;
  M3Rect safe_bounds;
  M3Scalar offset;
  TestScaffoldWidget widget;
  M3Bool visible;
  M3Size size;
  M3PaintContext ctx;
  M3InputEvent event;
  M3Bool handled;
  M3Bool hit;
  M3Scalar value;
  M3Scalar x;
  M3Scalar y;

  M3_TEST_EXPECT(m3_scaffold_test_validate_edges(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  edges.left = -1.0f;
  edges.top = 0.0f;
  edges.right = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_edges(&edges), M3_ERR_RANGE);
  edges.left = 1.0f;
  edges.right = 1.0f;
  edges.top = 2.0f;
  edges.bottom = 3.0f;
  M3_TEST_OK(m3_scaffold_test_validate_edges(&edges));

  M3_TEST_EXPECT(m3_scaffold_test_validate_style(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.fab_slide_duration = 0.0f;
  M3_TEST_OK(m3_scaffold_test_validate_style(&style));
  style.fab_margin_x = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.snackbar_margin_y = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.fab_snackbar_spacing = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.fab_slide_duration = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_style(&style), M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.fab_slide_easing = 99u;
  M3_TEST_EXPECT(m3_scaffold_test_validate_style(&style), M3_ERR_RANGE);

  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_measure_spec(spec),
                 M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_measure_spec(spec), M3_ERR_RANGE);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = -1.0f;
  M3_TEST_OK(m3_scaffold_test_validate_measure_spec(spec));

  M3_TEST_EXPECT(m3_scaffold_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = -1.0f;
  rect.height = 0.0f;
  M3_TEST_EXPECT(m3_scaffold_test_validate_rect(&rect), M3_ERR_RANGE);
  rect.width = 10.0f;
  rect.height = 10.0f;
  M3_TEST_OK(m3_scaffold_test_validate_rect(&rect));

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 10.0f;
  rect.height = 10.0f;
  edges.left = 6.0f;
  edges.right = 6.0f;
  edges.top = 0.0f;
  edges.bottom = 0.0f;
  M3_TEST_EXPECT(
      m3_scaffold_test_compute_safe_bounds(NULL, &edges, &safe_bounds),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_test_compute_safe_bounds(&rect, NULL, &safe_bounds),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_compute_safe_bounds(&rect, &edges, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_test_compute_safe_bounds(&rect, &edges, &safe_bounds),
      M3_ERR_RANGE);
  edges.left = 1.0f;
  edges.right = 1.0f;
  edges.top = 2.0f;
  edges.bottom = 3.0f;
  M3_TEST_OK(m3_scaffold_test_compute_safe_bounds(&rect, &edges, &safe_bounds));
  M3_TEST_ASSERT(safe_bounds.x == 1.0f);
  M3_TEST_ASSERT(safe_bounds.y == 2.0f);
  M3_TEST_ASSERT(safe_bounds.width == 8.0f);
  M3_TEST_ASSERT(safe_bounds.height == 5.0f);

  M3_TEST_EXPECT(m3_scaffold_test_compute_fab_target(NULL, 1.0f, &offset),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_EXPECT(m3_scaffold_test_compute_fab_target(&style, -1.0f, &offset),
                 M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_test_compute_fab_target(&style, 0.0f, &offset));
  M3_TEST_ASSERT(offset == 0.0f);
  style.fab_margin_y = 16.0f;
  style.snackbar_margin_y = 12.0f;
  style.fab_snackbar_spacing = 8.0f;
  M3_TEST_OK(m3_scaffold_test_compute_fab_target(&style, 40.0f, &offset));
  M3_TEST_ASSERT(offset == 44.0f);

  M3_TEST_EXPECT(m3_scaffold_test_widget_is_visible(NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_scaffold_test_widget_is_visible(NULL, &visible));
  M3_TEST_ASSERT(visible == M3_FALSE);
  M3_TEST_OK(test_widget_init(&widget, 10.0f, 10.0f));
  widget.widget.flags = M3_WIDGET_FLAG_HIDDEN;
  M3_TEST_OK(m3_scaffold_test_widget_is_visible(&widget.widget, &visible));
  M3_TEST_ASSERT(visible == M3_FALSE);
  widget.widget.flags = 0u;
  M3_TEST_OK(m3_scaffold_test_widget_is_visible(&widget.widget, &visible));
  M3_TEST_ASSERT(visible == M3_TRUE);

  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = 10.0f;
  M3_TEST_EXPECT(m3_scaffold_test_measure_child(NULL, spec, spec, &size),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_test_measure_child(&widget.widget, spec, spec, NULL),
      M3_ERR_INVALID_ARGUMENT);
  spec.mode = 99u;
  spec.size = 0.0f;
  M3_TEST_EXPECT(
      m3_scaffold_test_measure_child(&widget.widget, spec, spec, &size),
      M3_ERR_INVALID_ARGUMENT);
  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = -1.0f;
  M3_TEST_EXPECT(
      m3_scaffold_test_measure_child(&widget.widget, spec, spec, &size),
      M3_ERR_RANGE);
  spec.size = 10.0f;
  widget.widget.vtable = &g_test_widget_vtable_no_measure;
  M3_TEST_EXPECT(
      m3_scaffold_test_measure_child(&widget.widget, spec, spec, &size),
      M3_ERR_UNSUPPORTED);
  widget.widget.vtable = &g_test_widget_vtable;
  M3_TEST_OK(m3_scaffold_test_measure_child(&widget.widget, spec, spec, &size));

  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 10.0f;
  rect.height = 10.0f;
  M3_TEST_EXPECT(m3_scaffold_test_layout_child(NULL, &rect),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_layout_child(&widget.widget, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  rect.width = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_test_layout_child(&widget.widget, &rect),
                 M3_ERR_RANGE);
  rect.width = 10.0f;
  widget.widget.vtable = &g_test_widget_vtable_no_layout;
  M3_TEST_EXPECT(m3_scaffold_test_layout_child(&widget.widget, &rect),
                 M3_ERR_UNSUPPORTED);
  widget.widget.vtable = &g_test_widget_vtable;
  M3_TEST_OK(m3_scaffold_test_layout_child(&widget.widget, &rect));

  memset(&ctx, 0, sizeof(ctx));
  M3_TEST_EXPECT(m3_scaffold_test_paint_child(NULL, &ctx),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_paint_child(&widget.widget, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  widget.widget.vtable = &g_test_widget_vtable_no_paint;
  M3_TEST_EXPECT(m3_scaffold_test_paint_child(&widget.widget, &ctx),
                 M3_ERR_UNSUPPORTED);
  widget.widget.vtable = &g_test_widget_vtable;
  M3_TEST_OK(m3_scaffold_test_paint_child(&widget.widget, &ctx));

  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_KEY_DOWN;
  M3_TEST_EXPECT(m3_scaffold_test_event_child(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_event_child(&widget.widget, NULL, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_event_child(&widget.widget, &event, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  widget.widget.vtable = &g_test_widget_vtable_no_event;
  M3_TEST_OK(m3_scaffold_test_event_child(&widget.widget, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  widget.widget.vtable = &g_test_widget_vtable;
  M3_TEST_OK(m3_scaffold_test_event_child(&widget.widget, &event, &handled));

  M3_TEST_EXPECT(m3_scaffold_test_event_get_position(NULL, &visible, &x, &y),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_test_event_get_position(&event, NULL, &x, &y),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_test_event_get_position(&event, &visible, NULL, &y),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_test_event_get_position(&event, &visible, &x, NULL),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 5, 6));
  M3_TEST_OK(m3_scaffold_test_event_get_position(&event, &visible, &x, &y));
  M3_TEST_ASSERT(visible == M3_TRUE);
  M3_TEST_ASSERT(x == 5.0f);
  M3_TEST_ASSERT(y == 6.0f);
  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_GESTURE_TAP;
  event.data.gesture.x = 7.0f;
  event.data.gesture.y = 8.0f;
  M3_TEST_OK(m3_scaffold_test_event_get_position(&event, &visible, &x, &y));
  M3_TEST_ASSERT(visible == M3_TRUE);
  M3_TEST_ASSERT(x == 7.0f);
  M3_TEST_ASSERT(y == 8.0f);
  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_KEY_DOWN;
  M3_TEST_OK(m3_scaffold_test_event_get_position(&event, &visible, &x, &y));
  M3_TEST_ASSERT(visible == M3_FALSE);

  M3_TEST_EXPECT(
      m3_scaffold_test_child_hit(&widget.widget, &rect, 0.0f, 0.0f, NULL),
      M3_ERR_INVALID_ARGUMENT);
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.width = 10.0f;
  rect.height = 10.0f;
  widget.widget.flags = M3_WIDGET_FLAG_HIDDEN;
  M3_TEST_OK(
      m3_scaffold_test_child_hit(&widget.widget, &rect, 1.0f, 1.0f, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);
  widget.widget.flags = 0u;
  M3_TEST_EXPECT(
      m3_scaffold_test_child_hit(&widget.widget, NULL, 1.0f, 1.0f, &hit),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(
      m3_scaffold_test_child_hit(&widget.widget, &rect, 1.0f, 1.0f, &hit));
  M3_TEST_ASSERT(hit == M3_TRUE);
  M3_TEST_OK(
      m3_scaffold_test_child_hit(&widget.widget, &rect, 20.0f, 20.0f, &hit));
  M3_TEST_ASSERT(hit == M3_FALSE);

  spec.mode = M3_MEASURE_EXACTLY;
  spec.size = 5.0f;
  M3_TEST_EXPECT(m3_scaffold_test_apply_measure_spec(10.0f, spec, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_scaffold_test_apply_measure_spec(10.0f, spec, &value));
  M3_TEST_ASSERT(value == 5.0f);
  spec.mode = M3_MEASURE_AT_MOST;
  spec.size = 8.0f;
  M3_TEST_OK(m3_scaffold_test_apply_measure_spec(10.0f, spec, &value));
  M3_TEST_ASSERT(value == 8.0f);
  spec.mode = M3_MEASURE_UNSPECIFIED;
  spec.size = 0.0f;
  M3_TEST_OK(m3_scaffold_test_apply_measure_spec(10.0f, spec, &value));
  M3_TEST_ASSERT(value == 10.0f);

  return 0;
}

static int test_scaffold_style_init(void) {
  M3ScaffoldStyle style;

  M3_TEST_EXPECT(m3_scaffold_style_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_ASSERT(style.padding.left == M3_SCAFFOLD_DEFAULT_PADDING_X);
  M3_TEST_ASSERT(style.padding.right == M3_SCAFFOLD_DEFAULT_PADDING_X);
  M3_TEST_ASSERT(style.padding.top == M3_SCAFFOLD_DEFAULT_PADDING_Y);
  M3_TEST_ASSERT(style.padding.bottom == M3_SCAFFOLD_DEFAULT_PADDING_Y);
  M3_TEST_ASSERT(style.fab_margin_x == M3_SCAFFOLD_DEFAULT_FAB_MARGIN);
  M3_TEST_ASSERT(style.fab_margin_y == M3_SCAFFOLD_DEFAULT_FAB_MARGIN);
  M3_TEST_ASSERT(style.snackbar_margin_x ==
                 M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_X);
  M3_TEST_ASSERT(style.snackbar_margin_y ==
                 M3_SCAFFOLD_DEFAULT_SNACKBAR_MARGIN_Y);
  M3_TEST_ASSERT(style.fab_snackbar_spacing ==
                 M3_SCAFFOLD_DEFAULT_FAB_SNACKBAR_SPACING);
  M3_TEST_ASSERT(style.fab_slide_duration ==
                 M3_SCAFFOLD_DEFAULT_FAB_SLIDE_DURATION);
  M3_TEST_ASSERT(style.fab_slide_easing ==
                 M3_SCAFFOLD_DEFAULT_FAB_SLIDE_EASING);
  return 0;
}

static int test_scaffold_init_and_setters(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  M3LayoutEdges safe_area;
  TestScaffoldWidget body;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_OK(test_widget_init(&body, 100.0f, 100.0f));

  M3_TEST_EXPECT(
      m3_scaffold_init(NULL, &style, &body.widget, NULL, NULL, NULL, NULL),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_scaffold_init(&scaffold, NULL, &body.widget, NULL, NULL, NULL, NULL),
      M3_ERR_INVALID_ARGUMENT);

  style.fab_margin_x = -1.0f;
  M3_TEST_EXPECT(
      m3_scaffold_init(&scaffold, &style, &body.widget, NULL, NULL, NULL, NULL),
      M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, NULL, NULL, NULL,
                              NULL));
  M3_TEST_OK(m3_scaffold_set_body(&scaffold, NULL));
  M3_TEST_OK(m3_scaffold_set_top_bar(&scaffold, NULL));
  M3_TEST_OK(m3_scaffold_set_bottom_bar(&scaffold, NULL));
  M3_TEST_OK(m3_scaffold_set_fab(&scaffold, NULL));
  M3_TEST_OK(m3_scaffold_set_snackbar(&scaffold, NULL));

  M3_TEST_EXPECT(m3_scaffold_set_style(NULL, &style), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_set_style(&scaffold, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  style.fab_slide_duration = -1.0f;
  M3_TEST_EXPECT(m3_scaffold_set_style(&scaffold, &style), M3_ERR_RANGE);
  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_OK(m3_scaffold_set_style(&scaffold, &style));

  safe_area.left = -1.0f;
  safe_area.top = 0.0f;
  safe_area.right = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_EXPECT(m3_scaffold_set_safe_area(&scaffold, &safe_area),
                 M3_ERR_RANGE);
  safe_area.left = 1.0f;
  safe_area.top = 2.0f;
  safe_area.right = 3.0f;
  safe_area.bottom = 4.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));
  safe_area.left = 0.0f;
  safe_area.top = 0.0f;
  safe_area.right = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_get_safe_area(&scaffold, &safe_area));
  M3_TEST_ASSERT(safe_area.left == 1.0f);
  M3_TEST_ASSERT(safe_area.top == 2.0f);
  M3_TEST_ASSERT(safe_area.right == 3.0f);
  M3_TEST_ASSERT(safe_area.bottom == 4.0f);

  M3_TEST_EXPECT(m3_scaffold_get_safe_area(NULL, &safe_area),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_get_safe_area(&scaffold, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  return 0;
}

static int test_scaffold_measure(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  TestScaffoldWidget bad_body;
  TestScaffoldWidget top_bar;
  TestScaffoldWidget bottom_bar;
  TestScaffoldWidget fab;
  TestScaffoldWidget snackbar;
  M3MeasureSpec width_spec;
  M3MeasureSpec height_spec;
  M3Size size;
  M3LayoutEdges safe_area;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.padding.left = 10.0f;
  style.padding.right = 10.0f;
  style.padding.top = 8.0f;
  style.padding.bottom = 8.0f;

  M3_TEST_OK(test_widget_init(&body, 200.0f, 100.0f));
  M3_TEST_OK(test_widget_init(&top_bar, 300.0f, 50.0f));
  M3_TEST_OK(test_widget_init(&bottom_bar, 280.0f, 40.0f));
  M3_TEST_OK(test_widget_init(&fab, 50.0f, 50.0f));
  M3_TEST_OK(test_widget_init(&snackbar, 180.0f, 30.0f));

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, &top_bar.widget,
                              &bottom_bar.widget, &fab.widget,
                              &snackbar.widget));

  safe_area.left = 0.0f;
  safe_area.top = 0.0f;
  safe_area.right = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 320.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 480.0f;
  M3_TEST_OK(scaffold.widget.vtable->measure(scaffold.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(size.width == 320.0f);
  M3_TEST_ASSERT(size.height == 480.0f);

  width_spec.mode = M3_MEASURE_UNSPECIFIED;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_UNSPECIFIED;
  height_spec.size = 0.0f;
  M3_TEST_OK(scaffold.widget.vtable->measure(scaffold.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(size.width == 300.0f);
  M3_TEST_ASSERT(size.height == 206.0f);

  width_spec.mode = M3_MEASURE_AT_MOST;
  width_spec.size = 250.0f;
  height_spec.mode = M3_MEASURE_AT_MOST;
  height_spec.size = 200.0f;
  M3_TEST_OK(scaffold.widget.vtable->measure(scaffold.widget.ctx, width_spec,
                                             height_spec, &size));
  M3_TEST_ASSERT(size.width == 250.0f);
  M3_TEST_ASSERT(size.height == 200.0f);

  width_spec.mode = 99u;
  width_spec.size = 0.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 10.0f;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_INVALID_ARGUMENT);

  scaffold.style.padding.left = -1.0f;
  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 100.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 100.0f;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);
  scaffold.style.padding.left = style.padding.left;

  scaffold.safe_area.left = -1.0f;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);
  scaffold.safe_area.left = 0.0f;

  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 10.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 10.0f;
  safe_area.left = 8.0f;
  safe_area.right = 8.0f;
  safe_area.top = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);

  safe_area.left = 0.0f;
  safe_area.right = 0.0f;
  safe_area.top = 6.0f;
  safe_area.bottom = 6.0f;
  height_spec.size = 10.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_RANGE);

  safe_area.left = 0.0f;
  safe_area.right = 0.0f;
  safe_area.top = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));
  body.fail_measure = 1;
  width_spec.mode = M3_MEASURE_EXACTLY;
  width_spec.size = 100.0f;
  height_spec.mode = M3_MEASURE_EXACTLY;
  height_spec.size = 100.0f;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_IO);
  body.fail_measure = 0;

  M3_TEST_OK(test_widget_init(&bad_body, 10.0f, 10.0f));
  bad_body.widget.vtable = &g_test_widget_vtable_no_measure;
  M3_TEST_OK(m3_scaffold_set_body(&scaffold, &bad_body.widget));
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_UNSUPPORTED);

  M3_TEST_OK(m3_scaffold_set_body(&scaffold, &body.widget));
  top_bar.fail_measure = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_IO);
  top_bar.fail_measure = 0;

  bottom_bar.fail_measure = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_IO);
  bottom_bar.fail_measure = 0;

  fab.fail_measure = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_IO);
  fab.fail_measure = 0;

  snackbar.fail_measure = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->measure(
                     scaffold.widget.ctx, width_spec, height_spec, &size),
                 M3_ERR_IO);
  snackbar.fail_measure = 0;

  return 0;
}

static int test_scaffold_layout(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  TestScaffoldWidget top_bar;
  TestScaffoldWidget bottom_bar;
  TestScaffoldWidget fab;
  TestScaffoldWidget snackbar;
  M3LayoutEdges safe_area;
  M3Rect bounds;
  M3Rect last;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.padding.left = 12.0f;
  style.padding.right = 12.0f;
  style.padding.top = 8.0f;
  style.padding.bottom = 8.0f;
  style.fab_margin_x = 16.0f;
  style.fab_margin_y = 16.0f;
  style.snackbar_margin_x = 16.0f;
  style.snackbar_margin_y = 12.0f;
  style.fab_snackbar_spacing = 8.0f;
  style.fab_slide_duration = 0.0f;

  M3_TEST_OK(test_widget_init(&body, 100.0f, 100.0f));
  M3_TEST_OK(test_widget_init(&top_bar, 300.0f, 56.0f));
  M3_TEST_OK(test_widget_init(&bottom_bar, 300.0f, 64.0f));
  M3_TEST_OK(test_widget_init(&fab, 56.0f, 56.0f));
  M3_TEST_OK(test_widget_init(&snackbar, 200.0f, 40.0f));

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, &top_bar.widget,
                              &bottom_bar.widget, &fab.widget,
                              &snackbar.widget));

  safe_area.left = 10.0f;
  safe_area.top = 20.0f;
  safe_area.right = 10.0f;
  safe_area.bottom = 30.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 360.0f;
  bounds.height = 640.0f;
  M3_TEST_OK(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds));

  last = top_bar.last_bounds;
  M3_TEST_ASSERT(last.x == 10.0f);
  M3_TEST_ASSERT(last.y == 20.0f);
  M3_TEST_ASSERT(last.width == 340.0f);
  M3_TEST_ASSERT(last.height == 56.0f);

  last = bottom_bar.last_bounds;
  M3_TEST_ASSERT(last.x == 10.0f);
  M3_TEST_ASSERT(last.y == 546.0f);
  M3_TEST_ASSERT(last.width == 340.0f);
  M3_TEST_ASSERT(last.height == 64.0f);

  last = body.last_bounds;
  M3_TEST_ASSERT(last.x == 22.0f);
  M3_TEST_ASSERT(last.y == 84.0f);
  M3_TEST_ASSERT(last.width == 316.0f);
  M3_TEST_ASSERT(last.height == 454.0f);

  last = snackbar.last_bounds;
  M3_TEST_ASSERT(last.width == 200.0f);
  M3_TEST_ASSERT(last.height == 40.0f);
  M3_TEST_ASSERT(last.x == 80.0f);
  M3_TEST_ASSERT(last.y == 494.0f);

  last = fab.last_bounds;
  M3_TEST_ASSERT(last.width == 56.0f);
  M3_TEST_ASSERT(last.height == 56.0f);
  M3_TEST_ASSERT(last.x == 278.0f);
  M3_TEST_ASSERT(last.y == 430.0f);

  bounds.width = -1.0f;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_RANGE);

  bounds.width = 360.0f;
  safe_area.left = 200.0f;
  safe_area.right = 200.0f;
  safe_area.top = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_RANGE);
  safe_area.left = 0.0f;
  safe_area.right = 0.0f;
  safe_area.top = 0.0f;
  safe_area.bottom = 0.0f;
  M3_TEST_OK(m3_scaffold_set_safe_area(&scaffold, &safe_area));

  top_bar.fail_layout = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_IO);
  top_bar.fail_layout = 0;

  top_bar.widget.vtable = &g_test_widget_vtable_no_layout;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_UNSUPPORTED);
  top_bar.widget.vtable = &g_test_widget_vtable;

  bottom_bar.fail_layout = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_IO);
  bottom_bar.fail_layout = 0;

  fab.fail_layout = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_IO);
  fab.fail_layout = 0;

  snackbar.fail_layout = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_IO);
  snackbar.fail_layout = 0;

  body.widget.vtable = &g_test_widget_vtable_no_layout;
  M3_TEST_EXPECT(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds),
                 M3_ERR_UNSUPPORTED);
  body.widget.vtable = &g_test_widget_vtable;

  return 0;
}

static int test_scaffold_paint(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  TestScaffoldWidget top_bar;
  TestScaffoldWidget bottom_bar;
  TestScaffoldWidget fab;
  TestScaffoldWidget snackbar;
  M3Rect bounds;
  M3PaintContext ctx;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_OK(test_widget_init(&body, 100.0f, 100.0f));
  M3_TEST_OK(test_widget_init(&top_bar, 100.0f, 10.0f));
  M3_TEST_OK(test_widget_init(&bottom_bar, 100.0f, 10.0f));
  M3_TEST_OK(test_widget_init(&fab, 10.0f, 10.0f));
  M3_TEST_OK(test_widget_init(&snackbar, 50.0f, 10.0f));

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, &top_bar.widget,
                              &bottom_bar.widget, &fab.widget,
                              &snackbar.widget));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 200.0f;
  M3_TEST_OK(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds));

  memset(&ctx, 0, sizeof(ctx));
  M3_TEST_OK(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx));
  M3_TEST_ASSERT(body.paint_calls == 1);
  M3_TEST_ASSERT(top_bar.paint_calls == 1);
  M3_TEST_ASSERT(bottom_bar.paint_calls == 1);
  M3_TEST_ASSERT(snackbar.paint_calls == 1);
  M3_TEST_ASSERT(fab.paint_calls == 1);

  body.fail_paint = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_IO);
  body.fail_paint = 0;

  top_bar.fail_paint = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_IO);
  top_bar.fail_paint = 0;

  bottom_bar.fail_paint = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_IO);
  bottom_bar.fail_paint = 0;

  snackbar.fail_paint = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_IO);
  snackbar.fail_paint = 0;

  fab.fail_paint = 1;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_IO);
  fab.fail_paint = 0;

  body.widget.vtable = &g_test_widget_vtable_no_paint;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  body.widget.vtable = &g_test_widget_vtable;

  top_bar.widget.vtable = &g_test_widget_vtable_no_paint;
  M3_TEST_EXPECT(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx),
                 M3_ERR_UNSUPPORTED);
  top_bar.widget.vtable = &g_test_widget_vtable;

  {
    int prev_calls = body.paint_calls;

    body.widget.flags = M3_WIDGET_FLAG_HIDDEN;
    M3_TEST_OK(scaffold.widget.vtable->paint(scaffold.widget.ctx, &ctx));
    M3_TEST_ASSERT(body.paint_calls == prev_calls);
    body.widget.flags = 0u;
  }

  return 0;
}

static int test_scaffold_event(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  TestScaffoldWidget top_bar;
  TestScaffoldWidget bottom_bar;
  TestScaffoldWidget fab;
  TestScaffoldWidget snackbar;
  M3Rect bounds;
  M3InputEvent event;
  M3Bool handled;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_OK(test_widget_init(&body, 100.0f, 100.0f));
  M3_TEST_OK(test_widget_init(&top_bar, 100.0f, 20.0f));
  M3_TEST_OK(test_widget_init(&bottom_bar, 100.0f, 20.0f));
  M3_TEST_OK(test_widget_init(&fab, 10.0f, 10.0f));
  M3_TEST_OK(test_widget_init(&snackbar, 50.0f, 10.0f));

  fab.handle_events = 1;
  snackbar.handle_events = 1;
  top_bar.handle_events = 1;
  bottom_bar.handle_events = 1;
  body.handle_events = 1;

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, &top_bar.widget,
                              &bottom_bar.widget, &fab.widget,
                              &snackbar.widget));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 200.0f;
  M3_TEST_OK(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds));

  memset(&event, 0, sizeof(event));
  M3_TEST_EXPECT(scaffold.widget.vtable->event(NULL, &event, &handled),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      scaffold.widget.vtable->event(scaffold.widget.ctx, NULL, &handled),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, NULL),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(fab.last_bounds.x + 1.0f),
                                (m3_i32)(fab.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(fab.event_calls == 1);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(snackbar.last_bounds.x + 1.0f),
                                (m3_i32)(snackbar.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(snackbar.event_calls == 1);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(top_bar.last_bounds.x + 1.0f),
                                (m3_i32)(top_bar.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(top_bar.event_calls == 1);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(bottom_bar.last_bounds.x + 1.0f),
                                (m3_i32)(bottom_bar.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(bottom_bar.event_calls == 1);

  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(body.last_bounds.x + 1.0f),
                                (m3_i32)(body.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);
  M3_TEST_ASSERT(body.event_calls == 1);

  memset(&event, 0, sizeof(event));
  event.type = M3_INPUT_KEY_DOWN;
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_TRUE);

  M3_TEST_OK(m3_scaffold_set_body(&scaffold, NULL));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  M3_TEST_OK(m3_scaffold_set_body(&scaffold, &body.widget));

  scaffold.widget.flags = M3_WIDGET_FLAG_HIDDEN;
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  scaffold.widget.flags = 0u;

  body.widget.flags = M3_WIDGET_FLAG_HIDDEN;
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  body.widget.flags = 0u;

  scaffold.widget.flags = M3_WIDGET_FLAG_DISABLED;
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  scaffold.widget.flags = 0u;

  fab.fail_event = 1;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(fab.last_bounds.x + 1.0f),
                                (m3_i32)(fab.last_bounds.y + 1.0f)));
  M3_TEST_EXPECT(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled),
      M3_ERR_IO);
  fab.fail_event = 0;

  fab.widget.vtable = &g_test_widget_vtable_no_event;
  M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_DOWN,
                                (m3_i32)(fab.last_bounds.x + 1.0f),
                                (m3_i32)(fab.last_bounds.y + 1.0f)));
  M3_TEST_OK(
      scaffold.widget.vtable->event(scaffold.widget.ctx, &event, &handled));
  M3_TEST_ASSERT(handled == M3_FALSE);
  fab.widget.vtable = &g_test_widget_vtable;

  return 0;
}

static int test_scaffold_semantics_destroy(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  M3Semantics semantics;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  M3_TEST_OK(test_widget_init(&body, 10.0f, 10.0f));
  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, NULL, NULL, NULL,
                              NULL));

  M3_TEST_EXPECT(scaffold.widget.vtable->get_semantics(NULL, &semantics),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      scaffold.widget.vtable->get_semantics(scaffold.widget.ctx, NULL),
      M3_ERR_INVALID_ARGUMENT);

  scaffold.widget.flags = M3_WIDGET_FLAG_DISABLED | M3_WIDGET_FLAG_FOCUSABLE;
  M3_TEST_OK(
      scaffold.widget.vtable->get_semantics(scaffold.widget.ctx, &semantics));
  M3_TEST_ASSERT(semantics.role == M3_SEMANTIC_NONE);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_DISABLED) != 0u);
  M3_TEST_ASSERT((semantics.flags & M3_SEMANTIC_FLAG_FOCUSABLE) != 0u);
  M3_TEST_ASSERT(semantics.utf8_label == NULL);
  M3_TEST_ASSERT(semantics.utf8_hint == NULL);
  M3_TEST_ASSERT(semantics.utf8_value == NULL);

  M3_TEST_EXPECT(scaffold.widget.vtable->destroy(NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(scaffold.widget.vtable->destroy(scaffold.widget.ctx));
  M3_TEST_ASSERT(scaffold.widget.vtable == NULL);
  M3_TEST_ASSERT(scaffold.widget.ctx == NULL);
  M3_TEST_ASSERT(scaffold.body == NULL);
  M3_TEST_ASSERT(scaffold.top_bar == NULL);
  M3_TEST_ASSERT(scaffold.bottom_bar == NULL);
  M3_TEST_ASSERT(scaffold.fab == NULL);
  M3_TEST_ASSERT(scaffold.snackbar == NULL);
  return 0;
}

static int test_scaffold_step(void) {
  M3Scaffold scaffold;
  M3ScaffoldStyle style;
  TestScaffoldWidget body;
  TestScaffoldWidget fab;
  TestScaffoldWidget snackbar;
  M3Rect bounds;
  M3Bool changed;

  M3_TEST_OK(m3_scaffold_style_init(&style));
  style.fab_slide_duration = 0.5f;
  M3_TEST_OK(test_widget_init(&body, 100.0f, 100.0f));
  M3_TEST_OK(test_widget_init(&fab, 10.0f, 10.0f));
  M3_TEST_OK(test_widget_init(&snackbar, 50.0f, 10.0f));

  M3_TEST_OK(m3_scaffold_init(&scaffold, &style, &body.widget, NULL, NULL,
                              &fab.widget, &snackbar.widget));

  bounds.x = 0.0f;
  bounds.y = 0.0f;
  bounds.width = 200.0f;
  bounds.height = 200.0f;
  M3_TEST_OK(scaffold.widget.vtable->layout(scaffold.widget.ctx, bounds));

  M3_TEST_OK(m3_scaffold_step(&scaffold, 0.1f, &changed));
  M3_TEST_ASSERT(changed == M3_TRUE);
  M3_TEST_ASSERT(scaffold.fab_offset > 0.0f);
  M3_TEST_ASSERT(fab.layout_calls > 1);

  M3_TEST_EXPECT(m3_scaffold_step(NULL, 0.1f, &changed),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_scaffold_step(&scaffold, -1.0f, &changed), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_scaffold_step(&scaffold, 0.1f, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  return 0;
}

int main(void) {
  M3_TEST_ASSERT(test_scaffold_helpers() == 0);
  M3_TEST_ASSERT(test_scaffold_style_init() == 0);
  M3_TEST_ASSERT(test_scaffold_init_and_setters() == 0);
  M3_TEST_ASSERT(test_scaffold_measure() == 0);
  M3_TEST_ASSERT(test_scaffold_layout() == 0);
  M3_TEST_ASSERT(test_scaffold_paint() == 0);
  M3_TEST_ASSERT(test_scaffold_event() == 0);
  M3_TEST_ASSERT(test_scaffold_semantics_destroy() == 0);
  M3_TEST_ASSERT(test_scaffold_step() == 0);
  return 0;
}
