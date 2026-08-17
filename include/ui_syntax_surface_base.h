#ifndef UI_SYNTAX_SURFACE_BASE_H
#define UI_SYNTAX_SURFACE_BASE_H

/* clang-format off */
#include "ui_types.h"
#include "ui_error.h"
#include "ui_signal.h"
#include "ui_component.h"
#include "ui_rich_text_base.h"
#include "ui_geometry.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct ui_syntax_surface_base
 * @brief Opaque handle for the syntax surface component.
 */
struct ui_syntax_surface_base;

/**
 * @struct ui_syntax_fold_region
 * @brief Configuration for a single folding region.
 */
struct ui_syntax_fold_region {
  /** @brief The starting line of the fold region (0-indexed). */
  int start_line;
  /** @brief The ending line of the fold region (0-indexed). */
  int end_line;
  /** @brief True if the region is currently collapsed. */
  ui_bool_t is_collapsed;
};

/**
 * @struct ui_syntax_bracket_match
 * @brief Represents a bracket match result.
 */
struct ui_syntax_bracket_match {
  /** @brief True if a match was found. */
  ui_bool_t has_match;
  /** @brief The line of the opening bracket. */
  int open_line;
  /** @brief The column of the opening bracket. */
  int open_col;
  /** @brief The line of the closing bracket. */
  int close_line;
  /** @brief The column of the closing bracket. */
  int close_col;
};

/**
 * @brief Creates a syntax surface base component.
 *
 * @param arena The memory arena to use for allocation.
 * @param base_rich_text The underlying rich text editor this surface extends.
 * @param out_surface Pointer to receive the created component handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_syntax_surface_base_create(struct ui_arena *arena,
                              struct ui_rich_text_base *base_rich_text,
                              struct ui_syntax_surface_base **out_surface);

/**
 * @brief Destroys a syntax surface component.
 *
 * @param surface The component to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_syntax_surface_base_destroy(struct ui_syntax_surface_base *surface);

/**
 * @brief Calculates the required width of the gutter based on the total line
 * count and specified character width (e.g. for monospace font).
 *
 * @param surface The component.
 * @param total_lines The total number of lines in the document.
 * @param char_width The physical width of a single digit character.
 * @param out_width Pointer to receive the calculated gutter width.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_syntax_surface_base_calculate_gutter_width(
    const struct ui_syntax_surface_base *surface, int total_lines,
    float char_width, float *out_width);

/**
 * @brief Toggles a folding region (e.g. collapsing a function block).
 *
 * @param surface The component.
 * @param start_line The starting line of the folding region.
 * @param end_line The ending line of the folding region.
 * @param collapse True to collapse, false to expand.
 * @return UI_ERROR_NONE on success, UI_ERROR_OUT_OF_BOUNDS if invalid range.
 */
ui_error_t
ui_syntax_surface_base_set_fold_region(struct ui_syntax_surface_base *surface,
                                       int start_line, int end_line,
                                       ui_bool_t collapse);

/**
 * @brief Retrieves the signal emitted when folding topology changes.
 * Payload is NULL.
 *
 * @param surface The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_syntax_surface_base_get_fold_changed_signal(
    struct ui_syntax_surface_base *surface, ui_signal_t **out_signal);

/**
 * @brief Translates an absolute line number into a visual line index,
 * accounting for active fold regions that hide lines.
 *
 * @param surface The component.
 * @param absolute_line The physical line number (0-indexed).
 * @param out_visual_index Pointer to receive the visual index, or -1 if the
 * line is hidden inside a fold.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_syntax_surface_base_get_visual_line_index(
    const struct ui_syntax_surface_base *surface, int absolute_line,
    int *out_visual_index);

/**
 * @brief Sets the active line to be highlighted.
 *
 * @param surface The component.
 * @param absolute_line The physical line number (0-indexed).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_syntax_surface_base_set_active_line(struct ui_syntax_surface_base *surface,
                                       int absolute_line);

/**
 * @brief Retrieves the signal emitted when the active line changes.
 * Payload is an int representing the absolute_line.
 *
 * @param surface The component.
 * @param out_signal Pointer to receive the signal handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_syntax_surface_base_get_active_line_signal(
    struct ui_syntax_surface_base *surface, ui_signal_t **out_signal);

/**
 * @brief Sets the currently matched brackets to be highlighted.
 *
 * @param surface The component.
 * @param match The bracket match coordinates.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_syntax_surface_base_set_bracket_match(
    struct ui_syntax_surface_base *surface,
    const struct ui_syntax_bracket_match *match);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SYNTAX_SURFACE_BASE_H */
