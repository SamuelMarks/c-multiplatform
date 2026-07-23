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
  UI_IMAGE_FORMAT_UNKNOWN = 0,
  UI_IMAGE_FORMAT_PNG,
  UI_IMAGE_FORMAT_JPEG,
  UI_IMAGE_FORMAT_WEBP
};

/**
 * @brief Structure representing a decoded image.
 */
struct ui_image {
  void *pixels; /* RGBA 8888 pixel data */
  int width;
  int height;
  int channels;     /* Number of color channels (usually 4 for RGBA) */
  size_t data_size; /* Total size of the pixel buffer */
};

/**
 * @brief Abstract vtable for image decoders.
 */
struct ui_image_decoder_backend {
  /**
   * @brief Checks if this decoder can handle the given format.
   *
   * @param format The image format enum.
   * @return 1 if supported, 0 otherwise.
   */
  enum ui_error (*supports_format)(enum ui_image_format format,
                                   int *out_supported);

  /**
   * @brief Decodes image from memory.
   *
   * @param data Pointer to compressed image data.
   * @param size Size of the compressed data.
   * @param out_image Pointer to receive the decoded image structure.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*decode_memory)(const void *data, size_t size,
                                 struct ui_image *out_image);

  /**
   * @brief Frees the decoded image pixel data.
   *
   * @param image The image to free.
   * @return UI_ERROR_NONE on success.
   */
  enum ui_error (*free_image)(struct ui_image *image);
};

/**
 * @brief Decodes an image from memory using the appropriate registered backend.
 *
 * @param format The image format enum.
 * @param data Pointer to compressed image data.
 * @param size Size of the compressed data.
 * @param out_image Pointer to receive the decoded image structure.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_image_decode_memory(enum ui_image_format format,
                                     const void *data, size_t size,
                                     struct ui_image *out_image);

/**
 * @brief Frees a decoded image.
 *
 * @param image The image to free.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_image_free(struct ui_image *image);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_IMAGE_DECODER_H */
