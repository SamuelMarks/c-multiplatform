#include "cmpc/cmp_gesture.h"
#include "test_utils.h"

#include <string.h>

int CMP_CALL cmp_gesture_test_set_contains_error(CMPBool enable);
int CMP_CALL cmp_gesture_test_set_reset_fail(CMPBool enable);
int CMP_CALL cmp_gesture_test_set_velocity_error(CMPBool enable);
int CMP_CALL cmp_gesture_test_set_distance_error(CMPBool enable);
int CMP_CALL cmp_gesture_test_set_distance_error_after(cmp_i32 calls);
int CMP_CALL cmp_gesture_test_set_config_init_fail(CMPBool enable);
int CMP_CALL cmp_gesture_test_validate_rect(const CMPRect *rect);
int CMP_CALL cmp_gesture_test_distance_sq(CMPScalar x0, CMPScalar y0,
                                          CMPScalar x1, CMPScalar y1,
                                          CMPScalar *out_value);
int CMP_CALL cmp_gesture_test_time_delta(cmp_u32 start, cmp_u32 end,
                                         cmp_u32 *out_delta);
int CMP_CALL cmp_gesture_test_velocity(CMPScalar x0, CMPScalar y0, CMPScalar x1,
                                       CMPScalar y1, cmp_u32 dt_ms,
                                       CMPScalar *out_vx, CMPScalar *out_vy);
int CMP_CALL cmp_gesture_test_dispatch_to_widget(CMPWidget *widget,
                                                 const CMPInputEvent *event,
                                                 CMPBool *out_handled);
int CMP_CALL cmp_gesture_test_emit(CMPWidget *widget,
                                   const CMPInputEvent *source, cmp_u32 type,
                                   const CMPGestureEvent *gesture,
                                   CMPBool *out_handled);
int CMP_CALL cmp_gesture_test_reset_pointer(CMPGestureDispatcher *dispatcher);
int CMP_CALL cmp_gesture_test_clear_state(CMPGestureDispatcher *dispatcher);

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
  cmp_u32 last_type;
  CMPGestureEvent last_gesture;
  CMPBool handled;
  int fail_event;
  cmp_u32 fail_type;
} TestWidget;

