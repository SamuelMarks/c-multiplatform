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

#define UI_SPIN_BUTTON_INITIAL_DELAY_MS 500.0
#define UI_SPIN_BUTTON_REPEAT_RATE_MS 50.0

enum ui_spin_button_dir {
  UI_SPIN_BUTTON_DIR_NONE = 0,
  UI_SPIN_BUTTON_DIR_INC = 1,
  UI_SPIN_BUTTON_DIR_DEC = 2
};

/** \brief ui_spin_button_base */
struct ui_spin_button_base {
  struct ui_component *component;
  double value;
  double min_val;
  double max_val;
  double step;
  int disabled;

  enum ui_spin_button_dir continuous_dir;
  double continuous_timer_ms;
  int is_repeating;

  ui_spin_button_on_change_t on_change;
  void *on_change_user_data;

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;
};

static enum ui_error
trigger_cva_change(struct ui_spin_button_base *spin_button);
static enum ui_error
trigger_cva_touched(struct ui_spin_button_base *spin_button);
/** \brief ui_error */
enum ui_error
ui_spin_button_base_set_disabled(struct ui_spin_button_base *spin_button,
                                 int disabled);

static enum ui_error
ui_spin_button_base_update_aria(struct ui_spin_button_base *spin_button) {
  char buf[64];
  if (!spin_button || !spin_button->component ||
      !spin_button->component->shadow_root)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->value);
#else
  sprintf(buf, "%.2f", spin_button->value);
#endif
  ui_dom_node_set_attribute(spin_button->component->shadow_root,
                            "aria-valuenow", buf);
  return UI_ERROR_NONE;

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->min_val);
#else
  sprintf(buf, "%.2f", spin_button->min_val);
#endif
  ui_dom_node_set_attribute(spin_button->component->shadow_root,
                            "aria-valuemin", buf);

#if defined(_MSC_VER)
  sprintf_s(buf, sizeof(buf), "%.2f", spin_button->max_val);
#else
  sprintf(buf, "%.2f", spin_button->max_val);
#endif
  ui_dom_node_set_attribute(spin_button->component->shadow_root,
                            "aria-valuemax", buf);

  return UI_ERROR_NONE;
}

