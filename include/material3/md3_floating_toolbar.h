/**
 * @file md3_floating_toolbar.h
 * @brief Material 3 Floating Toolbar component wrapping
 * ui_floating_toolbar_base.
 */

#ifndef MATERIAL3_MD3_FLOATING_TOOLBAR_H
#define MATERIAL3_MD3_FLOATING_TOOLBAR_H

/* clang-format off */
#include "material3/md3_fab.h"
#include "ui_floating_toolbar_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_floating_toolbar_type
 * @brief Material 3 Floating Toolbar visual and interaction type.
 */
enum md3_floating_toolbar_type {
  /** @brief Standard persistent floating toolbar. */
  MD3_FLOATING_TOOLBAR_STANDARD = 0,
  /** @brief Expandable floating toolbar that springs open on tap. */
  MD3_FLOATING_TOOLBAR_EXPANDABLE = 1
};

/**
 * @struct md3_floating_toolbar
 * @brief Material 3 Floating Toolbar skin wrapping ui_floating_toolbar_base.
 */
struct md3_floating_toolbar;

/**
 * @brief Creates a Material 3 Floating Toolbar component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param orientation Horizontal or vertical toolbar layout.
 * @param type Standard or expandable toolbar behavior.
 * @param out_toolbar Pointer receiving newly created toolbar instance.
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_floating_toolbar_create(
    struct ui_engine *engine, enum ui_floating_toolbar_orientation orientation,
    enum md3_floating_toolbar_type type,
    struct md3_floating_toolbar **out_toolbar);

/**
 * @brief Destroys a Material 3 Floating Toolbar and releases all resources.
 *
 * @param toolbar Floating toolbar instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_floating_toolbar_destroy(struct md3_floating_toolbar *toolbar);

/**
 * @brief Retrieves underlying ui_floating_toolbar_base handle.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_base Pointer receiving underlying base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_floating_toolbar_get_base(struct md3_floating_toolbar *toolbar,
                              struct ui_floating_toolbar_base **out_base);

/**
 * @brief Mounts an embedded Material 3 FAB into the toolbar's primary anchor
 * slot.
 *
 * @param toolbar Floating toolbar instance.
 * @param fab Material 3 FAB instance to embed (or NULL to remove).
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_floating_toolbar_set_fab(
    struct md3_floating_toolbar *toolbar, struct md3_fab *fab);

/**
 * @brief Toggles expansion state of an expandable floating toolbar.
 *
 * @param toolbar Floating toolbar instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_floating_toolbar_toggle_expansion(struct md3_floating_toolbar *toolbar);

/**
 * @brief Queries whether the floating toolbar is currently in expanded state.
 *
 * @param toolbar Floating toolbar instance.
 * @param out_expanded Pointer receiving 1 if expanded, 0 if collapsed.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_floating_toolbar_is_expanded(
    const struct md3_floating_toolbar *toolbar, int *out_expanded);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_FLOATING_TOOLBAR_H */
