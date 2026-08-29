/**
 * @file ui_renderer_gdiplus.c
 * @brief ui_renderer_gdiplus.c implementation.
 */
/* clang-format off */
#include "../include/ui_renderer.h"
#include "../include/ui_error.h"
#include "../include/ui_css_values.h"
#include "../include/ui_font_manager.h"

#if defined(_WIN32)
/** @brief internal */
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <winuser.h>
#include "ui_internal_mem.h"
/* clang-format on */

/* GDI+ Flat API Declarations (C89 compatible, no C++ classes) */
typedef void GpGraphics;
typedef void GpBrush;
typedef void GpSolidFill;
typedef void GpPen;
typedef void GpPath;
typedef void GpBitmap;
typedef void GpImage;

typedef enum { Ok = 0 } GpStatus;

typedef enum { FillModeAlternate = 0, FillModeWinding = 1 } GpFillMode;

typedef DWORD ARGB;

/**
 * @struct GdiplusStartupInput
 * \brief GdiplusStartupInput
 */
struct GdiplusStartupInput {
  UINT32 GdiplusVersion;         /**< GdiplusVersion */
  void *DebugEventCallback;      /**< DebugEventCallback */
  BOOL SuppressBackgroundThread; /**< SuppressBackgroundThread */
  BOOL SuppressExternalCodecs;   /**< SuppressExternalCodecs */
};

/** @brief internal */
#define WINGDIPAPI __stdcall

GpStatus WINGDIPAPI GdiplusStartup(ULONG_PTR *token,
                                   const struct GdiplusStartupInput *input,
                                   void *output);
void WINGDIPAPI GdiplusShutdown(ULONG_PTR token);

GpStatus WINGDIPAPI GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);
GpStatus WINGDIPAPI GdipDeleteGraphics(GpGraphics *graphics);
GpStatus WINGDIPAPI GdipGraphicsClear(GpGraphics *graphics, ARGB color);

GpStatus WINGDIPAPI GdipCreateSolidFill(ARGB color, GpSolidFill **brush);
GpStatus WINGDIPAPI GdipDeleteBrush(GpBrush *brush);

GpStatus WINGDIPAPI GdipFillRectangle(GpGraphics *graphics, GpBrush *brush,
                                      float x, float y, float width,
                                      float height);
GpStatus WINGDIPAPI GdipSetClipRect(GpGraphics *graphics, float x, float y,
                                    float width, float height, int combineMode);
GpStatus WINGDIPAPI GdipResetClip(GpGraphics *graphics);

GpStatus WINGDIPAPI GdipCreatePath(GpFillMode brushMode, GpPath **path);
GpStatus WINGDIPAPI GdipDeletePath(GpPath *path);
GpStatus WINGDIPAPI GdipStartPathFigure(GpPath *path);
GpStatus WINGDIPAPI GdipClosePathFigure(GpPath *path);
GpStatus WINGDIPAPI GdipAddPathLine(GpPath *path, float x1, float y1, float x2,
                                    float y2);
GpStatus WINGDIPAPI GdipAddPathBezier(GpPath *path, float x1, float y1,
                                      float x2, float y2, float x3, float y3,
                                      float x4, float y4);
GpStatus WINGDIPAPI GdipAddPathEllipse(GpPath *path, float x, float y,
                                       float width, float height);
GpStatus WINGDIPAPI GdipFillPath(GpGraphics *graphics, GpBrush *brush,
                                 GpPath *path);

typedef void GpLineGradient;
typedef void GpPathGradient;
/* \brief enum
 */
typedef enum {
  WrapModeTile = 0,
  WrapModeTileFlipX = 1,
  WrapModeTileFlipY = 2,
  WrapModeTileFlipXY = 3,
  WrapModeClamp = 4
} GpWrapMode;

/* \brief struct
 */
typedef struct {
  float X;
  float Y;
  float Width;
  float Height;
} GpRectF;

/* \brief struct
 */
typedef struct {
  INT X;
  INT Y;
  INT Width;
  INT Height;
} GpRect;

/* \brief enum
 */
typedef enum {
  ImageLockModeRead = 1,
  ImageLockModeWrite = 2,
  ImageLockModeUserInputBuf = 4
} ImageLockMode;

/* \brief struct
 */
typedef struct {
  UINT Width;
  UINT Height;
  INT Stride;
  INT PixelFormat;
  VOID *Scan0;
  UINT_PTR Reserved;
} BitmapData;

/* \brief struct
 */
