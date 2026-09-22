/**
 * @file app_state.h
 * @brief Application state and mode synchronization coordinator.
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "oauth2_types.h"
#include "offline_store.h"
#include "oauth2_client.h"
/* clang-format on */

/**
 * @brief Forward declarations for engine pointers.
 */
struct ui_arena;
struct ui_router;

/**
 * @brief Unified application state coordinating views, offline store, and
 * online client.
 */
struct app_state {
  enum oauth2_app_mode mode;
  int is_authenticated;
  struct oauth2_user current_user;
  struct oauth2_session current_session;
  struct oauth2_offline_store *offline_store;
  struct oauth2_client *online_client;
  char last_error[128];
  struct ui_arena *arena;
  struct ui_router *router;
};

/**
 * @brief Allocates and initializes application state, offline store, and online
 * client.
 *
 * @param out_state Pointer to receive allocated app state.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_init(struct app_state **out_state);

/**
 * @brief Releases all resources held by the application state.
 *
 * @param state Pointer to application state.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_destroy(struct app_state *state);

/**
 * @brief Sets the operating mode (Online vs Offline).
 *
 * @param state Pointer to application state.
 * @param new_mode Target operating mode.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_set_mode(struct app_state *state,
                                         enum oauth2_app_mode new_mode);

/**
 * @brief Configures the live backend server URL endpoint.
 *
 * @param state Pointer to application state.
 * @param url Target backend server URL.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_set_backend_url(struct app_state *state,
                                                const char *url);

/**
 * @brief Retrieves the currently configured live backend server URL endpoint.
 *
 * @param state Pointer to application state.
 * @param out_url Pointer to receive base URL string pointer.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_get_backend_url(const struct app_state *state,
                                                const char **out_url);

/**
 * @brief Authenticates a user in either Offline or Online mode.
 *
 * @param state Pointer to application state.
 * @param username Credential username.
 * @param password Credential password.
 * @return OAUTH2_APP_OK on success, OAUTH2_APP_ERROR_AUTH_FAILED on invalid
 * credentials.
 */
enum oauth2_app_error app_state_login(struct app_state *state,
                                      const char *username,
                                      const char *password);

/**
 * @brief Logs out the active user and invalidates session credentials.
 *
 * @param state Pointer to application state.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_logout(struct app_state *state);

/**
 * @brief Updates the "About Me" description in local store or remote server.
 *
 * @param state Pointer to application state.
 * @param new_about_me New description text.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_update_about_me(struct app_state *state,
                                                const char *new_about_me);

/**
 * @brief Synchronizes offline dirty records to the online server.
 *
 * @param state Pointer to application state.
 * @param out_synced_count Pointer to receive count of synced records.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error app_state_sync_dirty_records(struct app_state *state,
                                                   size_t *out_synced_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_STATE_H */
