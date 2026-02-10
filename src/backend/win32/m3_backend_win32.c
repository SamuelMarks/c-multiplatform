#include "m3/m3_backend_win32.h"

#include "m3/m3_log.h"
#include "m3/m3_object.h"

#include <string.h>
#include <limits.h>
#include <wchar.h>

#if defined(M3_WIN32_AVAILABLE)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#endif

#define M3_WIN32_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_WIN32_RETURN_IF_ERROR_CLEANUP(rc, cleanup) \
    do { \
        if ((rc) != M3_OK) { \
            cleanup; \
            return (rc); \
        } \
    } while (0)

#define M3_WIN32_DEFAULT_HANDLE_CAPACITY 64u
#define M3_WIN32_EVENT_CAPACITY 256u
#define M3_WIN32_CLIP_STACK_CAPACITY 32u

static int m3_win32_backend_validate_config(const M3Win32BackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->handle_capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->allocator != NULL) {
        if (config->allocator->alloc == NULL || config->allocator->realloc == NULL || config->allocator->free == NULL) {
            return M3_ERR_INVALID_ARGUMENT;
        }
    }
    return M3_OK;
}

int M3_CALL m3_win32_backend_is_available(M3Bool *out_available)
{
    if (out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_WIN32_AVAILABLE)
    *out_available = M3_TRUE;
#else
    *out_available = M3_FALSE;
#endif
    return M3_OK;
}

int M3_CALL m3_win32_backend_config_init(M3Win32BackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = M3_WIN32_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (m3_usize)~(m3_usize)0;
    config->enable_logging = M3_TRUE;
    config->inline_tasks = M3_TRUE;
    return M3_OK;
}

#if defined(M3_WIN32_AVAILABLE)

#define M3_WIN32_TYPE_WINDOW 1
#define M3_WIN32_TYPE_TEXTURE 2
#define M3_WIN32_TYPE_FONT 3

typedef BOOL(WINAPI *M3Win32AlphaBlendFn)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

typedef struct M3Win32EventQueue {
    M3InputEvent events[M3_WIN32_EVENT_CAPACITY];
    m3_usize head;
    m3_usize tail;
    m3_usize count;
} M3Win32EventQueue;

typedef struct M3Win32Window {
    M3ObjectHeader header;
    struct M3Win32Backend *backend;
    HWND hwnd;
    HDC mem_dc;
    HBITMAP dib;
    void *dib_pixels;
    m3_i32 width;
    m3_i32 height;
    M3Scalar dpi_scale;
    M3Scalar dpi_scale_override;
    M3Bool has_scale_override;
    m3_u32 flags;
    M3Bool visible;
} M3Win32Window;

typedef struct M3Win32Texture {
    M3ObjectHeader header;
    struct M3Win32Backend *backend;
    HDC mem_dc;
    HBITMAP dib;
    void *pixels;
    m3_i32 width;
    m3_i32 height;
    m3_u32 format;
    m3_u32 bytes_per_pixel;
    m3_i32 stride;
} M3Win32Texture;

typedef struct M3Win32Font {
    M3ObjectHeader header;
    struct M3Win32Backend *backend;
    HFONT font;
    m3_i32 size_px;
    m3_i32 weight;
    M3Bool italic;
} M3Win32Font;

struct M3Win32Backend {
    M3Allocator allocator;
    M3HandleSystem handles;
    M3WS ws;
    M3Gfx gfx;
    M3Env env;
    M3IO io;
    M3Sensors sensors;
    M3Camera camera;
    M3Network network;
    M3Tasks tasks;
    M3Bool initialized;
    M3Bool log_enabled;
    M3Bool log_owner;
    M3Bool inline_tasks;
    m3_usize clipboard_limit;
    m3_u32 time_ms;
    HINSTANCE instance;
    HMODULE msimg32;
    M3Win32AlphaBlendFn alpha_blend;
    ATOM window_class;
    M3Bool class_registered;
    M3Win32Window *active_window;
    M3Mat3 transform;
    M3Bool has_transform;
    m3_usize clip_depth;
    M3Win32EventQueue event_queue;
};

static const wchar_t g_m3_win32_window_class_name[] = L"LibM3CWindow";

static int m3_win32_mul_usize(m3_usize a, m3_usize b, m3_usize *out_value)
{
    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (a != 0 && b > ((m3_usize)~(m3_usize)0) / a) {
        return M3_ERR_OVERFLOW;
    }
    *out_value = a * b;
    return M3_OK;
}

static int m3_win32_add_usize(m3_usize a, m3_usize b, m3_usize *out_value)
{
    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (b > ((m3_usize)~(m3_usize)0) - a) {
        return M3_ERR_OVERFLOW;
    }
    *out_value = a + b;
    return M3_OK;
}

static int m3_win32_network_error_from_winhttp(DWORD error_code)
{
    switch (error_code) {
    case ERROR_WINHTTP_TIMEOUT:
        return M3_ERR_TIMEOUT;
    case ERROR_WINHTTP_INVALID_URL:
        return M3_ERR_INVALID_ARGUMENT;
    case ERROR_WINHTTP_NAME_NOT_RESOLVED:
        return M3_ERR_NOT_FOUND;
    case ERROR_WINHTTP_SECURE_FAILURE:
    case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED:
    case ERROR_WINHTTP_LOGIN_FAILURE:
        return M3_ERR_PERMISSION;
    case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
        return M3_ERR_UNSUPPORTED;
    case ERROR_WINHTTP_CANNOT_CONNECT:
    case ERROR_WINHTTP_CONNECTION_ERROR:
    case ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR:
    case ERROR_WINHTTP_AUTO_PROXY_SERVICE_FAILED:
        return M3_ERR_IO;
    default:
        return M3_ERR_IO;
    }
}

static int m3_win32_network_copy_wide_range(struct M3Win32Backend *backend, const wchar_t *src, DWORD src_len,
    wchar_t **out_wide)
{
    m3_usize len_usize;
    m3_usize alloc_size;
    wchar_t *dst;
    int rc;

    if (backend == NULL || out_wide == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_wide = NULL;

    if (src_len > 0 && src == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_win32_add_usize((m3_usize)src_len, 1u, &len_usize);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_mul_usize(len_usize, (m3_usize)sizeof(wchar_t), &alloc_size);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = backend->allocator.alloc(backend->allocator.ctx, alloc_size, (void **)&dst);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (src_len > 0) {
        memcpy(dst, src, (size_t)src_len * sizeof(wchar_t));
    }
    dst[src_len] = L'\0';

    *out_wide = dst;
    return M3_OK;
}

static int m3_win32_backend_log(struct M3Win32Backend *backend, M3LogLevel level, const char *message)
{
    if (backend == NULL || message == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->log_enabled) {
        return M3_OK;
    }
    return m3_log_write(level, "m3.win32", message);
}

static int m3_win32_backend_log_last_error(struct M3Win32Backend *backend, const char *message)
{
    DWORD error_code;
    char buffer[256];
    int rc;

    if (backend == NULL || message == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_ERROR, message);
    M3_WIN32_RETURN_IF_ERROR(rc);

    error_code = GetLastError();
    if (error_code == 0u) {
        return M3_OK;
    }

    buffer[0] = '\0';
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error_code,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, (DWORD)sizeof(buffer), NULL);
    if (buffer[0] != '\0') {
        rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_ERROR, buffer);
        M3_WIN32_RETURN_IF_ERROR(rc);
    }
    return M3_OK;
}

static void m3_win32_backend_init_alpha_blend(struct M3Win32Backend *backend)
{
    HMODULE module;
    M3Win32AlphaBlendFn fn;

    if (backend == NULL) {
        return;
    }

    backend->msimg32 = NULL;
    backend->alpha_blend = NULL;

    module = LoadLibraryA("msimg32.dll");
    if (module == NULL) {
        return;
    }

    fn = (M3Win32AlphaBlendFn)GetProcAddress(module, "AlphaBlend");
    if (fn == NULL) {
        FreeLibrary(module);
        return;
    }

    backend->msimg32 = module;
    backend->alpha_blend = fn;
}

static void m3_win32_event_queue_init(M3Win32EventQueue *queue)
{
    if (queue == NULL) {
        return;
    }
    memset(queue, 0, sizeof(*queue));
}

static int m3_win32_event_queue_push(M3Win32EventQueue *queue, const M3InputEvent *event)
{
    if (queue == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (queue->count >= M3_WIN32_EVENT_CAPACITY) {
        return M3_ERR_OVERFLOW;
    }
    queue->events[queue->tail] = *event;
    queue->tail = (queue->tail + 1u) % M3_WIN32_EVENT_CAPACITY;
    queue->count += 1u;
    return M3_OK;
}

static int m3_win32_event_queue_pop(M3Win32EventQueue *queue, M3InputEvent *out_event, M3Bool *out_has_event)
{
    if (queue == NULL || out_event == NULL || out_has_event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (queue->count == 0u) {
        memset(out_event, 0, sizeof(*out_event));
        *out_has_event = M3_FALSE;
        return M3_OK;
    }
    *out_event = queue->events[queue->head];
    queue->head = (queue->head + 1u) % M3_WIN32_EVENT_CAPACITY;
    queue->count -= 1u;
    *out_has_event = M3_TRUE;
    return M3_OK;
}

static int m3_win32_backend_push_event(struct M3Win32Backend *backend, const M3InputEvent *event)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_win32_event_queue_push(&backend->event_queue, event);
    if (rc == M3_ERR_OVERFLOW) {
        return M3_OK;
    }
    return rc;
}

static m3_u32 m3_win32_get_modifiers(void)
{
    m3_u32 mods = 0u;

    if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
        mods |= M3_MOD_SHIFT;
    }
    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        mods |= M3_MOD_CTRL;
    }
    if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
        mods |= M3_MOD_ALT;
    }
    if ((GetKeyState(VK_LWIN) & 0x8000) != 0 || (GetKeyState(VK_RWIN) & 0x8000) != 0) {
        mods |= M3_MOD_META;
    }
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
        mods |= M3_MOD_CAPS;
    }
    if ((GetKeyState(VK_NUMLOCK) & 0x0001) != 0) {
        mods |= M3_MOD_NUM;
    }
    return mods;
}

