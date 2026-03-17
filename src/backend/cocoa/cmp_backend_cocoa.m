#include "cmpc/cmp_backend_cocoa.h"
#include "cmpc/cmp_a11y.h"
#include <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>

/* --- Helpers for Color --- */

/* Helper to extract color components from CMPColor struct */
static void get_cg_color_components(CMPColor c, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a) {
    /* Fix: Access struct members directly as floats or bytes depending on definition. */
    /* Assuming standard { uint8_t r, g, b, a; } based on build error context. */
    *r = (CGFloat)c.r / 255.0f;
    *g = (CGFloat)c.g / 255.0f;
    *b = (CGFloat)c.b / 255.0f;
    *a = (CGFloat)c.a / 255.0f;
}

/* --- Backend Struct Definition --- */

struct CMPCocoaBackend {
    CMPAllocator allocator;
    NSObject *lock; /* Used for @synchronized */

    /* Window System State */
    /* Typed as generic NSMutableArray to store NSWindow* or NSNull* */
    NSMutableArray *windows;

    /* GFX State */
    NSGraphicsContext *currentContext; /* Set between begin/end frame */
    CGContextRef cgContext;

    /* Time State */
    NSTimeInterval startTime;
};

/* --- Window System Implementation --- */

static int ws_init(void *ctx, const CMPWSConfig *config) {
    (void)config;
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;

    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps:YES];

        backend->startTime = [[NSDate date] timeIntervalSince1970];
    }
    return CMP_OK;
}

static int ws_shutdown(void *ctx) {
    (void)ctx;
    return CMP_OK;
}

static int ws_create_window(void *ctx, const CMPWSWindowConfig *config, CMPHandle *out_window) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        NSRect frame = NSMakeRect(0, 0, config->width, config->height);
        NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

        if (config->flags & CMP_WS_WINDOW_RESIZABLE) style |= NSWindowStyleMaskResizable;

        NSWindow *win = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:style
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
        if (config->utf8_title) {
            [win setTitle:[NSString stringWithUTF8String:config->utf8_title]];
        }
        [win center];

        /* Ensure the window has a valid content view for drawing */
        NSView *contentView = [[NSView alloc] initWithFrame:frame];
        [win setContentView:contentView];

        /* Apply macOS Visual Effect Materials if requested by CMP_WS_BACKDROP_* */
        if (config->backdrop_type != CMP_WS_BACKDROP_NONE) {
            NSVisualEffectView *effectView = [[NSVisualEffectView alloc] initWithFrame:frame];
            effectView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            effectView.blendingMode = NSVisualEffectBlendingModeBehindWindow;
            effectView.state = NSVisualEffectStateActive;
            
            if (config->backdrop_type == CMP_WS_BACKDROP_MICA || config->backdrop_type == CMP_WS_BACKDROP_MICA_ALT) {
                effectView.material = NSVisualEffectMaterialWindowBackground;
            } else if (config->backdrop_type == CMP_WS_BACKDROP_ACRYLIC) {
                effectView.material = NSVisualEffectMaterialPopover;
            } else {
                effectView.material = NSVisualEffectMaterialAppearanceBased;
            }
            
            [contentView addSubview:effectView];
            win.backgroundColor = [NSColor clearColor];
            win.opaque = NO;
        }

        [backend->windows addObject:win];
        out_window->id = (cmp_u32)(backend->windows.count - 1);
        out_window->generation = 1;
    }
    return CMP_OK;
}

static int ws_destroy_window(void *ctx, CMPHandle window) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        if (window.id < backend->windows.count) {
            id win = backend->windows[window.id];
            if (win != [NSNull null]) {
                [win close];
                [backend->windows replaceObjectAtIndex:window.id withObject:[NSNull null]];
            }
        }
    }
    return CMP_OK;
}

static int ws_show_window(void *ctx, CMPHandle window) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        if (window.id < backend->windows.count) {
            id win = backend->windows[window.id];
            if (win != [NSNull null]) {
                [win makeKeyAndOrderFront:nil];
            }
        }
    }
    return CMP_OK;
}

static int ws_hide_window(void *ctx, CMPHandle window) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        if (window.id < backend->windows.count) {
             id win = backend->windows[window.id];
            if (win != [NSNull null]) {
                [win orderOut:nil];
            }
        }
    }
    return CMP_OK;
}

