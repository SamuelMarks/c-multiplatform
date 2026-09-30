/**
 * @file md3_card.h
 * @brief Material 3 Elevated, Filled, and Outlined Card components wrapping
 * ui_card_base.
 */

#ifndef MATERIAL3_MD3_CARD_H
#define MATERIAL3_MD3_CARD_H

/* clang-format off */
#include "ui_card_base.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @enum md3_card_variant
 * @brief Visual styling variants for Material 3 Cards.
 */
enum md3_card_variant {
  MD3_CARD_ELEVATED = 0,
  MD3_CARD_FILLED,
  MD3_CARD_OUTLINED,
  MD3_CARD_VARIANT_COUNT
};

/**
 * @struct md3_card
 * @brief Material 3 Card skin wrapping ui_card_base.
 */
struct md3_card {
  struct ui_card_base *base;
  enum md3_card_variant variant;
  char title[64];
  char subtitle[64];
};

/**
 * @brief Creates a Material 3 Card component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param variant Visual variant (Elevated, Filled, or Outlined).
 * @param out_card Pointer to receive newly created card.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_create(struct ui_engine *engine, enum md3_card_variant variant,
                struct md3_card **out_card);

/**
 * @brief Destroys a Material 3 Card.
 *
 * @param card The card to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_destroy(struct md3_card *card);

/**
 * @brief Sets the card title text.
 *
 * @param card The card.
 * @param title The title text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_set_title(struct md3_card *card, const char *title);

/**
 * @brief Sets the card subtitle text.
 *
 * @param card The card.
 * @param subtitle The subtitle text string.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_set_subtitle(struct md3_card *card, const char *subtitle);

/**
 * @brief Sets the header content component.
 *
 * @param card The card.
 * @param header Header component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_set_header(struct md3_card *card, struct ui_component *header);

/**
 * @brief Sets the main body content component.
 *
 * @param card The card.
 * @param content Content component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_set_content(struct md3_card *card, struct ui_component *content);

/**
 * @brief Sets the action area component.
 *
 * @param card The card.
 * @param actions Actions component.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_set_actions(struct md3_card *card, struct ui_component *actions);

/**
 * @brief Retrieves underlying ui_card_base handle.
 *
 * @param card The card.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_card_get_base(struct md3_card *card, struct ui_card_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_CARD_H */
