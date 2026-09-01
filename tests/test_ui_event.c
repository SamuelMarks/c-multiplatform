extern int g_malloc_fail_countdown;
/* clang-format off */
#include "../include/ui_dom_node.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include "../include/ui_focus_manager.h"
#include "../include/ui_layout.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

struct test_state {
  int enter_count;
  int leave_count;
  int move_count;
  int click_count;
  int dblclick_count;
  int down_count;
  int up_count;
  int context_menu_count;
  int wheel_count;
  int key_down_count;
  int key_up_count;
  int key_press_count;
};

static ui_error_t handle_event(struct ui_dom_node *node,
                               const struct ui_event *event, void *user_data) {
  struct test_state *state = (struct test_state *)user_data;
  (void)node;
  switch (event->type) {
  case UI_EVENT_MOUSE_ENTER:
    state->enter_count++;
    break;
  case UI_EVENT_MOUSE_LEAVE:
    state->leave_count++;
    break;
  case UI_EVENT_MOUSE_MOVE:
    state->move_count++;
    break;
  case UI_EVENT_MOUSE_DOWN:
    state->down_count++;
    break;
  case UI_EVENT_MOUSE_UP:
    state->up_count++;
    break;
  case UI_EVENT_CLICK:
    state->click_count++;
    break;
  case UI_EVENT_DBLCLICK:
    state->dblclick_count++;
    break;
  case UI_EVENT_CONTEXT_MENU:
    state->context_menu_count++;
    break;
  case UI_EVENT_MOUSE_WHEEL:
    state->wheel_count++;
    break;
  case UI_EVENT_KEY_DOWN:
    state->key_down_count++;
    break;
  case UI_EVENT_KEY_UP:
    state->key_up_count++;
    break;
  case UI_EVENT_KEY_PRESS:
    state->key_press_count++;
    break;
  default:
    break;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_event_dispatch(const struct ui_layout_node *,
                             const struct ui_event *, struct ui_mouse_state *,
                             void *);

int g_mock_focus_fail = 0;
static void trigger_null_branches(void) {
  struct ui_mouse_state mouse_state = {NULL, NULL, 0.0, 0};
  struct ui_layout_node root_layout;
  struct ui_event ev;
  memset(&root_layout, 0, sizeof(root_layout));
  ev.type = UI_EVENT_MOUSE_MOVE;

  /* Missing params */
  ui_event_dispatch(NULL, &ev, &mouse_state, NULL);
  ui_event_dispatch(&root_layout, NULL, &mouse_state, NULL);
  ui_event_dispatch(&root_layout, &ev, NULL, NULL);
}

static ui_error_t failing_handler(struct ui_dom_node *node,
                                  const struct ui_event *event,
                                  void *user_data) {
  (void)node;
  (void)event;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static void test_failing_events(void) {
  struct ui_dom_node *root_dom = NULL;
  struct ui_layout_node root_layout;
  struct ui_mouse_state mouse_state = {NULL, NULL, 0.0, 0};
  struct ui_event ev;
  struct ui_focus_manager *focus_mgr = NULL;

  ui_focus_manager_create(&focus_mgr);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_dom);
  ui_dom_node_set_attribute(root_dom, "tabindex", "0");
  ui_dom_node_set_attribute(root_dom, "tabindex", "0");

  memset(&root_layout, 0, sizeof(root_layout));
  root_layout.dom_node = root_dom;
  root_layout.width = 100;
  root_layout.height = 100;

  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_LEAVE,
                                 failing_handler, NULL);
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ev.event_data.mouse.x = 150;
  ev.event_data.mouse.y = 150;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_LEAVE,
                                    failing_handler);

  mouse_state.hovered_node = NULL;
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_ENTER,
                                 failing_handler, NULL);
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_ENTER,
                                    failing_handler);

  mouse_state.hovered_node = &root_layout;
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_MOVE, failing_handler,
                                 NULL);
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_MOVE,
                                    failing_handler);

  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_DOWN, failing_handler,
                                 NULL);
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_DOWN,
                                    failing_handler);

  ui_dom_node_add_event_listener(root_dom, UI_EVENT_CONTEXT_MENU,
                                 failing_handler, NULL);
  ev.event_data.mouse.button = 1;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_CONTEXT_MENU,
                                    failing_handler);

  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_UP, failing_handler,
                                 NULL);
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_UP,
                                    failing_handler);

  mouse_state.active_node = &root_layout;
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_CLICK, failing_handler,
                                 NULL);
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_CLICK, failing_handler);

  mouse_state.active_node = &root_layout;
  mouse_state.click_count = 1;
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_DBLCLICK, failing_handler,
                                 NULL);
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_DBLCLICK,
                                    failing_handler);

  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_WHEEL,
                                 failing_handler, NULL);
  ev.type = UI_EVENT_MOUSE_WHEEL;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_MOUSE_WHEEL,
                                    failing_handler);

  ui_focus_manager_request_focus(focus_mgr, root_dom);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_KEY_DOWN, failing_handler,
                                 NULL);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = 32;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ui_dom_node_remove_event_listener(root_dom, UI_EVENT_KEY_DOWN,
                                    failing_handler);

  ev.event_data.keyboard.key_code = 9;
  g_malloc_fail_countdown = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  g_malloc_fail_countdown = -1;

  g_mock_focus_fail = 1;
  ev.type = UI_EVENT_KEY_DOWN;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  g_mock_focus_fail = 0;

  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus_mgr);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_dom);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}

