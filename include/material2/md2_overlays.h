/**
 * @file md2_overlays.h
 * @brief Material Design 2 Overlays and Feedback (Dialog, Bottom Sheet,
 * Snackbar, Tabs).
 */

#ifndef MATERIAL2_MD2_OVERLAYS_H
#define MATERIAL2_MD2_OVERLAYS_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_dialog_base.h"
#include "ui_bottom_sheet_base.h"
#include "ui_snackbar_base.h"
#include "ui_tabs_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct md2_dialog
 * @brief Opaque handle to a Material 2 dialog instance.
 */
struct md2_dialog;

/**
 * @struct md2_bottom_sheet
 * @brief Opaque handle to a Material 2 bottom sheet instance.
 */
struct md2_bottom_sheet;

/**
 * @struct md2_snackbar
 * @brief Opaque handle to a Material 2 snackbar instance.
 */
struct md2_snackbar;

/**
 * @struct md2_tabs
 * @brief Opaque handle to a Material 2 tabs instance.
 */
struct md2_tabs;

/**
 * @brief Creates a new Material Design 2 dialog.
 *
 * @param out_dialog Pointer to receive the allocated dialog instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_dialog_create(struct md2_dialog **out_dialog);

/**
 * @brief Destroys a Material Design 2 dialog instance.
 *
 * @param dialog The dialog to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_dialog_destroy(struct md2_dialog *dialog);

/**
 * @brief Retrieves the underlying base dialog component.
 *
 * @param dialog The dialog.
 * @param out_base Pointer to receive the base dialog.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_dialog_get_base(
    struct md2_dialog *dialog, struct ui_dialog_base **out_base);

/**
 * @brief Creates a new Material Design 2 bottom sheet.
 *
 * @param out_sheet Pointer to receive the allocated bottom sheet instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_bottom_sheet_create(struct md2_bottom_sheet **out_sheet);

/**
 * @brief Destroys a Material Design 2 bottom sheet instance.
 *
 * @param sheet The bottom sheet to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_bottom_sheet_destroy(struct md2_bottom_sheet *sheet);

/**
 * @brief Retrieves the underlying base bottom sheet component.
 *
 * @param sheet The bottom sheet.
 * @param out_base Pointer to receive the base bottom sheet.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_bottom_sheet_get_base(
    struct md2_bottom_sheet *sheet, struct ui_bottom_sheet_base **out_base);

struct ui_timer;
struct ui_overlay_director;

/**
 * @brief Creates a new Material Design 2 snackbar.
 *
 * @param timer The timer used for auto-dismissal.
 * @param director The overlay director managing the snackbar.
 * @param out_snackbar Pointer to receive the allocated snackbar instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_snackbar_create(
    struct ui_timer *timer, struct ui_overlay_director *director,
    struct md2_snackbar **out_snackbar);

/**
 * @brief Destroys a Material Design 2 snackbar instance.
 *
 * @param snackbar The snackbar to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_snackbar_destroy(struct md2_snackbar *snackbar);

/**
 * @brief Retrieves the underlying base snackbar component.
 *
 * @param snackbar The snackbar.
 * @param out_base Pointer to receive the base snackbar.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_snackbar_get_base(
    struct md2_snackbar *snackbar, struct ui_snackbar_base **out_base);

/**
 * @brief Creates a new Material Design 2 tabs component.
 *
 * @param out_tabs Pointer to receive the allocated tabs instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_tabs_create(struct md2_tabs **out_tabs);

/**
 * @brief Destroys a Material Design 2 tabs instance.
 *
 * @param tabs The tabs to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_tabs_destroy(struct md2_tabs *tabs);

/**
 * @brief Retrieves the underlying base tabs component.
 *
 * @param tabs The tabs component.
 * @param out_base Pointer to receive the base tabs.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_tabs_get_base(struct md2_tabs *tabs, struct ui_tabs_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_OVERLAYS_H */
