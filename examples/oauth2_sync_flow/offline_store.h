/**
 * @file offline_store.h
 * @brief Offline user repository and local cache persistence for OAuth2 sync
 * flow.
 */

#ifndef OFFLINE_STORE_H
#define OFFLINE_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include "oauth2_types.h"
/* clang-format on */

/**
 * @brief Maximum number of user accounts stored in the offline repository.
 */
#define OAUTH2_MAX_STORED_USERS 64

/**
 * @brief Offline user store instance.
 */
struct oauth2_offline_store {
  struct oauth2_user users[OAUTH2_MAX_STORED_USERS];
  size_t count;
};

/**
 * @brief Initializes a new offline store instance.
 *
 * @param out_store Pointer to receive the allocated store pointer.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_init(struct oauth2_offline_store **out_store);

/**
 * @brief Frees all memory associated with the offline store.
 *
 * @param store Pointer to offline store.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_destroy(struct oauth2_offline_store *store);

/**
 * @brief Seeds the store with standard offline test accounts (admin,
 * offline_alice).
 *
 * @param store Pointer to offline store.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_seed_defaults(struct oauth2_offline_store *store);

/**
 * @brief Adds or updates a user in the offline store.
 *
 * @param store Pointer to offline store.
 * @param user Pointer to user details to add or update.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_upsert_user(struct oauth2_offline_store *store,
                                 const struct oauth2_user *user);

/**
 * @brief Finds a user by ID.
 *
 * @param store Pointer to offline store.
 * @param user_id Identifier of the user.
 * @param out_user Pointer to receive the found user record.
 * @return OAUTH2_APP_OK on success, OAUTH2_APP_ERROR_NOT_FOUND if not found.
 */
enum oauth2_app_error
oauth2_offline_store_get_user_by_id(const struct oauth2_offline_store *store,
                                    const char *user_id,
                                    struct oauth2_user *out_user);

/**
 * @brief Finds a user by username.
 *
 * @param store Pointer to offline store.
 * @param username Username to search.
 * @param out_user Pointer to receive the found user record.
 * @return OAUTH2_APP_OK on success, OAUTH2_APP_ERROR_NOT_FOUND if not found.
 */
enum oauth2_app_error oauth2_offline_store_get_user_by_username(
    const struct oauth2_offline_store *store, const char *username,
    struct oauth2_user *out_user);

/**
 * @brief Returns an array of all users in the store.
 *
 * @param store Pointer to offline store.
 * @param out_users Pointer to receive internal pointer or copy array.
 * @param out_count Pointer to receive count of returned users.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_list_users(const struct oauth2_offline_store *store,
                                struct oauth2_user **out_users,
                                size_t *out_count);

/**
 * @brief Updates the free-text "About Me" bio for a specified user and flags it
 * as modified.
 *
 * @param store Pointer to offline store.
 * @param user_id Identifier of user to update.
 * @param new_about_me New description text.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_update_about_me(struct oauth2_offline_store *store,
                                     const char *user_id,
                                     const char *new_about_me);

/**
 * @brief Persists the offline store contents to a JSON file.
 *
 * @param store Pointer to offline store.
 * @param file_path Destination filesystem path.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_save_to_file(const struct oauth2_offline_store *store,
                                  const char *file_path);

/**
 * @brief Loads the offline store contents from a JSON file.
 *
 * @param store Pointer to offline store.
 * @param file_path Source filesystem path.
 * @return OAUTH2_APP_OK on success, or an error code.
 */
enum oauth2_app_error
oauth2_offline_store_load_from_file(struct oauth2_offline_store *store,
                                    const char *file_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OFFLINE_STORE_H */
