/**
 * @file ui_window_backend_ios.c
 * @brief ui_window_backend_ios.c implementation.
 */
#if defined(__APPLE__) && defined(__MACH__)
/* clang-format off */
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE

#include "../include/ui_window_backend_ios.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
    id window; /**< window */
    id context; /**< context */
    int width; /**< width */
    int height; /**< height */
    int is_closing; /**< is_closing */
    float scale_factor; /**< scale_factor */
    ui_error_t (*on_dpi_change_callback)(void*, float, float); /**< on_dpi_change_callback */
    void* on_dpi_change_user_data; /**< on_dpi_change_user_data */
};

/**
 * @brief ios_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t ios_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Bootstrapping UIKit in pure C using objc_msgSend requires careful architecture handling.
     * We initialize the opaque pointers to NULL until the objective-c FFI layer is finalized.
     */
    *out_window = NULL;
    return UI_ERROR_UNKNOWN;
}

/**
 * @brief ios_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t ios_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
}

/**
 * @brief ios_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_get_scale_factor.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_scale_factor Parameter out_scale_factor.
 * @return Return value.
 */
static ui_error_t ios_get_scale_factor(struct ui_window_backend* backend, struct ui_window* window, float* out_scale_factor) {
    Class clsUIScreen;
    id mainScreen;
    double scale = 2.0;

    if (!backend || !window || !out_scale_factor) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    clsUIScreen = objc_getClass("UIScreen");
    if (clsUIScreen) {
        mainScreen = ((id (*)(id, SEL))objc_msgSend)((id)clsUIScreen, sel_registerName("mainScreen"));
        if (mainScreen) {
            scale = ((double (*)(id, SEL))objc_msgSend)(mainScreen, sel_registerName("scale"));
        }
    }

    *out_scale_factor = (float)scale;
    return UI_ERROR_NONE;
}

/**
 * @brief ios_get_framebuffer_size.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_fb_width Parameter out_fb_width.
 * @param out_fb_height Parameter out_fb_height.
 * @return Return value.
 */
static ui_error_t ios_get_framebuffer_size(struct ui_window_backend* backend, struct ui_window* window, int* out_fb_width, int* out_fb_height) {
    float scale = 2.0f;
    ui_error_t rc;

    if (!backend || !window || !out_fb_width || !out_fb_height) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    rc = ios_get_scale_factor(backend, window, &scale);
    if (rc != UI_ERROR_NONE) {
        return rc;
    }

    *out_fb_width = (int)((float)window->width * scale);
    *out_fb_height = (int)((float)window->height * scale);
    return UI_ERROR_NONE;
}

/**
 * @brief ios_set_on_dpi_change_callback.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t ios_set_on_dpi_change_callback(
    struct ui_window_backend* backend, struct ui_window* window,
    ui_error_t (*callback)(void*, float, float), void* user_data) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    window->on_dpi_change_callback = callback;
    window->on_dpi_change_user_data = user_data;
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_ios_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    backend = (struct ui_window_backend*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = ios_create_window;
    backend->destroy_window = ios_destroy_window;
    backend->show_window = ios_show_window;
    backend->hide_window = ios_hide_window;
    backend->poll_events = ios_poll_events;
    backend->swap_buffers = ios_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = NULL;
    backend->set_on_resize_callback = NULL;
    backend->get_scale_factor = ios_get_scale_factor;
    backend->get_framebuffer_size = ios_get_framebuffer_size;
    backend->set_on_dpi_change_callback = ios_set_on_dpi_change_callback;
    backend->user_data = NULL;

    *out_backend = backend;
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    C_MULTIPLATFORM_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Apple platform but not iOS (e.g. macOS) */
#include "../include/ui_window_backend_ios.h"
#include <stddef.h>

ui_error_t ui_window_backend_ios_create(struct ui_window_backend** out_backend) {
    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;
    return UI_ERROR_UNKNOWN;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_UNKNOWN;
}
#endif
#else
/* Non-Apple Platform Stub */
#include "../include/ui_window_backend_ios.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief ui_window_backend_ios_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_ios_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
