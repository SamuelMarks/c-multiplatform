/**
 * @file md3_fab.h
 * @brief Material 3 Floating Action Button (FAB) component wrapping
 * ui_fab_base.
 */

#ifndef MATERIAL3_MD3_FAB_H
#define MATERIAL3_MD3_FAB_H

/* clang-format off */
#include "ui_fab_base.h"
#include "ui_button_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_fab_size
 * @brief Material 3 FAB sizing options.
 */
enum md3_fab_size {
  MD3_FAB_SIZE_SMALL = 0, /**< 40x40dp */
  MD3_FAB_SIZE_REGULAR,   /**< 56x56dp */
  MD3_FAB_SIZE_LARGE,     /**< 96x96dp */
  MD3_FAB_SIZE_EXTENDED,  /**< Extended FAB with icon and text label */
  MD3_FAB_SIZE_COUNT
};

/**
 * @enum md3_fab_variant
 * @brief Material 3 FAB color/surface styling variants.
 */
enum md3_fab_variant {
  MD3_FAB_SURFACE = 0,
  MD3_FAB_PRIMARY,
  MD3_FAB_SECONDARY,
  MD3_FAB_TERTIARY,
  MD3_FAB_VARIANT_COUNT
};

/**
 * @struct md3_fab
 * @brief Material 3 Floating Action Button skin wrapping ui_fab_base.
 */
struct md3_fab {
  struct ui_fab_base *base;
  enum md3_fab_size size;
  enum md3_fab_variant variant;
  unsigned int elevation;
  char icon_name[32];
  char label[64];
};

/**
 * @brief Creates a Material 3 FAB component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param size Sizing variant (Small, Regular, Large, Extended).
 * @param variant Color styling variant (Surface, Primary, Secondary, Tertiary).
 * @param icon Icon identifier (optional).
 * @param label Text label for Extended FAB (optional).
 * @param out_fab Pointer to receive newly created FAB.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_create(struct ui_engine *engine, enum md3_fab_size size,
               enum md3_fab_variant variant, const char *icon,
               const char *label, struct md3_fab **out_fab);

/**
 * @brief Destroys a Material 3 FAB and its underlying base.
 *
 * @param fab The FAB to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_destroy(struct md3_fab *fab);

/**
 * @brief Sets elevation level for the FAB (defaults to Level 3 resting).
 *
 * @param fab The FAB.
 * @param level Elevation level (0 through 5).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_set_elevation(struct md3_fab *fab, unsigned int level);

/**
 * @brief Adds a speed-dial secondary action to the FAB.
 *
 * @param fab The FAB.
 * @param icon Action icon identifier.
 * @param label Action label.
 * @param on_click Action click callback.
 * @param user_data User data for callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_add_speed_dial_action(
    struct md3_fab *fab, const char *icon, const char *label,
    ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Toggles speed-dial expansion state.
 *
 * @param fab The FAB.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_toggle(struct md3_fab *fab);

/**
 * @brief Advances the FAB speed-dial animation and ripple state.
 *
 * @param fab The FAB.
 * @param dt_ms Delta time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_tick(struct md3_fab *fab,
                                                      float dt_ms);

/**
 * @brief Queries current speed-dial state.
 *
 * @param fab The FAB.
 * @param out_state Pointer to receive speed-dial state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_get_speed_dial_state(
    const struct md3_fab *fab, enum ui_fab_state *out_state);

/**
 * @brief Sets click handler on the primary FAB button.
 *
 * @param fab The FAB.
 * @param on_click Callback function.
 * @param user_data User data pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_fab_set_on_click(
    struct md3_fab *fab, ui_button_on_click_t on_click, void *user_data);

/**
 * @brief Retrieves the underlying ui_fab_base handle.
 *
 * @param fab The FAB.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_fab_get_base(struct md3_fab *fab, struct ui_fab_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_FAB_H */
