/**
 * @file ui_image_base.c
 * @brief ui_image_base.c implementation.
 */
/* clang-format off */
#include "ui_image_base.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include "ui_dom_node.h"
#include <string.h>
/* clang-format on */

/**
 * @brief ui_image_base_init.
 * @param image Parameter image.
 * @param component Parameter component.
 * @return Return value.
 */
ui_error_t ui_image_base_init(struct ui_image_base *image,
                              struct ui_component *component) {
  if (!image || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  image->component = component;
  image->state = UI_IMAGE_STATE_IDLE;
  image->lazy_load = 0;
  image->src_url = NULL;
  image->alt_text = NULL;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_image_base_set_src.
 * @param image Parameter image.
 * @param src_url Parameter src_url.
 * @param lazy_load Parameter lazy_load.
 * @return Return value.
 */
ui_error_t ui_image_base_set_src(struct ui_image_base *image,
                                 const char *src_url, int lazy_load) {
  size_t len;

  if (!image || !src_url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (image->src_url) {
    C_MULTIPLATFORM_FREE(image->src_url);
    image->src_url = NULL;
  }

  len = strlen(src_url);
  image->src_url = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!image->src_url) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
#if defined(_MSC_VER)
  strcpy_s(image->src_url, len + 1, src_url);
#else
  UI_STRCPY(image->src_url, sizeof(image->src_url), src_url);
#endif

  image->lazy_load = lazy_load;
  image->state = lazy_load ? UI_IMAGE_STATE_IDLE : UI_IMAGE_STATE_LOADING;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_image_base_cleanup.
 * @param image Parameter image.
 * @return Return value.
 */
ui_error_t ui_image_base_cleanup(struct ui_image_base *image) {
  if (!image) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (image->src_url) {
    C_MULTIPLATFORM_FREE(image->src_url);
    image->src_url = NULL;
  }

  if (image->alt_text) {
    C_MULTIPLATFORM_FREE(image->alt_text);
    image->alt_text = NULL;
  }

  image->state = UI_IMAGE_STATE_IDLE;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_image_base_bind_src.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_image_base_bind_src(struct ui_image_base *widget,
                                  struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->src_signal = signal;
  return UI_ERROR_NONE;
}
