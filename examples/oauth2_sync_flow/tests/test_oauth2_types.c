/**
 * @file test_oauth2_types.c
 * @brief Unit tests for core types, safe strcpy, and initializers.
 */

/* clang-format off */
#include "../oauth2_types.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static enum oauth2_app_error test_types_init(void) {
  struct oauth2_user user;
  struct oauth2_session session;
  char buf[32];
  enum oauth2_app_error rc;

  /* Test null checks */
  rc = oauth2_user_init(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL user\n");
    return rc;
  }

  rc = oauth2_session_init(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL session\n");
    return rc;
  }

  rc = oauth2_safe_strcpy(NULL, 10, "test");
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL dest\n");
    return rc;
  }

  rc = oauth2_safe_strcpy(buf, 10, NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL src\n");
    return rc;
  }

  rc = oauth2_safe_strcpy(buf, 0, "test");
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for zero size\n");
    return rc;
  }

  /* Test valid user initialization */
  rc = oauth2_user_init(&user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "oauth2_user_init failed: %d\n", (int)rc);
    return rc;
  }
  if (user.role != OAUTH2_ROLE_USER ||
      user.sync_status != OAUTH2_SYNC_STATUS_SYNCED) {
    fprintf(stderr, "User fields mismatch after init\n");
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test valid session initialization */
  rc = oauth2_session_init(&session);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "oauth2_session_init failed: %d\n", (int)rc);
    return rc;
  }
  if (session.access_token[0] != '\0' || session.expires_in != 0) {
    fprintf(stderr, "Session fields mismatch after init\n");
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test safe string copy */
  rc = oauth2_safe_strcpy(buf, sizeof(buf), "hello oauth2");
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "oauth2_safe_strcpy failed: %d\n", (int)rc);
    return rc;
  }
  if (strcmp(buf, "hello oauth2") != 0) {
    fprintf(stderr, "String content mismatch: %s\n", buf);
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  return OAUTH2_APP_OK;
}

int main(void) {
  enum oauth2_app_error rc;

  printf("Running test_oauth2_types...\n");
  rc = test_types_init();
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "test_oauth2_types failed with code %d\n", (int)rc);
    return 1;
  }

  printf("test_oauth2_types passed successfully!\n");
  return 0;
}
