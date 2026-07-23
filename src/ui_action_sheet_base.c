/* clang-format off */
#include "ui_action_sheet_base.h"
#include "ui_focus_trap.h"
#include "ui_arena.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

struct ui_action_sheet_base {
  struct ui_bottom_sheet_base *bottom_sheet;
  struct ui_component *container;
  struct ui_component *actions_container;
  struct ui_component *cancel_container;

  struct ui_focus_trap *focus_trap;
  struct ui_focus_manager *focus_manager;
  struct ui_keyboard_responder *keyboard_responder;

  ui_action_sheet_on_close_t on_close;
  void *on_close_user_data;
};
static enum ui_error on_bottom_sheet_close(struct ui_bottom_sheet_base *bs,
                                           void *user_data) {
  struct ui_action_sheet_base *sheet = (struct ui_action_sheet_base *)user_data;
  enum ui_error rc = UI_ERROR_NONE;
  (void)bs;
  ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
  if (sheet->on_close) {
    rc = sheet->on_close(sheet, sheet->on_close_user_data);
  }
  return rc;
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_create(struct ui_action_sheet_base **out_sheet) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_action_sheet_base *sheet = NULL;

  if (!out_sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sheet = (struct ui_action_sheet_base *)UI_MALLOC(
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
    UI_FREE(sheet);
    return rc;
  }

  rc = ui_component_create(&sheet->container);
  if (rc == UI_ERROR_NONE) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                            &sheet->container->shadow_root);
    if (rc != UI_ERROR_NONE) {
      ui_component_destroy(sheet->container);
    }
  }
  if (rc != UI_ERROR_NONE) {
    ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
    UI_FREE(sheet);
    return rc;
  }
  ui_dom_node_set_tag_name(sheet->container->shadow_root, "div");

  rc = ui_component_create(&sheet->actions_container);
  if (rc == UI_ERROR_NONE) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                            &sheet->actions_container->shadow_root);
    if (rc != UI_ERROR_NONE) {
      ui_component_destroy(sheet->actions_container);
    }
  }
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(sheet->container);
    ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
    UI_FREE(sheet);
    return rc;
  }
  ui_dom_node_set_tag_name(sheet->actions_container->shadow_root, "div");
  ui_dom_node_set_attribute(sheet->actions_container->shadow_root, "role",
                            "group");

  rc = ui_component_create(&sheet->cancel_container);
  if (rc == UI_ERROR_NONE) {
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT,
                            &sheet->cancel_container->shadow_root);
    if (rc != UI_ERROR_NONE) {
      ui_component_destroy(sheet->cancel_container);
    }
  }
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(sheet->actions_container);
    ui_component_destroy(sheet->container);
    ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
    UI_FREE(sheet);
    return rc;
  }
  ui_dom_node_set_tag_name(sheet->cancel_container->shadow_root, "div");

  ui_dom_node_append_child(sheet->container->shadow_root,
                           sheet->actions_container->shadow_root);
  ui_dom_node_append_child(sheet->container->shadow_root,
                           sheet->cancel_container->shadow_root);

  ui_bottom_sheet_base_set_content(sheet->bottom_sheet, sheet->container);
  ui_bottom_sheet_base_set_on_close(sheet->bottom_sheet, on_bottom_sheet_close,
                                    sheet);

  rc = ui_focus_trap_create(&sheet->focus_trap);
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_remove_child(sheet->container->shadow_root,
                             sheet->cancel_container->shadow_root);
    ui_dom_node_remove_child(sheet->container->shadow_root,
                             sheet->actions_container->shadow_root);
    ui_component_destroy(sheet->cancel_container);
    ui_component_destroy(sheet->actions_container);
    ui_component_destroy(sheet->container);
    ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
    UI_FREE(sheet);
    return rc;
  }

  *out_sheet = sheet;
  return UI_ERROR_NONE;
}

