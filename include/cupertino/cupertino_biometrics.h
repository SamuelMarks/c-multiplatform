/**
 * @file cupertino_biometrics.h
 * @brief Apple Biometric Prompt Sheet (Face ID and Touch ID) conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_BIOMETRICS_H
#define CUPERTINO_CUPERTINO_BIOMETRICS_H

/* clang-format off */
#include "ui_dialog_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_BIOMETRICS_WIDTH 270.0f
#define CUPERTINO_BIOMETRICS_MIN_HEIGHT 190.0f
#define CUPERTINO_BIOMETRICS_CORNER_RADIUS 14.0f

/**
 * @enum cupertino_biometric_type
 * @brief Supported biometric hardware modalities.
 */
enum cupertino_biometric_type {
  CUPERTINO_BIOMETRIC_FACE_ID = 0, /**< Face ID infrared facial mesh. */
  CUPERTINO_BIOMETRIC_TOUCH_ID     /**< Touch ID capacitive fingerprint. */
};

/**
 * @enum cupertino_biometric_state
 * @brief Authentication lifecycle states.
 */
enum cupertino_biometric_state {
  CUPERTINO_BIOMETRIC_STATE_READY =
      0, /**< Prompt presented, waiting for sensor. */
  CUPERTINO_BIOMETRIC_STATE_SCANNING, /**< Active beam sweep / pulsing ring. */
  CUPERTINO_BIOMETRIC_STATE_SUCCESS,  /**< Match confirmed, checkmark morph. */
  CUPERTINO_BIOMETRIC_STATE_FAILED,   /**< Match failed, error shake. */
  CUPERTINO_BIOMETRIC_STATE_PASSCODE_FALLBACK /**< PIN passcode keypad
                                                 requested. */
};

/**
 * @struct cupertino_biometrics_descriptor
 * @brief Configuration descriptor for creating a biometric prompt sheet.
 */
struct cupertino_biometrics_descriptor {
  enum cupertino_biometric_type type; /**< Face ID vs Touch ID. */
  const char *reason; /**< Localized authorization description. */
  const char *passcode_button_title; /**< Fallback button label (or NULL for
                                        default). */
  const char
      *cancel_button_title; /**< Cancel button label (or NULL for default). */
};

/**
 * @struct cupertino_biometrics_prompt
 * @brief Biometric authentication prompt instance wrapping ui_dialog_base.
 */
struct cupertino_biometrics_prompt {
  struct ui_dialog_base *base;
  enum cupertino_biometric_type type;
  enum cupertino_biometric_state state;
  char reason[128];
  char passcode_button_title[64];
  char cancel_button_title[32];
  int is_presented;
  float scan_progress;  /**< Normalized [0.0, 1.0] sweep animation. */
  float shake_offset_x; /**< Horizontal shake displacement in points. */
  float shake_time_ms;  /**< Elapsed failure shake animation time. */
  float width;
  float height;
};

/**
 * @brief Creates a new Apple Biometric prompt sheet.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_prompt Pointer to receive newly created prompt instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_create(struct ui_engine *engine,
                            const struct cupertino_biometrics_descriptor *desc,
                            struct cupertino_biometrics_prompt **out_prompt);

/**
 * @brief Destroys a biometric prompt instance.
 *
 * @param prompt Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_destroy(struct cupertino_biometrics_prompt *prompt);

/**
 * @brief Presents the biometric prompt modal on screen.
 *
 * @param prompt Target prompt.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_present(struct cupertino_biometrics_prompt *prompt);

/**
 * @brief Dismisses the biometric prompt modal.
 *
 * @param prompt Target prompt.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_dismiss(struct cupertino_biometrics_prompt *prompt);

/**
 * @brief Processes biometric sensor match outcome.
 *
 * @param prompt Target prompt.
 * @param is_success 1 for successful match, 0 for failure/rejection.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_biometrics_authenticate(
    struct cupertino_biometrics_prompt *prompt, int is_success);

/**
 * @brief Switches prompt to passcode PIN fallback view.
 *
 * @param prompt Target prompt.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_switch_to_passcode(
    struct cupertino_biometrics_prompt *prompt);

/**
 * @brief Advances scan and shake animations frame.
 *
 * @param prompt Target prompt.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_biometrics_tick(
    struct cupertino_biometrics_prompt *prompt, float delta_ms);

/**
 * @brief Gets current authentication lifecycle state.
 *
 * @param prompt Target prompt.
 * @param out_state Pointer to receive state enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_get_state(const struct cupertino_biometrics_prompt *prompt,
                               enum cupertino_biometric_state *out_state);

/**
 * @brief Gets current horizontal shake displacement in points.
 *
 * @param prompt Target prompt.
 * @param out_offset_x Pointer to receive horizontal shake offset.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_biometrics_get_shake_offset(
    const struct cupertino_biometrics_prompt *prompt, float *out_offset_x);

/**
 * @brief Gets bounding dimensions of the prompt sheet.
 *
 * @param prompt Target prompt.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_biometrics_get_dimensions(
    const struct cupertino_biometrics_prompt *prompt, float *out_width,
    float *out_height);

/**
 * @brief Retrieves underlying CDK dialog base.
 *
 * @param prompt Target prompt.
 * @param out_base Pointer to receive ui_dialog_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_biometrics_get_base(struct cupertino_biometrics_prompt *prompt,
                              struct ui_dialog_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_BIOMETRICS_H */
