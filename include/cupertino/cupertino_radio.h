/**
 * @file cupertino_radio.h
 * @brief Cupertino Radio selection control wrapping ui_toggle_base and
 * ui_radio_group_base.
 */

#ifndef CUPERTINO_CUPERTINO_RADIO_H
#define CUPERTINO_CUPERTINO_RADIO_H

/* clang-format off */
#include "ui_color_space.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_radio_group_base.h"
#include "ui_toggle_base.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Standard Apple HIG radio dimensions.
 */
#define CUPERTINO_RADIO_OUTER_DIAMETER 18.0f
/**
 * @brief Standard Apple HIG radio inner dot diameter.
 */
#define CUPERTINO_RADIO_INNER_DOT_DIAMETER 8.0f

/**
 * @struct cupertino_radio
 * @brief Cupertino Radio button holding state and wrapping ui_toggle_base.
 */
struct cupertino_radio {
  struct ui_toggle_base *toggle; /**< Wrapped CDK toggle primitive. */
  struct ui_radio_group_base
      *group; /**< Attached radio group for mutual exclusion. */
  struct ui_control_value_accessor cva; /**< Control Value Accessor. */
  int is_checked;                       /**< Non-zero if selected. */
  int is_disabled;                      /**< Non-zero if disabled. */
  float scale;              /**< Spring scale (1.0 to 1.15 on select). */
  float outer_diameter;     /**< 18.0pt outer dimension. */
  float inner_dot_diameter; /**< 8.0pt inner white concentric dot. */
  ui_color_t active_color;  /**< SystemBlue (#007AFF) fill when ON. */
  ui_color_t border_color;  /**< SystemGray4 (#D1D1D6) outline when OFF. */
};

/**
 * @brief Creates a new Cupertino Radio component.
 *
 * @param engine Pointer to ui_engine.
 * @param out_radio Pointer to receive newly created radio.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_radio_create(
    struct ui_engine *engine, struct cupertino_radio **out_radio,
    struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Cupertino radio and its underlying toggle primitive.
 *
 * @param radio The radio to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_radio_destroy(struct cupertino_radio *radio);

/**
 * @brief Sets selection state of the radio, triggering spring scale pop.
 *
 * @param radio The radio.
 * @param checked 1 if selected, 0 if unselected.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_radio_set_checked(struct cupertino_radio *radio, int checked);

/**
 * @brief Gets current selection state.
 *
 * @param radio The radio.
 * @param out_checked Pointer to receive selected state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_radio_get_checked(
    const struct cupertino_radio *radio, int *out_checked);

/**
 * @brief Sets disabled state of the radio.
 *
 * @param radio The radio.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_radio_set_disabled(struct cupertino_radio *radio, int disabled);

/**
 * @brief Binds the radio button to a radio group for mutual exclusion.
 *
 * @param radio The radio.
 * @param group The group manager.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_radio_bind_group(
    struct cupertino_radio *radio, struct ui_radio_group_base *group);

/**
 * @brief Retrieves underlying ui_toggle_base handle.
 *
 * @param radio The radio.
 * @param out_toggle Pointer to receive ui_toggle_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_radio_get_toggle(
    struct cupertino_radio *radio, struct ui_toggle_base **out_toggle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_RADIO_H */
