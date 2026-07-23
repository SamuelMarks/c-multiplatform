#ifdef _WIN32
/* clang-format off */
#include <winsock2.h>
#include <wingdi.h>
#include "../include/ui_window_backend_win32.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

/** \brief ui_window */
struct ui_window {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    int is_closing;
    int has_resize;
    int new_width;
    int new_height;
    void (*on_resize_callback)(void*, int, int);
    void* on_resize_user_data;
};

static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    struct ui_window* win;
    if (uMsg == WM_CREATE) {
        CREATESTRUCTA* pCreate = (CREATESTRUCTA*)lParam;
        win = (struct ui_window*)pCreate->lpCreateParams;
#ifdef _WIN64
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)win);
#else
        SetWindowLongA(hwnd, GWL_USERDATA, (LONG)win);
#endif
    } else {
#ifdef _WIN64
        win = (struct ui_window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
#else
        win = (struct ui_window*)GetWindowLongA(hwnd, GWL_USERDATA);
#endif
    }

    if (win) {
        switch (uMsg) {
            case WM_CLOSE:
                win->is_closing = 1;
                return 0; /* Handled */
            case WM_DESTROY:
                /* We don't PostQuitMessage because we have multiple windows potentially and don't rely on global thread loop */
                return 0;
            case WM_SIZE:
                win->new_width = LOWORD(lParam);
                win->new_height = HIWORD(lParam);
                win->has_resize = 1;
                if (win->on_resize_callback) {
                    win->on_resize_callback(win->on_resize_user_data, win->new_width, win->new_height);
                }
                return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static enum ui_error win32_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    struct ui_window* win;
    WNDCLASSA wc;
    HINSTANCE hInstance;
    PIXELFORMATDESCRIPTOR pfd;
    int pixel_format;
    enum ui_error rc;

    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    hInstance = GetModuleHandleA(NULL);

    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(NULL, (LPCSTR)IDI_APPLICATION); /* cast for C89 */
    wc.hCursor = LoadCursorA(NULL, (LPCSTR)IDC_ARROW);   /* cast for C89 */
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "UIEngineWindow";

    RegisterClassA(&wc);

    win = (struct ui_window*)UI_MALLOC(sizeof(struct ui_window));
    if (!win) {
        rc = UI_ERROR_OUT_OF_MEMORY;
        goto cleanup;
    }
    win->is_closing = 0;
    win->has_resize = 0;
    win->new_width = 0;
    win->new_height = 0;
    win->on_resize_callback = NULL;
    win->on_resize_user_data = NULL;
    win->hwnd = NULL;
    win->hdc = NULL;
    win->hglrc = NULL;

    win->hwnd = CreateWindowExA(
        0, "UIEngineWindow", title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, win);

    if (!win->hwnd) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    win->hdc = GetDC(win->hwnd);
    if (!win->hdc) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    /* Initialize pfd to 0 manually for C89 */
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 0; pfd.cRedShift = 0;
    pfd.cGreenBits = 0; pfd.cGreenShift = 0;
    pfd.cBlueBits = 0; pfd.cBlueShift = 0;
    pfd.cAlphaBits = 8; pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0; pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0; pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.cAuxBuffers = 0;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;

    pixel_format = ChoosePixelFormat(win->hdc, &pfd);
    if (!pixel_format) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    if (!SetPixelFormat(win->hdc, pixel_format, &pfd)) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    win->hglrc = wglCreateContext(win->hdc);
    if (!win->hglrc) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    if (!wglMakeCurrent(win->hdc, win->hglrc)) {
        rc = UI_ERROR_UNKNOWN;
        goto cleanup;
    }

    *out_window = win;
    return UI_ERROR_NONE;

cleanup:
    if (win) {
        if (win->hdc && win->hwnd) {
            ReleaseDC(win->hwnd, win->hdc);
        }
        if (win->hwnd) {
            DestroyWindow(win->hwnd);
        }
        UI_FREE(win);
    }
    return rc;
}

static enum ui_error win32_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    if (window->hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(window->hglrc);
    }
    if (window->hdc && window->hwnd) {
        ReleaseDC(window->hwnd, window->hdc);
    }
    if (window->hwnd) {
        DestroyWindow(window->hwnd);
    }
    UI_FREE(window);
    return UI_ERROR_NONE;
}

static enum ui_error win32_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    ShowWindow(window->hwnd, SW_SHOW);
    return UI_ERROR_NONE;
}

static enum ui_error win32_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    ShowWindow(window->hwnd, SW_HIDE);
    return UI_ERROR_NONE;
}

static enum ui_error win32_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    MSG msg;

    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;

    if (PeekMessageA(&msg, window->hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    if (window->is_closing) {
        out_event->type = UI_EVENT_WINDOW_CLOSE;
        *out_has_event = 1;
        window->is_closing = 0; /* Reset state after dispatching */
    } else if (window->has_resize) {
        out_event->type = UI_EVENT_WINDOW_RESIZE;
        out_event->event_data.window.width = window->new_width;
        out_event->event_data.window.height = window->new_height;
        *out_has_event = 1;
        window->has_resize = 0; /* Reset state after dispatching */
    }

    return UI_ERROR_NONE;
}

static void* win32_get_os_handle(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) return NULL;
    return (void*)window->hwnd;
}

static enum ui_error win32_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    SwapBuffers(window->hdc);
    return UI_ERROR_NONE;
}

static enum ui_error win32_set_on_resize_callback(struct ui_window_backend* backend, struct ui_window* window, enum ui_error (*callback)(void*, int, int), void* user_data) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    window->on_resize_callback = callback;
    window->on_resize_user_data = user_data;
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_win32_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    backend = (struct ui_window_backend*)UI_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = win32_create_window;
    backend->destroy_window = win32_destroy_window;
    backend->show_window = win32_show_window;
    backend->hide_window = win32_hide_window;
    backend->poll_events = win32_poll_events;
    backend->swap_buffers = win32_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = win32_get_os_handle;
    backend->set_on_resize_callback = win32_set_on_resize_callback;
    backend->user_data = NULL;

    *out_backend = backend;
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_win32_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    UI_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Stub for non-Windows */
#include "../include/ui_window_backend_win32.h"
#include <stddef.h>
/* clang-format on */

enum ui_error
ui_window_backend_win32_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/** \brief ui_error */
enum ui_error
ui_window_backend_win32_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
