/**
 * @file ui_spin_button_base.c
 * @brief ui_spin_button_base.c implementation.
 */
/* clang-format off */
#include "ui_spin_button_base.h"
#include "ui_aria.h"
#include "ui_arena.h"
#include "ui_event.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
/* clang-format on */

/*
 * \file ui_spin_button_base.c
 * \brief Spin button base component implementation.
 */

/** @cond */
#define UI_SPIN_BUTTON_INITIAL_DELAY_MS 500.0
/** @endcond */
/** @cond */
#define UI_SPIN_BUTTON_REPEAT_RATE_MS 50.0
/** @endcond */

/**
 * \brief ui_spin_button_dir enum.
 * \details Spin button continuous scroll directions.
 */
/**
 * @brief ui_spin_button_dir.
 */
enum ui_spin_button_dir {
  UI_SPIN_BUTTON_DIR_NONE = 0, /**< No direction */
  UI_SPIN_BUTTON_DIR_INC = 1,  /**< Increment direction */
  UI_SPIN_BUTTON_DIR_DEC = 2   /**< Decrement direction */
};

/**
 * @struct ui_spin_button_base
 * \brief ui_spin_button_base structure.
 * \details Internal state for the spin button base component.
 */
struct ui_spin_button_base {
  struct ui_component *component; /**< component */
  double value;                   /**< value */
  double min_val;                 /**< min_val */
  double max_val;                 /**< max_val */
  double step;                    /**< step */
  int disabled;                   /**< disabled */

  enum ui_spin_button_dir continuous_dir; /**< continuous_dir */
  double continuous_timer_ms;             /**< continuous_timer_ms */
  int is_repeating;                       /**< is_repeating */

  ui_spin_button_on_change_t on_change; /**< on_change */
  void *on_change_user_data;            /**< on_change_user_data */

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data); /**< user_data) */
  void *cva_on_change_user_data;                /**< cva_on_change_user_data */

  ui_error_t (*cva_on_touched)(void *user_data); /**< user_data) */
  void *cva_on_touched_user_data; /**< cva_on_touched_user_data */
};

/**
 * \brief Triggers a CVA change event.
 * \param spin_button The component.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t trigger_cva_change(struct ui_spin_button_base *spin_button);
/**
 * \brief Triggers a CVA touched event.
 * \param spin_button The component.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t trigger_cva_touched(struct ui_spin_button_base *spin_button);
/**
 * \brief Sets the disabled state.
 * \param spin_button The spin button.
 * \param disabled The disabled state.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_disabled(struct ui_spin_button_base *spin_button,
                                 int disabled);

/** @cond */
#define UI_DOM_SET_ATTR_IGNORE(n, a, v) ui_dom_node_set_attribute((n), (a), (v))
/** @endcond */
/** @cond */
#define UI_DOM_REM_ATTR_IGNORE(n, a) ui_dom_node_remove_attribute((n), (a))
/** @endcond */

/**
 * \brief Updates ARIA attributes for the spin button.
 * \param spin_button The component.
 * \return UI_ERROR_NONE on success.
 */
static ui_error_t
/**
 * @brief ui_spin_button_base_update_aria.
 * @param spin_button Parameter spin_button.
 * @return Return value.
 */
ui_spin_button_base_update_aria(struct ui_spin_button_base *spin_button) {
  char buf[64];
  if (!spin_button->component || !spin_button->component->shadow_root)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->value);
#else
  sprintf(buf, "%.2f", spin_button->value);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(spin_button->component->shadow_root,
                               "aria-valuenow", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->min_val);
#else
  sprintf(buf, "%.2f", spin_button->min_val);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(spin_button->component->shadow_root,
                               "aria-valuemin", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->max_val);
#else
  sprintf(buf, "%.2f", spin_button->max_val);
#endif
  (void)UI_DOM_SET_ATTR_IGNORE(spin_button->component->shadow_root,
                               "aria-valuemax", buf);

  return UI_ERROR_NONE;
}

/** @cond */
#define UI_TRIG_CVA_CHG_IGNORE(s) trigger_cva_change((s))
/** @endcond */
/** @cond */
#define UI_TRIG_CVA_TOUCH_IGNORE(s) trigger_cva_touched((s))
/** @endcond */