static m3_u32 m3_win32_get_time_ms(void)
{
    return (m3_u32)GetTickCount();
}

static int m3_win32_utf8_to_wide_alloc(struct M3Win32Backend *backend, const char *utf8, int utf8_len,
                                       wchar_t **out_wide, int *out_wide_len)
{
    wchar_t *wide;
    int wide_len;
    int rc;

    if (backend == NULL || utf8 == NULL || out_wide == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_wide = NULL;
    if (out_wide_len != NULL) {
        *out_wide_len = 0;
    }

    wide_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len, NULL, 0);
    if (wide_len <= 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (utf8_len >= 0) {
        rc = backend->allocator.alloc(backend->allocator.ctx, (m3_usize)(wide_len + 1) * sizeof(wchar_t), (void **)&wide);
        M3_WIN32_RETURN_IF_ERROR(rc);
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len, wide, wide_len) != wide_len) {
            backend->allocator.free(backend->allocator.ctx, wide);
            return M3_ERR_INVALID_ARGUMENT;
        }
        wide[wide_len] = L'\0';
    } else {
        rc = backend->allocator.alloc(backend->allocator.ctx, (m3_usize)wide_len * sizeof(wchar_t), (void **)&wide);
        M3_WIN32_RETURN_IF_ERROR(rc);
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len, wide, wide_len) != wide_len) {
            backend->allocator.free(backend->allocator.ctx, wide);
            return M3_ERR_INVALID_ARGUMENT;
        }
    }

    *out_wide = wide;
    if (out_wide_len != NULL) {
        if (utf8_len >= 0) {
            *out_wide_len = wide_len;
        } else if (wide_len > 0) {
            *out_wide_len = wide_len - 1;
        } else {
            *out_wide_len = 0;
        }
    }
    return M3_OK;
}

static int m3_win32_wide_to_utf8(const wchar_t *wide, int wide_len, char *buffer, m3_usize buffer_size, m3_usize *out_length)
{
    int required;

    if (wide == NULL || buffer == NULL || out_length == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    required = WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, NULL, 0, NULL, NULL);
    if (required <= 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if ((m3_usize)required > buffer_size) {
        if (wide_len < 0) {
            *out_length = (m3_usize)(required - 1);
        } else {
            *out_length = (m3_usize)required;
        }
        return M3_ERR_RANGE;
    }

    if (WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, buffer, required, NULL, NULL) != required) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (wide_len < 0 && required > 0) {
        *out_length = (m3_usize)(required - 1);
    } else {
        *out_length = (m3_usize)required;
    }
    return M3_OK;
}

static int m3_win32_backend_resolve(struct M3Win32Backend *backend, M3Handle handle, m3_u32 type_id, void **out_obj)
{
    void *resolved;
    m3_u32 actual_type;
    int rc;

    if (out_obj != NULL) {
        *out_obj = NULL;
    }

    rc = backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
    M3_WIN32_RETURN_IF_ERROR(rc);
    if (actual_type != type_id) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (out_obj != NULL) {
        *out_obj = resolved;
    }
    return M3_OK;
}

