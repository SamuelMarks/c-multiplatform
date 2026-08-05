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

struct ui_form_control;

/**
 * @brief Synchronous validator function signature.
 *
 * @param control The form control being validated.
 * @param value The current value of the control.
 * @param user_data Opaque pointer for validator state.
 * @param out_is_valid Pointer to store the validation result (UI_TRUE if
 * valid).
 * @return ui_error_t
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
 * @param control The form control being validated.
 * @param value The current value of the control.
 * @param user_data Opaque pointer for validator state.
 * @param out_is_valid Pointer to store the validation result.
 * @return ui_error_t
 */
typedef ui_error_t (*ui_async_validator_fn)(struct ui_form_control *control,
                                            union ui_signal_payload value,
                                            void *user_data,
                                            ui_bool_t *out_is_valid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @brief Represents a registered synchronous validator.
 */
typedef struct ui_validator {
  ui_validator_fn fn;
  void *user_data;
} ui_validator_t;

/**
 * @brief Represents a registered asynchronous validator.
 */
typedef struct ui_async_validator {
  ui_async_validator_fn fn;
  void *user_data;
} ui_async_validator_t;

#endif /* UI_FORM_VALIDATORS_H */
