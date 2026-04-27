/* clang-format off */
#include "cmp_screen_capture.h"

#include <stdlib.h>
#include <string.h>

#include "cmp.h"
#include "cmp_log.h"
/* clang-format on */

struct cmp_screen_capture {
  int dummy;
};

/**
 * @brief Creates a screen capture instance.
 *
 * @param out_capture Pointer to receive the created instance.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_capture_create(cmp_screen_capture_t **out_capture) {
  int rc;
  cmp_screen_capture_t *cap;

  rc = CMP_SUCCESS;

  if (out_capture == NULL) {
    LOG_DEBUG("Invalid argument: out_capture is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_screen_capture_t), (void **)&cap);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  cap->dummy = 0;
  *out_capture = cap;
  return CMP_SUCCESS;
}

/**
 * @brief Destroys a screen capture instance.
 *
 * @param capture The instance to destroy.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_capture_destroy(cmp_screen_capture_t *capture) {
  int rc;

  rc = CMP_SUCCESS;

  if (capture == NULL) {
    LOG_DEBUG("Invalid argument: capture is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(capture);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Captures the active window's visual state.
 *
 * @param capture The capture instance.
 * @param out_pixels Pointer to receive the RGBA pixel array. Must be freed.
 * @param out_width Pointer to receive image width.
 * @param out_height Pointer to receive image height.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_capture_active_window(cmp_screen_capture_t *capture,
                                     unsigned char **out_pixels, int *out_width,
                                     int *out_height) {
  int rc;
  int size;
  unsigned char *pixels;

  rc = CMP_SUCCESS;

  if (capture == NULL || out_pixels == NULL || out_width == NULL ||
      out_height == NULL) {
    LOG_DEBUG("Invalid argument: capture, out_pixels, out_width or out_height "
              "is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Mock capture */
  *out_width = 800;
  *out_height = 600;
  size = 800 * 600 * 4;

  rc = CMP_MALLOC(size, (void **)&pixels);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(pixels, 255, size); /* White background mock */
  *out_pixels = pixels;
  return CMP_SUCCESS;
}

/**
 * @brief Captures a specific bounding box of the desktop.
 *
 * @param capture The capture instance.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param width Width of region.
 * @param height Height of region.
 * @param out_pixels Pointer to receive the RGBA pixel array. Must be freed.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_capture_region(cmp_screen_capture_t *capture, int x, int y,
                              int width, int height,
                              unsigned char **out_pixels) {
  int rc;
  int size;
  unsigned char *pixels;

  rc = CMP_SUCCESS;

  if (capture == NULL || out_pixels == NULL) {
    LOG_DEBUG("Invalid argument: capture or out_pixels is NULL\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (width <= 0 || height <= 0) {
    LOG_DEBUG("Invalid argument: width or height must be > 0\n");
    return CMP_ERROR_INVALID_ARG;
  }

  (void)x;
  (void)y;

  /* Mock capture */
  size = width * height * 4;

  rc = CMP_MALLOC(size, (void **)&pixels);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  memset(pixels, 128, size); /* Gray background mock */
  *out_pixels = pixels;
  return CMP_SUCCESS;
}

/**
 * @brief Frees the allocated pixel array.
 *
 * @param pixels The pixel array to free.
 * @return 0 on success, or an error code on failure.
 */
int cmp_screen_capture_free_pixels(unsigned char *pixels) {
  int rc;

  rc = CMP_SUCCESS;

  if (pixels != NULL) {
    rc = CMP_FREE(pixels);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
      return rc;
    }
  }

  return CMP_SUCCESS;
}
