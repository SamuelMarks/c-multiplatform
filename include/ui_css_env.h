#ifndef UI_CSS_ENV_H
#define UI_CSS_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief Represents a dictionary of CSS environment variables.
 */
struct ui_css_env_manager;

/**
 * @brief Creates a new environment variable manager.
 *
 * @param out_manager Pointer to receive the new manager.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_env_manager_create(struct ui_css_env_manager **out_manager);

/**
 * @brief Destroys an environment variable manager.
 *
 * @param manager The manager to destroy.
 */
void ui_css_env_manager_destroy(struct ui_css_env_manager *manager);

/**
 * @brief Sets an environment variable. If the variable exists, its value is
 * updated.
 *
 * @param manager The manager.
 * @param name The name of the environment variable (e.g.,
 * "safe-area-inset-top").
 * @param value The parsed CSS value to store. The manager takes ownership of
 * this value.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_env_manager_set(struct ui_css_env_manager *manager,
                                     const char *name,
                                     struct ui_css_value_ext *value);

/**
 * @brief Retrieves the value of an environment variable.
 *
 * @param manager The manager.
 * @param name The name of the environment variable.
 * @param out_value Pointer to receive a borrowed reference to the value.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if the variable does not
 * exist.
 */
enum ui_error ui_css_env_manager_get(const struct ui_css_env_manager *manager,
                                     const char *name,
                                     const struct ui_css_value_ext **out_value);

/**
 * @brief Removes an environment variable.
 *
 * @param manager The manager.
 * @param name The name of the environment variable to remove.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if it does not exist.
 */
enum ui_error ui_css_env_manager_remove(struct ui_css_env_manager *manager,
                                        const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_ENV_H */
