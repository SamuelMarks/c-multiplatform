/**
 * @file sampler_top_app_bar.h
 * @brief Top App Bar component and search field for Compose Material Catalog.
 */

#ifndef SAMPLER_TOP_APP_BAR_H
#define SAMPLER_TOP_APP_BAR_H

/* clang-format off */
#include "sampler/sampler_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ui_dom_node;

/**
 * @brief Callback invoked when navigation back button is clicked.
 * @param user_data Opaque pointer passed in configuration.
 * @return SAMPLER_SUCCESS on success, or error code.
 */
typedef sampler_error_t (*sampler_on_back_fn)(void *user_data);

/**
 * @brief Callback invoked when favorite pin button state is toggled.
 * @param new_state 1 if newly pinned, 0 if unpinned.
 * @param user_data Opaque pointer passed in configuration.
 * @return SAMPLER_SUCCESS on success, or error code.
 */
typedef sampler_error_t (*sampler_on_favorite_fn)(int new_state,
                                                  void *user_data);

/**
 * @brief Callback invoked when theme palette button is clicked.
 * @param user_data Opaque pointer passed in configuration.
 * @return SAMPLER_SUCCESS on success, or error code.
 */
typedef sampler_error_t (*sampler_on_theme_fn)(void *user_data);

/**
 * @brief Callback invoked when search query text changes.
 * @param query Current text in search field.
 * @param user_data Opaque pointer passed in configuration.
 * @return SAMPLER_SUCCESS on success, or error code.
 */
typedef sampler_error_t (*sampler_on_search_fn)(const char *query,
                                                void *user_data);

/**
 * @struct sampler_top_app_bar_config
 * @brief Configuration parameters for constructing Top App Bar.
 */
struct sampler_top_app_bar_config {
  /** @brief Header title text. */
  const char *title;
  /** @brief Nonzero to display leading back navigation arrow. */
  int show_back_button;
  /** @brief Nonzero to render collapsible search field. */
  int show_search_field;
  /** @brief Nonzero to display favorite push-pin action toggle. */
  int show_favorite_pin;
  /** @brief Initial favorite pin state (1 = pinned, 0 = unpinned). */
  int is_favorite_pinned;
  /** @brief Nonzero to display theme palette action icon. */
  int show_theme_button;
  /** @brief Nonzero to display more options (three dots) action menu. */
  int show_more_menu;
  /** @brief Optional external documentation / source URL link. */
  const char *external_url;
  /** @brief Back button click callback. */
  sampler_on_back_fn on_back;
  /** @brief Favorite toggle callback. */
  sampler_on_favorite_fn on_favorite;
  /** @brief Theme button click callback. */
  sampler_on_theme_fn on_theme;
  /** @brief Search field change callback. */
  sampler_on_search_fn on_search;
  /** @brief User data pointer passed to callbacks. */
  void *user_data;
};

struct ui_engine;

/**
 * @brief Construct a Top App Bar DOM hierarchy according to configuration.
 * @param engine Pointer to the UI engine.
 * @param config Pointer to top app bar configuration settings.
 * @param out_node Pointer receiving root DOM node of Top App Bar.
 * @return SAMPLER_SUCCESS on success, or appropriate error code.
 */
sampler_error_t
sampler_top_app_bar_create(struct ui_engine *engine,
                           const struct sampler_top_app_bar_config *config,
                           struct ui_dom_node **out_node);

/**
 * @brief Dynamically update favorite pin visual state on an existing Top App
 * Bar.
 * @param top_bar_node Root DOM node of the Top App Bar.
 * @param is_pinned 1 if pinned, 0 if unpinned.
 * @return SAMPLER_SUCCESS on success, or SAMPLER_ERROR_NULL_POINTER.
 */
sampler_error_t sampler_top_app_bar_set_pinned(struct ui_dom_node *top_bar_node,
                                               int is_pinned);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SAMPLER_TOP_APP_BAR_H */
