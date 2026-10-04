/* clang-format off */
#include "ui_image_decoder.h"
#include "ui_error.h"
#include <stdio.h>
#include "min_jpg.h"
#include "stb_image.h"
/* clang-format on */

extern struct ui_image_decoder_backend ui_image_decoder_jpeg;

int main(void) {
  struct ui_image img;
  unsigned char dummy_data[4] = {0, 0, 0, 0};
  ui_error_t rc;

  printf("Running ui_image_jpeg tests...\n");

  /* Test supports_format */
  {
    int supported = 0;
    if (ui_image_decoder_jpeg.supports_format(UI_IMAGE_FORMAT_JPEG,
                                              &supported) != UI_ERROR_NONE ||
        !supported)
      return 1;
    if (ui_image_decoder_jpeg.supports_format(UI_IMAGE_FORMAT_PNG,
                                              &supported) != UI_ERROR_NONE ||
        supported)
      return 1;
  }

  /* Test decode_memory */
  if (ui_image_decoder_jpeg.decode_memory(NULL, 10, &img) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_jpeg.decode_memory(dummy_data, 0, &img) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_image_decoder_jpeg.decode_memory(dummy_data, 4, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_image_decoder_jpeg.decode_memory(dummy_data, 4, &img);
  if (rc != UI_ERROR_UNKNOWN) {
    printf("Expected UI_ERROR_UNKNOWN from invalid data decode\n");
    return 1;
  }

  rc = ui_image_decoder_jpeg.decode_memory(tests_min_jpg, tests_min_jpg_len,
                                           &img);
  if (rc != UI_ERROR_NONE) {
    printf("Expected UI_ERROR_NONE from valid JPEG. stbi failure reason: %s\n",
           stbi_failure_reason());
    return 1;
  }

  if (img.width != 1 || img.height != 1 || img.channels != 4)
    return 1;

  /* Test free_image */
  if (ui_image_decoder_jpeg.free_image(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_image_decoder_jpeg.free_image(&img);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("ui_image_jpeg tests passed.\n");
  return 0;
}