static int m3_win32_object_retain(void *obj)
{
    return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_win32_object_release(void *obj)
{
    return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_win32_object_get_type_id(void *obj, m3_u32 *out_type_id)
{
    return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_win32_window_destroy(void *obj)
{
    M3Win32Window *window;
    struct M3Win32Backend *backend;
    int rc;

    window = (M3Win32Window *)obj;
    backend = window->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, window->header.handle);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (window->mem_dc != NULL) {
        DeleteDC(window->mem_dc);
        window->mem_dc = NULL;
    }
    if (window->dib != NULL) {
        DeleteObject(window->dib);
        window->dib = NULL;
        window->dib_pixels = NULL;
    }

    if (window->hwnd != NULL) {
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
    }

    rc = backend->allocator.free(backend->allocator.ctx, window);
    M3_WIN32_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static void m3_win32_window_cleanup_unregistered(struct M3Win32Backend *backend, M3Win32Window *window)
{
    if (window == NULL || backend == NULL) {
        return;
    }
    if (window->mem_dc != NULL) {
        DeleteDC(window->mem_dc);
        window->mem_dc = NULL;
    }
    if (window->dib != NULL) {
        DeleteObject(window->dib);
        window->dib = NULL;
        window->dib_pixels = NULL;
    }
    if (window->hwnd != NULL) {
        DestroyWindow(window->hwnd);
        window->hwnd = NULL;
    }
    backend->allocator.free(backend->allocator.ctx, window);
}

static int m3_win32_texture_destroy(void *obj)
{
    M3Win32Texture *texture;
    struct M3Win32Backend *backend;
    int rc;

    texture = (M3Win32Texture *)obj;
    backend = texture->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, texture->header.handle);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (texture->mem_dc != NULL) {
        DeleteDC(texture->mem_dc);
        texture->mem_dc = NULL;
    }
    if (texture->dib != NULL) {
        DeleteObject(texture->dib);
        texture->dib = NULL;
        texture->pixels = NULL;
    }

    rc = backend->allocator.free(backend->allocator.ctx, texture);
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static void m3_win32_texture_cleanup_unregistered(struct M3Win32Backend *backend, M3Win32Texture *texture)
{
    if (texture == NULL || backend == NULL) {
        return;
    }
    if (texture->mem_dc != NULL) {
        DeleteDC(texture->mem_dc);
        texture->mem_dc = NULL;
    }
    if (texture->dib != NULL) {
        DeleteObject(texture->dib);
        texture->dib = NULL;
        texture->pixels = NULL;
    }
    backend->allocator.free(backend->allocator.ctx, texture);
}

static int m3_win32_font_destroy(void *obj)
{
    M3Win32Font *font;
    struct M3Win32Backend *backend;
    int rc;

    font = (M3Win32Font *)obj;
    backend = font->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, font->header.handle);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (font->font != NULL) {
        DeleteObject(font->font);
        font->font = NULL;
    }

    rc = backend->allocator.free(backend->allocator.ctx, font);
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static void m3_win32_font_cleanup_unregistered(struct M3Win32Backend *backend, M3Win32Font *font)
{
    if (font == NULL || backend == NULL) {
        return;
    }
    if (font->font != NULL) {
        DeleteObject(font->font);
        font->font = NULL;
    }
    backend->allocator.free(backend->allocator.ctx, font);
}

static const M3ObjectVTable g_m3_win32_window_vtable = {
    m3_win32_object_retain,
    m3_win32_object_release,
    m3_win32_window_destroy,
    m3_win32_object_get_type_id
};

static const M3ObjectVTable g_m3_win32_texture_vtable = {
    m3_win32_object_retain,
    m3_win32_object_release,
    m3_win32_texture_destroy,
    m3_win32_object_get_type_id
};

static const M3ObjectVTable g_m3_win32_font_vtable = {
    m3_win32_object_retain,
    m3_win32_object_release,
    m3_win32_font_destroy,
    m3_win32_object_get_type_id
};

static int m3_win32_window_flags_to_style(m3_u32 flags, DWORD *out_style, DWORD *out_ex_style)
{
    DWORD style;
    DWORD ex_style;

    if (out_style == NULL || out_ex_style == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    style = 0u;
    ex_style = 0u;

    if ((flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
        style = WS_POPUP;
        ex_style = WS_EX_TOPMOST;
    } else if ((flags & M3_WS_WINDOW_BORDERLESS) != 0u) {
        style = WS_POPUP;
    } else {
        style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        if ((flags & M3_WS_WINDOW_RESIZABLE) != 0u) {
            style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
        }
    }

    *out_style = style;
    *out_ex_style = ex_style;
    return M3_OK;
}

static M3Scalar m3_win32_query_dpi_scale(HWND hwnd)
{
    HDC hdc;
    int dpi;
    M3Scalar scale;

    hdc = GetDC(hwnd);
    if (hdc == NULL) {
        return 1.0f;
    }
    dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(hwnd, hdc);
    if (dpi <= 0) {
        return 1.0f;
    }
    scale = (M3Scalar)dpi / 96.0f;
    if (scale <= 0.0f) {
        scale = 1.0f;
    }
    return scale;
}

static int m3_win32_window_ensure_backbuffer(M3Win32Window *window, m3_i32 width, m3_i32 height)
{
    BITMAPINFO bmi;
    void *pixels;
    HBITMAP dib;
    HDC mem_dc;

    if (window == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }

    if (window->mem_dc == NULL) {
        mem_dc = CreateCompatibleDC(NULL);
        if (mem_dc == NULL) {
            return M3_ERR_UNKNOWN;
        }
        window->mem_dc = mem_dc;
    }

    if (window->dib != NULL && window->width == width && window->height == height) {
        return M3_OK;
    }

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    dib = CreateDIBSection(window->mem_dc, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);
    if (dib == NULL || pixels == NULL) {
        return M3_ERR_UNKNOWN;
    }

    SelectObject(window->mem_dc, dib);
    if (window->dib != NULL) {
        DeleteObject(window->dib);
    }
    window->dib = dib;
    window->dib_pixels = pixels;
    window->width = width;
    window->height = height;
    return M3_OK;
}

static int m3_win32_apply_transform(M3Win32Window *window, const M3Mat3 *transform, M3Bool has_transform)
{
    XFORM xform;

    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (!SetGraphicsMode(window->mem_dc, GM_ADVANCED)) {
        return M3_ERR_UNKNOWN;
    }

    if (!has_transform) {
        xform.eM11 = 1.0f;
        xform.eM12 = 0.0f;
        xform.eM21 = 0.0f;
        xform.eM22 = 1.0f;
        xform.eDx = 0.0f;
        xform.eDy = 0.0f;
    } else {
        xform.eM11 = transform->m[0];
        xform.eM12 = transform->m[1];
        xform.eM21 = transform->m[3];
        xform.eM22 = transform->m[4];
        xform.eDx = transform->m[6];
        xform.eDy = transform->m[7];
    }

    if (!SetWorldTransform(window->mem_dc, &xform)) {
        return M3_ERR_UNKNOWN;
    }
    return M3_OK;
}

static m3_u8 m3_win32_channel_from_scalar(M3Scalar value)
{
    if (value <= 0.0f) {
        return 0u;
    }
    if (value >= 1.0f) {
        return 255u;
    }
    return (m3_u8)(value * 255.0f + 0.5f);
}

static COLORREF m3_win32_color_to_colorref(M3Color color)
{
    m3_u8 r = m3_win32_channel_from_scalar(color.r);
    m3_u8 g = m3_win32_channel_from_scalar(color.g);
    m3_u8 b = m3_win32_channel_from_scalar(color.b);
    return RGB(r, g, b);
}

static m3_u8 m3_win32_premultiply_channel(m3_u8 c, m3_u8 a)
{
    return (m3_u8)((((m3_u32)c) * ((m3_u32)a) + 127u) / 255u);
}

static void m3_win32_texture_copy_pixels(m3_u8 *dst, m3_i32 dst_stride, const m3_u8 *src, m3_i32 src_stride,
                                         m3_i32 width, m3_i32 height, m3_u32 format)
{
    m3_i32 y;
    m3_i32 x;
    m3_u8 r;
    m3_u8 g;
    m3_u8 b;
    m3_u8 a;
    const m3_u8 *srow;
    m3_u8 *drow;

    for (y = 0; y < height; y++) {
        srow = src + (m3_isize)y * (m3_isize)src_stride;
        drow = dst + (m3_isize)y * (m3_isize)dst_stride;
        for (x = 0; x < width; x++) {
            if (format == M3_TEX_FORMAT_RGBA8) {
                r = srow[0];
                g = srow[1];
                b = srow[2];
                a = srow[3];
                srow += 4;
            } else if (format == M3_TEX_FORMAT_BGRA8) {
                b = srow[0];
                g = srow[1];
                r = srow[2];
                a = srow[3];
                srow += 4;
            } else {
                a = srow[0];
                r = 255u;
                g = 255u;
                b = 255u;
                srow += 1;
            }

            r = m3_win32_premultiply_channel(r, a);
            g = m3_win32_premultiply_channel(g, a);
            b = m3_win32_premultiply_channel(b, a);

            drow[0] = b;
            drow[1] = g;
            drow[2] = r;
            drow[3] = a;
            drow += 4;
        }
    }
}

static LRESULT CALLBACK m3_win32_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    M3Win32Window *window;
    struct M3Win32Backend *backend;
    M3InputEvent event;
    int rc;

    window = (M3Win32Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lparam;
        window = (M3Win32Window *)create->lpCreateParams;
        if (window != NULL) {
            window->hwnd = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
        }
        return TRUE;
    }

    if (window == NULL) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    backend = window->backend;
    if (backend == NULL) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    memset(&event, 0, sizeof(event));
    event.window = window->header.handle;
    event.time_ms = (m3_u32)GetMessageTime();
    event.modifiers = m3_win32_get_modifiers();

    switch (msg) {
        case WM_CLOSE:
            event.type = M3_INPUT_WINDOW_CLOSE;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_SETFOCUS:
            event.type = M3_INPUT_WINDOW_FOCUS;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_KILLFOCUS:
            event.type = M3_INPUT_WINDOW_BLUR;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_SIZE:
            event.type = M3_INPUT_WINDOW_RESIZE;
            event.data.window.width = (m3_i32)LOWORD(lparam);
            event.data.window.height = (m3_i32)HIWORD(lparam);
            window->width = event.data.window.width;
            window->height = event.data.window.height;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            event.type = M3_INPUT_POINTER_DOWN;
            event.data.pointer.pointer_id = 0;
            event.data.pointer.x = (m3_i32)(short)LOWORD(lparam);
            event.data.pointer.y = (m3_i32)(short)HIWORD(lparam);
            event.data.pointer.buttons = (m3_i32)wparam;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            event.type = M3_INPUT_POINTER_UP;
            event.data.pointer.pointer_id = 0;
            event.data.pointer.x = (m3_i32)(short)LOWORD(lparam);
            event.data.pointer.y = (m3_i32)(short)HIWORD(lparam);
            event.data.pointer.buttons = (m3_i32)wparam;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_MOUSEMOVE:
            event.type = M3_INPUT_POINTER_MOVE;
            event.data.pointer.pointer_id = 0;
            event.data.pointer.x = (m3_i32)(short)LOWORD(lparam);
            event.data.pointer.y = (m3_i32)(short)HIWORD(lparam);
            event.data.pointer.buttons = (m3_i32)wparam;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_MOUSEWHEEL:
            event.type = M3_INPUT_POINTER_SCROLL;
            event.data.pointer.pointer_id = 0;
            event.data.pointer.x = (m3_i32)(short)LOWORD(lparam);
            event.data.pointer.y = (m3_i32)(short)HIWORD(lparam);
            event.data.pointer.scroll_x = 0;
            event.data.pointer.scroll_y = (m3_i32)(short)HIWORD(wparam);
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            event.type = M3_INPUT_KEY_DOWN;
            event.data.key.key_code = (m3_u32)wparam;
            event.data.key.native_code = (m3_u32)wparam;
            event.data.key.is_repeat = (lparam & (1 << 30)) ? M3_TRUE : M3_FALSE;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            event.type = M3_INPUT_KEY_UP;
            event.data.key.key_code = (m3_u32)wparam;
            event.data.key.native_code = (m3_u32)wparam;
            event.data.key.is_repeat = M3_FALSE;
            rc = m3_win32_backend_push_event(backend, &event);
            M3_UNUSED(rc);
            return 0;
        case WM_CHAR: {
            wchar_t wide_char;
            char utf8[8];
            int written;

            wide_char = (wchar_t)wparam;
            written = WideCharToMultiByte(CP_UTF8, 0, &wide_char, 1, utf8, (int)sizeof(utf8) - 1, NULL, NULL);
            if (written > 0) {
                utf8[written] = '\0';
                event.type = M3_INPUT_TEXT;
                memset(event.data.text.utf8, 0, sizeof(event.data.text.utf8));
                if (written > (int)sizeof(event.data.text.utf8) - 1) {
                    written = (int)sizeof(event.data.text.utf8) - 1;
                }
                memcpy(event.data.text.utf8, utf8, (m3_usize)written);
                event.data.text.length = (m3_u32)written;
                rc = m3_win32_backend_push_event(backend, &event);
                M3_UNUSED(rc);
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_NCDESTROY:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            window->hwnd = NULL;
            break;
        default:
            break;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

static int m3_win32_pump_messages(struct M3Win32Backend *backend)
{
    MSG msg;

    M3_UNUSED(backend);

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return M3_OK;
}

static int m3_win32_ws_init(void *ws, const M3WSConfig *config)
{
    struct M3Win32Backend *backend;
    int rc;

    if (ws == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_win32_ws_shutdown(void *ws)
{
    struct M3Win32Backend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_win32_ws_create_window(void *ws, const M3WSWindowConfig *config, M3Handle *out_window)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    DWORD style;
    DWORD ex_style;
    RECT rect;
    wchar_t *title_wide;
    int rc;
    m3_i32 width;
    m3_i32 height;
    int has_title;

    if (ws == NULL || config == NULL || out_window == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->width <= 0 || config->height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)ws;
    out_window->id = 0u;
    out_window->generation = 0u;

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3Win32Window), (void **)&window);
    M3_WIN32_RETURN_IF_ERROR(rc);

    memset(window, 0, sizeof(*window));
    window->backend = backend;
    window->width = config->width;
    window->height = config->height;
    window->dpi_scale = 1.0f;
    window->flags = config->flags;
    window->visible = M3_FALSE;
    window->has_scale_override = M3_FALSE;

    rc = m3_win32_window_flags_to_style(config->flags, &style, &ex_style);
    if (rc != M3_OK) {
        backend->allocator.free(backend->allocator.ctx, window);
        return rc;
    }

    width = config->width;
    height = config->height;
    if ((config->flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
        width = (m3_i32)GetSystemMetrics(SM_CXSCREEN);
        height = (m3_i32)GetSystemMetrics(SM_CYSCREEN);
    } else {
        rect.left = 0;
        rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        if (!AdjustWindowRectEx(&rect, style, FALSE, ex_style)) {
            backend->allocator.free(backend->allocator.ctx, window);
            return M3_ERR_UNKNOWN;
        }
        width = (m3_i32)(rect.right - rect.left);
        height = (m3_i32)(rect.bottom - rect.top);
    }

    title_wide = NULL;
    has_title = (config->utf8_title != NULL && config->utf8_title[0] != '\0');
    if (has_title) {
        rc = m3_win32_utf8_to_wide_alloc(backend, config->utf8_title, -1, &title_wide, NULL);
        if (rc != M3_OK) {
            backend->allocator.free(backend->allocator.ctx, window);
            return rc;
        }
    }

    window->hwnd = CreateWindowExW(ex_style, g_m3_win32_window_class_name,
                                   (has_title && title_wide != NULL) ? title_wide : L"",
                                   style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height, NULL, NULL,
                                   backend->instance, window);

    if (title_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, title_wide);
    }

    if (window->hwnd == NULL) {
        m3_win32_backend_log_last_error(backend, "ws.create_window");
        backend->allocator.free(backend->allocator.ctx, window);
        return M3_ERR_UNKNOWN;
    }

    window->dpi_scale = m3_win32_query_dpi_scale(window->hwnd);

    rc = m3_object_header_init(&window->header, M3_WIN32_TYPE_WINDOW, 0, &g_m3_win32_window_vtable);
    if (rc != M3_OK) {
        m3_win32_window_cleanup_unregistered(backend, window);
        return rc;
    }

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &window->header);
    if (rc != M3_OK) {
        m3_win32_window_cleanup_unregistered(backend, window);
        return rc;
    }

    *out_window = window->header.handle;
    return M3_OK;
}

static int m3_win32_ws_destroy_window(void *ws, M3Handle window)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
    M3_WIN32_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_win32_ws_show_window(void *ws, M3Handle window)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
    M3_WIN32_RETURN_IF_ERROR(rc);

    ShowWindow(resolved->hwnd, SW_SHOW);
    resolved->visible = M3_TRUE;
    return M3_OK;
}

static int m3_win32_ws_hide_window(void *ws, M3Handle window)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
    M3_WIN32_RETURN_IF_ERROR(rc);

    ShowWindow(resolved->hwnd, SW_HIDE);
    resolved->visible = M3_FALSE;
    return M3_OK;
}

static int m3_win32_ws_set_window_title(void *ws, M3Handle window, const char *utf8_title)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    wchar_t *wide;
    int rc;

    if (ws == NULL || utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_utf8_to_wide_alloc(backend, utf8_title, -1, &wide, NULL);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (!SetWindowTextW(resolved->hwnd, wide)) {
        backend->allocator.free(backend->allocator.ctx, wide);
        return M3_ERR_UNKNOWN;
    }

    backend->allocator.free(backend->allocator.ctx, wide);
    return M3_OK;
}

static int m3_win32_ws_set_window_size(void *ws, M3Handle window, m3_i32 width, m3_i32 height)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    RECT rect;
    DWORD style;
    DWORD ex_style;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
    M3_WIN32_RETURN_IF_ERROR(rc);

    style = (DWORD)GetWindowLongPtr(resolved->hwnd, GWL_STYLE);
    ex_style = (DWORD)GetWindowLongPtr(resolved->hwnd, GWL_EXSTYLE);

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    if (!AdjustWindowRectEx(&rect, style, FALSE, ex_style)) {
        return M3_ERR_UNKNOWN;
    }

    if (!SetWindowPos(resolved->hwnd, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                      SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE)) {
        return M3_ERR_UNKNOWN;
    }

    resolved->width = width;
    resolved->height = height;
    return M3_OK;
}

static int m3_win32_ws_get_window_size(void *ws, M3Handle window, m3_i32 *out_width, m3_i32 *out_height)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    RECT rect;
    int rc;

    if (ws == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (!GetClientRect(resolved->hwnd, &rect)) {
        return M3_ERR_UNKNOWN;
    }

    *out_width = (m3_i32)(rect.right - rect.left);
    *out_height = (m3_i32)(rect.bottom - rect.top);
    resolved->width = *out_width;
    resolved->height = *out_height;
    return M3_OK;
}

static int m3_win32_ws_set_window_dpi_scale(void *ws, M3Handle window, M3Scalar scale)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (scale <= 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
    M3_WIN32_RETURN_IF_ERROR(rc);

    resolved->dpi_scale_override = scale;
    resolved->has_scale_override = M3_TRUE;
    return M3_OK;
}

static int m3_win32_ws_get_window_dpi_scale(void *ws, M3Handle window, M3Scalar *out_scale)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (ws == NULL || out_scale == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (resolved->has_scale_override) {
        *out_scale = resolved->dpi_scale_override;
    } else {
        *out_scale = m3_win32_query_dpi_scale(resolved->hwnd);
        resolved->dpi_scale = *out_scale;
    }
    return M3_OK;
}

static int m3_win32_ws_set_clipboard_text(void *ws, const char *utf8_text)
{
    struct M3Win32Backend *backend;
    m3_usize length;
    wchar_t *wide;
    int rc;
    HGLOBAL mem;
    void *dest;
    m3_usize bytes;

    if (ws == NULL || utf8_text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
    M3_WIN32_RETURN_IF_ERROR(rc);

    length = 0;
    while (utf8_text[length] != '\0') {
        length += 1u;
    }
    if (length > backend->clipboard_limit) {
        return M3_ERR_RANGE;
    }

    rc = m3_win32_utf8_to_wide_alloc(backend, utf8_text, -1, &wide, NULL);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (!OpenClipboard(NULL)) {
        backend->allocator.free(backend->allocator.ctx, wide);
        return M3_ERR_STATE;
    }

    EmptyClipboard();

    bytes = (m3_usize)(wcslen(wide) + 1u) * sizeof(wchar_t);
    mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (mem == NULL) {
        CloseClipboard();
        backend->allocator.free(backend->allocator.ctx, wide);
        return M3_ERR_OUT_OF_MEMORY;
    }

    dest = GlobalLock(mem);
    if (dest == NULL) {
        GlobalFree(mem);
        CloseClipboard();
        backend->allocator.free(backend->allocator.ctx, wide);
        return M3_ERR_UNKNOWN;
    }
    memcpy(dest, wide, bytes);
    GlobalUnlock(mem);

    if (SetClipboardData(CF_UNICODETEXT, mem) == NULL) {
        GlobalFree(mem);
        CloseClipboard();
        backend->allocator.free(backend->allocator.ctx, wide);
        return M3_ERR_UNKNOWN;
    }

    CloseClipboard();
    backend->allocator.free(backend->allocator.ctx, wide);
    return M3_OK;
}

static int m3_win32_ws_get_clipboard_text(void *ws, char *buffer, m3_usize buffer_size, m3_usize *out_length)
{
    struct M3Win32Backend *backend;
    HANDLE mem;
    wchar_t *wide;
    int rc;

    if (ws == NULL || buffer == NULL || out_length == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (!OpenClipboard(NULL)) {
        return M3_ERR_STATE;
    }

    mem = GetClipboardData(CF_UNICODETEXT);
    if (mem == NULL) {
        CloseClipboard();
        buffer[0] = '\0';
        *out_length = 0;
        return M3_OK;
    }

    wide = (wchar_t *)GlobalLock(mem);
    if (wide == NULL) {
        CloseClipboard();
        return M3_ERR_UNKNOWN;
    }

    rc = m3_win32_wide_to_utf8(wide, -1, buffer, buffer_size, out_length);
    GlobalUnlock(mem);
    CloseClipboard();
    return rc;
}

static int m3_win32_ws_poll_event(void *ws, M3InputEvent *out_event, M3Bool *out_has_event)
{
    struct M3Win32Backend *backend;
    int rc;

    if (ws == NULL || out_event == NULL || out_has_event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (backend->event_queue.count == 0u) {
        rc = m3_win32_pump_messages(backend);
        M3_WIN32_RETURN_IF_ERROR(rc);
    }

    return m3_win32_event_queue_pop(&backend->event_queue, out_event, out_has_event);
}

static int m3_win32_ws_pump_events(void *ws)
{
    struct M3Win32Backend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
    M3_WIN32_RETURN_IF_ERROR(rc);

    return m3_win32_pump_messages(backend);
}

static int m3_win32_ws_get_time_ms(void *ws, m3_u32 *out_time_ms)
{
    struct M3Win32Backend *backend;
    int rc;

    if (ws == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)ws;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
    M3_WIN32_RETURN_IF_ERROR(rc);

    backend->time_ms = m3_win32_get_time_ms();
    *out_time_ms = backend->time_ms;
    return M3_OK;
}

static const M3WSVTable g_m3_win32_ws_vtable = {
    m3_win32_ws_init,
    m3_win32_ws_shutdown,
    m3_win32_ws_create_window,
    m3_win32_ws_destroy_window,
    m3_win32_ws_show_window,
    m3_win32_ws_hide_window,
    m3_win32_ws_set_window_title,
    m3_win32_ws_set_window_size,
    m3_win32_ws_get_window_size,
    m3_win32_ws_set_window_dpi_scale,
    m3_win32_ws_get_window_dpi_scale,
    m3_win32_ws_set_clipboard_text,
    m3_win32_ws_get_clipboard_text,
    m3_win32_ws_poll_event,
    m3_win32_ws_pump_events,
    m3_win32_ws_get_time_ms
};

static int m3_win32_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width, m3_i32 height, M3Scalar dpi_scale)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (dpi_scale <= 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)gfx;
    if (backend->active_window != NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_window_ensure_backbuffer(resolved, width, height);
    M3_WIN32_RETURN_IF_ERROR(rc);

    while (backend->clip_depth > 0u) {
        RestoreDC(resolved->mem_dc, -1);
        backend->clip_depth -= 1u;
    }
    SelectClipRgn(resolved->mem_dc, NULL);

    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    backend->active_window = resolved;
    backend->has_transform = M3_FALSE;
    backend->clip_depth = 0u;

    rc = m3_win32_apply_transform(resolved, &backend->transform, backend->has_transform);
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_win32_gfx_end_frame(void *gfx, M3Handle window)
{
    struct M3Win32Backend *backend;
    M3Win32Window *resolved;
    HDC hdc;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    rc = m3_win32_backend_resolve(backend, window, M3_WIN32_TYPE_WINDOW, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (backend->active_window != resolved) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (resolved->mem_dc != NULL && resolved->hwnd != NULL) {
        hdc = GetDC(resolved->hwnd);
        if (hdc != NULL) {
            BitBlt(hdc, 0, 0, resolved->width, resolved->height, resolved->mem_dc, 0, 0, SRCCOPY);
            ReleaseDC(resolved->hwnd, hdc);
        }
    }

    while (backend->clip_depth > 0u) {
        RestoreDC(resolved->mem_dc, -1);
        backend->clip_depth -= 1u;
    }

    backend->active_window = NULL;
    return M3_OK;
}

static int m3_win32_gfx_clear(void *gfx, M3Color color)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    HBRUSH brush;
    RECT rect;
    COLORREF ref;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
    M3_WIN32_RETURN_IF_ERROR(rc);

    ref = m3_win32_color_to_colorref(color);
    brush = CreateSolidBrush(ref);
    if (brush == NULL) {
        return M3_ERR_UNKNOWN;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = window->width;
    rect.bottom = window->height;
    FillRect(window->mem_dc, &rect, brush);
    DeleteObject(brush);
    return M3_OK;
}

static int m3_win32_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    RECT r;
    COLORREF ref;
    HBRUSH brush;
    HPEN pen;
    HPEN old_pen;
    HBRUSH old_brush;
    int radius;
    int rc;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (corner_radius < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (rect->width == 0.0f || rect->height == 0.0f) {
        return M3_OK;
    }

    r.left = (int)rect->x;
    r.top = (int)rect->y;
    r.right = (int)(rect->x + rect->width);
    r.bottom = (int)(rect->y + rect->height);

    ref = m3_win32_color_to_colorref(color);
    brush = CreateSolidBrush(ref);
    if (brush == NULL) {
        return M3_ERR_UNKNOWN;
    }

    radius = (int)(corner_radius + 0.5f);
    if (radius <= 0) {
        FillRect(window->mem_dc, &r, brush);
        DeleteObject(brush);
        return M3_OK;
    }

    pen = CreatePen(PS_NULL, 0, ref);
    if (pen == NULL) {
        DeleteObject(brush);
        return M3_ERR_UNKNOWN;
    }

    old_pen = (HPEN)SelectObject(window->mem_dc, pen);
    old_brush = (HBRUSH)SelectObject(window->mem_dc, brush);
    RoundRect(window->mem_dc, r.left, r.top, r.right, r.bottom, radius * 2, radius * 2);
    SelectObject(window->mem_dc, old_pen);
    SelectObject(window->mem_dc, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
    return M3_OK;
}

static int m3_win32_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1, M3Scalar y1, M3Color color, M3Scalar thickness)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    COLORREF ref;
    HPEN pen;
    HPEN old_pen;
    int width;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (thickness < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
    M3_WIN32_RETURN_IF_ERROR(rc);

    width = (int)(thickness + 0.5f);
    if (width <= 0) {
        width = 1;
    }

    ref = m3_win32_color_to_colorref(color);
    pen = CreatePen(PS_SOLID, width, ref);
    if (pen == NULL) {
        return M3_ERR_UNKNOWN;
    }
    old_pen = (HPEN)SelectObject(window->mem_dc, pen);
    MoveToEx(window->mem_dc, (int)x0, (int)y0, NULL);
    LineTo(window->mem_dc, (int)x1, (int)y1);
    SelectObject(window->mem_dc, old_pen);
    DeleteObject(pen);
    return M3_OK;
}

static int m3_win32_gfx_push_clip(void *gfx, const M3Rect *rect)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    int rc;
    int save_id;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (backend->clip_depth >= M3_WIN32_CLIP_STACK_CAPACITY) {
        return M3_ERR_OVERFLOW;
    }

    save_id = SaveDC(window->mem_dc);
    if (save_id == 0) {
        return M3_ERR_UNKNOWN;
    }
    IntersectClipRect(window->mem_dc, (int)rect->x, (int)rect->y,
                      (int)(rect->x + rect->width), (int)(rect->y + rect->height));
    backend->clip_depth += 1u;
    return M3_OK;
}

static int m3_win32_gfx_pop_clip(void *gfx)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (backend->clip_depth == 0u) {
        return M3_ERR_STATE;
    }

    if (!RestoreDC(window->mem_dc, -1)) {
        return M3_ERR_UNKNOWN;
    }
    backend->clip_depth -= 1u;
    rc = m3_win32_apply_transform(window, &backend->transform, backend->has_transform);
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_win32_gfx_set_transform(void *gfx, const M3Mat3 *transform)
{
    struct M3Win32Backend *backend;
    M3Win32Window *window;
    int rc;

    if (gfx == NULL || transform == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
    M3_WIN32_RETURN_IF_ERROR(rc);

    backend->transform = *transform;
    backend->has_transform = M3_TRUE;
    return m3_win32_apply_transform(window, transform, M3_TRUE);
}

static int m3_win32_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height, m3_u32 format, const void *pixels, m3_usize size,
                                       M3Handle *out_texture)
{
    struct M3Win32Backend *backend;
    M3Win32Texture *texture;
    BITMAPINFO bmi;
    void *dib_pixels;
    HBITMAP dib;
    HDC mem_dc;
    m3_usize required;
    m3_usize row_bytes;
    m3_usize data_bytes;
    m3_u32 bpp;
    int rc;

    if (gfx == NULL || out_texture == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (format != M3_TEX_FORMAT_RGBA8 && format != M3_TEX_FORMAT_BGRA8 && format != M3_TEX_FORMAT_A8) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (pixels == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    out_texture->id = 0u;
    out_texture->generation = 0u;

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
    M3_WIN32_RETURN_IF_ERROR(rc);

    bpp = (format == M3_TEX_FORMAT_A8) ? 1u : 4u;
    rc = m3_win32_mul_usize((m3_usize)width, (m3_usize)height, &required);
    M3_WIN32_RETURN_IF_ERROR(rc);
    rc = m3_win32_mul_usize(required, (m3_usize)bpp, &data_bytes);
    M3_WIN32_RETURN_IF_ERROR(rc);
    if (pixels != NULL && size < data_bytes) {
        return M3_ERR_RANGE;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3Win32Texture), (void **)&texture);
    M3_WIN32_RETURN_IF_ERROR(rc);
    memset(texture, 0, sizeof(*texture));
    texture->backend = backend;
    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->bytes_per_pixel = bpp;
    texture->stride = width * 4;

    mem_dc = CreateCompatibleDC(NULL);
    if (mem_dc == NULL) {
        backend->allocator.free(backend->allocator.ctx, texture);
        return M3_ERR_UNKNOWN;
    }

    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    dib = CreateDIBSection(mem_dc, &bmi, DIB_RGB_COLORS, &dib_pixels, NULL, 0);
    if (dib == NULL || dib_pixels == NULL) {
        DeleteDC(mem_dc);
        backend->allocator.free(backend->allocator.ctx, texture);
        return M3_ERR_UNKNOWN;
    }

    SelectObject(mem_dc, dib);
    texture->mem_dc = mem_dc;
    texture->dib = dib;
    texture->pixels = dib_pixels;

    if (pixels != NULL && size != 0) {
        rc = m3_win32_mul_usize((m3_usize)width, (m3_usize)bpp, &row_bytes);
        if (rc != M3_OK) {
            m3_win32_texture_cleanup_unregistered(backend, texture);
            return rc;
        }
        m3_win32_texture_copy_pixels((m3_u8 *)texture->pixels, texture->stride, (const m3_u8 *)pixels, (m3_i32)row_bytes,
                                     width, height, format);
    }

    rc = m3_object_header_init(&texture->header, M3_WIN32_TYPE_TEXTURE, 0, &g_m3_win32_texture_vtable);
    if (rc != M3_OK) {
        m3_win32_texture_cleanup_unregistered(backend, texture);
        return rc;
    }

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &texture->header);
    if (rc != M3_OK) {
        m3_win32_texture_cleanup_unregistered(backend, texture);
        return rc;
    }

    *out_texture = texture->header.handle;
    return M3_OK;
}

static int m3_win32_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x, m3_i32 y, m3_i32 width, m3_i32 height, const void *pixels,
                                       m3_usize size)
{
    struct M3Win32Backend *backend;
    M3Win32Texture *resolved;
    m3_usize required;
    m3_usize row_bytes;
    m3_u8 *dst;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (x < 0 || y < 0 || width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (pixels == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    rc = m3_win32_backend_resolve(backend, texture, M3_WIN32_TYPE_TEXTURE, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (x + width > resolved->width || y + height > resolved->height) {
        return M3_ERR_RANGE;
    }

    rc = m3_win32_mul_usize((m3_usize)width, (m3_usize)height, &required);
    M3_WIN32_RETURN_IF_ERROR(rc);
    rc = m3_win32_mul_usize(required, (m3_usize)resolved->bytes_per_pixel, &required);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (pixels != NULL && size < required) {
        return M3_ERR_RANGE;
    }
    if (pixels == NULL || size == 0) {
        return M3_OK;
    }

    rc = m3_win32_mul_usize((m3_usize)width, (m3_usize)resolved->bytes_per_pixel, &row_bytes);
    M3_WIN32_RETURN_IF_ERROR(rc);

    dst = (m3_u8 *)resolved->pixels + (m3_isize)y * (m3_isize)resolved->stride + (m3_isize)x * 4;
    m3_win32_texture_copy_pixels(dst, resolved->stride, (const m3_u8 *)pixels, (m3_i32)row_bytes, width, height, resolved->format);
    return M3_OK;
}

static int m3_win32_gfx_destroy_texture(void *gfx, M3Handle texture)
{
    struct M3Win32Backend *backend;
    M3Win32Texture *resolved;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)gfx;
    rc = m3_win32_backend_resolve(backend, texture, M3_WIN32_TYPE_TEXTURE, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
    M3_WIN32_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_win32_gfx_draw_texture(void *gfx, M3Handle texture, const M3Rect *src, const M3Rect *dst, M3Scalar opacity)
{
    struct M3Win32Backend *backend;
    M3Win32Texture *resolved;
    M3Win32Window *window;
    BLENDFUNCTION blend;
    int rc;
    int src_x;
    int src_y;
    int src_w;
    int src_h;
    int dst_x;
    int dst_y;
    int dst_w;
    int dst_h;
    m3_u8 alpha;

    if (gfx == NULL || src == NULL || dst == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (opacity < 0.0f || opacity > 1.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)gfx;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_resolve(backend, texture, M3_WIN32_TYPE_TEXTURE, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
    M3_WIN32_RETURN_IF_ERROR(rc);

    src_x = (int)src->x;
    src_y = (int)src->y;
    src_w = (int)src->width;
    src_h = (int)src->height;
    dst_x = (int)dst->x;
    dst_y = (int)dst->y;
    dst_w = (int)dst->width;
    dst_h = (int)dst->height;

    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        return M3_OK;
    }
    if (src_x < 0 || src_y < 0 || src_x + src_w > resolved->width || src_y + src_h > resolved->height) {
        return M3_ERR_RANGE;
    }
    if (opacity <= 0.0f) {
        return M3_OK;
    }

    if (backend->alpha_blend != NULL) {
        alpha = (m3_u8)(opacity * 255.0f + 0.5f);
        blend.BlendOp = AC_SRC_OVER;
        blend.BlendFlags = 0;
        blend.SourceConstantAlpha = (BYTE)alpha;
        blend.AlphaFormat = AC_SRC_ALPHA;

        if (!backend->alpha_blend(window->mem_dc, dst_x, dst_y, dst_w, dst_h, resolved->mem_dc, src_x, src_y, src_w, src_h,
                                  blend)) {
            return M3_ERR_UNKNOWN;
        }
    } else {
        int old_mode;
        if (opacity < 1.0f) {
            return M3_ERR_UNSUPPORTED;
        }
        old_mode = SetStretchBltMode(window->mem_dc, HALFTONE);
        if (!StretchBlt(window->mem_dc, dst_x, dst_y, dst_w, dst_h, resolved->mem_dc, src_x, src_y, src_w, src_h, SRCCOPY)) {
            if (old_mode != 0) {
                SetStretchBltMode(window->mem_dc, old_mode);
            }
            return M3_ERR_UNKNOWN;
        }
        if (old_mode != 0) {
            SetStretchBltMode(window->mem_dc, old_mode);
        }
    }
    return M3_OK;
}

static const M3GfxVTable g_m3_win32_gfx_vtable = {
    m3_win32_gfx_begin_frame,
    m3_win32_gfx_end_frame,
    m3_win32_gfx_clear,
    m3_win32_gfx_draw_rect,
    m3_win32_gfx_draw_line,
    m3_win32_gfx_push_clip,
    m3_win32_gfx_pop_clip,
    m3_win32_gfx_set_transform,
    m3_win32_gfx_create_texture,
    m3_win32_gfx_update_texture,
    m3_win32_gfx_destroy_texture,
    m3_win32_gfx_draw_texture
};

static int m3_win32_text_create_font(void *text, const char *utf8_family, m3_i32 size_px, m3_i32 weight, M3Bool italic, M3Handle *out_font)
{
    struct M3Win32Backend *backend;
    M3Win32Font *font;
    wchar_t *family;
    int rc;
    int height;
    int font_weight;
    HFONT hfont;

    if (text == NULL || out_font == NULL || utf8_family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)text;
    out_font->id = 0u;
    out_font->generation = 0u;

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_utf8_to_wide_alloc(backend, utf8_family, -1, &family, NULL);
    M3_WIN32_RETURN_IF_ERROR(rc);

    height = -size_px;
    font_weight = weight;
    hfont = CreateFontW(height, 0, 0, 0, font_weight, italic ? TRUE : FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family);

    backend->allocator.free(backend->allocator.ctx, family);

    if (hfont == NULL) {
        return M3_ERR_UNKNOWN;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3Win32Font), (void **)&font);
    M3_WIN32_RETURN_IF_ERROR_CLEANUP(rc, DeleteObject(hfont));

    memset(font, 0, sizeof(*font));
    font->backend = backend;
    font->font = hfont;
    font->size_px = size_px;
    font->weight = weight;
    font->italic = italic ? M3_TRUE : M3_FALSE;

    rc = m3_object_header_init(&font->header, M3_WIN32_TYPE_FONT, 0, &g_m3_win32_font_vtable);
    if (rc != M3_OK) {
        m3_win32_font_cleanup_unregistered(backend, font);
        return rc;
    }

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &font->header);
    if (rc != M3_OK) {
        m3_win32_font_cleanup_unregistered(backend, font);
        return rc;
    }

    *out_font = font->header.handle;
    return M3_OK;
}

static int m3_win32_text_destroy_font(void *text, M3Handle font)
{
    struct M3Win32Backend *backend;
    M3Win32Font *resolved;
    int rc;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)text;
    rc = m3_win32_backend_resolve(backend, font, M3_WIN32_TYPE_FONT, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
    M3_WIN32_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_win32_text_measure_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar *out_width,
                                      M3Scalar *out_height, M3Scalar *out_baseline)
{
    struct M3Win32Backend *backend;
    M3Win32Font *resolved;
    wchar_t *wide;
    int wide_len;
    int rc;
    HDC hdc;
    HFONT old_font;
    SIZE size;
    TEXTMETRIC tm;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_len > (m3_usize)INT_MAX) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)text;
    rc = m3_win32_backend_resolve(backend, font, M3_WIN32_TYPE_FONT, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
    M3_WIN32_RETURN_IF_ERROR(rc);

    hdc = GetDC(NULL);
    if (hdc == NULL) {
        return M3_ERR_UNKNOWN;
    }

    old_font = (HFONT)SelectObject(hdc, resolved->font);
    if (!GetTextMetrics(hdc, &tm)) {
        SelectObject(hdc, old_font);
        ReleaseDC(NULL, hdc);
        return M3_ERR_UNKNOWN;
    }

    if (utf8_len == 0) {
        size.cx = 0;
        size.cy = tm.tmHeight;
    } else {
        wide = NULL;
        wide_len = 0;
        rc = m3_win32_utf8_to_wide_alloc(backend, utf8, (int)utf8_len, &wide, &wide_len);
        if (rc != M3_OK) {
            SelectObject(hdc, old_font);
            ReleaseDC(NULL, hdc);
            return rc;
        }
        if (!GetTextExtentPoint32W(hdc, wide, wide_len, &size)) {
            backend->allocator.free(backend->allocator.ctx, wide);
            SelectObject(hdc, old_font);
            ReleaseDC(NULL, hdc);
            return M3_ERR_UNKNOWN;
        }
        backend->allocator.free(backend->allocator.ctx, wide);
    }

    SelectObject(hdc, old_font);
    ReleaseDC(NULL, hdc);

    *out_width = (M3Scalar)size.cx;
    *out_height = (M3Scalar)size.cy;
    *out_baseline = (M3Scalar)tm.tmAscent;
    return M3_OK;
}

static int m3_win32_text_draw_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar x, M3Scalar y, M3Color color)
{
    struct M3Win32Backend *backend;
    M3Win32Font *resolved;
    M3Win32Window *window;
    wchar_t *wide;
    int wide_len;
    int rc;
    HFONT old_font;
    COLORREF ref;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8_len > (m3_usize)INT_MAX) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3Win32Backend *)text;
    window = backend->active_window;
    if (window == NULL || window->mem_dc == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_win32_backend_resolve(backend, font, M3_WIN32_TYPE_FONT, (void **)&resolved);
    M3_WIN32_RETURN_IF_ERROR(rc);

    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (utf8_len == 0) {
        return M3_OK;
    }

    wide = NULL;
    wide_len = 0;
    rc = m3_win32_utf8_to_wide_alloc(backend, utf8, (int)utf8_len, &wide, &wide_len);
    M3_WIN32_RETURN_IF_ERROR(rc);

    old_font = (HFONT)SelectObject(window->mem_dc, resolved->font);
    SetBkMode(window->mem_dc, TRANSPARENT);
    ref = m3_win32_color_to_colorref(color);
    SetTextColor(window->mem_dc, ref);

    TextOutW(window->mem_dc, (int)x, (int)y, wide, wide_len);

    SelectObject(window->mem_dc, old_font);
    backend->allocator.free(backend->allocator.ctx, wide);
    return M3_OK;
}

static const M3TextVTable g_m3_win32_text_vtable = {
    m3_win32_text_create_font,
    m3_win32_text_destroy_font,
    m3_win32_text_measure_text,
    m3_win32_text_draw_text
};

static int m3_win32_io_read_file(void *io, const char *utf8_path, void *buffer, m3_usize buffer_size, m3_usize *out_read)
{
    struct M3Win32Backend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_read == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (buffer == NULL && buffer_size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_read = 0;
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_io_read_file_alloc(void *io, const char *utf8_path, const M3Allocator *allocator, void **out_data, m3_usize *out_size)
{
    struct M3Win32Backend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || allocator == NULL || out_data == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_data = NULL;
    *out_size = 0;
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_io_write_file(void *io, const char *utf8_path, const void *data, m3_usize size, M3Bool overwrite)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(overwrite);

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (data == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_io_file_exists(void *io, const char *utf8_path, M3Bool *out_exists)
{
    struct M3Win32Backend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_exists == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_exists = M3_FALSE;
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_io_delete_file(void *io, const char *utf8_path)
{
    struct M3Win32Backend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_io_stat_file(void *io, const char *utf8_path, M3FileInfo *out_info)
{
    struct M3Win32Backend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_info == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)io;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
    M3_WIN32_RETURN_IF_ERROR(rc);

    memset(out_info, 0, sizeof(*out_info));
    return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_win32_io_vtable = {
    m3_win32_io_read_file,
    m3_win32_io_read_file_alloc,
    m3_win32_io_write_file,
    m3_win32_io_file_exists,
    m3_win32_io_delete_file,
    m3_win32_io_stat_file
};

static int m3_win32_sensors_is_available(void *sensors, m3_u32 type, M3Bool *out_available)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)sensors;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_available = M3_FALSE;
    return M3_OK;
}

static int m3_win32_sensors_start(void *sensors, m3_u32 type)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)sensors;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_sensors_stop(void *sensors, m3_u32 type)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)sensors;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_sensors_read(void *sensors, m3_u32 type, M3SensorReading *out_reading, M3Bool *out_has_reading)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)sensors;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
    M3_WIN32_RETURN_IF_ERROR(rc);

    memset(out_reading, 0, sizeof(*out_reading));
    *out_has_reading = M3_FALSE;
    return M3_OK;
}

static const M3SensorsVTable g_m3_win32_sensors_vtable = {
    m3_win32_sensors_is_available,
    m3_win32_sensors_start,
    m3_win32_sensors_stop,
    m3_win32_sensors_read
};

static int m3_win32_camera_open(void *camera, m3_u32 camera_id)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(camera_id);

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)camera;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_camera_open_with_config(void *camera, const M3CameraConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return m3_win32_camera_open(camera, config->camera_id);
}

static int m3_win32_camera_close(void *camera)
{
    struct M3Win32Backend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)camera;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_camera_start(void *camera)
{
    struct M3Win32Backend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)camera;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_camera_stop(void *camera)
{
    struct M3Win32Backend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)camera;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_camera_read_frame(void *camera, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    struct M3Win32Backend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)camera;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
    M3_WIN32_RETURN_IF_ERROR(rc);

    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = M3_FALSE;
    return M3_OK;
}

static const M3CameraVTable g_m3_win32_camera_vtable = {
    m3_win32_camera_open,
    m3_win32_camera_open_with_config,
    m3_win32_camera_close,
    m3_win32_camera_start,
    m3_win32_camera_stop,
    m3_win32_camera_read_frame
};

static int m3_win32_network_request(void *net, const M3NetworkRequest *request, const M3Allocator *allocator, M3NetworkResponse *out_response)
{
    struct M3Win32Backend *backend;
    URL_COMPONENTS components;
    HINTERNET session;
    HINTERNET connection;
    HINTERNET request_handle;
    wchar_t *url_wide;
    wchar_t *method_wide;
    wchar_t *headers_wide;
    wchar_t *host_wide;
    wchar_t *object_wide;
    void *body;
    void *new_body;
    m3_usize total_size;
    m3_usize capacity;
    m3_usize path_len;
    m3_usize extra_len;
    m3_usize object_len;
    m3_usize alloc_size;
    DWORD flags;
    DWORD status_code;
    DWORD status_code_size;
    DWORD bytes_available;
    DWORD bytes_read;
    DWORD body_len;
    DWORD last_error;
    INTERNET_PORT port;
    int rc;
    int log_rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)net;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (request->method == NULL || request->url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->timeout_ms > 0u && request->timeout_ms > (m3_u32)INT_MAX) {
        return M3_ERR_RANGE;
    }
    if (request->body_size > (m3_usize)~(DWORD)0) {
        return M3_ERR_RANGE;
    }

    session = NULL;
    connection = NULL;
    request_handle = NULL;
    url_wide = NULL;
    method_wide = NULL;
    headers_wide = NULL;
    host_wide = NULL;
    object_wide = NULL;
    body = NULL;
    total_size = 0;
    capacity = 0;

    memset(out_response, 0, sizeof(*out_response));

    rc = m3_win32_utf8_to_wide_alloc(backend, request->url, -1, &url_wide, NULL);
    M3_WIN32_RETURN_IF_ERROR(rc);

    memset(&components, 0, sizeof(components));
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = (DWORD)-1;
    components.dwHostNameLength = (DWORD)-1;
    components.dwUrlPathLength = (DWORD)-1;
    components.dwExtraInfoLength = (DWORD)-1;

    if (!WinHttpCrackUrl(url_wide, 0, 0, &components)) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.crack_url");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    if (components.nScheme != INTERNET_SCHEME_HTTP && components.nScheme != INTERNET_SCHEME_HTTPS) {
        rc = M3_ERR_UNSUPPORTED;
        goto cleanup;
    }

    if (components.dwHostNameLength == 0 || components.lpszHostName == NULL) {
        rc = M3_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }

    rc = m3_win32_network_copy_wide_range(backend, components.lpszHostName, components.dwHostNameLength, &host_wide);
    if (rc != M3_OK) {
        goto cleanup;
    }

    path_len = (m3_usize)components.dwUrlPathLength;
    extra_len = (m3_usize)components.dwExtraInfoLength;
    if (path_len > 0 && components.lpszUrlPath == NULL) {
        rc = M3_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (extra_len > 0 && components.lpszExtraInfo == NULL) {
        rc = M3_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }

    if (path_len == 0 && extra_len == 0) {
        object_len = 1u;
    } else if (path_len == 0) {
        rc = m3_win32_add_usize(1u, extra_len, &object_len);
        if (rc != M3_OK) {
            goto cleanup;
        }
    } else {
        rc = m3_win32_add_usize(path_len, extra_len, &object_len);
        if (rc != M3_OK) {
            goto cleanup;
        }
    }

    rc = m3_win32_add_usize(object_len, 1u, &alloc_size);
    if (rc != M3_OK) {
        goto cleanup;
    }
    rc = m3_win32_mul_usize(alloc_size, (m3_usize)sizeof(wchar_t), &alloc_size);
    if (rc != M3_OK) {
        goto cleanup;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, alloc_size, (void **)&object_wide);
    if (rc != M3_OK) {
        goto cleanup;
    }

    if (path_len == 0 && extra_len == 0) {
        object_wide[0] = L'/';
        object_wide[1] = L'\0';
    } else if (path_len == 0) {
        object_wide[0] = L'/';
        if (extra_len > 0) {
            memcpy(object_wide + 1, components.lpszExtraInfo, (size_t)extra_len * sizeof(wchar_t));
        }
        object_wide[object_len] = L'\0';
    } else {
        if (path_len > 0) {
            memcpy(object_wide, components.lpszUrlPath, (size_t)path_len * sizeof(wchar_t));
        }
        if (extra_len > 0) {
            memcpy(object_wide + path_len, components.lpszExtraInfo, (size_t)extra_len * sizeof(wchar_t));
        }
        object_wide[object_len] = L'\0';
    }

    rc = m3_win32_utf8_to_wide_alloc(backend, request->method, -1, &method_wide, NULL);
    if (rc != M3_OK) {
        goto cleanup;
    }

    if (request->headers != NULL && request->headers[0] != '\0') {
        rc = m3_win32_utf8_to_wide_alloc(backend, request->headers, -1, &headers_wide, NULL);
        if (rc != M3_OK) {
            goto cleanup;
        }
    }

    session = WinHttpOpen(L"LibM3C/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (session == NULL) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.open_session");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    if (request->timeout_ms > 0u) {
        if (!WinHttpSetTimeouts(session, (int)request->timeout_ms, (int)request->timeout_ms,
                (int)request->timeout_ms, (int)request->timeout_ms)) {
            last_error = GetLastError();
            rc = m3_win32_network_error_from_winhttp(last_error);
            log_rc = m3_win32_backend_log_last_error(backend, "network.set_timeouts");
            if (log_rc != M3_OK && rc == M3_OK) {
                rc = log_rc;
            }
            goto cleanup;
        }
    }

    port = components.nPort;
    if (port == 0) {
        if (components.nScheme == INTERNET_SCHEME_HTTPS) {
            port = INTERNET_DEFAULT_HTTPS_PORT;
        } else {
            port = INTERNET_DEFAULT_HTTP_PORT;
        }
    }

    connection = WinHttpConnect(session, host_wide, port, 0);
    if (connection == NULL) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.connect");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    flags = 0u;
    if (components.nScheme == INTERNET_SCHEME_HTTPS) {
        flags |= WINHTTP_FLAG_SECURE;
    }

    request_handle = WinHttpOpenRequest(connection, method_wide, object_wide, NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (request_handle == NULL) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.open_request");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    body_len = (DWORD)request->body_size;
    if (!WinHttpSendRequest(request_handle,
            headers_wide != NULL ? headers_wide : WINHTTP_NO_ADDITIONAL_HEADERS,
            headers_wide != NULL ? (DWORD)-1 : 0,
            request->body_size > 0 ? (LPVOID)request->body : NULL,
            body_len, body_len, 0)) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.send_request");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    if (!WinHttpReceiveResponse(request_handle, NULL)) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.receive_response");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    status_code = 0u;
    status_code_size = sizeof(status_code);
    if (!WinHttpQueryHeaders(request_handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &status_code_size, WINHTTP_NO_HEADER_INDEX)) {
        last_error = GetLastError();
        rc = m3_win32_network_error_from_winhttp(last_error);
        log_rc = m3_win32_backend_log_last_error(backend, "network.query_status");
        if (log_rc != M3_OK && rc == M3_OK) {
            rc = log_rc;
        }
        goto cleanup;
    }

    for (;;) {
        bytes_available = 0;
        if (!WinHttpQueryDataAvailable(request_handle, &bytes_available)) {
            last_error = GetLastError();
            rc = m3_win32_network_error_from_winhttp(last_error);
            log_rc = m3_win32_backend_log_last_error(backend, "network.query_data_available");
            if (log_rc != M3_OK && rc == M3_OK) {
                rc = log_rc;
            }
            goto cleanup;
        }
        if (bytes_available == 0) {
            break;
        }

        rc = m3_win32_add_usize(total_size, (m3_usize)bytes_available, &alloc_size);
        if (rc != M3_OK) {
            goto cleanup;
        }

        if (alloc_size > capacity) {
            new_body = NULL;
            if (body == NULL) {
                rc = allocator->alloc(allocator->ctx, alloc_size, &new_body);
            } else {
                rc = allocator->realloc(allocator->ctx, body, alloc_size, &new_body);
            }
            if (rc != M3_OK) {
                goto cleanup;
            }
            body = new_body;
            capacity = alloc_size;
        }

        bytes_read = 0;
        if (!WinHttpReadData(request_handle, (m3_u8 *)body + total_size, bytes_available, &bytes_read)) {
            last_error = GetLastError();
            rc = m3_win32_network_error_from_winhttp(last_error);
            log_rc = m3_win32_backend_log_last_error(backend, "network.read_data");
            if (log_rc != M3_OK && rc == M3_OK) {
                rc = log_rc;
            }
            goto cleanup;
        }
        if (bytes_read == 0) {
            rc = M3_ERR_IO;
            goto cleanup;
        }
        if (bytes_read > bytes_available) {
            rc = M3_ERR_IO;
            goto cleanup;
        }

        total_size += (m3_usize)bytes_read;
    }

    out_response->status_code = (m3_u32)status_code;
    out_response->body = body;
    out_response->body_size = total_size;
    body = NULL;
    rc = M3_OK;

cleanup:
    if (request_handle != NULL) {
        WinHttpCloseHandle(request_handle);
    }
    if (connection != NULL) {
        WinHttpCloseHandle(connection);
    }
    if (session != NULL) {
        WinHttpCloseHandle(session);
    }
    if (url_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, url_wide);
    }
    if (method_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, method_wide);
    }
    if (headers_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, headers_wide);
    }
    if (host_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, host_wide);
    }
    if (object_wide != NULL) {
        backend->allocator.free(backend->allocator.ctx, object_wide);
    }
    if (body != NULL) {
        allocator->free(allocator->ctx, body);
    }

    if (rc != M3_OK) {
        memset(out_response, 0, sizeof(*out_response));
    }

    return rc;
}

