/**
 * @file ui_image_decoder.c
 * @brief ui_image_decoder.c implementation.
 */
/* clang-format off */
#include "ui_image_decoder.h"
#include "ui_error.h"
/* clang-format on */

/** @brief PNG decoder backend */
extern struct ui_image_decoder_backend ui_image_decoder_png;
/** @brief JPEG decoder backend */
extern struct ui_image_decoder_backend ui_image_decoder_jpeg;
/** @brief WebP decoder backend */
extern struct ui_image_decoder_backend ui_image_decoder_webp;

/**
 * @brief get_backend_for_format.
 * @param format Parameter format.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
static ui_error_t
get_backend_for_format(enum ui_image_format format,
                       struct ui_image_decoder_backend **out_backend) {
  int supported = 0;
  ui_error_t err;

  *out_backend = NULL;

  err = ui_image_decoder_png.supports_format(format, &supported);
  if (err != UI_ERROR_NONE)
    return err;
  if (supported) {
    *out_backend = &ui_image_decoder_png;
    return UI_ERROR_NONE;
  }

  err = ui_image_decoder_jpeg.supports_format(format, &supported);
  if (err != UI_ERROR_NONE)
    return err;
  if (supported) {
    *out_backend = &ui_image_decoder_jpeg;
    return UI_ERROR_NONE;
  }

  err = ui_image_decoder_webp.supports_format(format, &supported);
  if (err != UI_ERROR_NONE)
    return err;
  if (supported) {
    *out_backend = &ui_image_decoder_webp;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_image_decode_memory.
 * @param format Parameter format.
 * @param data Parameter data.
 * @param size Parameter size.
 * @param out_image Parameter out_image.
 * @return Return value.
 */
ui_error_t ui_image_decode_memory(enum ui_image_format format, const void *data,
                                  size_t size, struct ui_image *out_image) {
  struct ui_image_decoder_backend *backend;

  if (!data || size == 0 || !out_image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  {
    ui_error_t backend_rc = get_backend_for_format(format, &backend);
    if (backend_rc != UI_ERROR_NONE)
      return backend_rc;
  }
  if (!backend) {
    return UI_ERROR_UNKNOWN;
  }

  return backend->decode_memory(data, size, out_image);
}

/**
 * @brief ui_image_free.
 * @param image Parameter image.
 * @return Return value.
 */
ui_error_t ui_image_free(struct ui_image *image) {
  /* For a real implementation, we might need to store which backend allocated
     the image, but for now we'll just dispatch to a generic free or let the
     backend do it. In this stub, we just safely null it out. */
  if (!image)
    return UI_ERROR_INVALID_ARGUMENT;

  /* TODO: Call appropriate backend free_image */
  image->pixels = NULL;

  return UI_ERROR_NONE;
}
