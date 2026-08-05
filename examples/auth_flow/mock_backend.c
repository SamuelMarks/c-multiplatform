/**
 * @file mock_backend.c
 * @brief Implementation of the mock backend authentication service.
 */

/* clang-format off */
#include "mock_backend.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Internal duplicate of the application state structure for mock
 * testing.
 */
struct app_state {
  void *arena;                  /**< Memory arena placeholder */
  void *theme_manager;          /**< Theme manager placeholder */
  void *i18n;                   /**< I18n manager placeholder */
  void *router;                 /**< Router placeholder */
  void *root;                   /**< Root node placeholder */
  void *toolbar;                /**< Toolbar node placeholder */
  void *toolbar_title;          /**< Toolbar title placeholder */
  void *router_outlet;          /**< Router outlet placeholder */
  void *btn_theme;              /**< Theme button placeholder */
  void *btn_lang;               /**< Language button placeholder */
  void *host_theme;             /**< Theme host placeholder */
  void *host_lang;              /**< Language host placeholder */
  int current_lang_idx;         /**< Current language index */
  int is_authenticated;         /**< Authentication state flag */
  char current_user[64];        /**< Current username buffer */
  char auth_error_message[128]; /**< Error message buffer */
};

ui_error_t mock_login(struct app_state *state, const char *username,
                      const char *password) {
  if (strcmp(username, "admin") == 0 && strcmp(password, "password") == 0) {
    state->is_authenticated = 1;
    strncpy(state->current_user, username, 63);
    state->current_user[63] = '\0';
    state->auth_error_message[0] = '\0';
    return UI_ERROR_NONE;
  }

  state->is_authenticated = 0;
  strncpy(state->auth_error_message, "err_invalid_credentials", 127);
  state->auth_error_message[127] = '\0';
  return UI_ERROR_INVALID_ARGUMENT;
}

ui_error_t mock_signup(struct app_state *state, const char *username,
                       const char *password) {
  if (strcmp(username, "admin") == 0) {
    state->is_authenticated = 0;
    strncpy(state->auth_error_message, "err_user_exists", 127);
    state->auth_error_message[127] = '\0';
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (strlen(password) < 4) {
    state->is_authenticated = 0;
    strncpy(state->auth_error_message, "err_invalid_credentials", 127);
    state->auth_error_message[127] = '\0';
    return UI_ERROR_INVALID_ARGUMENT;
  }

  state->is_authenticated = 1;
  strncpy(state->current_user, username, 63);
  state->current_user[63] = '\0';
  state->auth_error_message[0] = '\0';
  return UI_ERROR_NONE;
}

ui_error_t mock_logout(struct app_state *state) {
  state->is_authenticated = 0;
  state->current_user[0] = '\0';
  state->auth_error_message[0] = '\0';
  return UI_ERROR_NONE;
}
