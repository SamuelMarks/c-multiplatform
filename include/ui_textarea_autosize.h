#ifndef UI_TEXTAREA_AUTOSIZE_H
#define UI_TEXTAREA_AUTOSIZE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Configuration for textarea autosize calculations.
 */
struct ui_textarea_autosize_config {
  float min_height;     /**< Minimum height allowed */
  float max_height;     /**< Maximum height allowed (or 0.0f for unbounded) */
  float line_height;    /**< Height of a single line of text */
  float padding_top;    /**< Top padding */
  float padding_bottom; /**< Bottom padding */
};

/**
 * @brief State representing the current calculated layout of the textarea.
 */
struct ui_textarea_autosize_state {
  float content_height; /**< The raw calculated height of the text content */
  float
      target_height; /**< The final clamped height for the textarea component */
  int has_scrollbar; /**< Non-zero if the content exceeds max_height,
                        necessitating a scrollbar */
};

/**
 * @brief Initializes a textarea autosize configuration with default values.
 *
 * @param out_config Pointer to the config struct to initialize.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t ui_textarea_autosize_config_init(
    struct ui_textarea_autosize_config *out_config);

/**
 * @brief Calculates the target height for a textarea given its content length
 * (in lines).
 *
 * @param config The configuration to use for the calculation.
 * @param num_lines The number of lines currently in the textarea.
 * @param out_state Pointer to the state struct to populate with layout
 * dimensions.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t
ui_textarea_autosize_calculate(const struct ui_textarea_autosize_config *config,
                               size_t num_lines,
                               struct ui_textarea_autosize_state *out_state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_TEXTAREA_AUTOSIZE_H */
