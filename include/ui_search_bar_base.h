#ifndef UI_SEARCH_BAR_BASE_H
#define UI_SEARCH_BAR_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_control_value_accessor.h"
#include <stddef.h>
/* clang-format on */

struct ui_component;
struct ui_icon_base;

/**
 * @brief Base state for a Search Bar widget.
 */
struct ui_search_bar_base {
  struct ui_component *component;
  char *query;
  struct ui_icon_base *leading_icon;
  struct ui_icon_base *trailing_icon;
  int is_loading;

  enum ui_error (*cva_on_change)(union ui_signal_payload new_value,
                                 void *user_data);
  void *cva_on_change_user_data;

  enum ui_error (*cva_on_touched)(void *user_data);
  void *cva_on_touched_user_data;

  int is_disabled;
};

/**
 * @brief Initializes a base search bar component.
 *
 * @param search_bar Pointer to the search bar base struct.
 * @param component The UI component to bind to.
 * @param out_cva Optional pointer to receive the CVA interface.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_search_bar_base_init(struct ui_search_bar_base *search_bar,
                        struct ui_component *component,
                        struct ui_control_value_accessor *out_cva);

/**
 * @brief Sets the text query in the search bar.
 *
 * @param search_bar Pointer to the search bar base struct.
 * @param query The query text.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_search_bar_base_set_query(struct ui_search_bar_base *search_bar,
                             const char *query);

/**
 * @brief Sets whether the search bar is in a loading state.
 *
 * @param search_bar Pointer to the search bar base struct.
 * @param is_loading The new loading state.
 */
enum ui_error
ui_search_bar_base_set_loading(struct ui_search_bar_base *search_bar,
                               int is_loading);

/**
 * @brief Cleans up resources allocated by the search bar base.
 *
 * @param search_bar Pointer to the search bar base struct.
 */
enum ui_error ui_search_bar_base_cleanup(struct ui_search_bar_base *search_bar);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_SEARCH_BAR_BASE_H */
