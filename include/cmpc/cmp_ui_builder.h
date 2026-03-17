#ifndef CMP_UI_BUILDER_H
#define CMP_UI_BUILDER_H

/**
 * @file cmp_ui_builder.h
 * @brief Declarative UI builder layer for constructing CMPLayoutNode trees.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "cmpc/cmp_layout.h"
#include "cmpc/cmp_form.h"
/* clang-format on */

/**
 * @brief Internal builder node structure.
 */
struct CMPUIBuilderNode;

/**
 * @brief Declarative builder for layout and widget trees.
 *
 * Manages allocation of CMPLayoutNode structures and child arrays.
 */
typedef struct CMPUIBuilder {
  CMPAllocator allocator;           /**< Allocator used for the builder. */
  struct CMPUIBuilderNode *head;    /**< Linked list of allocated nodes. */
  struct CMPUIBuilderNode *current; /**< Current parent node. */
  CMPLayoutNode *root_layout;       /**< The resulting root layout node. */
} CMPUIBuilder;

/**
 * @brief Initialize a UI builder.
 * @param builder Builder instance.
 * @param allocator Allocator for allocating layout nodes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_init(CMPUIBuilder *builder,
                                         const CMPAllocator *allocator);

/**
 * @brief Destroy a UI builder, freeing all allocated layout nodes.
 * @param builder Builder instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_destroy(CMPUIBuilder *builder);

/**
 * @brief Begin a new layout column.
 * @param builder Builder instance.
 * @param style Layout style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_begin_column(CMPUIBuilder *builder,
                                                 const CMPLayoutStyle *style);

/**
 * @brief Begin a new layout row.
 * @param builder Builder instance.
 * @param style Layout style.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_begin_row(CMPUIBuilder *builder,
                                              const CMPLayoutStyle *style);

/**
 * @brief Add a widget to the current layout parent.
 * @param builder Builder instance.
 * @param style Layout style.
 * @param widget Widget instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_add_widget(CMPUIBuilder *builder,
                                               const CMPLayoutStyle *style,
                                               CMPWidget *widget);

/**
 * @brief End the current layout parent (row or column).
 * @param builder Builder instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_end(CMPUIBuilder *builder);

/**
 * @brief Get the constructed root layout node.
 * @param builder Builder instance.
 * @param out_root Receives the root layout node pointer.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_ui_builder_get_root(CMPUIBuilder *builder,
                                             CMPLayoutNode **out_root);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_UI_BUILDER_H */