static int test_widget_init(TestWidget *widget) {
  if (widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(widget, 0, sizeof(*widget));
  widget->handled = CMP_TRUE;
  return CMP_OK;
}

static int test_widget_event(void *ctx, const CMPInputEvent *event,
                             CMPBool *out_handled) {
  TestWidget *widget;

  if (ctx == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget = (TestWidget *)ctx;
  widget->event_calls += 1;
  widget->last_type = event->type;

  switch (event->type) {
  case CMP_INPUT_GESTURE_TAP:
    widget->gesture_calls += 1;
    widget->tap_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_DOUBLE_TAP:
    widget->gesture_calls += 1;
    widget->double_tap_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_LONG_PRESS:
    widget->gesture_calls += 1;
    widget->long_press_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_DRAG_START:
    widget->gesture_calls += 1;
    widget->drag_start_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_DRAG_UPDATE:
    widget->gesture_calls += 1;
    widget->drag_update_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_DRAG_END:
    widget->gesture_calls += 1;
    widget->drag_end_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  case CMP_INPUT_GESTURE_FLING:
    widget->gesture_calls += 1;
    widget->fling_calls += 1;
    widget->last_gesture = event->data.gesture;
    break;
  default:
    break;
  }

  if (widget->fail_event) {
    return CMP_ERR_UNKNOWN;
  }
  if (widget->fail_type != 0u && event->type == widget->fail_type) {
    return CMP_ERR_UNKNOWN;
  }

  *out_handled = widget->handled;
  return CMP_OK;
}

static int setup_widget(CMPWidget *widget, CMPWidgetVTable *vtable,
                        TestWidget *state, cmp_u32 flags, cmp_u32 id) {
  if (widget == NULL || vtable == NULL || state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget->ctx = state;
  widget->vtable = vtable;
  widget->handle.id = id;
  widget->handle.generation = 1;
  widget->flags = flags;
  return CMP_OK;
}

static int init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                              cmp_i32 y, cmp_i32 pointer_id, cmp_u32 time_ms) {
  if (event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(event, 0, sizeof(*event));
  event->type = type;
  event->time_ms = time_ms;
  event->data.pointer.pointer_id = pointer_id;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
  return CMP_OK;
}

int main(void) {
  {
    CMPGestureConfig config;
#ifdef CMP_TESTING
    CMPGestureConfig invalid;
#endif

    CMP_TEST_EXPECT(cmp_gesture_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_gesture_config_init(&config));
    CMP_TEST_ASSERT(config.tap_max_ms == CMP_GESTURE_DEFAULT_TAP_MAX_MS);
    CMP_TEST_ASSERT(config.tap_max_distance ==
                    CMP_GESTURE_DEFAULT_TAP_MAX_DISTANCE);
    CMP_TEST_ASSERT(config.double_tap_max_ms ==
                    CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_MS);
    CMP_TEST_ASSERT(config.double_tap_max_distance ==
                    CMP_GESTURE_DEFAULT_DOUBLE_TAP_MAX_DISTANCE);
    CMP_TEST_ASSERT(config.long_press_ms == CMP_GESTURE_DEFAULT_LONG_PRESS_MS);
    CMP_TEST_ASSERT(config.drag_start_distance ==
                    CMP_GESTURE_DEFAULT_DRAG_START_DISTANCE);
    CMP_TEST_ASSERT(config.fling_min_velocity ==
                    CMP_GESTURE_DEFAULT_FLING_MIN_VELOCITY);

#ifdef CMP_TESTING
    CMP_TEST_EXPECT(cmp_gesture_test_validate_config(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    invalid = config;
    invalid.tap_max_distance = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_test_validate_config(&invalid), CMP_ERR_RANGE);
    invalid = config;
    invalid.double_tap_max_distance = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_test_validate_config(&invalid), CMP_ERR_RANGE);
    invalid = config;
    invalid.drag_start_distance = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_test_validate_config(&invalid), CMP_ERR_RANGE);
    invalid = config;
    invalid.fling_min_velocity = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_test_validate_config(&invalid), CMP_ERR_RANGE);
#endif
  }

  {
    CMPRect rect;
    CMPScalar value;
    cmp_u32 delta;
    CMPScalar vx;
    CMPScalar vy;
    CMPGestureConfig config;
    CMPGestureConfig invalid_config;
    CMPGestureDispatcher dispatcher;
    CMPWidget widget;
    CMPInputEvent event;
    CMPGestureEvent gesture;
    CMPBool handled;
    CMPRenderNode node;
    CMPWidget *hit;

    CMP_TEST_EXPECT(cmp_gesture_test_validate_rect(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    CMP_TEST_EXPECT(cmp_gesture_test_validate_rect(&rect), CMP_ERR_RANGE);
    rect.width = 1.0f;
    CMP_TEST_OK(cmp_gesture_test_validate_rect(&rect));

    CMP_TEST_EXPECT(cmp_gesture_test_distance_sq(0.0f, 0.0f, 1.0f, 1.0f, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_gesture_test_distance_sq(0.0f, 0.0f, 1.0f, 1.0f, &value));

    CMP_TEST_EXPECT(cmp_gesture_test_time_delta(0u, 1u, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_test_time_delta(10u, 5u, &delta),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_gesture_test_time_delta(5u, 10u, &delta));

    CMP_TEST_EXPECT(
        cmp_gesture_test_velocity(0.0f, 0.0f, 1.0f, 1.0f, 10u, NULL, &vy),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_gesture_test_velocity(0.0f, 0.0f, 1.0f, 1.0f, 10u, &vx, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        cmp_gesture_test_velocity(0.0f, 0.0f, 1.0f, 1.0f, 0u, &vx, &vy));

    CMP_TEST_EXPECT(cmp_gesture_test_set_distance_error_after(-1),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_gesture_test_set_distance_error_after(0));

    memset(&widget, 0, sizeof(widget));
    widget.flags = 0u;
    node.widget = &widget;
    node.bounds = rect;
    node.child_count = 0u;
    node.children = NULL;
    hit = NULL;
    CMP_TEST_OK(cmp_gesture_test_hit_test(&node, 100.0f, 100.0f, &hit));
    CMP_TEST_ASSERT(hit == NULL);

    CMP_TEST_OK(cmp_gesture_test_set_contains_error(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&node, 0.5f, 0.5f, &hit),
                    CMP_ERR_RANGE);

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_GESTURE_TAP;
    CMP_TEST_EXPECT(cmp_gesture_test_dispatch_to_widget(&widget, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_test_dispatch_to_widget(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_gesture_test_dispatch_to_widget(&widget, NULL, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    widget.vtable = NULL;
    CMP_TEST_OK(cmp_gesture_test_dispatch_to_widget(&widget, &event, &handled));

    memset(&gesture, 0, sizeof(gesture));
    CMP_TEST_EXPECT(cmp_gesture_test_emit(
                        &widget, &event, CMP_INPUT_GESTURE_TAP, &gesture, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_gesture_test_emit(NULL, &event, CMP_INPUT_GESTURE_TAP,
                                      &gesture, &handled));
    CMP_TEST_EXPECT(cmp_gesture_test_emit(&widget, NULL, CMP_INPUT_GESTURE_TAP,
                                          &gesture, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_test_emit(
                        &widget, &event, CMP_INPUT_GESTURE_TAP, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_gesture_test_reset_pointer(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_test_clear_state(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    memset(&dispatcher, 0, sizeof(dispatcher));
    CMP_TEST_OK(cmp_gesture_test_set_reset_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_gesture_test_clear_state(&dispatcher), CMP_ERR_UNKNOWN);

    memset(&dispatcher, 0, sizeof(dispatcher));
    CMP_TEST_OK(cmp_gesture_config_init(&config));
    CMP_TEST_OK(cmp_gesture_test_set_config_init_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_init(&dispatcher, NULL),
                    CMP_ERR_UNKNOWN);

    invalid_config = config;
    invalid_config.tap_max_distance = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_init(&dispatcher, &invalid_config),
                    CMP_ERR_RANGE);

    CMP_TEST_OK(cmp_gesture_test_set_reset_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_init(&dispatcher, &config),
                    CMP_ERR_UNKNOWN);

    CMP_TEST_OK(cmp_gesture_dispatcher_init(&dispatcher, &config));
    CMP_TEST_OK(cmp_gesture_test_set_reset_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_shutdown(&dispatcher),
                    CMP_ERR_UNKNOWN);

    memset(&dispatcher, 0, sizeof(dispatcher));
    CMP_TEST_OK(cmp_gesture_dispatcher_init(&dispatcher, NULL));
    CMP_TEST_OK(cmp_gesture_dispatcher_shutdown(&dispatcher));
  }

  {
    CMPGestureDispatcher dispatcher;
    CMPGestureConfig config;
    CMPGestureConfig out_config;

    memset(&dispatcher, 0, sizeof(dispatcher));
    CMP_TEST_OK(cmp_gesture_config_init(&config));

    CMP_TEST_EXPECT(cmp_gesture_dispatcher_init(NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_shutdown(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_reset(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_set_config(NULL, &config),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_get_config(NULL, &out_config),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_gesture_dispatcher_shutdown(&dispatcher),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_reset(&dispatcher), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_set_config(&dispatcher, &config),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_get_config(&dispatcher, &out_config),
                    CMP_ERR_STATE);

    CMP_TEST_OK(cmp_gesture_dispatcher_init(&dispatcher, &config));
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_init(&dispatcher, &config),
                    CMP_ERR_STATE);

    CMP_TEST_OK(cmp_gesture_dispatcher_get_config(&dispatcher, &out_config));
    CMP_TEST_ASSERT(out_config.tap_max_ms == config.tap_max_ms);

    config.tap_max_distance = -1.0f;
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_set_config(&dispatcher, &config),
                    CMP_ERR_RANGE);
    config.tap_max_distance = CMP_GESTURE_DEFAULT_TAP_MAX_DISTANCE;
    CMP_TEST_OK(cmp_gesture_dispatcher_set_config(&dispatcher, &config));

    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(cmp_gesture_dispatcher_shutdown(&dispatcher));
    CMP_TEST_EXPECT(cmp_gesture_dispatcher_shutdown(&dispatcher),
                    CMP_ERR_STATE);
  }

  {
    CMPRenderNode root;
    CMPRenderNode child1;
    CMPRenderNode child2;
    CMPRenderNode *children[2];
    CMPWidget root_widget;
    CMPWidget widget1;
    CMPWidget widget2;
    CMPWidgetVTable vtable;
    TestWidget root_state;
    TestWidget state1;
    TestWidget state2;
    CMPRect root_bounds;
    CMPRect child_bounds;
    CMPWidget *hit;

    memset(&vtable, 0, sizeof(vtable));
    vtable.event = test_widget_event;

    CMP_TEST_OK(test_widget_init(&root_state));
    CMP_TEST_OK(test_widget_init(&state1));
    CMP_TEST_OK(test_widget_init(&state2));

    root_bounds.x = 0.0f;
    root_bounds.y = 0.0f;
    root_bounds.width = 100.0f;
    root_bounds.height = 100.0f;
    child_bounds.x = 0.0f;
    child_bounds.y = 0.0f;
    child_bounds.width = 50.0f;
    child_bounds.height = 50.0f;

    CMP_TEST_OK(setup_widget(&root_widget, &vtable, &root_state, 0, 1));
    CMP_TEST_OK(setup_widget(&widget1, &vtable, &state1, 0, 2));
    CMP_TEST_OK(setup_widget(&widget2, &vtable, &state2, 0, 3));

    CMP_TEST_OK(cmp_render_node_init(&root, &root_widget, &root_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child1, &widget1, &child_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    CMP_TEST_OK(cmp_render_node_set_children(&root, children, 2));

#ifdef CMP_TESTING
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(NULL, 0.0f, 0.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&root, 0.0f, 0.0f, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
#endif

    hit = NULL;
    CMP_TEST_OK(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    CMP_TEST_ASSERT(hit == &widget2);

    widget2.flags = CMP_WIDGET_FLAG_HIDDEN;
    hit = NULL;
    CMP_TEST_OK(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    CMP_TEST_ASSERT(hit == &widget1);
    widget2.flags = 0;

    widget2.flags = CMP_WIDGET_FLAG_DISABLED;
    hit = NULL;
    CMP_TEST_OK(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit));
    CMP_TEST_ASSERT(hit == &widget1);
    widget2.flags = 0;

    widget2.flags = CMP_WIDGET_FLAG_HIDDEN;
    child1.bounds.width = -1.0f;
    hit = NULL;
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                    CMP_ERR_RANGE);
    child_bounds.width = 50.0f;
    CMP_TEST_OK(cmp_render_node_set_bounds(&child1, &child_bounds));
    widget2.flags = 0;

    root.child_count = 1;
    root.children = NULL;
    hit = NULL;
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    root.children = children;
    root.child_count = 2;

    root.widget = NULL;
    hit = NULL;
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    root.widget = &root_widget;

    children[1] = NULL;
    hit = NULL;
    CMP_TEST_EXPECT(cmp_gesture_test_hit_test(&root, 10.0f, 10.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    children[1] = &child2;
    CMP_TEST_OK(cmp_render_node_set_children(&root, children, 2));
  }

  {
    CMPGestureDispatcher dispatcher;
    CMPGestureConfig config;
    CMPRenderNode root;
    CMPRenderNode child1;
    CMPRenderNode child2;
    CMPRenderNode *children[2];
    CMPWidget root_widget;
    CMPWidget widget1;
    CMPWidget widget2;
    CMPWidgetVTable vtable;
    TestWidget root_state;
    TestWidget state1;
    TestWidget state2;
    CMPRect root_bounds;
    CMPRect child_bounds;
    CMPInputEvent event;
    CMPWidget *target;
    CMPBool handled;

    memset(&dispatcher, 0, sizeof(dispatcher));
    CMP_TEST_OK(cmp_gesture_config_init(&config));
    config.tap_max_ms = 300u;
    config.double_tap_max_ms = 400u;
    config.long_press_ms = 500u;
    config.tap_max_distance = 5.0f;
    config.double_tap_max_distance = 10.0f;
    config.drag_start_distance = 5.0f;
    config.fling_min_velocity = 900.0f;
    CMP_TEST_OK(cmp_gesture_dispatcher_init(&dispatcher, &config));

    memset(&vtable, 0, sizeof(vtable));
    vtable.event = test_widget_event;

    CMP_TEST_OK(test_widget_init(&root_state));
    CMP_TEST_OK(test_widget_init(&state1));
    CMP_TEST_OK(test_widget_init(&state2));

    root_bounds.x = 0.0f;
    root_bounds.y = 0.0f;
    root_bounds.width = 100.0f;
    root_bounds.height = 100.0f;
    child_bounds.x = 0.0f;
    child_bounds.y = 0.0f;
    child_bounds.width = 50.0f;
    child_bounds.height = 50.0f;

    CMP_TEST_OK(setup_widget(&root_widget, &vtable, &root_state, 0, 1));
    CMP_TEST_OK(setup_widget(&widget1, &vtable, &state1, 0, 2));
    CMP_TEST_OK(setup_widget(&widget2, &vtable, &state2, 0, 3));

    CMP_TEST_OK(cmp_render_node_init(&root, &root_widget, &root_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child1, &widget1, &child_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    CMP_TEST_OK(cmp_render_node_set_children(&root, children, 2));

    memset(&event, 0, sizeof(event));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(NULL, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, NULL, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, NULL, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, NULL),
        CMP_ERR_INVALID_ARGUMENT);

    {
      CMPGestureDispatcher idle_dispatcher;

      memset(&idle_dispatcher, 0, sizeof(idle_dispatcher));
      memset(&event, 0, sizeof(event));
      event.type = CMP_INPUT_POINTER_DOWN;
      CMP_TEST_EXPECT(cmp_gesture_dispatch(&idle_dispatcher, &root, &event,
                                           &target, &handled),
                      CMP_ERR_STATE);
    }

    event.type = CMP_INPUT_TEXT;
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_gesture_test_set_contains_error(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 5));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 500, 500, 1, 6));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == NULL);
    CMP_TEST_ASSERT(dispatcher.pointer_active == CMP_FALSE);

    dispatcher.pointer_active = CMP_TRUE;
    dispatcher.active_widget = NULL;
    dispatcher.active_pointer = 1;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 0, 0, 1, 10));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_STATE);

    dispatcher.pointer_active = CMP_TRUE;
    dispatcher.active_widget = NULL;
    dispatcher.active_pointer = 1;
    CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 0, 0, 1, 11));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_STATE);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 100));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_velocity_error(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 11, 11, 1, 110));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 200));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_distance_error(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 11, 11, 1, 210));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    state2.fail_event = 1;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0, 1, 1000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 0, 1, 1010));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_event = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0, 1, 1100));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 0, 1, 1110));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    state2.fail_event = 1;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 12, 0, 1, 1120));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_event = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(test_widget_init(&state2));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 1200));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 12, 11, 1, 1210));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.drag_start_calls == 0);
    CMP_TEST_ASSERT(state2.drag_update_calls == 0);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    dispatcher.pointer_active = CMP_TRUE;
    dispatcher.active_widget = &widget2;
    dispatcher.active_pointer = 1;
    dispatcher.last_time = 200u;
    dispatcher.down_time = 100u;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 150));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_RANGE);

    dispatcher.pointer_active = CMP_TRUE;
    dispatcher.active_widget = &widget2;
    dispatcher.active_pointer = 1;
    dispatcher.last_time = 100u;
    dispatcher.down_time = 200u;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 150));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_RANGE);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 1300));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_velocity_error(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 12, 1, 1310));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 1400));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_distance_error(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 12, 1, 1410));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    state2.fail_type = CMP_INPUT_GESTURE_DRAG_END;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0, 1, 1500));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 0, 1, 1510));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 30, 0, 1, 1520));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_type = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    state2.fail_type = CMP_INPUT_GESTURE_FLING;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0, 1, 1600));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 0, 1, 1610));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 40, 0, 1, 1620));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_type = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    state2.fail_type = CMP_INPUT_GESTURE_LONG_PRESS;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 1700));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 2310));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_type = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(test_widget_init(&state2));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 2400));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 2450));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_distance_error_after(2));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 12, 11, 1, 2500));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 11, 1, 2550));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 2600));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 2650));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    dispatcher.last_tap_time = 3000u;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 12, 11, 1, 2700));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 11, 1, 2750));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    state2.fail_type = CMP_INPUT_GESTURE_DOUBLE_TAP;
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 2800));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 2850));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 12, 11, 1, 2900));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 11, 1, 2950));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_type = 0;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 3000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(cmp_gesture_test_set_reset_fail(CMP_TRUE));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 3050));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_SCROLL;
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));

    CMP_TEST_OK(test_widget_init(&state1));
    CMP_TEST_OK(test_widget_init(&state2));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 0, 0, 1, 10));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    CMP_TEST_OK(init_pointer_event(&event, CMP_INPUT_POINTER_UP, 0, 0, 1, 10));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 100));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget2);
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 2, 110));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_BUSY);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 120));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_STATE);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 10, 2, 130));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 2, 140));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 10, 10, 1, 90));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_RANGE);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 150));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state2.tap_calls == 1);
    CMP_TEST_ASSERT(state2.last_type == CMP_INPUT_GESTURE_TAP);
    CMP_TEST_ASSERT(state2.last_gesture.tap_count == 1u);

    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(test_widget_init(&state1));
    CMP_TEST_OK(test_widget_init(&state2));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 1000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 1050));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.tap_calls == 1);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 12, 11, 1, 1200));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 12, 11, 1, 1240));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.double_tap_calls == 1);
    CMP_TEST_ASSERT(state2.tap_calls == 1);
    CMP_TEST_ASSERT(state2.last_type == CMP_INPUT_GESTURE_DOUBLE_TAP);
    CMP_TEST_ASSERT(state2.last_gesture.tap_count == 2u);

    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(test_widget_init(&state2));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 2000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 2600));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.long_press_calls == 1);
    CMP_TEST_ASSERT(state2.tap_calls == 0);
    CMP_TEST_ASSERT(state2.last_type == CMP_INPUT_GESTURE_LONG_PRESS);

    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(test_widget_init(&state2));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 0, 0, 1, 3000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 6, 0, 1, 3010));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.drag_start_calls == 1);
    CMP_TEST_ASSERT(state2.last_gesture.total_x == 6.0f);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_MOVE, 16, 4, 1, 3020));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.drag_update_calls == 1);
    CMP_TEST_ASSERT(state2.last_gesture.delta_x == 10.0f);
    CMP_TEST_ASSERT(state2.last_gesture.delta_y == 4.0f);

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 36, 4, 1, 3030));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state2.drag_end_calls == 1);
    CMP_TEST_ASSERT(state2.fling_calls == 1);
    CMP_TEST_ASSERT(state2.last_type == CMP_INPUT_GESTURE_FLING);
    CMP_TEST_ASSERT(state2.last_gesture.velocity_x >= 1900.0f);

    widget2.flags = CMP_WIDGET_FLAG_HIDDEN;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(test_widget_init(&state1));
    CMP_TEST_OK(test_widget_init(&state2));

    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 4000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 4050));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(state1.tap_calls == 1);
    CMP_TEST_ASSERT(state2.tap_calls == 0);

    widget2.flags = 0;
    state2.fail_event = 1;
    CMP_TEST_OK(cmp_gesture_dispatcher_reset(&dispatcher));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10, 1, 5000));
    CMP_TEST_OK(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_OK(
        init_pointer_event(&event, CMP_INPUT_POINTER_UP, 10, 10, 1, 5050));
    CMP_TEST_EXPECT(
        cmp_gesture_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);

    state2.fail_event = 0;

    CMP_TEST_OK(cmp_gesture_dispatcher_shutdown(&dispatcher));
  }

  return 0;
}
