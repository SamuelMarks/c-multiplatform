/**
 * @file cupertino_search_bar.h
 * @brief Cupertino Search Bar & Controller component conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SEARCH_BAR_H
#define CUPERTINO_CUPERTINO_SEARCH_BAR_H

/* clang-format off */
#include "ui_search_bar_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SEARCH_BAR_MAX_SCOPES 8
#define CUPERTINO_SEARCH_BAR_MAX_TOKENS 4
#define CUPERTINO_SEARCH_BAR_HEIGHT 36.0f
#define CUPERTINO_SEARCH_BAR_CORNER_RADIUS 10.0f

/**
 * @struct cupertino_search_bar_descriptor
 * @brief Configuration descriptor for creating a Cupertino search bar.
 */
struct cupertino_search_bar_descriptor {
  const char *placeholder; /**< Placeholder text hint. */
  int show_cancel_button;  /**< 1 if Cancel button is enabled. */
  const char *const
      *scope_titles;          /**< Optional array of scope bar tab titles. */
  size_t scope_count;         /**< Number of scope bar tabs. */
  size_t initial_scope_index; /**< Initially selected scope index. */
};

/**
 * @struct cupertino_search_bar
 * @brief Cupertino Search Bar instance wrapping ui_search_bar_base.
 */
struct cupertino_search_bar {
  struct ui_search_bar_base base; /**< CDK search bar primitive state. */
  struct ui_component *component; /**< Underlying dummy component. */
  char placeholder[128];          /**< Placeholder text string. */
  char query_buf[256];            /**< Internal query buffer. */
  int is_focused;                 /**< 1 if search field currently focused. */
  float cancel_button_progress; /**< Slide progress of Cancel button [0.0, 1.0].
                                 */
  size_t scope_count;           /**< Number of category scope filters. */
  size_t selected_scope_index;  /**< Active scope category index. */
  char scopes[CUPERTINO_SEARCH_BAR_MAX_SCOPES][64]; /**< Scope title strings. */
  size_t token_count; /**< Search token capsules count. */
  char tokens[CUPERTINO_SEARCH_BAR_MAX_TOKENS]
             [64];                      /**< Search token strings. */
  struct ui_control_value_accessor cva; /**< CVA form interface. */
};

/**
 * @brief Creates a new Cupertino search bar instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_search_bar Pointer to receive newly created search bar.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_create(struct ui_engine *engine,
                            const struct cupertino_search_bar_descriptor *desc,
                            struct cupertino_search_bar **out_search_bar);

/**
 * @brief Destroys a Cupertino search bar instance.
 *
 * @param search_bar Search bar instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_destroy(struct cupertino_search_bar *search_bar);

/**
 * @brief Sets the search query text.
 *
 * @param search_bar Target search bar.
 * @param query Search query string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_search_bar_set_query(
    struct cupertino_search_bar *search_bar, const char *query);

/**
 * @brief Retrieves the current search query text.
 *
 * @param search_bar Target search bar.
 * @param out_query Pointer to receive const pointer to query string.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_search_bar_get_query(
    const struct cupertino_search_bar *search_bar, const char **out_query);

/**
 * @brief Clears the search query and dismisses the clear button.
 *
 * @param search_bar Target search bar.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_clear(struct cupertino_search_bar *search_bar);

/**
 * @brief Sets input focus state, triggering Cancel button slide animation.
 *
 * @param search_bar Target search bar.
 * @param focused 1 if focused (Cancel slides in), 0 if blurred.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_search_bar_set_focused(
    struct cupertino_search_bar *search_bar, int focused);

/**
 * @brief Queries current input focus state.
 *
 * @param search_bar Target search bar.
 * @param out_focused Pointer to receive focus state.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_search_bar_is_focused(
    const struct cupertino_search_bar *search_bar, int *out_focused);

/**
 * @brief Retrieves the Cancel button animated slide progress.
 *
 * 0.0 indicates completely hidden off trailing edge; 1.0 indicates fully
 * visible.
 *
 * @param search_bar Target search bar.
 * @param out_progress Pointer to receive slide progress in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_get_cancel_progress(
    const struct cupertino_search_bar *search_bar, float *out_progress);

/**
 * @brief Selects active category in the optional Scope Bar.
 *
 * @param search_bar Target search bar.
 * @param index Scope index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_set_scope_selected(struct cupertino_search_bar *search_bar,
                                        size_t index);

/**
 * @brief Retrieves active category in the Scope Bar.
 *
 * @param search_bar Target search bar.
 * @param out_index Pointer to receive active scope index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_get_scope_selected(
    const struct cupertino_search_bar *search_bar, size_t *out_index);

/**
 * @brief Appends a search token capsule inside the query bar.
 *
 * @param search_bar Target search bar.
 * @param token Token text label.
 * @param out_index Pointer to receive added token index.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT, or
 * UI_ERROR_OUT_OF_MEMORY.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_add_token(struct cupertino_search_bar *search_bar,
                               const char *token, size_t *out_index);

/**
 * @brief Retrieves total count of search tokens.
 *
 * @param search_bar Target search bar.
 * @param out_count Pointer to receive token count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_search_bar_get_token_count(
    const struct cupertino_search_bar *search_bar, size_t *out_count);

/**
 * @brief Retrieves the CVA form interface for two-way binding.
 *
 * @param search_bar Target search bar.
 * @param out_cva Pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_get_cva(struct cupertino_search_bar *search_bar,
                             struct ui_control_value_accessor **out_cva);

/**
 * @brief Retrieves underlying CDK search bar base primitive.
 *
 * @param search_bar Target search bar.
 * @param out_base Pointer to receive ui_search_bar_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_search_bar_get_base(struct cupertino_search_bar *search_bar,
                              struct ui_search_bar_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SEARCH_BAR_H */
