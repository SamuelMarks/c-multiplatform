/**
 * @file ui_action_sheet_base.c
 * @brief Implementation of ui_action_sheet_base.c.
 */
/* clang-format off */
#include "ui_action_sheet_base.h"
#include "ui_focus_trap.h"
#include "ui_arena.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_action_sheet_mock_fail = 0;

static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_action_sheet_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_action_sheet_mock_fail == 10) {
    /* fail on second append_child */
    g_action_sheet_mock_fail = 1;
    return (ui_dom_node_append_child)(parent, child);
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

static ui_error_t
mock_bottom_sheet_set_content(struct ui_bottom_sheet_base *sheet,
                              struct ui_component *content) {
  if (g_action_sheet_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_set_content)(sheet, content);
}
#undef ui_bottom_sheet_base_set_content
/** @cond */
#define ui_bottom_sheet_base_set_content mock_bottom_sheet_set_content
/** @endcond */

ui_bottom_sheet_on_close_t g_action_sheet_close_cb = NULL;

static ui_error_t
mock_bottom_sheet_set_on_close(struct ui_bottom_sheet_base *sheet,
                               ui_bottom_sheet_on_close_t cb, void *u) {
  g_action_sheet_close_cb = cb;
  if (g_action_sheet_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_set_on_close)(sheet, cb, u);
}
#undef ui_bottom_sheet_base_set_on_close
/** @cond */
#define ui_bottom_sheet_base_set_on_close mock_bottom_sheet_set_on_close
/** @endcond */

static ui_error_t mock_focus_trap_create(struct ui_focus_trap **ft) {
  if (g_action_sheet_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_focus_trap_create)(ft);
}
#undef ui_focus_trap_create
/** @cond */
#define ui_focus_trap_create mock_focus_trap_create
/** @endcond */

static ui_error_t mock_focus_trap_deactivate(struct ui_focus_trap *ft,
                                             struct ui_focus_manager *fm) {
  if (g_action_sheet_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  if (g_action_sheet_mock_fail == 11) {
    /* succeed first (for set_open(0)), then fail on second call (for
     * on_bottom_sheet_close) */
    g_action_sheet_mock_fail = 5;
    return (ui_focus_trap_deactivate)(ft, fm);
  }
  return (ui_focus_trap_deactivate)(ft, fm);
}
#undef ui_focus_trap_deactivate
/** @cond */
#define ui_focus_trap_deactivate mock_focus_trap_deactivate
/** @endcond */

static ui_error_t mock_focus_trap_activate(struct ui_focus_trap *ft,
                                           struct ui_focus_manager *fm,
                                           struct ui_dom_node *root) {
  if (g_action_sheet_mock_fail == 6) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_focus_trap_activate)(ft, fm, root);
}
#undef ui_focus_trap_activate
/** @cond */
#define ui_focus_trap_activate mock_focus_trap_activate
/** @endcond */

static ui_error_t mock_dom_node_remove_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_action_sheet_mock_fail == 7) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_child)(parent, child);
}
#undef ui_dom_node_remove_child
/** @cond */
#define ui_dom_node_remove_child mock_dom_node_remove_child
/** @endcond */

static ui_error_t
mock_bottom_sheet_is_open(const struct ui_bottom_sheet_base *sheet, int *out) {
  if (g_action_sheet_mock_fail == 8) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_is_open)(sheet, out);
}
#undef ui_bottom_sheet_base_is_open
/** @cond */
#define ui_bottom_sheet_base_is_open mock_bottom_sheet_is_open
/** @endcond */

static ui_error_t mock_bottom_sheet_set_open(struct ui_bottom_sheet_base *sheet,
                                             int is_open) {
  if (g_action_sheet_mock_fail == 9) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_bottom_sheet_base_set_open)(sheet, is_open);
}
#undef ui_bottom_sheet_base_set_open
/** @cond */
#define ui_bottom_sheet_base_set_open mock_bottom_sheet_set_open
/** @endcond */

#endif

/**
 * @struct ui_action_sheet_base
 * @struct ui_action_sheet_base
 * @brief Internal implementation of the action sheet base.
 */
struct ui_action_sheet_base {
  /* @brief Underlying bottom sheet component. */
  struct ui_bottom_sheet_base *bottom_sheet; /**< bottom_sheet */
  /* @brief Container for the entire sheet. */
  struct ui_component *container; /**< container */
  /* @brief Container for the main actions. */
  struct ui_component *actions_container; /**< actions_container */
  /* @brief Container for the cancel action. */
  struct ui_component *cancel_container; /**< cancel_container */

  /* @brief Trap to keep focus within the sheet when open. */
  struct ui_focus_trap *focus_trap; /**< focus_trap */
  /* @brief Focus manager reference. */
  struct ui_focus_manager *focus_manager; /**< focus_manager */
  /* @brief Keyboard responder reference. */
  struct ui_keyboard_responder *keyboard_responder; /**< keyboard_responder */