typedef struct {
  float X;
  float Y;
} GpPointF;

GpStatus WINGDIPAPI GdipCreateLineBrushFromRectWithAngle(
    const GpRectF *rect, ARGB color1, ARGB color2, float angle,
    BOOL isAngleScalable, GpWrapMode wrapMode, GpLineGradient **lineGradient);
GpStatus WINGDIPAPI GdipSetLinePresetBlend(GpLineGradient *brush,
                                           const ARGB *blend,
                                           const float *positions, int count);
GpStatus WINGDIPAPI
GdipCreatePathGradientFromPath(GpPath *path, GpPathGradient **polyGradient);
GpStatus WINGDIPAPI GdipSetPathGradientPresetBlend(GpPathGradient *brush,
                                                   const ARGB *blend,
                                                   const float *positions,
                                                   int count);
GpStatus WINGDIPAPI GdipSetPathGradientCenterPoint(GpPathGradient *brush,
                                                   GpPointF *points);
GpStatus WINGDIPAPI GdipSetPathGradientCenterColor(GpPathGradient *brush,
                                                   ARGB colors);
GpStatus WINGDIPAPI GdipSetPathGradientSurroundColorsWithCount(
    GpPathGradient *brush, ARGB *color, int *count);

GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(int width, int height, int stride,
                                              int format, BYTE *scan0,
                                              GpBitmap **bitmap);
GpStatus WINGDIPAPI GdipBitmapLockBits(GpBitmap *bitmap, const GpRect *rect,
                                       UINT flags, INT format,
                                       BitmapData *lockedBitmapData);
GpStatus WINGDIPAPI GdipBitmapUnlockBits(GpBitmap *bitmap,
                                         BitmapData *lockedBitmapData);
GpStatus WINGDIPAPI GdipGetImageGraphicsContext(GpImage *image,
                                                GpGraphics **graphics);
GpStatus WINGDIPAPI GdipDisposeImage(GpImage *image);

typedef void GpFontCollection;
typedef void GpFontFamily;
typedef void GpFont;
typedef void GpStringFormat;

GpStatus WINGDIPAPI
GdipNewPrivateFontCollection(GpFontCollection **fontCollection);
GpStatus WINGDIPAPI
GdipDeletePrivateFontCollection(GpFontCollection **fontCollection);
GpStatus WINGDIPAPI GdipPrivateAddMemoryFont(GpFontCollection *fontCollection,
                                             const void *memory, INT length);
GpStatus WINGDIPAPI GdipGetFontCollectionFamilyCount(
    GpFontCollection *fontCollection, INT *numFound);
GpStatus WINGDIPAPI
GdipGetFontCollectionFamilyList(GpFontCollection *fontCollection, INT numSought,
                                GpFontFamily *gpfamilies[], INT *numFound);
GpStatus WINGDIPAPI GdipCreateFont(const GpFontFamily *fontFamily, float emSize,
                                   INT style, int unit, GpFont **font);
GpStatus WINGDIPAPI GdipDeleteFont(GpFont *font);
GpStatus WINGDIPAPI GdipDrawString(GpGraphics *graphics, const WCHAR *string,
                                   INT length, const GpFont *font,
                                   const GpRectF *layoutRect,
                                   const GpStringFormat *stringFormat,
                                   const GpBrush *brush);

/** @brief internal */
#define PixelFormat32bppARGB 0x26200A

/* GDI+ Renderer Context */
/**
 * @struct gdiplus_context
 * \brief gdiplus_context
 */
struct gdiplus_context {
  ULONG_PTR token;            /**< token */
  GpBitmap *offscreen_bitmap; /**< offscreen_bitmap */
  GpGraphics *graphics;       /**< graphics */
  int current_width;          /**< current_width */
  int current_height;         /**< current_height */
};

/**
 * @brief ui_color_to_argb.
 * @param c Parameter c.
 * @return Return value.
 */
static ARGB ui_color_to_argb(const struct ui_color *c) {
  BYTE a = (BYTE)(c->a * 255.0f);
  BYTE r = (BYTE)(c->r * 255.0f);
  BYTE g = (BYTE)(c->g * 255.0f);
  BYTE b = (BYTE)(c->b * 255.0f);
  return ((ARGB)a << 24) | ((ARGB)r << 16) | ((ARGB)g << 8) | (ARGB)b;
}

/**
 * @brief ui_css_color_to_argb.
 * @param c Parameter c.
 * @return Return value.
 */
