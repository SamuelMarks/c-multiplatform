/**
 * @file md2_selection_controls.h
 * @brief Material Design 2 Selection Controls (Checkbox, Radio, Switch,
 * Slider).
 */

#ifndef MATERIAL2_MD2_SELECTION_CONTROLS_H
#define MATERIAL2_MD2_SELECTION_CONTROLS_H

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_checkbox_base.h"
#include "ui_radio_group_base.h"
#include "ui_slide_toggle_base.h"
#include "ui_slider_base.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct md2_checkbox
 * @brief Opaque handle to a Material 2 checkbox.
 */
struct md2_checkbox;

/**
 * @struct md2_radio_button
 * @brief Opaque handle to a Material 2 radio button.
 */
struct md2_radio_button;

/**
 * @struct md2_switch
 * @brief Opaque handle to a Material 2 switch.
 */
struct md2_switch;

/**
 * @struct md2_slider
 * @brief Opaque handle to a Material 2 slider.
 */
struct md2_slider;

/**
 * @brief Creates a new Material Design 2 checkbox.
 *
 * @param out_checkbox Pointer to receive the allocated checkbox instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_checkbox_create(struct md2_checkbox **out_checkbox);

/**
 * @brief Destroys a Material Design 2 checkbox instance.
 *
 * @param checkbox The checkbox to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_checkbox_destroy(struct md2_checkbox *checkbox);

/**
 * @brief Retrieves the underlying component wrapper for a checkbox.
 *
 * @param checkbox The checkbox.
 * @param out_component Pointer to receive the component.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_checkbox_get_component(
    struct md2_checkbox *checkbox, struct ui_component **out_component);

/**
 * @brief Creates a new Material Design 2 radio button.
 *
 * @param out_radio Pointer to receive the allocated radio button instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_radio_button_create(struct md2_radio_button **out_radio);

/**
 * @brief Destroys a Material Design 2 radio button instance.
 *
 * @param radio The radio button to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_radio_button_destroy(struct md2_radio_button *radio);

/**
 * @brief Retrieves the underlying radio group base for a radio button.
 *
 * @param radio The radio button.
 * @param out_base Pointer to receive the base.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_radio_button_get_base(
    struct md2_radio_button *radio, struct ui_radio_group_base **out_base);

/**
 * @brief Creates a new Material Design 2 switch.
 *
 * @param out_switch Pointer to receive the allocated switch instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_switch_create(struct md2_switch **out_switch);

/**
 * @brief Destroys a Material Design 2 switch instance.
 *
 * @param sw The switch to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_switch_destroy(struct md2_switch *sw);

/**
 * @brief Retrieves the underlying slide toggle base for a switch.
 *
 * @param sw The switch.
 * @param out_base Pointer to receive the base.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_switch_get_base(
    struct md2_switch *sw, struct ui_slide_toggle_base **out_base);

/**
 * @brief Creates a new Material Design 2 slider.
 *
 * @param out_slider Pointer to receive the allocated slider instance.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_slider_create(struct md2_slider **out_slider);

/**
 * @brief Destroys a Material Design 2 slider instance.
 *
 * @param slider The slider to destroy.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md2_slider_destroy(struct md2_slider *slider);

/**
 * @brief Retrieves the underlying slider base for a slider.
 *
 * @param slider The slider.
 * @param out_base Pointer to receive the base.
 * @return UI_ERROR_NONE on success.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md2_slider_get_base(
    struct md2_slider *slider, struct ui_slider_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL2_MD2_SELECTION_CONTROLS_H */
