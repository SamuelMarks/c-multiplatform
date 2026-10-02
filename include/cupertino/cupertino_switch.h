/**
 * @file cupertino_switch.h
 * @brief Cupertino Switch component wrapping ui_slide_toggle_base with Apple
 * HIG styling.
 */

#ifndef CUPERTINO_CUPERTINO_SWITCH_H
#define CUPERTINO_CUPERTINO_SWITCH_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_slide_toggle_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard Apple HIG switch track width (51pt).
 */
#define CUPERTINO_SWITCH_TRACK_WIDTH 51.0f
/**
 * @brief Standard Apple HIG switch track height (31pt).
 */
#define CUPERTINO_SWITCH_TRACK_HEIGHT 31.0f
/**
 * @brief Standard Apple HIG switch thumb diameter (27pt).
 */
#define CUPERTINO_SWITCH_THUMB_DIAMETER 27.0f
/**
 * @brief Standard Apple HIG switch thumb drag width (33pt).
 */
#define CUPERTINO_SWITCH_THUMB_DRAG_WIDTH 33.0f

/**
 * @struct cupertino_switch
 * @brief Cupertino Switch widget holding state and wrapping
 * ui_slide_toggle_base.
 */
struct cupertino_switch {
  struct ui_slide_toggle_base *base;    /**< Wrapped CDK toggle primitive. */
  struct ui_control_value_accessor cva; /**< Exported CVA interface. */
  int show_accessibility_labels; /**< 1 to render 'I' and 'O' on/off labels. */
  int is_dragging;               /**< 1 if user is interactively dragging. */
  float thumb_width;             /**< 27.0f normal, 33.0f when dragging. */
  ui_color_t active_color;       /**< Track fill color when ON. */
  ui_color_t track_color;        /**< Track fill color when OFF. */
};

/**
 * @brief Creates a new Cupertino Switch wrapping ui_slide_toggle_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_switch Pointer to receive newly created switch.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_switch_create(
    struct ui_engine *engine, struct cupertino_switch **out_switch,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino switch and its underlying base.
 *
 * @param sw Switch to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_switch_destroy(struct cupertino_switch *sw);

/**
 * @brief Sets checked state of the switch.
 *
 * @param sw The switch.
 * @param checked 1 if selected (ON), 0 if unselected (OFF).
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_switch_set_checked(struct cupertino_switch *sw, int checked);

/**
 * @brief Gets checked state of the switch.
 *
 * @param sw The switch.
 * @param out_checked Pointer to receive checked state (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_switch_get_checked(
    const struct cupertino_switch *sw, int *out_checked);

/**
 * @brief Sets disabled state of the switch.
 *
 * @param sw The switch.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_switch_set_disabled(struct cupertino_switch *sw, int disabled);

/**
 * @brief Enables or disables VoiceOver on/off glyph labels ('I' and 'O').
 *
 * @param sw The switch.
 * @param show_labels 1 to render on/off symbols, 0 to hide.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_switch_set_show_accessibility_labels(struct cupertino_switch *sw,
                                               int show_labels);

/**
 * @brief Sets interactive dragging state, stretching the thumb horizontally.
 *
 * @param sw The switch.
 * @param is_dragging 1 if dragging (thumb stretches to 33pt), 0 if released.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_switch_set_dragging(struct cupertino_switch *sw, int is_dragging);

/**
 * @brief Retrieves underlying ui_slide_toggle_base handle.
 *
 * @param sw The switch.
 * @param out_base Pointer to receive ui_slide_toggle_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_switch_get_base(
    struct cupertino_switch *sw, struct ui_slide_toggle_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SWITCH_H */
