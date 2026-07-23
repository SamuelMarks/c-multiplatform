/* clang-format off */
#include "ui_color_quantization.h"
#include "ui_internal_mem.h"
#include <stddef.h>
/* clang-format on */

struct ui_cluster {
  int r, g, b;
  long long sum_r, sum_g, sum_b;
  int count;
};

static int calc_color_distance(int r1, int g1, int b1, int r2, int g2, int b2) {
  int dr = r1 - r2;
  int dg = g1 - g2;
  int db = b1 - b2;
  return dr * dr + dg * dg + db * db;
}

/** \brief ui_error */
enum ui_error
ui_color_quantize_kmeans(const unsigned char *pixels, size_t width,
                         size_t height, int channels,
                         const struct ui_color_quantization_options *options,
                         ui_color_t *out_colors, size_t *out_color_count) {
  int step;
  int k;
  int iter;
  int num_clusters = 0;
  size_t x, y;
  struct ui_cluster *clusters = NULL;
  enum ui_error rc = UI_ERROR_NONE;
  int has_changed;

  if (!pixels || !options || !out_colors || !out_color_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (channels < 3) {
    return UI_ERROR_UNSUPPORTED;
  }

  k = options->max_colors;
  if (k <= 0) {
    *out_color_count = 0;
    return UI_ERROR_NONE;
  }

  step = options->downsample_step;
  if (step < 1) {
    step = 1;
  }

  clusters = (struct ui_cluster *)UI_MALLOC(sizeof(struct ui_cluster) * k);
  if (!clusters) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  /* Initialize clusters with distinct colors from the image */
  for (y = 0; y < height; y += step) {
    for (x = 0; x < width; x += step) {
      size_t idx = (y * width + x) * channels;
      int r = pixels[idx];
      int g = pixels[idx + 1];
      int b = pixels[idx + 2];
      int is_distinct = 1;
      int c;

      if (num_clusters >= k) {
        break;
      }

      /* Alpha check for RGBA */
      if (channels == 4) {
        if (pixels[idx + 3] < 128) {
          continue; /* Ignore highly transparent pixels */
        }
      }

      for (c = 0; c < num_clusters; ++c) {
        /* If squared distance is < 100, we consider it the same color cluster
         */
        if (calc_color_distance(r, g, b, clusters[c].r, clusters[c].g,
                                clusters[c].b) < 100) {
          is_distinct = 0;
          break;
        }
      }

      if (is_distinct) {
        clusters[num_clusters].r = r;
        clusters[num_clusters].g = g;
        clusters[num_clusters].b = b;
        num_clusters++;
      }
    }
    if (num_clusters >= k) {
      break;
    }
  }

  /* If we couldn't find distinct colors, duplicate the first non-transparent
   * pixel if available */
  if (num_clusters == 0) {
    size_t idx = 0;
    for (; idx < width * height * channels; idx += channels) {
      if (pixels[idx + 3] >= 128) {
        clusters[0].r = pixels[idx];
        clusters[0].g = pixels[idx + 1];
        clusters[0].b = pixels[idx + 2];
        num_clusters = 1;
        break;
      }
    }
    if (num_clusters == 0) {
      /* All transparent or empty */
      UI_FREE(clusters);
      *out_color_count = 0;
      return UI_ERROR_NONE;
    }
  }

  /* K-Means Iterations */
  for (iter = 0; iter < options->max_iterations; ++iter) {
    int c;
    has_changed = 0;

    for (c = 0; c < num_clusters; ++c) {
      clusters[c].sum_r = 0;
      clusters[c].sum_g = 0;
      clusters[c].sum_b = 0;
      clusters[c].count = 0;
    }

    for (y = 0; y < height; y += step) {
      for (x = 0; x < width; x += step) {
        size_t idx = (y * width + x) * channels;
        int r = pixels[idx];
        int g = pixels[idx + 1];
        int b = pixels[idx + 2];
        int best_cluster = 0;
        int min_dist = 2147483647; /* INT_MAX approx */

        if (channels == 4) {
          if (pixels[idx + 3] < 128) {
            continue;
          }
        }

        for (c = 0; c < num_clusters; ++c) {
          int dist = calc_color_distance(r, g, b, clusters[c].r, clusters[c].g,
                                         clusters[c].b);
          if (dist < min_dist) {
            min_dist = dist;
            best_cluster = c;
          }
        }

        clusters[best_cluster].sum_r += r;
        clusters[best_cluster].sum_g += g;
        clusters[best_cluster].sum_b += b;
        clusters[best_cluster].count++;
      }
    }

    for (c = 0; c < num_clusters; ++c) {
      if (clusters[c].count > 0) {
        int new_r = (int)(clusters[c].sum_r / clusters[c].count);
        int new_g = (int)(clusters[c].sum_g / clusters[c].count);
        int new_b = (int)(clusters[c].sum_b / clusters[c].count);

        if (new_r != clusters[c].r || new_g != clusters[c].g ||
            new_b != clusters[c].b) {
          has_changed = 1;
        }

        if (has_changed) {
          clusters[c].r = new_r;
          clusters[c].g = new_g;
          clusters[c].b = new_b;
        }
      }
    }

    if (!has_changed) {
      break;
    }
  }

  /* Simple sort (bubble) by cluster size to get the most dominant colors first
   */
  {
    int i, j;
    for (i = 0; i < num_clusters - 1; ++i) {
      for (j = 0; j < num_clusters - i - 1; ++j) {
        if (clusters[j].count < clusters[j + 1].count) {
          struct ui_cluster temp = clusters[j];
          clusters[j] = clusters[j + 1];
          clusters[j + 1] = temp;
        }
      }
    }
  }

  /* Output */
  {
    int i;
    for (i = 0; i < num_clusters; ++i) {
      out_colors[i] =
          UI_COLOR_ARGB(255, clusters[i].r, clusters[i].g, clusters[i].b);
    }
    *out_color_count = (size_t)num_clusters;
  }

  UI_FREE(clusters);
  return rc;
}
