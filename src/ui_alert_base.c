#include "ui_alert_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_alert_mock_fail = 0;
static ui_error_t mock_signal_set(struct ui_signal *signal,
                                  union ui_signal_payload payload) {
  if (g_alert_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return (ui_signal_set)(signal, payload);
}
static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *node,
                                              const char *name,
                                              const char *value) {
  if (g_alert_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_set_attribute)(node, name, value);
}
static ui_error_t mock_dom_node_remove_attribute(struct ui_dom_node *node,
                                                 const char *name) {
  if (g_alert_mock_fail == 3)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_remove_attribute)(node, name);
}

#undef ui_signal_set
#define ui_signal_set mock_signal_set
#undef ui_dom_node_set_attribute
#define ui_dom_node_set_attribute mock_dom_node_set_attribute
#undef ui_dom_node_remove_attribute
#define ui_dom_node_remove_attribute mock_dom_node_remove_attribute

static ui_error_t mock_on_dismiss_fail(struct ui_alert_base *alert, void *u) {
  (void)alert;
  (void)u;
  return UI_ERROR_UNKNOWN;
}

ui_error_t run_alert_coverage(void);
ui_error_t run_alert_coverage(void) {
  struct ui_alert_base *alert = NULL;
  struct ui_signal *sig = NULL;

  union ui_signal_payload initial;
  ui_alert_base_create(&alert);
  initial.bool_val = 0;
  ui_signal_create(NULL, initial, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                   UI_SIGNAL_MODE_SINGLE_THREADED, &sig);

  /* Line 261: on_dismiss fails */
  ui_alert_base_set_dismissible(alert, 1);
  ui_alert_base_set_on_dismiss(alert, mock_on_dismiss_fail, NULL);
  ui_alert_base_set_open(alert, 1);
  ui_alert_base_set_open(alert, 0); /* actually triggers dismiss */
  ui_alert_base_set_on_dismiss(alert, NULL, NULL);

  /* Line 272: signal_set fails */
  ui_alert_base_bind_open(alert, sig);
  g_alert_mock_fail = 1;
  ui_alert_base_set_open(alert, 1);
  g_alert_mock_fail = 0;

  /* Line 279: set_attribute fails */
  ui_alert_base_set_open(alert, 0);
  g_alert_mock_fail = 2;
  ui_alert_base_set_open(alert, 1);
  g_alert_mock_fail = 0;

  /* Line 283: remove_attribute fails */
  ui_alert_base_set_open(alert, 1); /* open first */
  g_alert_mock_fail = 3;
  ui_alert_base_set_open(alert, 0);
  g_alert_mock_fail = 0;

  (void)ui_signal_destroy(sig);
  (void)ui_alert_base_destroy(alert);
  return UI_ERROR_NONE;
}
#endif

struct ui_alert_base {
  struct ui_component *component;
  enum ui_alert_role role;
  int dismissible;
  ui_alert_on_dismiss_t on_dismiss;
  void *on_dismiss_data;
  struct ui_component *focus_return;
  int is_dismissed;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

/**
 * @brief Creates a new unstyled alert base component.
 *
 * @param out_alert Pointer to receive the allocated alert base.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_alert_base_create(struct ui_alert_base **out_alert) {
  struct ui_alert_base *alert;
  ui_error_t rc;

  if (!out_alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert = (struct ui_alert_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_alert_base));
  if (!alert) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  alert->role = UI_ALERT_ROLE_ALERT;
  alert->dismissible = 0;
  alert->on_dismiss = NULL;
  alert->on_dismiss_data = NULL;
  alert->focus_return = NULL;
  alert->is_dismissed = 0;
  alert->open_signal = NULL;
  alert->animating_signal = NULL;
  alert->component = NULL;

  rc = ui_component_create(&alert->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(alert);
    return rc;
  }

  {
    struct ui_dom_node *root_node = NULL;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
    if (rc != UI_ERROR_NONE) {
      (void)ui_component_destroy(alert->component);
      C_MULTIPLATFORM_FREE(alert);
      return rc;
    }

    rc = ui_dom_node_set_tag_name(root_node, "dialog");
    if (rc != UI_ERROR_NONE) {
      (void)ui_component_destroy(alert->component);
      C_MULTIPLATFORM_FREE(alert);
      return rc;
    }

    rc = ui_dom_node_set_attribute(root_node, "role", "alert");
    if (rc != UI_ERROR_NONE) {
      (void)ui_component_destroy(alert->component);
      C_MULTIPLATFORM_FREE(alert);
      return rc;
    }

    alert->component->shadow_root = root_node;
  }

  /* We could set the initial ARIA role on the shadow root or host node,
   * but typically this is managed by the layout or DOM mapping logic. */

