#ifndef UI_AUTH_H
#define UI_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_promise.h"
/* clang-format on */

/**
 * @brief Represents the result of an authentication request.
 */
enum ui_auth_result {
  UI_AUTH_RESULT_SUCCESS = 0,
  UI_AUTH_RESULT_FAILED = 1,
  UI_AUTH_RESULT_CANCELLED = 2,
  UI_AUTH_RESULT_NOT_AVAILABLE = 3
};

/**
 * @brief Configuration for a biometric/secure enclave authentication request.
 */
struct ui_auth_request_config {
  const char *reason; /**< The reason presented to the user (e.g., "Authenticate
                         to view secure data") */
  const char *cancel_title; /**< The localized title for the cancel button, if
                               supported */
  int allow_device_credential; /**< If true, fallback to PIN/Password is allowed
                                */
};

/**
 * @brief Checks if biometric authentication is available on the current device.
 *
 * @param out_is_available Pointer to receive the availability status (1 if
 * available, 0 otherwise).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_auth_is_supported(int *out_is_available);

/**
 * @brief Requests authentication using the system's secure enclave
 * (Biometrics/Windows Hello). The function returns immediately. The provided
 * promise will be resolved with a pointer to an `enum ui_auth_result` (which
 * the caller should interpret or copy) when the OS dialog completes.
 *
 * @param config The authentication configuration.
 * @param promise A pending promise that will be resolved/rejected upon
 * completion.
 * @return UI_ERROR_NONE if the request was successfully dispatched.
 */
enum ui_error ui_auth_request_async(const struct ui_auth_request_config *config,
                                    struct ui_promise *promise);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_AUTH_H */
