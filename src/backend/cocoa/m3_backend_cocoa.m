#include "m3/m3_backend_cocoa.h"

#include "m3/m3_log.h"
#include "m3/m3_object.h"

#include <limits.h>
#include <string.h>
#include <errno.h>

#if defined(M3_COCOA_AVAILABLE)
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CFNetwork/CFNetwork.h>
#endif

#define M3_COCOA_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, cleanup) \
    do { \
        if ((rc) != M3_OK) { \
            cleanup; \
            return (rc); \
        } \
    } while (0)

#define M3_COCOA_DEFAULT_HANDLE_CAPACITY 64
#define M3_COCOA_EVENT_CAPACITY 256u
#define M3_COCOA_CLIP_STACK_CAPACITY 32u
#define M3_COCOA_CAMERA_DEFAULT_WIDTH 640u
#define M3_COCOA_CAMERA_DEFAULT_HEIGHT 480u

#define M3_COCOA_TYPE_WINDOW 1
#define M3_COCOA_TYPE_TEXTURE 2
#define M3_COCOA_TYPE_FONT 3

#if defined(M3_COCOA_AVAILABLE)
struct M3CocoaBackend;

@interface M3CocoaCameraDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    struct M3CocoaBackend *backend;
}
- (id)initWithBackend:(struct M3CocoaBackend *)backend;
@end

static int m3_cocoa_camera_update_frame(struct M3CocoaBackend *backend, CMSampleBufferRef sample);
#endif

#if defined(__has_feature)
#if __has_feature(objc_arc)
#define M3_COCOA_ARC 1
#endif
#endif
#ifndef M3_COCOA_ARC
#define M3_COCOA_ARC 0
#endif

#if defined(M3_COCOA_AVAILABLE)

typedef struct M3CocoaWindow {
    M3ObjectHeader header;
    struct M3CocoaBackend *backend;
    NSWindow *window;
    NSView *content_view;
    m3_i32 width;
    m3_i32 height;
    M3Scalar dpi_scale;
    M3Scalar dpi_scale_override;
    M3Bool has_scale_override;
    m3_u32 flags;
    M3Bool visible;
} M3CocoaWindow;

typedef struct M3CocoaTexture {
    M3ObjectHeader header;
    struct M3CocoaBackend *backend;
    m3_i32 width;
    m3_i32 height;
    m3_u32 format;
    m3_u32 bytes_per_pixel;
    m3_i32 stride;
    m3_usize size;
    unsigned char *pixels;
} M3CocoaTexture;

typedef struct M3CocoaFont {
    M3ObjectHeader header;
    struct M3CocoaBackend *backend;
    m3_i32 size_px;
    m3_i32 weight;
    M3Bool italic;
    CTFontRef font;
} M3CocoaFont;

struct M3CocoaBackend {
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
    M3Bool camera_opened;
    M3Bool camera_streaming;
    M3Bool camera_has_frame;
    m3_u32 camera_format;
    m3_u32 camera_width;
    m3_u32 camera_height;
    m3_u32 camera_requested_format;
    M3Bool camera_swizzle;
    m3_usize camera_frame_capacity;
    m3_usize camera_frame_size;
    unsigned char *camera_frame;
    int camera_error;
    AVCaptureSession *camera_session;
    AVCaptureDeviceInput *camera_input;
    AVCaptureVideoDataOutput *camera_output;
    dispatch_queue_t camera_queue;
    M3CocoaCameraDelegate *camera_delegate;
    M3Bool initialized;
    M3Bool log_enabled;
    M3Bool log_owner;
    M3Bool inline_tasks;
    m3_usize clipboard_limit;
    M3CocoaWindow *active_window;
    M3Bool in_frame;
    CGContextRef frame_ctx;
    void *frame_data;
    m3_usize frame_data_size;
    m3_i32 frame_width;
    m3_i32 frame_height;
    m3_usize frame_stride;
    M3Mat3 transform;
    M3Bool has_transform;
    M3Rect clip_stack[M3_COCOA_CLIP_STACK_CAPACITY];
    m3_usize clip_depth;
    M3InputEvent event_queue[M3_COCOA_EVENT_CAPACITY];
    m3_usize event_head;
    m3_usize event_tail;
    m3_usize event_count;
    char *last_text;
    double time_origin;
    NSApplication *app;
};

#if defined(M3_COCOA_AVAILABLE)
@implementation M3CocoaCameraDelegate
- (id)initWithBackend:(struct M3CocoaBackend *)backend_ptr
{
    self = [super init];
    if (self) {
        backend = backend_ptr;
    }
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)output
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    int rc;

    M3_UNUSED(output);
    M3_UNUSED(connection);

    if (backend == NULL) {
        return;
    }

    rc = m3_cocoa_camera_update_frame(backend, sampleBuffer);
    M3_UNUSED(rc);
}
@end
#endif

static void m3_cocoa_event_queue_init(struct M3CocoaBackend *backend)
{
    if (backend == NULL) {
        return;
    }
    backend->event_head = 0u;
    backend->event_tail = 0u;
    backend->event_count = 0u;
}

static int m3_cocoa_event_queue_push(struct M3CocoaBackend *backend, const M3InputEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (backend->event_count >= M3_COCOA_EVENT_CAPACITY) {
        return M3_ERR_OVERFLOW;
    }
    backend->event_queue[backend->event_tail] = *event;
    backend->event_tail = (backend->event_tail + 1u) % M3_COCOA_EVENT_CAPACITY;
    backend->event_count += 1u;
    return M3_OK;
}

static int m3_cocoa_event_queue_pop(struct M3CocoaBackend *backend, M3InputEvent *out_event, M3Bool *out_has_event)
{
    if (backend == NULL || out_event == NULL || out_has_event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (backend->event_count == 0u) {
        memset(out_event, 0, sizeof(*out_event));
        *out_has_event = M3_FALSE;
        return M3_OK;
    }
    *out_event = backend->event_queue[backend->event_head];
    backend->event_head = (backend->event_head + 1u) % M3_COCOA_EVENT_CAPACITY;
    backend->event_count -= 1u;
    *out_has_event = M3_TRUE;
    return M3_OK;
}

static int m3_cocoa_backend_validate_config(const M3CocoaBackendConfig *config)
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

static int m3_cocoa_backend_log(struct M3CocoaBackend *backend, M3LogLevel level, const char *message)
{
    if (!backend->log_enabled) {
        return M3_OK;
    }
    return m3_log_write(level, "m3.cocoa", message);
}

static int m3_cocoa_backend_resolve(struct M3CocoaBackend *backend, M3Handle handle, m3_u32 type_id, void **out_obj)
{
    void *resolved;
    m3_u32 actual_type;
    int rc;

    if (out_obj != NULL) {
        *out_obj = NULL;
    }

    rc = backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
    M3_COCOA_RETURN_IF_ERROR(rc);
    if (actual_type != type_id) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (out_obj != NULL) {
        *out_obj = resolved;
    }
    return M3_OK;
}

static int m3_cocoa_object_retain(void *obj)
{
    return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_cocoa_object_release(void *obj)
{
    return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_cocoa_object_get_type_id(void *obj, m3_u32 *out_type_id)
{
    return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_cocoa_window_destroy(void *obj)
{
    M3CocoaWindow *window;
    struct M3CocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    window = (M3CocoaWindow *)obj;
    backend = window->backend;

    if (backend != NULL && backend->active_window == window) {
        backend->active_window = NULL;
        backend->in_frame = M3_FALSE;
    }

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, window->header.handle);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }

    if (window->window != NULL) {
        [window->window orderOut:nil];
#if !M3_COCOA_ARC
        [window->window release];
#endif
        window->window = nil;
    }
    window->content_view = nil;

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, window);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }
    return M3_OK;
}

static int m3_cocoa_texture_destroy(void *obj)
{
    M3CocoaTexture *texture;
    struct M3CocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    texture = (M3CocoaTexture *)obj;
    backend = texture->backend;

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, texture->header.handle);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }

    if (texture->pixels != NULL && backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, texture->pixels);
        M3_COCOA_RETURN_IF_ERROR(rc);
        texture->pixels = NULL;
    }

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, texture);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }
    return M3_OK;
}

static int m3_cocoa_font_destroy(void *obj)
{
    M3CocoaFont *font;
    struct M3CocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    font = (M3CocoaFont *)obj;
    backend = font->backend;

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, font->header.handle);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }

    if (font->font != NULL) {
        CFRelease(font->font);
        font->font = NULL;
    }

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, font);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }
    return M3_OK;
}

static const M3ObjectVTable g_m3_cocoa_window_vtable = {
    m3_cocoa_object_retain,
    m3_cocoa_object_release,
    m3_cocoa_window_destroy,
    m3_cocoa_object_get_type_id
};

static const M3ObjectVTable g_m3_cocoa_texture_vtable = {
    m3_cocoa_object_retain,
    m3_cocoa_object_release,
    m3_cocoa_texture_destroy,
    m3_cocoa_object_get_type_id
};

static const M3ObjectVTable g_m3_cocoa_font_vtable = {
    m3_cocoa_object_retain,
    m3_cocoa_object_release,
    m3_cocoa_font_destroy,
    m3_cocoa_object_get_type_id
};

static int m3_cocoa_color_validate(M3Color color)
{
    if (color.r < 0.0f || color.r > 1.0f) {
        return M3_ERR_RANGE;
    }
    if (color.g < 0.0f || color.g > 1.0f) {
        return M3_ERR_RANGE;
    }
    if (color.b < 0.0f || color.b > 1.0f) {
        return M3_ERR_RANGE;
    }
    if (color.a < 0.0f || color.a > 1.0f) {
        return M3_ERR_RANGE;
    }
    return M3_OK;
}

static int m3_cocoa_get_active_context(struct M3CocoaBackend *backend, CGContextRef *out_ctx)
{
    if (backend == NULL || out_ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (backend->frame_ctx == NULL || backend->active_window == NULL || !backend->in_frame) {
        return M3_ERR_STATE;
    }
    *out_ctx = backend->frame_ctx;
    return M3_OK;
}

static int m3_cocoa_mat3_to_cgaffine(const M3Mat3 *mat, CGAffineTransform *out_transform)
{
    if (mat == NULL || out_transform == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    out_transform->a = (CGFloat)mat->m[0];
    out_transform->b = (CGFloat)mat->m[1];
    out_transform->c = (CGFloat)mat->m[3];
    out_transform->d = (CGFloat)mat->m[4];
    out_transform->tx = (CGFloat)mat->m[6];
    out_transform->ty = (CGFloat)mat->m[7];
    return M3_OK;
}

static int m3_cocoa_time_now_ms(struct M3CocoaBackend *backend, m3_u32 *out_time_ms)
{
    double now;
    double elapsed;
    double max_ms;

    if (backend == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    now = CFAbsoluteTimeGetCurrent();
    elapsed = now - backend->time_origin;
    if (elapsed < 0.0) {
        elapsed = 0.0;
    }
    max_ms = (double)((m3_u32)~(m3_u32)0);
    if (elapsed * 1000.0 > max_ms) {
        *out_time_ms = (m3_u32)~(m3_u32)0;
        return M3_OK;
    }
    *out_time_ms = (m3_u32)(elapsed * 1000.0);
    return M3_OK;
}

static void m3_cocoa_release_last_text(struct M3CocoaBackend *backend)
{
    int rc;

    if (backend == NULL || backend->last_text == NULL) {
        return;
    }

    rc = backend->allocator.free(backend->allocator.ctx, backend->last_text);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text.free_failed");
    }
    backend->last_text = NULL;
}

static void m3_cocoa_free_event_text(struct M3CocoaBackend *backend, M3InputEvent *event)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return;
    }

    if (event->type == M3_INPUT_TEXT_UTF8) {
        if (event->data.text_utf8.utf8 != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, (void *)event->data.text_utf8.utf8);
            if (rc != M3_OK) {
                m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text_utf8.free_failed");
            }
            event->data.text_utf8.utf8 = NULL;
        }
    } else if (event->type == M3_INPUT_TEXT_EDIT) {
        if (event->data.text_edit.utf8 != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, (void *)event->data.text_edit.utf8);
            if (rc != M3_OK) {
                m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text_edit.free_failed");
            }
            event->data.text_edit.utf8 = NULL;
        }
    }
}

static int m3_cocoa_backend_push_event(struct M3CocoaBackend *backend, const M3InputEvent *event)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_cocoa_event_queue_push(backend, event);
    if (rc == M3_ERR_OVERFLOW) {
        M3InputEvent local_event = *event;
        m3_cocoa_free_event_text(backend, &local_event);
        return M3_OK;
    }
    return rc;
}

static m3_u32 m3_cocoa_modifiers_from_flags(NSEventModifierFlags flags)
{
    m3_u32 mods = 0u;

#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
    if ((flags & NSEventModifierFlagShift) != 0u) {
        mods |= M3_MOD_SHIFT;
    }
    if ((flags & NSEventModifierFlagControl) != 0u) {
        mods |= M3_MOD_CTRL;
    }
    if ((flags & NSEventModifierFlagOption) != 0u) {
        mods |= M3_MOD_ALT;
    }
    if ((flags & NSEventModifierFlagCommand) != 0u) {
        mods |= M3_MOD_META;
    }
    if ((flags & NSEventModifierFlagCapsLock) != 0u) {
        mods |= M3_MOD_CAPS;
    }
    if ((flags & NSEventModifierFlagNumericPad) != 0u) {
        mods |= M3_MOD_NUM;
    }
#else
    if ((flags & NSShiftKeyMask) != 0u) {
        mods |= M3_MOD_SHIFT;
    }
    if ((flags & NSControlKeyMask) != 0u) {
        mods |= M3_MOD_CTRL;
    }
    if ((flags & NSAlternateKeyMask) != 0u) {
        mods |= M3_MOD_ALT;
    }
    if ((flags & NSCommandKeyMask) != 0u) {
        mods |= M3_MOD_META;
    }
    if ((flags & NSAlphaShiftKeyMask) != 0u) {
        mods |= M3_MOD_CAPS;
    }
    if ((flags & NSNumericPadKeyMask) != 0u) {
        mods |= M3_MOD_NUM;
    }
#endif

    return mods;
}

