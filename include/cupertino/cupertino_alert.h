/**
 * @file cupertino_alert.h
 * @brief Cupertino Alert Dialog component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_ALERT_H
#define CUPERTINO_CUPERTINO_ALERT_H

/* clang-format off */
#include "ui_alert_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_ALERT_MAX_ACTIONS 8
#define CUPERTINO_ALERT_MAX_TEXT_FIELDS 2
#define CUPERTINO_ALERT_STANDARD_WIDTH 270.0f
#define CUPERTINO_ALERT_CORNER_RADIUS 14.0f

/**
 * @enum cupertino_alert_action_style
 * @brief Styling variants for Cupertino alert actions.
 */
enum cupertino_alert_action_style {
  CUPERTINO_ALERT_ACTION_DEFAULT = 0, /**< Standard blue action. */
  CUPERTINO_ALERT_ACTION_CANCEL,      /**< Bold blue cancel action. */
  CUPERTINO_ALERT_ACTION_DESTRUCTIVE  /**< Red destructive action. */
};

/**
 * @struct cupertino_alert_action
 * @brief Represents an individual action button in a Cupertino alert dialog.
 */
struct cupertino_alert_action {
  char title[64];                          /**< Button label text. */
  enum cupertino_alert_action_style style; /**< Visual button style. */
  int is_disabled;                         /**< Disabled state flag. */
};

/**
 * @struct cupertino_alert_text_field
 * @brief Text field input embedded inside an alert dialog.
 */
struct cupertino_alert_text_field {
  char placeholder[64]; /**< Placeholder hint text. */
  char text[128];       /**< User-entered text. */
  int is_secure;        /**< 1 for password dots. */
};

/**
 * @struct cupertino_alert_descriptor
 * @brief Configuration descriptor for creating a Cupertino alert dialog.
 */
struct cupertino_alert_descriptor {
  const char *title;   /**< Bold header title. */
  const char *message; /**< Explanatory body text. */
  int is_status_role;  /**< 1 for status, 0 for alert. */
};

/**
 * @struct cupertino_alert
 * @brief Cupertino Alert Dialog instance wrapping ui_alert_base.
 */
struct cupertino_alert {
  struct ui_alert_base *base; /**< CDK alert primitive. */
  char title[128];            /**< Header title text. */
  char message[256];          /**< Body message text. */
  size_t action_count;        /**< Action buttons count. */
  struct cupertino_alert_action
      actions[CUPERTINO_ALERT_MAX_ACTIONS]; /**< Actions. */
  size_t text_field_count;                  /**< Text fields count. */
  struct cupertino_alert_text_field
      text_fields[CUPERTINO_ALERT_MAX_TEXT_FIELDS]; /**< Fields. */
  int is_open;                                      /**< Presentation state. */
};

/**
 * @brief Creates a new Cupertino alert dialog instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_alert Pointer to receive newly created alert instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_create(
    struct ui_engine *engine, const struct cupertino_alert_descriptor *desc,
    struct cupertino_alert **out_alert);

/**
 * @brief Destroys a Cupertino alert dialog instance.
 *
 * @param alert Alert instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_alert_destroy(struct cupertino_alert *alert);

/**
 * @brief Updates the alert title text.
 *
 * @param alert Target alert instance.
 * @param title New title text.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_alert_set_title(struct cupertino_alert *alert, const char *title);

/**
 * @brief Retrieves the alert title text.
 *
 * @param alert Target alert instance.
 * @param out_title Pointer to receive const pointer to title string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_title(
    const struct cupertino_alert *alert, const char **out_title);

/**
 * @brief Updates the alert body message text.
 *
 * @param alert Target alert instance.
 * @param message New message text.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_alert_set_message(struct cupertino_alert *alert, const char *message);

/**
 * @brief Retrieves the alert message text.
 *
 * @param alert Target alert instance.
 * @param out_message Pointer to receive const pointer to message string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_message(
    const struct cupertino_alert *alert, const char **out_message);

/**
 * @brief Appends an action button to the alert dialog.
 *
 * @param alert Target alert instance.
 * @param title Action label.
 * @param style Action visual style (default, cancel, destructive).
 * @param out_index Pointer to receive index of newly added action.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_add_action(
    struct cupertino_alert *alert, const char *title,
    enum cupertino_alert_action_style style, size_t *out_index);

/**
 * @brief Appends a text field input to the alert dialog (max 2).
 *
 * @param alert Target alert instance.
 * @param placeholder Placeholder hint string.
 * @param is_secure 1 for password dots, 0 for plain text.
 * @param out_index Pointer to receive index of added text field.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_add_text_field(
    struct cupertino_alert *alert, const char *placeholder, int is_secure,
    size_t *out_index);

/**
 * @brief Sets the text content of an embedded text field.
 *
 * @param alert Target alert instance.
 * @param index Text field index.
 * @param text New text string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_set_text_field_text(
    struct cupertino_alert *alert, size_t index, const char *text);

/**
 * @brief Retrieves the text content of an embedded text field.
 *
 * @param alert Target alert instance.
 * @param index Text field index.
 * @param out_text Pointer to receive const pointer to text string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_text_field_text(
    const struct cupertino_alert *alert, size_t index, const char **out_text);

/**
 * @brief Sets the open/visible state of the alert dialog.
 *
 * @param alert Target alert instance.
 * @param is_open 1 to open, 0 to dismiss.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_alert_set_open(struct cupertino_alert *alert, int is_open);

/**
 * @brief Queries whether the alert is open.
 *
 * @param alert Target alert instance.
 * @param out_is_open Pointer to receive open status (1 if open, 0 if closed).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_alert_is_open(const struct cupertino_alert *alert, int *out_is_open);

/**
 * @brief Retrieves total number of actions in the alert dialog.
 *
 * @param alert Target alert instance.
 * @param out_count Pointer to receive action count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_action_count(
    const struct cupertino_alert *alert, size_t *out_count);

/**
 * @brief Computes spring scale factor for HIG modal presentation animation.
 *
 * Maps animation progress in [0.0, 1.0] through Apple's spring curve:
 * 0.85 scale at progress 0.0, overshoots to 1.05 at progress 0.65, settles
 * to 1.0.
 *
 * @param alert Target alert instance.
 * @param progress Animation progress normalized to [0.0, 1.0].
 * @param out_scale Pointer to receive computed scale factor.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_spring_scale(
    const struct cupertino_alert *alert, float progress, float *out_scale);

/**
 * @brief Retrieves the underlying CDK alert base primitive.
 *
 * @param alert Target alert instance.
 * @param out_base Pointer to receive ui_alert_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_alert_get_base(
    struct cupertino_alert *alert, struct ui_alert_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_ALERT_H */
