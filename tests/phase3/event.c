#include "cmpc/cmp_event.h"
#include "test_utils.h"

#include <string.h>

typedef struct TestWidget {
  int event_calls;
  int last_type;
  int fail_event;
  CMPBool handled;
  CMPScalar last_x;
  CMPScalar last_y;
  char last_text[64];
  cmp_usize last_text_len;
  cmp_i32 last_cursor;
  cmp_i32 last_selection;
} TestWidget;

static void test_widget_init(TestWidget *widget) {
  memset(widget, 0, sizeof(*widget));
  widget->handled = CMP_TRUE;
  widget->last_text[0] = '\0';
}

static int test_widget_event(void *ctx, const CMPInputEvent *event,
                             CMPBool *out_handled) {
  TestWidget *widget;
  const char *text;
  cmp_usize text_len;
  cmp_usize i;
  cmp_usize max_len;

  if (ctx == NULL || event == NULL || out_handled == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  widget = (TestWidget *)ctx;
  widget->event_calls += 1;
  widget->last_type = (int)event->type;
  widget->last_text_len = 0;
  widget->last_text[0] = '\0';
  widget->last_cursor = 0;
  widget->last_selection = 0;
  if (event->type == CMP_INPUT_POINTER_DOWN ||
      event->type == CMP_INPUT_POINTER_UP ||
      event->type == CMP_INPUT_POINTER_MOVE ||
      event->type == CMP_INPUT_POINTER_SCROLL) {
    widget->last_x = (CMPScalar)event->data.pointer.x;
    widget->last_y = (CMPScalar)event->data.pointer.y;
  } else if (event->type == CMP_INPUT_TEXT) {
    text = event->data.text.utf8;
    text_len = (cmp_usize)event->data.text.length;
    max_len = (cmp_usize)(sizeof(widget->last_text) - 1);
    if (text != NULL) {
      for (i = 0; i < text_len && i < max_len && text[i] != '\0'; ++i) {
        widget->last_text[i] = text[i];
      }
      widget->last_text[i] = '\0';
      widget->last_text_len = i;
    }
  } else if (event->type == CMP_INPUT_TEXT_UTF8) {
    text = event->data.text_utf8.utf8;
    text_len = event->data.text_utf8.length;
    max_len = (cmp_usize)(sizeof(widget->last_text) - 1);
    if (text != NULL) {
      for (i = 0; i < text_len && i < max_len && text[i] != '\0'; ++i) {
        widget->last_text[i] = text[i];
      }
      widget->last_text[i] = '\0';
      widget->last_text_len = i;
    }
  } else if (event->type == CMP_INPUT_TEXT_EDIT) {
    text = event->data.text_edit.utf8;
    text_len = event->data.text_edit.length;
    max_len = (cmp_usize)(sizeof(widget->last_text) - 1);
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
    return CMP_ERR_UNKNOWN;
  }

  *out_handled = widget->handled;
  return CMP_OK;
}

static void init_pointer_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 x,
                               cmp_i32 y) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.pointer.x = x;
  event->data.pointer.y = y;
}

static void init_key_event(CMPInputEvent *event, cmp_u32 type,
                           cmp_u32 key_code) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.key.key_code = key_code;
}

static void init_text_event(CMPInputEvent *event, const char *text) {
  cmp_usize i;

  memset(event, 0, sizeof(*event));
  event->type = CMP_INPUT_TEXT;
  for (i = 0;
       text[i] != '\0' && i < (cmp_usize)(sizeof(event->data.text.utf8) - 1);
       ++i) {
    event->data.text.utf8[i] = text[i];
  }
  event->data.text.utf8[i] = '\0';
  event->data.text.length = (cmp_u32)i;
}

static void init_text_utf8_event(CMPInputEvent *event, const char *text) {
  cmp_usize len;

  memset(event, 0, sizeof(*event));
  event->type = CMP_INPUT_TEXT_UTF8;
  event->data.text_utf8.utf8 = text;
  len = 0;
  if (text != NULL) {
    while (text[len] != '\0') {
      len += 1;
    }
  }
  event->data.text_utf8.length = len;
}