static m3_i32 m3_cocoa_buttons_from_mask(NSUInteger mask)
{
    m3_i32 buttons = 0;

    if ((mask & (1u << 0)) != 0u) {
        buttons |= 1;
    }
    if ((mask & (1u << 1)) != 0u) {
        buttons |= 2;
    }
    if ((mask & (1u << 2)) != 0u) {
        buttons |= 4;
    }
    if ((mask & (1u << 3)) != 0u) {
        buttons |= 8;
    }
    if ((mask & (1u << 4)) != 0u) {
        buttons |= 16;
    }
    return buttons;
}

static m3_i32 m3_cocoa_button_mask(NSInteger button_number)
{
    if (button_number == 0) {
        return 1;
    }
    if (button_number == 1) {
        return 2;
    }
    if (button_number == 2) {
        return 4;
    }
    if (button_number == 3) {
        return 8;
    }
    if (button_number == 4) {
        return 16;
    }
    return 0;
}

static int m3_cocoa_fill_event_header(struct M3CocoaBackend *backend, M3InputEvent *event, m3_u32 type, m3_u32 modifiers,
                                      M3Handle window)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    event->type = type;
    event->modifiers = modifiers;
    event->reserved = 0u;
    event->window = window;
    rc = m3_cocoa_time_now_ms(backend, &event->time_ms);
    return rc;
}

static int m3_cocoa_emit_pointer_event(struct M3CocoaBackend *backend, M3Handle window, m3_u32 type, m3_i32 x, m3_i32 y,
                                       m3_i32 buttons, m3_i32 scroll_x, m3_i32 scroll_y, m3_u32 modifiers)
{
    M3InputEvent event;
    int rc;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(&event, 0, sizeof(event));
    rc = m3_cocoa_fill_event_header(backend, &event, type, modifiers, window);
    M3_COCOA_RETURN_IF_ERROR(rc);

    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = x;
    event.data.pointer.y = y;
    event.data.pointer.buttons = buttons;
    event.data.pointer.scroll_x = scroll_x;
    event.data.pointer.scroll_y = scroll_y;
    return m3_cocoa_backend_push_event(backend, &event);
}

static int m3_cocoa_emit_key_event(struct M3CocoaBackend *backend, M3Handle window, m3_u32 type, m3_u32 modifiers,
                                   m3_u32 key_code, m3_u32 native_code, M3Bool is_repeat)
{
    M3InputEvent event;
    int rc;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(&event, 0, sizeof(event));
    rc = m3_cocoa_fill_event_header(backend, &event, type, modifiers, window);
    M3_COCOA_RETURN_IF_ERROR(rc);

    event.data.key.key_code = key_code;
    event.data.key.native_code = native_code;
    event.data.key.is_repeat = is_repeat;
    return m3_cocoa_backend_push_event(backend, &event);
}

static int m3_cocoa_copy_utf8(struct M3CocoaBackend *backend, NSString *string, char **out_text, m3_usize *out_len)
{
    const char *utf8;
    m3_usize length;
    char *buffer;
    int rc;

    if (backend == NULL || out_text == NULL || out_len == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_text = NULL;
    *out_len = 0u;

    if (string == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    utf8 = [string UTF8String];
    if (utf8 == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    length = 0u;
    while (utf8[length] != '\0') {
        length += 1u;
    }

    if (length == 0u) {
        return M3_OK;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, length + 1u, (void **)&buffer);
    M3_COCOA_RETURN_IF_ERROR(rc);
    memcpy(buffer, utf8, length + 1u);

    *out_text = buffer;
    *out_len = length;
    return M3_OK;
}

static int m3_cocoa_emit_text_event(struct M3CocoaBackend *backend, M3Handle window, m3_u32 modifiers, NSString *string)
{
    M3InputEvent event;
    const char *utf8;
    m3_usize length;
    int rc;

    if (backend == NULL || string == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    utf8 = [string UTF8String];
    if (utf8 == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    length = 0u;
    while (utf8[length] != '\0') {
        length += 1u;
    }

    if (length == 0u) {
        return M3_OK;
    }

    if (length <= (m3_usize)(sizeof(event.data.text.utf8) - 1u)) {
        memset(&event, 0, sizeof(event));
        rc = m3_cocoa_fill_event_header(backend, &event, M3_INPUT_TEXT, modifiers, window);
        M3_COCOA_RETURN_IF_ERROR(rc);
        memcpy(event.data.text.utf8, utf8, length);
        event.data.text.utf8[length] = '\0';
        event.data.text.length = (m3_u32)length;
        return m3_cocoa_backend_push_event(backend, &event);
    }

    {
        char *buffer;
        m3_usize out_len;

        rc = m3_cocoa_copy_utf8(backend, string, &buffer, &out_len);
        M3_COCOA_RETURN_IF_ERROR(rc);
        if (buffer == NULL || out_len == 0u) {
            return M3_OK;
        }

        memset(&event, 0, sizeof(event));
        rc = m3_cocoa_fill_event_header(backend, &event, M3_INPUT_TEXT_UTF8, modifiers, window);
        if (rc != M3_OK) {
            backend->allocator.free(backend->allocator.ctx, buffer);
            return rc;
        }
        event.data.text_utf8.utf8 = buffer;
        event.data.text_utf8.length = out_len;
        return m3_cocoa_backend_push_event(backend, &event);
    }
}

static int m3_cocoa_utf8_bytes_for_range(NSString *string, NSRange range, m3_i32 *out_offset, m3_i32 *out_length)
{
    NSUInteger string_length;
    NSString *prefix;
    NSString *selection;
    NSUInteger offset_bytes;
    NSUInteger length_bytes;

    if (string == nil || out_offset == NULL || out_length == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    string_length = [string length];
    if (range.location == NSNotFound || range.location > string_length) {
        *out_offset = 0;
        *out_length = 0;
        return M3_OK;
    }

    if (range.location + range.length > string_length) {
        range.length = string_length - range.location;
    }

    prefix = [string substringToIndex:range.location];
    selection = [string substringWithRange:range];
    offset_bytes = [prefix lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    length_bytes = [selection lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

    if (offset_bytes > (NSUInteger)INT_MAX || length_bytes > (NSUInteger)INT_MAX) {
        return M3_ERR_OVERFLOW;
    }

    *out_offset = (m3_i32)offset_bytes;
    *out_length = (m3_i32)length_bytes;
    return M3_OK;
}

static int m3_cocoa_emit_text_edit_event(struct M3CocoaBackend *backend, M3Handle window, m3_u32 modifiers, NSString *string,
                                         NSRange selected_range)
{
    M3InputEvent event;
    char *buffer;
    m3_usize out_len;
    m3_i32 cursor;
    m3_i32 selection_len;
    int rc;

    if (backend == NULL || string == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = m3_cocoa_copy_utf8(backend, string, &buffer, &out_len);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_utf8_bytes_for_range(string, selected_range, &cursor, &selection_len);
    if (rc != M3_OK) {
        if (buffer != NULL) {
            backend->allocator.free(backend->allocator.ctx, buffer);
        }
        return rc;
    }

    memset(&event, 0, sizeof(event));
    rc = m3_cocoa_fill_event_header(backend, &event, M3_INPUT_TEXT_EDIT, modifiers, window);
    if (rc != M3_OK) {
        if (buffer != NULL) {
            backend->allocator.free(backend->allocator.ctx, buffer);
        }
        return rc;
    }

    event.data.text_edit.utf8 = buffer;
    event.data.text_edit.length = out_len;
    event.data.text_edit.cursor = cursor;
    event.data.text_edit.selection_length = selection_len;
    return m3_cocoa_backend_push_event(backend, &event);
}

@interface M3CocoaView : NSView <NSTextInputClient>
{
@public
    struct M3CocoaBackend *backend;
    M3Handle window_handle;
    NSString *marked_text;
    NSRange marked_range;
    NSRange selected_range;
}
@end

@implementation M3CocoaView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self != nil) {
        backend = NULL;
        window_handle.id = 0u;
        window_handle.generation = 0u;
        marked_text = nil;
        marked_range = NSMakeRange(NSNotFound, 0);
        selected_range = NSMakeRange(0, 0);
    }
    return self;
}

- (BOOL)isFlipped
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)dealloc
{
#if !M3_COCOA_ARC
    if (marked_text != nil) {
        [marked_text release];
        marked_text = nil;
    }
    [super dealloc];
#endif
}

- (void)m3_update_marked_text:(NSString *)text selectedRange:(NSRange)range
{
    if (marked_text != nil) {
#if !M3_COCOA_ARC
        [marked_text release];
#endif
        marked_text = nil;
    }

    if (text != nil && [text length] > 0) {
#if M3_COCOA_ARC
        marked_text = text;
#else
        marked_text = [text copy];
#endif
        marked_range = NSMakeRange(0, [text length]);
        selected_range = range;
    } else {
        marked_range = NSMakeRange(NSNotFound, 0);
        selected_range = NSMakeRange(0, 0);
    }
}

- (m3_u32)m3_current_modifiers
{
    return m3_cocoa_modifiers_from_flags([NSEvent modifierFlags]);
}

- (int)m3_event_location:(NSEvent *)event outX:(m3_i32 *)out_x outY:(m3_i32 *)out_y
{
    NSPoint point;

    if (event == nil || out_x == NULL || out_y == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    point = [self convertPoint:[event locationInWindow] fromView:nil];
    *out_x = (m3_i32)point.x;
    *out_y = (m3_i32)point.y;
    return M3_OK;
}

- (void)m3_post_pointer_event:(NSEvent *)event type:(m3_u32)type includeButton:(M3Bool)include_button
{
    m3_i32 x;
    m3_i32 y;
    m3_i32 buttons;
    m3_i32 button_mask;
    m3_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    rc = [self m3_event_location:event outX:&x outY:&y];
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.location_failed");
        return;
    }

    buttons = m3_cocoa_buttons_from_mask([NSEvent pressedMouseButtons]);
    button_mask = m3_cocoa_button_mask([event buttonNumber]);
    if (include_button) {
        if (type == M3_INPUT_POINTER_UP) {
            buttons &= ~button_mask;
        } else if (type == M3_INPUT_POINTER_DOWN) {
            buttons |= button_mask;
        }
    }

    modifiers = m3_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = m3_cocoa_emit_pointer_event(backend, window_handle, type, x, y, buttons, 0, 0, modifiers);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.pointer.queue_failed");
    }
}

- (void)mouseDown:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_DOWN includeButton:M3_TRUE];
}

- (void)mouseUp:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_UP includeButton:M3_TRUE];
}

- (void)rightMouseDown:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_DOWN includeButton:M3_TRUE];
}

- (void)rightMouseUp:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_UP includeButton:M3_TRUE];
}

- (void)otherMouseDown:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_DOWN includeButton:M3_TRUE];
}

- (void)otherMouseUp:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_UP includeButton:M3_TRUE];
}

- (void)mouseMoved:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_MOVE includeButton:M3_FALSE];
}

- (void)mouseDragged:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_MOVE includeButton:M3_FALSE];
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_MOVE includeButton:M3_FALSE];
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self m3_post_pointer_event:event type:M3_INPUT_POINTER_MOVE includeButton:M3_FALSE];
}

- (void)scrollWheel:(NSEvent *)event
{
    m3_i32 x;
    m3_i32 y;
    m3_i32 buttons;
    m3_i32 scroll_x;
    m3_i32 scroll_y;
    m3_u32 modifiers;
    double dx;
    double dy;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    rc = [self m3_event_location:event outX:&x outY:&y];
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.location_failed");
        return;
    }

    buttons = m3_cocoa_buttons_from_mask([NSEvent pressedMouseButtons]);
    modifiers = m3_cocoa_modifiers_from_flags([event modifierFlags]);

    dx = [event scrollingDeltaX];
    dy = [event scrollingDeltaY];
    scroll_x = (m3_i32)dx;
    scroll_y = (m3_i32)dy;

    rc = m3_cocoa_emit_pointer_event(backend, window_handle, M3_INPUT_POINTER_SCROLL, x, y, buttons, scroll_x, scroll_y, modifiers);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.scroll.queue_failed");
    }
}

- (void)keyDown:(NSEvent *)event
{
    NSArray *events;
    m3_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    modifiers = m3_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = m3_cocoa_emit_key_event(backend, window_handle, M3_INPUT_KEY_DOWN, modifiers, (m3_u32)[event keyCode],
                                 (m3_u32)[event keyCode], event.isARepeat ? M3_TRUE : M3_FALSE);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.key_down.queue_failed");
    }

    events = [NSArray arrayWithObject:event];
    [self interpretKeyEvents:events];
}

- (void)keyUp:(NSEvent *)event
{
    m3_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    modifiers = m3_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = m3_cocoa_emit_key_event(backend, window_handle, M3_INPUT_KEY_UP, modifiers, (m3_u32)[event keyCode],
                                 (m3_u32)[event keyCode], M3_FALSE);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.key_up.queue_failed");
    }
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString *text;
    m3_u32 modifiers;
    int rc;

    M3_UNUSED(replacementRange);

    if (backend == NULL || string == nil) {
        return;
    }

    if ([string isKindOfClass:[NSAttributedString class]]) {
        text = [string string];
    } else {
        text = (NSString *)string;
    }

    if (text == nil) {
        return;
    }

    [self m3_update_marked_text:nil selectedRange:NSMakeRange(0, 0)];
    modifiers = [self m3_current_modifiers];
    rc = m3_cocoa_emit_text_event(backend, window_handle, modifiers, text);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text.queue_failed");
    }
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    NSString *text;
    m3_u32 modifiers;
    int rc;

    M3_UNUSED(replacementRange);

    if (backend == NULL || string == nil) {
        return;
    }

    if ([string isKindOfClass:[NSAttributedString class]]) {
        text = [string string];
    } else {
        text = (NSString *)string;
    }

    if (text == nil) {
        return;
    }

    [self m3_update_marked_text:text selectedRange:selectedRange];
    modifiers = [self m3_current_modifiers];
    rc = m3_cocoa_emit_text_edit_event(backend, window_handle, modifiers, text, selectedRange);
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text_edit.queue_failed");
    }
}

