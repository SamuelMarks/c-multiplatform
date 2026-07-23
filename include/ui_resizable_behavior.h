/* clang-format off */
#ifndef UI_RESIZABLE_BEHAVIOR_H
#define UI_RESIZABLE_BEHAVIOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_error.h"
#include "ui_event.h"

/* clang-format on */

struct ui_resizable_behavior;

/**
 * @brief Flags for resizable edges.
 */
enum ui_resizable_edge {
  UI_RESIZABLE_EDGE_NONE = 0,
  UI_RESIZABLE_EDGE_LEFT = 1 << 0,
  UI_RESIZABLE_EDGE_RIGHT = 1 << 1,
  UI_RESIZABLE_EDGE_TOP = 1 << 2,
  UI_RESIZABLE_EDGE_BOTTOM = 1 << 3,
  UI_RESIZABLE_EDGE_ALL = 0xF
};

/**
 * @brief Callback invoked when a resize operation requests a size change.
 */
typedef enum ui_error (*ui_resizable_on_resize_t)(int new_width, int new_height,
                                                  void *user_data);

/**
 * @brief Creates a new resizable behavior.
 *
 * @param out_behavior Pointer to receive the allocated behavior.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_resizable_behavior_create(struct ui_resizable_behavior **out_behavior);

/**
 * @brief Destroys a resizable behavior.
 *
 * @param behavior The behavior to destroy.
 */
void ui_resizable_behavior_destroy(struct ui_resizable_behavior *behavior);

/**
 * @brief Configures the resizable constraints and edges.
 *
 * @param behavior The resizable behavior.
 * @param edges Bitmask of edges that can be dragged.
 * @param min_width Minimum width (or -1 for none).
 * @param min_height Minimum height (or -1 for none).
 * @param max_width Maximum width (or -1 for none).
 * @param max_height Maximum height (or -1 for none).
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_resizable_behavior_configure(struct ui_resizable_behavior *behavior,
                                unsigned int edges, int min_width,
                                int min_height, int max_width, int max_height);

/**
 * @brief Sets the callback to be invoked when a resize is requested.
 *
 * @param behavior The resizable behavior.
 * @param on_resize The callback function.
 * @param user_data Opaque user data for the callback.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_resizable_behavior_set_on_resize(struct ui_resizable_behavior *behavior,
                                    ui_resizable_on_resize_t on_resize,
                                    void *user_data);

/**
 * @brief Processes input events to handle drag-to-resize logic.
 *
 * @param behavior The resizable behavior.
 * @param event The input event.
 * @param current_width The current width of the container.
 * @param current_height The current height of the container.
 * @param hit_test_thickness Thickness of the draggable edge area in pixels.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_resizable_behavior_process_event(
    struct ui_resizable_behavior *behavior, const struct ui_event *event,
    int current_width, int current_height, int hit_test_thickness);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_RESIZABLE_BEHAVIOR_H */
