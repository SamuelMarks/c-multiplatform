/**
 * @file md3_init.c
 * @brief Implementation of Material 3 and Material 3 Expressive lifecycle.
 */

/* clang-format off */
#include "material3/md3_init.h"
#include <string.h>
/* clang-format on */

static int g_md3_initialized = 0;

/**
 * @brief Vtable init implementation for Material 3.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t md3_vtable_init(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Vtable shutdown implementation for Material 3.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t md3_vtable_shutdown(struct ui_engine *engine) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Vtable apply_theme implementation for Material 3.
 *
 * @param engine Pointer to the UI engine instance.
 * @param theme_name Name of the theme to apply.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t md3_vtable_apply_theme(struct ui_engine *engine,
                                         const char *theme_name) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!theme_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief Vtable resolve_token implementation for Material 3.
 *
 * @param token_name The design token key to look up.
 * @param out_token Pointer to store the resolved token copy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND.
 */
static ui_error_t md3_vtable_resolve_token(const char *token_name,
                                           struct ui_design_token *out_token) {
  if (!token_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (!out_token) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NOT_FOUND;
}

static const struct ui_design_system_vtable g_md3_vtable = {
    md3_vtable_init, md3_vtable_shutdown, md3_vtable_apply_theme,
    md3_vtable_resolve_token};

static const struct ui_design_system_vtable g_md3_expressive_vtable = {
    md3_vtable_init, md3_vtable_shutdown, md3_vtable_apply_theme,
    md3_vtable_resolve_token};

/**
 * @brief Retrieves the Material 3 baseline design system operations vtable.
 *
 * @param out_vtable Pointer to receive the vtable pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if out_vtable
 * is NULL.
 */
ui_error_t md3_get_vtable(const struct ui_design_system_vtable **out_vtable) {
  if (!out_vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_vtable = &g_md3_vtable;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves the Material 3 Expressive design system operations vtable.
 *
 * @param out_vtable Pointer to receive the vtable pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if out_vtable
 * is NULL.
 */
ui_error_t
md3_expressive_get_vtable(const struct ui_design_system_vtable **out_vtable) {
  if (!out_vtable) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_vtable = &g_md3_expressive_vtable;
  return UI_ERROR_NONE;
}

/**
 * @brief Initializes the Material 3 design system module and registers it.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_initialize(struct ui_engine *engine) {
  ui_error_t rc;

  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_design_system_register("material3", &g_md3_vtable);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_design_system_register("material3-expressive",
                                 &g_md3_expressive_vtable);
  if (rc != UI_ERROR_NONE) {
    ui_design_system_unregister("material3");
    return rc;
  }

  g_md3_initialized = 1;
  return UI_ERROR_NONE;
}

/**
 * @brief Shuts down the Material 3 design system module and frees resources.
 *
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_shutdown(void) {
  ui_error_t rc;
  ui_error_t first_err = UI_ERROR_NONE;

  if (!g_md3_initialized) {
    return UI_ERROR_NONE;
  }

  rc = ui_design_system_unregister("material3-expressive");
  if (rc != UI_ERROR_NONE) {
    first_err = rc;
  }

  rc = ui_design_system_unregister("material3");
  if (rc != UI_ERROR_NONE && first_err == UI_ERROR_NONE) {
    first_err = rc;
  }

  g_md3_initialized = 0;
  return first_err;
}
