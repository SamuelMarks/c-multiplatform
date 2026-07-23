#ifndef UI_COLOR_QUANTIZATION_H
#define UI_COLOR_QUANTIZATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_color_space.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Configuration for color quantization algorithm.
 */
struct ui_color_quantization_options {
  int max_colors; /**< Maximum number of dominant colors to extract (e.g., 4) */
  int max_iterations;  /**< Maximum number of k-means iterations (e.g., 10) */
  int downsample_step; /**< Step size to skip pixels for performance (e.g., 10)
                        */
};

/**
 * @brief Extracts dominant colors from raw RGB/RGBA pixel data using K-Means
 * clustering.
 *
 * This algorithm is agnostic to the UI elements and processes raw pixel buffers
 * to identify dominant seed colors, typically used for Material You dynamic
 * palettes.
 *
 * @param pixels Raw byte array of pixels.
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param channels Number of bytes per pixel (3 for RGB, 4 for RGBA).
 * @param options Quantization options.
 * @param out_colors Pointer to a pre-allocated array of `max_colors` size to
 * receive colors.
 * @param out_color_count Pointer to receive the actual number of colors
 * extracted.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_color_quantize_kmeans(const unsigned char *pixels, size_t width,
                         size_t height, int channels,
                         const struct ui_color_quantization_options *options,
                         ui_color_t *out_colors, size_t *out_color_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_COLOR_QUANTIZATION_H */
