#ifndef UI_CAROUSEL_BASE_H
#define UI_CAROUSEL_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_component.h"
#include "ui_virtual_scroll_base.h"
#include "ui_gesture.h"
#include <stddef.h>
/* clang-format on */

struct ui_carousel_base;

/**
 * @brief Orientation of the carousel.
 */
enum ui_carousel_orientation {
  UI_CAROUSEL_ORIENTATION_HORIZONTAL = 0,
  UI_CAROUSEL_ORIENTATION_VERTICAL = 1
};

/**
 * @brief Configuration for the carousel.
 */
struct ui_carousel_config {
  enum ui_carousel_orientation orientation;
  ui_virtual_scroll_create_node_fn create_node;
  ui_virtual_scroll_update_node_fn update_node;
  void *user_data;
  size_t initial_item_count;
  float item_size; /* width if horiz, height if vert */
};

/**
 * @brief Creates a new carousel component.
 *
 * @param out_carousel Pointer to receive the allocated carousel instance.
 * @param config The configuration for the carousel.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY if allocation fails,
 * or UI_ERROR_INVALID_ARGUMENT if args are null.
 */
ui_error_t ui_carousel_base_create(struct ui_carousel_base **out_carousel,
                                   const struct ui_carousel_config *config);

/**
 * @brief Destroys the carousel.
 *
 * @param carousel The carousel to destroy.
 */
ui_error_t ui_carousel_base_destroy(struct ui_carousel_base *carousel);

/**
 * @brief Sets the number of items in the carousel.
 *
 * @param carousel The carousel instance.
 * @param count The number of items.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_carousel_base_set_item_count(struct ui_carousel_base *carousel,
                                           size_t count);

/**
 * @brief Sets the viewport size.
 *
 * @param carousel The carousel instance.
 * @param width Viewport width.
 * @param height Viewport height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_carousel_base_set_viewport_size(struct ui_carousel_base *carousel,
                                              float width, float height);

/**
 * @brief Gets the underlying component.
 *
 * @param carousel The carousel instance.
 * @return The component, or NULL if null.
 */
ui_error_t ui_carousel_base_get_component(struct ui_carousel_base *carousel,
                                          struct ui_component **out_component);

/**
 * @brief Feeds a raw event into the carousel for gesture paging.
 *
 * @param carousel The carousel instance.
 * @param event The UI event.
 * @param timestamp_ms Current timestamp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_carousel_base_process_event(struct ui_carousel_base *carousel,
                                          const struct ui_event *event,
                                          double timestamp_ms);

/**
 * @brief Updates the carousel per frame (for scroll animations/snapping).
 *
 * @param carousel The carousel instance.
 * @param timestamp_ms Current timestamp.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_carousel_base_tick(struct ui_carousel_base *carousel,
                                 double timestamp_ms);

/**
 * @brief Scrolls to a specific index.
 *
 * @param carousel The carousel.
 * @param index The item index.
 * @param smooth If true, animates to the index, else jumps instantly.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if index is out of
 * range, or UI_ERROR_INVALID_ARGUMENT if null.
 */
ui_error_t ui_carousel_base_scroll_to_index(struct ui_carousel_base *carousel,
                                            size_t index, int smooth);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_carousel_base_bind_data(struct ui_carousel_base *widget,
                                      struct ui_computed *signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CAROUSEL_BASE_H */
