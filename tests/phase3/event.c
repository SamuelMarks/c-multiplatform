#include "m3/m3_event.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestWidget {
  int event_calls;
  int last_type;
  int fail_event;
  M3Bool handled;
  M3Scalar last_x;
  M3Scalar last_y;
  char last_text[64];
  m3_usize last_text_len;
  m3_i32 last_cursor;
  m3_i32 last_selection;
} TestWidget;

static void test_widget_init(TestWidget *widget) {
  memset(widget, 0, sizeof(*widget));
  widget->handled = M3_TRUE;
  widget->last_text[0] = '\0';
}

static int test_widget_event(void *ctx, const M3InputEvent *event,
                             M3Bool *out_handled) {
  TestWidget *widget;
  const char *text;
  m3_usize text_len;
  m3_usize i;
  m3_usize max_len;

  if (ctx == NULL || event == NULL || out_handled == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  widget = (TestWidget *)ctx;
  widget->event_calls += 1;
  widget->last_type = (int)event->type;
  widget->last_text_len = 0;
  widget->last_text[0] = '\0';
  widget->last_cursor = 0;
  widget->last_selection = 0;
  if (event->type == M3_INPUT_POINTER_DOWN ||
      event->type == M3_INPUT_POINTER_UP ||
      event->type == M3_INPUT_POINTER_MOVE ||
      event->type == M3_INPUT_POINTER_SCROLL) {
    widget->last_x = (M3Scalar)event->data.pointer.x;
    widget->last_y = (M3Scalar)event->data.pointer.y;
  } else if (event->type == M3_INPUT_TEXT) {
    text = event->data.text.utf8;
    text_len = (m3_usize)event->data.text.length;
    max_len = (m3_usize)(sizeof(widget->last_text) - 1);
    if (text != NULL) {
      for (i = 0; i < text_len && i < max_len && text[i] != '\0'; ++i) {
        widget->last_text[i] = text[i];
      }
      widget->last_text[i] = '\0';
      widget->last_text_len = i;
    }
  } else if (event->type == M3_INPUT_TEXT_UTF8) {
    text = event->data.text_utf8.utf8;
    text_len = event->data.text_utf8.length;
    max_len = (m3_usize)(sizeof(widget->last_text) - 1);
    if (text != NULL) {
      for (i = 0; i < text_len && i < max_len && text[i] != '\0'; ++i) {
        widget->last_text[i] = text[i];
      }
      widget->last_text[i] = '\0';
      widget->last_text_len = i;
    }
  } else if (event->type == M3_INPUT_TEXT_EDIT) {
    text = event->data.text_edit.utf8;
    text_len = event->data.text_edit.length;
    max_len = (m3_usize)(sizeof(widget->last_text) - 1);
    if (text != NULL) {
      for (i = 0; i < text_len && i < max_len && text[i] != '\0'; ++i) {
        widget->last_text[i] = text[i];
      }
      widget->last_text[i] = '\0';
      widget->last_text_len = i;
    }
    widget->last_cursor = event->data.text_edit.cursor;
    widget->last_selection = event->data.text_edit.selection_length;
  }

  if (widget->fail_event) {
    return M3_ERR_UNKNOWN;
  }

  *out_handled = widget->handled;
  return M3_OK;
}

static void init_pointer_event(M3InputEvent *event, m3_u32 type, m3_i32 x,
                               m3_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

static void init_key_event(M3InputEvent *event, m3_u32 type, m3_u32 key_code) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.key.key_code = key_code;
}

static void init_text_event(M3InputEvent *event, const char *text) {
  m3_usize i;

  memset(event, 0, sizeof(*event));
  event->type = M3_INPUT_TEXT;
  for (i = 0;
       text[i] != '\0' && i < (m3_usize)(sizeof(event->data.text.utf8) - 1);
       ++i) {
    event->data.text.utf8[i] = text[i];
  }
  event->data.text.utf8[i] = '\0';
  event->data.text.length = (m3_u32)i;
}

static void init_text_utf8_event(M3InputEvent *event, const char *text) {
  m3_usize len;

  memset(event, 0, sizeof(*event));
  event->type = M3_INPUT_TEXT_UTF8;
  event->data.text_utf8.utf8 = text;
  len = 0;
  if (text != NULL) {
    while (text[len] != '\0') {
      len += 1;
    }
  }
  event->data.text_utf8.length = len;
}

static void init_text_edit_event(M3InputEvent *event, const char *text,
                                 m3_i32 cursor, m3_i32 selection_length) {
  m3_usize len;

  memset(event, 0, sizeof(*event));
  event->type = M3_INPUT_TEXT_EDIT;
  event->data.text_edit.utf8 = text;
  len = 0;
  if (text != NULL) {
    while (text[len] != '\0') {
      len += 1;
    }
  }
  event->data.text_edit.length = len;
  event->data.text_edit.cursor = cursor;
  event->data.text_edit.selection_length = selection_length;
}

static void init_window_event(M3InputEvent *event, m3_u32 type, m3_i32 width,
                              m3_i32 height) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.window.width = width;
  event->data.window.height = height;
}

