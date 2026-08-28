/**
 * @file ui_renderer_coregraphics.c
 * @brief ui_renderer_coregraphics.c implementation.
 */
#if defined(__APPLE__)
/** @brief internal */
#define GL_SILENCE_DEPRECATION
/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include "../include/ui_font_manager.h"
#include "ui_internal_mem.h"

#if defined(__APPLE__)
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_cg_fail;
/** @brief internal */
#define CGDataProviderCreateWithData(a, b, c, d) (g_mock_cg_fail == 1 ? NULL : CGDataProviderCreateWithData(a, b, c, d))
/** @brief internal */
#define CGFontCreateWithDataProvider(a) (g_mock_cg_fail == 2 ? NULL : CGFontCreateWithDataProvider(a))
/** @brief internal */
#define CTFontCreateWithGraphicsFont(a, b, c, d) (g_mock_cg_fail == 3 ? NULL : CTFontCreateWithGraphicsFont(a, b, c, d))
/** @brief internal */
#define CFStringCreateWithCString(a, b, c) (g_mock_cg_fail == 4 ? NULL : CFStringCreateWithCString(a, b, c))
/** @brief internal */
#define CGPathCreateMutable() (g_mock_cg_fail == 5 ? NULL : CGPathCreateMutable())
/** @brief internal */
#define CFAttributedStringCreate(a, b, c) (g_mock_cg_fail == 6 ? NULL : CFAttributedStringCreate(a, b, c))
/** @brief internal */
#define CTLineCreateWithAttributedString(a) (g_mock_cg_fail == 7 ? NULL : CTLineCreateWithAttributedString(a))
/** @cond */
#define ui_font_get_data(f, d, s) (g_mock_cg_fail == 8 ? (*(d) = (const unsigned char*)1, *(s) = 0, UI_ERROR_NONE) : ui_font_get_data(f, d, s))
/** @endcond */
#endif
/* clang-format on */

/* CoreGraphics Renderer Context */
/**
 * @struct cg_context
 * \brief cg_context
 */
struct cg_context {
  CGContextRef context; /**< context */
  int current_width;    /**< current_width */
  int current_height;   /**< current_height */
};

/**
 * @brief cg_begin_frame.
 * @param ctx Parameter ctx.
 * @param width Parameter width.
 * @param height Parameter height.
 * @return Return value.
 */
static ui_error_t cg_begin_frame(void *ctx, int width, int height) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  CGColorSpaceRef colorSpace;

  if (!cgc)
    return UI_ERROR_INVALID_ARGUMENT;

#include <stdio.h>
  printf("begin_frame: cw=%d w=%d ch=%d h=%d ctx=%p\n", cgc->current_width,
         width, cgc->current_height, height, (void *)cgc->context);

  /* Recreate offscreen context if dimensions change */
  if (cgc->current_width != width || cgc->current_height != height ||
      !cgc->context) {
    if (cgc->context) {
      CGContextRelease(cgc->context);
      cgc->context = NULL;
    }

    if (width > 0 && height > 0) {
      colorSpace = CGColorSpaceCreateDeviceRGB();
      cgc->context = CGBitmapContextCreate(NULL, (size_t)width, (size_t)height,
                                           8, (size_t)width * 4, colorSpace,
                                           kCGImageAlphaPremultipliedLast);
      CGColorSpaceRelease(colorSpace);
    }

    cgc->current_width = width;
    cgc->current_height = height;
  }

  if (cgc->context) {
    CGContextClearRect(cgc->context, CGRectMake(0, 0, width, height));
  }

  return UI_ERROR_NONE;
}

