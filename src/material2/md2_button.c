/**
 * @file md2_button.c
 * @brief Implementation of Material Design 2 Button component.
 */

/* clang-format off */
#include "material2/md2_button.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include <string.h>
#include <ctype.h>
/* clang-format on */

struct md2_button {
  struct ui_component *component;
  struct ui_button_base *base;
  enum md2_button_variant variant;
};

static void md2_to_uppercase(char *str) {
  while (*str) {
    *str = (char)toupper((unsigned char)*str);
    str++;
  }
}

ui_error_t md2_button_create(struct ui_engine *engine,
                             enum md2_button_variant variant, const char *text,
                             struct md2_button **out_button) {
  struct md2_button *btn;
  ui_error_t rc;
  char *upper_text = NULL;

  if (!engine || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct md2_button *)C_MULTIPLATFORM_MALLOC(sizeof(struct md2_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(btn, 0, sizeof(*btn));

  btn->variant = variant;

  rc = ui_component_create(&btn->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(btn->component);
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  if (text) {
    size_t len = strlen(text);
    upper_text = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
    if (!upper_text) {
      ui_button_base_destroy(btn->base);
      ui_component_destroy(btn->component);
      C_MULTIPLATFORM_FREE(btn);
      return UI_ERROR_OUT_OF_MEMORY;
    }
#if defined(_MSC_VER)
    strcpy_s(upper_text, len + 1, text);
#else
    strcpy(upper_text, text);
#endif
    md2_to_uppercase(upper_text);

    rc = ui_button_base_set_text(btn->base, upper_text);
    C_MULTIPLATFORM_FREE(upper_text);

    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(btn->base);
      ui_component_destroy(btn->component);
      C_MULTIPLATFORM_FREE(btn);
      return rc;
    }
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

ui_error_t md2_button_destroy(struct md2_button *button) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  ui_button_base_destroy(button->base);
  ui_component_destroy(button->component);

  C_MULTIPLATFORM_FREE(button);
  return UI_ERROR_NONE;
}

ui_error_t md2_button_set_text(struct md2_button *button, const char *text) {
  char *upper_text;
  size_t len;
  ui_error_t rc;

  if (!button || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  len = strlen(text);
  upper_text = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!upper_text) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(upper_text, len + 1, text);
#else
  strcpy(upper_text, text);
#endif

  md2_to_uppercase(upper_text);
  rc = ui_button_base_set_text(button->base, upper_text);
  C_MULTIPLATFORM_FREE(upper_text);

  return rc;
}

ui_error_t md2_button_set_on_click(struct md2_button *button,
                                   ui_button_on_click_t on_click,
                                   void *user_data) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_button_base_set_on_click(button->base, on_click, user_data);
}

ui_error_t md2_button_get_base(struct md2_button *button,
                               struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}
