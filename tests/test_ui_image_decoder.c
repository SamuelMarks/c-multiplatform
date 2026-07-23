/* clang-format off */
#include "ui_image_decoder.h"
#include <stdio.h>
/* clang-format on */

extern struct ui_image_decoder_backend ui_image_decoder_png;
extern struct ui_image_decoder_backend ui_image_decoder_jpeg;
extern struct ui_image_decoder_backend ui_image_decoder_webp;

int main(void) {
  struct ui_image image;
  enum ui_error err;
  const char *dummy_data = "dummy";

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
  if (ui_image_decode_memory(-1, dummy_data, 5, &image) != UI_ERROR_UNKNOWN)
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

  /* Test ui_image_free */
  if (ui_image_free(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_free(&image);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Direct testing of backends to ensure full branch coverage of their methods
   */
  {
    int supported = 0;
    if (ui_image_decoder_png.supports_format(-1, &supported) != UI_ERROR_NONE ||
        supported)
      return 1;
    if (ui_image_decoder_jpeg.supports_format(-1, &supported) !=
            UI_ERROR_NONE ||
        supported)
      return 1;
    if (ui_image_decoder_webp.supports_format(-1, &supported) !=
            UI_ERROR_NONE ||
        supported)
      return 1;
  }

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