int main(void) {
  struct ui_dom_node *root_dom = NULL;
  struct ui_layout_node root_layout;
  struct ui_mouse_state mouse_state = {NULL, NULL, 0.0, 0};
  struct ui_focus_manager *focus_mgr = NULL;
  struct test_state state;
  struct ui_event ev;
  struct ui_dom_node *child1 = NULL, *child2 = NULL;
  struct ui_dom_node *focused = NULL;
  struct ui_layout_node layout_no_dom;

  memset(&state, 0, sizeof(state));

  trigger_null_branches();
  test_failing_events();

  ui_focus_manager_create(&focus_mgr);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_dom);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_ENTER,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_LEAVE,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_MOVE,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_DOWN,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_UP,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_CLICK,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_DBLCLICK,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_CONTEXT_MENU,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_WHEEL,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_KEY_DOWN,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_KEY_UP,
                                 (ui_event_handler_t)handle_event, &state);
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_KEY_PRESS,
                                 (ui_event_handler_t)handle_event, &state);
  /* NULL handler test */
  ui_dom_node_add_event_listener(root_dom, UI_EVENT_MOUSE_MOVE, NULL, NULL);

  memset(&root_layout, 0, sizeof(root_layout));
  root_layout.dom_node = root_dom;
  root_layout.x = 10.0f;
  root_layout.y = 10.0f;
  root_layout.width = 100.0f;
  root_layout.height = 100.0f;

  /* Move outside (y < node->y and y > node->y + height) */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 5;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);

  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 200;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);

  /* Move outside */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 5;
  ev.event_data.mouse.y = 5;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);

  /* Move inside -> Enter */
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.enter_count != 1 || state.move_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Move inside -> Move */
  ev.event_data.mouse.x = 60;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.move_count != 2) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Mouse down */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0; /* Left */
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.down_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Mouse up -> Click */
  ev.type = UI_EVENT_MOUSE_UP;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.up_count != 1 || state.click_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Double click test */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ev.type = UI_EVENT_MOUSE_UP;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.click_count != 2 || state.dblclick_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Right click context menu */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 1; /* Right */
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.context_menu_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }
  ev.type = UI_EVENT_MOUSE_UP;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);

  /* Wheel */
  ev.type = UI_EVENT_MOUSE_WHEEL;
  ev.event_data.mouse.wheel_y = 1.0f;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.wheel_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Mouse leave */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 200;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.leave_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Keyboard focus tests */
  ui_focus_manager_request_focus(focus_mgr, root_dom);

  /* Key Down */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  ev.event_data.keyboard.modifiers = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.key_down_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Key Down with NULL focus_mgr */
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);

  /* Key Press */
  ev.type = UI_EVENT_KEY_PRESS;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.key_press_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Key Up */
  ev.type = UI_EVENT_KEY_UP;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  if (state.key_up_count != 1) {
    printf("Failed at line %d\n", __LINE__);
    return 1;
  }

  /* Tab Focus Routing Tests */
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child1);
  ui_dom_node_set_attribute(child1, "tabindex", "0");
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child2);
  ui_dom_node_set_attribute(child2, "tabindex", "0");
  ui_dom_node_append_child(root_dom, child1);
  ui_dom_node_append_child(root_dom, child2);

  /* Reset focus */
  ui_focus_manager_request_focus(focus_mgr, NULL);

  /* Press Tab (forward) */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_TAB;
  ev.event_data.keyboard.modifiers = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ui_focus_manager_get_focused_node(focus_mgr, &focused);
  if (focused != child1) {
    printf("Tab routing 1 failed\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Press Tab with no root_layout dom_node */
  root_layout.dom_node = NULL;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  root_layout.dom_node = root_dom;

  /* Press Tab again (forward) */
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ui_focus_manager_get_focused_node(focus_mgr, &focused);
  if (focused != child2) {
    printf("Tab routing 2 failed\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Press Shift+Tab (backward) */
  ev.event_data.keyboard.modifiers = UI_MODIFIER_SHIFT;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ui_focus_manager_get_focused_node(focus_mgr, &focused);
  if (focused != child1) {
    printf("Shift+Tab routing failed\n");
    {
      printf("Failed at line %d\n", __LINE__);
      return 1;
    }
  }

  /* Tests with layout node having NULL dom_node */
  memset(&layout_no_dom, 0, sizeof(layout_no_dom));
  layout_no_dom.dom_node = NULL;
  layout_no_dom.x = 300.0f;
  layout_no_dom.y = 300.0f;
  layout_no_dom.width = 100.0f;
  layout_no_dom.height = 100.0f;

  /* Move into layout_no_dom */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 350;
  ev.event_data.mouse.y = 350;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Mouse down in layout_no_dom */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Mouse up in layout_no_dom */
  ev.type = UI_EVENT_MOUSE_UP;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Context menu in layout_no_dom */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 1;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Wheel in layout_no_dom */
  ev.type = UI_EVENT_MOUSE_WHEEL;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Move out of layout_no_dom */
  ev.type = UI_EVENT_MOUSE_MOVE;
  ev.event_data.mouse.x = 500;
  ev.event_data.mouse.y = 500;
  ui_event_dispatch(&layout_no_dom, &ev, &mouse_state, focus_mgr);

  /* Down then move out then up */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);

  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 500;
  ev.event_data.mouse.y = 500;
  ui_event_dispatch(&root_layout, &ev, &mouse_state,
                    focus_mgr); /* trigger branch: hit != active_node */

  /* Missed branches: events outside bounds (hit == NULL) */
  ev.type = UI_EVENT_MOUSE_DOWN;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ev.event_data.mouse.button = 1;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  ev.type = UI_EVENT_MOUSE_WHEEL;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  /* Unhandled event type to trigger final else branch */
  ev.type = UI_EVENT_CLICK;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  /* Tab with NULL focus_mgr */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_TAB;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, NULL);
  /* MOUSE_UP with hit != state->active_node */
  mouse_state.active_node = NULL;
  ev.type = UI_EVENT_MOUSE_UP;
  ev.event_data.mouse.x = 50;
  ev.event_data.mouse.y = 50;
  ev.event_data.mouse.button = 0;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);

  g_mock_focus_fail = 1;
  ev.type = UI_EVENT_KEY_DOWN;
  ui_event_dispatch(&root_layout, &ev, &mouse_state, focus_mgr);
  g_mock_focus_fail = 0;

  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus_mgr);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(root_dom);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  printf("All UI Event tests passed.\n");
  return 0;
}