static int ws_poll_event(void *ctx, CMPInputEvent *out_event, CMPBool *out_has_event) {
    (void)ctx;
    (void)out_event;
    *out_has_event = CMP_FALSE;
    return CMP_OK;
}

static int ws_set_window_title(void *ctx, CMPHandle window, const char *title) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        if (window.id < backend->windows.count) {
            id win = backend->windows[window.id];
            if (win != [NSNull null] && title) {
                [win setTitle:[NSString stringWithUTF8String:title]];
            }
        }
    }
    return CMP_OK;
}

static int ws_set_window_size(void *ctx, CMPHandle window, int w, int h) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    @synchronized (backend->lock) {
        if (window.id < backend->windows.count) {
            id win = backend->windows[window.id];
            if (win != [NSNull null]) {
                NSRect frame = [win frame];
                frame.size = NSMakeSize(w, h);
                NSRect contentRect = [win contentRectForFrameRect:frame];
                [win setContentSize:contentRect.size];
            }
        }
    }
    return CMP_OK;
}

static int ws_get_window_size(void *ctx, CMPHandle window, int *w, int *h) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (window.id < backend->windows.count) {
        id win = backend->windows[window.id];
        if (win != [NSNull null]) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            *w = (int)content.size.width;
            *h = (int)content.size.height;
        }
    }
    return CMP_OK;
}

static int ws_set_window_dpi(void *ctx, CMPHandle window, float scale) {
    (void)ctx; (void)window; (void)scale;
    return CMP_OK;
}

static int ws_get_window_dpi(void *ctx, CMPHandle window, float *scale) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (window.id < backend->windows.count) {
        id win = backend->windows[window.id];
        if (win != [NSNull null]) {
            *scale = (float)[win backingScaleFactor];
            return CMP_OK;
        }
    }
    *scale = 1.0f;
    return CMP_OK;
}

static int ws_set_clipboard(void *ctx, const char *text) {
    (void)ctx;
    if (!text) return CMP_ERR_INVALID_ARGUMENT;
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    [pasteboard setString:[NSString stringWithUTF8String:text] forType:NSPasteboardTypeString];
    return CMP_OK;
}

static int ws_get_clipboard(void *ctx, char *buf, size_t size, size_t *len) {
    (void)ctx;
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSString *str = [pasteboard stringForType:NSPasteboardTypeString];

    if (!str) {
        *len = 0;
        return CMP_OK;
    }

    const char *utf8 = [str UTF8String];
    size_t utf8Len = strlen(utf8);
    *len = utf8Len;

    if (buf && size > 0) {
#if defined(_MSC_VER)
        strncpy_s(buf, size, utf8, _TRUNCATE);
#else
        CMP_STRNCPY(buf, utf8, size - 1);
        buf[size - 1] = '\0';
#endif
    }

    return CMP_OK;
}

static int ws_pump(void *ctx) {
    (void)ctx;
    @autoreleasepool {
        while (true) {
            NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (!event) break;
            [NSApp sendEvent:event];
        }
        [NSApp updateWindows];
    }
    return CMP_OK;
}

static int ws_get_time(void *ctx, uint32_t *ms) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
    *ms = (uint32_t)((now - backend->startTime) * 1000.0);
    return CMP_OK;
}