- (void)unmarkText
{
    m3_u32 modifiers;
    int rc;

    if (backend == NULL) {
        return;
    }

    if (marked_text == nil || [marked_text length] == 0) {
        return;
    }

    [self m3_update_marked_text:nil selectedRange:NSMakeRange(0, 0)];
    modifiers = [self m3_current_modifiers];
    rc = m3_cocoa_emit_text_edit_event(backend, window_handle, modifiers, @"", NSMakeRange(0, 0));
    if (rc != M3_OK) {
        m3_cocoa_backend_log(backend, M3_LOG_LEVEL_ERROR, "event.text_edit.queue_failed");
    }
}

- (BOOL)hasMarkedText
{
    return (marked_text != nil && [marked_text length] > 0);
}

- (NSRange)markedRange
{
    if (marked_text == nil || [marked_text length] == 0) {
        return NSMakeRange(NSNotFound, 0);
    }
    return marked_range;
}

- (NSRange)selectedRange
{
    return selected_range;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    M3_UNUSED(range);
    if (actualRange != NULL) {
        *actualRange = NSMakeRange(NSNotFound, 0);
    }
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    M3_UNUSED(point);
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    NSRect rect;
    NSWindow *window;

    if (actualRange != NULL) {
        *actualRange = range;
    }

    rect = [self bounds];
    window = [self window];
    if (window != nil) {
        rect = [self convertRect:rect toView:nil];
        rect = [window convertRectToScreen:rect];
    }
    return rect;
}

- (void)doCommandBySelector:(SEL)selector
{
    M3_UNUSED(selector);
}

@end

static int m3_cocoa_ws_init(void *ws, const M3WSConfig *config)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (ws == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
    M3_COCOA_RETURN_IF_ERROR(rc);

    backend->app = [NSApplication sharedApplication];
    if (backend->app != nil) {
        [backend->app setActivationPolicy:NSApplicationActivationPolicyRegular];
    }
    return M3_OK;
}

static int m3_cocoa_ws_shutdown(void *ws)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_cocoa_ws_create_window(void *ws, const M3WSWindowConfig *config, M3Handle *out_window)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *window;
    NSWindowStyleMask style_mask;
    NSRect rect;
    NSString *title;
    NSView *content_view;
    int rc;

    if (ws == NULL || config == NULL || out_window == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->width <= 0 || config->height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)ws;
    out_window->id = 0u;
    out_window->generation = 0u;

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
    M3_COCOA_RETURN_IF_ERROR(rc);

    style_mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (config->flags & M3_WS_WINDOW_RESIZABLE) {
        style_mask |= NSWindowStyleMaskResizable;
    }
    if (config->flags & M3_WS_WINDOW_BORDERLESS) {
        style_mask = NSWindowStyleMaskBorderless;
    }

    rect = NSMakeRect(0.0, 0.0, (CGFloat)config->width, (CGFloat)config->height);
    title = [[NSString alloc] initWithUTF8String:config->utf8_title];
    if (title == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3CocoaWindow), (void **)&window);
    if (rc != M3_OK) {
#if !M3_COCOA_ARC
        [title release];
#endif
        return rc;
    }

    memset(window, 0, sizeof(*window));
    window->backend = backend;
    window->width = config->width;
    window->height = config->height;
    window->dpi_scale = 1.0f;
    window->flags = config->flags;
    window->visible = M3_FALSE;

    window->window = [[NSWindow alloc] initWithContentRect:rect
                                                 styleMask:style_mask
                                                   backing:NSBackingStoreBuffered
                                                     defer:NO];
    if (window->window == nil) {
#if !M3_COCOA_ARC
        [title release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return M3_ERR_UNKNOWN;
    }

    content_view = [[M3CocoaView alloc] initWithFrame:rect];
    if (content_view == nil) {
#if !M3_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return M3_ERR_OUT_OF_MEMORY;
    }
    [content_view setWantsLayer:YES];
    [window->window setContentView:content_view];
    window->content_view = content_view;
#if !M3_COCOA_ARC
    [content_view release];
#endif
    [window->window setAcceptsMouseMovedEvents:YES];

    [window->window setReleasedWhenClosed:NO];
    [window->window setTitle:title];
#if !M3_COCOA_ARC
    [title release];
#endif

    if ((config->flags & M3_WS_WINDOW_HIGH_DPI) != 0u) {
        window->dpi_scale = (M3Scalar)[window->window backingScaleFactor];
    }

    rc = m3_object_header_init(&window->header, M3_COCOA_TYPE_WINDOW, 0, &g_m3_cocoa_window_vtable);
    if (rc != M3_OK) {
        [window->window orderOut:nil];
#if !M3_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return rc;
    }

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &window->header);
    if (rc != M3_OK) {
        [window->window orderOut:nil];
#if !M3_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return rc;
    }

    if (window->content_view != nil) {
        M3CocoaView *view = (M3CocoaView *)window->content_view;
        view->backend = backend;
        view->window_handle = window->header.handle;
        [window->window makeFirstResponder:window->content_view];
    }

    if ((config->flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
        [window->window toggleFullScreen:nil];
    }

    *out_window = window->header.handle;
    return M3_OK;
}

static int m3_cocoa_ws_destroy_window(void *ws, M3Handle window)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
    M3_COCOA_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_cocoa_ws_show_window(void *ws, M3Handle window)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        [resolved->window makeKeyAndOrderFront:nil];
        if (resolved->content_view != nil) {
            [resolved->window makeFirstResponder:resolved->content_view];
        }
        if (backend->app != nil) {
            [backend->app activateIgnoringOtherApps:YES];
        }
    }
    resolved->visible = M3_TRUE;
    return M3_OK;
}

static int m3_cocoa_ws_hide_window(void *ws, M3Handle window)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        [resolved->window orderOut:nil];
    }
    resolved->visible = M3_FALSE;
    return M3_OK;
}

static int m3_cocoa_ws_set_window_title(void *ws, M3Handle window, const char *utf8_title)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    NSString *title;
    int rc;

    if (ws == NULL || utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
    M3_COCOA_RETURN_IF_ERROR(rc);

    title = [[NSString alloc] initWithUTF8String:utf8_title];
    if (title == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (resolved->window != nil) {
        [resolved->window setTitle:title];
    }
#if !M3_COCOA_ARC
    [title release];
#endif
    return M3_OK;
}

static int m3_cocoa_ws_set_window_size(void *ws, M3Handle window, m3_i32 width, m3_i32 height)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    NSSize size;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
    M3_COCOA_RETURN_IF_ERROR(rc);

    size.width = (CGFloat)width;
    size.height = (CGFloat)height;
    if (resolved->window != nil) {
        [resolved->window setContentSize:size];
    }
    resolved->width = width;
    resolved->height = height;
    return M3_OK;
}

static int m3_cocoa_ws_get_window_size(void *ws, M3Handle window, m3_i32 *out_width, m3_i32 *out_height)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    NSRect rect;
    int rc;

    if (ws == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        rect = [resolved->window contentRectForFrameRect:[resolved->window frame]];
        resolved->width = (m3_i32)rect.size.width;
        resolved->height = (m3_i32)rect.size.height;
    }

    *out_width = resolved->width;
    *out_height = resolved->height;
    return M3_OK;
}

static int m3_cocoa_ws_set_window_dpi_scale(void *ws, M3Handle window, M3Scalar scale)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (scale <= 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
    M3_COCOA_RETURN_IF_ERROR(rc);

    resolved->dpi_scale_override = scale;
    resolved->has_scale_override = M3_TRUE;
    return M3_OK;
}

static int m3_cocoa_ws_get_window_dpi_scale(void *ws, M3Handle window, M3Scalar *out_scale)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int rc;

    if (ws == NULL || out_scale == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->has_scale_override) {
        *out_scale = resolved->dpi_scale_override;
        return M3_OK;
    }

    if (resolved->window != nil) {
        resolved->dpi_scale = (M3Scalar)[resolved->window backingScaleFactor];
    }
    *out_scale = resolved->dpi_scale;
    return M3_OK;
}

static int m3_cocoa_ws_set_clipboard_text(void *ws, const char *utf8_text)
{
    struct M3CocoaBackend *backend;
    m3_usize length;
    NSString *text;
    NSPasteboard *pasteboard;
    int rc;

    if (ws == NULL || utf8_text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
    M3_COCOA_RETURN_IF_ERROR(rc);

    length = 0;
    while (utf8_text[length] != '\0') {
        length += 1;
    }
    if (length > backend->clipboard_limit) {
        return M3_ERR_RANGE;
    }

    text = [[NSString alloc] initWithBytes:utf8_text length:length encoding:NSUTF8StringEncoding];
    if (text == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard == nil) {
#if !M3_COCOA_ARC
        [text release];
#endif
        return M3_ERR_STATE;
    }

    [pasteboard clearContents];
    if (![pasteboard setString:text forType:NSPasteboardTypeString]) {
#if !M3_COCOA_ARC
        [text release];
#endif
        return M3_ERR_UNKNOWN;
    }

#if !M3_COCOA_ARC
    [text release];
#endif
    return M3_OK;
}

static int m3_cocoa_ws_get_clipboard_text(void *ws, char *buffer, m3_usize buffer_size, m3_usize *out_length)
{
    struct M3CocoaBackend *backend;
    NSPasteboard *pasteboard;
    NSString *text;
    const char *utf8;
    m3_usize length;
    int rc;

    if (ws == NULL || buffer == NULL || out_length == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
    M3_COCOA_RETURN_IF_ERROR(rc);

    pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard == nil) {
        return M3_ERR_STATE;
    }

    text = [pasteboard stringForType:NSPasteboardTypeString];
    if (text == nil) {
        if (buffer_size == 0) {
            return M3_ERR_RANGE;
        }
        buffer[0] = '\0';
        *out_length = 0;
        return M3_OK;
    }

    utf8 = [text UTF8String];
    if (utf8 == NULL) {
        if (buffer_size == 0) {
            return M3_ERR_RANGE;
        }
        buffer[0] = '\0';
        *out_length = 0;
        return M3_OK;
    }

    length = 0;
    while (utf8[length] != '\0') {
        length += 1;
    }

    if (buffer_size < length + 1) {
        *out_length = length;
        return M3_ERR_RANGE;
    }

    memcpy(buffer, utf8, length + 1);
    *out_length = length;
    return M3_OK;
}

static int m3_cocoa_ws_pump_events(void *ws);

static int m3_cocoa_ws_poll_event(void *ws, M3InputEvent *out_event, M3Bool *out_has_event)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (ws == NULL || out_event == NULL || out_has_event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
    M3_COCOA_RETURN_IF_ERROR(rc);

    m3_cocoa_release_last_text(backend);

    if (backend->event_count == 0u) {
        rc = m3_cocoa_ws_pump_events(ws);
        M3_COCOA_RETURN_IF_ERROR(rc);
    }

    rc = m3_cocoa_event_queue_pop(backend, out_event, out_has_event);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (*out_has_event) {
        if (out_event->type == M3_INPUT_TEXT_UTF8 && out_event->data.text_utf8.utf8 != NULL) {
            backend->last_text = (char *)out_event->data.text_utf8.utf8;
        } else if (out_event->type == M3_INPUT_TEXT_EDIT && out_event->data.text_edit.utf8 != NULL) {
            backend->last_text = (char *)out_event->data.text_edit.utf8;
        }
    }
    return M3_OK;
}

static int m3_cocoa_ws_pump_events(void *ws)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->app != nil) {
        NSEvent *event;
        do {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
            event = [backend->app nextEventMatchingMask:NSEventMaskAny
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES];
#else
            event = [backend->app nextEventMatchingMask:NSAnyEventMask
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES];
#endif
            if (event != nil) {
                [backend->app sendEvent:event];
            }
        } while (event != nil);
    }

    return M3_OK;
}

static int m3_cocoa_ws_get_time_ms(void *ws, m3_u32 *out_time_ms)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (ws == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)ws;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
    M3_COCOA_RETURN_IF_ERROR(rc);

    return m3_cocoa_time_now_ms(backend, out_time_ms);
}

static const M3WSVTable g_m3_cocoa_ws_vtable = {
    m3_cocoa_ws_init,
    m3_cocoa_ws_shutdown,
    m3_cocoa_ws_create_window,
    m3_cocoa_ws_destroy_window,
    m3_cocoa_ws_show_window,
    m3_cocoa_ws_hide_window,
    m3_cocoa_ws_set_window_title,
    m3_cocoa_ws_set_window_size,
    m3_cocoa_ws_get_window_size,
    m3_cocoa_ws_set_window_dpi_scale,
    m3_cocoa_ws_get_window_dpi_scale,
    m3_cocoa_ws_set_clipboard_text,
    m3_cocoa_ws_get_clipboard_text,
    m3_cocoa_ws_poll_event,
    m3_cocoa_ws_pump_events,
    m3_cocoa_ws_get_time_ms
};

