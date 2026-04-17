#ifndef CMP_SCREEN_CAPTURE_H
#define CMP_SCREEN_CAPTURE_H

/* clang-format off */
#include "cmp_ffi.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_screen_capture.h
 * @brief OS-agnostic screen capture primitives for context sensing.
 */

typedef struct cmp_screen_capture cmp_screen_capture_t;

/**
 * @brief Creates a screen capture instance.
 * @param out_capture Pointer to receive the created instance.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_screen_capture_create(cmp_screen_capture_t **out_capture);

/**
 * @brief Destroys a screen capture instance.
 * @param capture The instance to destroy.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_screen_capture_destroy(cmp_screen_capture_t *capture);

/**
 * @brief Captures the active window's visual state.
 * @param capture The capture instance.
 * @param out_pixels Pointer to receive the RGBA pixel array. Must be freed.
 * @param out_width Pointer to receive image width.
 * @param out_height Pointer to receive image height.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_screen_capture_active_window(cmp_screen_capture_t *capture,
                                             unsigned char **out_pixels,
                                             int *out_width, int *out_height);

/**
 * @brief Captures a specific bounding box of the desktop.
 * @param capture The capture instance.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param width Width of region.
 * @param height Height of region.
 * @param out_pixels Pointer to receive the RGBA pixel array. Must be freed.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_screen_capture_region(cmp_screen_capture_t *capture, int x,
                                      int y, int width, int height,
                                      unsigned char **out_pixels);

/**
 * @brief Frees the allocated pixel array.
 * @param pixels The pixel array to free.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_screen_capture_free_pixels(unsigned char *pixels);

#ifdef __cplusplus
}
#endif

#endif /* CMP_SCREEN_CAPTURE_H */
