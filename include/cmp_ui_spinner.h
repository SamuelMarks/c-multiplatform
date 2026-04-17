#ifndef CMP_UI_SPINNER_H
#define CMP_UI_SPINNER_H

/* clang-format off */
#include "cmp_ffi.h"
#include "cmp.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \file cmp_ui_spinner.h
 * \brief UI Component for rendering a loading spinner.
 */

/**
 * \brief Opaque handle to a spinner UI component.
 */
typedef struct cmp_ui_spinner cmp_ui_spinner_t;

/**
 * \brief Creates a new spinner component.
 *
 * \param out_spinner Pointer to store the created spinner handle.
 * \param size The width and height of the spinner.
 * \param color The color of the spinner (ARGB).
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_spinner_create(cmp_ui_spinner_t **out_spinner, float size,
                                  uint32_t color);

/**
 * \brief Destroys a spinner component.
 *
 * \param spinner The spinner component to destroy.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_spinner_destroy(cmp_ui_spinner_t *spinner);

/**
 * \brief Retrieves the underlying UI node for the spinner.
 *
 * \param spinner The spinner component.
 * \param out_node Pointer to store the UI node.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_spinner_get_node(cmp_ui_spinner_t *spinner,
                                    cmp_ui_node_t **out_node);

/**
 * \brief Updates the spinner animation state.
 *
 * \param spinner The spinner component.
 * \param dt_ms The time delta in milliseconds since the last update.
 * \return 0 on success, or a non-zero error code on failure.
 */
CMP_API int cmp_ui_spinner_update(cmp_ui_spinner_t *spinner, float dt_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_UI_SPINNER_H */
