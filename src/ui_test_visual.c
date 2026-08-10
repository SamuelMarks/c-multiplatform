/* clang-format off */
#include "../include/ui_test_visual.h"
#include <math.h>
#include <stdlib.h>
#include "ui_internal_mem.h"
#include <stdio.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4204) /* nonstandard extension used: non-constant aggregate initializer */
#pragma warning(disable : 4244) /* possible loss of data */
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wc++-compat"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wformat="
#endif
#pragma GCC diagnostic ignored "-Wendif-labels"
#pragma GCC diagnostic ignored "-Wcomment"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
/* clang-format on */

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

static void rgb_to_lab(unsigned char r_in, unsigned char g_in,
                       unsigned char b_in, double *L_out, double *a_out,
                       double *b_out) {
  double r, g, b, x, y, z;
  double x_ref = 95.047;
  double y_ref = 100.000;
  double z_ref = 108.883;

  r = r_in / 255.0;
  g = g_in / 255.0;
  b = b_in / 255.0;

  r = (r > 0.04045) ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
  g = (g > 0.04045) ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
  b = (b > 0.04045) ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

  r *= 100.0;
  g *= 100.0;
  b *= 100.0;

  x = r * 0.4124 + g * 0.3576 + b * 0.1805;
  y = r * 0.2126 + g * 0.7152 + b * 0.0722;
  z = r * 0.0193 + g * 0.1192 + b * 0.9505;

  x /= x_ref;
  y /= y_ref;
  z /= z_ref;

  x = (x > 0.008856) ? pow(x, 1.0 / 3.0) : (7.787 * x) + (16.0 / 116.0);
  y = (y > 0.008856) ? pow(y, 1.0 / 3.0) : (7.787 * y) + (16.0 / 116.0);
  z = (z > 0.008856) ? pow(z, 1.0 / 3.0) : (7.787 * z) + (16.0 / 116.0);

  *L_out = (116.0 * y) - 16.0;
  *a_out = 500.0 * (x - y);
  *b_out = 200.0 * (y - z);
}

static void calculate_delta_e(unsigned char r1, unsigned char g1,
                              unsigned char b1, unsigned char r2,
                              unsigned char g2, unsigned char b2,
                              double *out_delta_e) {
  double L1, a1, b_lab1;
  double L2, a2, b_lab2;

  rgb_to_lab(r1, g1, b1, &L1, &a1, &b_lab1);
  rgb_to_lab(r2, g2, b2, &L2, &a2, &b_lab2);

  *out_delta_e =
      sqrt(pow(L1 - L2, 2.0) + pow(a1 - a2, 2.0) + pow(b_lab1 - b_lab2, 2.0));
}

