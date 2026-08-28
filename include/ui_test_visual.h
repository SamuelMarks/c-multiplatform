/**
 * @file ui_test_visual.h
 * @brief Visual testing utilities.
 */
#ifndef UI_TEST_VISUAL_H
#define UI_TEST_VISUAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @struct ui_visual_test_config
 * @brief Configuration for visual tests.
 */
struct ui_visual_test_config {
  double rms_threshold;        /**< RMS error threshold. */
  double delta_e_threshold;    /**< Delta E threshold. */
  double max_drift_percentage; /**< Max drift. */
};

/**
 * @brief Compare two RGBA buffers fuzzy matching.
 *
 * @param img_a First image RGBA buffer.
 * @param img_b Second image RGBA buffer.
 * @param width Width of both images in pixels.
 * @param height Height of both images in pixels.
 * @param config Visual test configuration.
 * @param out_matched Pointer to an integer, set to non-zero if images match.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_visual_fuzzy_match(const unsigned char *img_a,
                                 const unsigned char *img_b, int width,
                                 int height,
                                 const struct ui_visual_test_config *config,
                                 int *out_matched);

/**
 * @brief Generate a heatmap based on the difference.
 *
 * output_heatmap must point to an allocated RGBA buffer of size width * height
 * * 4. Identical pixels become transparent; differing pixels become scaled
 * magenta.
 *
 * @param img_a First image RGBA buffer.
 * @param img_b Second image RGBA buffer.
 * @param width Width of both images.
 * @param height Height of both images.
 * @param output_heatmap Buffer to store the resulting heatmap RGBA pixels.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_visual_generate_heatmap(const unsigned char *img_a,
                                      const unsigned char *img_b, int width,
                                      int height,
                                      unsigned char *output_heatmap);

/**
 * @brief Writes a heatmap RGBA buffer to disk as a PNG using stb_image_write.
 *
 * @param filepath The destination path for the PNG.
 * @param heatmap_data The RGBA buffer of the heatmap.
 * @param width Width of the heatmap.
 * @param height Height of the heatmap.
 * @return UI_ERROR_NONE on success, UI_ERROR_IO on write failure.
 */
ui_error_t ui_visual_write_heatmap_to_disk(const char *filepath,
                                           const unsigned char *heatmap_data,
                                           int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* UI_TEST_VISUAL_H */
