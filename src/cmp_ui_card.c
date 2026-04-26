/* clang-format off */
#include "cmp_ui_card.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_card {
  cmp_ui_node_t *node_root;
  cmp_ui_card_style_t style;
};

/**
 * @brief cmp_ui_card_create
 *
 * @param out_card Parameter description.
 * @param style Parameter description.
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
    return rc;
  }
  memset(card, 0, sizeof(cmp_ui_card_t));

  card->style = style;

  rc = cmp_ui_box_create(&card->node_root);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_ui_card_create: cmp_ui_box_create failed\n");
    CMP_FREE(card);
    return rc;
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

/**
 * @brief cmp_ui_card_destroy
 *
 * @param card Parameter description.
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
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_get_node
 *
 * @param card Parameter description.
 * @param out_node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_ui_card_get_node(cmp_ui_card_t *card, cmp_ui_node_t **out_node) {
  if (!card || !out_node) {
    LOG_DEBUG("cmp_ui_card_get_node: Invalid arg\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = card->node_root;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_add_child
 *
 * @param card Parameter description.
 * @param child Parameter description.
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
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief cmp_ui_card_bind_a11y
 *
 * @param widget Parameter description.
 * @param tree Parameter description.
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
    return rc;
  }

  return CMP_SUCCESS;
}