static int m3_cocoa_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width, m3_i32 height, M3Scalar dpi_scale)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    CGColorSpaceRef color_space;
    CGContextRef context;
    m3_usize row_bytes;
    m3_usize total_bytes;
    void *data;
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

    backend = (struct M3CocoaBackend *)gfx;
    if (backend->active_window != NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
    M3_COCOA_RETURN_IF_ERROR(rc);

    row_bytes = (m3_usize)width * 4u;
    if (row_bytes / 4u != (m3_usize)width) {
        return M3_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (m3_usize)height;
    if (total_bytes / (m3_usize)height != row_bytes) {
        return M3_ERR_OVERFLOW;
    }

    if (backend->frame_data == NULL || backend->frame_data_size < total_bytes) {
        if (backend->frame_data != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, backend->frame_data);
            M3_COCOA_RETURN_IF_ERROR(rc);
            backend->frame_data = NULL;
            backend->frame_data_size = 0u;
        }
        rc = backend->allocator.alloc(backend->allocator.ctx, total_bytes, &data);
        M3_COCOA_RETURN_IF_ERROR(rc);
        backend->frame_data = data;
        backend->frame_data_size = total_bytes;
    } else {
        data = backend->frame_data;
    }
    memset(data, 0, total_bytes);

    color_space = CGColorSpaceCreateDeviceRGB();
    if (color_space == NULL) {
        backend->allocator.free(backend->allocator.ctx, data);
        return M3_ERR_UNKNOWN;
    }

    context = CGBitmapContextCreate(data, (size_t)width, (size_t)height, 8, (size_t)row_bytes, color_space,
                                    kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(color_space);
    if (context == NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, data);
        M3_COCOA_RETURN_IF_ERROR(rc);
        if (backend->frame_data == data) {
            backend->frame_data = NULL;
            backend->frame_data_size = 0u;
        }
        return M3_ERR_UNKNOWN;
    }

    CGContextTranslateCTM(context, 0.0, (CGFloat)height);
    CGContextScaleCTM(context, 1.0, -1.0);

    backend->frame_ctx = context;
    backend->frame_data = data;
    backend->frame_data_size = total_bytes;
    backend->frame_width = width;
    backend->frame_height = height;
    backend->frame_stride = row_bytes;
    backend->active_window = resolved;
    backend->clip_depth = 0u;
    backend->has_transform = M3_FALSE;
    backend->in_frame = M3_TRUE;

    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    return M3_OK;
}

static int m3_cocoa_gfx_end_frame(void *gfx, M3Handle window)
{
    struct M3CocoaBackend *backend;
    M3CocoaWindow *resolved;
    int result;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_resolve(backend, window, M3_COCOA_TYPE_WINDOW, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->active_window != resolved) {
        return M3_ERR_STATE;
    }

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
    M3_COCOA_RETURN_IF_ERROR(rc);

    result = M3_OK;

    if (backend->frame_ctx != NULL) {
        CGImageRef image;
        NSView *view;
        id layer;

        CGContextFlush(backend->frame_ctx);
        image = CGBitmapContextCreateImage(backend->frame_ctx);
        if (image == NULL) {
            result = M3_ERR_UNKNOWN;
        } else {
            view = resolved->content_view;
            if (view == nil && resolved->window != nil) {
                view = [resolved->window contentView];
            }
            if (view != nil) {
                layer = [view layer];
                if (layer != nil) {
                    [layer setContents:(id)image];
                }
            }
            CGImageRelease(image);
        }
        CGContextRelease(backend->frame_ctx);
        backend->frame_ctx = NULL;
    }
    backend->frame_width = 0;
    backend->frame_height = 0;
    backend->frame_stride = 0u;
    backend->active_window = NULL;
    backend->in_frame = M3_FALSE;
    return result;
}

static int m3_cocoa_gfx_clear(void *gfx, M3Color color)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    CGRect rect;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_color_validate(color);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rect = CGRectMake(0.0, 0.0, (CGFloat)backend->frame_width, (CGFloat)backend->frame_height);
    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextFillRect(ctx, rect);
    return M3_OK;
}

static int m3_cocoa_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    CGRect cg_rect;
    CGPathRef path;
    CGAffineTransform transform;
    M3Bool apply_transform;
    M3Scalar radius;
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

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_color_validate(color);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    cg_rect = CGRectMake((CGFloat)rect->x, (CGFloat)rect->y, (CGFloat)rect->width, (CGFloat)rect->height);
    radius = corner_radius;
    if (radius > 0.0f) {
        if (radius * 2.0f > rect->width) {
            radius = rect->width * 0.5f;
        }
        if (radius * 2.0f > rect->height) {
            radius = rect->height * 0.5f;
        }
    }

    apply_transform = backend->has_transform ? M3_TRUE : M3_FALSE;
    if (apply_transform) {
        rc = m3_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        M3_COCOA_RETURN_IF_ERROR(rc);
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    if (radius > 0.0f) {
        path = CGPathCreateWithRoundedRect(cg_rect, (CGFloat)radius, (CGFloat)radius, NULL);
        if (path != NULL) {
            CGContextAddPath(ctx, path);
            CGContextFillPath(ctx);
            CGPathRelease(path);
        } else {
            CGContextFillRect(ctx, cg_rect);
        }
    } else {
        CGContextFillRect(ctx, cg_rect);
    }

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }
    return M3_OK;
}

static int m3_cocoa_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1, M3Scalar y1, M3Color color, M3Scalar thickness)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    CGAffineTransform transform;
    M3Bool apply_transform;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (thickness < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_color_validate(color);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    apply_transform = backend->has_transform ? M3_TRUE : M3_FALSE;
    if (apply_transform) {
        rc = m3_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        M3_COCOA_RETURN_IF_ERROR(rc);
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBStrokeColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextSetLineWidth(ctx, (CGFloat)thickness);
    CGContextMoveToPoint(ctx, (CGFloat)x0, (CGFloat)y0);
    CGContextAddLineToPoint(ctx, (CGFloat)x1, (CGFloat)y1);
    CGContextStrokePath(ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }
    return M3_OK;
}

static int m3_cocoa_path_build(const M3Path *path, CGMutablePathRef cg_path)
{
    M3Scalar current_x;
    M3Scalar current_y;
    M3Scalar start_x;
    M3Scalar start_y;
    M3Bool has_current;
    m3_usize i;

    if (path == NULL || cg_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (path->commands == NULL) {
        return M3_ERR_STATE;
    }
    if (path->count == 0) {
        return M3_OK;
    }

    has_current = M3_FALSE;
    current_x = 0.0f;
    current_y = 0.0f;
    start_x = 0.0f;
    start_y = 0.0f;

    for (i = 0; i < path->count; ++i) {
        const M3PathCmd *cmd = &path->commands[i];
        switch (cmd->type) {
        case M3_PATH_CMD_MOVE_TO:
            current_x = cmd->data.move_to.x;
            current_y = cmd->data.move_to.y;
            start_x = current_x;
            start_y = current_y;
            has_current = M3_TRUE;
            CGPathMoveToPoint(cg_path, NULL, (CGFloat)current_x, (CGFloat)current_y);
            break;
        case M3_PATH_CMD_LINE_TO:
            if (!has_current) {
                return M3_ERR_STATE;
            }
            current_x = cmd->data.line_to.x;
            current_y = cmd->data.line_to.y;
            CGPathAddLineToPoint(cg_path, NULL, (CGFloat)current_x, (CGFloat)current_y);
            break;
        case M3_PATH_CMD_QUAD_TO:
            if (!has_current) {
                return M3_ERR_STATE;
            }
            current_x = cmd->data.quad_to.x;
            current_y = cmd->data.quad_to.y;
            CGPathAddQuadCurveToPoint(cg_path, NULL,
                                      (CGFloat)cmd->data.quad_to.cx,
                                      (CGFloat)cmd->data.quad_to.cy,
                                      (CGFloat)current_x, (CGFloat)current_y);
            break;
        case M3_PATH_CMD_CUBIC_TO:
            if (!has_current) {
                return M3_ERR_STATE;
            }
            current_x = cmd->data.cubic_to.x;
            current_y = cmd->data.cubic_to.y;
            CGPathAddCurveToPoint(cg_path, NULL,
                                  (CGFloat)cmd->data.cubic_to.cx1,
                                  (CGFloat)cmd->data.cubic_to.cy1,
                                  (CGFloat)cmd->data.cubic_to.cx2,
                                  (CGFloat)cmd->data.cubic_to.cy2,
                                  (CGFloat)current_x, (CGFloat)current_y);
            break;
        case M3_PATH_CMD_CLOSE:
            if (!has_current) {
                return M3_ERR_STATE;
            }
            CGPathCloseSubpath(cg_path);
            current_x = start_x;
            current_y = start_y;
            has_current = M3_FALSE;
            break;
        default:
            return M3_ERR_INVALID_ARGUMENT;
        }
    }
    return M3_OK;
}

static int m3_cocoa_gfx_draw_path(void *gfx, const M3Path *path, M3Color color)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    CGMutablePathRef cg_path;
    CGAffineTransform transform;
    M3Bool apply_transform;
    int rc;

    if (gfx == NULL || path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (path->commands == NULL) {
        return M3_ERR_STATE;
    }
    if (path->count > path->capacity) {
        return M3_ERR_STATE;
    }
    if (path->count == 0) {
        return M3_OK;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_path");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_color_validate(color);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    cg_path = CGPathCreateMutable();
    if (cg_path == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    rc = m3_cocoa_path_build(path, cg_path);
    if (rc != M3_OK) {
        CGPathRelease(cg_path);
        return rc;
    }

    apply_transform = backend->has_transform ? M3_TRUE : M3_FALSE;
    if (apply_transform) {
        rc = m3_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CGPathRelease(cg_path));
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextAddPath(ctx, cg_path);
    CGContextFillPath(ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }

    CGPathRelease(cg_path);
    return M3_OK;
}

static int m3_cocoa_gfx_push_clip(void *gfx, const M3Rect *rect)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    CGRect cg_rect;
    int rc;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->clip_depth >= M3_COCOA_CLIP_STACK_CAPACITY) {
        return M3_ERR_OVERFLOW;
    }

    cg_rect = CGRectMake((CGFloat)rect->x, (CGFloat)rect->y, (CGFloat)rect->width, (CGFloat)rect->height);
    CGContextSaveGState(ctx);
    CGContextClipToRect(ctx, cg_rect);
    backend->clip_stack[backend->clip_depth] = *rect;
    backend->clip_depth += 1u;
    return M3_OK;
}

static int m3_cocoa_gfx_pop_clip(void *gfx)
{
    struct M3CocoaBackend *backend;
    CGContextRef ctx;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->clip_depth == 0u) {
        return M3_ERR_STATE;
    }

    CGContextRestoreGState(ctx);
    backend->clip_depth -= 1u;
    return M3_OK;
}

static int m3_cocoa_gfx_set_transform(void *gfx, const M3Mat3 *transform)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (gfx == NULL || transform == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
    M3_COCOA_RETURN_IF_ERROR(rc);

    backend->transform = *transform;
    backend->has_transform = M3_TRUE;
    return M3_OK;
}

static int m3_cocoa_texture_format_info(m3_u32 format, m3_u32 *out_bpp, CGBitmapInfo *out_info, CGColorSpaceRef *out_space)
{
    CGColorSpaceRef color_space;
    CGBitmapInfo info;
    m3_u32 bpp;

    if (out_bpp == NULL || out_info == NULL || out_space == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    color_space = NULL;
    info = 0u;
    bpp = 0u;

    if (format == M3_TEX_FORMAT_RGBA8) {
        color_space = CGColorSpaceCreateDeviceRGB();
        info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
        bpp = 4u;
    } else if (format == M3_TEX_FORMAT_BGRA8) {
        color_space = CGColorSpaceCreateDeviceRGB();
        info = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
        bpp = 4u;
    } else if (format == M3_TEX_FORMAT_A8) {
        color_space = CGColorSpaceCreateDeviceGray();
        info = (CGBitmapInfo)kCGImageAlphaOnly;
        bpp = 1u;
    } else {
        return M3_ERR_UNSUPPORTED;
    }

    if (color_space == NULL) {
        return M3_ERR_UNKNOWN;
    }

    *out_bpp = bpp;
    *out_info = info;
    *out_space = color_space;
    return M3_OK;
}

static int m3_cocoa_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height, m3_u32 format, const void *pixels, m3_usize size,
                                      M3Handle *out_texture)
{
    struct M3CocoaBackend *backend;
    M3CocoaTexture *texture;
    m3_u32 bpp;
    CGBitmapInfo info;
    CGColorSpaceRef color_space;
    m3_usize row_bytes;
    m3_usize total_bytes;
    int rc;

    if (gfx == NULL || out_texture == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (pixels == NULL && size != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    out_texture->id = 0u;
    out_texture->generation = 0u;

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
    M3_COCOA_RETURN_IF_ERROR(rc);

    color_space = NULL;
    rc = m3_cocoa_texture_format_info(format, &bpp, &info, &color_space);
    if (rc != M3_OK) {
        if (color_space != NULL) {
            CGColorSpaceRelease(color_space);
        }
        return rc;
    }
    CGColorSpaceRelease(color_space);

    row_bytes = (m3_usize)width * (m3_usize)bpp;
    if (row_bytes / (m3_usize)bpp != (m3_usize)width) {
        return M3_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (m3_usize)height;
    if (total_bytes / (m3_usize)height != row_bytes) {
        return M3_ERR_OVERFLOW;
    }
    if (pixels != NULL && size < total_bytes) {
        return M3_ERR_RANGE;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3CocoaTexture), (void **)&texture);
    M3_COCOA_RETURN_IF_ERROR(rc);

    memset(texture, 0, sizeof(*texture));
    texture->backend = backend;
    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->bytes_per_pixel = bpp;
    texture->stride = (m3_i32)row_bytes;
    texture->size = total_bytes;

    rc = backend->allocator.alloc(backend->allocator.ctx, total_bytes, (void **)&texture->pixels);
    if (rc != M3_OK) {
        backend->allocator.free(backend->allocator.ctx, texture);
        return rc;
    }

    if (pixels != NULL && total_bytes > 0u) {
        memcpy(texture->pixels, pixels, total_bytes);
    } else if (total_bytes > 0u) {
        memset(texture->pixels, 0, total_bytes);
    }

    rc = m3_object_header_init(&texture->header, M3_COCOA_TYPE_TEXTURE, 0, &g_m3_cocoa_texture_vtable);
    M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture->pixels);
                                     backend->allocator.free(backend->allocator.ctx, texture));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &texture->header);
    M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture->pixels);
                                     backend->allocator.free(backend->allocator.ctx, texture));

    *out_texture = texture->header.handle;
    return M3_OK;
}

