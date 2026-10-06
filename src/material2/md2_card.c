/**
 * @file md2_card.c
 * @brief Implementation of Material Design 2 Card component wrapping
 * ui_card_base.
 */

/* clang-format off */
#include "material2/md2_card.h"
#include "ui_component.h"
#include "ui_dom_node.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct md2_card {
  struct ui_card_base *base;
  enum md2_card_variant variant;
  char title[64];
  char subtitle[64];
};

ui_error_t md2_card_create(struct ui_engine *engine,
                           enum md2_card_variant variant,
                           struct md2_card **out_card) {
  struct md2_card *card = NULL;
  struct ui_component *comp = NULL;
  ui_error_t rc;

  if (engine == NULL || out_card == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  card = (struct md2_card *)C_MULTIPLATFORM_MALLOC(sizeof(struct md2_card));
  if (card == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(card, 0, sizeof(*card));

  card->variant = variant;

  rc = ui_card_base_create(&card->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(card);
    return rc;
  }

  ui_card_base_get_component(card->base, &comp);

  const char *cls = (variant == MD2_CARD_OUTLINED)
                        ? "md2-card md2-card-outlined"
                        : "md2-card md2-card-elevated";
  rc = ui_dom_node_set_attribute(comp->shadow_root, "class", cls);
  if (rc != UI_ERROR_NONE) {
    ui_card_base_destroy(card->base);
    C_MULTIPLATFORM_FREE(card);
    return rc;
  }

  *out_card = card;
  return UI_ERROR_NONE;
}

ui_error_t md2_card_destroy(struct md2_card *card) {
  ui_error_t rc;

  if (card == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_card_base_destroy(card->base);
  card->base = NULL;

  C_MULTIPLATFORM_FREE(card);
  return rc;
}

ui_error_t md2_card_get_base(struct md2_card *card,
                             struct ui_card_base **out_base) {
  if (card == NULL || out_base == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = card->base;
  return UI_ERROR_NONE;
}

ui_error_t md2_card_set_title(struct md2_card *card, const char *title) {
  if (card == NULL || title == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(card->title, sizeof(card->title), title, _TRUNCATE);
#else
  strncpy(card->title, title, sizeof(card->title) - 1);
  card->title[sizeof(card->title) - 1] = '\0';
#endif

  return ui_card_base_set_title(card->base, title);
}

ui_error_t md2_card_set_subtitle(struct md2_card *card, const char *subtitle) {
  if (card == NULL || subtitle == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(card->subtitle, sizeof(card->subtitle), subtitle, _TRUNCATE);
#else
  strncpy(card->subtitle, subtitle, sizeof(card->subtitle) - 1);
  card->subtitle[sizeof(card->subtitle) - 1] = '\0';
#endif

  return ui_card_base_set_subtitle(card->base, subtitle);
}
