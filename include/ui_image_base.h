#ifndef UI_IMAGE_BASE_H
#define UI_IMAGE_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;

/**
 * @brief Represents the loading state of a base image.
 */
enum ui_image_state {
  UI_IMAGE_STATE_IDLE,
  UI_IMAGE_STATE_LOADING,
  UI_IMAGE_STATE_LOADED,
  UI_IMAGE_STATE_ERROR
};

/**
 * @brief Base component state for an Image widget.
 */
struct ui_image_base {
  struct ui_component *component;
  enum ui_image_state state;
  int lazy_load;
  char *src_url;
  char *alt_text;
  struct ui_signal *src_signal;
};

/**
 * @brief Initializes a base image component.
 *
 * @param image Pointer to the image base struct.
 * @param component The UI component to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_image_base_init(struct ui_image_base *image,
                                 struct ui_component *component);

/**
 * @brief Sets the image source URL and optional lazy load behavior.
 *
 * @param image Pointer to the image base struct.
 * @param src_url URL or file path.
 * @param lazy_load If true, image will load lazily.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_image_base_set_src(struct ui_image_base *image,
                                    const char *src_url, int lazy_load);

/**
 * @brief Cleans up resources allocated by the image base.
 *
 * @param image Pointer to the image base struct.
 */
enum ui_error ui_image_base_cleanup(struct ui_image_base *image);

/**
 * @brief Binds the src property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_image_base_bind_src(struct ui_image_base *widget,
                                     struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_IMAGE_BASE_H */
