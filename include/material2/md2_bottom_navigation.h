/**
 * @file md2_bottom_navigation.h
 * @brief Material Design 2 Bottom Navigation component wrapping
 * ui_bottom_nav_base.
 */

#ifndef MATERIAL2_MD2_BOTTOM_NAVIGATION_H
#define MATERIAL2_MD2_BOTTOM_NAVIGATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_bottom_nav_base.h"
#include <stddef.h>
/* clang-format on */

struct ui_engine;
struct md2_bottom_navigation;
struct md2_bottom_navigation_item;

/**
 * @brief Material 2 Bottom Navigation visual variants.
 */
enum md2_bottom_navigation_variant {
  MD2_BOTTOM_NAVIGATION_FIXED,   /**< Fixed bottom nav (3-5 items). */
  MD2_BOTTOM_NAVIGATION_SHIFTING /**< Shifting bottom nav (4-5 items). */
};

/**
 * @brief Creates a Material 2 Bottom Navigation component.
 *
 * @param engine Pointer to ui_engine.
 * @param variant Fixed or Shifting.
 * @param out_nav Pointer to receive newly created Bottom Navigation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_bottom_navigation_create(
    struct ui_engine *engine, enum md2_bottom_navigation_variant variant,
    struct md2_bottom_navigation **out_nav);

/**
 * @brief Destroys a Material 2 Bottom Navigation component.
 *
 * @param nav Bottom Navigation to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_bottom_navigation_destroy(struct md2_bottom_navigation *nav);

/**
 * @brief Retrieves the underlying ui_bottom_nav_base handle.
 *
 * @param nav The Bottom Navigation.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_bottom_navigation_get_base(
    struct md2_bottom_navigation *nav, struct ui_bottom_nav_base **out_base);

/**
 * @brief Appends an item to the Bottom Navigation.
 *
 * @param nav The Bottom Navigation.
 * @param item The Bottom Navigation item to append.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_bottom_navigation_append_item(
    struct md2_bottom_navigation *nav, struct md2_bottom_navigation_item *item);

/**
 * @brief Creates a Material 2 Bottom Navigation item component.
 *
 * @param engine Pointer to ui_engine.
 * @param label Text label for the item.
 * @param icon Icon string or id for the item.
 * @param out_item Pointer to receive newly created Bottom Navigation item.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_bottom_navigation_item_create(
    struct ui_engine *engine, const char *label, const char *icon,
    struct md2_bottom_navigation_item **out_item);

/**
 * @brief Destroys a Material 2 Bottom Navigation item component.
 *
 * @param item Bottom Navigation item to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_bottom_navigation_item_destroy(struct md2_bottom_navigation_item *item);

/**
 * @brief Retrieves the underlying ui_bottom_nav_item_base handle.
 *
 * @param item The Bottom Navigation item.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_bottom_navigation_item_get_base(struct md2_bottom_navigation_item *item,
                                    struct ui_bottom_nav_item_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL2_MD2_BOTTOM_NAVIGATION_H */
