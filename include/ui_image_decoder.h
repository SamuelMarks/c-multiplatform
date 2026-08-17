/**
 * @file ui_image_decoder.h
 * @brief Image decoding abstractions.
 *
 * This header provides structures and interfaces for decoding standard image
 * formats (PNG, JPEG, WEBP) from memory buffers into raw RGBA pixel data via
 * registered backends.
 */

#ifndef UI_IMAGE_DECODER_H
#define UI_IMAGE_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>

#include "ui_types.h"
/* clang-format on */

/**
 * @brief Enum defining image formats.
 */
enum ui_image_format {
  UI_IMAGE_FORMAT_UNKNOWN = 0, /**< Unknown image format. */
  UI_IMAGE_FORMAT_PNG,         /**< Portable Network Graphics (PNG) format. */
  UI_IMAGE_FORMAT_JPEG, /**< Joint Photographic Experts Group (JPEG) format. */
  UI_IMAGE_FORMAT_WEBP  /**< WebP image format. */
};

/**
 * @brief Structure representing a decoded image.
 */
struct ui_image {
  void *pixels;     /**< RGBA 8888 pixel data. */
  int width;        /**< Width of the image in pixels. */
  int height;       /**< Height of the image in pixels. */
  int channels;     /**< Number of color channels (usually 4 for RGBA). */
  size_t data_size; /**< Total size of the pixel buffer in bytes. */
};

/**
 * @brief Abstract vtable for image decoders.
 */
struct ui_image_decoder_backend {
  /**
   * @brief Checks if this decoder can handle the given format.
   *
   * @param format The image format enum to check.
   * @param out_supported Pointer to receive 1 if supported, 0 otherwise.
   * @return `UI_ERROR_NONE` on success, or an appropriate error code.
   */
  ui_error_t (*supports_format)(enum ui_image_format format,
                                int *out_supported);

  /**
   * @brief Decodes image from memory.
   *
   * @param data Pointer to compressed image data.
   * @param size Size of the compressed data.
   * @param out_image Pointer to receive the decoded image structure.
   * @return `UI_ERROR_NONE` on success, or an appropriate error code.
   */
  ui_error_t (*decode_memory)(const void *data, size_t size,
                              struct ui_image *out_image);

  /**
   * @brief Frees the decoded image pixel data.
   *
   * @param image Pointer to the image structure whose pixels should be freed.
   * @return `UI_ERROR_NONE` on success, or an appropriate error code.
   */
  ui_error_t (*free_image)(struct ui_image *image);
};

/**
 * @brief Decodes an image from memory using the appropriate registered backend.
 *
 * @param format The image format enum.
 * @param data Pointer to compressed image data.
 * @param size Size of the compressed data.
 * @param out_image Pointer to receive the decoded image structure.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_decode_memory(enum ui_image_format format, const void *data,
                                  size_t size, struct ui_image *out_image);

/**
 * @brief Frees a decoded image.
 *
 * @param image Pointer to the image to free.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_free(struct ui_image *image);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_IMAGE_DECODER_H */