/**
 * @brief cg_end_frame.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t cg_end_frame(void *ctx) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  if (cgc && cgc->context) {
    CGContextFlush(cgc->context);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief cg_draw_rect.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t cg_draw_rect(void *ctx, const struct ui_rect *r,
                               const struct ui_color *c) {
  struct cg_context *cgc = (struct cg_context *)ctx;

  if (!cgc || !cgc->context || !r || !c)
    return UI_ERROR_INVALID_ARGUMENT;

  CGContextSetRGBFillColor(cgc->context, c->r, c->g, c->b, c->a);
  CGContextFillRect(cgc->context, CGRectMake(r->x, r->y, r->width, r->height));

  return UI_ERROR_NONE;
}

/**
 * @brief cg_draw_text.
 * @param ctx Parameter ctx.
 * @param text Parameter text.
 * @param f Parameter f.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t cg_draw_text(void *ctx, const char *text,
                               const struct ui_font *f,
                               const struct ui_rect *r) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  const unsigned char *font_data = NULL;
  size_t font_size_bytes = 0;
  CGDataProviderRef dataProvider = NULL;
  CGFontRef cgFont = NULL;
  CTFontRef ctFont = NULL;
  CFStringRef stringRef = NULL;
  CFStringRef keys[2];
  CFTypeRef values[2];
  CFDictionaryRef attributes = NULL;
  CFAttributedStringRef attrString = NULL;
  CTLineRef line = NULL;
  struct ui_color text_color = {0.0f, 0.0f, 0.0f, 1.0f}; /* Default */
  CGColorSpaceRef colorSpace = NULL;
  CGFloat components[4];
  CGColorRef cgColor = NULL;

  if (!cgc || !cgc->context || !text || !r)
    return UI_ERROR_INVALID_ARGUMENT;

  ui_font_get_data((struct ui_font *)f, &font_data, &font_size_bytes);
  if (!font_data || font_size_bytes == 0)
    return UI_ERROR_INVALID_ARGUMENT;

  dataProvider =
      CGDataProviderCreateWithData(NULL, font_data, font_size_bytes, NULL);
  if (!dataProvider)
    return UI_ERROR_INVALID_ARGUMENT;

  cgFont = CGFontCreateWithDataProvider(dataProvider);
  CGDataProviderRelease(dataProvider);
  if (!cgFont)
    return UI_ERROR_INVALID_ARGUMENT;

  ctFont = CTFontCreateWithGraphicsFont(cgFont, r->height, NULL, NULL);
  CGFontRelease(cgFont);
  if (!ctFont)
    return UI_ERROR_INVALID_ARGUMENT;

  stringRef = CFStringCreateWithCString(NULL, text, kCFStringEncodingUTF8);
  if (!stringRef) {
    CFRelease(ctFont);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  colorSpace = CGColorSpaceCreateDeviceRGB();
  components[0] = text_color.r;
  components[1] = text_color.g;
  components[2] = text_color.b;
  components[3] = text_color.a;
  cgColor = CGColorCreate(colorSpace, components);
  CGColorSpaceRelease(colorSpace);

  keys[0] = kCTFontAttributeName;
  keys[1] = kCTForegroundColorAttributeName;
  values[0] = (CFTypeRef)ctFont;
  values[1] = (CFTypeRef)cgColor;

  attributes = CFDictionaryCreate(
      NULL, (const void **)keys, (const void **)values, 2,
      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

  attrString = CFAttributedStringCreate(NULL, stringRef, attributes);
  CFRelease(stringRef);
  CFRelease(attributes);
  CFRelease(ctFont);
  CGColorRelease(cgColor);

  if (!attrString)
    return UI_ERROR_INVALID_ARGUMENT;

  line = CTLineCreateWithAttributedString(attrString);
  CFRelease(attrString);

  if (!line)
    return UI_ERROR_INVALID_ARGUMENT;

  CGContextSaveGState(cgc->context);
  CGContextSetTextMatrix(cgc->context, CGAffineTransformIdentity);
  /* In CoreGraphics text is drawn from the baseline. We flip the text matrix to
     draw properly if the context is flipped. Normally, context might be flipped
     (y goes down). CoreText expects standard Cartesian (y goes up). We
     translate to the rect's x/y, flip the y-axis, and draw. */
  CGContextTranslateCTM(cgc->context, r->x, r->y + r->height);
  CGContextScaleCTM(cgc->context, 1.0, -1.0);
  CGContextSetTextPosition(cgc->context, 0, 0);
  CTLineDraw(line, cgc->context);

  CGContextRestoreGState(cgc->context);

  CFRelease(line);

  return UI_ERROR_NONE;
}

/**
 * @brief cg_draw_image.
 * @param ctx Parameter ctx.
 * @param img Parameter img.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t cg_draw_image(void *ctx, const struct ui_image *img,
                                const struct ui_rect *r) {
  (void)ctx;
  (void)img;
  (void)r;
  /* Stub image */
  return UI_ERROR_NONE;
}

/**
 * @brief cg_draw_gradient.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param gradient Parameter gradient.
 * @return Return value.
 */
static ui_error_t cg_draw_gradient(void *ctx, const struct ui_rect *r,
                                   const struct ui_css_image *gradient) {
  (void)ctx;
  (void)r;
  (void)gradient;
  /* Stub gradient */
  return UI_ERROR_NONE;
}

