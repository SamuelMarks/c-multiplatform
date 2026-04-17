#ifndef CMP_UI_VIRTUAL_LIST_H
#define CMP_UI_VIRTUAL_LIST_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_virtual_list.h
 * \brief UI Component for rendering a virtualized list of items.
 */

/**
 * \brief Opaque handle to a virtual list UI component.
 */
typedef struct cmp_ui_virtual_list cmp_ui_virtual_list_t;

/**
 * \brief Creates a new virtual list component.
 *
 * \param out_list Pointer to store the created list handle.
 * \param item_count The total number of items in the list.
 * \param item_height The uniform height of each item.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_virtual_list_create(cmp_ui_virtual_list_t **out_list,
                                       size_t item_count, float item_height);

/**
 * \brief Destroys a virtual list component.
 *
 * \param list The component to destroy.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_virtual_list_destroy(cmp_ui_virtual_list_t *list);

/**
 * \brief Retrieves the underlying UI node.
 *
 * \param list The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_virtual_list_get_node(cmp_ui_virtual_list_t *list,
                                         cmp_ui_node_t **out_node);

/**
 * \brief Updates the total item count.
 *
 * \param list The component.
 * \param item_count The new total item count.
 * \return 0 on success, or a non-zero error code.
 */
CMP_API int cmp_ui_virtual_list_set_item_count(cmp_ui_virtual_list_t *list,
                                               size_t item_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_VIRTUAL_LIST_H */
