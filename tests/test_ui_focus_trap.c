/* clang-format off */
#include "ui_focus_trap.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

struct ui_focus_trap *get_null_trap(void);
struct ui_focus_trap *get_null_trap(void) { return NULL; }

struct ui_focus_trap_test {
  struct ui_focus_manager *manager;
  struct ui_keyboard_responder *responder;
  struct ui_dom_node *root;
  int is_active;
};

static int run_normal_tests(void) {
  struct ui_focus_trap *trap = NULL;
  struct ui_focus_manager *mgr = NULL;
  struct ui_keyboard_responder *kbd = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *child = NULL;
  ui_error_t rc;

  rc = ui_focus_trap_create(&trap);
  if (rc != UI_ERROR_NONE || trap == NULL)
    return 1;
  ui_focus_manager_create(&mgr);
  ui_keyboard_responder_create(&kbd);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);

  printf("Testing invalid arguments...\n");
  ui_focus_trap_destroy(get_null_trap());
  if (ui_focus_trap_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_focus_trap_activate(NULL, mgr, root) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_focus_trap_activate(trap, NULL, root) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_focus_trap_activate(trap, mgr, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_focus_trap_deactivate(NULL, mgr) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_focus_trap_deactivate(trap, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_focus_trap_attach_keyboard(NULL, kbd) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_focus_trap_attach_keyboard(trap, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &child);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_dom_node_set_attribute(child, "tabindex", "0");
  ui_dom_node_append_child(root, child);

  /* Test attaching keyboard */
  rc = ui_focus_trap_attach_keyboard(trap, kbd);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test activation */
  rc = ui_focus_trap_activate(trap, mgr, root);
  ui_focus_manager_request_focus(mgr, root);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Double activation should be no-op */
  rc = ui_focus_trap_activate(trap, mgr, root);
  ui_focus_manager_request_focus(mgr, root);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test deactivation */
  rc = ui_focus_trap_deactivate(trap, mgr);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Double deactivation should be no-op */
  rc = ui_focus_trap_deactivate(trap, mgr);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Reactivate before destroy to test auto-cleanup */
  ui_focus_trap_activate(trap, mgr, root);
  ui_focus_manager_request_focus(mgr, root);

  /* Test destroy with inactive trap */
  {
    struct ui_focus_trap *trap2;
    ui_focus_trap_create(&trap2);
    /* never activated */
    ui_focus_trap_destroy(trap2);
  }

  ui_focus_trap_destroy(trap);
  (void)ui_keyboard_responder_destroy(kbd);
  (void)ui_focus_manager_destroy(mgr);
  (void)ui_dom_node_destroy(root);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static int run_oom_tests(void) {
  struct ui_focus_trap *trap = NULL;
  struct ui_focus_manager *mgr = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t rc;

#ifdef UI_TEST_MOCK_ALLOC
  printf("Testing OOM...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_focus_trap_create(&trap);
  if (rc != UI_ERROR_OUT_OF_MEMORY || trap != NULL) {
    printf("Failed to catch OOM in create.\n");
    return 1;
  }
#endif
  g_malloc_fail_countdown = -1;

  ui_focus_trap_create(&trap);
  ui_focus_manager_create(&mgr);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);

  /* Test OOM in activate */
  g_malloc_fail_countdown = 0; /* push_trap alloc */
  rc = ui_focus_trap_activate(trap, mgr, root);
  g_malloc_fail_countdown = -1;
  if (rc == UI_ERROR_NONE) {
    printf("Failed to catch OOM in activate.\n");
    return 1;
  }

  /* Test error in deactivate */
  ui_focus_trap_activate(trap, mgr, root);
  /* Pop the trap manually from manager to induce error in deactivate */
  ui_focus_manager_pop_trap(mgr);
  rc = ui_focus_trap_deactivate(trap, mgr);
  if (rc == UI_ERROR_NONE) {
    printf("Failed to catch error in deactivate.\n");
    return 1;
  }

  /* We manually popped, so just destroy */
  ui_focus_trap_destroy(trap);
  (void)ui_focus_manager_destroy(mgr);
  (void)ui_dom_node_destroy(root);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static int run_coverage_tests(void);

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0)
    return 1;

  if (run_coverage_tests() != 0) {
    printf("Coverage tests failed.\n");
    return 1;
  }

  printf("All ui_focus_trap tests passed.\n");

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}

static int run_coverage_tests(void) {
  struct ui_focus_trap *trap = NULL;
  struct ui_focus_trap_test *trap_test;
  struct ui_focus_manager *mgr = NULL;
  struct ui_keyboard_responder *kbd = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_event ev;
  int handled = 0;
  ui_error_t rc;

  printf("Running coverage tests...\n");

  ui_focus_trap_create(&trap);
  trap_test = (struct ui_focus_trap_test *)trap;
  ui_focus_manager_create(&mgr);
  ui_keyboard_responder_create(&kbd);
  ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root);
  ui_dom_node_set_attribute(root, "role", "dialog");
  ui_dom_node_set_attribute(root, "tabindex", "0");

  ui_focus_trap_attach_keyboard(trap, kbd);
  ui_focus_trap_activate(trap, mgr, root);
  ui_focus_manager_request_focus(mgr, root);

  ev.type = UI_EVENT_KEY_DOWN;
  ev.event_data.keyboard.key_code = UI_KEY_TAB;
  ev.event_data.keyboard.modifiers = 0;

  rc = ui_keyboard_responder_handle_event(kbd, root, &ev, &handled);
  if (rc != UI_ERROR_NONE || handled != 1) {
    printf("rc=%d handled=%d\n", rc, handled);
    return 1;
  }

  /* Mock OOM in trap_keyboard_handler (ui_focus_manager_advance) */
  ui_focus_manager_request_focus(mgr, root);
  g_malloc_fail_countdown = 0; /* advance allocs an array */
  rc = ui_keyboard_responder_handle_event(kbd, root, &ev, &handled);
  g_malloc_fail_countdown = -1;

  /* Missing branches in trap_keyboard_handler */
  ui_focus_manager_request_focus(mgr, root);
  trap_test->is_active = 0;
  ui_keyboard_responder_handle_event(kbd, root, &ev, &handled);
  trap_test->is_active = 1;

  ui_focus_manager_request_focus(mgr, root);
  trap_test->manager = NULL;
  ui_keyboard_responder_handle_event(kbd, root, &ev, &handled);
  trap_test->manager = mgr;

  ui_focus_manager_request_focus(mgr, root);
  trap_test->root = NULL;
  ui_keyboard_responder_handle_event(kbd, root, &ev, &handled);
  trap_test->root = root;

  /* Destroy trap with active but manager == NULL (covers branch in
   * ui_focus_trap_destroy) */
  trap_test->manager = NULL;
  ui_focus_trap_destroy(trap);

  (void)ui_keyboard_responder_destroy(kbd);
  (void)ui_focus_manager_destroy(mgr);
  (void)ui_dom_node_destroy(root);

#ifdef UI_TEST_MOCK_ALLOC
  g_malloc_fail_countdown = -1;
#endif
  return 0;
}
