/* clang-format off */
#include "ui_image_decoder.h"
#include "ui_error.h"
/* clang-format on */

extern struct ui_image_decoder_backend ui_image_decoder_png;
extern struct ui_image_decoder_backend ui_image_decoder_jpeg;
extern struct ui_image_decoder_backend ui_image_decoder_webp;

static enum ui_error
get_backend_for_format(enum ui_image_format format,
                       struct ui_image_decoder_backend **out_backend) {
  int supported = 0;
  enum ui_error err;

  *out_backend = NULL;

  err = ui_image_decoder_png.supports_format(format, &supported);
  if (err == UI_ERROR_NONE && supported) {
    *out_backend = &ui_image_decoder_png;
    return UI_ERROR_NONE;
  }

  err = ui_image_decoder_jpeg.supports_format(format, &supported);
  if (err == UI_ERROR_NONE && supported) {
    *out_backend = &ui_image_decoder_jpeg;
    return UI_ERROR_NONE;
  }

  err = ui_image_decoder_webp.supports_format(format, &supported);
  if (err == UI_ERROR_NONE && supported) {
    *out_backend = &ui_image_decoder_webp;
    return UI_ERROR_NONE;
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_image_decode_memory(enum ui_image_format format,
                                     const void *data, size_t size,
                                     struct ui_image *out_image) {
  struct ui_image_decoder_backend *backend;

  if (!data || size == 0 || !out_image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  (void)get_backend_for_format(format, &backend);
  if (!backend) {
    return UI_ERROR_UNKNOWN;
  }

  return backend->decode_memory(data, size, out_image);
}

enum ui_error ui_image_free(struct ui_image *image) {
  /* For a real implementation, we might need to store which backend allocated
     the image, but for now we'll just dispatch to a generic free or let the
     backend do it. In this stub, we just safely null it out. */
  if (!image)
    return UI_ERROR_INVALID_ARGUMENT;

  /* TODO: Call appropriate backend free_image */
  image->pixels = NULL;

  return UI_ERROR_NONE;
}
