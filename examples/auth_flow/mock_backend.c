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

/**
 * @brief Authenticates credentials against the mock database.
 * @param state The application state.
 * @param username Entered username.
 * @param password Entered password.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t mock_login(struct app_state *state, const char *username,
                      const char *password) {
  if (!state || !username || !password) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (strcmp(username, "admin") == 0 && strcmp(password, "password") == 0) {
    state->is_authenticated = 1;
#if defined(_MSC_VER)
    strncpy_s(state->current_user, sizeof(state->current_user), username,
              _TRUNCATE);
#else
    strncpy(state->current_user, username, sizeof(state->current_user) - 1);
    state->current_user[sizeof(state->current_user) - 1] = '\0';
#endif
    state->auth_error_message[0] = '\0';
    return UI_ERROR_NONE;
  }

  state->is_authenticated = 0;
#if defined(_MSC_VER)
  strncpy_s(state->auth_error_message, sizeof(state->auth_error_message),
            "err_invalid_credentials", _TRUNCATE);
#else
  strncpy(state->auth_error_message, "err_invalid_credentials",
          sizeof(state->auth_error_message) - 1);
  state->auth_error_message[sizeof(state->auth_error_message) - 1] = '\0';
#endif
  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief Registers a new user in the mock database.
 * @param state The application state.
 * @param username Entered username.
 * @param password Entered password.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t mock_signup(struct app_state *state, const char *username,
                       const char *password) {
  if (!state || !username || !password) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (strcmp(username, "admin") == 0) {
    state->is_authenticated = 0;
#if defined(_MSC_VER)
    strncpy_s(state->auth_error_message, sizeof(state->auth_error_message),
              "err_user_exists", _TRUNCATE);
#else
    strncpy(state->auth_error_message, "err_user_exists",
            sizeof(state->auth_error_message) - 1);
    state->auth_error_message[sizeof(state->auth_error_message) - 1] = '\0';
#endif
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (strlen(password) < 4) {
    state->is_authenticated = 0;
#if defined(_MSC_VER)
    strncpy_s(state->auth_error_message, sizeof(state->auth_error_message),
              "err_invalid_credentials", _TRUNCATE);
#else
    strncpy(state->auth_error_message, "err_invalid_credentials",
            sizeof(state->auth_error_message) - 1);
    state->auth_error_message[sizeof(state->auth_error_message) - 1] = '\0';
#endif
    return UI_ERROR_INVALID_ARGUMENT;
  }

  state->is_authenticated = 1;
#if defined(_MSC_VER)
  strncpy_s(state->current_user, sizeof(state->current_user), username,
            _TRUNCATE);
#else
  strncpy(state->current_user, username, sizeof(state->current_user) - 1);
  state->current_user[sizeof(state->current_user) - 1] = '\0';
#endif
  state->auth_error_message[0] = '\0';
  return UI_ERROR_NONE;
}

/**
 * @brief Clears authentication credentials and resets user state.
 * @param state The application state.
 * @return UI_ERROR_NONE on success, or an error code on failure.
 */
ui_error_t mock_logout(struct app_state *state) {
  if (!state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  state->is_authenticated = 0;
  state->current_user[0] = '\0';
  state->auth_error_message[0] = '\0';
  return UI_ERROR_NONE;
}
