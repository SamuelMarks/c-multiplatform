/* clang-format off */
#include "ui_snackbar_base.h"
#include "ui_internal_mem.h"
#include "ui_ring_buffer.h"
#include "ui_overlay_director.h"
#include "ui_css_parser.h"
#include <string.h>
#include <stdio.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

#if defined(_MSC_VER)
#define SAFE_STRDUP(dest, src)                                                 \
  do {                                                                         \
    size_t len = strlen(src);                                                  \
    (dest) = (char *)UI_MALLOC(len + 1);                                       \
    if (dest) {                                                                \
      strcpy_s((dest), len + 1, (src));                                        \
    }                                                                          \
  } while (0)
#else
#define SAFE_STRDUP(dest, src)                                                 \
  do {                                                                         \
    size_t len = strlen(src);                                                  \
    (dest) = (char *)UI_MALLOC(len + 1);                                       \
    if (dest) {                                                                \
      strcpy((dest), (src));                                                   \
    }                                                                          \
  } while (0)
#endif

#define MAX_SNACKBARS_IN_QUEUE 10

static const char *ui_snackbar_base_css =
    ":host { "
    "position: fixed; "
    "bottom: 0; left: 0; right: 0; "
    "display: flex; "
    "justify-content: center; "
    "pointer-events: none; "
    "z-index: 1000; "
    "} "
    ".snackbar-wrapper { "
    "background: var(--snackbar-bg, #323232); "
    "color: var(--snackbar-color, #ffffff); "
    "border-radius: var(--snackbar-radius, 4px); "
    "box-shadow: 0 3px 5px -1px rgba(0,0,0,.2), 0 6px 10px 0 rgba(0,0,0,.14), "
    "0 1px 18px 0 rgba(0,0,0,.12); "
    "padding: 14px 16px; "
    "min-width: 344px; "
    "max-width: 672px; "
    "margin-bottom: 24px; "
    "display: flex; "
    "align-items: center; "
    "pointer-events: auto; "
    "} "
    ".message { "
    "flex: 1 1 auto; "
    "font-family: var(--snackbar-font, Roboto, sans-serif); "
    "font-size: 14px; "
    "font-weight: 400; "
    "} "
    ".action { "
    "margin-left: auto; "
    "padding-left: 24px; "
    "color: var(--snackbar-action-color, #ff4081); "
    "font-weight: 500; "
    "text-transform: uppercase; "
    "cursor: pointer; "
    "}";

/* Internal representation of a queued snackbar */
struct internal_snackbar {
  char *message;
  char *action_label;
  ui_snackbar_action_cb action_callback;
  void *action_user_data;
  double duration_secs;
};

/** \brief ui_snackbar_base */
struct ui_snackbar_base {
  struct ui_timer *timer;
  struct ui_overlay_director *director;
  struct ui_ring_buffer *queue;

  /* Component state */
  struct ui_component *component;
  struct ui_dom_node *root_node;
  struct ui_dom_node *wrapper_node;
  struct ui_dom_node *message_node;
  struct ui_dom_node *message_text_node;
  struct ui_dom_node *action_node;
  struct ui_dom_node *action_text_node;
  struct ui_overlay *overlay_handle;

  /* Active snackbar state */
  int is_active;
  struct internal_snackbar current;
  double show_time;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

enum ui_error ui_snackbar_base_create(struct ui_timer *timer,
                                      struct ui_overlay_director *director,
                                      struct ui_snackbar_base **out_snackbar) {
  struct ui_snackbar_base *sb;
  enum ui_error rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!timer || !director || !out_snackbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sb = (struct ui_snackbar_base *)UI_MALLOC(sizeof(struct ui_snackbar_base));
  if (!sb)
    return UI_ERROR_OUT_OF_MEMORY;
  memset(sb, 0, sizeof(struct ui_snackbar_base));

  sb->timer = timer;
  sb->director = director;

