/**
 * @file ui_bottom_app_bar_base.h
 * @brief Bottom App Bar base component.
 */

#ifndef UI_BOTTOM_APP_BAR_BASE_H
#define UI_BOTTOM_APP_BAR_BASE_H

/** @brief Forward declaration of ui_signal. */
struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_fab_base.h"
/* clang-format on */

/** @brief Opaque handle to a bottom app bar component. */
struct ui_bottom_app_bar_base;

/**
 * @brief Specifies the alignment of the Floating Action Button within the
 * Bottom App Bar.
 */
enum ui_bottom_app_bar_fab_alignment {
  UI_BOTTOM_APP_BAR_FAB_CENTER, /**< FAB aligned in the center. */
  UI_BOTTOM_APP_BAR_FAB_END     /**< FAB aligned to the end (trailing side). */
};

/**
 * @brief Creates a new unstyled bottom app bar base component.
 *
 * This component acts as a structural layout manager that automatically
 * applies geometric cutouts (notches) using CSS clip-paths to accommodate
 * an overlapping Floating Action Button (FAB).
 *
 * @param out_bar Pointer to receive the allocated bottom app bar base.
 * @return UI_ERROR_NONE on success, or an appropriate error enum.
 */
ui_error_t
ui_bottom_app_bar_base_create(struct ui_bottom_app_bar_base **out_bar);

/**
 * @brief Destroys a bottom app bar component.
 *
 * @param bar The bottom app bar to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_bottom_app_bar_base_destroy(struct ui_bottom_app_bar_base *bar);

/**
 * @brief Gets the underlying component.
 *
 * @param bar The bottom app bar.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_app_bar_base_get_component(struct ui_bottom_app_bar_base *bar,
                                     struct ui_component **out_component);

/**
 * @brief Attaches a FAB to the app bar to generate the cutout geometry.
 *
 * The bar will use the FAB's geometry to compute the proper CSS clip-path for
 * the notch.
 *
 * @param bar The bottom app bar.
 * @param fab The floating action button to integrate.
 * @param alignment The alignment of the FAB (center or end).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_app_bar_base_set_fab(struct ui_bottom_app_bar_base *bar,
                               struct ui_fab_base *fab,
                               enum ui_bottom_app_bar_fab_alignment alignment);

/**
 * @brief Binds the active state/index to a signal.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_bottom_app_bar_base_bind_active_index(struct ui_bottom_app_bar_base *widget,
                                         struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_BOTTOM_APP_BAR_BASE_H */
