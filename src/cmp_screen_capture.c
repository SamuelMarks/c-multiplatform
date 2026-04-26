/* clang-format off */
#include "cmp_screen_capture.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_screen_capture {
  int dummy;
};

/**
 * @brief cmp_screen_capture_create
 *
 * @param out_capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_screen_capture_create(cmp_screen_capture_t **out_capture) {
  int rc = CMP_SUCCESS;
  cmp_screen_capture_t *cap;
  if (!out_capture) {
    return -1;
  }
  rc = CMP_MALLOC(sizeof(cmp_screen_capture_t), (void **)&(cap));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  cap->dummy = 0;
  *out_capture = cap;
  return 0;
}

/**
 * @brief cmp_screen_capture_destroy
 *
 * @param capture Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_screen_capture_destroy(cmp_screen_capture_t *capture) {
  int rc = CMP_SUCCESS;
  if (!capture) {
    return -1;
  }
  rc = CMP_FREE(capture);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}

/**
 * @brief cmp_screen_capture_active_window
 *
 * @param capture Parameter description.
 * @param out_pixels Parameter description.
 * @param out_width Parameter description.
 * @param out_height Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_screen_capture_active_window(cmp_screen_capture_t *capture,
                                     unsigned char **out_pixels, int *out_width,
                                     int *out_height) {
  int rc = CMP_SUCCESS;
  int size;
  unsigned char *pixels;
  if (!capture || !out_pixels || !out_width || !out_height) {
    return -1;
  }

  /* Mock capture */
  *out_width = 800;
  *out_height = 600;
  size = 800 * 600 * 4;
  rc = CMP_MALLOC(size, (void **)&(pixels));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(pixels, 255, size); /* White background mock */
  *out_pixels = pixels;
  return 0;
}

/**
 * @brief cmp_screen_capture_region
 *
 * @param capture Parameter description.
 * @param x Parameter description.
 * @param y Parameter description.
 * @param width Parameter description.
 * @param height Parameter description.
 * @param out_pixels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_screen_capture_region(cmp_screen_capture_t *capture, int x, int y,
                              int width, int height,
                              unsigned char **out_pixels) {
  int rc = CMP_SUCCESS;
  int size;
  unsigned char *pixels;
  if (!capture || !out_pixels || width <= 0 || height <= 0) {
    return -1;
  }
  (void)x;
  (void)y;

  /* Mock capture */
  size = width * height * 4;
  rc = CMP_MALLOC(size, (void **)&(pixels));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }
  memset(pixels, 128, size); /* Gray background mock */
  *out_pixels = pixels;
  return 0;
}

/**
 * @brief cmp_screen_capture_free_pixels
 *
 * @param pixels Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_screen_capture_free_pixels(unsigned char *pixels) {
  int rc = CMP_SUCCESS;
  if (pixels) {
    rc = CMP_FREE(pixels);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  return 0;
}