static int m3_cocoa_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x, m3_i32 y, m3_i32 width, m3_i32 height, const void *pixels,
                                      m3_usize size)
{
    struct M3CocoaBackend *backend;
    M3CocoaTexture *resolved;
    m3_usize row_bytes;
    m3_usize total_bytes;
    m3_usize row;
    const unsigned char *src;
    unsigned char *dst;
    int rc;

    if (gfx == NULL || pixels == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_resolve(backend, texture, M3_COCOA_TYPE_TEXTURE, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (x < 0 || y < 0 || x + width > resolved->width || y + height > resolved->height) {
        return M3_ERR_RANGE;
    }

    row_bytes = (m3_usize)width * (m3_usize)resolved->bytes_per_pixel;
    if (row_bytes / (m3_usize)resolved->bytes_per_pixel != (m3_usize)width) {
        return M3_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (m3_usize)height;
    if (total_bytes / (m3_usize)height != row_bytes) {
        return M3_ERR_OVERFLOW;
    }
    if (size < total_bytes) {
        return M3_ERR_RANGE;
    }

    src = (const unsigned char *)pixels;
    dst = resolved->pixels + ((m3_usize)y * (m3_usize)resolved->stride) + ((m3_usize)x * (m3_usize)resolved->bytes_per_pixel);
    for (row = 0; row < (m3_usize)height; row += 1u) {
        memcpy(dst, src, row_bytes);
        src += row_bytes;
        dst += (m3_usize)resolved->stride;
    }

    return M3_OK;
}

static int m3_cocoa_gfx_destroy_texture(void *gfx, M3Handle texture)
{
    struct M3CocoaBackend *backend;
    M3CocoaTexture *resolved;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_resolve(backend, texture, M3_COCOA_TYPE_TEXTURE, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
    M3_COCOA_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_cocoa_gfx_draw_texture(void *gfx, M3Handle texture, const M3Rect *src, const M3Rect *dst, M3Scalar opacity)
{
    struct M3CocoaBackend *backend;
    M3CocoaTexture *resolved;
    CGContextRef ctx;
    CGColorSpaceRef color_space;
    CGDataProviderRef provider;
    CGImageRef image;
    CGImageRef sub_image;
    CGBitmapInfo info;
    CGRect src_rect;
    CGRect dst_rect;
    CGAffineTransform transform;
    M3Bool apply_transform;
    m3_u32 bpp;
    int rc;

    if (gfx == NULL || src == NULL || dst == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (opacity < 0.0f || opacity > 1.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)gfx;
    rc = m3_cocoa_backend_resolve(backend, texture, M3_COCOA_TYPE_TEXTURE, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->pixels == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (src->width < 0.0f || src->height < 0.0f || dst->width < 0.0f || dst->height < 0.0f) {
        return M3_ERR_RANGE;
    }

    color_space = NULL;
    rc = m3_cocoa_texture_format_info(resolved->format, &bpp, &info, &color_space);
    if (rc != M3_OK) {
        if (color_space != NULL) {
            CGColorSpaceRelease(color_space);
        }
        return rc;
    }

    provider = CGDataProviderCreateWithData(NULL, resolved->pixels, (size_t)resolved->size, NULL);
    if (provider == NULL) {
        CGColorSpaceRelease(color_space);
        return M3_ERR_UNKNOWN;
    }

    image = CGImageCreate((size_t)resolved->width, (size_t)resolved->height, 8, (size_t)(bpp * 8u),
                          (size_t)resolved->stride, color_space, info, provider, NULL, 0, kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
    if (image == NULL) {
        return M3_ERR_UNKNOWN;
    }

    src_rect = CGRectMake((CGFloat)src->x, (CGFloat)src->y, (CGFloat)src->width, (CGFloat)src->height);
    if (src_rect.size.width <= 0.0 || src_rect.size.height <= 0.0) {
        CGImageRelease(image);
        return M3_OK;
    }

    sub_image = CGImageCreateWithImageInRect(image, src_rect);
    CGImageRelease(image);
    if (sub_image == NULL) {
        return M3_ERR_UNKNOWN;
    }

    dst_rect = CGRectMake((CGFloat)dst->x, (CGFloat)dst->y, (CGFloat)dst->width, (CGFloat)dst->height);

    apply_transform = backend->has_transform ? M3_TRUE : M3_FALSE;
    CGContextSaveGState(ctx);
    if (apply_transform) {
        rc = m3_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        if (rc != M3_OK) {
            CGContextRestoreGState(ctx);
            CGImageRelease(sub_image);
            return rc;
        }
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetAlpha(ctx, (CGFloat)opacity);
    CGContextDrawImage(ctx, dst_rect, sub_image);
    CGContextRestoreGState(ctx);

    CGImageRelease(sub_image);
    return M3_OK;
}

static const M3GfxVTable g_m3_cocoa_gfx_vtable = {
    m3_cocoa_gfx_begin_frame,
    m3_cocoa_gfx_end_frame,
    m3_cocoa_gfx_clear,
    m3_cocoa_gfx_draw_rect,
    m3_cocoa_gfx_draw_line,
    m3_cocoa_gfx_draw_path,
    m3_cocoa_gfx_push_clip,
    m3_cocoa_gfx_pop_clip,
    m3_cocoa_gfx_set_transform,
    m3_cocoa_gfx_create_texture,
    m3_cocoa_gfx_update_texture,
    m3_cocoa_gfx_destroy_texture,
    m3_cocoa_gfx_draw_texture
};

static int m3_cocoa_text_create_font(void *text, const char *utf8_family, m3_i32 size_px, m3_i32 weight, M3Bool italic, M3Handle *out_font)
{
    struct M3CocoaBackend *backend;
    M3CocoaFont *font;
    CFStringRef family;
    CFMutableDictionaryRef traits;
    CFMutableDictionaryRef attributes;
    CTFontDescriptorRef descriptor;
    CTFontRef ct_font;
    float weight_norm;
    int trait_value;
    CFNumberRef weight_number;
    CFNumberRef trait_number;
    int rc;

    if (text == NULL || out_font == NULL || utf8_family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)text;
    out_font->id = 0u;
    out_font->generation = 0u;

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
    M3_COCOA_RETURN_IF_ERROR(rc);

    family = CFStringCreateWithCString(kCFAllocatorDefault, utf8_family, kCFStringEncodingUTF8);
    if (family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    traits = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (traits == NULL) {
        CFRelease(family);
        return M3_ERR_OUT_OF_MEMORY;
    }

    weight_norm = ((float)weight - 400.0f) / 500.0f;
    if (weight_norm < -1.0f) {
        weight_norm = -1.0f;
    }
    if (weight_norm > 1.0f) {
        weight_norm = 1.0f;
    }
    weight_number = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &weight_norm);
    if (weight_number == NULL) {
        CFRelease(traits);
        CFRelease(family);
        return M3_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(traits, kCTFontWeightTrait, weight_number);
    CFRelease(weight_number);

    if (italic) {
        trait_value = (int)kCTFontItalicTrait;
        trait_number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &trait_value);
        if (trait_number == NULL) {
            CFRelease(traits);
            CFRelease(family);
            return M3_ERR_OUT_OF_MEMORY;
        }
        CFDictionarySetValue(traits, kCTFontSymbolicTrait, trait_number);
        CFRelease(trait_number);
    }

    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL) {
        CFRelease(traits);
        CFRelease(family);
        return M3_ERR_OUT_OF_MEMORY;
    }

    CFDictionarySetValue(attributes, kCTFontFamilyNameAttribute, family);
    CFDictionarySetValue(attributes, kCTFontTraitsAttribute, traits);
    CFRelease(traits);
    CFRelease(family);

    descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    CFRelease(attributes);
    if (descriptor == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    ct_font = CTFontCreateWithFontDescriptor(descriptor, (CGFloat)size_px, NULL);
    CFRelease(descriptor);
    if (ct_font == NULL) {
        return M3_ERR_NOT_FOUND;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3CocoaFont), (void **)&font);
    if (rc != M3_OK) {
        CFRelease(ct_font);
        return rc;
    }

    memset(font, 0, sizeof(*font));
    font->backend = backend;
    font->size_px = size_px;
    font->weight = weight;
    font->italic = italic ? M3_TRUE : M3_FALSE;
    font->font = ct_font;

    rc = m3_object_header_init(&font->header, M3_COCOA_TYPE_FONT, 0, &g_m3_cocoa_font_vtable);
    M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CFRelease(ct_font);
                                     backend->allocator.free(backend->allocator.ctx, font));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &font->header);
    M3_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CFRelease(ct_font);
                                     backend->allocator.free(backend->allocator.ctx, font));

    *out_font = font->header.handle;
    return M3_OK;
}

static int m3_cocoa_text_destroy_font(void *text, M3Handle font)
{
    struct M3CocoaBackend *backend;
    M3CocoaFont *resolved;
    int rc;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)text;
    rc = m3_cocoa_backend_resolve(backend, font, M3_COCOA_TYPE_FONT, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
    M3_COCOA_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_cocoa_text_measure_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar *out_width, M3Scalar *out_height,
                                     M3Scalar *out_baseline)
{
    struct M3CocoaBackend *backend;
    M3CocoaFont *resolved;
    CFStringRef string_ref;
    CFDictionaryRef attributes;
    CFAttributedStringRef attr_string;
    CTLineRef line;
    CFMutableDictionaryRef attrs_mut;
    double width;
    double ascent;
    double descent;
    double leading;
    int rc;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)text;
    rc = m3_cocoa_backend_resolve(backend, font, M3_COCOA_TYPE_FONT, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->font == NULL) {
        return M3_ERR_STATE;
    }

    if (utf8_len == 0u) {
        *out_width = 0.0f;
        *out_height = (M3Scalar)(CTFontGetAscent(resolved->font) + CTFontGetDescent(resolved->font));
        *out_baseline = (M3Scalar)CTFontGetAscent(resolved->font);
        return M3_OK;
    }

    string_ref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)utf8, (CFIndex)utf8_len, kCFStringEncodingUTF8, false);
    if (string_ref == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    attrs_mut = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attrs_mut == NULL) {
        CFRelease(string_ref);
        return M3_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(attrs_mut, kCTFontAttributeName, resolved->font);
    attributes = attrs_mut;

    attr_string = CFAttributedStringCreate(kCFAllocatorDefault, string_ref, attributes);
    CFRelease(string_ref);
    CFRelease(attrs_mut);
    if (attr_string == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    line = CTLineCreateWithAttributedString(attr_string);
    CFRelease(attr_string);
    if (line == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
    CFRelease(line);

    *out_width = (M3Scalar)width;
    *out_height = (M3Scalar)(ascent + descent);
    *out_baseline = (M3Scalar)ascent;
    return M3_OK;
}

static int m3_cocoa_text_draw_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar x, M3Scalar y, M3Color color)
{
    struct M3CocoaBackend *backend;
    M3CocoaFont *resolved;
    CGContextRef ctx;
    CFStringRef string_ref;
    CFDictionaryRef attributes;
    CFAttributedStringRef attr_string;
    CFMutableDictionaryRef attrs_mut;
    CTLineRef line;
    CGAffineTransform transform;
    M3Bool apply_transform;
    int rc;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)text;
    rc = m3_cocoa_backend_resolve(backend, font, M3_COCOA_TYPE_FONT, (void **)&resolved);
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
    M3_COCOA_RETURN_IF_ERROR(rc);

    rc = m3_cocoa_color_validate(color);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (utf8_len == 0u) {
        return M3_OK;
    }

    if (resolved->font == NULL) {
        return M3_ERR_STATE;
    }

    rc = m3_cocoa_get_active_context(backend, &ctx);
    M3_COCOA_RETURN_IF_ERROR(rc);

    string_ref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)utf8, (CFIndex)utf8_len, kCFStringEncodingUTF8, false);
    if (string_ref == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    attrs_mut = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attrs_mut == NULL) {
        CFRelease(string_ref);
        return M3_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(attrs_mut, kCTFontAttributeName, resolved->font);
    attributes = attrs_mut;

    attr_string = CFAttributedStringCreate(kCFAllocatorDefault, string_ref, attributes);
    CFRelease(string_ref);
    CFRelease(attrs_mut);
    if (attr_string == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    line = CTLineCreateWithAttributedString(attr_string);
    CFRelease(attr_string);
    if (line == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    apply_transform = backend->has_transform ? M3_TRUE : M3_FALSE;
    if (apply_transform) {
        rc = m3_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        if (rc != M3_OK) {
            CFRelease(line);
            return rc;
        }
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);
    CGContextSetTextPosition(ctx, (CGFloat)x, (CGFloat)(y + CTFontGetAscent(resolved->font)));
    CTLineDraw(line, ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }

    CFRelease(line);
    return M3_OK;
}

static const M3TextVTable g_m3_cocoa_text_vtable = {
    m3_cocoa_text_create_font,
    m3_cocoa_text_destroy_font,
    m3_cocoa_text_measure_text,
    m3_cocoa_text_draw_text
};

static int m3_cocoa_io_read_file(void *io, const char *utf8_path, void *buffer, m3_usize buffer_size, m3_usize *out_read)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_read == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (buffer == NULL && buffer_size != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_read = 0u;
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_io_read_file_alloc(void *io, const char *utf8_path, const M3Allocator *allocator, void **out_data, m3_usize *out_size)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || allocator == NULL || out_data == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_data = NULL;
    *out_size = 0u;
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_io_write_file(void *io, const char *utf8_path, const void *data, m3_usize size, M3Bool overwrite)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(overwrite);

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (data == NULL && size != 0u) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_io_file_exists(void *io, const char *utf8_path, M3Bool *out_exists)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_exists == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_exists = M3_FALSE;
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_io_delete_file(void *io, const char *utf8_path)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_io_stat_file(void *io, const char *utf8_path, M3FileInfo *out_info)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_info == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)io;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
    M3_COCOA_RETURN_IF_ERROR(rc);

    memset(out_info, 0, sizeof(*out_info));
    return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_cocoa_io_vtable = {
    m3_cocoa_io_read_file,
    m3_cocoa_io_read_file_alloc,
    m3_cocoa_io_write_file,
    m3_cocoa_io_file_exists,
    m3_cocoa_io_delete_file,
    m3_cocoa_io_stat_file
};

static int m3_cocoa_sensors_is_available(void *sensors, m3_u32 type, M3Bool *out_available)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)sensors;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_available = M3_FALSE;
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_sensors_start(void *sensors, m3_u32 type)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)sensors;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_sensors_stop(void *sensors, m3_u32 type)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)sensors;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_sensors_read(void *sensors, m3_u32 type, M3SensorReading *out_reading, M3Bool *out_has_reading)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)sensors;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
    M3_COCOA_RETURN_IF_ERROR(rc);

    memset(out_reading, 0, sizeof(*out_reading));
    *out_has_reading = M3_FALSE;
    return M3_ERR_UNSUPPORTED;
}

