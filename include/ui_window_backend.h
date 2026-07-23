#ifndef UI_WINDOW_BACKEND_H
#define UI_WINDOW_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Forward declaration for unified input event struct.
 */
struct ui_event;

/**
 * @brief Opaque handle representing a native window.
 */
struct ui_window;

/**
 * @brief Abstract vtable for platform-specific windowing and rendering context
 * creation.
 */
struct ui_window_backend {
  /**
   * @brief Creates a native window and rendering context.
   *
   * @param backend The backend instance.
   * @param title The initial window title.
   * @param width The initial window width in virtual pixels.
   * @param height The initial window height in virtual pixels.
   * @param out_window Pointer to receive the native window handle.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*create_window)(struct ui_window_backend *backend,
                                 const char *title, int width, int height,
                                 struct ui_window **out_window);

  /**
   * @brief Destroys a native window and its rendering context.
   *
   * @param backend The backend instance.
   * @param window The window to destroy.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*destroy_window)(struct ui_window_backend *backend,
                                  struct ui_window *window);

  /**
   * @brief Shows a previously hidden window.
   *
   * @param backend The backend instance.
   * @param window The window to show.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*show_window)(struct ui_window_backend *backend,
                               struct ui_window *window);

  /**
   * @brief Hides a visible window.
   *
   * @param backend The backend instance.
   * @param window The window to hide.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*hide_window)(struct ui_window_backend *backend,
                               struct ui_window *window);

  /**
   * @brief Polls the OS event queue for the specified window.
   *
   * @param backend The backend instance.
   * @param window The window to poll.
   * @param out_event Pointer to receive the event data.
   * @param out_has_event Set to 1 if an event was dequeued, 0 if queue is
   * empty.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*poll_events)(struct ui_window_backend *backend,
                               struct ui_window *window,
                               struct ui_event *out_event, int *out_has_event);

  /**
   * @brief Swaps the rendering buffers (e.g., EGLSwapBuffers, SwapBuffers,
   * etc).
   *
   * @param backend The backend instance.
   * @param window The window whose buffers should be swapped.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*swap_buffers)(struct ui_window_backend *backend,
                                struct ui_window *window);

  /**
   * @brief Injects an OS-level deep link URI into the event queue (e.g. from
   * App Links or Universal Links).
   *
   * @param backend The backend instance.
   * @param uri The deep link URI string.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*push_deep_link)(struct ui_window_backend *backend,
                                  const char *uri);

  /**
   * @brief Retrieves the native OS window handle (e.g., HWND, NSWindow*, etc.).
   *
   * @param backend The backend instance.
   * @param window The window instance.
   * @return The native handle, or NULL if unavailable.
   */
  void *(*get_os_handle)(struct ui_window_backend *backend,
                         struct ui_window *window);

  /**
   * @brief Sets a callback to be invoked immediately when the window is
   * resized.
   *
   * @param backend The backend instance.
   * @param window The window instance.
   * @param callback The callback function.
   * @param user_data User data for the callback.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*set_on_resize_callback)(
      struct ui_window_backend *backend, struct ui_window *window,
      enum ui_error (*callback)(void *user_data, int width, int height),
      void *user_data);

  /**
   * @brief Opaque user data for the specific backend implementation.
   */
  void *user_data;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WINDOW_BACKEND_H */
