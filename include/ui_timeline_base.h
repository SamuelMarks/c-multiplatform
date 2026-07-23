#ifndef UI_TIMELINE_BASE_H
#define UI_TIMELINE_BASE_H

struct ui_computed;

struct ui_signal;

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Alignment modes for the timeline nodes.
 */
enum ui_timeline_alignment {
  UI_TIMELINE_ALIGN_LEFT,     /**< All nodes align to the left of the axis */
  UI_TIMELINE_ALIGN_RIGHT,    /**< All nodes align to the right of the axis */
  UI_TIMELINE_ALIGN_ALTERNATE /**< Nodes alternate left and right */
};

struct ui_timeline_base;
struct ui_timeline_node;

/**
 * @brief Creates a new unstyled timeline base component.
 *
 * @param out_timeline Pointer to receive the allocated timeline base.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_MEMORY on allocation
 * failure.
 */
enum ui_error ui_timeline_base_create(struct ui_timeline_base **out_timeline);

/**
 * @brief Destroys a timeline base component and frees all nodes.
 *
 * @param timeline The timeline to destroy.
 */
void ui_timeline_base_destroy(struct ui_timeline_base *timeline);

/**
 * @brief Sets the alignment of the timeline.
 *
 * @param timeline The timeline.
 * @param alignment The alignment to set.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_timeline_base_set_alignment(struct ui_timeline_base *timeline,
                               enum ui_timeline_alignment alignment);

/**
 * @brief Gets the alignment of the timeline.
 *
 * @param timeline The timeline.
 * @param out_alignment Pointer to receive the alignment.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_timeline_base_get_alignment(const struct ui_timeline_base *timeline,
                               enum ui_timeline_alignment *out_alignment);

/**
 * @brief Adds a new node to the timeline.
 *
 * @param timeline The timeline.
 * @param title The title of the node (copied internally).
 * @param description The description of the node (copied internally).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers,
 * UI_ERROR_OUT_OF_MEMORY on allocation failure.
 */
enum ui_error ui_timeline_base_add_node(struct ui_timeline_base *timeline,
                                        const char *title,
                                        const char *description);

/**
 * @brief Gets the number of nodes in the timeline.
 *
 * @param timeline The timeline.
 * @param out_count Pointer to receive the number of nodes.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error
ui_timeline_base_get_node_count(const struct ui_timeline_base *timeline,
                                size_t *out_count);

/**
 * @brief Retrieves a specific node's data from the timeline.
 *
 * @param timeline The timeline.
 * @param index The index of the node to retrieve.
 * @param out_title Pointer to receive the title string pointer (owned by
 * timeline).
 * @param out_description Pointer to receive the description string pointer
 * (owned by timeline).
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointers,
 * UI_ERROR_OUT_OF_BOUNDS if index is invalid.
 */
enum ui_error ui_timeline_base_get_node(const struct ui_timeline_base *timeline,
                                        size_t index, const char **out_title,
                                        const char **out_description);

/**
 * @brief A stub rendering routine for the timeline.
 *
 * Calculates abstract rendering positions or simply validates state.
 *
 * @param timeline The timeline.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT on null pointer.
 */
enum ui_error ui_timeline_base_render(struct ui_timeline_base *timeline);

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_timeline_base_bind_data(struct ui_timeline_base *widget,
                                         struct ui_computed *signal);

#ifdef __cplusplus
}
#endif

#endif /* UI_TIMELINE_BASE_H */
