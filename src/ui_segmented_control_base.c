#include "ui_segmented_control_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

struct ui_segmented_control_base {
  struct ui_component *component;
  enum ui_segmented_control_mode mode;

  struct ui_segmented_button_base **buttons;
  int button_count;
  int button_capacity;

  ui_error_t (*cva_on_change)(union ui_signal_payload new_value,
                              void *user_data);
  void *cva_on_change_user_data;
  ui_error_t (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

/** \brief ui_segmented_button_base */
struct ui_segmented_button_base {
  struct ui_component *component;
  int selected;
  struct ui_segmented_control_base *parent;
  int index;
};
static ui_error_t
trigger_cva_change(struct ui_segmented_control_base *control,
                   struct ui_segmented_button_base *active_button) {
  (void)active_button;
  if (control && control->cva_on_change) {
    union ui_signal_payload payload;
    int i;
    int active_idx = -1;
    /* Find active index for SINGLE mode */
    for (i = 0; i < control->button_count; ++i) {
      if (control->buttons[i]->selected) {
        active_idx = i;
        break;
      }
    }
    payload.int_val = active_idx;
    return control->cva_on_change(payload, control->cva_on_change_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t
trigger_cva_touched(struct ui_segmented_control_base *control) {
  if (control && control->cva_on_touched) {
    return control->cva_on_touched(control->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static ui_error_t segmented_cva_write_value(void *component,
                                            union ui_signal_payload value) {
  struct ui_segmented_control_base *control =
      (struct ui_segmented_control_base *)component;
  int index;
  int i;

  if (!control)
    return UI_ERROR_INVALID_ARGUMENT;

  index = value.int_val;

  for (i = 0; i < control->button_count; ++i) {
    control->buttons[i]->selected = (i == index);
  }

  return UI_ERROR_NONE;
}

/** \brief segmented_cva_register_on_change */
static ui_error_t segmented_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_segmented_control_base *control =
      (struct ui_segmented_control_base *)component;
  if (!control)
    return UI_ERROR_INVALID_ARGUMENT;
  control->cva_on_change = callback;
  control->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t segmented_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_segmented_control_base *control =
      (struct ui_segmented_control_base *)component;
  if (!control)
    return UI_ERROR_INVALID_ARGUMENT;
  control->cva_on_touched = callback;
  control->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t segmented_cva_set_disabled_state(void *component,
                                                   int is_disabled) {
  struct ui_segmented_control_base *control =
      (struct ui_segmented_control_base *)component;
  if (!control)
    return UI_ERROR_INVALID_ARGUMENT;
  control->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_control_base_create(struct ui_segmented_control_base **out_control,
                                 struct ui_control_value_accessor *out_cva) {
  struct ui_segmented_control_base *control;
  ui_error_t rc;

  if (!out_control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  control = (struct ui_segmented_control_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_segmented_control_base));
  if (!control) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  control->buttons = NULL;
  control->button_count = 0;
  control->button_capacity = 0;
  control->cva_on_change = NULL;
  control->cva_on_change_user_data = NULL;
  control->cva_on_touched = NULL;
  control->cva_on_touched_user_data = NULL;
  control->is_disabled = 0;

  rc = ui_component_create(&control->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(control);
    return rc;
  }

  control->mode = UI_SEGMENTED_CONTROL_MODE_SINGLE;

  if (out_cva) {
    out_cva->write_value = segmented_cva_write_value;
    out_cva->register_on_change = segmented_cva_register_on_change;
    out_cva->register_on_touched = segmented_cva_register_on_touched;
    out_cva->set_disabled_state = segmented_cva_set_disabled_state;
  }

  /* Real implementation would assign ARIA role radiogroup or tablist based on
   * usage context */

  *out_control = control;
  return UI_ERROR_NONE;
}

/** \brief ui_segmented_control_base_destroy */
ui_error_t
ui_segmented_control_base_destroy(struct ui_segmented_control_base *control) {
  if (!control) {
    return UI_ERROR_NONE;
  }
  if (control->component) {
    (void)ui_component_destroy(control->component);
  }
  if (control->buttons) {
    C_MULTIPLATFORM_FREE(control->buttons);
  }
  C_MULTIPLATFORM_FREE(control);
  return UI_ERROR_NONE;
}

/** \brief ui_segmented_control_base_get_component */
ui_error_t ui_segmented_control_base_get_component(
    struct ui_segmented_control_base *control,
    struct ui_component **out_component) {
  if (!control || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = control->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_control_base_set_mode(struct ui_segmented_control_base *control,
                                   enum ui_segmented_control_mode mode) {
  if (!control) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  control->mode = mode;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_control_base_get_mode(struct ui_segmented_control_base *control,
                                   enum ui_segmented_control_mode *out_mode) {
  if (!control || !out_mode) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_mode = control->mode;
  return UI_ERROR_NONE;
}

/** \brief ui_segmented_control_base_append_segment */
ui_error_t ui_segmented_control_base_append_segment(
    struct ui_segmented_control_base *control,
    struct ui_segmented_button_base *button) {
  if (!control || !button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (control->button_count >= control->button_capacity) {
    int new_cap =
        control->button_capacity == 0 ? 4 : control->button_capacity * 2;
    struct ui_segmented_button_base **new_arr =
        (struct ui_segmented_button_base **)C_MULTIPLATFORM_REALLOC(
            control->buttons,
            new_cap * sizeof(struct ui_segmented_button_base *));
    if (!new_arr) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    control->buttons = new_arr;
    control->button_capacity = new_cap;
  }

  button->parent = control;
  button->index = control->button_count;
  control->buttons[control->button_count++] = button;

  /* Structural DOM/Shadow Root appending logic would exist here */

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_button_base_create(struct ui_segmented_button_base **out_button) {
  struct ui_segmented_button_base *button;
  ui_error_t rc;

  if (!out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button = (struct ui_segmented_button_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_segmented_button_base));
  if (!button) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&button->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(button);
    return rc;
  }

  button->selected = 0;
  button->parent = NULL;
  button->index = -1;

  /* Sets initial unselected state visually and ARIA properties */

  *out_button = button;
  return UI_ERROR_NONE;
}

ui_error_t
ui_segmented_button_base_destroy(struct ui_segmented_button_base *button) {
  if (!button) {
    return UI_ERROR_NONE;
  }
  if (button->component) {
    (void)ui_component_destroy(button->component);
  }
  C_MULTIPLATFORM_FREE(button);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_button_base_get_component(struct ui_segmented_button_base *button,
                                       struct ui_component **out_component) {
  if (!button || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = button->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_button_base_set_selected(struct ui_segmented_button_base *button,
                                      int selected) {
  int i;
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->parent && button->parent->is_disabled) {
    return UI_ERROR_NONE;
  }

  if (button->selected != selected) {
    button->selected = selected;

    if (button->parent) {
      ui_error_t t_rc = trigger_cva_touched(button->parent);
      if (t_rc != UI_ERROR_NONE)
        return t_rc;

      if (selected &&
          button->parent->mode == UI_SEGMENTED_CONTROL_MODE_SINGLE) {
        /* Deselect others */
        for (i = 0; i < button->parent->button_count; i++) {
          if (button->parent->buttons[i] != button) {
            button->parent->buttons[i]->selected = 0;
          }
        }
      }
      return trigger_cva_change(button->parent, button);
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_segmented_button_base_get_selected(struct ui_segmented_button_base *button,
                                      int *out_selected) {
  if (!button || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_selected = button->selected;
  return UI_ERROR_NONE;
}