static int m3_win32_network_free_response(void *net, const M3Allocator *allocator, M3NetworkResponse *response)
{
    struct M3Win32Backend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (response->body_size > 0 && response->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)net;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (response->body != NULL) {
        rc = allocator->free(allocator->ctx, (void *)response->body);
        if (rc != M3_OK) {
            return rc;
        }
    }

    response->body = NULL;
    response->body_size = 0;
    response->status_code = 0;
    return M3_OK;
}

static const M3NetworkVTable g_m3_win32_network_vtable = {
    m3_win32_network_request,
    m3_win32_network_free_response
};

static int m3_win32_tasks_thread_create(void *tasks, M3ThreadFn entry, void *user, M3Handle *out_thread)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(entry);
    M3_UNUSED(user);

    if (tasks == NULL || out_thread == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_thread_join(void *tasks, M3Handle thread)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(thread);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_mutex_create(void *tasks, M3Handle *out_mutex)
{
    struct M3Win32Backend *backend;
    int rc;

    if (tasks == NULL || out_mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_mutex_destroy(void *tasks, M3Handle mutex)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_mutex_lock(void *tasks, M3Handle mutex)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_mutex_unlock(void *tasks, M3Handle mutex)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
    M3_WIN32_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_win32_tasks_sleep_ms(void *tasks, m3_u32 ms)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(ms);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
    M3_WIN32_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return M3_OK;
}

static int m3_win32_tasks_post(void *tasks, M3TaskFn fn, void *user)
{
    struct M3Win32Backend *backend;
    int rc;

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
    M3_WIN32_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static int m3_win32_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user, m3_u32 delay_ms)
{
    struct M3Win32Backend *backend;
    int rc;

    M3_UNUSED(delay_ms);

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)tasks;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
    M3_WIN32_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static const M3TasksVTable g_m3_win32_tasks_vtable = {
    m3_win32_tasks_thread_create,
    m3_win32_tasks_thread_join,
    m3_win32_tasks_mutex_create,
    m3_win32_tasks_mutex_destroy,
    m3_win32_tasks_mutex_lock,
    m3_win32_tasks_mutex_unlock,
    m3_win32_tasks_sleep_ms,
    m3_win32_tasks_post,
    m3_win32_tasks_post_delayed
};

static int m3_win32_env_get_io(void *env, M3IO *out_io)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_io == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_io = backend->io;
    return M3_OK;
}

static int m3_win32_env_get_sensors(void *env, M3Sensors *out_sensors)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_sensors = backend->sensors;
    return M3_OK;
}

