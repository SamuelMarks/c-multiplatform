/**
 * @file ui_image_base.h
 * @brief Base logic and state management for Image components.
 *
 * This header defines the component structure for images, handling loading
 * states, sources, lazy loading flags, and signal bindings.
 */

#ifndef UI_IMAGE_BASE_H
#define UI_IMAGE_BASE_H

struct ui_computed;
struct ui_signal;
struct ui_component;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents the loading state of a base image.
 */
enum ui_image_state {
  UI_IMAGE_STATE_IDLE,    /**< Image component is idle/uninitialized. */
  UI_IMAGE_STATE_LOADING, /**< Image data is currently being fetched. */
  UI_IMAGE_STATE_LOADED,  /**< Image is fully loaded. */
  UI_IMAGE_STATE_ERROR    /**< Image failed to load. */
};

/**
 * @brief Base component state for an Image widget.
 */
struct ui_image_base {
  struct ui_component
      *component;            /**< Pointer to the underlying UI component. */
  enum ui_image_state state; /**< Current loading state. */
  int lazy_load;             /**< 1 if lazy loading is enabled, 0 otherwise. */
  char *src_url;             /**< URL or local path to the image source. */
  char *alt_text;            /**< Alternative text for accessibility. */
  struct ui_signal *src_signal; /**< Signal bound to the source URL. */
};

/**
 * @brief Initializes a base image component.
 *
 * @param image Pointer to the image base struct to initialize.
 * @param component Pointer to the UI component to bind to.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_base_init(struct ui_image_base *image,
                              struct ui_component *component);

/**
 * @brief Sets the image source URL and optional lazy load behavior.
 *
 * @param image Pointer to the image base struct.
 * @param src_url The URL or local file path string.
 * @param lazy_load 1 if the image should load lazily, 0 for immediate load.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_base_set_src(struct ui_image_base *image,
                                 const char *src_url, int lazy_load);

/**
 * @brief Cleans up resources allocated by the image base.
 *
 * @param image Pointer to the image base struct.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_base_cleanup(struct ui_image_base *image);

/**
 * @brief Binds a signal to the image source property.
 *
 * @param widget Pointer to the image base struct.
 * @param signal Pointer to the signal to bind to.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_image_base_bind_src(struct ui_image_base *widget,
                                  struct ui_signal *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_IMAGE_BASE_H */
