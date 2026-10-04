/**
 * @file sample_stub.c
 * @brief Generic stub for unimplemented sampler examples.
 */

/* clang-format off */
#include "sampler/sampler_samples.h"
#include "material3/md3_card.h"
#include "ui_dom_node.h"
#include "ui_card_base.h"
#include <stddef.h>
/* clang-format on */

sampler_error_t sample_stub_create(struct ui_engine *engine,
                                   struct ui_dom_node *container) {
  struct md3_card *card = NULL;
  struct ui_card_base *base = NULL;
  struct ui_component *comp = NULL;
  struct ui_dom_node *root = NULL;
  ui_error_t u_rc;

  u_rc = md3_card_create(engine, MD3_CARD_OUTLINED, &card);
  if (u_rc != UI_ERROR_NONE) {
    return SAMPLER_ERROR_OUT_OF_MEMORY;
  }

  u_rc = md3_card_set_title(card, "Not Implemented");
  if (u_rc != UI_ERROR_NONE) {
    md3_card_destroy(card);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  u_rc = md3_card_set_subtitle(card, "This sample is coming soon.");
  if (u_rc != UI_ERROR_NONE) {
    md3_card_destroy(card);
    return SAMPLER_ERROR_STYLE_PROP_FAILED;
  }

  u_rc = md3_card_get_base(card, &base);
  if (u_rc == UI_ERROR_NONE && base != NULL) {
    u_rc = ui_card_base_get_component(base, &comp);
    if (u_rc == UI_ERROR_NONE && comp != NULL) {
      root = comp->shadow_root;
    }
  }

  if (root != NULL) {
    u_rc = ui_dom_node_append_child(container, root);
    if (u_rc != UI_ERROR_NONE) {
      md3_card_destroy(card);
      return SAMPLER_ERROR_DOM_ATTACH_FAILED;
    }
  } else {
    md3_card_destroy(card);
    return SAMPLER_ERROR_DOM_ATTACH_FAILED;
  }

  return SAMPLER_SUCCESS;
}
