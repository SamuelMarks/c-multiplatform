#ifndef UI_RATING_BASE_H
#define UI_RATING_BASE_H

/**
 * \file ui_rating_base.h
 * \brief UI Rating Base component.
 *
 * This file contains definitions for a rating component
 * allowing selection and display of a fractional rating value (e.g., star
 * rating).
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_icon_base.h"
#include "ui_control_value_accessor.h"
/* clang-format on */

/**
 * @brief Represents a rating component instance.
 */
struct ui_rating_base;

/**
 * @brief Creates a new unstyled rating component.
 *
 * @param out_rating Pointer to receive the allocated rating base.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
ui_error_t ui_rating_base_create(struct ui_rating_base **out_rating,
                                 struct ui_control_value_accessor *out_cva);

/**
 * @brief Destroys a rating component and frees all resources.
 *
 * @param rating The rating component to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_rating_base_destroy(struct ui_rating_base *rating);

/**
 * @brief Sets the maximum rating bound (number of items/stars).
 *
 * @param rating The rating component.
 * @param max_rating The maximum rating (must be > 0).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_set_max(struct ui_rating_base *rating,
                                  unsigned int max_rating);

/**
 * @brief Gets the maximum rating bound.
 *
 * @param rating The rating component.
 * @param out_max Pointer to receive the maximum rating.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_max(const struct ui_rating_base *rating,
                                  unsigned int *out_max);

/**
 * @brief Sets the current fractional rating value.
 *
 * The value will be clamped between 0.0 and max_rating.
 *
 * @param rating The rating component.
 * @param value The rating value.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_set_value(struct ui_rating_base *rating, float value);

/**
 * @brief Gets the current fractional rating value.
 *
 * @param rating The rating component.
 * @param out_value Pointer to receive the rating value.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_value(const struct ui_rating_base *rating,
                                    float *out_value);

/**
 * @brief Gets the fraction to render for a given item index based on the
 * current rating.
 *
 * This function calculates whether the item at the zero-based index should be
 * rendered as full (1.0), empty (0.0), or fractional (between 0.0 and 1.0).
 *
 * @param rating The rating component.
 * @param index The zero-based index of the item (must be < max_rating).
 * @param out_fraction Pointer to receive the fraction.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters or index out of bounds.
 */
ui_error_t ui_rating_base_get_item_fraction(const struct ui_rating_base *rating,
                                            unsigned int index,
                                            float *out_fraction);

/**
 * @brief Retrieves the icon component used to represent a fully filled rating
 * item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * @param rating The rating component.
 * @param out_icon Pointer to receive the icon.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_full_icon(struct ui_rating_base *rating,
                                        struct ui_icon_base **out_icon);

/**
 * @brief Retrieves the icon component used to represent a partially filled
 * (half) rating item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * @param rating The rating component.
 * @param out_icon Pointer to receive the icon.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_half_icon(struct ui_rating_base *rating,
                                        struct ui_icon_base **out_icon);

/**
 * @brief Retrieves the icon component used to represent an empty rating item.
 *
 * The returned icon is owned by the rating component and can be modified
 * directly.
 *
 * @param rating The rating component.
 * @param out_icon Pointer to receive the icon.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on invalid
 * parameters.
 */
ui_error_t ui_rating_base_get_empty_icon(struct ui_rating_base *rating,
                                         struct ui_icon_base **out_icon);

#ifdef __cplusplus
}
#endif

#endif /* UI_RATING_BASE_H */