static int m3_win32_env_get_camera(void *env, M3Camera *out_camera)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_camera = backend->camera;
    return M3_OK;
}

static int m3_win32_env_get_network(void *env, M3Network *out_network)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_network == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_network = backend->network;
    return M3_OK;
}

static int m3_win32_env_get_tasks(void *env, M3Tasks *out_tasks)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
    M3_WIN32_RETURN_IF_ERROR(rc);

    *out_tasks = backend->tasks;
    return M3_OK;
}

static int m3_win32_env_get_time_ms(void *env, m3_u32 *out_time_ms)
{
    struct M3Win32Backend *backend;
    int rc;

    if (env == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3Win32Backend *)env;
    rc = m3_win32_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
    M3_WIN32_RETURN_IF_ERROR(rc);

    backend->time_ms = m3_win32_get_time_ms();
    *out_time_ms = backend->time_ms;
    return M3_OK;
}

static const M3EnvVTable g_m3_win32_env_vtable = {
    m3_win32_env_get_io,
    m3_win32_env_get_sensors,
    m3_win32_env_get_camera,
    m3_win32_env_get_network,
    m3_win32_env_get_tasks,
    m3_win32_env_get_time_ms
};

int M3_CALL m3_win32_backend_create(const M3Win32BackendConfig *config, M3Win32Backend **out_backend)
{
    M3Win32BackendConfig local_config;
    M3Allocator allocator;
    struct M3Win32Backend *backend;
    WNDCLASSEXW wc;
    ATOM atom;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_win32_backend_config_init(&local_config);
        M3_WIN32_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_win32_backend_validate_config(config);
    M3_WIN32_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        M3_WIN32_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(struct M3Win32Backend), (void **)&backend);
    M3_WIN32_RETURN_IF_ERROR(rc);
    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;
    backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
    backend->inline_tasks = config->inline_tasks ? M3_TRUE : M3_FALSE;
    backend->clipboard_limit = config->clipboard_limit;
    backend->instance = GetModuleHandle(NULL);

    if (backend->log_enabled) {
        rc = m3_log_init(&allocator);
        if (rc != M3_OK && rc != M3_ERR_STATE) {
            allocator.free(allocator.ctx, backend);
            return rc;
        }
        backend->log_owner = (rc == M3_OK) ? M3_TRUE : M3_FALSE;
    }

    rc = m3_handle_system_default_create(config->handle_capacity, &allocator, &backend->handles);
    if (rc != M3_OK) {
        if (backend->log_owner) {
            m3_log_shutdown();
        }
        allocator.free(allocator.ctx, backend);
        return rc;
    }

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = m3_win32_wndproc;
    wc.hInstance = backend->instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = g_m3_win32_window_class_name;

    atom = RegisterClassExW(&wc);
    if (atom == 0) {
        DWORD err = GetLastError();
        if (err != ERROR_CLASS_ALREADY_EXISTS) {
            m3_handle_system_default_destroy(&backend->handles);
            if (backend->log_owner) {
                m3_log_shutdown();
            }
            allocator.free(allocator.ctx, backend);
            return M3_ERR_UNKNOWN;
        }
    } else {
        backend->class_registered = M3_TRUE;
    }
    backend->window_class = atom;

    m3_win32_backend_init_alpha_blend(backend);
    m3_win32_event_queue_init(&backend->event_queue);

    backend->ws.ctx = backend;
    backend->ws.vtable = &g_m3_win32_ws_vtable;
    backend->gfx.ctx = backend;
    backend->gfx.vtable = &g_m3_win32_gfx_vtable;
    backend->gfx.text_vtable = &g_m3_win32_text_vtable;
    backend->env.ctx = backend;
    backend->env.vtable = &g_m3_win32_env_vtable;
    backend->io.ctx = backend;
    backend->io.vtable = &g_m3_win32_io_vtable;
    backend->sensors.ctx = backend;
    backend->sensors.vtable = &g_m3_win32_sensors_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_m3_win32_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_m3_win32_network_vtable;
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_m3_win32_tasks_vtable;
    backend->initialized = M3_TRUE;

    *out_backend = backend;
    return M3_OK;
}

