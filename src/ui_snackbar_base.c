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
    (dest) = (char *)C_MULTIPLATFORM_MALLOC(len + 1);                          \
    if (dest) {                                                                \
      strcpy_s((dest), len + 1, (src));                                        \
    }                                                                          \
  } while (0)
#else
#define SAFE_STRDUP(dest, src)                                                 \
  do {                                                                         \
    size_t len = strlen(src);                                                  \
    (dest) = (char *)C_MULTIPLATFORM_MALLOC(len + 1);                          \
    if (dest) {                                                                \
      strcpy((dest), (src));                                                   \
    }                                                                          \
  } while (0)
#endif

#define MAX_SNACKBARS_IN_QUEUE 10

static const char ui_snackbar_base_css[] = {
    58,  104, 111, 115, 116, 32,  123, 32,  112, 111, 115, 105, 116, 105, 111,
    110, 58,  32,  102, 105, 120, 101, 100, 59,  32,  98,  111, 116, 116, 111,
    109, 58,  32,  48,  59,  32,  108, 101, 102, 116, 58,  32,  48,  59,  32,
    114, 105, 103, 104, 116, 58,  32,  48,  59,  32,  100, 105, 115, 112, 108,
    97,  121, 58,  32,  102, 108, 101, 120, 59,  32,  106, 117, 115, 116, 105,
    102, 121, 45,  99,  111, 110, 116, 101, 110, 116, 58,  32,  99,  101, 110,
    116, 101, 114, 59,  32,  112, 111, 105, 110, 116, 101, 114, 45,  101, 118,
    101, 110, 116, 115, 58,  32,  110, 111, 110, 101, 59,  32,  122, 45,  105,
    110, 100, 101, 120, 58,  32,  49,  48,  48,  48,  59,  32,  125, 32,  46,
    115, 110, 97,  99,  107, 98,  97,  114, 45,  119, 114, 97,  112, 112, 101,
    114, 32,  123, 32,  98,  97,  99,  107, 103, 114, 111, 117, 110, 100, 58,
    32,  118, 97,  114, 40,  45,  45,  115, 110, 97,  99,  107, 98,  97,  114,
    45,  98,  103, 44,  32,  35,  51,  50,  51,  50,  51,  50,  41,  59,  32,
    99,  111, 108, 111, 114, 58,  32,  118, 97,  114, 40,  45,  45,  115, 110,
    97,  99,  107, 98,  97,  114, 45,  99,  111, 108, 111, 114, 44,  32,  35,
    102, 102, 102, 102, 102, 102, 41,  59,  32,  98,  111, 114, 100, 101, 114,
    45,  114, 97,  100, 105, 117, 115, 58,  32,  118, 97,  114, 40,  45,  45,
    115, 110, 97,  99,  107, 98,  97,  114, 45,  114, 97,  100, 105, 117, 115,
    44,  32,  52,  112, 120, 41,  59,  32,  98,  111, 120, 45,  115, 104, 97,
    100, 111, 119, 58,  32,  48,  32,  51,  112, 120, 32,  53,  112, 120, 32,
    45,  49,  112, 120, 32,  114, 103, 98,  97,  40,  48,  44,  48,  44,  48,
    44,  46,  50,  41,  44,  32,  48,  32,  54,  112, 120, 32,  49,  48,  112,
    120, 32,  48,  32,  114, 103, 98,  97,  40,  48,  44,  48,  44,  48,  44,
    46,  49,  52,  41,  44,  32,  48,  32,  49,  112, 120, 32,  49,  56,  112,
    120, 32,  48,  32,  114, 103, 98,  97,  40,  48,  44,  48,  44,  48,  44,
    46,  49,  50,  41,  59,  32,  112, 97,  100, 100, 105, 110, 103, 58,  32,
    49,  52,  112, 120, 32,  49,  54,  112, 120, 59,  32,  109, 105, 110, 45,
    119, 105, 100, 116, 104, 58,  32,  51,  52,  52,  112, 120, 59,  32,  109,
    97,  120, 45,  119, 105, 100, 116, 104, 58,  32,  54,  55,  50,  112, 120,
    59,  32,  109, 97,  114, 103, 105, 110, 45,  98,  111, 116, 116, 111, 109,
    58,  32,  50,  52,  112, 120, 59,  32,  100, 105, 115, 112, 108, 97,  121,
    58,  32,  102, 108, 101, 120, 59,  32,  97,  108, 105, 103, 110, 45,  105,
    116, 101, 109, 115, 58,  32,  99,  101, 110, 116, 101, 114, 59,  32,  112,
    111, 105, 110, 116, 101, 114, 45,  101, 118, 101, 110, 116, 115, 58,  32,
    97,  117, 116, 111, 59,  32,  125, 32,  46,  109, 101, 115, 115, 97,  103,
    101, 32,  123, 32,  102, 108, 101, 120, 58,  32,  49,  32,  49,  32,  97,
    117, 116, 111, 59,  32,  102, 111, 110, 116, 45,  102, 97,  109, 105, 108,
    121, 58,  32,  118, 97,  114, 40,  45,  45,  115, 110, 97,  99,  107, 98,
    97,  114, 45,  102, 111, 110, 116, 44,  32,  82,  111, 98,  111, 116, 111,
    44,  32,  115, 97,  110, 115, 45,  115, 101, 114, 105, 102, 41,  59,  32,
    102, 111, 110, 116, 45,  115, 105, 122, 101, 58,  32,  49,  52,  112, 120,
    59,  32,  102, 111, 110, 116, 45,  119, 101, 105, 103, 104, 116, 58,  32,
    52,  48,  48,  59,  32,  125, 32,  46,  97,  99,  116, 105, 111, 110, 32,
    123, 32,  109, 97,  114, 103, 105, 110, 45,  108, 101, 102, 116, 58,  32,
    97,  117, 116, 111, 59,  32,  112, 97,  100, 100, 105, 110, 103, 45,  108,
    101, 102, 116, 58,  32,  50,  52,  112, 120, 59,  32,  99,  111, 108, 111,
    114, 58,  32,  118, 97,  114, 40,  45,  45,  115, 110, 97,  99,  107, 98,
    97,  114, 45,  97,  99,  116, 105, 111, 110, 45,  99,  111, 108, 111, 114,
    44,  32,  35,  102, 102, 52,  48,  56,  49,  41,  59,  32,  102, 111, 110,
    116, 45,  119, 101, 105, 103, 104, 116, 58,  32,  53,  48,  48,  59,  32,
    116, 101, 120, 116, 45,  116, 114, 97,  110, 115, 102, 111, 114, 109, 58,
    32,  117, 112, 112, 101, 114, 99,  97,  115, 101, 59,  32,  99,  117, 114,
    115, 111, 114, 58,  32,  112, 111, 105, 110, 116, 101, 114, 59,  32,  125,
    0};

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

