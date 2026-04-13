#ifndef CMP_UI_CARD_H
#define CMP_UI_CARD_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_card.h
 * \brief UI Component for rendering a Card (Elevated/Filled/Outlined).
 */

/**
 * \brief The style of the Card.
 */
typedef enum {
  CMP_UI_CARD_STYLE_ELEVATED = 0,
  CMP_UI_CARD_STYLE_FILLED,
  CMP_UI_CARD_STYLE_OUTLINED
} cmp_ui_card_style_t;

/**
 * \brief Opaque handle to a Card UI component.
 */
typedef struct cmp_ui_card cmp_ui_card_t;

/**
 * \brief Creates a new Card component.
 *
 * \param out_card Pointer to store the created handle.
 * \param style The style of the Card.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_card_create(cmp_ui_card_t **out_card,
                               cmp_ui_card_style_t style);

/**
 * \brief Destroys a Card component.
 *
 * \param card The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_card_destroy(cmp_ui_card_t *card);

/**
 * \brief Retrieves the underlying UI node for the Card.
 *
 * \param card The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_card_get_node(cmp_ui_card_t *card, cmp_ui_node_t **out_node);

/**
 * \brief Adds a child node to the Card's content area.
 *
 * \param card The component.
 * \param child The child node to add.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_card_add_child(cmp_ui_card_t *card, cmp_ui_node_t *child);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_card_bind_a11y(cmp_ui_card_t *widget, cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_CARD_H */