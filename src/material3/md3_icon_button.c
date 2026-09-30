/**
 * @file md3_icon_button.c
 * @brief Material 3 Icon Button component implementation.
 */

/* clang-format off */
#include "material3/md3_icon_button.h"
#include "ui_internal_mem.h"
#include "ui_aria.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_icon_button_create(struct ui_engine *engine,
                                  enum md3_icon_button_variant variant,
                                  const char *icon_name,
                                  struct md3_icon_button **out_button) {
  struct md3_icon_button *btn;
  ui_error_t rc;

  if (!engine || !out_button ||
      (unsigned)variant >= MD3_ICON_BUTTON_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct md3_icon_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_icon_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(struct md3_icon_button));
  btn->variant = variant;

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  if (icon_name) {
    rc = md3_icon_button_set_icon(btn, icon_name);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(btn->base);
      C_MULTIPLATFORM_FREE(btn);
      return rc;
    }
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_button_destroy(struct md3_icon_button *button) {
  ui_error_t rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_button_base_destroy(button->base);
  C_MULTIPLATFORM_FREE(button);
  return rc;
}

ui_error_t md3_icon_button_set_toggleable(struct md3_icon_button *button,
                                          int toggleable,
                                          const char *selected_icon_name) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_toggleable = toggleable;
  if (selected_icon_name) {
#if defined(_MSC_VER)
    strncpy_s(button->selected_icon_name, sizeof(button->selected_icon_name),
              selected_icon_name, sizeof(button->selected_icon_name) - 1);
#else
    strncpy(button->selected_icon_name, selected_icon_name,
            sizeof(button->selected_icon_name) - 1);
    button->selected_icon_name[sizeof(button->selected_icon_name) - 1] = '\0';
#endif
  } else {
    button->selected_icon_name[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_icon_button_set_selected(struct md3_icon_button *button,
                                        int selected) {
  struct ui_component *comp;
  ui_error_t rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_selected = selected ? 1 : 0;

  if (button->is_toggleable && button->selected_icon_name[0] != '\0' &&
      button->is_selected) {
    rc = ui_button_base_set_text(button->base, button->selected_icon_name);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else if (button->icon_name[0] != '\0') {
    rc = ui_button_base_set_text(button->base, button->icon_name);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  rc = ui_button_base_get_component(button->base, &comp);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_dom_node_set_attribute(comp->shadow_root, "aria-pressed",
                                   button->is_selected ? "true" : "false");
}

ui_error_t md3_icon_button_is_selected(const struct md3_icon_button *button,
                                       int *out_selected) {
  if (!button || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_selected = button->is_selected;
  return UI_ERROR_NONE;
}

ui_error_t md3_icon_button_set_disabled(struct md3_icon_button *button,
                                        int disabled) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_button_base_set_disabled(button->base, disabled);
}

ui_error_t md3_icon_button_set_on_click(struct md3_icon_button *button,
                                        ui_button_on_click_t on_click,
                                        void *user_data) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_button_base_set_on_click(button->base, on_click, user_data);
}

ui_error_t md3_icon_button_set_icon(struct md3_icon_button *button,
                                    const char *icon_name) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (icon_name) {
#if defined(_MSC_VER)
    strncpy_s(button->icon_name, sizeof(button->icon_name), icon_name,
              sizeof(button->icon_name) - 1);
#else
    strncpy(button->icon_name, icon_name, sizeof(button->icon_name) - 1);
    button->icon_name[sizeof(button->icon_name) - 1] = '\0';
#endif
    return ui_button_base_set_text(button->base, icon_name);
  } else {
    button->icon_name[0] = '\0';
    return UI_ERROR_NONE;
  }
}

ui_error_t md3_icon_button_get_base(struct md3_icon_button *button,
                                    struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}