void ui_action_sheet_base_destroy(struct ui_action_sheet_base *sheet) {
  if (!sheet) {
    return;
  }
  ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
  ui_focus_trap_destroy(sheet->focus_trap);

  ui_dom_node_destroy(sheet->container->shadow_root);
  sheet->container->shadow_root = NULL;
  ui_component_destroy(sheet->container);

  sheet->cancel_container->shadow_root = NULL;
  ui_component_destroy(sheet->cancel_container);

  sheet->actions_container->shadow_root = NULL;
  ui_component_destroy(sheet->actions_container);

  ui_bottom_sheet_base_destroy(sheet->bottom_sheet);
  UI_FREE(sheet);
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_add_action(struct ui_action_sheet_base *sheet,
                                struct ui_component *action_comp) {
  if (!sheet || !action_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_dom_node_append_child(sheet->actions_container->shadow_root,
                                  action_comp->shadow_root);
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_set_cancel_action(struct ui_action_sheet_base *sheet,
                                       struct ui_component *cancel_comp) {
  if (!sheet || !cancel_comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* clear existing if any by replacing children basically, actually just remove
   * first child */
  while (sheet->cancel_container->shadow_root->first_child) {
    ui_dom_node_remove_child(sheet->cancel_container->shadow_root,
                             sheet->cancel_container->shadow_root->first_child);
  }
  return ui_dom_node_append_child(sheet->cancel_container->shadow_root,
                                  cancel_comp->shadow_root);
}

enum ui_error ui_action_sheet_base_set_open(struct ui_action_sheet_base *sheet,
                                            int is_open) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_open) {
    ui_focus_trap_activate(sheet->focus_trap, sheet->focus_manager,
                           sheet->container->shadow_root);
  } else {
    ui_focus_trap_deactivate(sheet->focus_trap, sheet->focus_manager);
  }

  return ui_bottom_sheet_base_set_open(sheet->bottom_sheet, is_open);
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_is_open(const struct ui_action_sheet_base *sheet,
                             int *out_is_open) {
  if (!sheet || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_is_open(sheet->bottom_sheet, out_is_open);
}

/** \brief ui_action_sheet_base_set_overlay_director */
enum ui_error ui_action_sheet_base_set_overlay_director(
    struct ui_action_sheet_base *sheet, struct ui_overlay_director *director) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_set_overlay_director(sheet->bottom_sheet,
                                                   director);
}

/** \brief ui_error */
enum ui_error
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

/** \brief ui_action_sheet_base_attach_focus_and_keyboard */
enum ui_error ui_action_sheet_base_attach_focus_and_keyboard(
    struct ui_action_sheet_base *sheet, struct ui_focus_manager *focus_manager,
    struct ui_keyboard_responder *keyboard_responder) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  sheet->focus_manager = focus_manager;
  sheet->keyboard_responder = keyboard_responder;
  ui_focus_trap_attach_keyboard(sheet->focus_trap, keyboard_responder);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_process_event(struct ui_action_sheet_base *sheet,
                                   const struct ui_event *event,
                                   double timestamp_ms) {
  if (!sheet || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* ESC to dismiss */
  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_ESCAPE) {
      int is_open = 0;
      ui_action_sheet_base_is_open(sheet, &is_open);
      if (is_open) {
        ui_action_sheet_base_set_open(sheet, 0);
        (void)on_bottom_sheet_close(sheet->bottom_sheet, sheet);
        return UI_ERROR_NONE;
      }
    }
  }

  return ui_bottom_sheet_base_process_event(sheet->bottom_sheet, event,
                                            timestamp_ms);
}

enum ui_error ui_action_sheet_base_update(struct ui_action_sheet_base *sheet,
                                          double timestamp_ms) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_update(sheet->bottom_sheet, timestamp_ms);
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_get_component(struct ui_action_sheet_base *sheet,
                                   struct ui_component **out_component) {
  if (!sheet || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_get_component(sheet->bottom_sheet, out_component);
}

enum ui_error ui_action_sheet_base_bind_open(struct ui_action_sheet_base *sheet,
                                             struct ui_signal *open_signal) {
  if (!sheet) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_bind_open(sheet->bottom_sheet, open_signal);
}

/** \brief ui_error */
enum ui_error
ui_action_sheet_base_get_animating_signal(struct ui_action_sheet_base *sheet,
                                          struct ui_computed **out_animating) {
  if (!sheet || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_bottom_sheet_base_get_animating_signal(sheet->bottom_sheet,
                                                   out_animating);
}
