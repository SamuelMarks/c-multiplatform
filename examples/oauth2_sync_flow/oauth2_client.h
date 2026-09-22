/**
 * @file oauth2_client.h
 * @brief OAuth2 and REST client communicating with c-rest-framework server.
 */

#ifndef OAUTH2_CLIENT_H
#define OAUTH2_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "oauth2_types.h"
/* clang-format on */

/**
 * @brief Maximum users simulated or cached in client directory.
 */
#define OAUTH2_CLIENT_MAX_USERS 32

/**
 * @brief OAuth2 and REST client instance structure.
 */
struct oauth2_client {
  char base_url[128];
  int simulate_network_failure;
  struct oauth2_user server_users[OAUTH2_CLIENT_MAX_USERS];
  size_t server_user_count;
};

/**
 * @brief Initializes an OAuth2 REST client instance.
 *
 * @param base_url The endpoint URL for the c-rest-framework server.
 * @param out_client Pointer to receive the allocated client.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error oauth2_client_init(const char *base_url,
                                         struct oauth2_client **out_client);

/**
 * @brief Destroys an OAuth2 client instance and releases resources.
 *
 * @param client Pointer to client instance.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error oauth2_client_destroy(struct oauth2_client *client);

/**
 * @brief Toggles network failure simulation for testing error percolation.
 *
 * @param client Pointer to client instance.
 * @param enable 1 to simulate network failure, 0 for normal operation.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_client_set_network_failure(struct oauth2_client *client, int enable);

/**
 * @brief Sets or updates the base URL for the backend server.
 *
 * @param client Pointer to client instance.
 * @param new_url New base URL string.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error oauth2_client_set_base_url(struct oauth2_client *client,
                                                 const char *new_url);

/**
 * @brief Retrieves the currently configured base URL of the backend server.
 *
 * @param client Pointer to client instance.
 * @param out_url Pointer to receive base URL string pointer.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_client_get_base_url(const struct oauth2_client *client,
                           const char **out_url);

/**
 * @brief Upserts a user record on the remote server (used by sync engine).
 *
 * @param client Pointer to client instance.
 * @param user User record to create or update.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error oauth2_client_upsert_user(struct oauth2_client *client,
                                                const struct oauth2_user *user);

/**
 * @brief Authenticates via OAuth2 Resource Owner Password Credentials Grant.
 *
 * @param client Pointer to client instance.
 * @param username User credential.
 * @param password Password credential.
 * @param out_session Pointer to receive granted session token.
 * @return OAUTH2_APP_OK on success, OAUTH2_APP_ERROR_AUTH_FAILED on invalid
 * credentials.
 */
enum oauth2_app_error
oauth2_client_login_password(struct oauth2_client *client, const char *username,
                             const char *password,
                             struct oauth2_session *out_session);

/**
 * @brief Renews an expired access token using a refresh token.
 *
 * @param client Pointer to client instance.
 * @param refresh_token Current refresh token.
 * @param out_session Pointer to receive renewed session.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_client_refresh_token(struct oauth2_client *client,
                            const char *refresh_token,
                            struct oauth2_session *out_session);

/**
 * @brief Fetches current user profile from GET /api/v1/users/me.
 *
 * @param client Pointer to client instance.
 * @param session Active session credentials.
 * @param out_user Pointer to receive user profile.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_client_get_current_user(struct oauth2_client *client,
                               const struct oauth2_session *session,
                               struct oauth2_user *out_user);

/**
 * @brief Updates user bio on remote server via PUT /api/v1/users/me.
 *
 * @param client Pointer to client instance.
 * @param session Active session credentials.
 * @param about_me Updated description string.
 * @param out_user Pointer to receive updated user record.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error oauth2_client_update_profile(
    struct oauth2_client *client, const struct oauth2_session *session,
    const char *about_me, struct oauth2_user *out_user);

/**
 * @brief Lists all users on server via GET /api/v1/users (Admin-only).
 *
 * @param client Pointer to client instance.
 * @param session Active session credentials.
 * @param out_users Pointer to receive array of user records.
 * @param out_count Pointer to receive count of returned users.
 * @return OAUTH2_APP_OK on success, OAUTH2_APP_ERROR_PERMISSION_DENIED if not
 * admin.
 */
enum oauth2_app_error
oauth2_client_list_all_users(struct oauth2_client *client,
                             const struct oauth2_session *session,
                             struct oauth2_user **out_users, size_t *out_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OAUTH2_CLIENT_H */
