/* clang-format off */
#include "ui_action_sheet_base.h"
#include "ui_focus_manager.h"
#include "ui_keyboard_responder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#undef NDEBUG
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int close_count = 0;

static ui_error_t on_close(struct ui_action_sheet_base *sheet,
                           void *user_data) {
  if (sheet) {
  }
  if (user_data) {
  }
  close_count++;
  return UI_ERROR_NONE;
}

static void test_action_sheet_edge_cases(void) {
  struct ui_action_sheet_base *sheet = NULL;
  struct ui_component *cancel = NULL;
  struct ui_component *out_component = NULL;
  struct ui_computed *out_computed = NULL;
  int out_open = 0;
  int i;
  ui_error_t rc;

  /* 1. NULL pointer args */
  assert(ui_action_sheet_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_action_sheet_base_destroy(NULL); /* Should not crash */

  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);

  assert(ui_action_sheet_base_add_action(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_add_action(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_set_cancel_action(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_set_cancel_action(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_set_open(NULL, 1) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_is_open(NULL, &out_open) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_is_open(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_set_on_close(NULL, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_set_overlay_director(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_set_overlay_director(sheet, NULL) ==
         UI_ERROR_NONE); /* Assuming it works with NULL */

  assert(ui_action_sheet_base_attach_focus_and_keyboard(NULL, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_attach_focus_and_keyboard(sheet, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_process_event(NULL, NULL, 0.0) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_process_event(sheet, NULL, 0.0) ==
         UI_ERROR_INVALID_ARGUMENT);

  assert(ui_action_sheet_base_update(NULL, 0.0) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_update(sheet, 10.0) == UI_ERROR_NONE);

  assert(ui_action_sheet_base_get_component(NULL, &out_component) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_get_component(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_get_component(sheet, &out_component) ==
         UI_ERROR_NONE);

  assert(ui_action_sheet_base_bind_open(NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_bind_open(sheet, NULL) ==
         UI_ERROR_NONE); /* Assuming can be NULL */

  assert(ui_action_sheet_base_get_animating_signal(NULL, &out_computed) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_get_animating_signal(sheet, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_action_sheet_base_get_animating_signal(sheet, &out_computed) ==
         UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_action_sheet_base_destroy(sheet);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  /* 2. OOM in create */
  for (i = 0; i < 1000; ++i) {
    g_malloc_fail_countdown = i;
    rc = ui_action_sheet_base_create(&sheet);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* Expected */
    } else if (rc == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_action_sheet_base_destroy(sheet);
        assert(rc_cleanup == UI_ERROR_NONE);
      }
      break;
    } else {
      assert(0);
    }
  }
  g_malloc_fail_countdown = -1;

  /* 3. cancel action replacing existing */
  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);

  ui_component_create(&cancel);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cancel->shadow_root);
  ui_action_sheet_base_set_cancel_action(sheet, cancel);

  /* Replace cancel action */
  struct ui_component *cancel2;
  ui_component_create(&cancel2);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cancel2->shadow_root);
  ui_action_sheet_base_set_cancel_action(sheet, cancel2);

  {
    ui_error_t rc_cleanup = ui_action_sheet_base_destroy(sheet);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  /* cancel was removed from the tree, so its shadow_root is still valid and
   * needs to be freed */
  {
    ui_error_t rc_cleanup = ui_component_destroy(cancel);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  cancel2->shadow_root = NULL;
  {
    ui_error_t rc_cleanup = ui_component_destroy(cancel2);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
}

#ifdef UI_TEST_MOCK_ALLOC
extern int g_action_sheet_mock_fail;

static ui_error_t
test_action_sheet_failing_on_close(struct ui_action_sheet_base *sheet,
                                   void *user_data) {
  if (sheet || user_data) {
  }
  return UI_ERROR_UNKNOWN;
}

static int test_action_sheet_mock_failures(void) {
  struct ui_action_sheet_base *sheet = NULL;
  struct ui_component *comp = NULL;
  struct ui_focus_manager *focus = NULL;
  struct ui_keyboard_responder *keyboard = NULL;
  struct ui_event ev;
  ui_error_t rc;

  rc = ui_focus_manager_create(&focus);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_keyboard_responder_create(&keyboard);
  if (rc != UI_ERROR_NONE) {
    ui_focus_manager_destroy(focus);
    return 1;
  }

  /* 1. ui_action_sheet_base_create failure branches */
  /* append_child fails on first append */
  g_action_sheet_mock_fail = 1;
  rc = ui_action_sheet_base_create(&sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* append_child fails on second append */
  g_action_sheet_mock_fail = 10;
  rc = ui_action_sheet_base_create(&sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* set_content fails */
  g_action_sheet_mock_fail = 2;
  rc = ui_action_sheet_base_create(&sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* set_on_close fails */
  g_action_sheet_mock_fail = 3;
  rc = ui_action_sheet_base_create(&sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* focus_trap_create fails */
  g_action_sheet_mock_fail = 4;
  rc = ui_action_sheet_base_create(&sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* Successful create */
  rc = ui_action_sheet_base_create(&sheet);
  if (rc != UI_ERROR_NONE) {
    ui_keyboard_responder_destroy(keyboard);
    ui_focus_manager_destroy(focus);
    return 1;
  }

  rc = ui_action_sheet_base_attach_focus_and_keyboard(sheet, focus, keyboard);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* 2. set_open failures */
  /* activate fails */
  g_action_sheet_mock_fail = 6;
  rc = ui_action_sheet_base_set_open(sheet, 1);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* set_open(1) succeeds */
  rc = ui_action_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* deactivate fails in set_open(0) */
  g_action_sheet_mock_fail = 5;
  rc = ui_action_sheet_base_set_open(sheet, 0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* 3. remove_child failure in set_cancel_action */
  rc = ui_component_create(&comp);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &comp->shadow_root);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_action_sheet_base_set_cancel_action(sheet, comp);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* Setting cancel action again removes the old child */
  g_action_sheet_mock_fail = 7;
  rc = ui_action_sheet_base_set_cancel_action(sheet, comp);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* 4. process_event failures */
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;

  /* is_open fails */
  g_action_sheet_mock_fail = 8;
  rc = ui_action_sheet_base_process_event(sheet, &ev, 0.0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* set_open fails during ESC */
  g_action_sheet_mock_fail = 9;
  rc = ui_action_sheet_base_process_event(sheet, &ev, 0.0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* Re-open sheet */
  rc = ui_action_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  /* on_bottom_sheet_close fails (deactivate fails) */
  g_action_sheet_mock_fail = 11;
  rc = ui_action_sheet_base_process_event(sheet, &ev, 0.0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* on_close callback fails */
  rc = ui_action_sheet_base_set_open(sheet, 1);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_action_sheet_base_set_on_close(
      sheet, test_action_sheet_failing_on_close, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_action_sheet_base_process_event(sheet, &ev, 0.0);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  rc = ui_action_sheet_base_set_on_close(sheet, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* Test close callback directly with nulls and valid */
  {
    extern ui_bottom_sheet_on_close_t g_action_sheet_close_cb;
    if (g_action_sheet_close_cb) {
      rc = g_action_sheet_close_cb(NULL, sheet);
      if (rc != UI_ERROR_INVALID_ARGUMENT) {
        return 1;
      }
      rc = g_action_sheet_close_cb((struct ui_bottom_sheet_base *)1, NULL);
      if (rc != UI_ERROR_INVALID_ARGUMENT) {
        return 1;
      }
      rc = g_action_sheet_close_cb((struct ui_bottom_sheet_base *)1, sheet);
      if (rc != UI_ERROR_NONE) {
        return 1;
      }
      {
        struct ui_action_sheet_base *temp_sheet = NULL;
        rc = ui_action_sheet_base_create(&temp_sheet);
        if (rc != UI_ERROR_NONE) {
          return 1;
        }
        rc = g_action_sheet_close_cb((struct ui_bottom_sheet_base *)1,
                                     temp_sheet);
        if (rc != UI_ERROR_NONE) {
          return 1;
        }
        rc = ui_action_sheet_base_destroy(temp_sheet);
        if (rc != UI_ERROR_NONE) {
          return 1;
        }
      }
    }
  }

  /* 5. destroy with deactivate failure */
  g_action_sheet_mock_fail = 5;
  rc = ui_action_sheet_base_destroy(sheet);
  if (rc == UI_ERROR_NONE) {
    return 1;
  }
  g_action_sheet_mock_fail = 0;

  /* destroy for real */
  rc = ui_action_sheet_base_destroy(sheet);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  /* Clean up standalone component */
  comp->shadow_root = NULL;
  ui_component_destroy(comp);

  ui_keyboard_responder_destroy(keyboard);
  ui_focus_manager_destroy(focus);
  return 0;
}
#endif

int main(void) {
  struct ui_action_sheet_base *sheet = NULL;
  struct ui_component *action1 = NULL;
  struct ui_component *cancel = NULL;
  ui_error_t rc;

  struct ui_keyboard_responder *keyboard = NULL;
  struct ui_focus_manager *focus = NULL;

  rc = ui_focus_manager_create(&focus);
  assert(rc == UI_ERROR_NONE);

  rc = ui_keyboard_responder_create(&keyboard);
  assert(rc == UI_ERROR_NONE);

  test_action_sheet_edge_cases();

  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 1) == UI_ERROR_NONE);
  assert(ui_action_sheet_base_set_open(sheet, 0) == UI_ERROR_NONE);
  assert(sheet != NULL);

  rc = ui_action_sheet_base_attach_focus_and_keyboard(sheet, focus, keyboard);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_create(&action1);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &action1->shadow_root);

  rc = ui_component_create(&cancel);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cancel->shadow_root);

  rc = ui_action_sheet_base_add_action(sheet, action1);
  assert(rc == UI_ERROR_NONE);

  rc = ui_action_sheet_base_set_cancel_action(sheet, cancel);
  assert(rc == UI_ERROR_NONE);

  rc = ui_action_sheet_base_set_on_close(sheet, on_close, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_action_sheet_base_set_open(sheet, 1);
  assert(rc == UI_ERROR_NONE);
  {
    int is_open = 0;
    ui_action_sheet_base_is_open(sheet, &is_open);
    assert(is_open == 1);
  }

  /* Send ENTER event (unhandled by action sheet) */
  struct ui_event ev_enter;
  memset(&ev_enter, 0, sizeof(ev_enter));
  ev_enter.type = UI_EVENT_KEY_DOWN;
  ev_enter.event_data.keyboard.key_code = UI_KEY_ENTER;
  rc = ui_action_sheet_base_process_event(sheet, &ev_enter, 100.0);
  assert(rc == UI_ERROR_NONE);

  /* Send random event to test non-keydown */
  ev_enter.type = UI_EVENT_MOUSE_UP;
  rc = ui_action_sheet_base_process_event(sheet, &ev_enter, 100.0);
  assert(rc == UI_ERROR_NONE);

  /* Send ESC event */
  struct ui_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;

  rc = ui_action_sheet_base_process_event(sheet, &ev, 100.0);
  assert(rc == UI_ERROR_NONE);

  {
    int is_open = 0;
    ui_action_sheet_base_is_open(sheet, &is_open);
    assert(is_open == 0);
  }
  assert(close_count == 1);

  /* Send ESC event while closed */
  rc = ui_action_sheet_base_process_event(sheet, &ev, 100.0);
  assert(rc == UI_ERROR_NONE);

  /* Set on_close to NULL and close it */
  ui_action_sheet_base_set_open(sheet, 1);
  ui_action_sheet_base_set_on_close(sheet, NULL, NULL);
  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_ESCAPE;
  rc = ui_action_sheet_base_process_event(sheet, &ev, 100.0);
  assert(rc == UI_ERROR_NONE);

  {
    ui_error_t rc_cleanup = ui_action_sheet_base_destroy(sheet);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  action1->shadow_root = NULL; /* was destroyed by action sheet */
  cancel->shadow_root = NULL;  /* was destroyed by action sheet */
  {
    ui_error_t rc_cleanup = ui_component_destroy(action1);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(cancel);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  {
    ui_error_t rc_cleanup = ui_focus_manager_destroy(focus);
    assert(rc_cleanup == UI_ERROR_NONE);
  }
  {
    ui_error_t rc_cleanup = ui_keyboard_responder_destroy(keyboard);
    assert(rc_cleanup == UI_ERROR_NONE);
  }

  printf("test_ui_action_sheet_base passed\n");
#ifdef UI_TEST_MOCK_ALLOC
  if (test_action_sheet_mock_failures() != 0) {
    printf("test_action_sheet_mock_failures failed\n");
    return 1;
  }
#endif
  return 0;
}
