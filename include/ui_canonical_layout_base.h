#ifndef UI_CANONICAL_LAYOUT_BASE_H
#define UI_CANONICAL_LAYOUT_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Window size classes for macro-adaptive layouts.
 */
enum ui_window_size_class {
  UI_WINDOW_SIZE_CLASS_COMPACT = 0,
  UI_WINDOW_SIZE_CLASS_MEDIUM = 1,
  UI_WINDOW_SIZE_CLASS_EXPANDED = 2
};

/**
 * @brief Opaque handle for the canonical layout component.
 */
struct ui_canonical_layout_base;

/**
 * @brief Configuration for creating a canonical layout.
 */
struct ui_canonical_layout_config {
  enum ui_window_size_class initial_size_class;
  ui_bool_t has_leading_pane;
  ui_bool_t has_trailing_pane;
  ui_bool_t has_bottom_bar;
};

/**
 * @brief Creates a canonical layout component.
 *
 * @param arena The memory arena to use for allocation.
 * @param config The layout configuration.
 * @param out_layout Pointer to receive the created layout handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_canonical_layout_base_create(struct ui_arena *arena,
                                const struct ui_canonical_layout_config *config,
                                struct ui_canonical_layout_base **out_layout);

/**
 * @brief Destroys a canonical layout component.
 *
 * @param layout The layout to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_canonical_layout_base_destroy(struct ui_canonical_layout_base *layout);

/**
 * @brief Updates the current window size class, potentially
 * collapsing/expanding panes.
 *
 * @param layout The layout component.
 * @param size_class The new size class.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_canonical_layout_base_set_size_class(struct ui_canonical_layout_base *layout,
                                        enum ui_window_size_class size_class);

/**
 * @brief Retrieves the current window size class.
 *
 * @param layout The layout component.
 * @param out_size_class Pointer to receive the size class.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_canonical_layout_base_get_size_class(
    const struct ui_canonical_layout_base *layout,
    enum ui_window_size_class *out_size_class);

/**
 * @brief Sets the content component for the body region.
 *
 * @param layout The layout component.
 * @param body The component to place in the body region.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_canonical_layout_base_set_body(struct ui_canonical_layout_base *layout,
                                  struct ui_component *body);

/**
 * @brief Sets the content component for the leading pane region.
 *
 * @param layout The layout component.
 * @param leading_pane The component to place in the leading pane.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_canonical_layout_base_set_leading_pane(
    struct ui_canonical_layout_base *layout, struct ui_component *leading_pane);

/**
 * @brief Sets the content component for the trailing pane region.
 *
 * @param layout The layout component.
 * @param trailing_pane The component to place in the trailing pane.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_canonical_layout_base_set_trailing_pane(
    struct ui_canonical_layout_base *layout,
    struct ui_component *trailing_pane);

/**
 * @brief Sets the content component for the bottom bar region.
 *
 * @param layout The layout component.
 * @param bottom_bar The component to place in the bottom bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_canonical_layout_base_set_bottom_bar(struct ui_canonical_layout_base *layout,
                                        struct ui_component *bottom_bar);

/**
 * @brief Gets the underlying signal triggered when the size class or layout
 * topology changes. The signal payload is the int value of the new
 * ui_window_size_class.
 *
 * @param layout The layout component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_canonical_layout_base_get_layout_changed_signal(
    struct ui_canonical_layout_base *layout, ui_signal_t **out_signal);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CANONICAL_LAYOUT_BASE_H */
