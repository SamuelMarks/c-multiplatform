/**
 * @file oauth2_types.h
 * @brief Core types, error enums, and definitions for OAuth2 sync flow example.
 */

#ifndef OAUTH2_TYPES_H
#define OAUTH2_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef unsigned __int64 oauth2_uint64_t;
typedef __int64 oauth2_int64_t;
#else
#include <stdint.h>
typedef uint64_t oauth2_uint64_t;
typedef int64_t oauth2_int64_t;
#endif

#include "ui_error.h"
/* clang-format on */

#if defined(_MSC_VER)
#define OAUTH2_FMT_I64 "%I64d"
#define OAUTH2_FMT_U64 "%I64u"
#else
#define OAUTH2_FMT_I64 "%lld"
#define OAUTH2_FMT_U64 "%llu"
#endif

/**
 * @brief Error codes returned by all OAuth2 sync application operations.
 */
enum oauth2_app_error {
  OAUTH2_APP_OK = 0, /**< Operation completed successfully. */
  OAUTH2_APP_ERROR_INVALID_PARAM =
      1,                        /**< Null pointer or out of bounds argument. */
  OAUTH2_APP_ERROR_MEMORY = 2,  /**< Memory allocation failed. */
  OAUTH2_APP_ERROR_NETWORK = 3, /**< Network connection failure. */
  OAUTH2_APP_ERROR_AUTH_FAILED =
      4, /**< Invalid username, password, or token. */
  OAUTH2_APP_ERROR_NOT_FOUND =
      5, /**< Requested entity or user was not found. */
  OAUTH2_APP_ERROR_PERMISSION_DENIED = 6, /**< User lacks required role. */
  OAUTH2_APP_ERROR_STORAGE_FAILED = 7,    /**< Local cache read/write failed. */
  OAUTH2_APP_ERROR_PARSE_FAILED = 8 /**< JSON or payload format is invalid. */
};

/**
 * @brief Typedef for oauth2_app_error.
 */
typedef enum oauth2_app_error oauth2_app_error_t;

/**
 * @brief Typedef for oauth2_app_error with identical name for convenience.
 */
typedef enum oauth2_app_error oauth2_app_error;

/**
 * @brief Operating mode for data retrieval and profile persistence.
 */
enum oauth2_app_mode {
  OAUTH2_MODE_ONLINE = 0, /**< Live HTTP/REST connection to c-rest-framework. */
  OAUTH2_MODE_OFFLINE = 1 /**< Air-gapped local cache and offline users. */
};

/**
 * @brief Synchronization status for user profile records.
 */
enum oauth2_sync_status {
  OAUTH2_SYNC_STATUS_SYNCED = 0,           /**< In sync with remote backend. */
  OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY = 1, /**< Edited offline, pending sync. */
  OAUTH2_SYNC_STATUS_LOCAL_ONLY = 2        /**< Offline-only account. */
};

/**
 * @brief Security role assigned to a user account.
 */
enum oauth2_user_role {
  OAUTH2_ROLE_USER = 0, /**< Standard user role. */
  OAUTH2_ROLE_ADMIN =
      1 /**< Administrator role with directory viewing privileges. */
};

/**
 * @brief User record representation.
 */
struct oauth2_user {
  char id[64];
  char username[64];
  char about_me[512];
  enum oauth2_user_role role;
  enum oauth2_sync_status sync_status;
  oauth2_uint64_t updated_at;
};

/**
 * @brief Active OAuth2 session credentials.
 */
struct oauth2_session {
  char access_token[256];
  char refresh_token[256];
  char token_type[32];
  char user_id[64];
  long expires_in;
};

/**
 * @brief Safely copies a null-terminated string into a destination buffer.
 *
 * @param dest Destination buffer.
 * @param dest_sz Size in bytes of destination buffer.
 * @param src Source null-terminated string.
 * @return OAUTH2_APP_OK on success, or OAUTH2_APP_ERROR_INVALID_PARAM.
 */
enum oauth2_app_error oauth2_safe_strcpy(char *dest, size_t dest_sz,
                                         const char *src);

/**
 * @brief Initializes a user structure to default empty values.
 *
 * @param user Pointer to user structure.
 * @return OAUTH2_APP_OK on success, or OAUTH2_APP_ERROR_INVALID_PARAM.
 */
enum oauth2_app_error oauth2_user_init(struct oauth2_user *user);

/**
 * @brief Initializes an OAuth2 session structure to default empty values.
 *
 * @param session Pointer to session structure.
 * @return OAUTH2_APP_OK on success, or OAUTH2_APP_ERROR_INVALID_PARAM.
 */
enum oauth2_app_error oauth2_session_init(struct oauth2_session *session);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OAUTH2_TYPES_H */