static int cmp_backend_ws_get_system_color(void *ws, cmp_u32 color_type, CMPScalar *out_r, CMPScalar *out_g, CMPScalar *out_b, CMPScalar *out_a) {
  (void)ws;
  if (out_r == NULL || out_g == NULL || out_b == NULL || out_a == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  
  @autoreleasepool {
    if (color_type == CMP_SYSTEM_COLOR_ACCENT) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
      if (@available(macOS 10.14, *)) {
        NSColor *color = [[NSColor controlAccentColor] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        if (color) {
          CGFloat r, g, b, a;
          [color getRed:&r green:&g blue:&b alpha:&a];
          *out_r = (CMPScalar)r;
          *out_g = (CMPScalar)g;
          *out_b = (CMPScalar)b;
          *out_a = (CMPScalar)a;
          return CMP_OK;
        }
      }
#endif
      /* Fallback to generic blue */
      NSColor *color = [[NSColor selectedControlColor] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
      if (color) {
        CGFloat r, g, b, a;
        [color getRed:&r green:&g blue:&b alpha:&a];
        *out_r = (CMPScalar)r;
        *out_g = (CMPScalar)g;
        *out_b = (CMPScalar)b;
        *out_a = (CMPScalar)a;
        return CMP_OK;
      }
    } else if (color_type == CMP_SYSTEM_COLOR_BACKGROUND) {
      NSColor *color = [[NSColor windowBackgroundColor] colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
      if (color) {
        CGFloat r, g, b, a;
        [color getRed:&r green:&g blue:&b alpha:&a];
        *out_r = (CMPScalar)r;
        *out_g = (CMPScalar)g;
        *out_b = (CMPScalar)b;
        *out_a = (CMPScalar)a;
        return CMP_OK;
      }
    }
  }
  
  return CMP_ERR_UNSUPPORTED;
}


static NSAccessibilityRole get_cocoa_role(cmp_u32 role) {
    switch (role) {
        case 1: return NSAccessibilityButtonRole; /* BUTTON */
        case 2: return NSAccessibilityStaticTextRole; /* TEXT */
        case 3: return NSAccessibilityImageRole; /* IMAGE */
        case 4: return NSAccessibilitySliderRole; /* SLIDER */
        case 5: return NSAccessibilityCheckBoxRole; /* CHECKBOX */
        case 6: return NSAccessibilityCheckBoxRole; /* SWITCH */
        case 7: return NSAccessibilityRadioButtonRole; /* RADIO */
        case 8: return NSAccessibilityTextFieldRole; /* TEXT_FIELD */
        case 9: return NSAccessibilityProgressIndicatorRole; /* PROGRESS */
        case 10: return NSAccessibilityWindowRole; /* DIALOG */
        case 11: return NSAccessibilityMenuRole; /* MENU */
        case 12: return NSAccessibilityListRole; /* LIST */
        case 13: return NSAccessibilityRowRole; /* LIST_ITEM */
        case 14: return NSAccessibilityTabGroupRole; /* TAB_BAR */
        case 15: return NSAccessibilityRadioButtonRole; /* TAB */
        case 16: return NSAccessibilityStaticTextRole; /* HEADER */
        case 17: return NSAccessibilityGroupRole; /* CONTAINER */
        default: return NSAccessibilityUnknownRole;
    }
}

static NSAccessibilityElement* create_a11y_element(const void *node_ptr) {
    const CMPA11yNode *node = (const CMPA11yNode *)node_ptr;
    if (node == NULL) return nil;

    NSAccessibilityElement *el = [NSAccessibilityElement accessibilityElementWithRole:get_cocoa_role(node->semantics.role)
                                                                           frame:NSZeroRect
                                                                           label:node->semantics.utf8_label ? [NSString stringWithUTF8String:node->semantics.utf8_label] : @""
                                                                           parent:nil];
    if (node->semantics.utf8_value) {
        [el accessibilitySetValue:[NSString stringWithUTF8String:node->semantics.utf8_value] forAttribute:NSAccessibilityValueAttribute];
    }
    if (node->semantics.utf8_hint) {
        [el accessibilitySetValue:[NSString stringWithUTF8String:node->semantics.utf8_hint] forAttribute:NSAccessibilityHelpAttribute];
    }
    
    NSMutableArray *children = [NSMutableArray arrayWithCapacity:node->child_count];
    for (cmp_usize i = 0; i < node->child_count; i++) {
        NSAccessibilityElement *child = create_a11y_element(node->children[i]);
        if (child) {
            [child accessibilitySetOverrideValue:el forAttribute:NSAccessibilityParentAttribute];
            [children addObject:child];
        }
    }
    if (children.count > 0) {
        [el accessibilitySetOverrideValue:children forAttribute:NSAccessibilityChildrenAttribute];
    }
    return el;
}

static int cmp_backend_ws_update_a11y_tree(void *ws, const void *root_a11y_node) {
  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  CMPCocoaBackend *backend = (CMPCocoaBackend*)ws;
  if (backend->windows.count > 0) {
      id win = backend->windows[0];
      if (win != [NSNull null]) {
          NSView *view = [win contentView];
          NSAccessibilityElement *rootEl = create_a11y_element(root_a11y_node);
          if (rootEl) {
              [rootEl accessibilitySetOverrideValue:view forAttribute:NSAccessibilityParentAttribute];
              [view setAccessibilityChildren:@[rootEl]];
          }
      }
  }
  return CMP_OK;
}

static const CMPWSVTable g_ws_vtable = {
    ws_init, ws_shutdown, ws_create_window, ws_destroy_window,
    ws_show_window, ws_hide_window, ws_set_window_title,
    ws_set_window_size, ws_get_window_size,
    ws_set_window_dpi, ws_get_window_dpi,
    ws_set_clipboard, ws_get_clipboard,
    ws_poll_event, ws_pump, ws_get_time,
    cmp_backend_ws_get_system_color, cmp_backend_ws_update_a11y_tree
};

/* --- Graphics Implementation (CoreGraphics) --- */

static int gfx_begin_frame(void *ctx, CMPHandle w, int width, int height, float dpi) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    (void)width; (void)height; (void)dpi;

    if (w.id < backend->windows.count) {
        id win = backend->windows[w.id];
        if (win != [NSNull null]) {
            NSView *view = [win contentView];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            /* Immediate mode lock for legacy C-style drawing */
            [view lockFocus];
#pragma clang diagnostic pop

            backend->currentContext = [NSGraphicsContext currentContext];
            backend->cgContext = [backend->currentContext CGContext];

            CGContextSaveGState(backend->cgContext);
            /* Flip Y for standard 2D coords */
            CGContextTranslateCTM(backend->cgContext, 0, [view bounds].size.height);
            CGContextScaleCTM(backend->cgContext, 1.0, -1.0);
            return CMP_OK;
        }
    }
    return CMP_ERR_INVALID_ARGUMENT;
}

static int gfx_end_frame(void *ctx, CMPHandle w) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (backend->cgContext) {
        CGContextRestoreGState(backend->cgContext);
        backend->cgContext = NULL;
    }
    backend->currentContext = nil;

    if (w.id < backend->windows.count) {
        id win = backend->windows[w.id];
        if (win != [NSNull null]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            [[win contentView] unlockFocus];
#pragma clang diagnostic pop
            [[win contentView] setNeedsDisplay:YES];
        }
    }
    return CMP_OK;
}

static int gfx_clear(void *ctx, CMPColor c) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (!backend->cgContext) return CMP_ERR_INVALID_ARGUMENT;

    CGFloat r, g, b, a;
    get_cg_color_components(c, &r, &g, &b, &a);

    CGContextSetRGBFillColor(backend->cgContext, r, g, b, a);
    CGRect clip = CGContextGetClipBoundingBox(backend->cgContext);
    CGContextFillRect(backend->cgContext, clip);

    return CMP_OK;
}

