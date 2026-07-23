/* clang-format off */
#include "ui_action_sheet_base.h"
#include "ui_focus_manager.h"
#include "ui_keyboard_responder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#undef NDEBUG
#include <assert.h>

extern int g_malloc_fail_countdown;

static int close_count = 0;

static enum ui_error on_close(struct ui_action_sheet_base *sheet,
                              void *user_data) {
  (void)sheet;
  (void)user_data;
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
  enum ui_error rc;

  /* 1. NULL pointer args */
  assert(ui_action_sheet_base_create(NULL) == UI_ERROR_INVALID_ARGUMENT);
  ui_action_sheet_base_destroy(NULL); /* Should not crash */

  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);

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
         UI_ERROR_NONE); /* works with NULLs */

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

  ui_action_sheet_base_destroy(sheet);

  /* 2. OOM in create */
  for (i = 0; i < 1000; ++i) {
    g_malloc_fail_countdown = i;
    rc = ui_action_sheet_base_create(&sheet);
    if (rc == UI_ERROR_OUT_OF_MEMORY) {
      /* Expected */
    } else if (rc == UI_ERROR_NONE) {
      ui_action_sheet_base_destroy(sheet);
      break;
    } else {
      assert(0);
    }
  }
  g_malloc_fail_countdown = -1;

  /* 3. cancel action replacing existing */
  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);

  ui_component_create(&cancel);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cancel->shadow_root);
  ui_action_sheet_base_set_cancel_action(sheet, cancel);

  /* Replace cancel action */
  struct ui_component *cancel2;
  ui_component_create(&cancel2);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &cancel2->shadow_root);
  ui_action_sheet_base_set_cancel_action(sheet, cancel2);

  ui_action_sheet_base_destroy(sheet);
  /* cancel was removed from the tree, so its shadow_root is still valid and
   * needs to be freed */
  ui_component_destroy(cancel);
  cancel2->shadow_root = NULL;
  ui_component_destroy(cancel2);
}

int main(void) {
  struct ui_action_sheet_base *sheet = NULL;
  struct ui_component *action1 = NULL;
  struct ui_component *cancel = NULL;
  enum ui_error rc;

  struct ui_keyboard_responder *keyboard = NULL;
  struct ui_focus_manager *focus = NULL;

  rc = ui_focus_manager_create(&focus);
  assert(rc == UI_ERROR_NONE);

  rc = ui_keyboard_responder_create(&keyboard);
  assert(rc == UI_ERROR_NONE);

  test_action_sheet_edge_cases();

  rc = ui_action_sheet_base_create(&sheet);
  assert(rc == UI_ERROR_NONE);
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

  ui_action_sheet_base_destroy(sheet);
  action1->shadow_root = NULL; /* was destroyed by action sheet */
  cancel->shadow_root = NULL;  /* was destroyed by action sheet */
  ui_component_destroy(action1);
  ui_component_destroy(cancel);

  ui_focus_manager_destroy(focus);
  ui_keyboard_responder_destroy(keyboard);

  printf("test_ui_action_sheet_base passed\n");
  return 0;
}
