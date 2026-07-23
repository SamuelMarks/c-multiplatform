#if defined(__APPLE__) && defined(__MACH__)
/* clang-format off */
#include <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE

#include "../include/ui_window_backend_macos.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

/** \brief ui_window */
struct ui_window {
    id window;
    id context;
    int is_closing;
};

static enum ui_error macos_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Bootstrapping Cocoa in pure C using objc_msgSend requires careful architecture handling.
     * We initialize the opaque pointers to NULL until the objective-c FFI layer is finalized.
     */
    *out_window = NULL;
    return UI_ERROR_UNKNOWN;
}

static enum ui_error macos_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static enum ui_error macos_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static enum ui_error macos_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static enum ui_error macos_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
}

static enum ui_error macos_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_macos_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    backend = (struct ui_window_backend*)UI_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = macos_create_window;
    backend->destroy_window = macos_destroy_window;
    backend->show_window = macos_show_window;
    backend->hide_window = macos_hide_window;
    backend->poll_events = macos_poll_events;
    backend->swap_buffers = macos_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = NULL;
    backend->set_on_resize_callback = NULL;
    backend->user_data = NULL;

    *out_backend = backend;
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_macos_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    UI_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Apple platform but not macOS (e.g. iOS) */
#include "../include/ui_window_backend_macos.h"
#include <stddef.h>

enum ui_error ui_window_backend_macos_create(struct ui_window_backend** out_backend) {
    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;
    return UI_ERROR_UNKNOWN;
}

enum ui_error ui_window_backend_macos_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_UNKNOWN;
}
#endif
#else
/* Non-Apple Platform Stub */
#include "../include/ui_window_backend_macos.h"
#include <stddef.h>
/* clang-format on */

enum ui_error
ui_window_backend_macos_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/** \brief ui_error */
enum ui_error
ui_window_backend_macos_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
