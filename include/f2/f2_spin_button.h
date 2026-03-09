#ifndef F2_SPIN_BUTTON_H
#define F2_SPIN_BUTTON_H

/**
 * @file f2_spin_button.h
 * @brief Microsoft Fluent 2 SpinButton widget.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_ui.h"
#include "f2/f2_button.h"
#include "f2/f2_text_field.h"

/**
 * @brief Fluent 2 SpinButton style descriptor.
 */
typedef struct F2SpinButtonStyle {
  F2TextFieldStyle text_field_style; /**< Style for the text input area. */
  F2ButtonStyle button_style;        /**< Style for the up/down buttons. */
} F2SpinButtonStyle;

struct F2SpinButton;

/**
 * @brief SpinButton change callback.
 * @param ctx User callback context pointer.
 * @param spin_button SpinButton instance.
 * @param value New value.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *F2SpinButtonOnChange)(void *ctx,
                                            struct F2SpinButton *spin_button,
                                            CMPScalar value);

/**
 * @brief Fluent 2 SpinButton widget.
 */
typedef struct F2SpinButton {
  CMPWidget widget;               /**< Widget interface. */
  F2SpinButtonStyle style;        /**< Current style. */
  F2TextField text_field;         /**< Internal text field widget. */
  F2Button up_button;             /**< Internal up button. */
  F2Button down_button;           /**< Internal down button. */
  CMPRect bounds;                 /**< Layout bounds. */
  CMPScalar value;                /**< Current numeric value. */
  CMPScalar min_value;            /**< Minimum numeric value. */
  CMPScalar max_value;            /**< Maximum numeric value. */
  CMPScalar step;                 /**< Increment/decrement step. */
  F2SpinButtonOnChange on_change; /**< Change callback. */
  void *on_change_ctx;            /**< Callback context. */
} F2SpinButton;

/**
 * @brief Initialize a default Fluent 2 spin button style.
 * @param style Style descriptor to initialize.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_spin_button_style_init(F2SpinButtonStyle *style);

/**
 * @brief Initialize a Fluent 2 spin button.
 * @param spin_button Spin button instance.
 * @param backend Text backend.
 * @param style Style descriptor.
 * @param allocator Memory allocator for internal text buffers.
 * @param min_val Minimum allowed value.
 * @param max_val Maximum allowed value.
 * @param step Step to add/subtract per button click.
 * @param initial_val Initial value.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_spin_button_init(F2SpinButton *spin_button,
                                         const CMPTextBackend *backend,
                                         const F2SpinButtonStyle *style,
                                         CMPAllocator *allocator,
                                         CMPScalar min_val, CMPScalar max_val,
                                         CMPScalar step, CMPScalar initial_val);

/**
 * @brief Set the current value.
 * @param spin_button Spin button instance.
 * @param value New value.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_spin_button_set_value(F2SpinButton *spin_button,
                                              CMPScalar value);

/**
 * @brief Set the change callback.
 * @param spin_button Spin button instance.
 * @param on_change Callback function.
 * @param ctx Context pointer.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_spin_button_set_on_change(
    F2SpinButton *spin_button, F2SpinButtonOnChange on_change, void *ctx);

/**
 * @brief Clean up the spin button.
 * @param spin_button Spin button instance.
 * @return CMP_OK on success.
 */
CMP_API int CMP_CALL f2_spin_button_cleanup(F2SpinButton *spin_button);

#ifdef __cplusplus
}
#endif

#endif /* F2_SPIN_BUTTON_H */
