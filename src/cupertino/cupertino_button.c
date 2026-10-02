/**
 * @file cupertino_button.c
 * @brief Cupertino Button component wrapping ui_button_base implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_button_mock_set_text_fail = 0;
int g_cupertino_button_mock_set_disabled_fail = 0;
int g_cupertino_button_mock_destroy_fail = 0;

static ui_error_t mock_button_base_set_text(struct ui_button_base *b,
                                            const char *t) {
  if (g_cupertino_button_mock_set_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_text(b, t);
}
#undef ui_button_base_set_text
/** @cond */
#define ui_button_base_set_text mock_button_base_set_text
/** @endcond */

static ui_error_t mock_button_base_set_disabled(struct ui_button_base *b,
                                                int d) {
  if (g_cupertino_button_mock_set_disabled_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_disabled(b, d);
}
#undef ui_button_base_set_disabled
/** @cond */
#define ui_button_base_set_disabled mock_button_base_set_disabled
/** @endcond */

static ui_error_t mock_button_base_destroy(struct ui_button_base *b) {
  if (g_cupertino_button_mock_destroy_fail) {
    (ui_button_base_destroy)(b);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_button_base_destroy)(b);
}
#undef ui_button_base_destroy
/** @cond */
#define ui_button_base_destroy mock_button_base_destroy
/** @endcond */
#endif

/**
 * @brief Creates a new Cupertino button instance.
 */
ui_error_t
cupertino_button_create(struct ui_engine *engine,
                        const struct cupertino_button_descriptor *desc,
                        struct cupertino_button **out_button) {
  struct cupertino_button *btn;
  ui_error_t rc;

  if (!engine || !desc || !out_button || (int)desc->style < 0 ||
      desc->style >= CUPERTINO_BUTTON_STYLE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct cupertino_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(*btn));
  btn->style = desc->style;
  btn->size = desc->size;
  btn->is_pressed = 0;
  btn->press_scale = 1.0f;
  btn->press_opacity = 1.0f;

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  if (desc->text) {
#if defined(_MSC_VER)
    strncpy_s(btn->label, sizeof(btn->label), desc->text,
              sizeof(btn->label) - 1);
#else
    strncpy(btn->label, desc->text, sizeof(btn->label) - 1);
    btn->label[sizeof(btn->label) - 1] = '\0';
#endif
    rc = ui_button_base_set_text(btn->base, desc->text);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_button_base_destroy(btn->base);
      C_MULTIPLATFORM_FREE(btn);
      if (destroy_rc != UI_ERROR_NONE) {
        return destroy_rc;
      }
      return rc;
    }
  }

  if (desc->is_disabled) {
    rc = ui_button_base_set_disabled(btn->base, 1);
    if (rc != UI_ERROR_NONE) {
      ui_error_t destroy_rc;
      destroy_rc = ui_button_base_destroy(btn->base);
      C_MULTIPLATFORM_FREE(btn);
      if (destroy_rc != UI_ERROR_NONE) {
        return destroy_rc;
      }
      return rc;
    }
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Cupertino button and its underlying base primitive.
 */
ui_error_t cupertino_button_destroy(struct cupertino_button *button) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->base) {
    rc = ui_button_base_destroy(button->base);
    button->base = NULL;
  }

  C_MULTIPLATFORM_FREE(button);
  return rc;
}

/**
 * @brief Updates the visual style of a Cupertino button.
 */
ui_error_t cupertino_button_set_style(struct cupertino_button *button,
                                      enum cupertino_button_style style) {
  if (!button || (int)style < 0 || style >= CUPERTINO_BUTTON_STYLE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->style = style;
  return UI_ERROR_NONE;
}

/**
 * @brief Updates the label text of a Cupertino button.
 */
ui_error_t cupertino_button_set_text(struct cupertino_button *button,
                                     const char *text) {
  ui_error_t rc;

  if (!button || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(button->label, sizeof(button->label), text,
            sizeof(button->label) - 1);
#else
  strncpy(button->label, text, sizeof(button->label) - 1);
  button->label[sizeof(button->label) - 1] = '\0';
#endif

  if (button->base) {
    rc = ui_button_base_set_text(button->base, text);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the interactive pressed state, applying Apple spring scale and
 * opacity fade.
 */
ui_error_t cupertino_button_set_pressed(struct cupertino_button *button,
                                        int is_pressed) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_pressed = is_pressed ? 1 : 0;
  if (button->is_pressed) {
    button->press_scale = 0.96f;
    button->press_opacity = 0.60f;
  } else {
    button->press_scale = 1.0f;
    button->press_opacity = 1.0f;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the wrapped CDK ui_button_base primitive.
 */
ui_error_t cupertino_button_get_base(struct cupertino_button *button,
                                     struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}
