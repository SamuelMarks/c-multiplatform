#ifndef CMP_API_GFX_H
#define CMP_API_GFX_H

/**
 * @file cmp_api_gfx.h
 * @brief Graphics abstraction ABI for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_math.h"
#include "cmpc/cmp_object.h"

/** @brief RGBA8 texture format. */
#define CMP_TEX_FORMAT_RGBA8 1
/** @brief BGRA8 texture format. */
#define CMP_TEX_FORMAT_BGRA8 2
/** @brief Alpha-only 8-bit texture format. */
#define CMP_TEX_FORMAT_A8 3

/**
 * @brief Color in linear RGBA space (0..1 range).
 */
typedef struct CMPColor {
  CMPScalar r; /**< Red channel. */
  CMPScalar g; /**< Green channel. */
  CMPScalar b; /**< Blue channel. */
  CMPScalar a; /**< Alpha channel. */
} CMPColor;

/** @brief Path command: move to. */
#define CMP_PATH_CMD_MOVE_TO 1
/** @brief Path command: line to. */
#define CMP_PATH_CMD_LINE_TO 2
/** @brief Path command: quadratic curve to. */
#define CMP_PATH_CMD_QUAD_TO 3
/** @brief Path command: cubic curve to. */
#define CMP_PATH_CMD_CUBIC_TO 4
/** @brief Path command: close path. */
#define CMP_PATH_CMD_CLOSE 5

/**
 * @brief Move-to path command payload.
 */
typedef struct CMPPathCmdMoveTo {
  CMPScalar x; /**< Target X coordinate. */
  CMPScalar y; /**< Target Y coordinate. */
} CMPPathCmdMoveTo;

/**
 * @brief Line-to path command payload.
 */
typedef struct CMPPathCmdLineTo {
  CMPScalar x; /**< Target X coordinate. */
  CMPScalar y; /**< Target Y coordinate. */
} CMPPathCmdLineTo;

/**
 * @brief Quadratic curve path command payload.
 */
typedef struct CMPPathCmdQuadTo {
  CMPScalar cx; /**< Control point X coordinate. */
  CMPScalar cy; /**< Control point Y coordinate. */
  CMPScalar x;  /**< Target X coordinate. */
  CMPScalar y;  /**< Target Y coordinate. */
} CMPPathCmdQuadTo;

/**
 * @brief Cubic curve path command payload.
 */
typedef struct CMPPathCmdCubicTo {
  CMPScalar cx1; /**< First control point X coordinate. */
  CMPScalar cy1; /**< First control point Y coordinate. */
  CMPScalar cx2; /**< Second control point X coordinate. */
  CMPScalar cy2; /**< Second control point Y coordinate. */
  CMPScalar x;   /**< Target X coordinate. */
  CMPScalar y;   /**< Target Y coordinate. */
} CMPPathCmdCubicTo;

/**
 * @brief Path command payload union.
 */
typedef union CMPPathCmdData {
  CMPPathCmdMoveTo move_to;   /**< Move-to payload. */
  CMPPathCmdLineTo line_to;   /**< Line-to payload. */
  CMPPathCmdQuadTo quad_to;   /**< Quadratic curve payload. */
  CMPPathCmdCubicTo cubic_to; /**< Cubic curve payload. */
} CMPPathCmdData;

/**
 * @brief Path command entry.
 */
typedef struct CMPPathCmd {
  cmp_u32 type;        /**< Command type (CMP_PATH_CMD_*). */
  CMPPathCmdData data; /**< Command payload. */
} CMPPathCmd;

/**
 * @brief Vector path command list.
 */
typedef struct CMPPath {
  CMPPathCmd *commands;   /**< Command array (owned by the path). */
  cmp_usize count;        /**< Number of commands in the path. */
  cmp_usize capacity;     /**< Command array capacity. */
  CMPAllocator allocator; /**< Allocator used for command storage. */
} CMPPath;