static void setup_widget(M3Widget *widget, M3WidgetVTable *vtable,
                         TestWidget *state, m3_u32 flags, m3_u32 id) {
  widget->ctx = state;
  widget->vtable = vtable;
  widget->handle.id = id;
  widget->handle.generation = 1;
  widget->flags = flags;
}

int main(void) {
  {
    M3EventDispatcher dispatcher;
    M3Widget widget;
    M3WidgetVTable vtable;
    TestWidget state;
    M3Bool visible;
    M3Widget *focused;

    memset(&dispatcher, 0, sizeof(dispatcher));
    memset(&vtable, 0, sizeof(vtable));
    test_widget_init(&state);
    vtable.event = test_widget_event;
    setup_widget(&widget, &vtable, &state, 0, 1);

    M3_TEST_EXPECT(m3_event_dispatcher_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus(NULL, &focused),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus(&dispatcher, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(NULL, &widget),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(&dispatcher, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_clear_focus(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus_visible(NULL, &visible),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus_visible(&dispatcher, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus_visible(NULL, M3_TRUE),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(&dispatcher, &widget),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus_visible(&dispatcher, &visible),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus_visible(&dispatcher, M3_TRUE),
                   M3_ERR_STATE);

    M3_TEST_EXPECT(m3_event_dispatcher_shutdown(&dispatcher), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_event_dispatcher_get_focus(&dispatcher, &focused),
                   M3_ERR_STATE);
    M3_TEST_EXPECT(m3_event_dispatcher_clear_focus(&dispatcher), M3_ERR_STATE);

    M3_TEST_OK(m3_event_dispatcher_init(&dispatcher));
    M3_TEST_EXPECT(m3_event_dispatcher_init(&dispatcher), M3_ERR_STATE);

    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &focused));
    M3_TEST_ASSERT(focused == NULL);

    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(&dispatcher, &widget),
                   M3_ERR_INVALID_ARGUMENT);
    widget.flags = M3_WIDGET_FLAG_FOCUSABLE;
    M3_TEST_OK(m3_event_dispatcher_set_focus(&dispatcher, &widget));
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &focused));
    M3_TEST_ASSERT(focused == &widget);

    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    M3_TEST_ASSERT(visible == M3_FALSE);
    M3_TEST_OK(m3_event_dispatcher_set_focus_visible(&dispatcher, M3_TRUE));
    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    M3_TEST_ASSERT(visible == M3_TRUE);

    M3_TEST_OK(m3_event_dispatcher_clear_focus(&dispatcher));
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &focused));
    M3_TEST_ASSERT(focused == NULL);
    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    M3_TEST_ASSERT(visible == M3_FALSE);

    widget.flags = M3_WIDGET_FLAG_FOCUSABLE | M3_WIDGET_FLAG_DISABLED;
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(&dispatcher, &widget),
                   M3_ERR_INVALID_ARGUMENT);
    widget.flags = M3_WIDGET_FLAG_FOCUSABLE | M3_WIDGET_FLAG_HIDDEN;
    M3_TEST_EXPECT(m3_event_dispatcher_set_focus(&dispatcher, &widget),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_event_dispatcher_shutdown(&dispatcher));
    M3_TEST_EXPECT(m3_event_dispatcher_shutdown(&dispatcher), M3_ERR_STATE);
  }

  {
    M3EventDispatcher dispatcher;
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
    M3_TEST_OK(m3_event_dispatcher_init(&dispatcher));

    memset(&vtable, 0, sizeof(vtable));
    vtable.event = test_widget_event;
    test_widget_init(&root_state);
    test_widget_init(&state1);
    test_widget_init(&state2);

    root_bounds.x = 0.0f;
    root_bounds.y = 0.0f;
    root_bounds.width = 100.0f;
    root_bounds.height = 100.0f;
    child_bounds.x = 0.0f;
    child_bounds.y = 0.0f;
    child_bounds.width = 50.0f;
    child_bounds.height = 50.0f;

    setup_widget(&root_widget, &vtable, &root_state, 0, 1);
    setup_widget(&widget1, &vtable, &state1, M3_WIDGET_FLAG_FOCUSABLE, 2);
    setup_widget(&widget2, &vtable, &state2, M3_WIDGET_FLAG_FOCUSABLE, 3);

    M3_TEST_OK(m3_render_node_init(&root, &root_widget, &root_bounds));
    M3_TEST_OK(m3_render_node_init(&child1, &widget1, &child_bounds));
    M3_TEST_OK(m3_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    M3_TEST_OK(m3_render_node_set_children(&root, children, 2));

    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget2);
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state2.event_calls == 1);
    M3_TEST_ASSERT(state1.event_calls == 0);
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &target));
    M3_TEST_ASSERT(target == &widget2);
    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    state2.fail_event = 1;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_UNKNOWN);
    state2.fail_event = 0;

    widget2.flags = M3_WIDGET_FLAG_FOCUSABLE | M3_WIDGET_FLAG_DISABLED;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);
    M3_TEST_ASSERT(state1.event_calls == 1);

    widget2.flags = M3_WIDGET_FLAG_FOCUSABLE | M3_WIDGET_FLAG_HIDDEN;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);

    root_widget.flags = M3_WIDGET_FLAG_DISABLED;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == NULL);
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &target));
    M3_TEST_ASSERT(target == NULL);
    root_widget.flags = 0;

    widget1.flags = M3_WIDGET_FLAG_DISABLED;
    widget2.flags = 0;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget2);
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &target));
    M3_TEST_ASSERT(target == NULL);
    widget1.flags = M3_WIDGET_FLAG_FOCUSABLE;
    widget2.flags = M3_WIDGET_FLAG_FOCUSABLE;

    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 200, 200);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == NULL);
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &target));
    M3_TEST_ASSERT(target == NULL);

    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(m3_event_dispatch(&dispatcher, &root, &event, NULL, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    M3_TEST_OK(m3_event_dispatcher_set_focus(&dispatcher, &widget1));
    init_key_event(&event, M3_INPUT_KEY_DOWN, 42);
    state1.handled = M3_FALSE;
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);
    M3_TEST_ASSERT(handled == M3_FALSE);
    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    M3_TEST_ASSERT(handled == M3_TRUE);

    init_text_event(&event, "Hi");
    state1.handled = M3_TRUE;
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state1.last_type == M3_INPUT_TEXT);
    M3_TEST_ASSERT(state1.last_text_len == 2);
    M3_TEST_ASSERT(strcmp(state1.last_text, "Hi") == 0);

    init_text_utf8_event(&event, "Hello");
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state1.last_type == M3_INPUT_TEXT_UTF8);
    M3_TEST_ASSERT(state1.last_text_len == 5);
    M3_TEST_ASSERT(strcmp(state1.last_text, "Hello") == 0);

    init_text_edit_event(&event, "Compose", 2, 3);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget1);
    M3_TEST_ASSERT(handled == M3_TRUE);
    M3_TEST_ASSERT(state1.last_type == M3_INPUT_TEXT_EDIT);
    M3_TEST_ASSERT(state1.last_text_len == 7);
    M3_TEST_ASSERT(strcmp(state1.last_text, "Compose") == 0);
    M3_TEST_ASSERT(state1.last_cursor == 2);
    M3_TEST_ASSERT(state1.last_selection == 3);

    init_text_event(&event, "Hi");
    state1.fail_event = 1;
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_UNKNOWN);
    state1.fail_event = 0;

    init_key_event(&event, M3_INPUT_KEY_UP, 1);
    M3_TEST_OK(m3_event_dispatcher_clear_focus(&dispatcher));
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == NULL);
    M3_TEST_ASSERT(handled == M3_FALSE);

    init_window_event(&event, M3_INPUT_WINDOW_BLUR, 0, 0);
    M3_TEST_OK(m3_event_dispatcher_set_focus(&dispatcher, &widget1));
    M3_TEST_OK(m3_event_dispatcher_set_focus_visible(&dispatcher, M3_TRUE));
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &root_widget);
    M3_TEST_OK(m3_event_dispatcher_get_focus(&dispatcher, &target));
    M3_TEST_ASSERT(target == NULL);
    M3_TEST_OK(m3_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    M3_TEST_ASSERT(handled == M3_FALSE);

    init_window_event(&event, M3_INPUT_WINDOW_FOCUS, 0, 0);
    root_state.fail_event = 1;
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_UNKNOWN);
    root_state.fail_event = 0;

    init_window_event(&event, M3_INPUT_WINDOW_RESIZE, 640, 480);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &root_widget);

    event.type = 999;
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_event_dispatch(NULL, &root, &event, &target, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, NULL, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, NULL, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_dispatch(&dispatcher, &root, &event, &target, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    {
      M3EventDispatcher uninit;
      memset(&uninit, 0, sizeof(uninit));
      M3_TEST_EXPECT(
          m3_event_dispatch(&uninit, &root, &event, &target, &handled),
          M3_ERR_STATE);
    }

    root.child_count = 1;
    widget2.flags = M3_WIDGET_FLAG_HIDDEN;
    child1.bounds.width = -1.0f;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 1, 1);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_RANGE);
    child_bounds.width = 50.0f;
    M3_TEST_OK(m3_render_node_set_bounds(&child1, &child_bounds));
    widget2.flags = M3_WIDGET_FLAG_FOCUSABLE;
    root.child_count = 2;

    root.child_count = 1;
    root.children = NULL;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 1, 1);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    root.children = children;
    root.child_count = 2;

    root.widget = NULL;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 1, 1);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    root.widget = &root_widget;

    children[1] = NULL;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 1, 1);
    M3_TEST_EXPECT(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        M3_ERR_INVALID_ARGUMENT);
    children[1] = &child2;

    widget2.flags = M3_WIDGET_FLAG_FOCUSABLE;
    widget2.vtable = NULL;
    init_pointer_event(&event, M3_INPUT_POINTER_DOWN, 10, 10);
    M3_TEST_OK(
        m3_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    M3_TEST_ASSERT(target == &widget2);
    M3_TEST_ASSERT(handled == M3_FALSE);
    widget2.vtable = &vtable;

    M3_TEST_OK(m3_event_dispatcher_shutdown(&dispatcher));
  }

  {
    M3Rect rect;
    M3Bool focusable;
    M3Widget widget;
    M3RenderNode node;
    M3Widget *hit;
    M3InputEvent event;
    M3Bool handled;

    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    M3_TEST_EXPECT(m3_event_test_validate_rect(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_test_validate_rect(&rect), M3_ERR_RANGE);
    rect.width = 0.0f;
    M3_TEST_OK(m3_event_test_validate_rect(&rect));

    M3_TEST_EXPECT(m3_event_test_widget_focusable(NULL, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_event_test_widget_focusable(NULL, &focusable));
    M3_TEST_ASSERT(focusable == M3_FALSE);

    memset(&widget, 0, sizeof(widget));
    memset(&node, 0, sizeof(node));
    node.widget = &widget;
    node.bounds.x = 0.0f;
    node.bounds.y = 0.0f;
    node.bounds.width = 10.0f;
    node.bounds.height = 10.0f;

    M3_TEST_EXPECT(m3_event_test_hit_test(NULL, 0.0f, 0.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_test_hit_test(&node, 0.0f, 0.0f, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_event_test_set_force_contains_error(2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_event_test_set_force_contains_error(M3_TRUE));
    M3_TEST_EXPECT(m3_event_test_hit_test(&node, 1.0f, 1.0f, &hit),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_event_test_set_force_contains_error(M3_FALSE));

    memset(&event, 0, sizeof(event));
    event.type = M3_INPUT_POINTER_DOWN;
    M3_TEST_EXPECT(m3_event_test_dispatch_to_widget(&widget, &event, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_test_dispatch_to_widget(NULL, &event, &handled),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_event_test_dispatch_to_widget(&widget, NULL, &handled),
                   M3_ERR_INVALID_ARGUMENT);
  }

  return 0;
}
