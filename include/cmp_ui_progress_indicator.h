#ifndef CMP_UI_PROGRESS_INDICATOR_H
#define CMP_UI_PROGRESS_INDICATOR_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_progress_indicator.h
 * \brief UI Component for rendering a morphing progress indicator
 * (Linear/Circular).
 */

/**
 * \brief The type of progress indicator.
 */
typedef enum {
  CMP_UI_PROGRESS_INDICATOR_LINEAR = 0,
  CMP_UI_PROGRESS_INDICATOR_CIRCULAR
} cmp_ui_progress_indicator_type_t;

/**
 * \brief Opaque handle to a Progress Indicator UI component.
 */
typedef struct cmp_ui_progress_indicator cmp_ui_progress_indicator_t;

/**
 * \brief Creates a new Progress Indicator component.
 *
 * \param out_indicator Pointer to store the created handle.
 * \param type The initial type (Linear or Circular).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_create(cmp_ui_progress_indicator_t **out_indicator,
                                 cmp_ui_progress_indicator_type_t type);

/**
 * \brief Destroys a Progress Indicator component.
 *
 * \param indicator The component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_destroy(cmp_ui_progress_indicator_t *indicator);

/**
 * \brief Retrieves the underlying UI node for the Progress Indicator.
 *
 * \param indicator The component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_get_node(cmp_ui_progress_indicator_t *indicator,
                                   cmp_ui_node_t **out_node);

/**
 * \brief Updates the progress value.
 *
 * \param indicator The component.
 * \param progress The current progress (0.0f to 1.0f).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_set_progress(cmp_ui_progress_indicator_t *indicator,
                                       float progress);

/**
 * \brief Morphs the progress indicator to a new type.
 *
 * \param indicator The component.
 * \param new_type The new type to morph to.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_set_type(cmp_ui_progress_indicator_t *indicator,
                                   cmp_ui_progress_indicator_type_t new_type);

/**
 * \brief Binds the widget to the accessibility tree.
 *
 * \param widget The component.
 * \param tree The accessibility tree.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int
cmp_ui_progress_indicator_bind_a11y(cmp_ui_progress_indicator_t *widget,
                                    cmp_a11y_tree_t *tree);

#ifdef __cplusplus

#endif /* __cplusplus */

#endif /* CMP_UI_PROGRESS_INDICATOR_H */