static int gfx_draw_rect(void *ctx, const CMPRect *r, CMPColor c, float rad) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (!backend->cgContext) return CMP_ERR_INVALID_ARGUMENT;

    CGFloat red, green, blue, alpha;
    get_cg_color_components(c, &red, &green, &blue, &alpha);
    CGContextSetRGBFillColor(backend->cgContext, red, green, blue, alpha);

    /* FIX: access via width/height */
    CGRect cgRect = CGRectMake(r->x, r->y, r->width, r->height);

    if (rad > 0.0f) {
        /* FIX: Use NSBezierPath (AppKit) instead of UIBezierPath (UIKit) */
        NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:cgRect xRadius:rad yRadius:rad];
        [path fill];
    } else {
        CGContextFillRect(backend->cgContext, cgRect);
    }
    return CMP_OK;
}

static int gfx_draw_line(void *ctx, float x1, float y1, float x2, float y2, CMPColor c, float th) {
    CMPCocoaBackend *backend = (CMPCocoaBackend*)ctx;
    if (!backend->cgContext) return CMP_ERR_INVALID_ARGUMENT;

    CGFloat red, green, blue, alpha;
    get_cg_color_components(c, &red, &green, &blue, &alpha);
    CGContextSetRGBStrokeColor(backend->cgContext, red, green, blue, alpha);
    CGContextSetLineWidth(backend->cgContext, th);

    CGContextBeginPath(backend->cgContext);
    CGContextMoveToPoint(backend->cgContext, x1, y1);
    CGContextAddLineToPoint(backend->cgContext, x2, y2);
    CGContextStrokePath(backend->cgContext);

    return CMP_OK;
}

static int gfx_draw_path(void *ctx, const CMPPath *p, CMPColor c) {
    (void)ctx; (void)p; (void)c;
    return CMP_ERR_UNSUPPORTED;
}

static int gfx_push_clip(void *ctx, const CMPRect *r) {
    CMPCocoaBackend *b = (CMPCocoaBackend*)ctx;
    if(!b->cgContext) return CMP_ERR_INVALID_ARGUMENT;
    CGContextSaveGState(b->cgContext);
    /* FIX: width/height */
    CGRect clip = CGRectMake(r->x, r->y, r->width, r->height);
    CGContextClipToRect(b->cgContext, clip);
    return CMP_OK;
}

