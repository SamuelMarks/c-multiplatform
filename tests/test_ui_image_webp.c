/* clang-format off */
#include "../include/ui_image_decoder.h"
#include "../include/ui_error.h"
#include <stdio.h>
/* clang-format on */

extern struct ui_image_decoder_backend ui_image_decoder_webp;

int main(void) {
  struct ui_image img;
  unsigned char dummy_data[4] = {0, 0, 0, 0};
  enum ui_error rc;

  printf("Running ui_image_webp tests...\n");

  /* Test supports_format */
  {
    int supported = 0;
    if (ui_image_decoder_webp.supports_format(UI_IMAGE_FORMAT_WEBP,
                                              &supported) != UI_ERROR_NONE ||
        !supported)
      return 1;
    if (ui_image_decoder_webp.supports_format(UI_IMAGE_FORMAT_PNG,
                                              &supported) != UI_ERROR_NONE ||
        supported)
      return 1;
  }

  /* Test decode_memory */
  if (ui_image_decoder_webp.decode_memory(NULL, 10, &img) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_webp.decode_memory(dummy_data, 0, &img) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_webp.decode_memory(dummy_data, 4, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_image_decoder_webp.decode_memory(dummy_data, 4, &img);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Expected UI_ERROR_UNKNOWN from webp decoder stub\n");
    return 1;
  }

  /* Validate WEBP animated decoding loop mocked check */
  /* Real implementation would read frame counts and loop settings */

  if (img.width != 0 || img.height != 0 || img.channels != 4 ||
      img.data_size != 0)
    return 1;

  /* Test free_image */
  if (ui_image_decoder_webp.free_image(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_image_decoder_webp.free_image(&img);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("ui_image_webp tests passed.\n");
  return 0;
}
