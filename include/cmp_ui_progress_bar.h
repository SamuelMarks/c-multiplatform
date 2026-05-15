#ifndef CMP_UI_PROGRESS_BAR_H
#define CMP_UI_PROGRESS_BAR_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * \file cmp_ui_progress_bar.h
 * \brief UI Component for rendering a linear progress bar.
 */

/**
 * \brief Opaque handle to a progress bar UI component.
 */
typedef struct cmp_ui_progress_bar cmp_ui_progress_bar_t;

/**
 * \brief Creates a new progress bar component.
 *
 * \param out_bar Pointer to store the created progress bar handle.
 * \param track_color Background color of the progress track (ARGB).
 * \param fill_color Color of the filled progress indicator (ARGB).
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_progress_bar_create(cmp_ui_progress_bar_t **out_bar,
                                       uint32_t track_color,
                                       uint32_t fill_color);

/**
 * \brief Destroys a progress bar component.
 *
 * \param bar The progress bar component to destroy.
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_progress_bar_destroy(cmp_ui_progress_bar_t *bar);

/**
 * \brief Retrieves the underlying UI node for the progress bar.
 *
 * \param bar The progress bar component.
 * \param out_node Pointer to store the UI node.
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_progress_bar_get_node(cmp_ui_progress_bar_t *bar,
                                         cmp_ui_node_t **out_node);

/**
 * \brief Updates the progress value.
 *
 * \param bar The progress bar component.
 * \param progress The current progress (0.0f to 1.0f).
 *
 * \return 0 on success, or a non-zero error code on failure.
 */
int CMP_API cmp_ui_progress_bar_set_progress(cmp_ui_progress_bar_t *bar,
                                             float progress);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_PROGRESS_BAR_H */
