/* clang-format off */
#include "ui_image_decoder.h"
#include <stdio.h>
/* clang-format on */

extern struct ui_image_decoder_backend ui_image_decoder_png;
extern struct ui_image_decoder_backend ui_image_decoder_jpeg;
extern struct ui_image_decoder_backend ui_image_decoder_webp;

static ui_error_t (*orig_png_supports)(enum ui_image_format, int *);
static ui_error_t (*orig_jpeg_supports)(enum ui_image_format, int *);
static ui_error_t (*orig_webp_supports)(enum ui_image_format, int *);

static ui_error_t mock_png_supports_fail(enum ui_image_format format,
                                         int *out) {
  (void)format;
  (void)out;
  return UI_ERROR_UNKNOWN;
}
static ui_error_t mock_jpeg_supports_fail(enum ui_image_format format,
                                          int *out) {
  (void)format;
  (void)out;
  return UI_ERROR_UNKNOWN;
}
static ui_error_t mock_webp_supports_fail(enum ui_image_format format,
                                          int *out) {
  (void)format;
  (void)out;
  return UI_ERROR_UNKNOWN;
}

int main(void) {
  struct ui_image image;
  ui_error_t err;
  const char *dummy_data = "dummy";
  int supported = 0;

  /* Save original function pointers */
  orig_png_supports = ui_image_decoder_png.supports_format;
  orig_jpeg_supports = ui_image_decoder_jpeg.supports_format;
  orig_webp_supports = ui_image_decoder_webp.supports_format;

  /* Test ui_image_decode_memory */
  if (ui_image_decode_memory(UI_IMAGE_FORMAT_PNG, NULL, 5, &image) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decode_memory(UI_IMAGE_FORMAT_PNG, dummy_data, 0, &image) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decode_memory(UI_IMAGE_FORMAT_PNG, dummy_data, 5, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decode_memory((enum ui_image_format) - 1, dummy_data, 5,
                             &image) != UI_ERROR_UNKNOWN)
    return 1;

  err = ui_image_decode_memory(UI_IMAGE_FORMAT_PNG, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN && err != UI_ERROR_NONE)
    return 1;

  err = ui_image_decode_memory(UI_IMAGE_FORMAT_JPEG, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN && err != UI_ERROR_NONE)
    return 1;

  err = ui_image_decode_memory(UI_IMAGE_FORMAT_WEBP, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN && err != UI_ERROR_NONE)
    return 1;

  /* Test branch where supports_format fails for PNG */
  ui_image_decoder_png.supports_format = mock_png_supports_fail;
  err = ui_image_decode_memory(UI_IMAGE_FORMAT_PNG, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN)
    return 1;
  ui_image_decoder_png.supports_format = orig_png_supports; /* Restore */

  /* Test branch where supports_format fails for JPEG */
  ui_image_decoder_jpeg.supports_format = mock_jpeg_supports_fail;
  err = ui_image_decode_memory(UI_IMAGE_FORMAT_JPEG, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN)
    return 1;
  ui_image_decoder_jpeg.supports_format = orig_jpeg_supports; /* Restore */

  /* Test branch where supports_format fails for WEBP */
  ui_image_decoder_webp.supports_format = mock_webp_supports_fail;
  err = ui_image_decode_memory(UI_IMAGE_FORMAT_WEBP, dummy_data, 5, &image);
  if (err != UI_ERROR_UNKNOWN)
    return 1;
  ui_image_decoder_webp.supports_format = orig_webp_supports; /* Restore */

  /* Test ui_image_free */
  if (ui_image_free(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_free(&image);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Direct testing of backends to ensure full branch coverage of their methods
   */
  if (ui_image_decoder_png.supports_format((enum ui_image_format) - 1,
                                           &supported) != UI_ERROR_NONE ||
      supported)
    return 1;
  if (ui_image_decoder_jpeg.supports_format((enum ui_image_format) - 1,
                                            &supported) != UI_ERROR_NONE ||
      supported)
    return 1;
  if (ui_image_decoder_webp.supports_format((enum ui_image_format) - 1,
                                            &supported) != UI_ERROR_NONE ||
      supported)
    return 1;

  if (ui_image_decoder_png.decode_memory(NULL, 5, &image) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_jpeg.decode_memory(NULL, 5, &image) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_webp.decode_memory(NULL, 5, &image) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_image_decoder_png.free_image(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_jpeg.free_image(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_webp.free_image(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_image_decoder_png.free_image(&image) != UI_ERROR_NONE)
    return 1;
  if (ui_image_decoder_jpeg.free_image(&image) != UI_ERROR_NONE)
    return 1;
  if (ui_image_decoder_webp.free_image(&image) != UI_ERROR_NONE)
    return 1;

  printf("All test_ui_image_decoder passed\n");
  return 0;
}
