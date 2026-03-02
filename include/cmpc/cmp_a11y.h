#ifndef CMP_A11Y_H
#define CMP_A11Y_H

/**
 * @file cmp_a11y.h
 * @brief Accessibility semantics tree utilities for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"

/**
 * @brief Accessibility semantics node for tree traversal.
 *
 * Nodes can represent widgets or purely virtual accessibility elements.
 */
typedef struct CMPA11yNode {
  CMPSemantics semantics;        /**< Semantics payload for the node. */
  CMPWidget *widget;             /**< Backing widget instance (optional). */
  struct CMPA11yNode *parent;    /**< Parent node (NULL for root). */
  struct CMPA11yNode **children; /**< Child node array (not owned). */
  cmp_usize child_count;         /**< Number of child nodes. */
} CMPA11yNode;

/**
 * @brief Initialize a semantics descriptor with defaults.
 * @param semantics Semantics descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_semantics_init(CMPSemantics *semantics);

/**
 * @brief Initialize an accessibility node.
 * @param node Node to initialize.
 * @param widget Widget backing the node (may be NULL for virtual nodes).
 * @param semantics Semantics payload to copy (NULL clears to defaults).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_init(CMPA11yNode *node, CMPWidget *widget,
                                        const CMPSemantics *semantics);

/**
 * @brief Update semantics for an accessibility node.
 * @param node Node to update.
 * @param semantics Semantics payload to copy (NULL clears to defaults).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_set_semantics(CMPA11yNode *node,
                                                 const CMPSemantics *semantics);

/**
 * @brief Update the backing widget for an accessibility node.
 * @param node Node to update.
 * @param widget Widget backing the node (may be NULL for virtual nodes).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_set_widget(CMPA11yNode *node,
                                              CMPWidget *widget);

/**
 * @brief Assign children to an accessibility node.
 * @param node Parent node to update.
 * @param children Child node array (may be NULL when count is 0).
 * @param count Number of child nodes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_set_children(CMPA11yNode *node,
                                                CMPA11yNode **children,
                                                cmp_usize count);

/**
 * @brief Retrieve the parent of a node.
 * @param node Node to query.
 * @param out_parent Receives the parent node (may be NULL for root).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_get_parent(const CMPA11yNode *node,
                                              CMPA11yNode **out_parent);

/**
 * @brief Retrieve the number of children for a node.
 * @param node Node to query.
 * @param out_count Receives the child count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_get_child_count(const CMPA11yNode *node,
                                                   cmp_usize *out_count);

/**
 * @brief Retrieve a child node by index.
 * @param node Node to query.
 * @param index Child index.
 * @param out_child Receives the child node.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_get_child(const CMPA11yNode *node,
                                             cmp_usize index,
                                             CMPA11yNode **out_child);

/**
 * @brief Retrieve the next sibling of a node.
 * @param node Node to query.
 * @param out_sibling Receives the next sibling (NULL if none).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_get_next_sibling(const CMPA11yNode *node,
                                                    CMPA11yNode **out_sibling);

/**
 * @brief Retrieve the previous sibling of a node.
 * @param node Node to query.
 * @param out_sibling Receives the previous sibling (NULL if none).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_node_get_prev_sibling(const CMPA11yNode *node,
                                                    CMPA11yNode **out_sibling);

/**
 * @brief Find the next focusable node in the accessibility tree.
 * @param current Currently focused node (may be NULL to start from root).
 * @param root Root node of the tree.
 * @param out_next Receives the next focusable node (NULL if none found).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_focus_next(CMPA11yNode *current,
                                         CMPA11yNode *root,
                                         CMPA11yNode **out_next);

/**
 * @brief Find the previous focusable node in the accessibility tree.
 * @param current Currently focused node.
 * @param root Root node of the tree.
 * @param out_prev Receives the previous focusable node (NULL if none found).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_a11y_focus_prev(CMPA11yNode *current,
                                         CMPA11yNode *root,
                                         CMPA11yNode **out_prev);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_A11Y_H */