  *out_alert = alert;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an alert base component.
 *
 * @param alert The alert to destroy.
 */
ui_error_t ui_alert_base_destroy(struct ui_alert_base *alert) {
  if (!alert) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(alert->component);
  C_MULTIPLATFORM_FREE(alert);
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the underlying component for the alert.
 *
 * @param alert The alert.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_get_component(struct ui_alert_base *alert,
                                       struct ui_component **out_component) {
  if (!alert || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = alert->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the semantic role of the alert.
 *
 * @param alert The alert.
 * @param role The role to assign (alert or status).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_role(struct ui_alert_base *alert,
                                  enum ui_alert_role role) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert->role = role;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the current semantic role of the alert.
 *
 * @param alert The alert.
 * @param out_role Pointer to receive the current role.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_get_role(struct ui_alert_base *alert,
                                  enum ui_alert_role *out_role) {
  if (!alert || !out_role) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_role = alert->role;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets whether the alert is dismissible.
 *
 * If set to true, typically a dismiss button is presented, and dismiss logic
 * applies.
 *
 * @param alert The alert.
 * @param dismissible True if dismissible.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_dismissible(struct ui_alert_base *alert,
                                         int dismissible) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert->dismissible = dismissible;
  return UI_ERROR_NONE;
}

/**
 * @brief Checks if the alert is dismissible.
 *
 * @param alert The alert.
 * @param out_dismissible Pointer to receive the dismissible state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_is_dismissible(struct ui_alert_base *alert,
                                        int *out_dismissible) {
  if (!alert || !out_dismissible) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_dismissible = alert->dismissible;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the callback invoked when the alert is dismissed.
 *
 * @param alert The alert.
 * @param on_dismiss The callback function.
 * @param user_data Opaque data passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_on_dismiss(struct ui_alert_base *alert,
                                        ui_alert_on_dismiss_t on_dismiss,
                                        void *user_data) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert->on_dismiss = on_dismiss;
  alert->on_dismiss_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Dismisses the alert, updating state, firing callbacks, and managing
 * focus.
 *
 * @param alert The alert.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_dismiss(struct ui_alert_base *alert) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_alert_base_set_open(alert, 0);
}

/**
 * @brief Sets the component to return focus to when this alert is dismissed.
 *
 * @param alert The alert.
 * @param focus_return The component to focus upon dismissal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_focus_return(struct ui_alert_base *alert,
                                          struct ui_component *focus_return) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert->focus_return = focus_return;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the open state of the alert.
 *
 * @param alert The alert.
 * @param is_open 1 to open, 0 to close (dismiss).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_set_open(struct ui_alert_base *alert, int is_open) {
  ui_error_t rc = UI_ERROR_NONE;
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_open == !alert->is_dismissed) {
    return UI_ERROR_NONE;
  }

  if (!is_open) {
    if (!alert->dismissible) {
      return UI_ERROR_UNSUPPORTED;
    }
    alert->is_dismissed = 1;

    if (alert->on_dismiss) {
      rc = alert->on_dismiss(alert, alert->on_dismiss_data);
      if (rc != UI_ERROR_NONE)
        return rc;
    }
  } else {
    alert->is_dismissed = 0;
  }

  if (alert->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = is_open;
    rc = ui_signal_set(alert->open_signal, payload);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  if (is_open) {
    rc = ui_dom_node_set_attribute(alert->component->shadow_root, "open",
                                   "true");
    if (rc != UI_ERROR_NONE)
      return rc;
  } else {
    rc = ui_dom_node_remove_attribute(alert->component->shadow_root, "open");
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Checks if the alert is currently open.
 *
 * @param alert The alert.
 * @param out_is_open Pointer to receive the open state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_is_open(const struct ui_alert_base *alert,
                                 int *out_is_open) {
  if (!alert || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = !alert->is_dismissed;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the alert's open state to a signal.
 *
 * @param alert The alert.
 * @param open_signal The boolean signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_alert_base_bind_open(struct ui_alert_base *alert,
                                   struct ui_signal *open_signal) {
  if (!alert) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  alert->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the computed signal indicating if the alert is animating.
 *
 * @param alert The alert.
 * @param out_animating Pointer to receive the computed signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_alert_base_get_animating_signal(struct ui_alert_base *alert,
                                   struct ui_computed **out_animating) {
  if (!alert || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = alert->animating_signal;
  return UI_ERROR_NONE;
}
