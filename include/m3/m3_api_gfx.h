#ifndef M3_API_GFX_H
#define M3_API_GFX_H

/**
 * @file m3_api_gfx.h
 * @brief Graphics abstraction ABI for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_math.h"
#include "m3_object.h"

/** @brief RGBA8 texture format. */
#define M3_TEX_FORMAT_RGBA8 1
/** @brief BGRA8 texture format. */
#define M3_TEX_FORMAT_BGRA8 2
/** @brief Alpha-only 8-bit texture format. */
#define M3_TEX_FORMAT_A8 3

/**
 * @brief Color in linear RGBA space (0..1 range).
 */
typedef struct M3Color {
  M3Scalar r; /**< Red channel. */
  M3Scalar g; /**< Green channel. */
  M3Scalar b; /**< Blue channel. */
  M3Scalar a; /**< Alpha channel. */
} M3Color;

/** @brief Path command: move to. */
#define M3_PATH_CMD_MOVE_TO 1
/** @brief Path command: line to. */
#define M3_PATH_CMD_LINE_TO 2
/** @brief Path command: quadratic curve to. */
#define M3_PATH_CMD_QUAD_TO 3
/** @brief Path command: cubic curve to. */
#define M3_PATH_CMD_CUBIC_TO 4
/** @brief Path command: close path. */
#define M3_PATH_CMD_CLOSE 5

/**
 * @brief Move-to path command payload.
 */
typedef struct M3PathCmdMoveTo {
  M3Scalar x; /**< Target X coordinate. */
  M3Scalar y; /**< Target Y coordinate. */
} M3PathCmdMoveTo;

/**
 * @brief Line-to path command payload.
 */
typedef struct M3PathCmdLineTo {
  M3Scalar x; /**< Target X coordinate. */
  M3Scalar y; /**< Target Y coordinate. */
} M3PathCmdLineTo;

/**
 * @brief Quadratic curve path command payload.
 */
typedef struct M3PathCmdQuadTo {
  M3Scalar cx; /**< Control point X coordinate. */
  M3Scalar cy; /**< Control point Y coordinate. */
  M3Scalar x;  /**< Target X coordinate. */
  M3Scalar y;  /**< Target Y coordinate. */
} M3PathCmdQuadTo;

/**
 * @brief Cubic curve path command payload.
 */
typedef struct M3PathCmdCubicTo {
  M3Scalar cx1; /**< First control point X coordinate. */
  M3Scalar cy1; /**< First control point Y coordinate. */
  M3Scalar cx2; /**< Second control point X coordinate. */
  M3Scalar cy2; /**< Second control point Y coordinate. */
  M3Scalar x;   /**< Target X coordinate. */
  M3Scalar y;   /**< Target Y coordinate. */
} M3PathCmdCubicTo;

/**
 * @brief Path command payload union.
 */
typedef union M3PathCmdData {
  M3PathCmdMoveTo move_to;   /**< Move-to payload. */
  M3PathCmdLineTo line_to;   /**< Line-to payload. */
  M3PathCmdQuadTo quad_to;   /**< Quadratic curve payload. */
  M3PathCmdCubicTo cubic_to; /**< Cubic curve payload. */
} M3PathCmdData;

/**
 * @brief Path command entry.
 */
typedef struct M3PathCmd {
  m3_u32 type;        /**< Command type (M3_PATH_CMD_*). */
  M3PathCmdData data; /**< Command payload. */
} M3PathCmd;

/**
 * @brief Vector path command list.
 */
typedef struct M3Path {
  M3PathCmd *commands;   /**< Command array (owned by the path). */
  m3_usize count;        /**< Number of commands in the path. */
  m3_usize capacity;     /**< Command array capacity. */
  M3Allocator allocator; /**< Allocator used for command storage. */
} M3Path;

/**
 * @brief Graphics virtual table.
 */
