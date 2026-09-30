/**
 * @file ui_design_system.h
 * @brief Pluggable design system registry for registering, switching, and
 * managing design systems (Material 3, Material 3 Expressive, Fluent,
 * Cupertino).
 */

#ifndef UI_DESIGN_SYSTEM_H
#define UI_DESIGN_SYSTEM_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_design_tokens.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct ui_design_system_vtable
 * @brief Lifecycle and resolution operations for a pluggable design system.
 */
struct ui_design_system_vtable {
  /**
   * @brief Initializes the design system with the UI engine.
   * @param engine Pointer to the UI engine instance.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*init)(struct ui_engine *engine);

  /**
   * @brief Shuts down the design system and frees associated resources.
   * @param engine Pointer to the UI engine instance.
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*shutdown)(struct ui_engine *engine);

  /**
   * @brief Applies a named theme or color scheme to the UI engine.
   * @param engine Pointer to the UI engine instance.
   * @param theme_name Name of the theme to apply (e.g. "light", "dark").
   * @return UI_ERROR_NONE on success, or an appropriate error code.
   */
  ui_error_t (*apply_theme)(struct ui_engine *engine, const char *theme_name);

  /**
   * @brief Resolves a design token by name from the design system.
   * @param token_name The design token key to look up.
   * @param out_token Pointer to store the resolved token copy.
   * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if absent.
   */
  ui_error_t (*resolve_token)(const char *token_name,
                              struct ui_design_token *out_token);
};

/**
 * @brief Registers a design system implementation with the global registry.
 *
 * @param name Unique name identifier for the design system (e.g. "material3").
 * @param vtable Pointer to the design system operations vtable.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_design_system_register(
    const char *name, const struct ui_design_system_vtable *vtable);

/**
 * @brief Unregisters a design system by name.
 *
 * @param name Unique name identifier of the design system to unregister.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not registered.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_design_system_unregister(const char *name);

/**
 * @brief Retrieves the registered vtable for a design system.
 *
 * @param name The name of the design system to query.
 * @param out_vtable Pointer to receive the vtable pointer.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if not registered.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_design_system_get(
    const char *name, const struct ui_design_system_vtable **out_vtable);

/**
 * @brief Activates a registered design system on the given engine instance.
 *
 * @param engine Pointer to the UI engine instance.
 * @param name The name of the registered design system to activate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_design_system_set_active(struct ui_engine *engine, const char *name);

/**
 * @brief Retrieves the name of the currently active design system.
 *
 * @param out_name Pointer to receive the constant string name of the active
 * system.
 * @return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if none is active.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
ui_design_system_get_active(const char **out_name);

/**
 * @brief Resets and clears all registered design systems (useful for test
 * teardown).
 *
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t ui_design_system_registry_reset(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_DESIGN_SYSTEM_H */
