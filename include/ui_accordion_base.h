#ifndef UI_ACCORDION_BASE_H
#define UI_ACCORDION_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_disclosure_base.h"
/* clang-format on */

struct ui_accordion_base;

/**
 * @brief Callback invoked when the active disclosure in the accordion changes.
 *
 * @param accordion The accordion component.
 * @param active_disclosure The newly activated disclosure, or NULL if all are
 * collapsed.
 * @param user_data Opaque user data.
 */
typedef enum ui_error (*ui_accordion_on_change_t)(
    struct ui_accordion_base *accordion,
    struct ui_disclosure_base *active_disclosure, void *user_data);

/**
 * @brief Creates a new accordion base manager.
 *
 * @param out_accordion Pointer to receive the allocated accordion.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_create(struct ui_accordion_base **out_accordion);

/**
 * @brief Destroys an accordion base manager.
 * Note: This does not destroy the individual ui_disclosure_base components.
 *
 * @param accordion The accordion to destroy.
 */
void ui_accordion_base_destroy(struct ui_accordion_base *accordion);

/**
 * @brief Adds a disclosure to the accordion group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure component to add.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_add_disclosure(struct ui_accordion_base *accordion,
                                 struct ui_disclosure_base *disclosure);

/**
 * @brief Removes a disclosure from the accordion group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure component to remove.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_remove_disclosure(struct ui_accordion_base *accordion,
                                    struct ui_disclosure_base *disclosure);

/**
 * @brief Sets the currently active (expanded) disclosure in the group.
 * Unexpands all other disclosures in this group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure to set as active. If NULL, collapses all.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_set_active(struct ui_accordion_base *accordion,
                             struct ui_disclosure_base *disclosure);

/**
 * @brief Retrieves the currently active (expanded) disclosure in the group.
 *
 * @param accordion The accordion manager.
 * @return The active disclosure, or NULL if none are expanded.
 */
enum ui_error
ui_accordion_base_get_active(const struct ui_accordion_base *accordion,
                             struct ui_disclosure_base **out_active);

/**
 * @brief Sets the callback for when the active disclosure changes.
 *
 * @param accordion The accordion manager.
 * @param on_change The callback to invoke.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_set_on_change(struct ui_accordion_base *accordion,
                                ui_accordion_on_change_t on_change,
                                void *user_data);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_accordion_base_bind_data(struct ui_accordion_base *widget,
                                          struct ui_computed *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_ACCORDION_BASE_H */