ui_error_t ui_snackbar_base_create(struct ui_timer *timer,
                                   struct ui_overlay_director *director,
                                   struct ui_snackbar_base **out_snackbar) {
  struct ui_snackbar_base *sb;
  ui_error_t rc;
  struct ui_css_stylesheet *default_style = NULL;

  if (!timer || !director || !out_snackbar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sb = (struct ui_snackbar_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_snackbar_base));
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

#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
#define UI_DOM_APP_CHILD_IGNORE(p, c) ui_dom_node_append_child((p), (c))
#define UI_DOM_SET_TXT_IGNORE(n, t) ui_dom_node_set_text_content((n), (t))

#define UI_DOM_SET_TAG_IGNORE(n, t) ui_dom_node_set_tag_name((n), (t))

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->root_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TAG_IGNORE(sb->root_node, "div");

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->wrapper_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TAG_IGNORE(sb->wrapper_node, "div");
  (void)UI_DOM_SET_ATTR_IGNORE(sb->wrapper_node, "class", "snackbar-wrapper");
  (void)UI_DOM_APP_CHILD_IGNORE(sb->root_node, sb->wrapper_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->message_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TAG_IGNORE(sb->message_node, "div");
  (void)UI_DOM_SET_ATTR_IGNORE(sb->message_node, "class", "message");
  (void)UI_DOM_SET_ATTR_IGNORE(sb->message_node, "role", "alert");
  (void)UI_DOM_APP_CHILD_IGNORE(sb->wrapper_node, sb->message_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &sb->message_text_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TXT_IGNORE(sb->message_text_node, "");
  (void)UI_DOM_APP_CHILD_IGNORE(sb->message_node, sb->message_text_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &sb->action_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TAG_IGNORE(sb->action_node, "button");
  (void)UI_DOM_SET_ATTR_IGNORE(sb->action_node, "class", "action");
  (void)UI_DOM_APP_CHILD_IGNORE(sb->wrapper_node, sb->action_node);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &sb->action_text_node);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  (void)UI_DOM_SET_TXT_IGNORE(sb->action_text_node, "");
  (void)UI_DOM_APP_CHILD_IGNORE(sb->action_node, sb->action_text_node);

  rc = ui_css_parse_stylesheet(ui_snackbar_base_css, &default_style);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  {

    ui_error_t _ign_rc =
        ui_component_set_default_style(sb->component, default_style);

    (void)_ign_rc;
  }

  sb->component->shadow_root = sb->root_node;

  *out_snackbar = sb;
  return UI_ERROR_NONE;

cleanup:
  if (sb->root_node) {
    (void)ui_dom_node_destroy(sb->root_node);
  }
  if (sb->component)
    (void)ui_component_destroy(sb->component);
  if (sb->queue)
    ui_ring_buffer_destroy(sb->queue);
  C_MULTIPLATFORM_FREE(sb);
  return rc;
}

ui_error_t ui_snackbar_base_destroy(struct ui_snackbar_base *snackbar) {
  struct internal_snackbar pop_item;
  if (!snackbar)
    return UI_ERROR_NONE;

  if (snackbar->overlay_handle) {
    {
      ui_error_t unmount_rc = ui_overlay_director_unmount(
          snackbar->director, snackbar->overlay_handle);
      if (unmount_rc != UI_ERROR_NONE)
        return unmount_rc;
    }
  }

  if (snackbar->is_active) {
    if (snackbar->current.message)
      C_MULTIPLATFORM_FREE(snackbar->current.message);
    if (snackbar->current.action_label)
      C_MULTIPLATFORM_FREE(snackbar->current.action_label);
  }

  while (1) {
#define UI_RING_BUF_POP_IGNORE(q, i) ui_ring_buffer_pop((q), (i))
    ui_error_t pop_rc = UI_RING_BUF_POP_IGNORE(snackbar->queue, &pop_item);
    if (pop_rc != UI_ERROR_NONE) {
      break;
    }
    if (pop_item.message)
      C_MULTIPLATFORM_FREE(pop_item.message);
    if (pop_item.action_label)
      C_MULTIPLATFORM_FREE(pop_item.action_label);
  }

  if (snackbar->queue)
    ui_ring_buffer_destroy(snackbar->queue);
  if (snackbar->component)
    (void)ui_component_destroy(snackbar->component);
  C_MULTIPLATFORM_FREE(snackbar);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_snackbar_base_enqueue(struct ui_snackbar_base *snackbar,
                                    const struct ui_snackbar_config *config) {
  struct internal_snackbar item;
  ui_error_t rc;

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
      C_MULTIPLATFORM_FREE(item.message);
      return UI_ERROR_OUT_OF_MEMORY;
    }
  }

  item.action_callback = config->action_callback;
  item.action_user_data = config->action_user_data;
  item.duration_secs = config->duration_secs;

  rc = ui_ring_buffer_push(snackbar->queue, &item);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(item.message);
    if (item.action_label)
      C_MULTIPLATFORM_FREE(item.action_label);
    return rc;
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_snackbar_base_dismiss_current(struct ui_snackbar_base *snackbar) {
  if (!snackbar)
    return UI_ERROR_INVALID_ARGUMENT;

  if (snackbar->is_active) {
    if (snackbar->overlay_handle) {
#define ui_overlay_director_unmount(d, o) ui_overlay_director_unmount((d), (o))
      (void)ui_overlay_director_unmount(snackbar->director,
                                        snackbar->overlay_handle);
      snackbar->overlay_handle = NULL;
    }

    if (snackbar->current.message)
      C_MULTIPLATFORM_FREE(snackbar->current.message);
    if (snackbar->current.action_label)
      C_MULTIPLATFORM_FREE(snackbar->current.action_label);

    memset(&snackbar->current, 0, sizeof(struct internal_snackbar));
    snackbar->is_active = 0;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_snackbar_base_tick(struct ui_snackbar_base *snackbar) {
  double now;
  ui_error_t rc;

  if (!snackbar)
    return UI_ERROR_INVALID_ARGUMENT;

  {

    ui_error_t _ign_rc = ui_timer_now(snackbar->timer, &now);

    (void)_ign_rc;
  }

  /* Check auto-dismissal */
  if (snackbar->is_active && snackbar->current.duration_secs > 0.0) {
    if (now - snackbar->show_time >= snackbar->current.duration_secs) {
      ui_error_t dis_rc = ui_snackbar_base_dismiss_current(snackbar);
      if (dis_rc != UI_ERROR_NONE)
        return dis_rc;
    }
  }

  /* Show next in queue if not active */
  if (!snackbar->is_active) {
    ui_error_t pop_rc =
        UI_RING_BUF_POP_IGNORE(snackbar->queue, &snackbar->current);
    if (pop_rc == UI_ERROR_NONE) {
      snackbar->is_active = 1;
      snackbar->show_time = now;

      /* Update DOM */
      (void)UI_DOM_SET_TXT_IGNORE(
          snackbar->message_text_node,
          snackbar->current.message ? snackbar->current.message : "");
      (void)UI_DOM_SET_TXT_IGNORE(
          snackbar->action_text_node,
          snackbar->current.action_label ? snackbar->current.action_label : "");

      if (snackbar->current.action_label &&
          strlen(snackbar->current.action_label) > 0) {
        (void)UI_DOM_SET_ATTR_IGNORE(snackbar->action_node, "style",
                                     "display: block;");
      } else {
        (void)UI_DOM_SET_ATTR_IGNORE(snackbar->action_node, "style",
                                     "display: none;");
      }

      /* Mount to overlay director */
      if (!snackbar->overlay_handle) {
#define UI_OVR_DIR_MNT_IGNORE(d, c, z, o)                                      \
  ui_overlay_director_mount_component((d), (c), (z), (o))
        (void)UI_OVR_DIR_MNT_IGNORE(snackbar->director, snackbar->component,
                                    1000, &snackbar->overlay_handle);
      }
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_snackbar_base_process_event(struct ui_snackbar_base *snackbar,
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
      ui_error_t ac_rc = snackbar->current.action_callback(
          snackbar, snackbar->current.action_user_data);
      if (ac_rc != UI_ERROR_NONE)
        return ac_rc;
    }

    /* Dismiss on click */
    {
      ui_error_t dis_rc = ui_snackbar_base_dismiss_current(snackbar);
      if (dis_rc != UI_ERROR_NONE)
        return dis_rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_snackbar_base_bind_open(struct ui_snackbar_base *widget,
                                      struct ui_signal *open_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_snackbar_base_get_animating_signal(struct ui_snackbar_base *widget,
                                      struct ui_computed **out_animating) {
  if (!widget || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = widget->animating_signal;
  return UI_ERROR_NONE;
}