static ARGB ui_css_color_to_argb(const struct ui_css_color *c) {
  /* Assuming SRGB for now */
  BYTE a = (BYTE)(c->components[3] * 255.0f);
  BYTE r = (BYTE)(c->components[0] * 255.0f);
  BYTE g = (BYTE)(c->components[1] * 255.0f);
  BYTE b = (BYTE)(c->components[2] * 255.0f);
  return ((ARGB)a << 24) | ((ARGB)r << 16) | ((ARGB)g << 8) | (ARGB)b;
}

/**
 * @brief gdiplus_begin_frame.
 * @param ctx Parameter ctx.
 * @param width Parameter width.
 * @param height Parameter height.
 * @return Return value.
 */
static ui_error_t gdiplus_begin_frame(void *ctx, int width, int height) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;

  if (!gctx)
    return UI_ERROR_INVALID_ARGUMENT;

  /* If dimensions changed, recreate offscreen bitmap */
  if (gctx->current_width != width || gctx->current_height != height ||
      !gctx->offscreen_bitmap) {
    if (gctx->graphics) {
      GdipDeleteGraphics(gctx->graphics);
      gctx->graphics = NULL;
    }
    if (gctx->offscreen_bitmap) {
      GdipDisposeImage((GpImage *)gctx->offscreen_bitmap);
      gctx->offscreen_bitmap = NULL;
    }
    if (width > 0 && height > 0) {
      if (GdipCreateBitmapFromScan0(width, height, 0, PixelFormat32bppARGB,
                                    NULL, &gctx->offscreen_bitmap) == Ok) {
        GdipGetImageGraphicsContext((GpImage *)gctx->offscreen_bitmap,
                                    &gctx->graphics);
      }
    }
    gctx->current_width = width;
    gctx->current_height = height;
  }

  if (gctx->graphics) {
    GdipGraphicsClear(gctx->graphics, 0x00000000); /* Transparent black */
  }

  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_end_frame.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gdiplus_end_frame(void *ctx) {
  /* For offscreen, nothing to do here.
     When blitting to screen, we would extract the bitmap or draw it to the
     window HDC. */
  (void)ctx;
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_draw_rect.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t gdiplus_draw_rect(void *ctx, const struct ui_rect *r,
                                    const struct ui_color *c) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  GpSolidFill *brush = NULL;

  if (!gctx || !gctx->graphics || !r || !c)
    return UI_ERROR_INVALID_ARGUMENT;

  if (GdipCreateSolidFill(ui_color_to_argb(c), &brush) == Ok) {
    GdipFillRectangle(gctx->graphics, (GpBrush *)brush, r->x, r->y, r->width,
                      r->height);
    GdipDeleteBrush((GpBrush *)brush);
    return UI_ERROR_NONE;
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief gdiplus_draw_text.
 * @param ctx Parameter ctx.
 * @param text Parameter text.
 * @param f Parameter f.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gdiplus_draw_text(void *ctx, const char *text,
                                    const struct ui_font *f,
                                    const struct ui_rect *r) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  GpFontCollection *collection = NULL;
  GpFontFamily *family = NULL;
  GpFont *font = NULL;
  GpSolidFill *brush = NULL;
  GpRectF rect;
  int numFound = 0;
  const unsigned char *font_data = NULL;
  size_t font_size_bytes = 0;
  int text_len = 0;
  WCHAR *wtext = NULL;
  struct ui_color text_color = {
      0.0f, 0.0f, 0.0f,
      1.0f}; /* Default to black, we don't pass color in draw_text yet */

  if (!gctx || !gctx->graphics || !text || !f || !r)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Get TTF font data */
  ui_font_get_data((struct ui_font *)f, &font_data, &font_size_bytes);
  if (!font_data || font_size_bytes == 0)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Convert UTF-8 to UTF-16 */
  text_len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
  if (text_len <= 0)
    return UI_ERROR_INVALID_ARGUMENT;
  wtext = (WCHAR *)C_MULTIPLATFORM_MALLOC((size_t)text_len * sizeof(WCHAR));
  if (!wtext)
    return UI_ERROR_INVALID_ARGUMENT;
  MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, text_len);

  /* Load font from memory */
  if (GdipNewPrivateFontCollection(&collection) != Ok) {
    C_MULTIPLATFORM_FREE(wtext);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (GdipPrivateAddMemoryFont(collection, font_data, (INT)font_size_bytes) !=
      Ok) {
    GdipDeletePrivateFontCollection(&collection);
    C_MULTIPLATFORM_FREE(wtext);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (GdipGetFontCollectionFamilyCount(collection, &numFound) != Ok ||
      numFound == 0) {
    GdipDeletePrivateFontCollection(&collection);
    C_MULTIPLATFORM_FREE(wtext);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (GdipGetFontCollectionFamilyList(collection, 1, &family, &numFound) !=
          Ok ||
      numFound == 0) {
    GdipDeletePrivateFontCollection(&collection);
    C_MULTIPLATFORM_FREE(wtext);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Create font using height of rect as emSize for now, assuming UnitPixel = 2
   */
  if (GdipCreateFont(family, r->height, 0, 2, &font) != Ok) {
    GdipDeletePrivateFontCollection(&collection);
    C_MULTIPLATFORM_FREE(wtext);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rect.X = r->x;
  rect.Y = r->y;
  rect.Width = r->width;
  rect.Height = r->height;

  if (GdipCreateSolidFill(ui_color_to_argb(&text_color), &brush) == Ok) {
    GdipDrawString(gctx->graphics, wtext, text_len - 1, font, &rect, NULL,
                   (GpBrush *)brush);
    GdipDeleteBrush((GpBrush *)brush);
  }

  GdipDeleteFont(font);
  GdipDeletePrivateFontCollection(&collection);
  C_MULTIPLATFORM_FREE(wtext);

  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_draw_image.
 * @param ctx Parameter ctx.
 * @param img Parameter img.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gdiplus_draw_image(void *ctx, const struct ui_image *img,
                                     const struct ui_rect *r) {
  (void)ctx;
  (void)img;
  (void)r;
  /* Stub image */
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_draw_gradient.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @param gradient Parameter gradient.
 * @return Return value.
 */
static ui_error_t gdiplus_draw_gradient(void *ctx, const struct ui_rect *r,
                                        const struct ui_css_image *gradient) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  if (!gctx || !gctx->graphics || !r || !gradient)
    return UI_ERROR_INVALID_ARGUMENT;

  if (gradient->type == UI_CSS_IMAGE_LINEAR_GRADIENT) {
    GpLineGradient *brush = NULL;
    GpRectF rect;
    int i;
    int count = gradient->data.linear_gradient.stop_count;
    ARGB *colors = NULL;
    float *positions = NULL;

    if (count < 2)
      return UI_ERROR_INVALID_ARGUMENT;

    rect.X = r->x;
    rect.Y = r->y;
    rect.Width = r->width;
    rect.Height = r->height;

    /* In GDI+, angle 0 is horizontal left-to-right.
       CSS angle 0 is vertical bottom-to-top.
       We'll approximate by passing the angle directly or adjusting it.
       For now, use the angle directly. */
    if (GdipCreateLineBrushFromRectWithAngle(
            &rect, 0, 0, gradient->data.linear_gradient.angle, TRUE,
            WrapModeClamp, &brush) == Ok) {
      colors = (ARGB *)C_MULTIPLATFORM_MALLOC(sizeof(ARGB) * (size_t)count);
      positions =
          (float *)C_MULTIPLATFORM_MALLOC(sizeof(float) * (size_t)count);
      if (colors && positions) {
        for (i = 0; i < count; ++i) {
          colors[i] = ui_css_color_to_argb(
              &gradient->data.linear_gradient.stops[i].color);
          /* Approximate positions */
          positions[i] = (float)i / (float)(count - 1);
        }
        GdipSetLinePresetBlend(brush, colors, positions, count);
        GdipFillRectangle(gctx->graphics, (GpBrush *)brush, r->x, r->y,
                          r->width, r->height);
      }
      if (colors)
        C_MULTIPLATFORM_FREE(colors);
      if (positions)
        C_MULTIPLATFORM_FREE(positions);
      GdipDeleteBrush((GpBrush *)brush);
    }
    return UI_ERROR_NONE;
  } else if (gradient->type == UI_CSS_IMAGE_RADIAL_GRADIENT ||
             gradient->type == UI_CSS_IMAGE_CONIC_GRADIENT) {
    GpPath *path = NULL;
    GpPathGradient *brush = NULL;
    int i;
    int count = gradient->type == UI_CSS_IMAGE_RADIAL_GRADIENT
                    ? gradient->data.radial_gradient.stop_count
                    : gradient->data.conic_gradient.stop_count;
    const struct ui_css_gradient_stop *stops =
        gradient->type == UI_CSS_IMAGE_RADIAL_GRADIENT
            ? gradient->data.radial_gradient.stops
            : gradient->data.conic_gradient.stops;
    ARGB *colors = NULL;
    float *positions = NULL;

    if (count < 2)
      return UI_ERROR_INVALID_ARGUMENT;

    /* Create a circular path for the radial/conic gradient */
    if (GdipCreatePath(FillModeAlternate, &path) == Ok) {
      GdipAddPathEllipse(path, r->x, r->y, r->width, r->height);

      if (GdipCreatePathGradientFromPath(path, &brush) == Ok) {
        colors = (ARGB *)C_MULTIPLATFORM_MALLOC(sizeof(ARGB) * (size_t)count);
        positions =
            (float *)C_MULTIPLATFORM_MALLOC(sizeof(float) * (size_t)count);
        if (colors && positions) {
          GpPointF center;

          center.X = r->x + r->width / 2.0f;
          center.Y = r->y + r->height / 2.0f;
          GdipSetPathGradientCenterPoint(brush, &center);

          /* PathGradient supports PresetBlend */
          for (i = 0; i < count; ++i) {
            colors[i] = ui_css_color_to_argb(&stops[i].color);
            positions[i] = (float)i / (float)(count - 1);
          }
          /* For PathGradient, position 0 is center, 1 is boundary.
             We need to reverse positions if CSS expects 0 at center.
             Let's just use GdipSetPathGradientPresetBlend. */
          GdipSetPathGradientPresetBlend(brush, colors, positions, count);

          GdipFillRectangle(gctx->graphics, (GpBrush *)brush, r->x, r->y,
                            r->width, r->height);
        }
        if (colors)
          C_MULTIPLATFORM_FREE(colors);
        if (positions)
          C_MULTIPLATFORM_FREE(positions);
        GdipDeleteBrush((GpBrush *)brush);
      }
      GdipDeletePath(path);
    }
    return UI_ERROR_NONE;
  }

  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief gdiplus_draw_path.
 * @param ctx Parameter ctx.
 * @param p Parameter p.
 * @param c Parameter c.
 * @return Return value.
 */
static ui_error_t gdiplus_draw_path(void *ctx, const struct ui_path *p,
                                    const struct ui_color *c) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  GpPath *path = NULL;
  GpSolidFill *brush = NULL;
  int i;
  float cx = 0.0f, cy = 0.0f;

  if (!gctx || !gctx->graphics || !p || !c)
    return UI_ERROR_INVALID_ARGUMENT;

  if (GdipCreatePath(FillModeWinding, &path) != Ok)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < p->cmd_count; ++i) {
    struct ui_path_cmd *cmd = &p->cmds[i];
    switch (cmd->type) {
    case UI_PATH_CMD_MOVE_TO:
      GdipStartPathFigure(path);
      cx = cmd->x1;
      cy = cmd->y1;
      break;
    case UI_PATH_CMD_LINE_TO:
      GdipAddPathLine(path, cx, cy, cmd->x1, cmd->y1);
      cx = cmd->x1;
      cy = cmd->y1;
      break;
    case UI_PATH_CMD_BEZIER_TO:
      GdipAddPathBezier(path, cx, cy, cmd->x1, cmd->y1, cmd->x2, cmd->y2,
                        cmd->x3, cmd->y3);
      cx = cmd->x3;
      cy = cmd->y3;
      break;
    case UI_PATH_CMD_CLOSE:
      GdipClosePathFigure(path);
      break;
    }
  }

  if (GdipCreateSolidFill(ui_color_to_argb(c), &brush) == Ok) {
    GdipFillPath(gctx->graphics, (GpBrush *)brush, path);
    GdipDeleteBrush((GpBrush *)brush);
  }

  GdipDeletePath(path);
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_push_clip.
 * @param ctx Parameter ctx.
 * @param r Parameter r.
 * @return Return value.
 */
static ui_error_t gdiplus_push_clip(void *ctx, const struct ui_rect *r) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  if (!gctx || !gctx->graphics || !r)
    return UI_ERROR_INVALID_ARGUMENT;
  /* CombineModeIntersect = 1 */
  GdipSetClipRect(gctx->graphics, r->x, r->y, r->width, r->height, 1);
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_pop_clip.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gdiplus_pop_clip(void *ctx) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  if (!gctx || !gctx->graphics)
    return UI_ERROR_INVALID_ARGUMENT;
  GdipResetClip(gctx->graphics);
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_set_blend_mode.
 * @param ctx Parameter ctx.
 * @param mode Parameter mode.
 * @return Return value.
 */
static ui_error_t gdiplus_set_blend_mode(void *ctx,
                                         enum ui_css_blend_mode mode) {
  (void)ctx;
  (void)mode;
  /* Stub for blend mode */
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_set_shadow.
 * @param ctx Parameter ctx.
 * @param shadow Parameter shadow.
 * @return Return value.
 */
static ui_error_t gdiplus_set_shadow(void *ctx,
                                     const struct ui_css_shadow *shadow) {
  (void)ctx;
  (void)shadow;
  /* Stub for drop shadow */
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_read_pixels.
 * @param ctx Parameter ctx.
 * @param out_rgba_buffer Parameter out_rgba_buffer.
 * @return Return value.
 */
static ui_error_t gdiplus_read_pixels(void *ctx,
                                      unsigned char *out_rgba_buffer) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  BitmapData bitmapData;
  GpRect rect;
  int y;
  unsigned char *src_row;
  unsigned char *dst_row;

  if (!gctx || !gctx->offscreen_bitmap || !out_rgba_buffer)
    return UI_ERROR_INVALID_ARGUMENT;

  rect.X = 0;
  rect.Y = 0;
  rect.Width = gctx->current_width;
  rect.Height = gctx->current_height;

  if (GdipBitmapLockBits(gctx->offscreen_bitmap, &rect, ImageLockModeRead,
                         PixelFormat32bppARGB, &bitmapData) != Ok) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* GDI+ uses BGRA (or PBGRA). We need to copy and convert to RGBA. */
  src_row = (unsigned char *)bitmapData.Scan0;
  dst_row = out_rgba_buffer;

  for (y = 0; y < gctx->current_height; ++y) {
    int x;
    unsigned char *src = src_row;
    unsigned char *dst = dst_row;
    for (x = 0; x < gctx->current_width; ++x) {
      dst[0] = src[2]; /* R */
      dst[1] = src[1]; /* G */
      dst[2] = src[0]; /* B */
      dst[3] = src[3]; /* A */
      src += 4;
      dst += 4;
    }
    src_row += bitmapData.Stride;
    dst_row += gctx->current_width * 4;
  }

  GdipBitmapUnlockBits(gctx->offscreen_bitmap, &bitmapData);
  return UI_ERROR_NONE;
}

/**
 * @brief gdiplus_destroy.
 * @param ctx Parameter ctx.
 * @return Return value.
 */
static ui_error_t gdiplus_destroy(void *ctx) {
  struct gdiplus_context *gctx = (struct gdiplus_context *)ctx;
  if (gctx) {
    if (gctx->graphics)
      GdipDeleteGraphics(gctx->graphics);
    if (gctx->offscreen_bitmap)
      GdipDisposeImage((GpImage *)gctx->offscreen_bitmap);
    GdiplusShutdown(gctx->token);
    C_MULTIPLATFORM_FREE(gctx);
  }
  return UI_ERROR_NONE;
}

static const struct ui_renderer_vtable gdiplus_vtable = {
    gdiplus_begin_frame,    gdiplus_end_frame,  gdiplus_draw_rect,
    gdiplus_draw_text,      gdiplus_draw_image, gdiplus_draw_gradient,
    gdiplus_draw_path,      gdiplus_push_clip,  gdiplus_pop_clip,
    gdiplus_set_blend_mode, gdiplus_set_shadow, gdiplus_read_pixels,
    gdiplus_destroy};

/**
 * @brief ui_renderer_native_init.
 * @param renderer Parameter renderer.
 * @return Return value.
 */
ui_error_t ui_renderer_native_init(struct ui_renderer *renderer) {
  struct gdiplus_context *gctx;
  struct GdiplusStartupInput input;
  ULONG_PTR token;

  if (!renderer)
    return UI_ERROR_INVALID_ARGUMENT;

  input.GdiplusVersion = 1;
  input.DebugEventCallback = NULL;
  input.SuppressBackgroundThread = FALSE;
  input.SuppressExternalCodecs = FALSE;

  if (GdiplusStartup(&token, &input, NULL) != Ok) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  gctx = (struct gdiplus_context *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct gdiplus_context));
  if (!gctx) {
    GdiplusShutdown(token);
    return UI_ERROR_INVALID_ARGUMENT;
  }

  gctx->token = token;
  gctx->offscreen_bitmap = NULL;
  gctx->graphics = NULL;
  gctx->current_width = 0;
  gctx->current_height = 0;

  renderer->vtable = &gdiplus_vtable;
  renderer->ctx = gctx;

  return UI_ERROR_NONE;
}
#endif