  /* @brief Callback invoked when the sheet closes. */
  ui_action_sheet_on_close_t on_close; /**< on_close */
  /* @brief User data for the close callback. */
  void *on_close_user_data; /**< on_close_user_data */
};

/**
 * @brief on_bottom_sheet_close.
 * @param bs Parameter bs.
 * @param user_data Parameter user_data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t on_bottom_sheet_close(struct ui_bottom_sheet_base *bs,
                                        void *user_data) {
  struct ui_action_sheet_base *sheet = (struct ui_action_sheet_base *)user_data;
  ui_error_t rc = UI_ERROR_NONE;
  if (!bs || !sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sheet->focus_manager) {
    rc = ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  if (sheet->on_close) {
    rc = sheet->on_close(sheet, sheet->on_close_user_data);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return rc;
}

/**
 * @brief ui_action_sheet_base_create.
 * @param out_sheet Parameter out_sheet.
 * @return Return value.
 */

static ui_error_t
destroy_partially_created_action_sheet(struct ui_action_sheet_base *sheet,
                                       int actions_attached,
                                       int cancel_attached) {
  if (sheet->cancel_container) {
    if (cancel_attached) {
      sheet->cancel_container->shadow_root = NULL;
    }
    ui_component_destroy(sheet->cancel_container);
  }
  if (sheet->actions_container) {
    if (actions_attached) {
      sheet->actions_container->shadow_root = NULL;
    }
    ui_component_destroy(sheet->actions_container);
  }
  if (sheet->container) {
    if (sheet->container->shadow_root) {
      ui_dom_node_destroy(sheet->container->shadow_root);
      sheet->container->shadow_root = NULL;
    }
    ui_component_destroy(sheet->container);
  }
  if (sheet->bottom_sheet) {
    ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
  }
  C_MULTIPLATFORM_FREE(sheet);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_action_sheet_base_create.
 * @param out_sheet Parameter out_sheet.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_create(struct ui_action_sheet_base **out_sheet) {
  struct ui_action_sheet_base *sheet;
  ui_error_t rc;
  int actions_attached = 0;
  int cancel_attached = 0;

  if (!out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct ui_action_sheet_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_action_sheet_base));
  if (!sheet) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  sheet->bottom_sheet = NULL;
  sheet->container = NULL;
  sheet->actions_container = NULL;
  sheet->cancel_container = NULL;
  sheet->focus_trap = NULL;
  sheet->focus_manager = NULL;
  sheet->keyboard_responder = NULL;
  sheet->on_close = NULL;
  sheet->on_close_user_data = NULL;

  rc = ui_bottom_sheet_base_create(&sheet->bottom_sheet);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_create(&sheet->container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &sheet->container->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(sheet->container->shadow_root, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_create(&sheet->actions_container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &sheet->actions_container->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(sheet->actions_container->shadow_root, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(sheet->actions_container->shadow_root, "role",
                                 "group");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_create(&sheet->cancel_container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                          &sheet->cancel_container->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(sheet->cancel_container->shadow_root, "div");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_append_child(sheet->container->shadow_root,
                                sheet->actions_container->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  actions_attached = 1;

  rc = ui_dom_node_append_child(sheet->container->shadow_root,
                                sheet->cancel_container->shadow_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  cancel_attached = 1;

  rc = ui_bottom_sheet_base_set_content(sheet->bottom_sheet, sheet->container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_bottom_sheet_base_set_on_close(sheet->bottom_sheet,
                                         on_bottom_sheet_close, sheet);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_focus_trap_create(&sheet->focus_trap);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;

cleanup:
  destroy_partially_created_action_sheet(sheet, actions_attached,
                                         cancel_attached);
  return rc;
}

/**
 * @brief ui_action_sheet_base_destroy.
 * @param sheet Parameter sheet.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_destroy(struct ui_action_sheet_base *sheet) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sheet->focus_manager) {
    rc = ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  ui_focus_trap_destroy(sheet->focus_trap);
  ui_dom_node_destroy(sheet->container->shadow_root);
  sheet->container->shadow_root = NULL;
  ui_component_destroy(sheet->container);
  sheet->cancel_container->shadow_root = NULL;
  ui_component_destroy(sheet->cancel_container);
  sheet->actions_container->shadow_root = NULL;
  ui_component_destroy(sheet->actions_container);
  ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
  C_MULTIPLATFORM_FREE(sheet);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_action_sheet_base_add_action.
 * @param sheet Parameter sheet.
 * @param action_comp Parameter action_comp.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_add_action(struct ui_action_sheet_base *sheet,
                                           struct ui_component *action_comp) {
  if (!sheet || !action_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(sheet->actions_container->shadow_root,
                                  action_comp->shadow_root);
}

/**
 * @brief ui_action_sheet_base_set_cancel_action.
 * @param sheet Parameter sheet.
 * @param cancel_comp Parameter cancel_comp.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_set_cancel_action(struct ui_action_sheet_base *sheet,
                                       struct ui_component *cancel_comp) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet || !cancel_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* clear existing if any by replacing children basically, actually just remove
   * first child */
  while (sheet->cancel_container->shadow_root->first_child) {
    rc = ui_dom_node_remove_child(
        sheet->cancel_container->shadow_root,
        sheet->cancel_container->shadow_root->first_child);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return ui_dom_node_append_child(sheet->cancel_container->shadow_root,
                                  cancel_comp->shadow_root);
}

/**
 * @brief ui_action_sheet_base_set_open.
 * @param sheet Parameter sheet.
 * @param is_open Parameter is_open.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_set_open(struct ui_action_sheet_base *sheet,
                                         int is_open) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_open) {
    if (sheet->focus_manager) {
      rc = ui_focus_trap_activate(sheet->focus_trap, sheet->focus_manager,
                                  sheet->container->shadow_root);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  } else {
    if (sheet->focus_manager) {
      rc = ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }
  }

  return ui_bottom_sheet_base_set_open(sheet->bottom_sheet, is_open);
}

/**
 * @brief ui_action_sheet_base_is_open.
 * @param sheet Parameter sheet.
 * @param out_is_open Parameter out_is_open.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_is_open(const struct ui_action_sheet_base *sheet,
                             int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_is_open(sheet->bottom_sheet, out_is_open);
}

/**
 * @brief ui_action_sheet_base_set_overlay_director.
 * @param sheet Parameter sheet.
 * @param director Parameter director.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_set_overlay_director(
    struct ui_action_sheet_base *sheet, struct ui_overlay_director *director) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_set_overlay_director(sheet->bottom_sheet,
                                                   director);
}

/**
 * @brief ui_action_sheet_base_set_on_close.
 * @param sheet Parameter sheet.
 * @param on_close Parameter on_close.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_set_on_close(struct ui_action_sheet_base *sheet,
                                  ui_action_sheet_on_close_t on_close,
                                  void *user_data) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->on_close = on_close;
  sheet->on_close_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_action_sheet_base_attach_focus_and_keyboard.
 * @param sheet Parameter sheet.
 * @param focus_manager Parameter focus_manager.
 * @param keyboard_responder Parameter keyboard_responder.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_attach_focus_and_keyboard(
    struct ui_action_sheet_base *sheet, struct ui_focus_manager *focus_manager,
    struct ui_keyboard_responder *keyboard_responder) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->focus_manager = focus_manager;
  sheet->keyboard_responder = keyboard_responder;
  rc = ui_focus_trap_attach_keyboard(sheet->focus_trap, keyboard_responder);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_action_sheet_base_process_event.
 * @param sheet Parameter sheet.
 * @param event Parameter event.
 * @param timestamp_ms Parameter timestamp_ms.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_process_event(struct ui_action_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!sheet || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* ESC to dismiss */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
      int is_open = 0;
      rc = ui_action_sheet_base_is_open(sheet, &is_open);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
      if (is_open) {
        rc = ui_action_sheet_base_set_open(sheet, 0);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        rc = on_bottom_sheet_close(sheet->bottom_sheet, sheet);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
        return UI_ERROR_NONE;
      }
    }
  }

  return ui_bottom_sheet_base_process_event(sheet->bottom_sheet, event,
                                            timestamp_ms);
}

/**
 * @brief ui_action_sheet_base_update.
 * @param sheet Parameter sheet.
 * @param timestamp_ms Parameter timestamp_ms.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_update(struct ui_action_sheet_base *sheet,
                                       double timestamp_ms) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_update(sheet->bottom_sheet, timestamp_ms);
}

/**
 * @brief ui_action_sheet_base_get_component.
 * @param sheet Parameter sheet.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_get_component(struct ui_action_sheet_base *sheet,
                                   struct ui_component **out_component) {
  if (!sheet || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_get_component(sheet->bottom_sheet, out_component);
}

/**
 * @brief ui_action_sheet_base_bind_open.
 * @param sheet Parameter sheet.
 * @param open_signal Parameter open_signal.
 * @return Return value.
 */
ui_error_t ui_action_sheet_base_bind_open(struct ui_action_sheet_base *sheet,
                                          struct ui_signal *open_signal) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_bind_open(sheet->bottom_sheet, open_signal);
}

/**
 * @brief ui_action_sheet_base_get_animating_signal.
 * @param sheet Parameter sheet.
 * @param out_animating Parameter out_animating.
 * @return Return value.
 */
ui_error_t
ui_action_sheet_base_get_animating_signal(struct ui_action_sheet_base *sheet,
                                          struct ui_computed **out_animating) {
  if (!sheet || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_get_animating_signal(sheet->bottom_sheet,
                                                   out_animating);
}
