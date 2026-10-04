/**
 * @file md2_card.h
 * @brief Material Design 2 Card component wrapping ui_card_base.
 */

#ifndef MATERIAL2_MD2_CARD_H
#define MATERIAL2_MD2_CARD_H

/* clang-format off */
#include "ui_card_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md2_card_variant
 * @brief Material Design 2 Card visual styling variants.
 */
enum md2_card_variant {
  /** @brief Elevated card with 1dp resting elevation and 8dp pressed shadow. */
  MD2_CARD_ELEVATED = 0,
  /** @brief Outlined card with 1dp border outline and 4dp corner radius. */
  MD2_CARD_OUTLINED = 1
};

/**
 * @struct md2_card
 * @brief Material Design 2 Card handle wrapping ui_card_base.
 */
struct md2_card;

/**
 * @brief Creates a Material Design 2 Card component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Elevated or Outlined card styling.
 * @param out_card Pointer receiving newly created card instance.
 * @return UI_ERROR_NONE on success, or appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_card_create(struct ui_engine *engine, enum md2_card_variant variant,
                struct md2_card **out_card);

/**
 * @brief Destroys a Material Design 2 Card and releases resources.
 *
 * @param card Card instance to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_card_destroy(struct md2_card *card);

/**
 * @brief Retrieves underlying ui_card_base handle.
 *
 * @param card Card instance.
 * @param out_base Pointer receiving base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_card_get_base(struct md2_card *card, struct ui_card_base **out_base);

/**
 * @brief Sets the card primary title string.
 *
 * @param card Card instance.
 * @param title Title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_card_set_title(struct md2_card *card, const char *title);

/**
 * @brief Sets the card secondary subtitle string.
 *
 * @param card Card instance.
 * @param subtitle Subtitle string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_card_set_subtitle(struct md2_card *card, const char *subtitle);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_CARD_H */
