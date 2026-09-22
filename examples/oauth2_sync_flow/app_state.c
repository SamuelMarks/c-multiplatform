/**
 * @file app_state.c
 * @brief Application state and synchronization coordinator implementation.
 */

/* clang-format off */
#include "app_state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Safely retrieves an environment variable across compilers without
 * deprecation warnings.
 */
static enum oauth2_app_error get_env_var(const char *name, char *out_buf,
                                         size_t buf_size) {
#if defined(_MSC_VER)
  char *env_val;
  size_t env_len;
  errno_t err;

  if (name == NULL || out_buf == NULL || buf_size == 0) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  out_buf[0] = '\0';
  env_val = NULL;
  env_len = 0;
  err = _dupenv_s(&env_val, &env_len, name);
  if (err == 0 && env_val != NULL) {
    strncpy_s(out_buf, buf_size, env_val, _TRUNCATE);
    free(env_val);
    return OAUTH2_APP_OK;
  }
  return OAUTH2_APP_ERROR_NOT_FOUND;
#else
  const char *val;

  if (name == NULL || out_buf == NULL || buf_size == 0) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  out_buf[0] = '\0';
  val = getenv(name);
  if (val != NULL && val[0] != '\0') {
    strncpy(out_buf, val, buf_size - 1);
    out_buf[buf_size - 1] = '\0';
    return OAUTH2_APP_OK;
  }
  return OAUTH2_APP_ERROR_NOT_FOUND;
#endif
}

enum oauth2_app_error app_state_init(struct app_state **out_state) {
  struct app_state *state;
  char env_url_buf[256];
  enum oauth2_app_error rc;