typedef struct M3GfxVTable {
  /**
   * @brief Begin rendering a frame for a window.
   * @param gfx Graphics backend instance.
   * @param window Target window handle.
   * @param width Backbuffer width in pixels.
   * @param height Backbuffer height in pixels.
   * @param dpi_scale DPI scale factor.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *begin_frame)(void *gfx, M3Handle window, m3_i32 width,
                            m3_i32 height, M3Scalar dpi_scale);
  /**
   * @brief End rendering a frame for a window.
   * @param gfx Graphics backend instance.
   * @param window Target window handle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *end_frame)(void *gfx, M3Handle window);
  /**
   * @brief Clear the current render target.
   * @param gfx Graphics backend instance.
   * @param color Clear color.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *clear)(void *gfx, M3Color color);
  /**
   * @brief Draw a filled rectangle.
   * @param gfx Graphics backend instance.
   * @param rect Rectangle to draw.
   * @param color Fill color.
   * @param corner_radius Corner radius in pixels.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *draw_rect)(void *gfx, const M3Rect *rect, M3Color color,
                          M3Scalar corner_radius);
  /**
   * @brief Draw a line segment.
   * @param gfx Graphics backend instance.
   * @param x0 Start X coordinate.
   * @param y0 Start Y coordinate.
   * @param x1 End X coordinate.
   * @param y1 End Y coordinate.
   * @param color Line color.
   * @param thickness Line thickness in pixels.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *draw_line)(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1,
                          M3Scalar y1, M3Color color, M3Scalar thickness);
  /**
   * @brief Draw a filled path.
   * @param gfx Graphics backend instance.
   * @param path Path to draw.
   * @param color Fill color.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *draw_path)(void *gfx, const M3Path *path, M3Color color);
  /**
   * @brief Push a clipping rectangle.
   * @param gfx Graphics backend instance.
   * @param rect Clipping rectangle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *push_clip)(void *gfx, const M3Rect *rect);
  /**
   * @brief Pop the most recent clipping rectangle.
   * @param gfx Graphics backend instance.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *pop_clip)(void *gfx);
  /**
   * @brief Set the current transform matrix.
   * @param gfx Graphics backend instance.
   * @param transform Transform matrix.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *set_transform)(void *gfx, const M3Mat3 *transform);
  /**
   * @brief Create a texture resource.
   * @param gfx Graphics backend instance.
   * @param width Texture width in pixels.
   * @param height Texture height in pixels.
   * @param format Texture format (M3_TEX_FORMAT_*).
   * @param pixels Pixel data pointer.
   * @param size Size of pixel data in bytes.
   * @param out_texture Receives the created texture handle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *create_texture)(void *gfx, m3_i32 width, m3_i32 height,
                               m3_u32 format, const void *pixels, m3_usize size,
                               M3Handle *out_texture);
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
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *update_texture)(void *gfx, M3Handle texture, m3_i32 x, m3_i32 y,
                               m3_i32 width, m3_i32 height, const void *pixels,
                               m3_usize size);
  /**
   * @brief Destroy a texture resource.
   * @param gfx Graphics backend instance.
   * @param texture Texture handle to destroy.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *destroy_texture)(void *gfx, M3Handle texture);
  /**
   * @brief Draw a textured quad.
   * @param gfx Graphics backend instance.
   * @param texture Texture handle to draw.
   * @param src Source rectangle in texture space.
   * @param dst Destination rectangle in window space.
   * @param opacity Opacity multiplier (0..1).
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *draw_texture)(void *gfx, M3Handle texture, const M3Rect *src,
                             const M3Rect *dst, M3Scalar opacity);
} M3GfxVTable;

/**
 * @brief Text rendering virtual table.
 */
typedef struct M3TextVTable {
  /**
   * @brief Create a font resource.
   * @param text Text backend instance.
   * @param utf8_family Font family name in UTF-8.
   * @param size_px Font size in pixels.
   * @param weight Font weight (100..900).
   * @param italic M3_TRUE for italic style.
   * @param out_font Receives the created font handle.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *create_font)(void *text, const char *utf8_family, m3_i32 size_px,
                            m3_i32 weight, M3Bool italic, M3Handle *out_font);
  /**
   * @brief Destroy a font resource.
   * @param text Text backend instance.
   * @param font Font handle to destroy.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *destroy_font)(void *text, M3Handle font);
  /**
   * @brief Measure UTF-8 text using a font.
   * @param text Text backend instance.
   * @param font Font handle to use.
   * @param utf8 UTF-8 string pointer.
   * @param utf8_len Length of the UTF-8 string in bytes.
   * @param out_width Receives the text width in pixels.
   * @param out_height Receives the text height in pixels.
   * @param out_baseline Receives the baseline offset in pixels.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *measure_text)(void *text, M3Handle font, const char *utf8,
                             m3_usize utf8_len, M3Scalar *out_width,
                             M3Scalar *out_height, M3Scalar *out_baseline);
  /**
   * @brief Draw UTF-8 text.
   * @param text Text backend instance.
   * @param font Font handle to use.
   * @param utf8 UTF-8 string pointer.
   * @param utf8_len Length of the UTF-8 string in bytes.
   * @param x X origin in pixels.
   * @param y Y origin in pixels.
   * @param color Text color.
   * @return M3_OK on success or a failure code.
   */
  int(M3_CALL *draw_text)(void *text, M3Handle font, const char *utf8,
                          m3_usize utf8_len, M3Scalar x, M3Scalar y,
                          M3Color color);
} M3TextVTable;

/**
 * @brief Graphics interface.
 */
typedef struct M3Gfx {
  void *ctx;                       /**< Backend context pointer. */
  const M3GfxVTable *vtable;       /**< Graphics virtual table. */
  const M3TextVTable *text_vtable; /**< Text virtual table. */
} M3Gfx;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_API_GFX_H */
