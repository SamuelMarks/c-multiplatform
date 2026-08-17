/**
 * @file ui_font_manager.h
 * @brief Font loading, management, and atlas generation.
 *
 * This header defines structures, enumerations, and functions for handling
 * font resources, caching, and generating glyph atlases for rendering text.
 */

#ifndef UI_FONT_MANAGER_H
#define UI_FONT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Forward declaration of the font manager structure.
 */
struct ui_font_manager;

/**
 * @brief Forward declaration of a font structure.
 */
struct ui_font;

/**
 * @brief Represents font loading status (CSS Font Loading Module Level 3).
 */
enum ui_font_status {
  UI_FONT_STATUS_UNLOADED, /**< Font is not loaded. */
  UI_FONT_STATUS_LOADING,  /**< Font is currently loading. */
  UI_FONT_STATUS_LOADED,   /**< Font is successfully loaded. */
  UI_FONT_STATUS_ERROR     /**< Font failed to load. */
};

/**
 * @brief Glyph metrics for layout.
 */
struct ui_glyph_metrics {
  int width;     /**< Width of the glyph. */
  int height;    /**< Height of the glyph. */
  int bearing_x; /**< Horizontal bearing (offset from origin to left edge). */
  int bearing_y; /**< Vertical bearing (offset from origin to top edge). */
  int advance;   /**< Horizontal advance to the next glyph. */
};

/**
 * @brief Variable font axis.
 */
struct ui_font_axis {
  unsigned int tag; /**< Tag representing the axis (e.g., 'wght', 'opsz'). */
  float value;      /**< Value for this specific axis. */
};

/**
 * @brief Creates a font manager for loading fonts and generating dynamic glyph
 * atlases.
 *
 * @param out_manager Pointer to receive the allocated font manager.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_manager_create(struct ui_font_manager **out_manager);

/**
 * @brief Destroys a font manager and all loaded fonts.
 *
 * @param manager Pointer to the font manager to destroy.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_manager_destroy(struct ui_font_manager *manager);

/**
 * @brief Loads a font from memory.
 *
 * @param manager Pointer to the font manager.
 * @param font_data Pointer to the TTF/OTF font data.
 * @param data_size Size of the font data.
 * @param out_font Pointer to receive the loaded font handle.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_manager_load_font_memory(struct ui_font_manager *manager,
                                            const unsigned char *font_data,
                                            size_t data_size,
                                            struct ui_font **out_font);

/**
 * @brief Sets the CSS properties for a font for tracking.
 *
 * @param font Pointer to the font.
 * @param family The CSS font-family name.
 * @param weight The font weight (e.g., 400 for normal, 700 for bold).
 * @param is_italic 1 if italic/oblique, 0 otherwise.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_set_metadata(struct ui_font *font, const char *family,
                                int weight, int is_italic);

/**
 * @brief Gets the load status of the font.
 *
 * @param font Pointer to the font.
 * @param out_status Pointer to receive the font status.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_status(struct ui_font *font,
                              enum ui_font_status *out_status);

/**
 * @brief Sets the load status of the font (useful for async networking
 * tracking).
 *
 * @param font Pointer to the font.
 * @param status The new font status.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_set_status(struct ui_font *font, enum ui_font_status status);

/**
 * @brief Finds a font matching the CSS requirements in the manager.
 *
 * @param manager Pointer to the font manager.
 * @param family The CSS font-family name.
 * @param weight The font weight.
 * @param is_italic 1 if italic/oblique, 0 otherwise.
 * @param out_font Pointer to receive the matching font handle.
 * @return `UI_ERROR_NONE` if found, `UI_ERROR_NOT_FOUND` if not found, or an
 * appropriate error code.
 */
ui_error_t ui_font_manager_find_font(struct ui_font_manager *manager,
                                     const char *family, int weight,
                                     int is_italic, struct ui_font **out_font);

/**
 * @brief Sets the variable font axes for a specific font instance.
 *
 * @param font Pointer to the font.
 * @param axes Array of variable font axes.
 * @param axis_count Number of axes in the array.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_set_variations(struct ui_font *font,
                                  const struct ui_font_axis *axes,
                                  int axis_count);

/**
 * @brief Gets the variable font axes set for a specific font instance.
 *
 * @param font Pointer to the font.
 * @param out_axes Pointer to receive the array of axes.
 * @param out_axis_count Pointer to receive the number of axes.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_variations(struct ui_font *font,
                                  struct ui_font_axis **out_axes,
                                  int *out_axis_count);

/**
 * @brief Retrieves the glyph metrics for a character at a specific size.
 *
 * @param font Pointer to the font.
 * @param codepoint The Unicode codepoint.
 * @param font_size The size of the font in pixels.
 * @param out_metrics Pointer to receive the glyph metrics.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_glyph_metrics(struct ui_font *font, int codepoint,
                                     float font_size,
                                     struct ui_glyph_metrics *out_metrics);

/**
 * @brief Retrieves vertical font metrics for a specific size.
 *
 * @param font Pointer to the font.
 * @param font_size The size of the font in pixels.
 * @param out_ascent Pointer to receive the ascent.
 * @param out_descent Pointer to receive the descent.
 * @param out_line_gap Pointer to receive the line gap.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_vmetrics(struct ui_font *font, float font_size,
                                float *out_ascent, float *out_descent,
                                float *out_line_gap);

/**
 * @brief Retrieves the kerning advance between two codepoints for a specific
 * size.
 *
 * @param font Pointer to the font.
 * @param codepoint1 The first Unicode codepoint.
 * @param codepoint2 The second Unicode codepoint.
 * @param font_size The size of the font in pixels.
 * @param out_kerning Pointer to receive the kerning advance.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_kerning(struct ui_font *font, int codepoint1,
                               int codepoint2, float font_size,
                               float *out_kerning);

/**
 * @brief Retrieves the raw font data.
 *
 * @param font Pointer to the font.
 * @param out_data Pointer to receive the raw font data buffer.
 * @param out_size Pointer to receive the size of the font data.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_get_data(struct ui_font *font,
                            const unsigned char **out_data, size_t *out_size);

/**
 * @brief Generates an atlas texture for a set of codepoints.
 *
 * @param font Pointer to the font.
 * @param font_size The size of the font in pixels.
 * @param codepoints Array of Unicode codepoints to include in the atlas.
 * @param codepoint_count Number of codepoints in the array.
 * @param out_atlas_rgba Pointer to receive the generated RGBA atlas buffer.
 * @param out_width Pointer to receive the generated atlas width.
 * @param out_height Pointer to receive the generated atlas height.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_generate_atlas(struct ui_font *font, float font_size,
                                  const int *codepoints, int codepoint_count,
                                  unsigned char **out_atlas_rgba,
                                  int *out_width, int *out_height);

/**
 * @brief Frees a generated atlas buffer.
 *
 * @param atlas_rgba Pointer to the atlas buffer to free.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_font_free_atlas(unsigned char *atlas_rgba);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_FONT_MANAGER_H */
