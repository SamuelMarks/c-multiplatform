/**
 * @file cupertino_apple_pay.c
 * @brief Apple Pay and Sign in with Apple HIG buttons implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_apple_pay.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_apple_pay_mock_base_create_fail = 0;
int g_cupertino_apple_pay_mock_base_set_text_fail = 0;
int g_cupertino_apple_pay_mock_base_destroy_fail = 0;
struct ui_button_base *g_cupertino_apple_pay_last_created_base = NULL;

static ui_error_t mock_button_base_create(struct ui_button_base **out_base) {
  ui_error_t rc;
  if (g_cupertino_apple_pay_mock_base_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  rc = ui_button_base_create(out_base);
  if (rc == UI_ERROR_NONE) {
    g_cupertino_apple_pay_last_created_base = *out_base;
  }
  return rc;
}
#undef ui_button_base_create
/** @cond */
#define ui_button_base_create mock_button_base_create
/** @endcond */

static ui_error_t mock_button_base_set_text(struct ui_button_base *button,
                                            const char *text) {
  if (g_cupertino_apple_pay_mock_base_set_text_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_set_text(button, text);
}
#undef ui_button_base_set_text
/** @cond */
#define ui_button_base_set_text mock_button_base_set_text
/** @endcond */

static ui_error_t mock_button_base_destroy(struct ui_button_base *button) {
  if (g_cupertino_apple_pay_mock_base_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_button_base_destroy)(button);
}
#undef ui_button_base_destroy
/** @cond */
#define ui_button_base_destroy mock_button_base_destroy
/** @endcond */
#endif

static const char *get_apple_pay_label(enum cupertino_apple_pay_type type) {
  switch (type) {
  case CUPERTINO_APPLE_PAY_PLAIN:
    return "Apple Pay";
  case CUPERTINO_APPLE_PAY_BUY:
    return "Buy with Apple Pay";
  case CUPERTINO_APPLE_PAY_SET_UP:
    return "Set up Apple Pay";
  case CUPERTINO_APPLE_PAY_DONATE:
    return "Donate with Apple Pay";
  case CUPERTINO_APPLE_PAY_CHECKOUT:
    return "Check out with Apple Pay";
  case CUPERTINO_APPLE_PAY_SUBSCRIBE:
    return "Subscribe with Apple Pay";
  case CUPERTINO_APPLE_PAY_BOOK:
    return "Book with Apple Pay";
  case CUPERTINO_APPLE_PAY_RELOAD:
    return "Reload with Apple Pay";
  case CUPERTINO_APPLE_PAY_TOP_UP:
    return "Top Up with Apple Pay";
  case CUPERTINO_APPLE_PAY_RENT:
    return "Rent with Apple Pay";
  case CUPERTINO_APPLE_PAY_SUPPORT:
    return "Support with Apple Pay";
  default:
    return "Apple Pay";
  }
}

static const char *
get_apple_id_label(enum cupertino_apple_id_button_type type) {
  switch (type) {
  case CUPERTINO_APPLE_ID_SIGN_IN:
    return "Sign in with Apple";
  case CUPERTINO_APPLE_ID_CONTINUE:
    return "Continue with Apple";
  case CUPERTINO_APPLE_ID_SIGN_UP:
    return "Sign up with Apple";
  default:
    return "Sign in with Apple";
  }
}

#ifdef UI_TEST_MOCK_ALLOC
const char *
test_cupertino_get_apple_pay_label(enum cupertino_apple_pay_type type) {
  return get_apple_pay_label(type);
}

const char *
test_cupertino_get_apple_id_label(enum cupertino_apple_id_button_type type) {
  return get_apple_id_label(type);
}
#endif

static void apply_style_colors(enum cupertino_apple_pay_style style,
                               ui_color_t *out_bg, ui_color_t *out_fg,
                               ui_color_t *out_border) {
  if (style == CUPERTINO_APPLE_PAY_STYLE_WHITE) {
    *out_bg = UI_COLOR_ARGB(255, 255, 255, 255);
    *out_fg = UI_COLOR_ARGB(255, 0, 0, 0);
    *out_border = 0;
  } else if (style == CUPERTINO_APPLE_PAY_STYLE_WHITE_OUTLINE) {
    *out_bg = UI_COLOR_ARGB(255, 255, 255, 255);
    *out_fg = UI_COLOR_ARGB(255, 0, 0, 0);
    *out_border = UI_COLOR_ARGB(255, 0, 0, 0);
  } else {
    *out_bg = UI_COLOR_ARGB(255, 0, 0, 0);
    *out_fg = UI_COLOR_ARGB(255, 255, 255, 255);
    *out_border = 0;
  }
}

ui_error_t
cupertino_apple_pay_create(struct ui_engine *engine,
                           const struct cupertino_apple_pay_descriptor *desc,
                           struct cupertino_apple_pay_button **out_button) {
  struct cupertino_apple_pay_button *btn;
  const char *lbl_text;
  ui_error_t rc;

  if (!engine || !desc || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->type < 0 ||
      (int)desc->type >= (int)CUPERTINO_APPLE_PAY_TYPE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->style < 0 || (int)desc->style > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct cupertino_apple_pay_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_apple_pay_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(*btn));
  btn->type = desc->type;
  btn->style = desc->style;
  btn->width = (desc->width >= CUPERTINO_APPLE_PAY_MIN_WIDTH)
                   ? desc->width
                   : CUPERTINO_APPLE_PAY_MIN_WIDTH;
  btn->height = (desc->height >= CUPERTINO_APPLE_PAY_MIN_HEIGHT)
                    ? desc->height
                    : CUPERTINO_APPLE_PAY_MIN_HEIGHT;
  btn->corner_radius = (desc->corner_radius > 0.0f)
                           ? desc->corner_radius
                           : CUPERTINO_APPLE_PAY_DEFAULT_CORNER_RADIUS;
  btn->scale = 1.0f;
  btn->opacity = 1.0f;
  btn->is_pressed = 0;

  apply_style_colors(btn->style, &btn->background_color, &btn->foreground_color,
                     &btn->border_color);

  lbl_text = get_apple_pay_label(btn->type);
#if defined(_MSC_VER)
  strncpy_s(btn->label, sizeof(btn->label), lbl_text, _TRUNCATE);
#else
  strncpy(btn->label, lbl_text, sizeof(btn->label) - 1);
  btn->label[sizeof(btn->label) - 1] = '\0';
#endif

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  rc = ui_button_base_set_text(btn->base, btn->label);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_button_base_destroy(btn->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_pay_destroy(struct cupertino_apple_pay_button *button) {
  ui_error_t rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->base) {
    rc = ui_button_base_destroy(button->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    button->base = NULL;
  }

  C_MULTIPLATFORM_FREE(button);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_pay_press_start(struct cupertino_apple_pay_button *button) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_pressed = 1;
  button->scale = 0.96f;
  button->opacity = 0.70f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_pay_press_end(struct cupertino_apple_pay_button *button) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_pressed = 0;
  button->scale = 1.0f;
  button->opacity = 1.0f;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_apple_pay_get_dimensions(
    const struct cupertino_apple_pay_button *button, float *out_width,
    float *out_height) {
  if (!button || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = button->width;
  *out_height = button->height;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_pay_get_label(const struct cupertino_apple_pay_button *button,
                              const char **out_label) {
  if (!button || !out_label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_label = button->label;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_pay_get_base(struct cupertino_apple_pay_button *button,
                             struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}

/* --- Sign in with Apple implementation --- */

ui_error_t cupertino_apple_id_button_create(
    struct ui_engine *engine, const struct cupertino_apple_id_descriptor *desc,
    struct cupertino_apple_id_button **out_button) {
  struct cupertino_apple_id_button *btn;
  const char *lbl_text;
  ui_error_t rc;

  if (!engine || !desc || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->type < 0 ||
      (int)desc->type >= (int)CUPERTINO_APPLE_ID_TYPE_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->style < 0 || (int)desc->style > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  btn = (struct cupertino_apple_id_button *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_apple_id_button));
  if (!btn) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(btn, 0, sizeof(*btn));
  btn->type = desc->type;
  btn->style = desc->style;
  btn->width = (desc->width >= CUPERTINO_APPLE_PAY_MIN_WIDTH)
                   ? desc->width
                   : CUPERTINO_APPLE_PAY_MIN_WIDTH;
  btn->height = (desc->height >= CUPERTINO_APPLE_PAY_MIN_HEIGHT)
                    ? desc->height
                    : CUPERTINO_APPLE_PAY_MIN_HEIGHT;
  btn->corner_radius = (desc->corner_radius > 0.0f)
                           ? desc->corner_radius
                           : CUPERTINO_APPLE_PAY_DEFAULT_CORNER_RADIUS;
  btn->scale = 1.0f;
  btn->opacity = 1.0f;
  btn->is_pressed = 0;

  apply_style_colors(btn->style, &btn->background_color, &btn->foreground_color,
                     &btn->border_color);

  lbl_text = get_apple_id_label(btn->type);
#if defined(_MSC_VER)
  strncpy_s(btn->label, sizeof(btn->label), lbl_text, _TRUNCATE);
#else
  strncpy(btn->label, lbl_text, sizeof(btn->label) - 1);
  btn->label[sizeof(btn->label) - 1] = '\0';
#endif

  rc = ui_button_base_create(&btn->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  rc = ui_button_base_set_text(btn->base, btn->label);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_button_base_destroy(btn->base);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(btn);
    return rc;
  }

  *out_button = btn;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_id_button_destroy(struct cupertino_apple_id_button *button) {
  ui_error_t rc;

  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (button->base) {
    rc = ui_button_base_destroy(button->base);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    button->base = NULL;
  }

  C_MULTIPLATFORM_FREE(button);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_apple_id_button_press_start(
    struct cupertino_apple_id_button *button) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_pressed = 1;
  button->scale = 0.96f;
  button->opacity = 0.70f;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_id_button_press_end(struct cupertino_apple_id_button *button) {
  if (!button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  button->is_pressed = 0;
  button->scale = 1.0f;
  button->opacity = 1.0f;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_apple_id_button_get_dimensions(
    const struct cupertino_apple_id_button *button, float *out_width,
    float *out_height) {
  if (!button || !out_width || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_width = button->width;
  *out_height = button->height;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_apple_id_button_get_label(
    const struct cupertino_apple_id_button *button, const char **out_label) {
  if (!button || !out_label) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_label = button->label;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_apple_id_button_get_base(struct cupertino_apple_id_button *button,
                                   struct ui_button_base **out_base) {
  if (!button || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = button->base;
  return UI_ERROR_NONE;
}