/**
 * @brief cg_draw_path.
 * @param ctx Parameter ctx.
 * @param p Parameter p.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t cg_draw_path(void *ctx, const struct ui_path *p,
                               const struct ui_color *c) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  CGMutablePathRef path;
  int i;

  if (!cgc || !cgc->context || !p || !c)
    return UI_ERROR_INVALID_ARGUMENT;

  path = CGPathCreateMutable();

  for (i = 0; i < p->cmd_count; ++i) {
    struct ui_path_cmd *cmd = &p->cmds[i];
    switch (cmd->type) {
    case UI_PATH_CMD_MOVE_TO:
      CGPathMoveToPoint(path, NULL, cmd->x1, cmd->y1);
      break;
    case UI_PATH_CMD_LINE_TO:
      CGPathAddLineToPoint(path, NULL, cmd->x1, cmd->y1);
      break;
    case UI_PATH_CMD_BEZIER_TO:
      CGPathAddCurveToPoint(path, NULL, cmd->x1, cmd->y1, cmd->x2, cmd->y2,
                            cmd->x3, cmd->y3);
      break;
    case UI_PATH_CMD_CLOSE:
      CGPathCloseSubpath(path);
      break;
    }
  }

  CGContextSetRGBFillColor(cgc->context, c->r, c->g, c->b, c->a);
  CGContextAddPath(cgc->context, path);
  CGContextFillPath(cgc->context);

  CGPathRelease(path);

  return UI_ERROR_NONE;
}

/**
 * @brief cg_push_clip.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t cg_push_clip(void *ctx, const struct ui_rect *r) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  if (!cgc || !cgc->context || !r)
    return UI_ERROR_INVALID_ARGUMENT;

  CGContextSaveGState(cgc->context);
  CGContextClipToRect(cgc->context,
                      CGRectMake(r->x, r->y, r->width, r->height));

  return UI_ERROR_NONE;
}

/**
 * @brief cg_pop_clip.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t cg_pop_clip(void *ctx) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  if (!cgc || !cgc->context)
    return UI_ERROR_INVALID_ARGUMENT;

  CGContextRestoreGState(cgc->context);

  return UI_ERROR_NONE;
}

/**
 * @brief cg_set_blend_mode.
 * @param ctx Parameter ctx.
 * @param mode Parameter mode.
 * @return Return value.
 */
static ui_error_t cg_set_blend_mode(void *ctx, enum ui_css_blend_mode mode) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  CGBlendMode cg_mode = kCGBlendModeNormal;
  (void)cg_mode;
  if (!cgc || !cgc->context)
    return UI_ERROR_INVALID_ARGUMENT;

  switch (mode) {
  case UI_CSS_BLEND_MODE_NORMAL:
    cg_mode = kCGBlendModeNormal;
    break;
  case UI_CSS_BLEND_MODE_MULTIPLY:
    cg_mode = kCGBlendModeMultiply;
    break;
  case UI_CSS_BLEND_MODE_SCREEN:
    cg_mode = kCGBlendModeScreen;
    break;
  case UI_CSS_BLEND_MODE_OVERLAY:
    cg_mode = kCGBlendModeOverlay;
    break;
  case UI_CSS_BLEND_MODE_DARKEN:
    cg_mode = kCGBlendModeDarken;
    break;
  case UI_CSS_BLEND_MODE_LIGHTEN:
    cg_mode = kCGBlendModeLighten;
    break;
  case UI_CSS_BLEND_MODE_COLOR_DODGE:
    cg_mode = kCGBlendModeColorDodge;
    break;
  case UI_CSS_BLEND_MODE_COLOR_BURN:
    cg_mode = kCGBlendModeColorBurn;
    break;
  case UI_CSS_BLEND_MODE_HARD_LIGHT:
    cg_mode = kCGBlendModeHardLight;
    break;
  case UI_CSS_BLEND_MODE_SOFT_LIGHT:
    cg_mode = kCGBlendModeSoftLight;
    break;
  case UI_CSS_BLEND_MODE_DIFFERENCE:
    cg_mode = kCGBlendModeDifference;
    break;
  case UI_CSS_BLEND_MODE_EXCLUSION:
    cg_mode = kCGBlendModeExclusion;
    break;
  case UI_CSS_BLEND_MODE_HUE:
    cg_mode = kCGBlendModeHue;
    break;
  case UI_CSS_BLEND_MODE_SATURATION:
    cg_mode = kCGBlendModeSaturation;
    break;
  case UI_CSS_BLEND_MODE_COLOR:
    cg_mode = kCGBlendModeColor;
    break;
  case UI_CSS_BLEND_MODE_LUMINOSITY:
    cg_mode = kCGBlendModeLuminosity;
    break;
  default:
    break;
  }

  CGContextSetBlendMode(cgc->context, cg_mode);
  return UI_ERROR_NONE;
}

