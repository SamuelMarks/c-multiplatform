/* clang-format off */
#include "cmp_ui_card.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Opaque internal structure for UI Card widget.
 */
struct cmp_ui_card {
  /** @brief The root node of the card */
  cmp_ui_node_t *node_root;
  /** @brief The styling variant of the card */
  cmp_ui_card_style_t style;
};

/**
 * @brief cmp_ui_card_create
 *
 * @param out_card Pointer to store the created card.
 * @param style The styling variant of the card.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_create(cmp_ui_card_t **out_card, cmp_ui_card_style_t style) {
  int rc;
  cmp_ui_card_t *card = NULL;

  if (!out_card) {
    LOG_DEBUG("cmp_ui_card_create: out_card is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_ui_card_t), (void **)&card);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_create: OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(card, 0, sizeof(cmp_ui_card_t));

  card->style = style;

  rc = cmp_ui_box_create(&card->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_create: cmp_ui_box_create failed\n");
    rc = CMP_FREE(card);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_card_create: CMP_FREE failed\n");
    }
    return CMP_ERROR_GENERAL;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&card->node_root->layout);
  if (rc == CMP_SUCCESS) {
    memset(card->node_root->layout, 0, sizeof(cmp_layout_node_t));
    card->node_root->layout->id = 1;
    card->node_root->layout->direction = CMP_FLEX_COLUMN;
  }

  if (style == CMP_UI_CARD_STYLE_ELEVATED) {
    if (card->node_root)
      card->node_root->bg_color = 0xFFFFFFFF;
  } else if (style == CMP_UI_CARD_STYLE_FILLED) {
    if (card->node_root)
      card->node_root->bg_color = 0xFFE0E0E0;
  } else if (style == CMP_UI_CARD_STYLE_OUTLINED) {
    if (card->node_root)
      card->node_root->bg_color = 0x00000000;
  }

  *out_card = card;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_destroy
 *
 * @param card The card component.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_destroy(cmp_ui_card_t *card) {
  int rc;

  if (!card) {
    LOG_DEBUG("cmp_ui_card_destroy: card is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (card->node_root) {
    rc = cmp_ui_node_destroy(card->node_root);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ui_card_destroy: cmp_ui_node_destroy failed\n");
    }
  }

  rc = CMP_FREE(card);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_destroy: CMP_FREE failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_get_node
 *
 * @param card The card component.
 * @param out_node Pointer to store the underlying UI node.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_get_node(cmp_ui_card_t *card, cmp_ui_node_t **out_node) {
  int rc;
  rc = 0;
  if (!card || !out_node) {
    LOG_DEBUG("cmp_ui_card_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = card->node_root;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief cmp_ui_card_add_child
 *
 * @param card The card component.
 * @param child The child node to add.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_add_child(cmp_ui_card_t *card, cmp_ui_node_t *child) {
  int rc;

  if (!card || !child) {
    LOG_DEBUG("cmp_ui_card_add_child: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_ui_node_add_child(card->node_root, child);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_add_child: cmp_ui_node_add_child failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_bind_a11y
 *
 * @param widget The component.
 * @param tree The accessibility tree.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_bind_a11y(cmp_ui_card_t *widget, cmp_a11y_tree_t *tree) {
  int rc;

  if (!widget || !tree) {
    LOG_DEBUG("cmp_ui_card_bind_a11y: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (!widget->node_root || !widget->node_root->layout) {
    LOG_DEBUG("cmp_ui_card_bind_a11y: widget missing layout\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "group",
                              "Card");
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_bind_a11y: cmp_a11y_tree_add_node failed\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  return CMP_SUCCESS;
}