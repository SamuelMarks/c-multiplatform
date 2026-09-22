/**
 * @file offline_store.c
 * @brief Offline user repository and local cache persistence implementation.
 */

/* clang-format off */
#include "offline_store.h"
#include "parson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* clang-format on */

enum oauth2_app_error
oauth2_offline_store_init(struct oauth2_offline_store **out_store) {
  struct oauth2_offline_store *store;

  if (out_store == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  store = (struct oauth2_offline_store *)malloc(
      sizeof(struct oauth2_offline_store));
  if (store == NULL) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  memset(store, 0, sizeof(struct oauth2_offline_store));
  store->count = 0;

  *out_store = store;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_destroy(struct oauth2_offline_store *store) {
  if (store == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  free(store);
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_upsert_user(struct oauth2_offline_store *store,
                                 const struct oauth2_user *user) {
  size_t i;
  enum oauth2_app_error rc;

  if (store == NULL || user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  if (user->id[0] == '\0' || user->username[0] == '\0') {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Check for existing user by id or username */
  for (i = 0; i < store->count; ++i) {
    if (strcmp(store->users[i].id, user->id) == 0 ||
        strcmp(store->users[i].username, user->username) == 0) {
      rc = oauth2_safe_strcpy(store->users[i].id, sizeof(store->users[i].id),
                              user->id);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(store->users[i].username,
                              sizeof(store->users[i].username), user->username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(store->users[i].about_me,
                              sizeof(store->users[i].about_me), user->about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      store->users[i].role = user->role;
      store->users[i].sync_status = user->sync_status;
      store->users[i].updated_at = user->updated_at;
      return OAUTH2_APP_OK;
    }
  }

  if (store->count >= OAUTH2_MAX_STORED_USERS) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  rc = oauth2_safe_strcpy(store->users[store->count].id,
                          sizeof(store->users[store->count].id), user->id);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(store->users[store->count].username,
                          sizeof(store->users[store->count].username),
                          user->username);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(store->users[store->count].about_me,
                          sizeof(store->users[store->count].about_me),
                          user->about_me);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  store->users[store->count].role = user->role;
  store->users[store->count].sync_status = user->sync_status;
  store->users[store->count].updated_at = user->updated_at;
  store->count++;

  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_seed_defaults(struct oauth2_offline_store *store) {
  struct oauth2_user u_admin;
  struct oauth2_user u_alice;
  struct oauth2_user u_bob;
  enum oauth2_app_error rc;

  if (store == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  rc = oauth2_user_init(&u_admin);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_admin.id, sizeof(u_admin.id), "user_admin_01");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_admin.username, sizeof(u_admin.username), "admin");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(
      u_admin.about_me, sizeof(u_admin.about_me),
      "Core platform administrator with full system permissions.");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  u_admin.role = OAUTH2_ROLE_ADMIN;
  u_admin.sync_status = OAUTH2_SYNC_STATUS_SYNCED;
  u_admin.updated_at = (oauth2_uint64_t)time(NULL);

  rc = oauth2_offline_store_upsert_user(store, &u_admin);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  rc = oauth2_user_init(&u_alice);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_alice.id, sizeof(u_alice.id), "user_alice_02");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_alice.username, sizeof(u_alice.username),
                          "offline_alice");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc =
      oauth2_safe_strcpy(u_alice.about_me, sizeof(u_alice.about_me),
                         "Field operator operating in an air-gapped facility.");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  u_alice.role = OAUTH2_ROLE_USER;
  u_alice.sync_status = OAUTH2_SYNC_STATUS_LOCAL_ONLY;
  u_alice.updated_at = (oauth2_uint64_t)time(NULL);

  rc = oauth2_offline_store_upsert_user(store, &u_alice);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  rc = oauth2_user_init(&u_bob);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_bob.id, sizeof(u_bob.id), "user_bob_03");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc =
      oauth2_safe_strcpy(u_bob.username, sizeof(u_bob.username), "offline_bob");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_safe_strcpy(u_bob.about_me, sizeof(u_bob.about_me),
                          "Remote surveyor collecting telemetry in the field.");
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  u_bob.role = OAUTH2_ROLE_USER;
  u_bob.sync_status = OAUTH2_SYNC_STATUS_LOCAL_ONLY;
  u_bob.updated_at = (oauth2_uint64_t)time(NULL);

  rc = oauth2_offline_store_upsert_user(store, &u_bob);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_get_user_by_id(const struct oauth2_offline_store *store,
                                    const char *user_id,
                                    struct oauth2_user *out_user) {
  size_t i;
  enum oauth2_app_error rc;

  if (store == NULL || user_id == NULL || out_user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  for (i = 0; i < store->count; ++i) {
    if (strcmp(store->users[i].id, user_id) == 0) {
      rc = oauth2_safe_strcpy(out_user->id, sizeof(out_user->id),
                              store->users[i].id);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->username, sizeof(out_user->username),
                              store->users[i].username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->about_me, sizeof(out_user->about_me),
                              store->users[i].about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      out_user->role = store->users[i].role;
      out_user->sync_status = store->users[i].sync_status;
      out_user->updated_at = store->users[i].updated_at;
      return OAUTH2_APP_OK;
    }
  }

  return OAUTH2_APP_ERROR_NOT_FOUND;
}

enum oauth2_app_error oauth2_offline_store_get_user_by_username(
    const struct oauth2_offline_store *store, const char *username,
    struct oauth2_user *out_user) {
  size_t i;
  enum oauth2_app_error rc;

  if (store == NULL || username == NULL || out_user == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  for (i = 0; i < store->count; ++i) {
    if (strcmp(store->users[i].username, username) == 0) {
      rc = oauth2_safe_strcpy(out_user->id, sizeof(out_user->id),
                              store->users[i].id);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->username, sizeof(out_user->username),
                              store->users[i].username);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      rc = oauth2_safe_strcpy(out_user->about_me, sizeof(out_user->about_me),
                              store->users[i].about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      out_user->role = store->users[i].role;
      out_user->sync_status = store->users[i].sync_status;
      out_user->updated_at = store->users[i].updated_at;
      return OAUTH2_APP_OK;
    }
  }

  return OAUTH2_APP_ERROR_NOT_FOUND;
}

enum oauth2_app_error
oauth2_offline_store_list_users(const struct oauth2_offline_store *store,
                                struct oauth2_user **out_users,
                                size_t *out_count) {
  if (store == NULL || out_users == NULL || out_count == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  *out_users = (struct oauth2_user *)store->users;
  *out_count = store->count;
  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_update_about_me(struct oauth2_offline_store *store,
                                     const char *user_id,
                                     const char *new_about_me) {
  size_t i;
  enum oauth2_app_error rc;

  if (store == NULL || user_id == NULL || new_about_me == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  for (i = 0; i < store->count; ++i) {
    if (strcmp(store->users[i].id, user_id) == 0) {
      rc = oauth2_safe_strcpy(store->users[i].about_me,
                              sizeof(store->users[i].about_me), new_about_me);
      if (rc != OAUTH2_APP_OK) {
        return rc;
      }
      /* Mark account as modified locally for sync */
      store->users[i].sync_status = OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY;
      store->users[i].updated_at = (oauth2_uint64_t)time(NULL);
      return OAUTH2_APP_OK;
    }
  }

  return OAUTH2_APP_ERROR_NOT_FOUND;
}

enum oauth2_app_error
oauth2_offline_store_save_to_file(const struct oauth2_offline_store *store,
                                  const char *file_path) {
  JSON_Value *root_val;
  JSON_Array *arr;
  size_t i;
  JSON_Status status;

  if (store == NULL || file_path == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  root_val = json_value_init_array();
  if (root_val == NULL) {
    return OAUTH2_APP_ERROR_MEMORY;
  }

  arr = json_value_get_array(root_val);
  for (i = 0; i < store->count; ++i) {
    JSON_Value *user_val;
    JSON_Object *user_obj;

    user_val = json_value_init_object();
    if (user_val == NULL) {
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_MEMORY;
    }

    user_obj = json_value_get_object(user_val);
    status = json_object_set_string(user_obj, "id", store->users[i].id);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
    status =
        json_object_set_string(user_obj, "username", store->users[i].username);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
    status =
        json_object_set_string(user_obj, "about_me", store->users[i].about_me);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
    status =
        json_object_set_number(user_obj, "role", (double)store->users[i].role);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
    status = json_object_set_number(user_obj, "sync_status",
                                    (double)store->users[i].sync_status);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
    status = json_object_set_number(user_obj, "updated_at",
                                    (double)store->users[i].updated_at);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }

    status = json_array_append_value(arr, user_val);
    if (status != JSONSuccess) {
      json_value_free(user_val);
      json_value_free(root_val);
      return OAUTH2_APP_ERROR_STORAGE_FAILED;
    }
  }

  status = json_serialize_to_file_pretty(root_val, file_path);
  json_value_free(root_val);

  if (status != JSONSuccess) {
    return OAUTH2_APP_ERROR_STORAGE_FAILED;
  }

  return OAUTH2_APP_OK;
}

enum oauth2_app_error
oauth2_offline_store_load_from_file(struct oauth2_offline_store *store,
                                    const char *file_path) {
  JSON_Value *root_val;
  JSON_Array *arr;
  size_t count;
  size_t i;

  if (store == NULL || file_path == NULL) {
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  root_val = json_parse_file_with_comments(file_path);
  if (root_val == NULL) {
    return OAUTH2_APP_ERROR_STORAGE_FAILED;
  }

  arr = json_value_get_array(root_val);
  if (arr == NULL) {
    json_value_free(root_val);
    return OAUTH2_APP_ERROR_PARSE_FAILED;
  }

  count = json_array_get_count(arr);
  store->count = 0;

  for (i = 0; i < count && i < OAUTH2_MAX_STORED_USERS; ++i) {
    JSON_Object *user_obj;
    const char *id_str;
    const char *username_str;
    const char *about_me_str;
    struct oauth2_user u;
    enum oauth2_app_error rc;

    user_obj = json_array_get_object(arr, i);
    if (user_obj == NULL) {
      continue;
    }

    id_str = json_object_get_string(user_obj, "id");
    username_str = json_object_get_string(user_obj, "username");
    about_me_str = json_object_get_string(user_obj, "about_me");

    if (id_str == NULL || username_str == NULL) {
      continue;
    }

    rc = oauth2_user_init(&u);
    if (rc != OAUTH2_APP_OK) {
      json_value_free(root_val);
      return rc;
    }

    rc = oauth2_safe_strcpy(u.id, sizeof(u.id), id_str);
    if (rc != OAUTH2_APP_OK) {
      json_value_free(root_val);
      return rc;
    }

    rc = oauth2_safe_strcpy(u.username, sizeof(u.username), username_str);
    if (rc != OAUTH2_APP_OK) {
      json_value_free(root_val);
      return rc;
    }

    if (about_me_str != NULL) {
      rc = oauth2_safe_strcpy(u.about_me, sizeof(u.about_me), about_me_str);
      if (rc != OAUTH2_APP_OK) {
        json_value_free(root_val);
        return rc;
      }
    }

    u.role =
        (enum oauth2_user_role)(int)json_object_get_number(user_obj, "role");
    u.sync_status = (enum oauth2_sync_status)(int)json_object_get_number(
        user_obj, "sync_status");
    u.updated_at =
        (oauth2_uint64_t)json_object_get_number(user_obj, "updated_at");

    rc = oauth2_offline_store_upsert_user(store, &u);
    if (rc != OAUTH2_APP_OK) {
      json_value_free(root_val);
      return rc;
    }
  }

  json_value_free(root_val);
  return OAUTH2_APP_OK;
}