static const M3SensorsVTable g_m3_cocoa_sensors_vtable = {
    m3_cocoa_sensors_is_available,
    m3_cocoa_sensors_start,
    m3_cocoa_sensors_stop,
    m3_cocoa_sensors_read
};

static int m3_cocoa_camera_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = (m3_usize)~(m3_usize)0;
    if (a != 0u && b > max_value / a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return M3_OK;
}

static int m3_cocoa_camera_select_device(const M3CameraConfig *config, AVCaptureDevice **out_device)
{
    NSArray *devices;
    AVCaptureDevice *device;
    AVCaptureDevicePosition desired;
    NSUInteger count;
    NSUInteger i;

    if (config == NULL || out_device == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_device = nil;
    devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    if (devices == nil) {
        return M3_ERR_NOT_FOUND;
    }

    count = [devices count];
    if (count == 0u) {
        return M3_ERR_NOT_FOUND;
    }

    if (config->camera_id != M3_CAMERA_ID_DEFAULT) {
        if (config->camera_id >= (m3_u32)count) {
            return M3_ERR_NOT_FOUND;
        }
        *out_device = [devices objectAtIndex:(NSUInteger)config->camera_id];
        return M3_OK;
    }

    if (config->facing == M3_CAMERA_FACING_UNSPECIFIED) {
        *out_device = [devices objectAtIndex:0u];
        return M3_OK;
    }

    desired = AVCaptureDevicePositionUnspecified;
    switch (config->facing) {
    case M3_CAMERA_FACING_FRONT:
        desired = AVCaptureDevicePositionFront;
        break;
    case M3_CAMERA_FACING_BACK:
        desired = AVCaptureDevicePositionBack;
        break;
    case M3_CAMERA_FACING_EXTERNAL:
    default:
        desired = AVCaptureDevicePositionUnspecified;
        break;
    }

    for (i = 0u; i < count; i += 1u) {
        device = [devices objectAtIndex:i];
        if (device != nil && [device position] == desired) {
            *out_device = device;
            return M3_OK;
        }
    }

    return M3_ERR_NOT_FOUND;
}

static int m3_cocoa_camera_apply_preset(AVCaptureSession *session, m3_u32 width, m3_u32 height)
{
    NSString *preset;
    m3_u32 target_width;
    m3_u32 target_height;
    M3Bool allow_fallback;

    if (session == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    allow_fallback = (width == 0u || height == 0u) ? M3_TRUE : M3_FALSE;
    target_width = (width == 0u) ? M3_COCOA_CAMERA_DEFAULT_WIDTH : width;
    target_height = (height == 0u) ? M3_COCOA_CAMERA_DEFAULT_HEIGHT : height;

    preset = nil;
    if (target_width == M3_COCOA_CAMERA_DEFAULT_WIDTH && target_height == M3_COCOA_CAMERA_DEFAULT_HEIGHT) {
        if ([session canSetSessionPreset:AVCaptureSessionPreset640x480]) {
            session.sessionPreset = AVCaptureSessionPreset640x480;
            return M3_OK;
        }
        if (allow_fallback && [session canSetSessionPreset:AVCaptureSessionPresetMedium]) {
            session.sessionPreset = AVCaptureSessionPresetMedium;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    } else if (target_width == 1280u && target_height == 720u) {
        preset = AVCaptureSessionPreset1280x720;
    } else if (target_width == 1920u && target_height == 1080u) {
        preset = AVCaptureSessionPreset1920x1080;
    } else if (target_width == 3840u && target_height == 2160u) {
        preset = AVCaptureSessionPreset3840x2160;
    } else {
        return M3_ERR_UNSUPPORTED;
    }

    if ([session canSetSessionPreset:preset]) {
        session.sessionPreset = preset;
        return M3_OK;
    }

    return M3_ERR_UNSUPPORTED;
}

static M3Bool m3_cocoa_camera_output_supports_format(AVCaptureVideoDataOutput *output, OSType pixel_format)
{
    NSArray *formats;
    NSNumber *value;
    NSUInteger count;
    NSUInteger i;

    if (output == nil) {
        return M3_FALSE;
    }

    formats = [output availableVideoCVPixelFormatTypes];
    if (formats == nil) {
        return M3_FALSE;
    }

    count = [formats count];
    for (i = 0u; i < count; i += 1u) {
        value = [formats objectAtIndex:i];
        if (value != nil && [value unsignedIntValue] == (unsigned int)pixel_format) {
            return M3_TRUE;
        }
    }

    return M3_FALSE;
}

static int m3_cocoa_camera_pick_pixel_format(AVCaptureVideoDataOutput *output, m3_u32 requested_format,
    OSType *out_format, M3Bool *out_swizzle)
{
    if (output == nil || out_format == NULL || out_swizzle == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_swizzle = M3_FALSE;

    if (requested_format == M3_CAMERA_FORMAT_ANY) {
        if (m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            *out_format = kCVPixelFormatType_32BGRA;
            return M3_OK;
        }
        if (m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return M3_OK;
        }
        if (m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    }

    if (requested_format == M3_CAMERA_FORMAT_BGRA8 || requested_format == M3_CAMERA_FORMAT_RGBA8) {
        if (!m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            return M3_ERR_UNSUPPORTED;
        }
        *out_format = kCVPixelFormatType_32BGRA;
        *out_swizzle = (requested_format == M3_CAMERA_FORMAT_RGBA8) ? M3_TRUE : M3_FALSE;
        return M3_OK;
    }

    if (requested_format == M3_CAMERA_FORMAT_NV12) {
        if (m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return M3_OK;
        }
        if (m3_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    }

    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_camera_update_frame(struct M3CocoaBackend *backend, CMSampleBufferRef sample)
{
    CVImageBufferRef image;
    CVPixelBufferRef pixel;
    OSType pixel_format;
    unsigned char *base_address;
    unsigned char *y_plane;
    unsigned char *uv_plane;
    size_t bytes_per_row;
    size_t row;
    size_t y_bytes_per_row;
    size_t uv_bytes_per_row;
    m3_usize width;
    m3_usize height;
    m3_usize row_bytes;
    m3_usize needed;
    m3_usize y_size;
    m3_usize uv_size;
    m3_usize col;
    int rc;
    int frame_rc;
    M3Bool swizzle;

    if (backend == NULL || sample == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    image = CMSampleBufferGetImageBuffer(sample);
    if (image == NULL) {
        return M3_ERR_NOT_READY;
    }

    pixel = (CVPixelBufferRef)image;
    pixel_format = CVPixelBufferGetPixelFormatType(pixel);

    if (CVPixelBufferLockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess) {
        return M3_ERR_IO;
    }

    base_address = NULL;
    y_plane = NULL;
    uv_plane = NULL;
    bytes_per_row = 0u;
    y_bytes_per_row = 0u;
    uv_bytes_per_row = 0u;
    width = 0u;
    height = 0u;
    row_bytes = 0u;
    needed = 0u;
    y_size = 0u;
    uv_size = 0u;
    swizzle = backend->camera_swizzle;
    frame_rc = M3_OK;
    if (pixel_format == kCVPixelFormatType_32BGRA) {
        base_address = (unsigned char *)CVPixelBufferGetBaseAddress(pixel);
        bytes_per_row = CVPixelBufferGetBytesPerRow(pixel);
        width = (m3_usize)CVPixelBufferGetWidth(pixel);
        height = (m3_usize)CVPixelBufferGetHeight(pixel);

        rc = m3_cocoa_camera_mul_overflow(width, (m3_usize)4u, &row_bytes);
        if (rc == M3_OK) {
            rc = m3_cocoa_camera_mul_overflow(row_bytes, height, &needed);
        }

        if (rc != M3_OK || base_address == NULL) {
            frame_rc = (rc == M3_OK) ? M3_ERR_IO : rc;
        } else {
            @synchronized (backend) {
                if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                    || backend->allocator.free == NULL) {
                    frame_rc = M3_ERR_INVALID_ARGUMENT;
                } else if (backend->camera_frame_capacity < needed) {
                    unsigned char *new_frame;
                    int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                        (void **)&new_frame);
                    if (alloc_rc != M3_OK) {
                        frame_rc = alloc_rc;
                    } else {
                        backend->camera_frame = new_frame;
                        backend->camera_frame_capacity = needed;
                    }
                }

                if (frame_rc == M3_OK) {
                    if (swizzle == M3_TRUE) {
                        unsigned char *src;
                        unsigned char *dst;
                        m3_usize idx;

                        for (row = 0u; row < height; row += 1u) {
                            src = base_address + row * bytes_per_row;
                            dst = backend->camera_frame + row * row_bytes;
                            for (col = 0u; col < width; col += 1u) {
                                idx = col * 4u;
                                dst[idx] = src[idx + 2u];
                                dst[idx + 1u] = src[idx + 1u];
                                dst[idx + 2u] = src[idx];
                                dst[idx + 3u] = src[idx + 3u];
                            }
                        }
                        backend->camera_format = M3_CAMERA_FORMAT_RGBA8;
                    } else {
                        if (bytes_per_row == (size_t)row_bytes) {
                            memcpy(backend->camera_frame, base_address, (size_t)needed);
                        } else {
                            for (row = 0u; row < height; row += 1u) {
                                memcpy(backend->camera_frame + row * row_bytes, base_address + row * bytes_per_row,
                                    (size_t)row_bytes);
                            }
                        }
                        backend->camera_format = M3_CAMERA_FORMAT_BGRA8;
                    }

                    backend->camera_width = (m3_u32)width;
                    backend->camera_height = (m3_u32)height;
                    backend->camera_frame_size = needed;
                    backend->camera_has_frame = M3_TRUE;
                }
                backend->camera_error = frame_rc;
            }
        }
    } else if (pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
        || pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
        if (!CVPixelBufferIsPlanar(pixel) || CVPixelBufferGetPlaneCount(pixel) < 2u) {
            frame_rc = M3_ERR_UNSUPPORTED;
        } else {
            y_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 0);
            uv_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 1);
            y_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 0);
            uv_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 1);
            width = (m3_usize)CVPixelBufferGetWidth(pixel);
            height = (m3_usize)CVPixelBufferGetHeight(pixel);

            rc = m3_cocoa_camera_mul_overflow(width, height, &y_size);
            if (rc == M3_OK) {
                uv_size = y_size / 2u;
                needed = y_size + uv_size;
            }

            if (rc != M3_OK || y_plane == NULL || uv_plane == NULL) {
                frame_rc = (rc == M3_OK) ? M3_ERR_IO : rc;
            } else {
                @synchronized (backend) {
                    if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                        || backend->allocator.free == NULL) {
                        frame_rc = M3_ERR_INVALID_ARGUMENT;
                    } else if (backend->camera_frame_capacity < needed) {
                        unsigned char *new_frame;
                        int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                            (void **)&new_frame);
                        if (alloc_rc != M3_OK) {
                            frame_rc = alloc_rc;
                        } else {
                            backend->camera_frame = new_frame;
                            backend->camera_frame_capacity = needed;
                        }
                    }

                    if (frame_rc == M3_OK) {
                        for (row = 0u; row < height; row += 1u) {
                            memcpy(backend->camera_frame + row * width,
                                y_plane + row * (m3_usize)y_bytes_per_row, (size_t)width);
                        }
                        for (row = 0u; row < height / 2u; row += 1u) {
                            memcpy(backend->camera_frame + y_size + row * width,
                                uv_plane + row * (m3_usize)uv_bytes_per_row, (size_t)width);
                        }

                        backend->camera_format = M3_CAMERA_FORMAT_NV12;
                        backend->camera_width = (m3_u32)width;
                        backend->camera_height = (m3_u32)height;
                        backend->camera_frame_size = needed;
                        backend->camera_has_frame = M3_TRUE;
                    }
                    backend->camera_error = frame_rc;
                }
            }
        }
    } else {
        frame_rc = M3_ERR_UNSUPPORTED;
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    if (frame_rc != M3_OK) {
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    CVPixelBufferUnlockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly);
    return frame_rc;
}

static int m3_cocoa_camera_open_with_config(void *camera, const M3CameraConfig *config);

static int m3_cocoa_camera_open(void *camera, m3_u32 camera_id)
{
    M3CameraConfig config;

    config.camera_id = camera_id;
    config.facing = M3_CAMERA_FACING_UNSPECIFIED;
    config.width = 0u;
    config.height = 0u;
    config.format = M3_CAMERA_FORMAT_ANY;

    return m3_cocoa_camera_open_with_config(camera, &config);
}

static int m3_cocoa_camera_open_with_config(void *camera, const M3CameraConfig *config)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (camera == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)camera;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened == M3_TRUE) {
        return M3_ERR_STATE;
    }

    @autoreleasepool {
        AVCaptureDevice *device;
        AVCaptureSession *session;
        AVCaptureDeviceInput *input;
        AVCaptureVideoDataOutput *output;
        NSDictionary *settings;
        M3CocoaCameraDelegate *delegate;
        dispatch_queue_t queue;
        NSError *error;
        OSType pixel_format;
        M3Bool swizzle;

        rc = m3_cocoa_camera_select_device(config, &device);
        if (rc != M3_OK) {
            return rc;
        }

        error = nil;
        input = [[AVCaptureDeviceInput alloc] initWithDevice:device error:&error];
        if (input == nil) {
            if (error != nil && [[error domain] isEqualToString:AVFoundationErrorDomain]) {
                if ([error code] == AVErrorApplicationIsNotAuthorizedToUseDevice) {
                    return M3_ERR_PERMISSION;
                }
                if ([error code] == AVErrorDeviceNotConnected) {
                    return M3_ERR_NOT_FOUND;
                }
                if ([error code] == AVErrorDeviceInUseByAnotherApplication) {
                    return M3_ERR_BUSY;
                }
            }
            return M3_ERR_UNKNOWN;
        }

        session = [[AVCaptureSession alloc] init];
        if (session == nil) {
#if !defined(M3_COCOA_ARC)
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        rc = m3_cocoa_camera_apply_preset(session, config->width, config->height);
        if (rc != M3_OK) {
#if !defined(M3_COCOA_ARC)
            [session release];
            [input release];
#endif
            return rc;
        }

        output = [[AVCaptureVideoDataOutput alloc] init];
        if (output == nil) {
#if !defined(M3_COCOA_ARC)
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        rc = m3_cocoa_camera_pick_pixel_format(output, config->format, &pixel_format, &swizzle);
        if (rc != M3_OK) {
#if !defined(M3_COCOA_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return rc;
        }

        settings = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:(unsigned int)pixel_format]
                                               forKey:(id)kCVPixelBufferPixelFormatTypeKey];
        output.videoSettings = settings;
        output.alwaysDiscardsLateVideoFrames = YES;

        delegate = [[M3CocoaCameraDelegate alloc] initWithBackend:backend];
        if (delegate == nil) {
#if !defined(M3_COCOA_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        queue = dispatch_queue_create("m3.cocoa.camera", DISPATCH_QUEUE_SERIAL);
        if (queue == NULL) {
#if !defined(M3_COCOA_ARC)
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        if (![session canAddInput:input] || ![session canAddOutput:output]) {
#if !defined(M3_COCOA_ARC)
            dispatch_release(queue);
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_UNSUPPORTED;
        }

        [session addInput:input];
        [session addOutput:output];
        [output setSampleBufferDelegate:delegate queue:queue];

        backend->camera_session = session;
        backend->camera_input = input;
        backend->camera_output = output;
        backend->camera_delegate = delegate;
        backend->camera_queue = queue;
        backend->camera_opened = M3_TRUE;
        backend->camera_streaming = M3_FALSE;
        backend->camera_has_frame = M3_FALSE;
        backend->camera_error = M3_OK;
        backend->camera_requested_format = config->format;
        backend->camera_swizzle = swizzle;
    }

    return M3_OK;
}

static int m3_cocoa_camera_close(void *camera)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)camera;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }

    if (backend->camera_streaming == M3_TRUE && backend->camera_session != nil) {
        [backend->camera_session stopRunning];
        backend->camera_streaming = M3_FALSE;
    }

    if (backend->camera_output != nil) {
        [backend->camera_output setSampleBufferDelegate:nil queue:NULL];
    }

    if (backend->camera_frame != NULL) {
        if (backend->allocator.free == NULL) {
            return M3_ERR_INVALID_ARGUMENT;
        }
        rc = backend->allocator.free(backend->allocator.ctx, backend->camera_frame);
        if (rc != M3_OK) {
            return rc;
        }
        backend->camera_frame = NULL;
        backend->camera_frame_capacity = 0u;
        backend->camera_frame_size = 0u;
    }

    if (backend->camera_delegate != nil) {
#if !defined(M3_COCOA_ARC)
        [backend->camera_delegate release];
#endif
        backend->camera_delegate = nil;
    }

    if (backend->camera_output != nil) {
#if !defined(M3_COCOA_ARC)
        [backend->camera_output release];
#endif
        backend->camera_output = nil;
    }

    if (backend->camera_input != nil) {
#if !defined(M3_COCOA_ARC)
        [backend->camera_input release];
#endif
        backend->camera_input = nil;
    }

    if (backend->camera_session != nil) {
#if !defined(M3_COCOA_ARC)
        [backend->camera_session release];
#endif
        backend->camera_session = nil;
    }

    if (backend->camera_queue != NULL) {
#if !defined(M3_COCOA_ARC)
        dispatch_release(backend->camera_queue);
#endif
        backend->camera_queue = NULL;
    }

    backend->camera_opened = M3_FALSE;
    backend->camera_streaming = M3_FALSE;
    backend->camera_has_frame = M3_FALSE;
    backend->camera_format = 0u;
    backend->camera_width = 0u;
    backend->camera_height = 0u;
    backend->camera_requested_format = 0u;
    backend->camera_swizzle = M3_FALSE;
    backend->camera_error = M3_OK;
    return M3_OK;
}

