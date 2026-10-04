/**
 * @file md2_selection_controls.c
 * @brief Implementation of Material Design 2 Selection Controls.
 */

/* clang-format off */
#include "material2/md2_selection_controls.h"
#include <stdlib.h>
/* clang-format on */

struct md2_checkbox {
  struct ui_checkbox_base *base;
};

ui_error_t md2_checkbox_create(struct md2_checkbox **out_checkbox) {
  struct md2_checkbox *checkbox;
  ui_error_t rc;

  if (out_checkbox == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  checkbox = (struct md2_checkbox *)malloc(sizeof(struct md2_checkbox));
  if (checkbox == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_checkbox_base_create(&checkbox->base);
  if (rc != UI_ERROR_NONE) {
    free(checkbox);
    return rc;
  }

  *out_checkbox = checkbox;
  return UI_ERROR_NONE;
}

ui_error_t md2_checkbox_destroy(struct md2_checkbox *checkbox) {
  if (checkbox == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_checkbox_base_destroy(checkbox->base);
  free(checkbox);
  return UI_ERROR_NONE;
}

ui_error_t md2_checkbox_get_component(struct md2_checkbox *checkbox,
                                      struct ui_component **out_component) {
  if (checkbox == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_checkbox_base_get_component(checkbox->base, out_component);
}

struct md2_radio_button {
  struct ui_radio_group_base *base;
};

ui_error_t md2_radio_button_create(struct md2_radio_button **out_radio) {
  struct md2_radio_button *radio;
  ui_error_t rc;

  if (out_radio == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  radio = (struct md2_radio_button *)malloc(sizeof(struct md2_radio_button));
  if (radio == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_radio_group_base_create(&radio->base, NULL);
  if (rc != UI_ERROR_NONE) {
    free(radio);
    return rc;
  }

  *out_radio = radio;
  return UI_ERROR_NONE;
}

ui_error_t md2_radio_button_destroy(struct md2_radio_button *radio) {
  if (radio == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_radio_group_base_destroy(radio->base);
  free(radio);
  return UI_ERROR_NONE;
}

ui_error_t md2_radio_button_get_base(struct md2_radio_button *radio,
                                     struct ui_radio_group_base **out_base) {
  if (radio == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = radio->base;
  return UI_ERROR_NONE;
}

struct md2_switch {
  struct ui_slide_toggle_base *base;
};

ui_error_t md2_switch_create(struct md2_switch **out_switch) {
  struct md2_switch *sw;
  ui_error_t rc;

  if (out_switch == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sw = (struct md2_switch *)malloc(sizeof(struct md2_switch));
  if (sw == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_slide_toggle_base_create(&sw->base, NULL);
  if (rc != UI_ERROR_NONE) {
    free(sw);
    return rc;
  }

  *out_switch = sw;
  return UI_ERROR_NONE;
}

ui_error_t md2_switch_destroy(struct md2_switch *sw) {
  if (sw == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_slide_toggle_base_destroy(sw->base);
  free(sw);
  return UI_ERROR_NONE;
}

ui_error_t md2_switch_get_base(struct md2_switch *sw,
                               struct ui_slide_toggle_base **out_base) {
  if (sw == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = sw->base;
  return UI_ERROR_NONE;
}

struct md2_slider {
  struct ui_slider_base *base;
};

ui_error_t md2_slider_create(struct md2_slider **out_slider) {
  struct md2_slider *slider;
  ui_error_t rc;

  if (out_slider == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  slider = (struct md2_slider *)malloc(sizeof(struct md2_slider));
  if (slider == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_slider_base_create(&slider->base, NULL);
  if (rc != UI_ERROR_NONE) {
    free(slider);
    return rc;
  }

  *out_slider = slider;
  return UI_ERROR_NONE;
}

ui_error_t md2_slider_destroy(struct md2_slider *slider) {
  if (slider == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_slider_base_destroy(slider->base);
  free(slider);
  return UI_ERROR_NONE;
}

ui_error_t md2_slider_get_base(struct md2_slider *slider,
                               struct ui_slider_base **out_base) {
  if (slider == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = slider->base;
  return UI_ERROR_NONE;
}
