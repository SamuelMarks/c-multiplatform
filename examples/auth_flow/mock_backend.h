/**
 * @file mock_backend.h
 * @brief Mock backend authentication service.
 *
 * Simulates server API calls for authentication flows including login, signup,
 * and logout.
 */
#ifndef MOCK_BACKEND_H
#define MOCK_BACKEND_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Forward declaration of application state.
 */
struct app_state;

/**
 * @brief Simulates a login attempt.
 *
 * @param state The global application state.
 * @param username The username string.
 * @param password The password string.
 * @return UI_ERROR_NONE on successful authentication, or an appropriate error
 * code.
 */
ui_error_t mock_login(struct app_state *state, const char *username,
                      const char *password);

/**
 * @brief Simulates a signup attempt.
 *
 * @param state The global application state.
 * @param username The requested username string.
 * @param password The requested password string.
 * @return UI_ERROR_NONE on successful registration and authentication, or an
 * appropriate error code.
 */
ui_error_t mock_signup(struct app_state *state, const char *username,
                       const char *password);

/**
 * @brief Simulates a logout request, clearing the current user state.
 *
 * @param state The global application state.
 * @return UI_ERROR_NONE on successful logout.
 */
ui_error_t mock_logout(struct app_state *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOCK_BACKEND_H */