static int gfx_pop_clip(void *ctx) {
    CMPCocoaBackend *b = (CMPCocoaBackend*)ctx;
    if(!b->cgContext) return CMP_ERR_INVALID_ARGUMENT;
    CGContextRestoreGState(b->cgContext);
    return CMP_OK;
}

static int gfx_set_transform(void *ctx, const CMPMat3 *m) {
    (void)ctx; (void)m;
    return CMP_OK;
}

static int gfx_create_tex(void *ct, int w, int h, uint32_t f, const void *p, size_t s, CMPHandle *out) {
    (void)ct; (void)w; (void)h; (void)f; (void)p; (void)s; (void)out;
    return CMP_ERR_UNSUPPORTED;
}
static int gfx_update_tex(void *ct, CMPHandle t, int x, int y, int w, int h, const void *p, size_t s) {
    (void)ct; (void)t; (void)x; (void)y; (void)w; (void)h; (void)p; (void)s;
    return CMP_ERR_UNSUPPORTED;
}
static int gfx_destroy_tex(void *ct, CMPHandle t) {
    (void)ct; (void)t;
    return CMP_ERR_UNSUPPORTED;
}
static int gfx_draw_tex(void *ct, CMPHandle t, const CMPRect *s, const CMPRect *d, float o) {
    (void)ct; (void)t; (void)s; (void)d; (void)o;
    return CMP_ERR_UNSUPPORTED;
}

/* Text stubs */
static int txt_measure(void *c, CMPHandle f, const char *u, size_t l, float *w, float *h, float *b) {
    (void)c; (void)f;
    NSString *text = [[NSString alloc] initWithBytes:u length:l encoding:NSUTF8StringEncoding];
    NSFont *font = [NSFont systemFontOfSize:12.0];

    NSDictionary *attrs = @{NSFontAttributeName: font};
    NSSize size = [text sizeWithAttributes:attrs];
    *w = (float)size.width;
    *h = (float)size.height;
    *b = (float)[font ascender];
    return CMP_OK;
}

static int txt_create(void*c, const char*n, int s, int w, int i, CMPHandle *o) {
    (void)c; (void)n; (void)s; (void)w; (void)i;
    o->id = 1;
    return CMP_OK;
}
static int txt_destroy(void*c, CMPHandle f) { (void)c; (void)f; return CMP_OK; }

static int txt_draw(void*c, CMPHandle f, const char*u, size_t l, float x, float y, CMPColor col) {
    (void)f;
    CMPCocoaBackend *backend = (CMPCocoaBackend*)c;
    if (!backend->cgContext) return CMP_ERR_INVALID_ARGUMENT;

    NSString *text = [[NSString alloc] initWithBytes:u length:l encoding:NSUTF8StringEncoding];

    CGFloat red, g, b, a;
    get_cg_color_components(col, &red, &g, &b, &a);
    NSColor *nsCol = [NSColor colorWithRed:red green:g blue:b alpha:a];

    NSFont *font = [NSFont systemFontOfSize:12.0];
    NSDictionary *attrs = @{ NSFontAttributeName: font, NSForegroundColorAttributeName: nsCol };

    [text drawAtPoint:NSMakePoint(x, y) withAttributes:attrs];

    return CMP_OK;
}

static const CMPTextVTable g_text_vtable = {
    txt_create, txt_destroy, txt_measure, txt_draw
};

static const CMPGfxVTable g_gfx_vtable = {
    gfx_begin_frame, gfx_end_frame, gfx_clear, gfx_draw_rect, gfx_draw_line,
    gfx_draw_path, gfx_push_clip, gfx_pop_clip, gfx_set_transform,
    gfx_create_tex, gfx_update_tex, gfx_destroy_tex, gfx_draw_tex
};

/* --- Environment Implementation --- */

static int env_cam_open_auto(void *ctx, const CMPCameraConfig *cfg) {
    (void)ctx; (void)cfg;
#if defined(__MAC_10_15) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_15
    if (@available(macOS 10.15, *)) {
        AVCaptureDeviceDiscoverySession *session = [AVCaptureDeviceDiscoverySession
            discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera]
                                  mediaType:AVMediaTypeVideo
                                   position:AVCaptureDevicePositionUnspecified];
        if(session.devices.count > 0) return CMP_OK;
    }
