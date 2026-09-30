/**
 * @file md3_chip.h
 * @brief Material 3 Chip components (Assist, Filter, Input, Suggestion)
 * wrapping ui_chips_base.
 */

#ifndef MATERIAL3_MD3_CHIP_H
#define MATERIAL3_MD3_CHIP_H

/* clang-format off */
#include "ui_chips_base.h"
#include "ui_error.h"
#include "ui_types.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_chip_type
 * @brief Material 3 Chip functional types.
 */
enum md3_chip_type {
  MD3_CHIP_ASSIST = 0, /**< Smart assist actions */
  MD3_CHIP_FILTER,     /**< Selection/toggle filters */
  MD3_CHIP_INPUT,      /**< Entered discrete entities with removal */
  MD3_CHIP_SUGGESTION, /**< Dynamically generated choices */
  MD3_CHIP_TYPE_COUNT
};

/**
 * @struct md3_chip
 * @brief Material 3 Chip collection manager wrapping ui_chips_base.
 */
struct md3_chip {
  struct ui_chips_base *base;
  enum md3_chip_type type;
  int elevated;
};

/**
 * @brief Creates a Material 3 Chip collection manager.
 *
 * @param engine Pointer to ui_engine instance.
 * @param type Chip functional type (Assist, Filter, Input, Suggestion).
 * @param out_chip Pointer to receive newly created chip manager.
 * @param out_cva Optional pointer to receive CVA for form binding.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chip_create(
    struct ui_engine *engine, enum md3_chip_type type,
    struct md3_chip **out_chip, struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a Material 3 Chip collection manager.
 *
 * @param chip The chip manager to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_chip_destroy(struct md3_chip *chip);

/**
 * @brief Adds a chip token string to the collection.
 *
 * @param chip The chip manager.
 * @param label The chip label string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chip_add(struct md3_chip *chip,
                                                      const char *label);

/**
 * @brief Removes a chip token at the specified index.
 *
 * @param chip The chip manager.
 * @param index Index of the token to remove.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chip_remove(struct md3_chip *chip,
                                                         size_t index);

/**
 * @brief Gets the number of chips in the collection.
 *
 * @param chip The chip manager.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_chip_get_count(const struct md3_chip *chip, size_t *out_count);

/**
 * @brief Gets the label of a chip at the specified index.
 *
 * @param chip The chip manager.
 * @param index Index of the chip.
 * @param out_label Pointer to receive label string pointer.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_chip_get_label(
    const struct md3_chip *chip, size_t index, const char **out_label);

/**
 * @brief Sets elevated visual styling for the chips.
 *
 * @param chip The chip manager.
 * @param elevated Non-zero for elevated style, 0 for flat/outlined.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_chip_set_elevated(struct md3_chip *chip, int elevated);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL3_MD3_CHIP_H */
