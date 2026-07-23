#ifndef UI_REACTOR_H
#define UI_REACTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Opaque handle representing a multi-reactor event loop.
 */
struct ui_reactor;

/**
 * @brief Event flags for reactor registration.
 */
#define UI_REACTOR_EVENT_READ 1
#define UI_REACTOR_EVENT_WRITE 2
#define UI_REACTOR_EVENT_ERROR 4

/**
 * @brief Creates a new reactor instance.
 *
 * @param out_reactor Pointer to receive the new reactor handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_reactor_create(struct ui_reactor **out_reactor);

/**
 * @brief Destroys a reactor instance.
 *
 * @param reactor The reactor to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if reactor is
 * NULL.
 */
enum ui_error ui_reactor_destroy(struct ui_reactor *reactor);

/**
 * @brief Registers an OS handle (fd or SOCKET) with the reactor.
 *
 * @param reactor The reactor to register with.
 * @param os_handle The OS handle to monitor (cast to void*).
 * @param events Bitmask of UI_REACTOR_EVENT_* flags.
 * @param callback The function to execute when events occur.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_reactor_register(struct ui_reactor *reactor, void *os_handle, int events,
                    enum ui_error (*callback)(void *, int, void *),
                    void *user_data);

/**
 * @brief Unregisters an OS handle from the reactor.
 *
 * @param reactor The reactor.
 * @param os_handle The OS handle to unregister.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_reactor_unregister(struct ui_reactor *reactor,
                                    void *os_handle);

/**
 * @brief Polls the reactor for events and dispatches callbacks.
 *
 * @param reactor The reactor to poll.
 * @param timeout_ms Maximum time to wait in milliseconds (-1 for infinite).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_reactor_poll(struct ui_reactor *reactor, int timeout_ms);

/**
 * @brief Schedules a callback to be executed on the reactor's thread.
 *
 * @param reactor The reactor.
 * @param callback The function to execute.
 * @param user_data Opaque pointer passed to the callback.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_reactor_schedule(struct ui_reactor *reactor,
                                  enum ui_error (*callback)(void *),
                                  void *user_data);

/**
 * @brief Wakes up a reactor blocked in polling.
 *
 * @param reactor The reactor to wake.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_reactor_wake(struct ui_reactor *reactor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_REACTOR_H */
