/* clang-format off */
#include "ui_dialog_base.h"
#include "ui_internal_mem.h"
#include "ui_css_parser.h"
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

static const char *ui_dialog_base_default_css =
    "dialog[role=\"dialog\"] { "
    "background: var(--dialog-bg, #fff); "
    "border: var(--dialog-border, none); "
    "border-radius: var(--dialog-border-radius, 8px); "
    "padding: var(--dialog-padding, 24px); "
    "box-shadow: var(--dialog-shadow, 0 4px 6px rgba(0,0,0,0.1)); "
    "position: relative; "
    "z-index: 100; "
    "}";

/** \brief ui_dialog_base */
struct ui_dialog_base {
  struct ui_component *component;
  struct ui_component *content_component;
  struct ui_overlay_director *director;
  struct ui_focus_manager *focus_manager;
  struct ui_backdrop *backdrop;
  struct ui_overlay *overlay;
  int is_open;
  ui_dialog_on_close_t on_close;
  void *user_data;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

enum ui_error ui_dialog_base_create(struct ui_dialog_base **out_dialog) {
  struct ui_dialog_base *dialog;
  enum ui_error rc;
  struct ui_dom_node *root_node = NULL;
  struct ui_css_stylesheet *default_style = NULL;

  if (!out_dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog = (struct ui_dialog_base *)UI_MALLOC(sizeof(struct ui_dialog_base));
  if (!dialog) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  dialog->component = NULL;
  dialog->content_component = NULL;
  dialog->director = NULL;
  dialog->focus_manager = NULL;
  dialog->backdrop = NULL;
  dialog->overlay = NULL;
  dialog->is_open = 0;
  dialog->on_close = NULL;
  dialog->user_data = NULL;

  rc = ui_component_create(&dialog->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_backdrop_create(&dialog->backdrop);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "dialog");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "role", "dialog");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "aria-modal", "true");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_attribute(root_node, "tabindex", "-1");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_css_parse_stylesheet(ui_dialog_base_default_css, &default_style);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  ui_component_set_default_style(dialog->component, default_style);

  dialog->component->shadow_root = root_node;
  root_node = NULL; /* Owned by component now */

  *out_dialog = dialog;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  ui_backdrop_destroy(dialog->backdrop);
  ui_component_destroy(dialog->component);
  UI_FREE(dialog);
  return rc;
}

void ui_dialog_base_destroy(struct ui_dialog_base *dialog) {
  if (!dialog) {
    return;
  }

  if (dialog->is_open && dialog->director) {
    ui_overlay_director_unmount(dialog->director, dialog->overlay);
  }

  if (dialog->is_open && dialog->focus_manager) {
    ui_focus_manager_pop_trap(dialog->focus_manager);
  }

  ui_backdrop_destroy(dialog->backdrop);

  ui_component_destroy(dialog->component);

  UI_FREE(dialog);
}

enum ui_error ui_dialog_base_set_content(struct ui_dialog_base *dialog,
                                         struct ui_component *content) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dialog->content_component = content;

  if (content) {
    return ui_component_mount(content, dialog->component->shadow_root);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_dialog_base_set_overlay_director(struct ui_dialog_base *dialog,
                                    struct ui_overlay_director *director) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  dialog->director = director;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_dialog_base_set_focus_manager(struct ui_dialog_base *dialog,
                                 struct ui_focus_manager *focus_manager) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  dialog->focus_manager = focus_manager;
  return UI_ERROR_NONE;
}

enum ui_error ui_dialog_base_set_open(struct ui_dialog_base *dialog,
                                      int is_open) {
  enum ui_error rc = UI_ERROR_NONE;

  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dialog->is_open == is_open) {
    return UI_ERROR_NONE;
  }

  if (is_open) {
    if (dialog->director) {
      /* Use a high z-index for dialogs by default */
      rc = ui_overlay_director_mount_component(
          dialog->director, dialog->component, 1000, &dialog->overlay);
      if (rc != UI_ERROR_NONE) {
        return rc;
      }
    }

    if (dialog->focus_manager) {
      rc = ui_focus_manager_push_trap(dialog->focus_manager,
                                      dialog->component->shadow_root);
      if (rc != UI_ERROR_NONE) {
        /* Rollback mount */
        if (dialog->director) {
          ui_overlay_director_unmount(dialog->director, dialog->overlay);
          dialog->overlay = NULL;
        }
        return rc;
      }
    }
  } else {
    if (dialog->focus_manager) {
      ui_focus_manager_pop_trap(dialog->focus_manager);
    }
    if (dialog->director) {
      ui_overlay_director_unmount(dialog->director, dialog->overlay);
      dialog->overlay = NULL;
    }
  }

  dialog->is_open = is_open;
  return UI_ERROR_NONE;
}

enum ui_error ui_dialog_base_is_open(const struct ui_dialog_base *dialog,
                                     int *out_is_open) {
  if (!dialog || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = dialog->is_open;
  return UI_ERROR_NONE;
}

enum ui_error ui_dialog_base_set_on_close(struct ui_dialog_base *dialog,
                                          ui_dialog_on_close_t on_close,
                                          void *user_data) {
  if (!dialog) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  dialog->on_close = on_close;
  dialog->user_data = user_data;
  return UI_ERROR_NONE;
}

enum ui_error ui_dialog_base_process_event(struct ui_dialog_base *dialog,
                                           const struct ui_event *event,
                                           double timestamp_ms) {
  int should_dismiss = 0;

  (void)timestamp_ms;

  if (!dialog || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!dialog->is_open) {
    return UI_ERROR_NONE;
  }

  /* In a real engine, content dimensions would come from the layout tree.
     For headless testing, we pass dummy bounds (0,0,0,0) or rely on Escape key.
   */
  ui_backdrop_process_event(dialog->backdrop, event, 0.0f, 0.0f, 0.0f, 0.0f,
                            &should_dismiss);

  if (should_dismiss) {
    if (dialog->on_close) {
      dialog->on_close(dialog, dialog->user_data);
    } else {
      /* Default behavior if no listener is attached: close it */
      ui_dialog_base_set_open(dialog, 0);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_dialog_base_get_component(struct ui_dialog_base *dialog,
                             struct ui_component **out_component) {
  if (!dialog || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = dialog->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_dialog_base_bind_open(struct ui_dialog_base *widget,
                                       struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_dialog_base_get_animating_signal(struct ui_dialog_base *widget,
                                    struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
