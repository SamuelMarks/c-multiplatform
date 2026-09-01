/* clang-format off */
#include "ui_keyboard_responder.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_callback_called = 0;
static void *g_callback_user_data = NULL;
static struct ui_dom_node *g_callback_node = NULL;

static ui_error_t mock_callback(struct ui_dom_node *node, void *user_data) {
  g_callback_called++;
  g_callback_node = node;
  g_callback_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t run_normal_tests(void) {
  struct ui_keyboard_responder *responder = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_event event;
  int handled = 0;
  ui_error_t rc;

  printf("Testing ui_keyboard_responder_create...\n");
  rc = ui_keyboard_responder_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_keyboard_responder_create(&responder);
  if (rc != UI_ERROR_NONE || !responder)
    return rc;

  printf("Testing ui_keyboard_responder_bind_key...\n");
  rc = ui_keyboard_responder_bind_key(NULL, "button", UI_KEY_SPACE,
                                      mock_callback, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_keyboard_responder_bind_key(responder, NULL, UI_KEY_SPACE,
                                      mock_callback, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE, NULL,
                                      NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE,
                                      mock_callback, (void *)0x123);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Force realloc to trigger new capacity */
  {
    int i;
    for (i = 0; i < 10; ++i) {
      char buf[32];
      sprintf(buf, "tag%d", i);
      rc = ui_keyboard_responder_bind_key(responder, buf, UI_KEY_ENTER,
                                          mock_callback, NULL);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  }

  printf("Testing ui_keyboard_responder_handle_event...\n");
  rc = ui_keyboard_responder_handle_event(NULL, node, &event, &handled);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  /* No event */
  rc = ui_keyboard_responder_handle_event(responder, node, NULL, &handled);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  /* No node */
  rc = ui_keyboard_responder_handle_event(responder, NULL, &event, &handled);
  if (rc != UI_ERROR_NONE || handled != 0)
    return UI_ERROR_UNKNOWN;

  /* Create node */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Wrong event type */
  memset(&event, 0, sizeof(event));
  event.type = UI_EVENT_KEY_UP;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE || handled != 0)
    return UI_ERROR_UNKNOWN;

  /* Right event type, no match (node has no tag or role yet) */
  event.type = UI_EVENT_KEY_DOWN;
  event.event_data.keyboard.key_code = UI_KEY_SPACE;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE || handled != 0)
    return UI_ERROR_UNKNOWN;

  /* Set tag to button */
  rc = ui_dom_node_set_tag_name(node, "button");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Should match button + space */
  g_callback_called = 0;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (handled != 1)
    return UI_ERROR_UNKNOWN;
  if (g_callback_called != 1)
    return UI_ERROR_UNKNOWN;
  if (g_callback_user_data != (void *)0x123)
    return UI_ERROR_UNKNOWN;

  /* Change node tag to div, set role to button */
  rc = ui_dom_node_set_tag_name(node, "div");
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_dom_node_set_attribute(node, "role", "button");
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Should match role button + space */
  g_callback_called = 0;
  handled = 0;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (handled != 1)
    return UI_ERROR_UNKNOWN;
  if (g_callback_called != 1)
    return UI_ERROR_UNKNOWN;

  /* Change node role to something else, check no match */
  rc = ui_dom_node_set_attribute(node, "role", "checkbox");
  handled = 0;
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (handled != 0)
    return UI_ERROR_UNKNOWN;

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  node = NULL;

  /* Destroy tests */
  rc = ui_keyboard_responder_destroy(responder);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_keyboard_responder_destroy(NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_keyboard_responder *responder = NULL;
  ui_error_t rc;

  /* Create OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_keyboard_responder_create(&responder);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  g_malloc_fail_countdown = -1;

  rc = ui_keyboard_responder_create(&responder);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Bind key OOM 1: realloc fails */
  g_malloc_fail_countdown = 0;
  rc = ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE,
                                      mock_callback, NULL);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  g_malloc_fail_countdown = -1;

  /* Bind key OOM 2: strdup fails */
  g_malloc_fail_countdown = 1; /* 1 means first alloc (realloc) succeeds, second
                                  alloc (malloc string) fails */
  rc = ui_keyboard_responder_bind_key(responder, "button", UI_KEY_SPACE,
                                      mock_callback, NULL);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_keyboard_responder_destroy(responder);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}

static ui_error_t run_error_paths(void) {
  struct ui_keyboard_responder *responder = NULL;
  struct ui_dom_node *node = NULL;
  struct ui_event event;
  int handled = 0;
  ui_error_t rc;

  rc = ui_keyboard_responder_create(&responder);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &node);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* For text node, handle event skips quickly */
  rc = ui_keyboard_responder_handle_event(responder, node, &event, &handled);
  if (rc != UI_ERROR_NONE)
    return rc;

  {
    ui_error_t rc_cleanup = ui_dom_node_destroy(node);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_keyboard_responder_destroy(responder);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return UI_ERROR_NONE;
}

int main(void) {
  if (run_normal_tests() != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return 1;
  }
  if (run_error_paths() != UI_ERROR_NONE) {
    printf("Error path tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
