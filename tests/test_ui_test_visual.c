/* clang-format off */
#include "../include/ui_test_visual.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

#define ACCUM_ERR(failed, expr)                                                \
  do {                                                                         \
    ui_error_t err__ = (expr);                                                 \
    if (err__ != UI_ERROR_NONE) {                                              \
      printf("Failed %d at %d\n", err__, __LINE__);                            \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)
#define ACCUM_FAIL(failed, expr)                                               \
  do {                                                                         \
    if (expr) {                                                                \
      printf("Failed true at %d\n", __LINE__);                                 \
      failed = 1;                                                              \
    }                                                                          \
  } while (0)

int main(void) {
  int failed = 0;
  unsigned char img1[16] = {255, 0, 0,   255, 0,   255, 0,   255,
                            0,   0, 255, 255, 255, 255, 255, 255};
  unsigned char img2[16] = {255, 0, 0,   255, 0,   255, 0,   255,
                            0,   0, 255, 255, 255, 255, 255, 255};
  unsigned char img3[16] = {0, 255, 0,   255, 255, 0, 0, 255,
                            0, 0,   255, 255, 0,   0, 0, 255};
  struct ui_visual_test_config cfg = {1.0, 1.0, 1.0};
  int matched = 0;

  ACCUM_ERR(failed, ui_visual_fuzzy_match(img1, img2, 2, 2, &cfg, &matched));
  ACCUM_FAIL(failed, matched != 1);

  ACCUM_ERR(failed, ui_visual_fuzzy_match(img1, img3, 2, 2, &cfg, &matched));
  ACCUM_FAIL(failed, matched != 0);

  ACCUM_FAIL(failed, ui_visual_fuzzy_match(NULL, img2, 2, 2, &cfg, &matched) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_fuzzy_match(img1, NULL, 2, 2, &cfg, &matched) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_fuzzy_match(img1, img2, 0, 2, &cfg, &matched) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_fuzzy_match(img1, img2, 2, 0, &cfg, &matched) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_fuzzy_match(img1, img2, 2, 2, NULL, &matched) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_fuzzy_match(img1, img2, 2, 2, &cfg, NULL) !=
                         UI_ERROR_INVALID_ARGUMENT);

  /* Test drift failure: 1 pixel off out of 4 (25%), set config to allow high
   * RMS but low drift (e.g. 10%) */
  cfg.rms_threshold = 1000.0;
  cfg.max_drift_percentage = 10.0;
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img1, img3, 2, 2, &cfg, &matched));
  ACCUM_FAIL(failed, matched != 0);

  /* Test alpha mismatch only (identical RGB, diff alpha) */
  unsigned char img_a_alpha[4] = {255, 255, 255, 255};
  unsigned char img_b_alpha[4] = {255, 255, 255, 128};
  cfg.rms_threshold = 1000.0;
  cfg.max_drift_percentage = 1.0;
  cfg.delta_e_threshold = 10.0; /* 10 * 2.55 = 25.5. alpha diff is 127 */
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img_a_alpha, img_b_alpha, 1, 1, &cfg,
                                          &matched));
  ACCUM_FAIL(failed, matched != 0);

  /* Test identical but different G */
  unsigned char img_g_diff[4] = {255, 128, 255, 255};
  cfg.delta_e_threshold = 0.0; /* Force it to trigger delta_E mismatch */
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img_a_alpha, img_g_diff, 1, 1, &cfg,
                                          &matched));

  /* Test different B */
  unsigned char img_b_diff[4] = {255, 255, 128, 255};
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img_a_alpha, img_b_diff, 1, 1, &cfg,
                                          &matched));

  /* Test mismatched pixels that are within thresholds to hit the false branch
   * of is_mismatch */
  unsigned char img_tiny_diff[4] = {255, 254, 255, 255};
  cfg.delta_e_threshold = 1000.0; /* Huge threshold so delta_e and alpha checks
                                     fail to trigger mismatch */
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img_a_alpha, img_tiny_diff, 1, 1,
                                          &cfg, &matched));
  ACCUM_FAIL(failed, matched != 1);
  cfg.delta_e_threshold = 10.0;
  ACCUM_ERR(failed, ui_visual_fuzzy_match(img_a_alpha, img_b_alpha, 1, 1, &cfg,
                                          &matched));
  ACCUM_FAIL(failed, matched != 0);

  /* Heatmap intensity clamping and diff paths */
  unsigned char heatmap_out[4];
  /* Force very small delta E (intensity < 50) */
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img_a_alpha, img_tiny_diff, 1, 1,
                                               heatmap_out));

  /* Force very large delta E (intensity > 255) */
  unsigned char img_huge_diff[4] = {0, 0, 0, 255};
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img_a_alpha, img_huge_diff, 1, 1,
                                               heatmap_out));

  /* Force diff B */
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img_a_alpha, img_b_diff, 1, 1,
                                               heatmap_out));

  /* Force diff A */
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img_a_alpha, img_b_alpha, 1, 1,
                                               heatmap_out));

  unsigned char heatmap[16];
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img1, img2, 2, 2, heatmap));
  ACCUM_ERR(failed, ui_visual_generate_heatmap(img1, img3, 2, 2, heatmap));

  ACCUM_FAIL(failed, ui_visual_generate_heatmap(NULL, img2, 2, 2, heatmap) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_generate_heatmap(img1, NULL, 2, 2, heatmap) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_generate_heatmap(img1, img2, 0, 2, heatmap) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_generate_heatmap(img1, img2, 2, 0, heatmap) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed, ui_visual_generate_heatmap(img1, img2, 2, 2, NULL) !=
                         UI_ERROR_INVALID_ARGUMENT);

  ACCUM_ERR(failed,
            ui_visual_write_heatmap_to_disk("test_heatmap.png", heatmap, 2, 2));

  ACCUM_FAIL(failed, ui_visual_write_heatmap_to_disk(NULL, heatmap, 2, 2) !=
                         UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed,
             ui_visual_write_heatmap_to_disk("test_heatmap.png", NULL, 2, 2) !=
                 UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed,
             ui_visual_write_heatmap_to_disk("test_heatmap.png", heatmap, 0,
                                             2) != UI_ERROR_INVALID_ARGUMENT);
  ACCUM_FAIL(failed,
             ui_visual_write_heatmap_to_disk("test_heatmap.png", heatmap, 2,
                                             0) != UI_ERROR_INVALID_ARGUMENT);

  /* Failed IO */
  ACCUM_FAIL(failed, ui_visual_write_heatmap_to_disk(
                         "/invalid/path/that/does/not/exist.png", heatmap, 2,
                         2) != UI_ERROR_IO_FAILED);

  return failed;
}
