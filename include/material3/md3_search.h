/**
 * @file md3_search.h
 * @brief Material 3 Search Bar and Search View components wrapping
 * ui_search_bar_base.
 */

#ifndef MATERIAL3_MD3_SEARCH_H
#define MATERIAL3_MD3_SEARCH_H

/* clang-format off */
#include "ui_component.h"
#include "ui_search_bar_base.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include "ui_types.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

/**
 * @brief Maximum number of history entries retained in search view.
 */
#define MD3_SEARCH_MAX_HISTORY 16
/**
 * @brief Maximum character length of each search history entry.
 */
#define MD3_SEARCH_HISTORY_LEN 128

/**
 * @struct md3_search_view
 * @brief Full-screen overlay search view showing search suggestions and
 * history.
 */
struct md3_search_view {
  int is_open;
  size_t history_count;
  char history[MD3_SEARCH_MAX_HISTORY][MD3_SEARCH_HISTORY_LEN];
};

/**
 * @struct md3_search_bar
 * @brief Material 3 Search Bar component wrapping ui_search_bar_base.
 */
struct md3_search_bar {
  struct ui_component component;
  struct ui_search_bar_base base;
  struct ui_control_value_accessor cva;
  struct md3_search_view view;
  char placeholder[64];
  int elevation;
};

/**
 * @brief Creates a Material 3 Search Bar component.
 *
 * @param engine Pointer to ui_engine instance.
 * @param placeholder Default hint text.
 * @param out_search_bar Pointer to receive newly created search bar.
 * @param out_cva Optional pointer to receive Control Value Accessor.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_search_bar_create(struct ui_engine *engine, const char *placeholder,
                      struct md3_search_bar **out_search_bar,
                      struct ui_control_value_accessor **out_cva);

/**
 * @brief Destroys a Material 3 Search Bar.
 *
 * @param search_bar The search bar to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_search_bar_destroy(struct md3_search_bar *search_bar);

/**
 * @brief Sets search query text.
 *
 * @param search_bar The search bar.
 * @param query The text to set.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_search_bar_set_query(struct md3_search_bar *search_bar, const char *query);

/**
 * @brief Gets current search query text.
 *
 * @param search_bar The search bar.
 * @param out_query Pointer to receive query string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_bar_get_query(
    const struct md3_search_bar *search_bar, const char **out_query);

/**
 * @brief Sets search loading indicator state.
 *
 * @param search_bar The search bar.
 * @param is_loading 1 for loading, 0 otherwise.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_search_bar_set_loading(struct md3_search_bar *search_bar, int is_loading);

/**
 * @brief Opens the associated full-screen search view overlay.
 *
 * @param search_bar The search bar.
 * @param out_view Pointer to receive search view handle.
 * @return UI_ERROR_NONE on success, or an error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_bar_open_view(
    struct md3_search_bar *search_bar, struct md3_search_view **out_view);

/**
 * @brief Closes the search view overlay.
 *
 * @param view The search view.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
md3_search_view_close(struct md3_search_view *view);

/**
 * @brief Appends an entry to search history.
 *
 * @param view The search view.
 * @param item History string to append.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_view_add_history_item(
    struct md3_search_view *view, const char *item);

/**
 * @brief Retrieves number of history entries.
 *
 * @param view The search view.
 * @param out_count Pointer to receive history item count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_view_get_history_count(
    const struct md3_search_view *view, size_t *out_count);

/**
 * @brief Retrieves history entry by index.
 *
 * @param view The search view.
 * @param index Zero-based item index.
 * @param out_item Pointer to receive string pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_OUT_OF_BOUNDS.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_view_get_history_item(
    const struct md3_search_view *view, size_t index, const char **out_item);

/**
 * @brief Retrieves the underlying ui_search_bar_base handle.
 *
 * @param search_bar The search bar.
 * @param out_base Pointer to receive base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t md3_search_bar_get_base(
    struct md3_search_bar *search_bar, struct ui_search_bar_base **out_base);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL3_MD3_SEARCH_H */