int M3_CALL m3_win32_backend_destroy(M3Win32Backend *backend)
{
    int rc;
    int first_error;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }

    rc = m3_handle_system_default_destroy(&backend->handles);
    M3_WIN32_RETURN_IF_ERROR(rc);

    first_error = M3_OK;

    if (backend->class_registered) {
        if (!UnregisterClassW(g_m3_win32_window_class_name, backend->instance) && first_error == M3_OK) {
            first_error = M3_ERR_UNKNOWN;
        }
        backend->class_registered = M3_FALSE;
    }

    if (backend->msimg32 != NULL) {
        FreeLibrary(backend->msimg32);
        backend->msimg32 = NULL;
        backend->alpha_blend = NULL;
    }

    if (backend->log_owner) {
        rc = m3_log_shutdown();
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
    }

    backend->initialized = M3_FALSE;
    backend->ws.ctx = NULL;
    backend->ws.vtable = NULL;
    backend->gfx.ctx = NULL;
    backend->gfx.vtable = NULL;
    backend->gfx.text_vtable = NULL;
    backend->env.ctx = NULL;
    backend->env.vtable = NULL;
    backend->io.ctx = NULL;
    backend->io.vtable = NULL;
    backend->sensors.ctx = NULL;
    backend->sensors.vtable = NULL;
    backend->camera.ctx = NULL;
    backend->camera.vtable = NULL;
    backend->network.ctx = NULL;
    backend->network.vtable = NULL;
    backend->tasks.ctx = NULL;
    backend->tasks.vtable = NULL;

    rc = backend->allocator.free(backend->allocator.ctx, backend);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    return first_error;
}

int M3_CALL m3_win32_backend_get_ws(M3Win32Backend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_ws = backend->ws;
    return M3_OK;
}

int M3_CALL m3_win32_backend_get_gfx(M3Win32Backend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_gfx = backend->gfx;
    return M3_OK;
}

int M3_CALL m3_win32_backend_get_env(M3Win32Backend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_env = backend->env;
    return M3_OK;
}

#else

int M3_CALL m3_win32_backend_create(const M3Win32BackendConfig *config, M3Win32Backend **out_backend)
{
    M3Win32BackendConfig local_config;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_win32_backend_config_init(&local_config);
        M3_WIN32_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_win32_backend_validate_config(config);
    M3_WIN32_RETURN_IF_ERROR(rc);

    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_win32_backend_destroy(M3Win32Backend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_win32_backend_get_ws(M3Win32Backend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_ws, 0, sizeof(*out_ws));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_win32_backend_get_gfx(M3Win32Backend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_gfx, 0, sizeof(*out_gfx));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_win32_backend_get_env(M3Win32Backend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_env, 0, sizeof(*out_env));
    return M3_ERR_UNSUPPORTED;
}

#endif