#endif
    return CMP_ERR_NOT_FOUND;
}

static CMPCameraVTable g_cam_vtable = {
    .open_with_config = env_cam_open_auto
};




static int env_get_io(void* c, CMPIO *io) { (void)c; (void)io; return CMP_ERR_UNSUPPORTED; }
static int env_get_sensors(void* c, CMPSensors *s) { (void)c; (void)s; return CMP_ERR_UNSUPPORTED; }
static int env_get_cam(void* c, CMPCamera *cam) {
    cam->ctx = c; cam->vtable = &g_cam_vtable; return CMP_OK;
}
static int env_get_img(void* c, CMPImage *img) { (void)c; (void)img; return CMP_ERR_UNSUPPORTED; }
static int env_get_vid(void* c, CMPVideo *v) { (void)c; (void)v; return CMP_ERR_UNSUPPORTED; }
static int env_get_aud(void* c, CMPAudio *a) { (void)c; (void)a; return CMP_ERR_UNSUPPORTED; }
static int env_get_tasks(void* c, CMPTasks *t) { (void)c; (void)t; return CMP_ERR_UNSUPPORTED; }
static int env_get_time(void* c, uint32_t *t) { return ws_get_time(c, t); }

static int CMP_CALL env_navigate_url(void *env, const char *utf8_url) {
  (void)env;
  (void)utf8_url;
  return CMP_ERR_UNSUPPORTED;
}

static int CMP_CALL env_get_arg(void *env, const char *key, const char **out_value) {
  (void)env;
  (void)key;
  if (out_value) *out_value = NULL;
  return CMP_ERR_NOT_FOUND;
}

static const CMPEnvVTable g_env_vtable = {
    env_get_io, env_get_sensors, env_get_cam, env_get_img,
    env_get_vid, env_get_aud, env_get_tasks, env_get_time,
    env_navigate_url, env_get_arg
};

/* --- Main Backend Entry Points --- */

int cmp_cocoa_backend_is_available(CMPBool *out_available) {
    *out_available = CMP_TRUE;
    return CMP_OK;
}

int cmp_cocoa_backend_config_init(CMPCocoaBackendConfig *config) {
    memset(config, 0, sizeof(*config));
    return CMP_OK;
}

int cmp_cocoa_backend_create(const CMPCocoaBackendConfig *config, CMPCocoaBackend **out_backend) {
    CMPCocoaBackend *be = malloc(sizeof(CMPCocoaBackend));
    if (!be) return CMP_ERR_OUT_OF_MEMORY;

    memset(be, 0, sizeof(*be));
    if (config && config->allocator) be->allocator = *config->allocator;

    be->lock = [[NSObject alloc] init];
    be->windows = [[NSMutableArray alloc] init];
    /* Placeholder objects to keep indices 0 valid if needed */
    [be->windows addObject:[NSNull null]];

    *out_backend = be;
    return CMP_OK;
}

int cmp_cocoa_backend_destroy(CMPCocoaBackend *backend) {
    if (backend) {
        backend->windows = nil;
        backend->lock = nil;
        free(backend);
    }
    return CMP_OK;
}

int cmp_cocoa_backend_get_ws(CMPCocoaBackend *backend, CMPWS *out_ws) {
    if (!out_ws) return CMP_ERR_INVALID_ARGUMENT;
    out_ws->ctx = backend;
    out_ws->vtable = &g_ws_vtable;
    return CMP_OK;
}

int cmp_cocoa_backend_get_gfx(CMPCocoaBackend *backend, CMPGfx *out_gfx) {
    if (!out_gfx) return CMP_ERR_INVALID_ARGUMENT;
    out_gfx->ctx = backend;
    out_gfx->vtable = &g_gfx_vtable;
    out_gfx->text_vtable = &g_text_vtable;
    return CMP_OK;
}

int cmp_cocoa_backend_get_env(CMPCocoaBackend *backend, CMPEnv *out_env) {
    if (!out_env) return CMP_ERR_INVALID_ARGUMENT;
    out_env->ctx = backend;
    out_env->vtable = &g_env_vtable;
    return CMP_OK;
}

#ifdef CMP_TESTING
int cmp_cocoa_backend_test_set_config_init_fail(CMPBool fail) { (void)fail; return CMP_OK; }
int cmp_cocoa_backend_test_validate_config(const CMPCocoaBackendConfig *config) { (void)config; return CMP_OK; }
#endif
