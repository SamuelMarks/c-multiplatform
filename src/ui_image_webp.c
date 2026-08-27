/* clang-format off */
#include "ui_image_decoder.h"
#include "ui_error.h"
/* clang-format on */

/*
 * @brief webp_supports_format.
 * @param format Parameter format.
 * @param out_supported Parameter out_supported.
 * @return Return value.
 */
static ui_error_t webp_supports_format(enum ui_image_format format,
                                       int *out_supported) {
  *out_supported = (format == UI_IMAGE_FORMAT_WEBP) ? 1 : 0;
  return UI_ERROR_NONE;
}

/*
 * @brief webp_decode_memory.
 * @param data Parameter data.
 * @param size Parameter size.
 * @param out_image Parameter out_image.
 * @return Return value.
 */
static ui_error_t webp_decode_memory(const void *data, size_t size,
                                     struct ui_image *out_image) {
  if (!data || size == 0 || !out_image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_image->pixels = NULL;
  out_image->width = 0;
  out_image->height = 0;
  out_image->channels = 4;
  out_image->data_size = 0;

  return UI_ERROR_UNKNOWN; /* TODO: Implement actual C89 WebP decoding */
}

/*
 * @brief webp_free_image.
 * @param image Parameter image.
 * @return Return value.
 */
static ui_error_t webp_free_image(struct ui_image *image) {
  if (!image)
    return UI_ERROR_INVALID_ARGUMENT;
  /* TODO: Free pixels when actual implementation is provided */
  image->pixels = NULL;
  return UI_ERROR_NONE;
}

struct ui_image_decoder_backend ui_image_decoder_webp = {
    webp_supports_format, webp_decode_memory, webp_free_image};
