/**
 * @file ui_image_jpeg.c
 * @brief ui_image_jpeg.c implementation.
 */
/* clang-format off */
#include "ui_image_decoder.h"
#include "ui_error.h"
#include "stb_image.h"
/* clang-format on */

/**
 * @brief jpeg_supports_format.
 * @param format Parameter format.
 * @param out_supported Parameter out_supported.
 * @return Return value.
 */
static ui_error_t jpeg_supports_format(enum ui_image_format format,
                                       int *out_supported) {
  *out_supported = (format == UI_IMAGE_FORMAT_JPEG) ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief jpeg_decode_memory.
 * @param data Parameter data.
 * @param size Parameter size.
 * @param out_image Parameter out_image.
 * @return Return value.
 */
static ui_error_t jpeg_decode_memory(const void *data, size_t size,
                                     struct ui_image *out_image) {
  int w, h, channels;
  stbi_uc *pixels;

  if (!data || size == 0 || !out_image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  pixels = stbi_load_from_memory((const stbi_uc *)data, (int)size, &w, &h,
                                 &channels, 4);
  if (!pixels) {
    return UI_ERROR_UNKNOWN;
  }

  out_image->pixels = pixels;
  out_image->width = w;
  out_image->height = h;
  out_image->channels = 4;
  out_image->data_size = (size_t)w * (size_t)h * 4;

  return UI_ERROR_NONE;
}

/**
 * @brief jpeg_free_image.
 * @param image Parameter image.
 * @return Return value.
 */
static ui_error_t jpeg_free_image(struct ui_image *image) {
  if (!image)
    return UI_ERROR_INVALID_ARGUMENT;
  if (image->pixels) {
    stbi_image_free(image->pixels);
    image->pixels = NULL;
  }
  return UI_ERROR_NONE;
}

/** @brief JPEG decoder backend */
struct ui_image_decoder_backend ui_image_decoder_jpeg = {
    jpeg_supports_format, jpeg_decode_memory, jpeg_free_image};
