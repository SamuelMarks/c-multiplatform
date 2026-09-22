/**
 * @file oauth2_client.c
 * @brief Implementation of OAuth2 REST client for c-rest-framework integration.
 */

/* clang-format off */
#include "oauth2_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* clang-format on */

enum oauth2_app_error oauth2_client_init(const char *base_url,
                                         struct oauth2_client **out_client) {
  struct oauth2_client *cli;
  enum oauth2_app_error rc;

  if (out_client == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  cli = (struct oauth2_client *)malloc(sizeof(struct oauth2_client));
  if (cli == NULL) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  memset(cli, 0, sizeof(struct oauth2_client));
  if (base_url != NULL) {
    rc = oauth2_safe_strcpy(cli->base_url, sizeof(cli->base_url), base_url);
    if (rc != OAUTH2_APP_OK) {
      free(cli);
      return rc;
    }
  } else {
    rc = oauth2_safe_strcpy(cli->base_url, sizeof(cli->base_url),
                            "http://127.0.0.1:8080");
    if (rc != OAUTH2_APP_OK) {
      free(cli);
      return rc;
    }
  }

  cli->simulate_network_failure = 0;

  /* Seed mock c-rest-framework server database */
  rc = oauth2_user_init(&cli->server_users[0]);
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc = oauth2_safe_strcpy(cli->server_users[0].id,
                          sizeof(cli->server_users[0].id), "srv_user_admin");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc =
      oauth2_safe_strcpy(cli->server_users[0].username,
                         sizeof(cli->server_users[0].username), "online_admin");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc = oauth2_safe_strcpy(
      cli->server_users[0].about_me, sizeof(cli->server_users[0].about_me),
      "Online production administrator for c-rest-framework cluster.");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  cli->server_users[0].role = OAUTH2_ROLE_ADMIN;
  cli->server_users[0].sync_status = OAUTH2_SYNC_STATUS_SYNCED;
  cli->server_users[0].updated_at = (oauth2_uint64_t)time(NULL);

  rc = oauth2_user_init(&cli->server_users[1]);
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc = oauth2_safe_strcpy(cli->server_users[1].id,
                          sizeof(cli->server_users[1].id), "srv_user_charlie");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc = oauth2_safe_strcpy(cli->server_users[1].username,
                          sizeof(cli->server_users[1].username),
                          "online_charlie");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  rc = oauth2_safe_strcpy(cli->server_users[1].about_me,
                          sizeof(cli->server_users[1].about_me),
                          "Cloud platform engineer maintaining microservices.");
  if (rc != OAUTH2_APP_OK) {
    free(cli);
    return rc;
  }
  cli->server_users[1].role = OAUTH2_ROLE_USER;
  cli->server_users[1].sync_status = OAUTH2_SYNC_STATUS_SYNCED;
  cli->server_users[1].updated_at = (oauth2_uint64_t)time(NULL);

  cli->server_user_count = 2;

  *out_client = cli;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error oauth2_client_destroy(struct oauth2_client *client) {
  if (client == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  free(client);
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_client_set_network_failure(struct oauth2_client *client, int enable) {
  if (client == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  client->simulate_network_failure = enable;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error oauth2_client_set_base_url(struct oauth2_client *client,
                                                 const char *new_url) {
  enum oauth2_app_error rc;

  if (client == NULL || new_url == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  rc = oauth2_safe_strcpy(client->base_url, sizeof(client->base_url), new_url);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_client_get_base_url(const struct oauth2_client *client,
                           const char **out_url) {
  if (client == NULL || out_url == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  *out_url = client->base_url;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_client_upsert_user(struct oauth2_client *client,
                          const struct oauth2_user *user) {
  size_t i;
  enum oauth2_app_error rc;

  if (client == NULL || user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  for (i = 0; i < client->server_user_count; ++i) {
    if (strcmp(client->server_users[i].id, user->id) == 0) {
      rc = oauth2_safe_strcpy(client->server_users[i].username,
                              sizeof(client->server_users[i].username),
                              user->username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(client->server_users[i].about_me,
                              sizeof(client->server_users[i].about_me),
                              user->about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      client->server_users[i].role = user->role;
      client->server_users[i].sync_status = OAUTH2_SYNC_STATUS_SYNCED;
      client->server_users[i].updated_at = (oauth2_uint64_t)time(NULL);
      return OAUTH2_APP_OK;
    }
  }

  if (client->server_user_count >= OAUTH2_CLIENT_MAX_USERS) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  i = client->server_user_count;
  rc = oauth2_user_init(&client->server_users[i]);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  rc = oauth2_safe_strcpy(client->server_users[i].id,
                          sizeof(client->server_users[i].id), user->id);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(client->server_users[i].username,
                          sizeof(client->server_users[i].username),
                          user->username);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(client->server_users[i].about_me,
                          sizeof(client->server_users[i].about_me),
                          user->about_me);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  client->server_users[i].role = user->role;
  client->server_users[i].sync_status = OAUTH2_SYNC_STATUS_SYNCED;
  client->server_users[i].updated_at = (oauth2_uint64_t)time(NULL);

  client->server_user_count++;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_client_login_password(struct oauth2_client *client, const char *username,
                             const char *password,
                             struct oauth2_session *out_session) {
  enum oauth2_app_error rc;

  if (client == NULL || username == NULL || password == NULL ||
      out_session == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  rc = oauth2_session_init(out_session);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  if (strcmp(username, "online_admin") == 0 &&
      strcmp(password, "password") == 0) {
    rc = oauth2_safe_strcpy(out_session->access_token,
                            sizeof(out_session->access_token),
                            "bearer_token_admin_live_xyz");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->refresh_token,
                            sizeof(out_session->refresh_token),
                            "refresh_token_admin_live_abc");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->token_type,
                            sizeof(out_session->token_type), "Bearer");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->user_id, sizeof(out_session->user_id),
                            "srv_user_admin");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    out_session->expires_in = 3600;
    return OAUTH2_APP_OK;
  } else if (strcmp(username, "online_charlie") == 0 &&
             strcmp(password, "password") == 0) {
    rc = oauth2_safe_strcpy(out_session->access_token,
                            sizeof(out_session->access_token),
                            "bearer_token_charlie_live_xyz");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->refresh_token,
                            sizeof(out_session->refresh_token),
                            "refresh_token_charlie_live_abc");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->token_type,
                            sizeof(out_session->token_type), "Bearer");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(out_session->user_id, sizeof(out_session->user_id),
                            "srv_user_charlie");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    out_session->expires_in = 3600;
    return OAUTH2_APP_OK;
  }

  return OAUTH2_APP_ERROR_AUTH_FAILED;
}

enum oauth2_app_error
oauth2_client_refresh_token(struct oauth2_client *client,
                            const char *refresh_token,
                            struct oauth2_session *out_session) {
  enum oauth2_app_error rc;

  if (client == NULL || refresh_token == NULL || out_session == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  if (strcmp(refresh_token, "refresh_token_admin_live_abc") == 0) {
    rc = oauth2_safe_strcpy(out_session->access_token,
                            sizeof(out_session->access_token),
                            "renewed_token_admin_live_123");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    out_session->expires_in = 3600;
    return OAUTH2_APP_OK;
  } else if (strcmp(refresh_token, "refresh_token_charlie_live_abc") == 0) {
    rc = oauth2_safe_strcpy(out_session->access_token,
                            sizeof(out_session->access_token),
                            "renewed_token_charlie_live_123");
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    out_session->expires_in = 3600;
    return OAUTH2_APP_OK;
  }

  return OAUTH2_APP_ERROR_AUTH_FAILED;
}

enum oauth2_app_error
oauth2_client_get_current_user(struct oauth2_client *client,
                               const struct oauth2_session *session,
                               struct oauth2_user *out_user) {
  size_t i;
  enum oauth2_app_error rc;

  if (client == NULL || session == NULL || out_user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  for (i = 0; i < client->server_user_count; ++i) {
    if (strcmp(client->server_users[i].id, session->user_id) == 0) {
      rc = oauth2_safe_strcpy(out_user->id, sizeof(out_user->id),
                              client->server_users[i].id);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->username, sizeof(out_user->username),
                              client->server_users[i].username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->about_me, sizeof(out_user->about_me),
                              client->server_users[i].about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      out_user->role = client->server_users[i].role;
      out_user->sync_status = client->server_users[i].sync_status;
      out_user->updated_at = client->server_users[i].updated_at;
      return OAUTH2_APP_OK;
    }
  }

  return OAUTH2_APP_ERROR_NOT_FOUND;
}

enum oauth2_app_error oauth2_client_update_profile(
    struct oauth2_client *client, const struct oauth2_session *session,
    const char *about_me, struct oauth2_user *out_user) {
  size_t i;
  enum oauth2_app_error rc;

  if (client == NULL || session == NULL || about_me == NULL ||
      out_user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  for (i = 0; i < client->server_user_count; ++i) {
    if (strcmp(client->server_users[i].id, session->user_id) == 0) {
      rc = oauth2_safe_strcpy(client->server_users[i].about_me,
                              sizeof(client->server_users[i].about_me),
                              about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      client->server_users[i].updated_at = (oauth2_uint64_t)time(NULL);

      rc = oauth2_safe_strcpy(out_user->id, sizeof(out_user->id),
                              client->server_users[i].id);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->username, sizeof(out_user->username),
                              client->server_users[i].username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->about_me, sizeof(out_user->about_me),
                              client->server_users[i].about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      out_user->role = client->server_users[i].role;
      out_user->sync_status = client->server_users[i].sync_status;
      out_user->updated_at = client->server_users[i].updated_at;
      return OAUTH2_APP_OK;
    }
  }

  return OAUTH2_APP_ERROR_NOT_FOUND;
}

enum oauth2_app_error oauth2_client_list_all_users(
    struct oauth2_client *client, const struct oauth2_session *session,
    struct oauth2_user **out_users, size_t *out_count) {
  struct oauth2_user me;
  enum oauth2_app_error rc;

  if (client == NULL || session == NULL || out_users == NULL ||
      out_count == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (client->simulate_network_failure) {
    return OAUTH2_APP_ERROR_NETWORK;
  }

  rc = oauth2_client_get_current_user(client, session, &me);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  if (me.role != OAUTH2_ROLE_ADMIN) {
    return OAUTH2_APP_ERROR_PERMISSION_DENIED;
  }

  *out_users = (struct oauth2_user *)client->server_users;
  *out_count = client->server_user_count;
  return OAUTH2_APP_OK;
}
