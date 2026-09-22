/**
 * @file test_app_state.c
 * @brief Unit tests for application state and synchronization coordinator.
 */

/* clang-format off */
#include "../app_state.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static enum oauth2_app_error test_state_flow(void) {
  struct app_state *state;
  size_t synced_count = 0;
  enum oauth2_app_error rc;

  /* Null checks */
  rc = app_state_init(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL state init\n");
    return rc;
  }

  rc = app_state_destroy(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL state destroy\n");
    return rc;
  }

  /* Init */
  rc = app_state_init(&state);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "State init failed: %d\n", (int)rc);
    return rc;
  }

  /* Test backend URL set/get and null handling */
  rc = app_state_set_backend_url(NULL, "http://localhost");
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL state set_backend_url\n");
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = app_state_get_backend_url(NULL, NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL state get_backend_url\n");
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  {
    const char *retrieved_url = NULL;
    rc = app_state_get_backend_url(state, &retrieved_url);
    if (rc != OAUTH2_APP_OK || retrieved_url == NULL) {
      fprintf(stderr, "get_backend_url failed: %d\n", (int)rc);
      rc = app_state_destroy(state);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    rc = app_state_set_backend_url(state, "http://custom.backend:8888");
    if (rc != OAUTH2_APP_OK) {
      fprintf(stderr, "set_backend_url failed: %d\n", (int)rc);
      rc = app_state_destroy(state);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    rc = app_state_get_backend_url(state, &retrieved_url);
    if (rc != OAUTH2_APP_OK ||
        strcmp(retrieved_url, "http://custom.backend:8888") != 0) {
      fprintf(stderr, "Backend URL mismatch after set: %s\n",
              retrieved_url != NULL ? retrieved_url : "NULL");
      rc = app_state_destroy(state);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return OAUTH2_APP_ERROR_INVALID_PARAM;
    }
  }

  /* Offline login */
  rc = app_state_login(state, "admin", "password");
  if (rc != OAUTH2_APP_OK || !state->is_authenticated) {
    fprintf(stderr, "Offline login failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Edit about me offline */
  rc = app_state_update_about_me(state, "Modified Offline Admin Bio");
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Update about me failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (state->current_user.sync_status != OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY) {
    fprintf(stderr, "Expected MODIFIED_LOCALLY status after offline edit\n");
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Switch mode to Online */
  rc = app_state_set_mode(state, OAUTH2_MODE_ONLINE);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Set mode failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Sync dirty records */
  rc = app_state_sync_dirty_records(state, &synced_count);
  if (rc != OAUTH2_APP_OK || synced_count == 0) {
    fprintf(stderr, "Sync dirty records failed: %d, count: %lu\n", (int)rc,
            (unsigned long)synced_count);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Online login */
  rc = app_state_login(state, "online_admin", "password");
  if (rc != OAUTH2_APP_OK || !state->is_authenticated) {
    fprintf(stderr, "Online login failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Online edit about me */
  rc = app_state_update_about_me(state, "Online Updated Admin Bio");
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Online update about me failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Logout */
  rc = app_state_logout(state);
  if (rc != OAUTH2_APP_OK || state->is_authenticated != 0) {
    fprintf(stderr, "Logout failed: %d\n", (int)rc);
    rc = app_state_destroy(state);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = app_state_destroy(state);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  return OAUTH2_APP_OK;
}

int main(void) {
  enum oauth2_app_error rc;

  printf("Running test_app_state...\n");
  rc = test_state_flow();
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "test_app_state failed with code %d\n", (int)rc);
    return 1;
  }

  printf("test_app_state passed successfully!\n");
  return 0;
}