static void init_text_edit_event(CMPInputEvent *event, const char *text,
                                 cmp_i32 cursor, cmp_i32 selection_length) {
  cmp_usize len;

  memset(event, 0, sizeof(*event));
  event->type = CMP_INPUT_TEXT_EDIT;
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

static void init_window_event(CMPInputEvent *event, cmp_u32 type, cmp_i32 width,
                              cmp_i32 height) {
  memset(event, 0, sizeof(*event));
  event->type = type;
  event->data.window.width = width;
  event->data.window.height = height;
}

static void setup_widget(CMPWidget *widget, CMPWidgetVTable *vtable,
                         TestWidget *state, cmp_u32 flags, cmp_u32 id) {
  widget->ctx = state;
  widget->vtable = vtable;
  widget->handle.id = id;
  widget->handle.generation = 1;
  widget->flags = flags;
}

int main(void) {
  {
    CMPEventDispatcher dispatcher;
    CMPWidget widget;
    CMPWidgetVTable vtable;
    TestWidget state;
    CMPBool visible;
    CMPWidget *focused;

    memset(&dispatcher, 0, sizeof(dispatcher));
    memset(&vtable, 0, sizeof(vtable));
    test_widget_init(&state);
    vtable.event = test_widget_event;
    setup_widget(&widget, &vtable, &state, 0, 1);

    CMP_TEST_EXPECT(cmp_event_dispatcher_init(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_shutdown(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_get_focus(NULL, &focused),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_get_focus(&dispatcher, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(NULL, &widget),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(&dispatcher, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_clear_focus(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_get_focus_visible(NULL, &visible),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_get_focus_visible(&dispatcher, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus_visible(NULL, CMP_TRUE),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(&dispatcher, &widget),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(
        cmp_event_dispatcher_get_focus_visible(&dispatcher, &visible),
        CMP_ERR_STATE);
    CMP_TEST_EXPECT(
        cmp_event_dispatcher_set_focus_visible(&dispatcher, CMP_TRUE),
        CMP_ERR_STATE);

    CMP_TEST_EXPECT(cmp_event_dispatcher_shutdown(&dispatcher), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_event_dispatcher_get_focus(&dispatcher, &focused),
                    CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_event_dispatcher_clear_focus(&dispatcher),
                    CMP_ERR_STATE);

    CMP_TEST_OK(cmp_event_dispatcher_init(&dispatcher));
    CMP_TEST_EXPECT(cmp_event_dispatcher_init(&dispatcher), CMP_ERR_STATE);

    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &focused));
    CMP_TEST_ASSERT(focused == NULL);

    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(&dispatcher, &widget),
                    CMP_ERR_INVALID_ARGUMENT);
    widget.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    CMP_TEST_OK(cmp_event_dispatcher_set_focus(&dispatcher, &widget));
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &focused));
    CMP_TEST_ASSERT(focused == &widget);

    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    CMP_TEST_ASSERT(visible == CMP_FALSE);
    CMP_TEST_OK(cmp_event_dispatcher_set_focus_visible(&dispatcher, CMP_TRUE));
    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    CMP_TEST_ASSERT(visible == CMP_TRUE);

    CMP_TEST_OK(cmp_event_dispatcher_clear_focus(&dispatcher));
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &focused));
    CMP_TEST_ASSERT(focused == NULL);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &visible));
    CMP_TEST_ASSERT(visible == CMP_FALSE);

    widget.flags = CMP_WIDGET_FLAG_FOCUSABLE | CMP_WIDGET_FLAG_DISABLED;
    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(&dispatcher, &widget),
                    CMP_ERR_INVALID_ARGUMENT);
    widget.flags = CMP_WIDGET_FLAG_FOCUSABLE | CMP_WIDGET_FLAG_HIDDEN;
    CMP_TEST_EXPECT(cmp_event_dispatcher_set_focus(&dispatcher, &widget),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_event_dispatcher_shutdown(&dispatcher));
    CMP_TEST_EXPECT(cmp_event_dispatcher_shutdown(&dispatcher), CMP_ERR_STATE);
  }

  {
    CMPEventDispatcher dispatcher;
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
    CMP_TEST_OK(cmp_event_dispatcher_init(&dispatcher));

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
    setup_widget(&widget1, &vtable, &state1, CMP_WIDGET_FLAG_FOCUSABLE, 2);
    setup_widget(&widget2, &vtable, &state2, CMP_WIDGET_FLAG_FOCUSABLE, 3);

    CMP_TEST_OK(cmp_render_node_init(&root, &root_widget, &root_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child1, &widget1, &child_bounds));
    CMP_TEST_OK(cmp_render_node_init(&child2, &widget2, &child_bounds));
    children[0] = &child1;
    children[1] = &child2;
    CMP_TEST_OK(cmp_render_node_set_children(&root, children, 2));

    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget2);
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state2.event_calls == 1);
    CMP_TEST_ASSERT(state1.event_calls == 0);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &target));
    CMP_TEST_ASSERT(target == &widget2);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    state2.fail_event = 1;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state2.fail_event = 0;

    widget2.flags = CMP_WIDGET_FLAG_FOCUSABLE | CMP_WIDGET_FLAG_DISABLED;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);
    CMP_TEST_ASSERT(state1.event_calls == 1);

    widget2.flags = CMP_WIDGET_FLAG_FOCUSABLE | CMP_WIDGET_FLAG_HIDDEN;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);

    root_widget.flags = CMP_WIDGET_FLAG_DISABLED;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == NULL);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &target));
    CMP_TEST_ASSERT(target == NULL);
    root_widget.flags = 0;

    widget1.flags = CMP_WIDGET_FLAG_DISABLED;
    widget2.flags = 0;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget2);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &target));
    CMP_TEST_ASSERT(target == NULL);
    widget1.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    widget2.flags = CMP_WIDGET_FLAG_FOCUSABLE;

    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 200, 200);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == NULL);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &target));
    CMP_TEST_ASSERT(target == NULL);

    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(cmp_event_dispatch(&dispatcher, &root, &event, NULL, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    CMP_TEST_OK(cmp_event_dispatcher_set_focus(&dispatcher, &widget1));
    init_key_event(&event, CMP_INPUT_KEY_DOWN, 42);
    state1.handled = CMP_FALSE;
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);
    CMP_TEST_ASSERT(handled == CMP_FALSE);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    CMP_TEST_ASSERT(handled == CMP_TRUE);

    init_text_event(&event, "Hi");
    state1.handled = CMP_TRUE;
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state1.last_type == CMP_INPUT_TEXT);
    CMP_TEST_ASSERT(state1.last_text_len == 2);
    CMP_TEST_ASSERT(strcmp(state1.last_text, "Hi") == 0);

    init_text_utf8_event(&event, "Hello");
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state1.last_type == CMP_INPUT_TEXT_UTF8);
    CMP_TEST_ASSERT(state1.last_text_len == 5);
    CMP_TEST_ASSERT(strcmp(state1.last_text, "Hello") == 0);

    init_text_edit_event(&event, "Compose", 2, 3);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget1);
    CMP_TEST_ASSERT(handled == CMP_TRUE);
    CMP_TEST_ASSERT(state1.last_type == CMP_INPUT_TEXT_EDIT);
    CMP_TEST_ASSERT(state1.last_text_len == 7);
    CMP_TEST_ASSERT(strcmp(state1.last_text, "Compose") == 0);
    CMP_TEST_ASSERT(state1.last_cursor == 2);
    CMP_TEST_ASSERT(state1.last_selection == 3);

    init_text_event(&event, "Hi");
    state1.fail_event = 1;
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    state1.fail_event = 0;

    init_key_event(&event, CMP_INPUT_KEY_UP, 1);
    CMP_TEST_OK(cmp_event_dispatcher_clear_focus(&dispatcher));
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == NULL);
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    init_window_event(&event, CMP_INPUT_WINDOW_BLUR, 0, 0);
    CMP_TEST_OK(cmp_event_dispatcher_set_focus(&dispatcher, &widget1));
    CMP_TEST_OK(cmp_event_dispatcher_set_focus_visible(&dispatcher, CMP_TRUE));
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &root_widget);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus(&dispatcher, &target));
    CMP_TEST_ASSERT(target == NULL);
    CMP_TEST_OK(cmp_event_dispatcher_get_focus_visible(&dispatcher, &handled));
    CMP_TEST_ASSERT(handled == CMP_FALSE);

    init_window_event(&event, CMP_INPUT_WINDOW_FOCUS, 0, 0);
    root_state.fail_event = 1;
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_UNKNOWN);
    root_state.fail_event = 0;

    init_window_event(&event, CMP_INPUT_WINDOW_RESIZE, 640, 480);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &root_widget);

    event.type = 999;
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_event_dispatch(NULL, &root, &event, &target, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, NULL, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, NULL, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, NULL),
        CMP_ERR_INVALID_ARGUMENT);

    {
      CMPEventDispatcher uninit;
      memset(&uninit, 0, sizeof(uninit));
      CMP_TEST_EXPECT(
          cmp_event_dispatch(&uninit, &root, &event, &target, &handled),
          CMP_ERR_STATE);
    }

    root.child_count = 1;
    widget2.flags = CMP_WIDGET_FLAG_HIDDEN;
    child1.bounds.width = -1.0f;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 1, 1);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_RANGE);
    child_bounds.width = 50.0f;
    CMP_TEST_OK(cmp_render_node_set_bounds(&child1, &child_bounds));
    widget2.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    root.child_count = 2;

    root.child_count = 1;
    root.children = NULL;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 1, 1);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    root.children = children;
    root.child_count = 2;

    root.widget = NULL;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 1, 1);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    root.widget = &root_widget;

    children[1] = NULL;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 1, 1);
    CMP_TEST_EXPECT(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled),
        CMP_ERR_INVALID_ARGUMENT);
    children[1] = &child2;

    widget2.flags = CMP_WIDGET_FLAG_FOCUSABLE;
    widget2.vtable = NULL;
    init_pointer_event(&event, CMP_INPUT_POINTER_DOWN, 10, 10);
    CMP_TEST_OK(
        cmp_event_dispatch(&dispatcher, &root, &event, &target, &handled));
    CMP_TEST_ASSERT(target == &widget2);
    CMP_TEST_ASSERT(handled == CMP_FALSE);
    widget2.vtable = &vtable;

    CMP_TEST_OK(cmp_event_dispatcher_shutdown(&dispatcher));
  }

  {
    CMPRect rect;
    CMPBool focusable;
    CMPWidget widget;
    CMPRenderNode node;
    CMPWidget *hit;
    CMPInputEvent event;
    CMPBool handled;

    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = -1.0f;
    rect.height = 1.0f;
    CMP_TEST_EXPECT(cmp_event_test_validate_rect(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_test_validate_rect(&rect), CMP_ERR_RANGE);
    rect.width = 0.0f;
    CMP_TEST_OK(cmp_event_test_validate_rect(&rect));

    CMP_TEST_EXPECT(cmp_event_test_widget_focusable(NULL, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_event_test_widget_focusable(NULL, &focusable));
    CMP_TEST_ASSERT(focusable == CMP_FALSE);

    memset(&widget, 0, sizeof(widget));
    memset(&node, 0, sizeof(node));
    node.widget = &widget;
    node.bounds.x = 0.0f;
    node.bounds.y = 0.0f;
    node.bounds.width = 10.0f;
    node.bounds.height = 10.0f;

    CMP_TEST_EXPECT(cmp_event_test_hit_test(NULL, 0.0f, 0.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_test_hit_test(&node, 0.0f, 0.0f, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(cmp_event_test_set_force_contains_error(2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_event_test_set_force_contains_error(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_event_test_hit_test(&node, 1.0f, 1.0f, &hit),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_event_test_set_force_contains_error(CMP_FALSE));

    memset(&event, 0, sizeof(event));
    event.type = CMP_INPUT_POINTER_DOWN;
    CMP_TEST_EXPECT(cmp_event_test_dispatch_to_widget(&widget, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_test_dispatch_to_widget(NULL, &event, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_event_test_dispatch_to_widget(&widget, NULL, &handled),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  return 0;
}