/**
 * @brief trigger_cva_change.
 * @param spin_button Parameter spin_button.
 * @return Return value.
 */
static ui_error_t trigger_cva_change(struct ui_spin_button_base *spin_button) {
  if (spin_button->cva_on_change) {
    union ui_signal_payload payload;
    payload.float_val = (float)spin_button->value;
/** @cond */
#define UI_CVA_ON_CHG_IGNORE(cb, p, u) ((cb)((p), (u)))
    /** @endcond */
    (void)UI_CVA_ON_CHG_IGNORE(spin_button->cva_on_change, payload,
                               spin_button->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief trigger_cva_touched.
 * @param spin_button Parameter spin_button.
 * @return Return value.
 */
static ui_error_t trigger_cva_touched(struct ui_spin_button_base *spin_button) {
  if (spin_button->cva_on_touched) {
/** @cond */
#define UI_CVA_ON_TOUCH_IGNORE(cb, u) ((cb)((u)))
    /** @endcond */
    (void)UI_CVA_ON_TOUCH_IGNORE(spin_button->cva_on_touched,
                                 spin_button->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Writes a value from CVA.
 * \param component The spin button.
 * \param value The value to write.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_button_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t spin_button_cva_write_value(void *component,
                                              union ui_signal_payload value) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;

  ui_error_t set_rc;
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  set_rc = ui_spin_button_base_set_value(spin_button, (double)value.float_val);
  if (set_rc != UI_ERROR_NONE)
    return set_rc;
  return UI_ERROR_NONE;
}

/**
 * \brief Registers an on-change callback for CVA.
 * \param component The spin button.
 * \param callback The callback to register.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_button_cva_register_on_change.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t spin_button_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  spin_button->cva_on_change = callback;
  spin_button->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Registers an on-touched callback for CVA.
 * \param component The spin button.
 * \param callback The callback to register.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_button_cva_register_on_touched.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t spin_button_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  spin_button->cva_on_touched = callback;
  spin_button->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the disabled state from CVA.
 * \param component The spin button.
 * \param is_disabled The disabled state.
 * \return UI_ERROR_NONE on success.
 */
/**
 * @brief spin_button_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t spin_button_cva_set_disabled_state(void *component,
                                                     int is_disabled) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  ui_error_t set_rc;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  set_rc = ui_spin_button_base_set_disabled(spin_button, is_disabled);
  if (set_rc != UI_ERROR_NONE)
    return set_rc;
  return UI_ERROR_NONE;
}

/**
 * \brief Creates a new spin button base component.
 * \param out_spin_button Pointer to store the component.
 * \param out_cva Optional pointer to store the CVA.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_create(struct ui_spin_button_base **out_spin_button,
                           struct ui_control_value_accessor *out_cva) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_spin_button_base *spin_button = NULL;
  struct ui_dom_node *root_node = NULL;

  if (!out_spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spin_button = (struct ui_spin_button_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_spin_button_base));
  if (!spin_button) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  spin_button->component = NULL;

  rc = ui_component_create(&spin_button->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(spin_button);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    (void)ui_component_destroy(spin_button->component);
    C_MULTIPLATFORM_FREE(spin_button);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(spin_button->component);
    C_MULTIPLATFORM_FREE(spin_button);
    return rc;
  }

  spin_button->component->shadow_root = root_node;
  spin_button->value = 0.0;
  spin_button->min_val = 0.0;
  spin_button->max_val = 100.0;
  spin_button->step = 1.0;
  spin_button->disabled = 0;
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_NONE;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  spin_button->on_change = NULL;
  spin_button->on_change_user_data = NULL;
  spin_button->cva_on_change = NULL;
  spin_button->cva_on_change_user_data = NULL;
  spin_button->cva_on_touched = NULL;
  spin_button->cva_on_touched_user_data = NULL;

  (void)UI_DOM_SET_ATTR_IGNORE(root_node, "role", "spinbutton");
  (void)UI_DOM_SET_ATTR_IGNORE(root_node, "tabindex", "0");
/** @cond */
#define UI_SPIN_UPDATE_ARIA_IGNORE(s) ui_spin_button_base_update_aria((s))
  /** @endcond */
  (void)UI_SPIN_UPDATE_ARIA_IGNORE(spin_button);

  if (out_cva) {
    out_cva->write_value = spin_button_cva_write_value;
    out_cva->register_on_change = spin_button_cva_register_on_change;
    out_cva->register_on_touched = spin_button_cva_register_on_touched;
    out_cva->set_disabled_state = spin_button_cva_set_disabled_state;
  }

  *out_spin_button = spin_button;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a spin button base component.
 * \param spin_button The component to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_destroy(struct ui_spin_button_base *spin_button) {
  if (!spin_button) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(spin_button->component);
  C_MULTIPLATFORM_FREE(spin_button);
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the minimum value.
 * \param spin_button The spin button.
 * \param min_val The minimum value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_min(struct ui_spin_button_base *spin_button,
                                       double min_val) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->min_val = min_val;
  if (spin_button->value < spin_button->min_val) {
    ui_error_t set_rc;
    set_rc = ui_spin_button_base_set_value(spin_button, spin_button->min_val);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  } else {
/** @cond */
#define UI_SPIN_UPDATE_ARIA_IGNORE(s) ui_spin_button_base_update_aria((s))
    /** @endcond */
    (void)UI_SPIN_UPDATE_ARIA_IGNORE(spin_button);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the maximum value.
 * \param spin_button The spin button.
 * \param max_val The maximum value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_max(struct ui_spin_button_base *spin_button,
                                       double max_val) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->max_val = max_val;
  if (spin_button->value > spin_button->max_val) {
    ui_error_t set_rc;
    set_rc = ui_spin_button_base_set_value(spin_button, spin_button->max_val);
    if (set_rc != UI_ERROR_NONE)
      return set_rc;
  } else {
/** @cond */
#define UI_SPIN_UPDATE_ARIA_IGNORE(s) ui_spin_button_base_update_aria((s))
    /** @endcond */
    (void)UI_SPIN_UPDATE_ARIA_IGNORE(spin_button);
  }
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the current value.
 * \param spin_button The spin button.
 * \param value The value to set.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_value(struct ui_spin_button_base *spin_button,
                              double value) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value < spin_button->min_val) {
    value = spin_button->min_val;
  }
  if (value > spin_button->max_val) {
    value = spin_button->max_val;
  }

  if (spin_button->value != value) {
    spin_button->value = value;
/** @cond */
#define UI_SPIN_UPDATE_ARIA_IGNORE(s) ui_spin_button_base_update_aria((s))
    /** @endcond */
    (void)UI_SPIN_UPDATE_ARIA_IGNORE(spin_button);

    if (spin_button->on_change) {
      ui_error_t oc_rc = spin_button->on_change(
          spin_button, spin_button->value, spin_button->on_change_user_data);
      if (oc_rc != UI_ERROR_NONE)
        return oc_rc;
    }
    (void)UI_TRIG_CVA_CHG_IGNORE(spin_button);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the current value.
 * \param spin_button The spin button.
 * \param out_val Pointer to store the value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_get_value(const struct ui_spin_button_base *spin_button,
                              double *out_val) {
  if (!spin_button || !out_val)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_val = spin_button->value;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the step value.
 * \param spin_button The spin button.
 * \param step The step value.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_set_step(struct ui_spin_button_base *spin_button,
                                        double step) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->step = step;
  return UI_ERROR_NONE;
}

/**
 * \brief Sets the disabled state.
 * \param spin_button The spin button.
 * \param disabled The disabled state.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_disabled(struct ui_spin_button_base *spin_button,
                                 int disabled) {
  if (!spin_button || !spin_button->component ||
      !spin_button->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spin_button->disabled = disabled;
  if (disabled) {
    (void)UI_DOM_SET_ATTR_IGNORE(spin_button->component->shadow_root,
                                 "aria-disabled", "true");
    (void)UI_DOM_REM_ATTR_IGNORE(spin_button->component->shadow_root,
                                 "tabindex");
    (void)ui_spin_button_base_stop_continuous(spin_button);
  } else {
    (void)UI_DOM_REM_ATTR_IGNORE(spin_button->component->shadow_root,
                                 "aria-disabled");
    (void)UI_DOM_SET_ATTR_IGNORE(spin_button->component->shadow_root,
                                 "tabindex", "0");
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Sets the on change callback.
 * \param spin_button The spin button.
 * \param on_change The callback function.
 * \param user_data User data for the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_set_on_change(struct ui_spin_button_base *spin_button,
                                  ui_spin_button_on_change_t on_change,
                                  void *user_data) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->on_change = on_change;
  spin_button->on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * \brief Increments the spin button value.
 * \param spin_button The spin button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_increment(struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_value(spin_button,
                                       spin_button->value + spin_button->step);
}

/**
 * \brief Decrements the spin button value.
 * \param spin_button The spin button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_decrement(struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_value(spin_button,
                                       spin_button->value - spin_button->step);
}

/**
 * \brief Starts continuous incrementation.
 * \param spin_button The spin button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_start_continuous_increment(
    struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_INC;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return ui_spin_button_base_increment(spin_button);
}

/**
 * \brief Starts continuous decrementation.
 * \param spin_button The spin button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_start_continuous_decrement(
    struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_DEC;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return ui_spin_button_base_decrement(spin_button);
}

/**
 * \brief Stops continuous value change.
 * \param spin_button The spin button.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_stop_continuous(struct ui_spin_button_base *spin_button) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_NONE;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return UI_ERROR_NONE;
}

/**
 * \brief Triggers a tick update for continuous scrolling.
 * \param spin_button The spin button.
 * \param delta_ms The time since the last tick in ms.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_spin_button_base_on_tick(struct ui_spin_button_base *spin_button,
                                       double delta_ms) {
  double threshold;

  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (spin_button->continuous_dir == UI_SPIN_BUTTON_DIR_NONE) {
    return UI_ERROR_NONE;
  }

  spin_button->continuous_timer_ms += delta_ms;

  threshold = spin_button->is_repeating ? UI_SPIN_BUTTON_REPEAT_RATE_MS
                                        : UI_SPIN_BUTTON_INITIAL_DELAY_MS;

  if (spin_button->continuous_timer_ms >= threshold) {
    spin_button->continuous_timer_ms = 0.0;
    spin_button->is_repeating = 1;

    if (spin_button->continuous_dir == UI_SPIN_BUTTON_DIR_INC) {
      ui_error_t inc_rc = ui_spin_button_base_increment(spin_button);
      if (inc_rc != UI_ERROR_NONE)
        return inc_rc;
    } else {
      ui_error_t dec_rc = ui_spin_button_base_decrement(spin_button);
      if (dec_rc != UI_ERROR_NONE)
        return dec_rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Processes an event.
 * \param spin_button The spin button.
 * \param event The event.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_process_event(struct ui_spin_button_base *spin_button,
                                  const struct ui_event *event) {
  if (!spin_button || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (spin_button->disabled) {
    return UI_ERROR_NONE;
  }

  (void)UI_TRIG_CVA_TOUCH_IGNORE(spin_button);

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_UP) {
      ui_error_t inc_rc = ui_spin_button_base_increment(spin_button);
      if (inc_rc != UI_ERROR_NONE)
        return inc_rc;
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
      ui_error_t dec_rc = ui_spin_button_base_decrement(spin_button);
      if (dec_rc != UI_ERROR_NONE)
        return dec_rc;
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_HOME) {
      ui_error_t set_rc;
      set_rc = ui_spin_button_base_set_value(spin_button, spin_button->min_val);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_END) {
      ui_error_t set_rc;
      set_rc = ui_spin_button_base_set_value(spin_button, spin_button->max_val);
      if (set_rc != UI_ERROR_NONE)
        return set_rc;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Gets the base component for the spin button.
 * \param spin_button The spin button.
 * \param out_component Pointer to store the component.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_spin_button_base_get_component(struct ui_spin_button_base *spin_button,
                                  struct ui_component **out_component) {
  if (!spin_button || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = spin_button->component;
  return UI_ERROR_NONE;
}
