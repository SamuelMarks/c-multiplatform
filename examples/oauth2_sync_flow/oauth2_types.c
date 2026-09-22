/**
 * @file oauth2_types.c
 * @brief Core types and utility implementations for OAuth2 sync flow example.
 */

/* clang-format off */
#include "oauth2_types.h"
/* clang-format on */

enum oauth2_app_error oauth2_safe_strcpy(char *dest, size_t dest_sz,
                                         const char *src) {
  if (dest == NULL || src == NULL || dest_sz == 0) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

#if defined(_MSC_VER)
  if (strcpy_s(dest, dest_sz, src) != 0) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }
#else
  strncpy(dest, src, dest_sz - 1);
  dest[dest_sz - 1] = '\0';
#endif

  return OAUTH2_APP_OK;
}

enum oauth2_app_error oauth2_user_init(struct oauth2_user *user) {
  if (user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  memset(user, 0, sizeof(struct oauth2_user));
  user->role = OAUTH2_ROLE_USER;
  user->sync_status = OAUTH2_SYNC_STATUS_SYNCED;
  user->updated_at = 0;

  return OAUTH2_APP_OK;
}

enum oauth2_app_error oauth2_session_init(struct oauth2_session *session) {
  if (session == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  memset(session, 0, sizeof(struct oauth2_session));
  return OAUTH2_APP_OK;
}