  rc = ui_ring_buffer_create(sizeof(struct internal_snackbar),
                             MAX_SNACKBARS_IN_QUEUE, &sb->queue);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_create(&sb->component);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sb->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->wrapper_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sb->wrapper_node, "div");
  ui_dom_node_set_attribute(sb->wrapper_node, "class", "snackbar-wrapper");
  ui_dom_node_append_child(sb->root_node, sb->wrapper_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->message_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sb->message_node, "div");
  ui_dom_node_set_attribute(sb->message_node, "class", "message");
  ui_dom_node_set_attribute(sb->message_node, "role", "alert");
  ui_dom_node_append_child(sb->wrapper_node, sb->message_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &sb->message_text_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_text_content(sb->message_text_node, "");
  ui_dom_node_append_child(sb->message_node, sb->message_text_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->action_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_tag_name(sb->action_node, "button");
  ui_dom_node_set_attribute(sb->action_node, "class", "action");
  ui_dom_node_append_child(sb->wrapper_node, sb->action_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &sb->action_text_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  ui_dom_node_set_text_content(sb->action_text_node, "");
  ui_dom_node_append_child(sb->action_node, sb->action_text_node);

  rc = ui_css_parse_stylesheet(ui_snackbar_base_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_component_set_default_style(sb->component, default_style);
  if (rc != UI_ERROR_NONE) {
    ui_css_stylesheet_destroy(default_style);
    goto cleanup;
  }

  sb->component->shadow_root = sb->root_node;

  *out_snackbar = sb;
  return UI_ERROR_NONE;

cleanup:
  if (sb->root_node)
    ui_dom_node_destroy(sb->root_node);
  else {
    if (sb->wrapper_node)
      ui_dom_node_destroy(sb->wrapper_node);
    if (sb->message_node)
      ui_dom_node_destroy(sb->message_node);
    if (sb->message_text_node)
      ui_dom_node_destroy(sb->message_text_node);
    if (sb->action_node)
      ui_dom_node_destroy(sb->action_node);
    if (sb->action_text_node)
      ui_dom_node_destroy(sb->action_text_node);
  }
  if (sb->component)
    ui_component_destroy(sb->component);
  if (sb->queue)
    ui_ring_buffer_destroy(sb->queue);
  UI_FREE(sb);
  return rc;
}

void ui_snackbar_base_destroy(struct ui_snackbar_base *snackbar) {
  struct internal_snackbar pop_item;
  if (!snackbar)
    return;

  if (snackbar->overlay_handle) {
    ui_overlay_director_unmount(snackbar->director, snackbar->overlay_handle);
  }

  if (snackbar->is_active) {
    if (snackbar->current.message)
      UI_FREE(snackbar->current.message);
    if (snackbar->current.action_label)
      UI_FREE(snackbar->current.action_label);
  }

  while (ui_ring_buffer_pop(snackbar->queue, &pop_item) == UI_ERROR_NONE) {
    if (pop_item.message)
      UI_FREE(pop_item.message);
    if (pop_item.action_label)
      UI_FREE(pop_item.action_label);
  }

  if (snackbar->queue)
    ui_ring_buffer_destroy(snackbar->queue);
  if (snackbar->component)
    ui_component_destroy(snackbar->component);
  UI_FREE(snackbar);
}

/** \brief ui_error */
enum ui_error
ui_snackbar_base_enqueue(struct ui_snackbar_base *snackbar,
                         const struct ui_snackbar_config *config) {
  struct internal_snackbar item;
  enum ui_error rc;

  if (!snackbar || !config || !config->message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&item, 0, sizeof(struct internal_snackbar));

  SAFE_STRDUP(item.message, config->message);
  if (!item.message)
    return UI_ERROR_OUT_OF_MEMORY;

  if (config->action_label) {
    SAFE_STRDUP(item.action_label, config->action_label);
    if (!item.action_label) {
      UI_FREE(item.message);
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  item.action_callback = config->action_callback;
  item.action_user_data = config->action_user_data;
  item.duration_secs = config->duration_secs;

  rc = ui_ring_buffer_push(snackbar->queue, &item);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(item.message);
    if (item.action_label)
      UI_FREE(item.action_label);
    return rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_snackbar_base_dismiss_current(struct ui_snackbar_base *snackbar) {
  if (!snackbar)
    return UI_ERROR_INVALID_ARGUMENT;

  if (snackbar->is_active) {
    if (snackbar->overlay_handle) {
      ui_overlay_director_unmount(snackbar->director, snackbar->overlay_handle);
      snackbar->overlay_handle = NULL;
    }

    if (snackbar->current.message)
      UI_FREE(snackbar->current.message);
    if (snackbar->current.action_label)
      UI_FREE(snackbar->current.action_label);

    memset(&snackbar->current, 0, sizeof(struct internal_snackbar));
    snackbar->is_active = 0;
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_snackbar_base_tick(struct ui_snackbar_base *snackbar) {
  double now;
  enum ui_error rc;

  if (!snackbar)
    return UI_ERROR_INVALID_ARGUMENT;

  rc = ui_timer_now(snackbar->timer, &now);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Check auto-dismissal */
  if (snackbar->is_active && snackbar->current.duration_secs > 0.0) {
    if (now - snackbar->show_time >= snackbar->current.duration_secs) {
      ui_snackbar_base_dismiss_current(snackbar);
    }
  }

  /* Show next in queue if not active */
  if (!snackbar->is_active) {
    rc = ui_ring_buffer_pop(snackbar->queue, &snackbar->current);
    if (rc == UI_ERROR_NONE) {
      snackbar->is_active = 1;
      snackbar->show_time = now;

      /* Update DOM */
      ui_dom_node_set_text_content(
          snackbar->message_text_node,
          snackbar->current.message ? snackbar->current.message : "");
      ui_dom_node_set_text_content(
          snackbar->action_text_node,
          snackbar->current.action_label ? snackbar->current.action_label : "");

      if (snackbar->current.action_label &&
          strlen(snackbar->current.action_label) > 0) {
        ui_dom_node_set_attribute(snackbar->action_node, "style",
                                  "display: block;");
      } else {
        ui_dom_node_set_attribute(snackbar->action_node, "style",
                                  "display: none;");
      }

      /* Mount to overlay director */
      if (!snackbar->overlay_handle) {
        ui_overlay_director_mount_component(snackbar->director,
                                            snackbar->component, 1000,
                                            &snackbar->overlay_handle);
      }
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_snackbar_base_process_event(struct ui_snackbar_base *snackbar,
                                             const struct ui_event *event,
                                             double timestamp_ms) {
  (void)timestamp_ms;
  if (!snackbar || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!snackbar->is_active)
    return UI_ERROR_NONE;

  /* Very simple hit testing against action_node is assumed or
   * simulated for a "button click" equivalent.
   * For the C UI engine architecture, actual geometry hit testing would
   * typically be dispatched down the DOM tree, but we'll assume a direct
   * click event fires this.
   */
  if (event->type == UI_EVENT_MOUSE_DOWN ||
      event->type == UI_EVENT_TOUCH_START) {
    /* If action callback exists and label exists, and we pretend it hit the
       action node. Normally you would do bounds checking here against
       action_node bounds. Since we lack the full layout context here, we just
       check if it was any click to dismiss or invoke. If it hits, we invoke
       action. For now, trigger action blindly on click if it exists. */
    if (snackbar->current.action_callback) {
      snackbar->current.action_callback(snackbar,
                                        snackbar->current.action_user_data);
    }

    /* Dismiss on click */
    ui_snackbar_base_dismiss_current(snackbar);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_snackbar_base_bind_open(struct ui_snackbar_base *widget,
                                         struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_snackbar_base_get_animating_signal(struct ui_snackbar_base *widget,
                                      struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
