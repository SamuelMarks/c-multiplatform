/**
 * @file rasterizer.h
 * @brief High-precision 2D software rasterizer for cross-platform screenshot
 * generation.
 */

#ifndef MD3_RASTERIZER_H
#define MD3_RASTERIZER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "material3/md3_color.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @enum icon_type
 * @brief Vector icon identifiers for Material Symbols and system icons.
 */
enum icon_type {
  ICON_CHECK,
  ICON_CLOSE,
  ICON_SEARCH,
  ICON_CALENDAR,
  ICON_CLOCK,
  ICON_ARROW_DOWN,
  ICON_ARROW_UP,
  ICON_ARROW_LEFT,
  ICON_ARROW_RIGHT,
  ICON_ADD,
  ICON_EDIT,
  ICON_MORE_VERT,
  ICON_MENU,
  ICON_STAR,
  ICON_STAR_OUTLINE,
  ICON_HEART,
  ICON_PERSON,
  ICON_SETTINGS,
  ICON_HOME,
  ICON_BELL,
  ICON_INFO,
  ICON_WARNING,
  ICON_DELETE,
  ICON_SHARE,
  ICON_CLOUD_UPLOAD,
  ICON_PLAY,
  ICON_PAUSE,
  ICON_DRAG_HANDLE,
  ICON_FOLDER,
  ICON_FILTER,
  ICON_REFRESH,
  ICON_TYPE_COUNT
};

/**
 * @struct canvas
 * @brief 32-bit RGBA offscreen pixel buffer.
 */
struct canvas {
  int width;             /**< Width of canvas in pixels */
  int height;            /**< Height of canvas in pixels */
  unsigned char *pixels; /**< Pointer to RGBA buffer (width * height * 4) */
};

/**
 * @brief Sets the global DPI scale factor for coordinate and size scaling.
 * @param dpi_scale The scale factor (e.g. 1.0f or 2.0f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t rasterizer_set_dpi_scale(float dpi_scale);

/**
 * @brief Gets the global DPI scale factor.
 * @param out_dpi_scale Pointer to receive scale factor.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t rasterizer_get_dpi_scale(float *out_dpi_scale);

/**
 * @brief Allocates a new canvas with the specified width and height.
 * @param width Canvas width in pixels.
 * @param height Canvas height in pixels.
 * @param out_canvas Pointer to receive the allocated canvas.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
ui_error_t canvas_create(int width, int height, struct canvas **out_canvas);

/**
 * @brief Frees all memory associated with a canvas.
 * @param c Pointer to the canvas to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if c is NULL.
 */
ui_error_t canvas_destroy(struct canvas *c);

/**
 * @brief Clears the entire canvas with a solid color.
 * @param c Target canvas.
 * @param color ARGB 32-bit fill color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t canvas_clear(struct canvas *c, ui_color_t color);

/**
 * @brief Draws an anti-aliased solid rounded rectangle.
 * @param c Target canvas.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param w Width of rectangle.
 * @param h Height of rectangle.
 * @param radius Corner radius in pixels.
 * @param color ARGB 32-bit fill color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_rounded_rect(struct canvas *c, float x, float y, float w,
                             float h, float radius, ui_color_t color);

/**
 * @brief Draws an anti-aliased stroked (outline) rounded rectangle.
 * @param c Target canvas.
 * @param x Left coordinate.
 * @param y Top coordinate.
 * @param w Width of rectangle.
 * @param h Height of rectangle.
 * @param radius Corner radius in pixels.
 * @param stroke_width Border thickness in pixels.
 * @param color ARGB 32-bit border color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_rounded_rect_stroke(struct canvas *c, float x, float y, float w,
                                    float h, float radius, float stroke_width,
                                    ui_color_t color);

/**
 * @brief Draws a photorealistic elevation drop shadow.
 * @param c Target canvas.
 * @param x Left coordinate of the shadowed element.
 * @param y Top coordinate of the shadowed element.
 * @param w Width of the shadowed element.
 * @param h Height of the shadowed element.
 * @param radius Corner radius of the element.
 * @param elevation Elevation level (1 to 5).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_shadow(struct canvas *c, float x, float y, float w, float h,
                       float radius, int elevation);

/**
 * @brief Draws an anti-aliased solid circle.
 * @param c Target canvas.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param radius Radius in pixels.
 * @param color ARGB 32-bit fill color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_circle(struct canvas *c, float cx, float cy, float radius,
                       ui_color_t color);

/**
 * @brief Draws an anti-aliased stroked circle.
 * @param c Target canvas.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param radius Radius in pixels.
 * @param stroke_width Stroke width in pixels.
 * @param color ARGB 32-bit stroke color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_circle_stroke(struct canvas *c, float cx, float cy,
                              float radius, float stroke_width,
                              ui_color_t color);

/**
 * @brief Draws an anti-aliased straight line segment.
 * @param c Target canvas.
 * @param x0 Start X.
 * @param y0 Start Y.
 * @param x1 End X.
 * @param y1 End Y.
 * @param width Line thickness in pixels.
 * @param color ARGB 32-bit line color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_line(struct canvas *c, float x0, float y0, float x1, float y1,
                     float width, ui_color_t color);

/**
 * @brief Measures the pixel width of a string at a given scale.
 * @param text The ASCII text string.
 * @param scale Font scaling multiplier (1.0 = 8px wide per char).
 * @param out_width Pointer to receive width in pixels.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t measure_text_width(const char *text, float scale, float *out_width);

/**
 * @brief Loads a TrueType font for vector typography in screenshot rendering.
 * @param ttf_path Path to the TTF file.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t rasterizer_load_font(const char *ttf_path);

/**
 * @brief Draws anti-aliased text.
 * @param c Target canvas.
 * @param x Left baseline X.
 * @param y Top baseline Y.
 * @param text The ASCII string to render.
 * @param scale Font scale factor.
 * @param color ARGB 32-bit text color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_text(struct canvas *c, float x, float y, const char *text,
                     float scale, ui_color_t color);

/**
 * @brief Draws anti-aliased text horizontally centered around cx.
 * @param c Target canvas.
 * @param cx Center X coordinate.
 * @param y Top baseline Y.
 * @param text The ASCII string to render.
 * @param scale Font scale factor.
 * @param color ARGB 32-bit text color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_text_centered(struct canvas *c, float cx, float y,
                              const char *text, float scale, ui_color_t color);

/**
 * @brief Draws a vector icon centered at (cx, cy).
 * @param c Target canvas.
 * @param cx Center X coordinate.
 * @param cy Center Y coordinate.
 * @param type Icon enum identifier.
 * @param size Icon width/height in pixels.
 * @param color ARGB 32-bit icon color.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t draw_icon(struct canvas *c, float cx, float cy, enum icon_type type,
                     float size, ui_color_t color);

/**
 * @brief Writes the canvas pixels directly to a PNG file.
 * @param c Target canvas.
 * @param filepath Full path to output PNG file.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t save_canvas_png(const struct canvas *c, const char *filepath);

#ifdef __cplusplus
}
#endif

#endif /* MD3_RASTERIZER_H */
