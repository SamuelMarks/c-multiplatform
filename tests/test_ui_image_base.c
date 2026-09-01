/* clang-format off */
#include "ui_image_base.h"
#include "../src/ui_internal_mem.h"
#include "ui_component.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t run_normal_tests(void) {
  struct ui_image_base image;
  struct ui_component comp;
  ui_error_t rc;

  printf("Testing ui_image_base_init...\n");
  rc = ui_image_base_init(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_image_base_init(&image, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_image_base_init(&image, &comp);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (image.component != &comp)
    return UI_ERROR_UNKNOWN;
  if (image.state != UI_IMAGE_STATE_IDLE)
    return UI_ERROR_UNKNOWN;
  if (image.lazy_load != 0)
    return UI_ERROR_UNKNOWN;
  if (image.src_url != NULL)
    return UI_ERROR_UNKNOWN;
  if (image.alt_text != NULL)
    return UI_ERROR_UNKNOWN;

  printf("Testing ui_image_base_set_src...\n");
  rc = ui_image_base_set_src(NULL, "http://example.com/image.png", 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_image_base_set_src(&image, NULL, 0);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  /* Normal set src */
  rc = ui_image_base_set_src(&image, "http://example.com/image.png", 0);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (strcmp(image.src_url, "http://example.com/image.png") != 0)
    return UI_ERROR_UNKNOWN;
  if (image.lazy_load != 0)
    return UI_ERROR_UNKNOWN;
  if (image.state != UI_IMAGE_STATE_LOADING)
    return UI_ERROR_UNKNOWN;

  /* Overwrite existing src and use lazy_load */
  rc = ui_image_base_set_src(&image, "http://example.com/image2.png", 1);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (strcmp(image.src_url, "http://example.com/image2.png") != 0)
    return UI_ERROR_UNKNOWN;
  if (image.lazy_load != 1)
    return UI_ERROR_UNKNOWN;
  if (image.state != UI_IMAGE_STATE_IDLE)
    return UI_ERROR_UNKNOWN;

  printf("Testing ui_image_base_bind_src...\n");
  rc = ui_image_base_bind_src(NULL, (struct ui_signal *)1);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_image_base_bind_src(&image, (struct ui_signal *)1);
  if (rc != UI_ERROR_NONE)
    return rc;

  printf("Testing ui_image_base_cleanup...\n");
  rc = ui_image_base_cleanup(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  /* Populate alt_text manually to cover that branch */
  image.alt_text = (char *)malloc(10);
  if (image.alt_text)
    UI_STRCPY(image.alt_text, sizeof(image.alt_text), "alt");

  rc = ui_image_base_cleanup(&image);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (image.src_url != NULL)
    return UI_ERROR_UNKNOWN;
  if (image.alt_text != NULL)
    return UI_ERROR_UNKNOWN;
  if (image.state != UI_IMAGE_STATE_IDLE)
    return UI_ERROR_UNKNOWN;

  /* Double cleanup to cover the false branches of src_url and alt_text */
  rc = ui_image_base_cleanup(&image);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_image_base image;
  struct ui_component comp;
  ui_error_t rc;

  {
    ui_error_t rc_cleanup = ui_image_base_init(&image, &comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  g_malloc_fail_countdown = 0;
  rc = ui_image_base_set_src(&image, "http://example.com/oom.png", 0);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return UI_ERROR_UNKNOWN;
  }
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_image_base_cleanup(&image);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return UI_ERROR_NONE;
}

int main(void) {
  if (run_normal_tests() != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }
  if (run_oom_tests() != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
