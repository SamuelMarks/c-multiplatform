/* clang-format off */
#include "ui_keyboard_responder.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_action_called = 0;
static struct ui_dom_node *g_action_node = NULL;
static void *g_action_user_data = NULL;

static enum ui_error test_action_cb(struct ui_dom_node *node, void *user_data) {
  g_action_called++;
  g_action_node = node;
  g_action_user_data = user_data;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static enum ui_error test_keyboard_responder_create_destroy() {
  struct ui_keyboard_responder *responder = NULL;
  enum ui_error err;

  err = ui_keyboard_responder_create(NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_keyboard_responder_create(&responder);
  assert(err == UI_ERROR_NONE);
  assert(responder != NULL);

  ui_keyboard_responder_destroy(NULL);
  ui_keyboard_responder_destroy(responder);
  printf("test_keyboard_responder_create_destroy passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_keyboard_responder_bind_and_handle() {
  struct ui_keyboard_responder *responder = NULL;
  struct ui_dom_node *button_node = NULL;
  struct ui_dom_node *generic_node = NULL;
  struct ui_event ev;
  int handled = 0;
  int user_data_val = 42;

  ui_keyboard_responder_create(&responder);

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &button_node);
  ui_dom_node_set_tag_name(button_node, "button");

  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &generic_node);
  ui_dom_node_set_tag_name(generic_node, "div");
  ui_dom_node_set_attribute(generic_node, "role", "button");

  /* Error checks */
  assert(ui_keyboard_responder_bind_key(NULL, "button", UI_KEY_SPACE,
                                        test_action_cb,
                                        NULL) == UI_ERROR_INVALID_ARGUMENT);

  assert(ui_keyboard_responder_handle_event(responder, button_node, &ev,
                                            NULL) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_keyboard_responder_handle_event(NULL, button_node, &ev, &handled) ==
         UI_ERROR_INVALID_ARGUMENT);

  {
    struct ui_dom_node text_node;
    memset(&text_node, 0, sizeof(text_node));
    text_node.type = UI_DOM_NODE_TYPE_TEXT;

    handled = -1;
    ui_keyboard_responder_handle_event(responder, NULL, &ev, &handled);
    assert(handled == 0);

    handled = -1;
    ui_keyboard_responder_handle_event(responder, &text_node, &ev, &handled);
    assert(handled == 0);
  }

  /* Bind Space key on "button" */
  ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE,
                                 test_action_cb, &user_data_val);

  /* Test 1: Button tag with Space key */
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;

  g_action_called = 0;
  ui_keyboard_responder_handle_event(responder, button_node, &ev, &handled);
  assert(handled == 1);
  assert(g_action_called == 1);
  assert(g_action_node == button_node);
  assert(g_action_user_data == &user_data_val);

  /* Test 2: Role="button" with Space key */
  g_action_called = 0;
  handled = 0;
  ui_keyboard_responder_handle_event(responder, generic_node, &ev, &handled);
  assert(handled == 1);
  assert(g_action_called == 1);
  assert(g_action_node == generic_node);

  /* Test 3: Unhandled key (Enter) */
  ev.event_data.keyboard.key_code = UI_KEY_ENTER;
  g_action_called = 0;
  handled = 0;
  ui_keyboard_responder_handle_event(responder, button_node, &ev, &handled);
  assert(handled == 0);
  assert(g_action_called == 0);

  /* Test 4: Bidi failure fallback (UI_KEY_A should fallback) */
  ev.event_data.keyboard.key_code = 'A';
  g_action_called = 0;
  handled = 0;
  ui_keyboard_responder_handle_event(responder, button_node, &ev, &handled);
  assert(handled == 0);
  assert(g_action_called == 0);

  /* Test 5: Unhandled event type (KEY_UP) */
  ev.type = UI_EVENT_KEY_UP;
  ev.event_data.keyboard.key_code = UI_KEY_SPACE;
  g_action_called = 0;
  handled = 0;
  ui_keyboard_responder_handle_event(responder, button_node, &ev, &handled);
  assert(handled == 0);
  assert(g_action_called == 0);

  ui_dom_node_destroy(button_node);
  ui_dom_node_destroy(generic_node);
  ui_keyboard_responder_destroy(responder);
  printf("test_keyboard_responder_bind_and_handle passed\n");
  return UI_ERROR_NONE;
}

static enum ui_error test_oom(void) {
  struct ui_keyboard_responder *responder = NULL;
  enum ui_error err;
  int i;

  /* Creation OOM */
  g_malloc_fail_countdown = 0;
  err = ui_keyboard_responder_create(&responder);
  g_malloc_fail_countdown = -1;
  assert(err == UI_ERROR_OUT_OF_MEMORY);

  ui_keyboard_responder_create(&responder);

  /* Bind OOM */
  for (i = 0; i < 2; i++) {
    g_malloc_fail_countdown = i;
    err = ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE,
                                         test_action_cb, NULL);
    g_malloc_fail_countdown = -1;
    assert(err == UI_ERROR_OUT_OF_MEMORY);
  }

  ui_keyboard_responder_destroy(responder);
}

int main() {
  test_keyboard_responder_create_destroy();
  test_keyboard_responder_bind_and_handle();
  test_oom();
  printf("All test_ui_keyboard_responder passed\n");
  return 0;
}
