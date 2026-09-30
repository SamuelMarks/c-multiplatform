/**
 * @file md3_divider.h
 * @brief Material 3 Horizontal and Vertical Dividers wrapping ui_divider_base.
 */

#ifndef MATERIAL3_MD3_DIVIDER_H
#define MATERIAL3_MD3_DIVIDER_H

/* clang-format off */
#include "ui_divider_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @struct md3_divider
 * @brief Material 3 Divider skin wrapping ui_divider_base.
 */
struct md3_divider {
  struct ui_divider_base *base;
  enum ui_divider_orientation orientation;
  int is_inset;
};

/**
 * @brief Creates a Material 3 Divider component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param orientation Horizontal or Vertical divider line.
 * @param is_inset 1 for inset divider with leading margin, 0 for full-bleed.
 * @param out_divider Pointer to receive newly created divider.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_divider_create(
    struct ui_engine *engine, enum ui_divider_orientation orientation,
    int is_inset, struct md3_divider **out_divider);

/**
 * @brief Destroys a Material 3 Divider.
 *
 * @param divider The divider to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_divider_destroy(struct md3_divider *divider);

/**
 * @brief Sets divider orientation.
 *
 * @param divider The divider.
 * @param orientation Horizontal or Vertical.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_divider_set_orientation(
    struct md3_divider *divider, enum ui_divider_orientation orientation);

/**
 * @brief Sets inset margin flag.
 *
 * @param divider The divider.
 * @param is_inset 1 for inset, 0 for full-bleed.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_divider_set_inset(struct md3_divider *divider, int is_inset);

/**
 * @brief Retrieves underlying ui_divider_base handle.
 *
 * @param divider The divider.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_divider_get_base(
    struct md3_divider *divider, struct ui_divider_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_DIVIDER_H */