/**
 * @brief Begin rendering a frame for a window.
 * @param gfx Graphics backend instance.
 * @param window Target window handle.
 * @param width Backbuffer width in pixels.
 * @param height Backbuffer height in pixels.
 * @param dpi_scale DPI scale factor.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxBeginFrameFn)(void *gfx, CMPHandle window,
                                          cmp_i32 width, cmp_i32 height,
                                          CMPScalar dpi_scale);

/**
 * @brief End rendering a frame for a window.
 * @param gfx Graphics backend instance.
 * @param window Target window handle.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxEndFrameFn)(void *gfx, CMPHandle window);

/**
 * @brief Clear the current render target.
 * @param gfx Graphics backend instance.
 * @param color Clear color.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxClearFn)(void *gfx, CMPColor color);

/**
 * @brief Draw a filled rectangle.
 * @param gfx Graphics backend instance.
 * @param rect Rectangle to draw.
 * @param color Fill color.
 * @param corner_radius Corner radius in pixels.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxDrawRectFn)(void *gfx, const CMPRect *rect,
                                        CMPColor color,
                                        CMPScalar corner_radius);

/**
 * @brief Draw a line segment.
 * @param gfx Graphics backend instance.
 * @param x0 Start X coordinate.
 * @param y0 Start Y coordinate.
 * @param x1 End X coordinate.
 * @param y1 End Y coordinate.
 * @param color Line color.
 * @param thickness Line thickness in pixels.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxDrawLineFn)(void *gfx, CMPScalar x0, CMPScalar y0,
                                        CMPScalar x1, CMPScalar y1,
                                        CMPColor color, CMPScalar thickness);

/**
 * @brief Draw a filled path.
 * @param gfx Graphics backend instance.
 * @param path Path to draw.
 * @param color Fill color.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxDrawPathFn)(void *gfx, const CMPPath *path,
                                        CMPColor color);

/**
 * @brief Push a clipping rectangle.
 * @param gfx Graphics backend instance.
 * @param rect Clipping rectangle.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxPushClipFn)(void *gfx, const CMPRect *rect);

/**
 * @brief Pop the most recent clipping rectangle.
 * @param gfx Graphics backend instance.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxPopClipFn)(void *gfx);

/**
 * @brief Set the current transform matrix.
 * @param gfx Graphics backend instance.
 * @param transform Transform matrix.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxSetTransformFn)(void *gfx,
                                            const CMPMat3 *transform);

/**
 * @brief Create a texture resource.
 * @param gfx Graphics backend instance.
 * @param width Texture width in pixels.
 * @param height Texture height in pixels.
 * @param format Texture format (CMP_TEX_FORMAT_*).
 * @param pixels Pixel data pointer.
 * @param size Size of pixel data in bytes.
 * @param out_texture Receives the created texture handle.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxCreateTextureFn)(void *gfx, cmp_i32 width,
                                             cmp_i32 height, cmp_u32 format,
                                             const void *pixels, cmp_usize size,
                                             CMPHandle *out_texture);

/**
 * @brief Update an existing texture.
 * @param gfx Graphics backend instance.
 * @param texture Texture handle to update.
 * @param x X offset in pixels.
 * @param y Y offset in pixels.
 * @param width Width of update region.
 * @param height Height of update region.
 * @param pixels Pixel data pointer.
 * @param size Size of pixel data in bytes.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxUpdateTextureFn)(void *gfx, CMPHandle texture,
                                             cmp_i32 x, cmp_i32 y,
                                             cmp_i32 width, cmp_i32 height,
                                             const void *pixels,
                                             cmp_usize size);

/**
 * @brief Destroy a texture resource.
 * @param gfx Graphics backend instance.
 * @param texture Texture handle to destroy.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxDestroyTextureFn)(void *gfx, CMPHandle texture);

/**
 * @brief Draw a textured quad.
 * @param gfx Graphics backend instance.
 * @param texture Texture handle to draw.
 * @param src Source rectangle in texture space.
 * @param dst Destination rectangle in window space.
 * @param opacity Opacity multiplier (0..1).
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPGfxDrawTextureFn)(void *gfx, CMPHandle texture,
                                           const CMPRect *src,
                                           const CMPRect *dst,
                                           CMPScalar opacity);

/**
 * @brief Graphics virtual table.
 */