static enum ui_error
trigger_cva_change(struct ui_spin_button_base *spin_button) {
  if (spin_button && spin_button->cva_on_change) {
    union ui_signal_payload payload;
    payload.float_val = (float)spin_button->value;
    (void)spin_button->cva_on_change(payload,
                                     spin_button->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error
trigger_cva_touched(struct ui_spin_button_base *spin_button) {
  if (spin_button && spin_button->cva_on_touched) {
    (void)spin_button->cva_on_touched(spin_button->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error
spin_button_cva_write_value(void *component, union ui_signal_payload value) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;

  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_spin_button_base_set_value(spin_button, (double)value.float_val);
  return UI_ERROR_NONE;
}

/** \brief spin_button_cva_register_on_change */
static enum ui_error spin_button_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  spin_button->cva_on_change = callback;
  spin_button->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error
spin_button_cva_register_on_touched(void *component,
                                    enum ui_error (*callback)(void *user_data),
                                    void *user_data) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  spin_button->cva_on_touched = callback;
  spin_button->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error spin_button_cva_set_disabled_state(void *component,
                                                        int is_disabled) {
  struct ui_spin_button_base *spin_button =
      (struct ui_spin_button_base *)component;
  if (!spin_button)
    return UI_ERROR_INVALID_ARGUMENT;
  ui_spin_button_base_set_disabled(spin_button, is_disabled);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_create(struct ui_spin_button_base **out_spin_button,
                           struct ui_control_value_accessor *out_cva) {
  enum ui_error rc = UI_ERROR_NONE;
  struct ui_spin_button_base *spin_button = NULL;
  struct ui_dom_node *root_node = NULL;

  if (!out_spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spin_button = (struct ui_spin_button_base *)UI_MALLOC(
      sizeof(struct ui_spin_button_base));
  if (!spin_button) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  spin_button->component = NULL;

  rc = ui_component_create(&spin_button->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(spin_button);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(spin_button->component);
    UI_FREE(spin_button);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "div");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(root_node);
    ui_component_destroy(spin_button->component);
    UI_FREE(spin_button);
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

  ui_dom_node_set_attribute(root_node, "role", "spinbutton");
  ui_dom_node_set_attribute(root_node, "tabindex", "0");
  ui_spin_button_base_update_aria(spin_button);

  if (out_cva) {
    out_cva->write_value = spin_button_cva_write_value;
    out_cva->register_on_change = spin_button_cva_register_on_change;
    out_cva->register_on_touched = spin_button_cva_register_on_touched;
    out_cva->set_disabled_state = spin_button_cva_set_disabled_state;
  }

  *out_spin_button = spin_button;
  return UI_ERROR_NONE;
}

void ui_spin_button_base_destroy(struct ui_spin_button_base *spin_button) {
  if (!spin_button) {
    return;
  }
  if (spin_button->component) {
    ui_component_destroy(spin_button->component);
  }
  UI_FREE(spin_button);
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_set_min(struct ui_spin_button_base *spin_button,
                            double min_val) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->min_val = min_val;
  if (spin_button->value < spin_button->min_val) {
    ui_spin_button_base_set_value(spin_button, spin_button->min_val);
  } else {
    ui_spin_button_base_update_aria(spin_button);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_set_max(struct ui_spin_button_base *spin_button,
                            double max_val) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->max_val = max_val;
  if (spin_button->value > spin_button->max_val) {
    ui_spin_button_base_set_value(spin_button, spin_button->max_val);
  } else {
    ui_spin_button_base_update_aria(spin_button);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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
    ui_spin_button_base_update_aria(spin_button);

    if (spin_button->on_change) {
      spin_button->on_change(spin_button, spin_button->value,
                             spin_button->on_change_user_data);
    }
    (void)trigger_cva_change(spin_button);
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_get_value(const struct ui_spin_button_base *spin_button,
                              double *out_val) {
  if (!spin_button || !out_val)
    return UI_ERROR_INVALID_ARGUMENT;
  *out_val = spin_button->value;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_set_step(struct ui_spin_button_base *spin_button,
                             double step) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->step = step;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_set_disabled(struct ui_spin_button_base *spin_button,
                                 int disabled) {
  if (!spin_button || !spin_button->component ||
      !spin_button->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spin_button->disabled = disabled;
  if (disabled) {
    ui_dom_node_set_attribute(spin_button->component->shadow_root,
                              "aria-disabled", "true");
    ui_dom_node_remove_attribute(spin_button->component->shadow_root,
                                 "tabindex");
    ui_spin_button_base_stop_continuous(spin_button);
  } else {
    ui_dom_node_remove_attribute(spin_button->component->shadow_root,
                                 "aria-disabled");
    ui_dom_node_set_attribute(spin_button->component->shadow_root, "tabindex",
                              "0");
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
ui_spin_button_base_increment(struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_value(spin_button,
                                       spin_button->value + spin_button->step);
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_decrement(struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_spin_button_base_set_value(spin_button,
                                       spin_button->value - spin_button->step);
}

/** \brief ui_spin_button_base_start_continuous_increment */
enum ui_error ui_spin_button_base_start_continuous_increment(
    struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_INC;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return ui_spin_button_base_increment(spin_button);
}

/** \brief ui_spin_button_base_start_continuous_decrement */
enum ui_error ui_spin_button_base_start_continuous_decrement(
    struct ui_spin_button_base *spin_button) {
  if (!spin_button || spin_button->disabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_DEC;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return ui_spin_button_base_decrement(spin_button);
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_stop_continuous(struct ui_spin_button_base *spin_button) {
  if (!spin_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spin_button->continuous_dir = UI_SPIN_BUTTON_DIR_NONE;
  spin_button->continuous_timer_ms = 0.0;
  spin_button->is_repeating = 0;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_on_tick(struct ui_spin_button_base *spin_button,
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
      ui_spin_button_base_increment(spin_button);
    } else if (spin_button->continuous_dir == UI_SPIN_BUTTON_DIR_DEC) {
      ui_spin_button_base_decrement(spin_button);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_process_event(struct ui_spin_button_base *spin_button,
                                  const struct ui_event *event) {
  if (!spin_button || !event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (spin_button->disabled) {
    return UI_ERROR_NONE;
  }

  (void)trigger_cva_touched(spin_button);

  if (event->type == UI_EVENT_KEY_DOWN) {
    if (event->event_data.keyboard.key_code == UI_KEY_UP) {
      ui_spin_button_base_increment(spin_button);
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_DOWN) {
      ui_spin_button_base_decrement(spin_button);
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_HOME) {
      ui_spin_button_base_set_value(spin_button, spin_button->min_val);
      return UI_ERROR_NONE;
    } else if (event->event_data.keyboard.key_code == UI_KEY_END) {
      ui_spin_button_base_set_value(spin_button, spin_button->max_val);
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_spin_button_base_get_component(struct ui_spin_button_base *spin_button,
                                  struct ui_component **out_component) {
  if (!spin_button || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = spin_button->component;
  return UI_ERROR_NONE;
}
