/* clang-format off */
#include <assert.h>
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

static enum ui_error test_image_init(void) {
  struct ui_image_base image;
  struct ui_component comp;
  enum ui_error err;

  err = ui_image_base_init(NULL, &comp);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_image_base_init(&image, NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_image_base_init(&image, &comp);
  assert(err == UI_ERROR_NONE);
  assert(image.component == &comp);
  assert(image.state == UI_IMAGE_STATE_IDLE);
  assert(image.lazy_load == 0);
  assert(image.src_url == NULL);
  assert(image.alt_text == NULL);
  return UI_ERROR_NONE;
}

static enum ui_error test_image_set_src(void) {
  struct ui_image_base image;
  struct ui_component comp;
  enum ui_error err;
  char *alt;

  ui_image_base_init(&image, &comp);

  err = ui_image_base_set_src(NULL, "https://example.com/img.png", 0);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_image_base_set_src(&image, NULL, 0);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  g_malloc_fail_countdown = 0;
  err = ui_image_base_set_src(&image, "https://example.com/img.png", 0);
  assert(err == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  err = ui_image_base_set_src(&image, "https://example.com/img.png", 0);
  assert(err == UI_ERROR_NONE);
  assert(strcmp(image.src_url, "https://example.com/img.png") == 0);
  assert(image.lazy_load == 0);
  assert(image.state == UI_IMAGE_STATE_LOADING);

  err = ui_image_base_set_src(&image, "https://example.com/lazy.png", 1);
  assert(err == UI_ERROR_NONE);
  assert(strcmp(image.src_url, "https://example.com/lazy.png") == 0);
  assert(image.lazy_load == 1);
  assert(image.state == UI_IMAGE_STATE_IDLE);

  alt = (char *)malloc(10);
  strcpy(alt, "alt");
  image.alt_text = alt;

  err = ui_image_base_cleanup(NULL);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  ui_image_base_cleanup(&image);
  assert(image.src_url == NULL);
  assert(image.alt_text == NULL);
  return UI_ERROR_NONE;
}

static enum ui_error test_image_bind_src(void) {
  struct ui_image_base image;
  struct ui_component comp;
  struct ui_signal *signal = (struct ui_signal *)1;
  enum ui_error err;

  ui_image_base_init(&image, &comp);

  err = ui_image_base_bind_src(NULL, signal);
  assert(err == UI_ERROR_INVALID_ARGUMENT);

  err = ui_image_base_bind_src(&image, signal);
  assert(err == UI_ERROR_NONE);
  assert(image.src_signal == signal);

  return UI_ERROR_NONE;
}

int main(void) {
  test_image_init();
  test_image_set_src();
  test_image_bind_src();
  return 0;
}
