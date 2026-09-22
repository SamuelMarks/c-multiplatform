/**
 * @file test_oauth2_client.c
 * @brief Unit tests for OAuth2 client authentication, profile, and admin
 * directory.
 */

/* clang-format off */
#include "../oauth2_client.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static enum oauth2_app_error test_client_flow(void) {
  struct oauth2_client *cli;
  struct oauth2_session session_admin;
  struct oauth2_session session_user;
  struct oauth2_session session_renewed;
  struct oauth2_user user;
  struct oauth2_user *users_list;
  size_t count;
  enum oauth2_app_error rc;

  /* Test NULL checks */
  rc = oauth2_client_init(NULL, NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL out_client\n");
    return rc;
  }

  rc = oauth2_client_destroy(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL client destroy\n");
    return rc;
  }

  rc = oauth2_client_set_base_url(NULL, "https://api.example.com");
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL client set_base_url\n");
    return rc;
  }

  rc = oauth2_client_get_base_url(NULL, NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL client get_base_url\n");
    return rc;
  }

  /* Valid client init */
  rc = oauth2_client_init("http://localhost:8080", &cli);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Client init failed: %d\n", (int)rc);
    return rc;
  }

  /* Test base_url set & get */
  {
    const char *retrieved_url = NULL;
    rc = oauth2_client_get_base_url(cli, &retrieved_url);
    if (rc != OAUTH2_APP_OK ||
        strcmp(retrieved_url, "http://localhost:8080") != 0) {
      fprintf(stderr, "Initial base_url mismatch\n");
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return OAUTH2_APP_ERROR_INVALID_PARAM;
    }

    rc = oauth2_client_set_base_url(cli, "https://live.backend.internal:9090");
    if (rc != OAUTH2_APP_OK) {
      fprintf(stderr, "set_base_url failed\n");
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    rc = oauth2_client_get_base_url(cli, &retrieved_url);
    if (rc != OAUTH2_APP_OK ||
        strcmp(retrieved_url, "https://live.backend.internal:9090") != 0) {
      fprintf(stderr, "Updated base_url mismatch\n");
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return OAUTH2_APP_ERROR_INVALID_PARAM;
    }
  }

  /* Test upsert user */
  {
    struct oauth2_user new_u;
    rc = oauth2_user_init(&new_u);
    if (rc != OAUTH2_APP_OK) {
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }
    rc = oauth2_safe_strcpy(new_u.id, sizeof(new_u.id), "srv_new_user");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(new_u.username, sizeof(new_u.username), "new_user");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(new_u.about_me, sizeof(new_u.about_me),
                            "Newly upserted user");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    new_u.role = OAUTH2_ROLE_USER;
    new_u.sync_status = OAUTH2_SYNC_STATUS_SYNCED;

    rc = oauth2_client_upsert_user(cli, &new_u);
    if (rc != OAUTH2_APP_OK) {
      fprintf(stderr, "Failed to upsert new user: %d\n", (int)rc);
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    /* Update existing */
    rc = oauth2_safe_strcpy(new_u.about_me, sizeof(new_u.about_me),
                            "Updated bio text");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_client_upsert_user(cli, &new_u);
    if (rc != OAUTH2_APP_OK) {
      fprintf(stderr, "Failed to update existing upserted user: %d\n", (int)rc);
      rc = oauth2_client_destroy(cli);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }
  }

  /* Failed login */
  rc = oauth2_client_login_password(cli, "online_admin", "bad_pass",
                                    &session_admin);
  if (rc != OAUTH2_APP_ERROR_AUTH_FAILED) {
    fprintf(stderr, "Expected AUTH_FAILED on bad password\n");
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Successful admin login */
  rc = oauth2_client_login_password(cli, "online_admin", "password",
                                    &session_admin);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Admin login failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Fetch current user */
  rc = oauth2_client_get_current_user(cli, &session_admin, &user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Get current user failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (user.role != OAUTH2_ROLE_ADMIN) {
    fprintf(stderr, "Expected ADMIN role for online_admin\n");
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Update profile */
  rc = oauth2_client_update_profile(cli, &session_admin, "New Online Admin Bio",
                                    &user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Update profile failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (strcmp(user.about_me, "New Online Admin Bio") != 0) {
    fprintf(stderr, "Profile bio not updated\n");
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Refresh token */
  rc = oauth2_client_refresh_token(cli, session_admin.refresh_token,
                                   &session_renewed);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Refresh token failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* List users as Admin */
  rc = oauth2_client_list_all_users(cli, &session_admin, &users_list, &count);
  if (rc != OAUTH2_APP_OK || count < 2) {
    fprintf(stderr, "Admin list all users failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Login as standard user */
  rc = oauth2_client_login_password(cli, "online_charlie", "password",
                                    &session_user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Standard user login failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* List users as standard user (must fail with PERMISSION_DENIED) */
  rc = oauth2_client_list_all_users(cli, &session_user, &users_list, &count);
  if (rc != OAUTH2_APP_ERROR_PERMISSION_DENIED) {
    fprintf(stderr, "Expected PERMISSION_DENIED for standard user\n");
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Simulate network failure */
  rc = oauth2_client_set_network_failure(cli, 1);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Set network failure failed: %d\n", (int)rc);
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = oauth2_client_login_password(cli, "online_admin", "password",
                                    &session_admin);
  if (rc != OAUTH2_APP_ERROR_NETWORK) {
    fprintf(stderr, "Expected NETWORK error on simulated failure\n");
    rc = oauth2_client_destroy(cli);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = oauth2_client_destroy(cli);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  return OAUTH2_APP_OK;
}

int main(void) {
  enum oauth2_app_error rc;

  printf("Running test_oauth2_client...\n");
  rc = test_client_flow();
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "test_oauth2_client failed with code %d\n", (int)rc);
    return 1;
  }

  printf("test_oauth2_client passed successfully!\n");
  return 0;
}