  if (out_state == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  state = (struct app_state *)malloc(sizeof(struct app_state));
  if (state == NULL) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  memset(state, 0, sizeof(struct app_state));
  state->mode = OAUTH2_MODE_OFFLINE;
  state->is_authenticated = 0;

  rc = oauth2_user_init(&state->current_user);
  if (rc != OAUTH2_APP_OK) {
    free(state);
    return rc;
  }

  rc = oauth2_session_init(&state->current_session);
  if (rc != OAUTH2_APP_OK) {
    free(state);
    return rc;
  }

  rc = oauth2_offline_store_init(&state->offline_store);
  if (rc != OAUTH2_APP_OK) {
    free(state);
    return rc;
  }

  rc = oauth2_offline_store_seed_defaults(state->offline_store);
  if (rc != OAUTH2_APP_OK) {
    enum oauth2_app_error clean_rc =
        oauth2_offline_store_destroy(state->offline_store);
    if (clean_rc != OAUTH2_APP_OK) {
      /* Keep original error */
    }
    free(state);
    return rc;
  }

  env_url_buf[0] = '\0';
  rc = get_env_var("OAUTH2_BACKEND_URL", env_url_buf, sizeof(env_url_buf));
  if (rc != OAUTH2_APP_OK) {
    rc = get_env_var("BACKEND_URL", env_url_buf, sizeof(env_url_buf));
  }

  rc = oauth2_client_init(env_url_buf[0] != '\0' ? env_url_buf
                                                 : "http://127.0.0.1:8080",
                          &state->online_client);
  if (rc != OAUTH2_APP_OK) {
    enum oauth2_app_error clean_rc =
        oauth2_offline_store_destroy(state->offline_store);
    if (clean_rc != OAUTH2_APP_OK) {
      /* Keep original error */
    }
    free(state);
    return rc;
  }

  *out_state = state;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error app_state_destroy(struct app_state *state) {
  enum oauth2_app_error rc;
  enum oauth2_app_error ret = OAUTH2_APP_OK;

  if (state == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (state->offline_store != NULL) {
    rc = oauth2_offline_store_destroy(state->offline_store);
    if (rc != OAUTH2_APP_OK && ret == OAUTH2_APP_OK) {
      ret = rc;
    }
    state->offline_store = NULL;
  }

  if (state->online_client != NULL) {
    rc = oauth2_client_destroy(state->online_client);
    if (rc != OAUTH2_APP_OK && ret == OAUTH2_APP_OK) {
      ret = rc;
    }
    state->online_client = NULL;
  }

  free(state);
  return ret;
}

enum oauth2_app_error app_state_set_mode(struct app_state *state,
                                         enum oauth2_app_mode new_mode) {
  if (state == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  state->mode = new_mode;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error app_state_set_backend_url(struct app_state *state,
                                                const char *url) {
  if (state == NULL || url == NULL || state->online_client == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  return oauth2_client_set_base_url(state->online_client, url);
}

enum oauth2_app_error app_state_get_backend_url(const struct app_state *state,
                                                const char **out_url) {
  if (state == NULL || out_url == NULL || state->online_client == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  return oauth2_client_get_base_url(state->online_client, out_url);
}

enum oauth2_app_error app_state_login(struct app_state *state,
                                      const char *username,
                                      const char *password) {
  enum oauth2_app_error rc;

  if (state == NULL || username == NULL || password == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  state->last_error[0] = '\0';

  if (state->mode == OAUTH2_MODE_OFFLINE) {
    struct oauth2_user u;
    rc = oauth2_offline_store_get_user_by_username(state->offline_store,
                                                   username, &u);
    if (rc != OAUTH2_APP_OK) {
      rc = oauth2_safe_strcpy(state->last_error, sizeof(state->last_error),
                              "Offline user not found");
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return OAUTH2_APP_ERROR_AUTH_FAILED;
    }

    /* Offline password validation (allows standard demo password or username as
     * pass) */
    if (strcmp(password, "password") != 0 && strcmp(password, username) != 0) {
      rc = oauth2_safe_strcpy(state->last_error, sizeof(state->last_error),
                              "Invalid offline password");
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return OAUTH2_APP_ERROR_AUTH_FAILED;
    }

    state->current_user = u;
    state->is_authenticated = 1;
    rc = oauth2_session_init(&state->current_session);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_safe_strcpy(state->current_session.user_id,
                            sizeof(state->current_session.user_id), u.id);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_OK;
  } else {
    rc = oauth2_client_login_password(state->online_client, username, password,
                                      &state->current_session);
    if (rc != OAUTH2_APP_OK) {
      rc = oauth2_safe_strcpy(state->last_error, sizeof(state->last_error),
                              "Online OAuth2 login failed");
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    rc = oauth2_client_get_current_user(
        state->online_client, &state->current_session, &state->current_user);
    if (rc != OAUTH2_APP_OK) {
      rc = oauth2_safe_strcpy(state->last_error, sizeof(state->last_error),
                              "Failed to retrieve user profile from server");
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      return rc;
    }

    state->is_authenticated = 1;
    return OAUTH2_APP_OK;
  }
}

enum oauth2_app_error app_state_logout(struct app_state *state) {
  enum oauth2_app_error rc;

  if (state == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  state->is_authenticated = 0;
  rc = oauth2_user_init(&state->current_user);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  rc = oauth2_session_init(&state->current_session);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  state->last_error[0] = '\0';
  return OAUTH2_APP_OK;
}

enum oauth2_app_error app_state_update_about_me(struct app_state *state,
                                                const char *new_about_me) {
  enum oauth2_app_error rc;

  if (state == NULL || new_about_me == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (!state->is_authenticated) {
    return OAUTH2_APP_ERROR_AUTH_FAILED;
  }

  if (state->mode == OAUTH2_MODE_OFFLINE) {
    rc = oauth2_offline_store_update_about_me(
        state->offline_store, state->current_user.id, new_about_me);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }

    rc = oauth2_offline_store_get_user_by_id(
        state->offline_store, state->current_user.id, &state->current_user);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }

    return OAUTH2_APP_OK;
  } else {
    rc = oauth2_client_update_profile(state->online_client,
                                      &state->current_session, new_about_me,
                                      &state->current_user);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }

    return OAUTH2_APP_OK;
  }
}

enum oauth2_app_error app_state_sync_dirty_records(struct app_state *state,
                                                   size_t *out_synced_count) {
  struct oauth2_user *users_list;
  size_t count;
  size_t synced = 0;
  size_t i;
  enum oauth2_app_error rc;

  if (state == NULL || out_synced_count == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (state->mode != OAUTH2_MODE_ONLINE) {
    *out_synced_count = 0;
    return OAUTH2_APP_OK;
  }

  rc = oauth2_offline_store_list_users(state->offline_store, &users_list,
                                       &count);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  for (i = 0; i < count; ++i) {
    if (users_list[i].sync_status == OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY) {
      /* In online mode, push the modified bio to remote server */
      rc = oauth2_client_upsert_user(state->online_client, &users_list[i]);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      users_list[i].sync_status = OAUTH2_SYNC_STATUS_SYNCED;
      if (strcmp(state->current_user.id, users_list[i].id) == 0) {
        state->current_user.sync_status = OAUTH2_SYNC_STATUS_SYNCED;
      }
      synced++;
    }
  }

  *out_synced_count = synced;
  return OAUTH2_APP_OK;
}
