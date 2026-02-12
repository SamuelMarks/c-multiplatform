#ifndef M3_A11Y_H
#define M3_A11Y_H

/**
 * @file m3_a11y.h
 * @brief Accessibility semantics tree utilities for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_ui.h"

/**
 * @brief Accessibility semantics node for tree traversal.
 *
 * Nodes can represent widgets or purely virtual accessibility elements.
 */
typedef struct M3A11yNode {
  M3Semantics semantics;        /**< Semantics payload for the node. */
  M3Widget *widget;             /**< Backing widget instance (optional). */
  struct M3A11yNode *parent;    /**< Parent node (NULL for root). */
  struct M3A11yNode **children; /**< Child node array (not owned). */
  m3_usize child_count;         /**< Number of child nodes. */
} M3A11yNode;

/**
 * @brief Initialize a semantics descriptor with defaults.
 * @param semantics Semantics descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_semantics_init(M3Semantics *semantics);

/**
 * @brief Initialize an accessibility node.
 * @param node Node to initialize.
 * @param widget Widget backing the node (may be NULL for virtual nodes).
 * @param semantics Semantics payload to copy (NULL clears to defaults).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_init(M3A11yNode *node, M3Widget *widget,
                                     const M3Semantics *semantics);

/**
 * @brief Update semantics for an accessibility node.
 * @param node Node to update.
 * @param semantics Semantics payload to copy (NULL clears to defaults).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_set_semantics(M3A11yNode *node,
                                              const M3Semantics *semantics);

/**
 * @brief Update the backing widget for an accessibility node.
 * @param node Node to update.
 * @param widget Widget backing the node (may be NULL for virtual nodes).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_set_widget(M3A11yNode *node, M3Widget *widget);

/**
 * @brief Assign children to an accessibility node.
 * @param node Parent node to update.
 * @param children Child node array (may be NULL when count is 0).
 * @param count Number of child nodes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_set_children(M3A11yNode *node,
                                             M3A11yNode **children,
                                             m3_usize count);

/**
 * @brief Retrieve the parent of a node.
 * @param node Node to query.
 * @param out_parent Receives the parent node (may be NULL for root).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_get_parent(const M3A11yNode *node,
                                           M3A11yNode **out_parent);

/**
 * @brief Retrieve the number of children for a node.
 * @param node Node to query.
 * @param out_count Receives the child count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_get_child_count(const M3A11yNode *node,
                                                m3_usize *out_count);

/**
 * @brief Retrieve a child node by index.
 * @param node Node to query.
 * @param index Child index.
 * @param out_child Receives the child node.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_get_child(const M3A11yNode *node,
                                          m3_usize index,
                                          M3A11yNode **out_child);

/**
 * @brief Retrieve the next sibling of a node.
 * @param node Node to query.
 * @param out_sibling Receives the next sibling (NULL if none).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_get_next_sibling(const M3A11yNode *node,
                                                 M3A11yNode **out_sibling);

/**
 * @brief Retrieve the previous sibling of a node.
 * @param node Node to query.
 * @param out_sibling Receives the previous sibling (NULL if none).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_a11y_node_get_prev_sibling(const M3A11yNode *node,
                                                 M3A11yNode **out_sibling);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_A11Y_H */
