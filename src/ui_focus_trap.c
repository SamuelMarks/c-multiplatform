/* clang-format off */
#include "ui_focus_trap.h"
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_focus_trap {
  struct ui_dom_node *root;
  struct ui_focus_manager *manager;
  struct ui_keyboard_responder *responder;
  int is_active;
};

static ui_error_t trap_keyboard_handler(struct ui_dom_node *node,
                                        void *user_data) {
  struct ui_focus_trap *trap = (struct ui_focus_trap *)user_data;
  (void)node;
  if (trap->is_active) {
    /* Default advance forward. If shift was held, should be backward.
       For scaffolding, we'll just test the integration plumbing calling
       advance. */
    ui_error_t rc = ui_focus_manager_advance(trap->manager, trap->root, 1);
    if (rc != UI_ERROR_NONE)
      return rc;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_focus_trap_create(struct ui_focus_trap **out_trap) {
  struct ui_focus_trap *trap;

  if (out_trap == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  trap = (struct ui_focus_trap *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_focus_trap));
  if (trap == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  trap->root = NULL;
  trap->manager = NULL;
  trap->responder = NULL;
  trap->is_active = 0;

  *out_trap = trap;
  return UI_ERROR_NONE;
}

ui_error_t ui_focus_trap_destroy(struct ui_focus_trap *trap) {
  ui_error_t rc = UI_ERROR_NONE;
  if (trap != NULL) {
    if (trap->is_active) {
      rc = ui_focus_trap_deactivate(trap, trap->manager);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
    C_MULTIPLATFORM_FREE(trap);
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_focus_trap_activate(struct ui_focus_trap *trap,
                                  struct ui_focus_manager *manager,
                                  struct ui_dom_node *root) {
  ui_error_t rc;

  if (trap == NULL || manager == NULL || root == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (trap->is_active) {
    return UI_ERROR_NONE; /* Already active */
  }

  rc = ui_focus_manager_push_trap(manager, root);
  if (rc != UI_ERROR_NONE)
    return rc;

  trap->manager = manager;
  trap->root = root;
  trap->is_active = 1;

  return UI_ERROR_NONE;
}

ui_error_t ui_focus_trap_deactivate(struct ui_focus_trap *trap,
                                    struct ui_focus_manager *manager) {
  ui_error_t rc;

  if (trap == NULL || manager == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!trap->is_active) {
    return UI_ERROR_NONE;
  }

  rc = ui_focus_manager_pop_trap(manager);
  if (rc != UI_ERROR_NONE)
    return rc;

  trap->manager = NULL;
  trap->root = NULL;
  trap->is_active = 0;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_focus_trap_attach_keyboard(struct ui_focus_trap *trap,
                              struct ui_keyboard_responder *responder) {
  if (trap == NULL || responder == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  trap->responder = responder;

  /* Bind to a pseudo-role for the trap. In practice, the trap's root node
   * should have this role assigned. */
  return ui_keyboard_responder_bind_key(responder, "dialog", UI_KEY_TAB,
                                        trap_keyboard_handler, trap);
}
