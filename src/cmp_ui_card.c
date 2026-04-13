/* clang-format off */
#include "cmp_ui_card.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_card {
  cmp_ui_node_t *node_root;
  cmp_ui_card_style_t style;
};

int cmp_ui_card_create(cmp_ui_card_t **out_card, cmp_ui_card_style_t style) {
  cmp_ui_card_t *card;
  int err;

  if (!out_card) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_card_t), (void **)&card);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(card, 0, sizeof(cmp_ui_card_t));

  card->style = style;

  err = cmp_ui_box_create(&card->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(card);
    return err;
  }

  card->node_root->layout->direction = CMP_FLEX_COLUMN;
  /* Specific stylings like drop shadow or borders will be mapped via CSS logic
   */
  if (style == CMP_UI_CARD_STYLE_ELEVATED) {
    /* Set elevation class/variables */
    card->node_root->bg_color = 0xFFFFFFFF;
  } else if (style == CMP_UI_CARD_STYLE_FILLED) {
    card->node_root->bg_color = 0xFFE0E0E0;
  } else if (style == CMP_UI_CARD_STYLE_OUTLINED) {
    card->node_root->bg_color = 0x00000000;
    /* would also have a border... */
  }

  *out_card = card;
  return CMP_SUCCESS;
}

int cmp_ui_card_destroy(cmp_ui_card_t *card) {
  if (!card) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(card);
  return CMP_SUCCESS;
}

int cmp_ui_card_get_node(cmp_ui_card_t *card, cmp_ui_node_t **out_node) {
  if (!card || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = card->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_card_add_child(cmp_ui_card_t *card, cmp_ui_node_t *child) {
  if (!card || !child) {
    return CMP_ERROR_INVALID_ARG;
  }
  return cmp_ui_node_add_child(card->node_root, child);
}
int cmp_ui_card_bind_a11y(cmp_ui_card_t *widget, cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "group", "Card");
  return CMP_SUCCESS;
}
