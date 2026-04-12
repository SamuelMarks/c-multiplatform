/* clang-format off */
#include "cmp_screen_capture.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_screen_capture {
  int dummy;
};

int cmp_screen_capture_create(cmp_screen_capture_t **out_capture) {
  cmp_screen_capture_t *cap;
  if (!out_capture) {
    return -1;
  }
  cap = (cmp_screen_capture_t *)malloc(sizeof(cmp_screen_capture_t));
  if (!cap) {
    return -2;
  }
  cap->dummy = 0;
  *out_capture = cap;
  return 0;
}

int cmp_screen_capture_destroy(cmp_screen_capture_t *capture) {
  if (!capture) {
    return -1;
  }
  free(capture);
  return 0;
}

int cmp_screen_capture_active_window(cmp_screen_capture_t *capture,
                                     unsigned char **out_pixels, int *out_width,
                                     int *out_height) {
  int size;
  unsigned char *pixels;
  if (!capture || !out_pixels || !out_width || !out_height) {
    return -1;
  }

  /* Mock capture */
  *out_width = 800;
  *out_height = 600;
  size = 800 * 600 * 4;
  pixels = (unsigned char *)malloc(size);
  if (!pixels) {
    return -2;
  }
  memset(pixels, 255, size); /* White background mock */
  *out_pixels = pixels;
  return 0;
}

int cmp_screen_capture_region(cmp_screen_capture_t *capture, int x, int y,
                              int width, int height,
                              unsigned char **out_pixels) {
  int size;
  unsigned char *pixels;
  if (!capture || !out_pixels || width <= 0 || height <= 0) {
    return -1;
  }
  (void)x;
  (void)y;

  /* Mock capture */
  size = width * height * 4;
  pixels = (unsigned char *)malloc(size);
  if (!pixels) {
    return -2;
  }
  memset(pixels, 128, size); /* Gray background mock */
  *out_pixels = pixels;
  return 0;
}

int cmp_screen_capture_free_pixels(unsigned char *pixels) {
  if (pixels) {
    free(pixels);
  }
  return 0;
}
