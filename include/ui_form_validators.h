/**
 * @file ui_form_validators.h
 * @brief Form validation callback signatures and structs.
 *
 * This header provides the structures and type definitions for synchronous
 * and asynchronous form validation.
 */

#ifndef UI_FORM_VALIDATORS_H
#define UI_FORM_VALIDATORS_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_thread_pool.h"
#include "ui_reactor.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration for a form control.
 */
struct ui_form_control;

/**
 * @brief Synchronous validator function signature.
 *
 * @param control Pointer to the form control being validated.
 * @param value The current value of the control.
 * @param user_data Opaque pointer for validator state.
 * @param out_is_valid Pointer to store the validation result (1 for valid, 0
 * for invalid).
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_validator_fn)(struct ui_form_control *control,
                                      union ui_signal_payload value,
                                      void *user_data, ui_bool_t *out_is_valid);

/**
 * @brief Asynchronous validator function signature.
 *
 * This function is designed to be executed on a background thread.
 * It should perform HTTP/IO checks or long-running computations.
 *
 * @param control Pointer to the form control being validated.
 * @param value The current value of the control.
 * @param user_data Opaque pointer for validator state.
 * @param out_is_valid Pointer to store the validation result (1 for valid, 0
 * for invalid).
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
typedef ui_error_t (*ui_async_validator_fn)(struct ui_form_control *control,
                                            union ui_signal_payload value,
                                            void *user_data,
                                            ui_bool_t *out_is_valid);

/**
 * @brief Represents a registered synchronous validator.
 */
typedef struct ui_validator {
  ui_validator_fn fn; /**< The synchronous validator callback function. */
  void *user_data;    /**< Opaque user data for the validator. */
} ui_validator_t;

/**
 * @brief Represents a registered asynchronous validator.
 */
typedef struct ui_async_validator {
  ui_async_validator_fn
      fn;          /**< The asynchronous validator callback function. */
  void *user_data; /**< Opaque user data for the async validator. */
} ui_async_validator_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FORM_VALIDATORS_H */
