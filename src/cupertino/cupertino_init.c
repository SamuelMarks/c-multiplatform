/**
 * @file cupertino_init.c
 * @brief Implementation of Cupertino lifecycle and design system vtable.
 */

/* clang-format off */
#include "cupertino/cupertino_init.h"
#include "cupertino/cupertino_tokens.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_init_mock_register_fail = 0;
int g_cupertino_init_mock_unregister_fail = 0;
int g_cupertino_init_mock_get_color_fail = 0;

static ui_error_t
mock_ui_design_system_register(const char *name,
                               const struct ui_design_system_vtable *vtable) {
  if (g_cupertino_init_mock_register_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_design_system_register(name, vtable);
}
#undef ui_design_system_register
/** @cond */
#define ui_design_system_register mock_ui_design_system_register
/** @endcond */

static ui_error_t mock_ui_design_system_unregister(const char *name) {
  if (g_cupertino_init_mock_unregister_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return ui_design_system_unregister(name);
}
#undef ui_design_system_unregister
/** @cond */
#define ui_design_system_unregister mock_ui_design_system_unregister
/** @endcond */

static ui_error_t
mock_cupertino_get_system_color(enum cupertino_system_color color, int is_dark,
                                int high_contrast, ui_color_t *out_color) {
  if (g_cupertino_init_mock_get_color_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_get_system_color(color, is_dark, high_contrast, out_color);
}
#undef cupertino_get_system_color
/** @cond */
#define cupertino_get_system_color mock_cupertino_get_system_color
/** @endcond */
#endif

static int g_cupertino_initialized = 0;
static enum cupertino_theme_mode g_current_theme = CUPERTINO_THEME_IOS_LIGHT;

/**
 * @brief Vtable init implementation for Cupertino.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t cupertino_vtable_init(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Vtable shutdown implementation for Cupertino.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t cupertino_vtable_shutdown(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Vtable apply_theme implementation for Cupertino.
 *
 * @param engine Pointer to the UI engine instance.
 * @param theme_name Name of the theme to apply ("ios_light", "ios_dark",
 * "macos_light", "macos_dark", "visionos").
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
static ui_error_t cupertino_vtable_apply_theme(struct ui_engine *engine,
                                               const char *theme_name) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!theme_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(theme_name, "ios_light") == 0) {
    g_current_theme = CUPERTINO_THEME_IOS_LIGHT;
  } else if (strcmp(theme_name, "ios_dark") == 0) {
    g_current_theme = CUPERTINO_THEME_IOS_DARK;
  } else if (strcmp(theme_name, "macos_light") == 0) {
    g_current_theme = CUPERTINO_THEME_MACOS_LIGHT;
  } else if (strcmp(theme_name, "macos_dark") == 0) {
    g_current_theme = CUPERTINO_THEME_MACOS_DARK;
  } else if (strcmp(theme_name, "visionos") == 0) {
    g_current_theme = CUPERTINO_THEME_VISIONOS;
  } else {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Vtable resolve_token implementation for Cupertino.
 *
 * @param token_name The design token key to look up.
 * @param out_token Pointer to store the resolved token copy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
static ui_error_t
cupertino_vtable_resolve_token(const char *token_name,
                               struct ui_design_token *out_token) {
  int is_dark;
  ui_color_t col = 0;
  ui_error_t rc;

  if (!token_name || !out_token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  is_dark = (g_current_theme == CUPERTINO_THEME_IOS_DARK ||
             g_current_theme == CUPERTINO_THEME_MACOS_DARK ||
             g_current_theme == CUPERTINO_THEME_VISIONOS);

  rc = cupertino_resolve_named_color(token_name, is_dark, &col);
  if (rc == UI_ERROR_NONE) {
    out_token->type = UI_TOKEN_TYPE_COLOR;
    out_token->value.color_val = col;
    return UI_ERROR_NONE;
  }

  if (strcmp(token_name, "--apple-system-blue") == 0) {
    rc = cupertino_get_system_color(CUPERTINO_COLOR_BLUE, is_dark, 0, &col);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    out_token->type = UI_TOKEN_TYPE_COLOR;
    out_token->value.color_val = col;
    return UI_ERROR_NONE;
  }
  if (strcmp(token_name, "--apple-system-green") == 0) {
    rc = cupertino_get_system_color(CUPERTINO_COLOR_GREEN, is_dark, 0, &col);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    out_token->type = UI_TOKEN_TYPE_COLOR;
    out_token->value.color_val = col;
    return UI_ERROR_NONE;
  }
  if (strcmp(token_name, "--apple-system-red") == 0) {
    rc = cupertino_get_system_color(CUPERTINO_COLOR_RED, is_dark, 0, &col);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    out_token->type = UI_TOKEN_TYPE_COLOR;
    out_token->value.color_val = col;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NOT_FOUND;
}

static const struct ui_design_system_vtable g_cupertino_vtable = {
    cupertino_vtable_init, cupertino_vtable_shutdown,
    cupertino_vtable_apply_theme, cupertino_vtable_resolve_token};

/**
 * @brief Retrieves the Cupertino baseline design system operations vtable.
 */
ui_error_t
cupertino_get_vtable(const struct ui_design_system_vtable **out_vtable) {
  if (!out_vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_vtable = &g_cupertino_vtable;
  return UI_ERROR_NONE;
}

/**
 * @brief Initializes the Cupertino design system module and registers it.
 */
ui_error_t cupertino_initialize(struct ui_engine *engine) {
  ui_error_t rc;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_design_system_register("cupertino", &g_cupertino_vtable);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  g_cupertino_initialized = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Shuts down the Cupertino design system module and frees resources.
 */
ui_error_t cupertino_shutdown(void) {
  ui_error_t rc;

  if (!g_cupertino_initialized) {
    return UI_ERROR_NONE;
  }

  rc = ui_design_system_unregister("cupertino");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  g_cupertino_initialized = 0;
  return UI_ERROR_NONE;
}
