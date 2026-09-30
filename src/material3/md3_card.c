/**
 * @file md3_card.c
 * @brief Material 3 Card component implementation wrapping ui_card_base.
 */

/* clang-format off */
#include "material3/md3_card.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_card_create(struct ui_engine *engine,
                           enum md3_card_variant variant,
                           struct md3_card **out_card) {
  struct md3_card *card;
  ui_error_t rc;

  if (!engine || !out_card || (unsigned)variant >= MD3_CARD_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  card = (struct md3_card *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_card));
  if (!card) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(card, 0, sizeof(struct md3_card));
  card->variant = variant;

  rc = ui_card_base_create(&card->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(card);
    return rc;
  }

  *out_card = card;
  return UI_ERROR_NONE;
}

ui_error_t md3_card_destroy(struct md3_card *card) {
  ui_error_t rc;

  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_card_base_destroy(card->base);
  C_MULTIPLATFORM_FREE(card);
  return rc;
}

ui_error_t md3_card_set_title(struct md3_card *card, const char *title) {
  ui_error_t rc;

  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (title) {
#if defined(_MSC_VER)
    strncpy_s(card->title, sizeof(card->title), title, sizeof(card->title) - 1);
#else
    strncpy(card->title, title, sizeof(card->title) - 1);
    card->title[sizeof(card->title) - 1] = '\0';
#endif
  } else {
    card->title[0] = '\0';
  }

  rc = ui_card_base_set_title(card->base, title);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_card_set_subtitle(struct md3_card *card, const char *subtitle) {
  ui_error_t rc;

  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (subtitle) {
#if defined(_MSC_VER)
    strncpy_s(card->subtitle, sizeof(card->subtitle), subtitle,
              sizeof(card->subtitle) - 1);
#else
    strncpy(card->subtitle, subtitle, sizeof(card->subtitle) - 1);
    card->subtitle[sizeof(card->subtitle) - 1] = '\0';
#endif
  } else {
    card->subtitle[0] = '\0';
  }

  rc = ui_card_base_set_subtitle(card->base, subtitle);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_card_set_header(struct md3_card *card,
                               struct ui_component *header) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_card_base_set_header(card->base, header);
}

ui_error_t md3_card_set_content(struct md3_card *card,
                                struct ui_component *content) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_card_base_set_content(card->base, content);
}

ui_error_t md3_card_set_actions(struct md3_card *card,
                                struct ui_component *actions) {
  if (!card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_card_base_set_actions(card->base, actions);
}

ui_error_t md3_card_get_base(struct md3_card *card,
                             struct ui_card_base **out_base) {
  if (!card || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = card->base;
  return UI_ERROR_NONE;
}
