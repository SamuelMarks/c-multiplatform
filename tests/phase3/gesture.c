#include "m3/m3_gesture.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestWidget {
  int event_calls;
  int gesture_calls;
  int tap_calls;
  int double_tap_calls;
  int long_press_calls;
  int drag_start_calls;
  int drag_update_calls;
  int drag_end_calls;
  int fling_calls;
  m3_u32 last_type;
  M3GestureEvent last_gesture;
  M3Bool handled;
  int fail_event;
} TestWidget;

static int test_widget_init(TestWidget *widget) {
  if (widget == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(widget, 0, sizeof(*widget));
  widget->handled = M3_TRUE;
  return M3_OK;
}

static int test_widget_event(void *ctx, const M3InputEvent *event,
                             M3Bool *out_handled) {
  TestWidget *widget;

  if (ctx == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  widget = (TestWidget *)ctx;
  widget->event_calls += 1;
  widget->last_type = event->type;

  switch (event->type) {
  case M3_INPUT_GESTURE_TAP:
    widget->gesture_calls += 1;
    widget->tap_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_DOUBLE_TAP:
    widget->gesture_calls += 1;
    widget->double_tap_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_LONG_PRESS:
    widget->gesture_calls += 1;
    widget->long_press_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_DRAG_START:
    widget->gesture_calls += 1;
    widget->drag_start_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_DRAG_UPDATE:
    widget->gesture_calls += 1;
    widget->drag_update_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_DRAG_END:
    widget->gesture_calls += 1;
    widget->drag_end_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case M3_INPUT_GESTURE_FLING:
    widget->gesture_calls += 1;
    widget->fling_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  default:
    break;
  }

  if (widget->fail_event) {
    return M3_ERR_UNKNOWN;
  }

  *out_handled = widget->handled;
  return M3_OK;
}

static int setup_widget(M3Widget *widget, M3WidgetVTable *vtable,
                        TestWidget *state, m3_u32 flags, m3_u32 id) {
  if (widget == NULL || vtable == NULL || state == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  widget->ctx = state;
  widget->vtable = vtable;
  widget->handle.id = id;
  widget->handle.generation = 1;
  widget->flags = flags;
  return M3_OK;
}

static int init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                              m3_i32 y, m3_i32 pointer_id, m3_u32 time_ms) {
  if (event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->time_ms = time_ms;
  event->data.pointer.pointer_id = pointer_id;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return M3_OK;
}

int main(void) {
  {
    M3GestureConfig config;
#ifdef M3_TESTING
    M3GestureConfig invalid;
#endif

    M3_TEST_EXPECT(m3_gesture_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_gesture_config_init(&config));
    M3_TEST_ASSERT(config.tap_max_ms == M3_GESTURE_DEFAULT_TAP_MAX_MS);
    M3_TEST_ASSERT(config.tap_max_distance ==
                   M3_GESTURE_DEFAULT_TAP_MAX_DISTANCE);
    M3_TEST_ASSERT(config.double_tap_max_ms ==
                   M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS);
    M3_TEST_ASSERT(config.double_tap_max_distance ==
                   M3_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE);
    M3_TEST_ASSERT(config.long_press_ms == M3_GESTURE_DEFAULT_LONG_PRESS_MS);
    M3_TEST_ASSERT(config.drag_start_distance ==
                   M3_GESTURE_DEFAULT_DRAG_START_DISTANCE);
    M3_TEST_ASSERT(config.fling_min_velocity ==
                   M3_GESTURE_DEFAULT_FLING_MIN_VELOCITY);

#ifdef M3_TESTING
    M3_TEST_EXPECT(m3_gesture_test_validate_config(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    invalid = config;
    invalid.tap_max_distance = -1.0f;
    M3_TEST_EXPECT(m3_gesture_test_validate_config(&invalid), M3_ERR_RANGE);
    invalid = config;
    invalid.double_tap_max_distance = -1.0f;
    M3_TEST_EXPECT(m3_gesture_test_validate_config(&invalid), M3_ERR_RANGE);
    invalid = config;
    invalid.drag_start_distance = -1.0f;
    M3_TEST_EXPECT(m3_gesture_test_validate_config(&invalid), M3_ERR_RANGE);
    invalid = config;
    invalid.fling_min_velocity = -1.0f;
    M3_TEST_EXPECT(m3_gesture_test_validate_config(&invalid), M3_ERR_RANGE);
#endif
  }

  {
    M3GestureDispatcher dispatcher;
    M3GestureConfig config;
    M3GestureConfig out_config;

    memset(&dispatcher, 0, sizeof(dispatcher));
    M3_TEST_OK(m3_gesture_config_init(&config));

    M3_TEST_EXPECT(m3_gesture_dispatcher_init(NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_gesture_dispatcher_shutdown(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_gesture_dispatcher_reset(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_gesture_dispatcher_set_config(NULL, &config),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_gesture_dispatcher_get_config(NULL, &out_config),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_gesture_dispatcher_shutdown(&dispatcher), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_gesture_dispatcher_reset(&dispatcher), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_gesture_dispatcher_set_config(&dispatcher, &config),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_gesture_dispatcher_get_config(&dispatcher, &out_config),
                   M3_ERR_STATE);

    M3_TEST_OK(m3_gesture_dispatcher_init(&dispatcher, &config));
    M3_TEST_EXPECT(m3_gesture_dispatcher_init(&dispatcher, &config),
                   M3_ERR_STATE);

    M3_TEST_OK(m3_gesture_dispatcher_get_config(&dispatcher, &out_config));
    M3_TEST_ASSERT(out_config.tap_max_ms == config.tap_max_ms);

    config.tap_max_distance = -1.0f;
    M3_TEST_EXPECT(m3_gesture_dispatcher_set_config(&dispatcher, &config),
                   M3_ERR_RANGE);
    config.tap_max_distance = M3_GESTURE_DEFAULT_TAP_MAX_DISTANCE;
    M3_TEST_OK(m3_gesture_dispatcher_set_config(&dispatcher, &config));

    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(m3_gesture_dispatcher_shutdown(&dispatcher));
    M3_TEST_EXPECT(m3_gesture_dispatcher_shutdown(&dispatcher), M3_ERR_STATE);
  }

  {
    M3RenderNode root;
    M3RenderNode child1;
    M3RenderNode child2;
    M3RenderNode *children[2];
    M3Widget root_widget;
    M3Widget widget1;
    M3Widget widget2;
    M3WidgetVTable vtable;
    TestWidget root_state;
    TestWidget state1;
    TestWidget state2;
    M3Rect root_bounds;
    M3Rect child_bounds;
    M3Widget *hit;

    memset(&vtable, 0, sizeof(vtable));
    vtable.event = test_widget_event;

    M3_TEST_OK(test_widget_init(&root_state));
    M3_TEST_OK(test_widget_init(&state1));
    M3_TEST_OK(test_widget_init(&state2));

    root_bounds.x = 0.0f;
    root_bounds.y = 0.0f;
    root_bounds.width = 100.0f;
    root_bounds.height = 100.0f;
    child_bounds.x = 0.0f;
    child_bounds.y = 0.0f;
    child_bounds.width = 50.0f;
    child_bounds.height = 50.0f;

    M3_TEST_OK(setup_widget(&root_widget, &vtable, &root_state, 0, 1));
    M3_TEST_OK(setup_widget(&widget1, &vtable, &state1, 0, 2));
    M3_TEST_OK(setup_widget(&widget2, &vtable, &state2, 0, 3));

    M3_TEST_OK(m3_render_node_init(&root, &root_widget, &root_bounds));
    M3_TEST_OK(m3_render_node_init(&child1, &widget1, &child_bounds));
    M3_TEST_OK(m3_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    M3_TEST_OK(m3_render_node_set_children(&root, children, 2));

#ifdef M3_TESTING
    M3_TEST_EXPECT(m3_gesture_test_hit_test(NULL, 0.0f, 0.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_gesture_test_hit_test(&root, 0.0f, 0.0f, NULL),
                   M3_ERR_INVALID_ARGUMENT);
#endif

    hit = NULL;
    M3_TEST_OK(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    M3_TEST_ASSERT(hit == &widget2);

    widget2.flags = M3_WIDGET_FLAG_HIDDEN;
    hit = NULL;
    M3_TEST_OK(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    M3_TEST_ASSERT(hit == &widget1);
    widget2.flags = 0;

    widget2.flags = M3_WIDGET_FLAG_DISABLED;
    hit = NULL;
    M3_TEST_OK(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    M3_TEST_ASSERT(hit == &widget1);
    widget2.flags = 0;

    widget2.flags = M3_WIDGET_FLAG_HIDDEN;
    child1.bounds.width = -1.0f;
    hit = NULL;
    M3_TEST_EXPECT(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                   M3_ERR_RANGE);
    child_bounds.width = 50.0f;
    M3_TEST_OK(m3_render_node_set_bounds(&child1, &child_bounds));
    widget2.flags = 0;

    root.child_count = 1;
    root.children = NULL;
    hit = NULL;
    M3_TEST_EXPECT(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    root.children = children;
    root.child_count = 2;

    root.widget = NULL;
    hit = NULL;
    M3_TEST_EXPECT(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    root.widget = &root_widget;

    children[1] = NULL;
    hit = NULL;
    M3_TEST_EXPECT(m3_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    children[1] = &child2;
    M3_TEST_OK(m3_render_node_set_children(&root, children, 2));
  }

  {
    M3GestureDispatcher dispatcher;
    M3GestureConfig config;
    M3RenderNode root;
    M3RenderNode child1;
    M3RenderNode child2;
    M3RenderNode *children[2];
    M3Widget root_widget;
    M3Widget widget1;
    M3Widget widget2;
    M3WidgetVTable vtable;
    TestWidget root_state;
    TestWidget state1;
    TestWidget state2;
    M3Rect root_bounds;
    M3Rect child_bounds;
    M3InputEvent event;
    M3Widget *target;
    M3Bool handled;

    memset(&dispatcher, 0, sizeof(dispatcher));
    M3_TEST_OK(m3_gesture_config_init(&config));
    config.tap_max_ms = 300u;
    config.double_tap_max_ms = 400u;
    config.long_press_ms = 500u;
    config.tap_max_distance = 5.0f;
    config.double_tap_max_distance = 10.0f;
    config.drag_start_distance = 5.0f;
    config.fling_min_velocity = 900.0f;
    M3_TEST_OK(m3_gesture_dispatcher_init(&dispatcher, &config));

    memset(&vtable, 0, sizeof(vtable));
    vtable.event = test_widget_event;

    M3_TEST_OK(test_widget_init(&root_state));
    M3_TEST_OK(test_widget_init(&state1));
    M3_TEST_OK(test_widget_init(&state2));

    root_bounds.x = 0.0f;
    root_bounds.y = 0.0f;
    root_bounds.width = 100.0f;
    root_bounds.height = 100.0f;
    child_bounds.x = 0.0f;
    child_bounds.y = 0.0f;
    child_bounds.width = 50.0f;
    child_bounds.height = 50.0f;

    M3_TEST_OK(setup_widget(&root_widget, &vtable, &root_state, 0, 1));
    M3_TEST_OK(setup_widget(&widget1, &vtable, &state1, 0, 2));
    M3_TEST_OK(setup_widget(&widget2, &vtable, &state2, 0, 3));

    M3_TEST_OK(m3_render_node_init(&root, &root_widget, &root_bounds));
    M3_TEST_OK(m3_render_node_init(&child1, &widget1, &child_bounds));
    M3_TEST_OK(m3_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    M3_TEST_OK(m3_render_node_set_children(&root, children, 2));

    M3_TEST_EXPECT(m3_gesture_dispatch(NULL, &root, &event, &target, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, NULL, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, NULL, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, NULL),
        M3_ERR_INVALID_ARGUMENT);

    event.type = M3_INPUT_TEXT;
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 0, 0, 1, 10));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 0, 0, 1, 10));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 100));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget2);
    M3_TEST_ASSERT(handled == M3_FALSE);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 2, 110));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_BUSY);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 120));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_STATE);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 10, 10, 2, 130));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 2, 140));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 10, 10, 1, 90));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_RANGE);

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 1, 150));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state2.tap_calls == 1);
    M3_TEST_ASSERT(state2.last_type == M3_INPUT_GESTURE_TAP);
    M3_TEST_ASSERT(state2.last_gesture.tap_count == 1u);

    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(test_widget_init(&state1));
    M3_TEST_OK(test_widget_init(&state2));

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 1000));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 1, 1050));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.tap_calls == 1);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 12, 11, 1, 1200));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_UP, 12, 11, 1, 1240));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.double_tap_calls == 1);
    M3_TEST_ASSERT(state2.tap_calls == 1);
    M3_TEST_ASSERT(state2.last_type == M3_INPUT_GESTURE_DOUBLE_TAP);
    M3_TEST_ASSERT(state2.last_gesture.tap_count == 2u);

    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(test_widget_init(&state2));

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 2000));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 1, 2600));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.long_press_calls == 1);
    M3_TEST_ASSERT(state2.tap_calls == 0);
    M3_TEST_ASSERT(state2.last_type == M3_INPUT_GESTURE_LONG_PRESS);

    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(test_widget_init(&state2));

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 0, 0, 1, 3000));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 6, 0, 1, 3010));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.drag_start_calls == 1);
    M3_TEST_ASSERT(state2.last_gesture.total_x == 6.0f);

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_MOVE, 16, 4, 1, 3020));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.drag_update_calls == 1);
    M3_TEST_ASSERT(state2.last_gesture.delta_x == 10.0f);
    M3_TEST_ASSERT(state2.last_gesture.delta_y == 4.0f);

    M3_TEST_OK(init_pointer_event(&event, M3_INPUT_POINTER_UP, 36, 4, 1, 3030));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state2.drag_end_calls == 1);
    M3_TEST_ASSERT(state2.fling_calls == 1);
    M3_TEST_ASSERT(state2.last_type == M3_INPUT_GESTURE_FLING);
    M3_TEST_ASSERT(state2.last_gesture.velocity_x >= 1900.0f);

    widget2.flags = M3_WIDGET_FLAG_HIDDEN;
    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(test_widget_init(&state1));
    M3_TEST_OK(test_widget_init(&state2));

    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 4000));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 1, 4050));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(state1.tap_calls == 1);
    M3_TEST_ASSERT(state2.tap_calls == 0);

    widget2.flags = 0;
    state2.fail_event = 1;
    M3_TEST_OK(m3_gesture_dispatcher_reset(&dispatcher));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10, 1, 5000));
    M3_TEST_OK(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_OK(
        init_pointer_event(&event, M3_INPUT_POINTER_UP, 10, 10, 1, 5050));
    M3_TEST_EXPECT(
        m3_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_UNKNOWN);

    state2.fail_event = 0;

    M3_TEST_OK(m3_gesture_dispatcher_shutdown(&dispatcher));
  }

  return 0;
}