/**
 * @brief cg_set_shadow.
 * @param ctx Parameter ctx.
 * @param shadow Parameter shadow.
 * @return Return value.
 */
static ui_error_t cg_set_shadow(void *ctx, const struct ui_css_shadow *shadow) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  CGColorSpaceRef colorSpace;
  CGFloat components[4];
  CGColorRef cgColor;
  CGSize offset;

  if (!cgc || !cgc->context)
    return UI_ERROR_INVALID_ARGUMENT;

  if (!shadow) {
    /* Clear shadow by setting a clear color or 0 offset/blur */

    CGContextSetShadowWithColor(cgc->context, CGSizeMake(0, 0), 0, NULL);

    return UI_ERROR_NONE;
    return UI_ERROR_NONE;
  }

  offset = CGSizeMake(shadow->offset_x.value, shadow->offset_y.value);
  (void)offset;

  colorSpace = CGColorSpaceCreateDeviceRGB();
  /* Convert sRGB 0-1 values from ui_css_color to CG components */
  components[0] = shadow->color.components[0];
  components[1] = shadow->color.components[1];
  components[2] = shadow->color.components[2];
  components[3] = shadow->color.components[3];

  cgColor = CGColorCreate(colorSpace, components);

  CGContextSetShadowWithColor(cgc->context, offset, shadow->blur_radius.value,
                              cgColor);

  CGColorRelease(cgColor);
  CGColorSpaceRelease(colorSpace);
  return UI_ERROR_NONE;
}

/**
 * @brief cg_read_pixels.
 * @param ctx Parameter ctx.
 * @param out_rgba_buffer Parameter out_rgba_buffer.
 * @return Return value.
 */
static ui_error_t cg_read_pixels(void *ctx, unsigned char *out_rgba_buffer) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  void *data;
  size_t bytes_per_row;
  size_t y;
  unsigned char *src_row;
  unsigned char *dst_row;

  if (!cgc || !cgc->context || !out_rgba_buffer)
    return UI_ERROR_INVALID_ARGUMENT;

  data = CGBitmapContextGetData(cgc->context);
  if (!data)
    return UI_ERROR_INVALID_ARGUMENT;

  bytes_per_row = CGBitmapContextGetBytesPerRow(cgc->context);
  src_row = (unsigned char *)data;
  dst_row = out_rgba_buffer;

  for (y = 0; y < (size_t)cgc->current_height; ++y) {
    /* CoreGraphics RGBA format was specified as kCGImageAlphaPremultipliedLast
       in CGBitmapContextCreate. This means RGBA bytes in memory. */
    memcpy(dst_row, src_row, cgc->current_width * 4);
    src_row += bytes_per_row;
    dst_row += cgc->current_width * 4;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief cg_destroy.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t cg_destroy(void *ctx) {
  struct cg_context *cgc = (struct cg_context *)ctx;
  if (cgc) {
    printf("cg_destroy: cgc->context = %p\n", (void *)cgc->context);
    if (cgc->context) {
      CGContextRelease(cgc->context);
    }
    C_MULTIPLATFORM_FREE(cgc);
  }
  return UI_ERROR_NONE;
}

static const struct ui_renderer_vtable cg_vtable = {
    cg_begin_frame, cg_end_frame,      cg_draw_rect,  cg_draw_text,
    cg_draw_image,  cg_draw_gradient,  cg_draw_path,  cg_push_clip,
    cg_pop_clip,    cg_set_blend_mode, cg_set_shadow, cg_read_pixels,
    cg_destroy};

/**
 * @brief ui_renderer_native_init.
 * @param renderer Parameter renderer.
 * @return Return value.
 */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer) {
  struct cg_context *cgc;

  if (!renderer)
    return UI_ERROR_INVALID_ARGUMENT;

  cgc = (struct cg_context *)C_MULTIPLATFORM_MALLOC(sizeof(struct cg_context));
  if (!cgc) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  cgc->context = NULL;
  cgc->current_width = 0;
  cgc->current_height = 0;

  renderer->vtable = &cg_vtable;
  renderer->ctx = cgc;

  return UI_ERROR_NONE;
}
#endif
#endif

/* Prevent empty translation unit warning on MSVC */
/**
 * @brief ui_renderer_coregraphics_dummy_t.
 */
typedef int ui_renderer_coregraphics_dummy_t;