ui_error_t ui_visual_fuzzy_match(const unsigned char *img_a,
                                 const unsigned char *img_b, int width,
                                 int height,
                                 const struct ui_visual_test_config *config,
                                 int *out_matched) {
  int i;
  int total_pixels = width * height;
  double sum_sq_diff = 0.0;
  int mismatched_pixels = 0;

  if (!img_a || !img_b || !config || width <= 0 || height <= 0 ||
      !out_matched) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_matched = 0;

  for (i = 0; i < total_pixels; ++i) {
    int idx = i * 4;
    unsigned char r1 = img_a[idx];
    unsigned char g1 = img_a[idx + 1];
    unsigned char b1 = img_a[idx + 2];
    unsigned char a1 = img_a[idx + 3];

    unsigned char r2 = img_b[idx];
    unsigned char g2 = img_b[idx + 1];
    unsigned char b2 = img_b[idx + 2];
    unsigned char a2 = img_b[idx + 3];

    double diff_r = (double)r1 - (double)r2;
    double diff_g = (double)g1 - (double)g2;
    double diff_b = (double)b1 - (double)b2;
    double diff_a = (double)a1 - (double)a2;

    sum_sq_diff += (diff_r * diff_r) + (diff_g * diff_g) + (diff_b * diff_b) +
                   (diff_a * diff_a);

    if (r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) {
      double delta_e;
      int is_mismatch = 0;
      calculate_delta_e(r1, g1, b1, r2, g2, b2, &delta_e);
      /* Include alpha difference conceptually in drift, although deltaE is RGB
       * only */
      if (delta_e > config->delta_e_threshold) {
        is_mismatch = 1;
      } else if (abs((int)a1 - (int)a2) >
                 (int)(config->delta_e_threshold * 2.55)) {
        is_mismatch = 1;
      }
      if (is_mismatch) {
        mismatched_pixels++;
      }
    }
  }

  {
    double rms = sqrt(sum_sq_diff / (double)(total_pixels * 4));
    double drift_percentage =
        ((double)mismatched_pixels / (double)total_pixels) * 100.0;

    *out_matched = 1; /* Match */
    if (rms > config->rms_threshold) {
      *out_matched = 0; /* Mismatch */
    } else if (drift_percentage > config->max_drift_percentage) {
      *out_matched = 0; /* Mismatch */
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_visual_generate_heatmap(const unsigned char *img_a,
                                      const unsigned char *img_b, int width,
                                      int height,
                                      unsigned char *output_heatmap) {
  int i;
  int total_pixels = width * height;

  if (!img_a || !img_b || !output_heatmap || width <= 0 || height <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < total_pixels; ++i) {
    int idx = i * 4;
    unsigned char r1 = img_a[idx];
    unsigned char g1 = img_a[idx + 1];
    unsigned char b1 = img_a[idx + 2];
    unsigned char a1 = img_a[idx + 3];

    unsigned char r2 = img_b[idx];
    unsigned char g2 = img_b[idx + 1];
    unsigned char b2 = img_b[idx + 2];
    unsigned char a2 = img_b[idx + 3];

    int identical = 0;
    if (r1 == r2) {
      if (g1 == g2) {
        if (b1 == b2) {
          if (a1 == a2) {
            identical = 1;
          }
        }
      }
    }

    if (identical) {
      output_heatmap[idx] = 0;     /* R */
      output_heatmap[idx + 1] = 0; /* G */
      output_heatmap[idx + 2] = 0; /* B */
      output_heatmap[idx + 3] = 0; /* A (Transparent) */
    } else {
      double delta_e;
      int intensity;
      calculate_delta_e(r1, g1, b1, r2, g2, b2, &delta_e);
      /* Scale intensity based on difference */
      intensity = (int)((delta_e / 100.0) * 255.0);
      if (intensity > 255)
        intensity = 255;
      if (intensity < 50)
        intensity = 50; /* minimum visibility */

      output_heatmap[idx] = 255;                          /* R */
      output_heatmap[idx + 1] = 0;                        /* G */
      output_heatmap[idx + 2] = 255;                      /* B (Magenta) */
      output_heatmap[idx + 3] = (unsigned char)intensity; /* A */
    }
  }

  return UI_ERROR_NONE;
}

static void log_stbi_write_c_file(void *context, void *data, int size) {
  FILE *f = (FILE *)context;
  fwrite(data, 1, size, f);
}

ui_error_t ui_visual_write_heatmap_to_disk(const char *filepath,
                                           const unsigned char *heatmap_data,
                                           int width, int height) {
  FILE *f = NULL;
  int rc;

  if (!filepath || !heatmap_data || width <= 0 || height <= 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, filepath, "wb") != 0) {
    return UI_ERROR_IO_FAILED;
  }
#else
  f = fopen(filepath, "wb");
  if (!f) {
    return UI_ERROR_IO_FAILED;
  }
#endif

  rc = stbi_write_png_to_func(log_stbi_write_c_file, f, width, height, 4,
                              heatmap_data, width * 4);
  fclose(f);

  return (rc == 0) ? UI_ERROR_IO_FAILED : UI_ERROR_NONE;
}