static int m3_cocoa_camera_start(void *camera)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)camera;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming == M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return M3_ERR_STATE;
    }

    [backend->camera_session startRunning];
    if (![backend->camera_session isRunning]) {
        return M3_ERR_UNKNOWN;
    }

    backend->camera_streaming = M3_TRUE;
    return M3_OK;
}

static int m3_cocoa_camera_stop(void *camera)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)camera;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return M3_ERR_STATE;
    }

    [backend->camera_session stopRunning];
    backend->camera_streaming = M3_FALSE;
    return M3_OK;
}

static int m3_cocoa_camera_read_frame(void *camera, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)camera;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }

    @synchronized (backend) {
        if (backend->camera_error != M3_OK) {
            rc = backend->camera_error;
            backend->camera_error = M3_OK;
            return rc;
        }

        if (backend->camera_has_frame != M3_TRUE || backend->camera_frame == NULL) {
            memset(out_frame, 0, sizeof(*out_frame));
            *out_has_frame = M3_FALSE;
            return M3_OK;
        }

        out_frame->format = backend->camera_format;
        out_frame->width = backend->camera_width;
        out_frame->height = backend->camera_height;
        out_frame->data = backend->camera_frame;
        out_frame->size = backend->camera_frame_size;
        *out_has_frame = M3_TRUE;
    }

    return M3_OK;
}

static const M3CameraVTable g_m3_cocoa_camera_vtable = {
    m3_cocoa_camera_open,
    m3_cocoa_camera_open_with_config,
    m3_cocoa_camera_close,
    m3_cocoa_camera_start,
    m3_cocoa_camera_stop,
    m3_cocoa_camera_read_frame
};

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_cocoa_network_error_from_ns_error(NSError *error)
{
    NSString *domain;
    NSInteger code;

    if (error == nil) {
        return M3_OK;
    }

    domain = [error domain];
    code = [error code];
    if ([domain isEqualToString:NSURLErrorDomain]) {
        switch (code) {
        case NSURLErrorTimedOut:
            return M3_ERR_TIMEOUT;
        case NSURLErrorBadURL:
            return M3_ERR_INVALID_ARGUMENT;
        case NSURLErrorUnsupportedURL:
            return M3_ERR_UNSUPPORTED;
        case NSURLErrorCannotFindHost:
        case NSURLErrorDNSLookupFailed:
            return M3_ERR_NOT_FOUND;
        case NSURLErrorUserAuthenticationRequired:
        case NSURLErrorUserCancelledAuthentication:
        case NSURLErrorSecureConnectionFailed:
            return M3_ERR_PERMISSION;
        case NSURLErrorCannotConnectToHost:
        case NSURLErrorNetworkConnectionLost:
        case NSURLErrorNotConnectedToInternet:
            return M3_ERR_IO;
        default:
            return M3_ERR_IO;
        }
    }

    return M3_ERR_IO;
}

#endif

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_cocoa_network_apply_headers(NSMutableURLRequest *request, const char *headers)
{
    const char *cursor;

    if (request == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return M3_OK;
    }

    cursor = headers;
    while (*cursor != '\0') {
        const char *line_start;
        const char *line_end;
        const char *colon;
        const char *key_start;
        const char *key_end;
        const char *value_start;
        const char *value_end;
        NSString *key;
        NSString *value;

        line_start = cursor;
        line_end = line_start;
        while (*line_end != '\0' && *line_end != '\r' && *line_end != '\n') {
            line_end++;
        }

        if (line_end != line_start) {
            colon = line_start;
            while (colon < line_end && *colon != ':') {
                colon++;
            }
            if (colon == line_end) {
                return M3_ERR_INVALID_ARGUMENT;
            }

            key_start = line_start;
            key_end = colon;
            while (key_start < key_end && (*key_start == ' ' || *key_start == '\t')) {
                key_start++;
            }
            while (key_end > key_start && (key_end[-1] == ' ' || key_end[-1] == '\t')) {
                key_end--;
            }

            value_start = colon + 1;
            value_end = line_end;
            while (value_start < value_end && (*value_start == ' ' || *value_start == '\t')) {
                value_start++;
            }
            while (value_end > value_start && (value_end[-1] == ' ' || value_end[-1] == '\t')) {
                value_end--;
            }

            if (key_start == key_end) {
                return M3_ERR_INVALID_ARGUMENT;
            }

            key = [[NSString alloc] initWithBytes:key_start
                                           length:(NSUInteger)(key_end - key_start)
                                         encoding:NSUTF8StringEncoding];
            if (key == nil) {
                return M3_ERR_INVALID_ARGUMENT;
            }
            value = [[NSString alloc] initWithBytes:value_start
                                             length:(NSUInteger)(value_end - value_start)
                                           encoding:NSUTF8StringEncoding];
            if (value == nil) {
#if !M3_COCOA_ARC
                [key release];
#endif
                return M3_ERR_INVALID_ARGUMENT;
            }

            [request setValue:value forHTTPHeaderField:key];

#if !M3_COCOA_ARC
            [value release];
            [key release];
#endif
        }

        if (*line_end == '\r' && line_end[1] == '\n') {
            cursor = line_end + 2;
        } else if (*line_end == '\r' || *line_end == '\n') {
            cursor = line_end + 1;
        } else {
            cursor = line_end;
        }
    }

    return M3_OK;
}

#endif

#if defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_cocoa_network_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
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

static int m3_cocoa_network_error_from_cf_stream(CFStreamError error)
{
    if (error.domain == 0) {
        return M3_ERR_IO;
    }
    if (error.domain == kCFStreamErrorDomainPOSIX) {
        if (error.error == ETIMEDOUT) {
            return M3_ERR_TIMEOUT;
        }
        return M3_ERR_IO;
    }
    return M3_ERR_IO;
}

static int m3_cocoa_network_apply_cf_headers(CFHTTPMessageRef message, const char *headers)
{
    const char *cursor;

    if (message == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return M3_OK;
    }

    cursor = headers;
    while (*cursor != '\0') {
        const char *line_start;
        const char *line_end;
        const char *colon;
        const char *key_start;
        const char *key_end;
        const char *value_start;
        const char *value_end;
        CFStringRef key;
        CFStringRef value;

        line_start = cursor;
        line_end = line_start;
        while (*line_end != '\0' && *line_end != '\r' && *line_end != '\n') {
            line_end++;
        }

        if (line_end != line_start) {
            colon = line_start;
            while (colon < line_end && *colon != ':') {
                colon++;
            }
            if (colon == line_end) {
                return M3_ERR_INVALID_ARGUMENT;
            }

            key_start = line_start;
            key_end = colon;
            while (key_start < key_end && (*key_start == ' ' || *key_start == '\t')) {
                key_start++;
            }
            while (key_end > key_start && (key_end[-1] == ' ' || key_end[-1] == '\t')) {
                key_end--;
            }

            value_start = colon + 1;
            value_end = line_end;
            while (value_start < value_end && (*value_start == ' ' || *value_start == '\t')) {
                value_start++;
            }
            while (value_end > value_start && (value_end[-1] == ' ' || value_end[-1] == '\t')) {
                value_end--;
            }

            if (key_start == key_end) {
                return M3_ERR_INVALID_ARGUMENT;
            }

            if ((m3_usize)(key_end - key_start) > (m3_usize)LONG_MAX
                    || (m3_usize)(value_end - value_start) > (m3_usize)LONG_MAX) {
                return M3_ERR_RANGE;
            }

            key = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)key_start,
                (CFIndex)(key_end - key_start), kCFStringEncodingUTF8, false);
            if (key == NULL) {
                return M3_ERR_INVALID_ARGUMENT;
            }
            value = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)value_start,
                (CFIndex)(value_end - value_start), kCFStringEncodingUTF8, false);
            if (value == NULL) {
                CFRelease(key);
                return M3_ERR_INVALID_ARGUMENT;
            }

            CFHTTPMessageSetHeaderFieldValue(message, key, value);
            CFRelease(value);
            CFRelease(key);
        }

        if (*line_end == '\r' && line_end[1] == '\n') {
            cursor = line_end + 2;
        } else if (*line_end == '\r' || *line_end == '\n') {
            cursor = line_end + 1;
        } else {
            cursor = line_end;
        }
    }

    return M3_OK;
}

