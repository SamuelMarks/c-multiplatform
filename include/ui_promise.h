#ifndef UI_PROMISE_H
#define UI_PROMISE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a simulated promise/future.
 */
struct ui_promise;

/**
 * @brief Represents the current state of a promise.
 */
enum ui_promise_state {
  UI_PROMISE_PENDING = 0,
  UI_PROMISE_FULFILLED = 1,
  UI_PROMISE_REJECTED = 2
};

/**
 * @brief Creates a new pending promise.
 *
 * @param out_promise Pointer to receive the new promise handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_promise_create(struct ui_promise **out_promise);

/**
 * @brief Destroys a promise and frees its resources.
 *
 * @param promise The promise to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_promise_destroy(struct ui_promise *promise);

/**
 * @brief Adds callbacks to be executed when the promise is settled.
 *        If already settled, the callback is invoked immediately.
 *
 * @param promise The promise.
 * @param on_resolve Callback invoked if the promise is fulfilled.
 * @param on_reject Callback invoked if the promise is rejected.
 * @param user_data Opaque pointer passed to the callbacks.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_promise_then(struct ui_promise *promise,
                enum ui_error (*on_resolve)(void *, void *, void **),
                enum ui_error (*on_reject)(enum ui_error, void *, void **),
                void *user_data, struct ui_promise **out_promise);
enum ui_error
ui_promise_catch(struct ui_promise *promise,
                 enum ui_error (*on_reject)(enum ui_error, void *, void **),
                 void *user_data, struct ui_promise **out_promise);
enum ui_error ui_promise_finally(struct ui_promise *promise,
                                 enum ui_error (*on_finally)(void *),
                                 void *user_data,
                                 struct ui_promise **out_promise);

/**
 * @brief Resolves the promise with a value, triggering on_resolve callbacks.
 *
 * @param promise The promise to resolve.
 * @param result The result value (cast to void*).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_promise_resolve(struct ui_promise *promise, void *result);

/**
 * @brief Rejects the promise with an error, triggering on_reject callbacks.
 *
 * @param promise The promise to reject.
 * @param error The error discriminant.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_promise_reject(struct ui_promise *promise,
                                enum ui_error error);

/**
 * @brief Gets the current state of the promise.
 *
 * @param promise The promise.
 * @param out_state Pointer to receive the current state.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_promise_get_state(struct ui_promise *promise,
                                   enum ui_promise_state *out_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PROMISE_H */
