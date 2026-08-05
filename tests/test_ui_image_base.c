/* clang-format off */
#include <string.h>
#include <stdlib.h>

#include "ui_image_base.h"
#include "ui_error.h"
#include "ui_signal.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

/* Dummy component struct for testing */
struct ui_component {
  int id;
};

static int test_image_init(void) {
  struct ui_image_base image;
  struct ui_component comp;
  ui_error_t err;

  err = ui_image_base_init(NULL, &comp);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_base_init(&image, NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_base_init(&image, &comp);
  if (err != UI_ERROR_NONE)
    return 1;
  if (image.component != &comp)
    return 1;
  if (image.state != UI_IMAGE_STATE_IDLE)
    return 1;
  if (image.lazy_load != 0)
    return 1;
  if (image.src_url != NULL)
    return 1;
  if (image.alt_text != NULL)
    return 1;
  return 0;
}

static int test_image_set_src(void) {
  struct ui_image_base image;
  struct ui_component comp;
  ui_error_t err;
  char *alt;

  if (ui_image_base_init(&image, &comp) != UI_ERROR_NONE)
    return 1;

  err = ui_image_base_set_src(NULL, "https://example.com/img.png", 0);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_base_set_src(&image, NULL, 0);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  err = ui_image_base_set_src(&image, "https://example.com/img.png", 0);
  if (err != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  err = ui_image_base_set_src(&image, "https://example.com/img.png", 0);
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(image.src_url, "https://example.com/img.png") != 0)
    return 1;
  if (image.lazy_load != 0)
    return 1;
  if (image.state != UI_IMAGE_STATE_LOADING)
    return 1;

  err = ui_image_base_set_src(&image, "https://example.com/lazy.png", 1);
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(image.src_url, "https://example.com/lazy.png") != 0)
    return 1;
  if (image.lazy_load != 1)
    return 1;
  if (image.state != UI_IMAGE_STATE_IDLE)
    return 1;

  alt = (char *)malloc(10);
  if (alt) {
    strcpy(alt, "alt");
    image.alt_text = alt;
  }

  err = ui_image_base_cleanup(NULL);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_image_base_cleanup(&image) != UI_ERROR_NONE)
    return 1;
  if (image.src_url != NULL)
    return 1;
  if (image.alt_text != NULL)
    return 1;
  return 0;
}

static int test_image_bind_src(void) {
  struct ui_image_base image;
  struct ui_component comp;
  struct ui_signal *signal = (struct ui_signal *)1;
  ui_error_t err;

  if (ui_image_base_init(&image, &comp) != UI_ERROR_NONE)
    return 1;

  err = ui_image_base_bind_src(NULL, signal);
  if (err != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  err = ui_image_base_bind_src(&image, signal);
  if (err != UI_ERROR_NONE)
    return 1;
  if (image.src_signal != signal)
    return 1;

  return 0;
}

int main(void) {
  if (test_image_init())
    return 1;
  if (test_image_set_src())
    return 1;
  if (test_image_bind_src())
    return 1;
  return 0;
}