static int m3_cocoa_network_request_cfnetwork(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
    struct M3CocoaBackend *backend;
    CFStringRef url_string;
    CFURLRef url;
    CFStringRef method;
    CFHTTPMessageRef message;
    CFDataRef body_data;
    CFReadStreamRef stream;
    CFHTTPMessageRef response_message;
    CFStreamError stream_error;
    CFIndex bytes_read;
    UInt8 buffer[4096];
    void *body;
    void *new_body;
    m3_usize total_size;
    m3_usize capacity;
    m3_usize alloc_size;
    m3_usize url_len;
    m3_usize method_len;
    int rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    url_len = (m3_usize)strlen(request->url);
    if (url_len > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }
    method_len = (m3_usize)strlen(request->method);
    if (method_len > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }
    if (request->body_size > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3CocoaBackend *)net;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
    M3_COCOA_RETURN_IF_ERROR(rc);

    memset(out_response, 0, sizeof(*out_response));

    url_string = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->url, (CFIndex)url_len,
        kCFStringEncodingUTF8, false);
    if (url_string == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    url = CFURLCreateWithString(kCFAllocatorDefault, url_string, NULL);
    CFRelease(url_string);
    if (url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    method = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->method, (CFIndex)method_len,
        kCFStringEncodingUTF8, false);
    if (method == NULL) {
        CFRelease(url);
        return M3_ERR_INVALID_ARGUMENT;
    }

    message = CFHTTPMessageCreateRequest(kCFAllocatorDefault, method, url, kCFHTTPVersion1_1);
    CFRelease(method);
    CFRelease(url);
    if (message == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    rc = m3_cocoa_network_apply_cf_headers(message, request->headers);
    if (rc != M3_OK) {
        CFRelease(message);
        return rc;
    }

    if (request->body_size > 0) {
        body_data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)request->body, (CFIndex)request->body_size);
        if (body_data == NULL) {
            CFRelease(message);
            return M3_ERR_OUT_OF_MEMORY;
        }
        CFHTTPMessageSetBody(message, body_data);
        CFRelease(body_data);
    }

    stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, message);
    CFRelease(message);
    if (stream == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    if (request->timeout_ms > 0u) {
        double seconds = ((double)request->timeout_ms) / 1000.0;
        CFNumberRef timeout = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &seconds);
        if (timeout != NULL) {
            CFReadStreamSetProperty(stream, kCFStreamPropertyReadTimeout, timeout);
            CFReadStreamSetProperty(stream, kCFStreamPropertyWriteTimeout, timeout);
            CFRelease(timeout);
        }
    }

    if (!CFReadStreamOpen(stream)) {
        stream_error = CFReadStreamGetError(stream);
        CFReadStreamClose(stream);
        CFRelease(stream);
        return m3_cocoa_network_error_from_cf_stream(stream_error);
    }

    response_message = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    if (response_message != NULL) {
        long status_code = CFHTTPMessageGetResponseStatusCode(response_message);
        if (status_code > 0) {
            out_response->status_code = (m3_u32)status_code;
        }
        CFRelease(response_message);
    }

    body = NULL;
    new_body = NULL;
    total_size = 0;
    capacity = 0;

    for (;;) {
        bytes_read = CFReadStreamRead(stream, buffer, (CFIndex)sizeof(buffer));
        if (bytes_read < 0) {
            stream_error = CFReadStreamGetError(stream);
            rc = m3_cocoa_network_error_from_cf_stream(stream_error);
            goto cleanup;
        }
        if (bytes_read == 0) {
            rc = M3_OK;
            break;
        }

        rc = m3_cocoa_network_add_overflow(total_size, (m3_usize)bytes_read, &alloc_size);
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

        memcpy((m3_u8 *)body + total_size, buffer, (size_t)bytes_read);
        total_size += (m3_usize)bytes_read;
    }

cleanup:
    CFReadStreamClose(stream);
    CFRelease(stream);

    if (rc != M3_OK) {
        if (body != NULL) {
            allocator->free(allocator->ctx, body);
        }
        memset(out_response, 0, sizeof(*out_response));
        return rc;
    }

    out_response->body = body;
    out_response->body_size = total_size;
    return M3_OK;
}

#endif

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_cocoa_network_request_foundation(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
    struct M3CocoaBackend *backend;
    NSString *url_string;
    NSURL *url;
    NSMutableURLRequest *url_request;
    NSString *method;
    NSData *body_data;
    NSURLResponse *response;
    NSHTTPURLResponse *http_response;
    NSError *error;
    NSData *data;
    void *body;
    m3_usize body_size;
    int rc;

    if (net == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)net;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (request == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(out_response, 0, sizeof(*out_response));

    url_string = [[NSString alloc] initWithBytes:request->url
                                          length:(NSUInteger)strlen(request->url)
                                        encoding:NSUTF8StringEncoding];
    if (url_string == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    url = [NSURL URLWithString:url_string];
    if (url == nil) {
#if !M3_COCOA_ARC
        [url_string release];
#endif
        return M3_ERR_INVALID_ARGUMENT;
    }

    url_request = [[NSMutableURLRequest alloc] initWithURL:url];
    if (url_request == nil) {
#if !M3_COCOA_ARC
        [url_string release];
#endif
        return M3_ERR_OUT_OF_MEMORY;
    }

    method = [[NSString alloc] initWithBytes:request->method
                                      length:(NSUInteger)strlen(request->method)
                                    encoding:NSUTF8StringEncoding];
    if (method == nil) {
#if !M3_COCOA_ARC
        [url_request release];
        [url_string release];
#endif
        return M3_ERR_INVALID_ARGUMENT;
    }
    [url_request setHTTPMethod:method];

    if (request->timeout_ms > 0u) {
        [url_request setTimeoutInterval:((NSTimeInterval)request->timeout_ms) / 1000.0];
    }

    rc = m3_cocoa_network_apply_headers(url_request, request->headers);
    if (rc != M3_OK) {
#if !M3_COCOA_ARC
        [method release];
        [url_request release];
        [url_string release];
#endif
        return rc;
    }

    body_data = nil;
    if (request->body_size > 0) {
        body_data = [[NSData alloc] initWithBytes:request->body length:(NSUInteger)request->body_size];
        if (body_data == nil) {
#if !M3_COCOA_ARC
            [method release];
            [url_request release];
            [url_string release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }
        [url_request setHTTPBody:body_data];
    }

    response = nil;
    error = nil;
    data = [NSURLConnection sendSynchronousRequest:url_request returningResponse:&response error:&error];

#if !M3_COCOA_ARC
    if (body_data != nil) {
        [body_data release];
    }
    [method release];
    [url_request release];
    [url_string release];
#endif

    if (error != nil) {
        return m3_cocoa_network_error_from_ns_error(error);
    }

    out_response->status_code = 0u;
    if (response != nil && [response isKindOfClass:[NSHTTPURLResponse class]]) {
        http_response = (NSHTTPURLResponse *)response;
        if ([http_response statusCode] > 0) {
            out_response->status_code = (m3_u32)[http_response statusCode];
        }
    }

    body = NULL;
    body_size = 0u;
    if (data != nil && [data length] > 0) {
        body_size = (m3_usize)[data length];
        rc = allocator->alloc(allocator->ctx, body_size, &body);
        if (rc != M3_OK) {
            out_response->status_code = 0u;
            out_response->body = NULL;
            out_response->body_size = 0u;
            return rc;
        }
        memcpy(body, [data bytes], (size_t)body_size);
    }

    out_response->body = body;
    out_response->body_size = body_size;
    return M3_OK;
}

#endif

static int m3_cocoa_network_request(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
#if defined(M3_APPLE_USE_CFNETWORK_C)
    return m3_cocoa_network_request_cfnetwork(net, request, allocator, out_response);
#else
    return m3_cocoa_network_request_foundation(net, request, allocator, out_response);
#endif
}

static int m3_cocoa_network_free_response(void *net, const M3Allocator *allocator, M3NetworkResponse *response)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)net;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (response->body_size > 0 && response->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
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

static const M3NetworkVTable g_m3_cocoa_network_vtable = {
    m3_cocoa_network_request,
    m3_cocoa_network_free_response
};

static int m3_cocoa_tasks_thread_create(void *tasks, M3ThreadFn entry, void *user, M3Handle *out_thread)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(entry);
    M3_UNUSED(user);

    if (tasks == NULL || out_thread == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_thread_join(void *tasks, M3Handle thread)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(thread);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_mutex_create(void *tasks, M3Handle *out_mutex)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (tasks == NULL || out_mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_mutex_destroy(void *tasks, M3Handle mutex)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_mutex_lock(void *tasks, M3Handle mutex)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_mutex_unlock(void *tasks, M3Handle mutex)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
    M3_COCOA_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_cocoa_tasks_sleep_ms(void *tasks, m3_u32 ms)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(ms);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
    M3_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return M3_OK;
}

static int m3_cocoa_tasks_post(void *tasks, M3TaskFn fn, void *user)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
    M3_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static int m3_cocoa_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user, m3_u32 delay_ms)
{
    struct M3CocoaBackend *backend;
    int rc;

    M3_UNUSED(delay_ms);

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)tasks;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
    M3_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static const M3TasksVTable g_m3_cocoa_tasks_vtable = {
    m3_cocoa_tasks_thread_create,
    m3_cocoa_tasks_thread_join,
    m3_cocoa_tasks_mutex_create,
    m3_cocoa_tasks_mutex_destroy,
    m3_cocoa_tasks_mutex_lock,
    m3_cocoa_tasks_mutex_unlock,
    m3_cocoa_tasks_sleep_ms,
    m3_cocoa_tasks_post,
    m3_cocoa_tasks_post_delayed
};

static int m3_cocoa_env_get_io(void *env, M3IO *out_io)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_io == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_io = backend->io;
    return M3_OK;
}

static int m3_cocoa_env_get_sensors(void *env, M3Sensors *out_sensors)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_sensors = backend->sensors;
    return M3_OK;
}

static int m3_cocoa_env_get_camera(void *env, M3Camera *out_camera)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_camera = backend->camera;
    return M3_OK;
}

static int m3_cocoa_env_get_network(void *env, M3Network *out_network)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_network == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_network = backend->network;
    return M3_OK;
}

static int m3_cocoa_env_get_tasks(void *env, M3Tasks *out_tasks)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
    M3_COCOA_RETURN_IF_ERROR(rc);

    *out_tasks = backend->tasks;
    return M3_OK;
}

static int m3_cocoa_env_get_time_ms(void *env, m3_u32 *out_time_ms)
{
    struct M3CocoaBackend *backend;
    int rc;

    if (env == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3CocoaBackend *)env;
    rc = m3_cocoa_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
    M3_COCOA_RETURN_IF_ERROR(rc);

    return m3_cocoa_time_now_ms(backend, out_time_ms);
}

static const M3EnvVTable g_m3_cocoa_env_vtable = {
    m3_cocoa_env_get_io,
    m3_cocoa_env_get_sensors,
    m3_cocoa_env_get_camera,
    m3_cocoa_env_get_network,
    m3_cocoa_env_get_tasks,
    m3_cocoa_env_get_time_ms
};

int M3_CALL m3_cocoa_backend_create(const M3CocoaBackendConfig *config, M3CocoaBackend **out_backend)
{
    M3CocoaBackendConfig local_config;
    M3Allocator allocator;
    struct M3CocoaBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_cocoa_backend_config_init(&local_config);
        M3_COCOA_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_cocoa_backend_validate_config(config);
    M3_COCOA_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        M3_COCOA_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(M3CocoaBackend), (void **)&backend);
    M3_COCOA_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;
    backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
    backend->inline_tasks = config->inline_tasks ? M3_TRUE : M3_FALSE;
    backend->clipboard_limit = config->clipboard_limit;
    m3_cocoa_event_queue_init(backend);
    backend->last_text = NULL;
    backend->time_origin = CFAbsoluteTimeGetCurrent();

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

    backend->ws.ctx = backend;
    backend->ws.vtable = &g_m3_cocoa_ws_vtable;
    backend->gfx.ctx = backend;
    backend->gfx.vtable = &g_m3_cocoa_gfx_vtable;
    backend->gfx.text_vtable = &g_m3_cocoa_text_vtable;
    backend->env.ctx = backend;
    backend->env.vtable = &g_m3_cocoa_env_vtable;
    backend->io.ctx = backend;
    backend->io.vtable = &g_m3_cocoa_io_vtable;
    backend->sensors.ctx = backend;
    backend->sensors.vtable = &g_m3_cocoa_sensors_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_m3_cocoa_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_m3_cocoa_network_vtable;
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_m3_cocoa_tasks_vtable;
    backend->initialized = M3_TRUE;

    *out_backend = backend;
    return M3_OK;
}

int M3_CALL m3_cocoa_backend_destroy(M3CocoaBackend *backend)
{
    int rc;
    int first_error;
    m3_usize i;
    m3_usize idx;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }

    first_error = M3_OK;

    m3_cocoa_release_last_text(backend);
    for (i = 0u; i < backend->event_count; i += 1u) {
        idx = (backend->event_head + i) % M3_COCOA_EVENT_CAPACITY;
        m3_cocoa_free_event_text(backend, &backend->event_queue[idx]);
    }

    rc = m3_handle_system_default_destroy(&backend->handles);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    if (backend->frame_ctx != NULL) {
        CGContextRelease(backend->frame_ctx);
        backend->frame_ctx = NULL;
    }

    if (backend->frame_data != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, backend->frame_data);
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
        backend->frame_data = NULL;
    }

    if (backend->camera_opened == M3_TRUE) {
        rc = m3_cocoa_camera_close(backend);
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
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

int M3_CALL m3_cocoa_backend_get_ws(M3CocoaBackend *backend, M3WS *out_ws)
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

int M3_CALL m3_cocoa_backend_get_gfx(M3CocoaBackend *backend, M3Gfx *out_gfx)
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

int M3_CALL m3_cocoa_backend_get_env(M3CocoaBackend *backend, M3Env *out_env)
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

#endif
