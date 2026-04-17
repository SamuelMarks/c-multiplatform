#ifndef CMP_UI_SKELETON_H
#define CMP_UI_SKELETON_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_skeleton.h
 * \brief UI Component for rendering a loading skeleton shimmer.
 */

/**
 * \brief Opaque handle to a skeleton UI component.
 */
typedef struct cmp_ui_skeleton cmp_ui_skeleton_t;

/**
 * \brief Creates a new skeleton component.
 *
 * \param out_skeleton Pointer to store the created skeleton handle.
 * \param width Width of the skeleton block.
 * \param height Height of the skeleton block.
 * \param base_color Base background color (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_skeleton_create(cmp_ui_skeleton_t **out_skeleton,
                                   float width, float height,
                                   uint32_t base_color);

/**
 * \brief Destroys a skeleton component.
 *
 * \param skeleton The skeleton component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_skeleton_destroy(cmp_ui_skeleton_t *skeleton);

/**
 * \brief Retrieves the underlying UI node for the skeleton.
 *
 * \param skeleton The skeleton component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_skeleton_get_node(cmp_ui_skeleton_t *skeleton,
                                     cmp_ui_node_t **out_node);

/**
 * \brief Updates the skeleton animation state.
 *
 * \param skeleton The skeleton component.
 * \param dt_ms The time delta in milliseconds since the last update.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_skeleton_update(cmp_ui_skeleton_t *skeleton, float dt_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_SKELETON_H */