typedef struct CMPGfxVTable {
  /** @brief Begin rendering a frame for a window. */
  CMPGfxBeginFrameFn begin_frame;
  /** @brief End rendering a frame for a window. */
  CMPGfxEndFrameFn end_frame;
  /** @brief Clear the current render target. */
  CMPGfxClearFn clear;
  /** @brief Draw a filled rectangle. */
  CMPGfxDrawRectFn draw_rect;
  /** @brief Draw a line segment. */
  CMPGfxDrawLineFn draw_line;
  /** @brief Draw a filled path. */
  CMPGfxDrawPathFn draw_path;
  /** @brief Push a clipping rectangle. */
  CMPGfxPushClipFn push_clip;
  /** @brief Pop the most recent clipping rectangle. */
  CMPGfxPopClipFn pop_clip;
  /** @brief Set the current transform matrix. */
  CMPGfxSetTransformFn set_transform;
  /** @brief Create a texture resource. */
  CMPGfxCreateTextureFn create_texture;
  /** @brief Update an existing texture. */
  CMPGfxUpdateTextureFn update_texture;
  /** @brief Destroy a texture resource. */
  CMPGfxDestroyTextureFn destroy_texture;
  /** @brief Draw a textured quad. */
  CMPGfxDrawTextureFn draw_texture;
} CMPGfxVTable;

/**
 * @brief Create a font resource.
 * @param text Text backend instance.
 * @param utf8_family Font family name in UTF-8.
 * @param size_px Font size in pixels.
 * @param weight Font weight (100..900).
 * @param italic CMP_TRUE for italic style.
 * @param out_font Receives the created font handle.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTextCreateFontFn)(void *text, const char *utf8_family,
                                           cmp_i32 size_px, cmp_i32 weight,
                                           CMPBool italic, CMPHandle *out_font);

/**
 * @brief Destroy a font resource.
 * @param text Text backend instance.
 * @param font Font handle to destroy.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTextDestroyFontFn)(void *text, CMPHandle font);

/**
 * @brief Measure UTF-8 text using a font.
 * @param text Text backend instance.
 * @param font Font handle to use.
 * @param utf8 UTF-8 string pointer.
 * @param utf8_len Length of the UTF-8 string in bytes.
 * @param out_width Receives the text width in pixels.
 * @param out_height Receives the text height in pixels.
 * @param out_baseline Receives the baseline offset in pixels.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTextMeasureTextFn)(
    void *text, CMPHandle font, const char *utf8, cmp_usize utf8_len,
    CMPScalar *out_width, CMPScalar *out_height, CMPScalar *out_baseline);

/**
 * @brief Draw UTF-8 text.
 * @param text Text backend instance.
 * @param font Font handle to use.
 * @param utf8 UTF-8 string pointer.
 * @param utf8_len Length of the UTF-8 string in bytes.
 * @param x X origin in pixels.
 * @param y Y origin in pixels.
 * @param color Text color.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPTextDrawTextFn)(void *text, CMPHandle font,
                                         const char *utf8, cmp_usize utf8_len,
                                         CMPScalar x, CMPScalar y,
                                         CMPColor color);

/**
 * @brief Text rendering virtual table.
 */
typedef struct CMPTextVTable {
  /** @brief Create a font resource. */
  CMPTextCreateFontFn create_font;
  /** @brief Destroy a font resource. */
  CMPTextDestroyFontFn destroy_font;
  /** @brief Measure UTF-8 text using a font. */
  CMPTextMeasureTextFn measure_text;
  /** @brief Draw UTF-8 text. */
  CMPTextDrawTextFn draw_text;
} CMPTextVTable;

/**
 * @brief Graphics interface.
 */
typedef struct CMPGfx {
  void *ctx;                        /**< Backend context pointer. */
  const CMPGfxVTable *vtable;       /**< Graphics virtual table. */
  const CMPTextVTable *text_vtable; /**< Text virtual table. */
} CMPGfx;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_API_GFX_H */
