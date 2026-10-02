/**
 * @file cupertino_init.h
 * @brief Cupertino (Apple HIG) lifecycle and initialization APIs.
 */

#ifndef CUPERTINO_CUPERTINO_INIT_H
#define CUPERTINO_CUPERTINO_INIT_H

/* clang-format off */
#include "ui_error.h"
#include "ui_types.h"
#include "ui_design_system.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Initializes the Cupertino design system module and registers it.
 *
 * @param engine Pointer to the UI engine instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_initialize(struct ui_engine *engine);

/**
 * @brief Shuts down the Cupertino design system module and frees resources.
 *
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shutdown(void);

/**
 * @brief Retrieves the Cupertino baseline design system operations vtable.
 *
 * @param out_vtable Pointer to receive the vtable pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if out_vtable
 * is NULL.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_get_vtable(const struct ui_design_system_vtable **out_vtable);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_INIT_H */
