/**
 * @file ui_window_backend_android.c
 * @brief ui_window_backend_android.c implementation.
 */
#ifdef __ANDROID__

/* clang-format off */
#include "../include/ui_window_backend_android.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <android/native_activity.h>
#include <EGL/egl.h>

/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
    ANativeWindow* window; /**< window */
    EGLDisplay display; /**< display */
    EGLSurface surface; /**< surface */
    EGLContext context; /**< context */
    int is_closing; /**< is_closing */
};

/**
 * @brief android_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t android_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_window = NULL;
    return UI_ERROR_UNKNOWN; /* Stubbed native setup until hooked to Android lifecycle */
}

/**
 * @brief android_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t android_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief android_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t android_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief android_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t android_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief android_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t android_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
}

/**
 * @brief android_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t android_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_android_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_android_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    backend = (struct ui_window_backend*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = android_create_window;
    backend->destroy_window = android_destroy_window;
    backend->show_window = android_show_window;
    backend->hide_window = android_hide_window;
    backend->poll_events = android_poll_events;
    backend->swap_buffers = android_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = NULL;
    backend->set_on_resize_callback = NULL;
    backend->user_data = NULL;

    *out_backend = backend;
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_android_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_android_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    C_MULTIPLATFORM_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Non-Android Platform Stub */
#include "../include/ui_window_backend_android.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief ui_window_backend_android_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_android_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/* \brief ui_error
 */
ui_error_t
ui_window_backend_android_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
