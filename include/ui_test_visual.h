#ifndef UI_TEST_VISUAL_H
#define UI_TEST_VISUAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_visual_test_config {
  double rms_threshold;
  double delta_e_threshold;
  double max_drift_percentage;
};

/**
 * @brief Compare two RGBA buffers fuzzy matching.
 * @return 0 if images match within threshold, non-zero if mismatch.
 */
ui_error_t ui_visual_fuzzy_match(const unsigned char *img_a,
                                 const unsigned char *img_b, int width,
                                 int height,
                                 const struct ui_visual_test_config *config,
                                 int *out_matched);

/**
 * @brief Generate a heatmap based on the difference.
 * output_heatmap must point to an allocated RGBA buffer of size width * height
 * * 4. Identical pixels become transparent; differing pixels become scaled
 * magenta.
 */
ui_error_t ui_visual_generate_heatmap(const unsigned char *img_a,
                                      const unsigned char *img_b, int width,
                                      int height,
                                      unsigned char *output_heatmap);

/**
 * @brief Writes a heatmap RGBA buffer to disk as a PNG using stb_image_write.
 * @return 0 on success, non-zero on failure.
 */
ui_error_t ui_visual_write_heatmap_to_disk(const char *filepath,
                                           const unsigned char *heatmap_data,
                                           int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* UI_TEST_VISUAL_H */
