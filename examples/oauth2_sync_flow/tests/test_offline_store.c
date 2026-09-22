/**
 * @file test_offline_store.c
 * @brief Unit tests for offline user repository and file persistence.
 */

/* clang-format off */
#include "../offline_store.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

static enum oauth2_app_error test_store_lifecycle(void) {
  struct oauth2_offline_store *store;
  struct oauth2_offline_store *loaded_store;
  struct oauth2_user user;
  struct oauth2_user *users_list;
  size_t count;
  enum oauth2_app_error rc;
  const char *tmp_file = "test_offline_store_dump.json";

  /* Test NULL checks */
  rc = oauth2_offline_store_init(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL out_store\n");
    return rc;
  }

  rc = oauth2_offline_store_destroy(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL store destroy\n");
    return rc;
  }

  rc = oauth2_offline_store_seed_defaults(NULL);
  if (rc != OAUTH2_APP_ERROR_INVALID_PARAM) {
    fprintf(stderr, "Expected INVALID_PARAM for NULL store seed\n");
    return rc;
  }

  /* Allocate and seed */
  rc = oauth2_offline_store_init(&store);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Store init failed: %d\n", (int)rc);
    return rc;
  }

  rc = oauth2_offline_store_seed_defaults(store);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Store seed failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Test query by ID */
  rc = oauth2_offline_store_get_user_by_id(store, "user_admin_01", &user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Get user admin failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (user.role != OAUTH2_ROLE_ADMIN) {
    fprintf(stderr, "Expected admin role\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test query non-existent ID */
  rc = oauth2_offline_store_get_user_by_id(store, "non_existent", &user);
  if (rc != OAUTH2_APP_ERROR_NOT_FOUND) {
    fprintf(stderr, "Expected NOT_FOUND for non-existent ID\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  /* Test query by username */
  rc = oauth2_offline_store_get_user_by_username(store, "offline_alice", &user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Get user alice failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (user.sync_status != OAUTH2_SYNC_STATUS_LOCAL_ONLY) {
    fprintf(stderr, "Expected LOCAL_ONLY sync status for alice\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test updating About Me */
  rc = oauth2_offline_store_update_about_me(store, "user_admin_01",
                                            "Updated Administrator Bio");
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Update about me failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = oauth2_offline_store_get_user_by_id(store, "user_admin_01", &user);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Fetch updated admin failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (strcmp(user.about_me, "Updated Administrator Bio") != 0) {
    fprintf(stderr, "About me bio was not updated\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }
  if (user.sync_status != OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY) {
    fprintf(stderr, "Expected MODIFIED_LOCALLY sync status after edit\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test list users */
  rc = oauth2_offline_store_list_users(store, &users_list, &count);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "List users failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (count < 3) {
    fprintf(stderr, "Expected at least 3 users in seeded store\n");
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  /* Test persistence save and load */
  rc = oauth2_offline_store_save_to_file(store, tmp_file);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Save to file failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = oauth2_offline_store_init(&loaded_store);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Loaded store init failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc = oauth2_offline_store_load_from_file(loaded_store, tmp_file);
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Load from file failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(loaded_store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }

  rc =
      oauth2_offline_store_get_user_by_id(loaded_store, "user_admin_01", &user);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "Fetch from loaded store failed: %d\n", (int)rc);
    rc = oauth2_offline_store_destroy(loaded_store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return rc;
  }
  if (strcmp(user.about_me, "Updated Administrator Bio") != 0) {
    fprintf(stderr, "Loaded bio does not match saved bio\n");
    rc = oauth2_offline_store_destroy(loaded_store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    rc = oauth2_offline_store_destroy(store);
    if (rc != OAUTH2_APP_OK) {
      return rc;
    }
    return OAUTH2_APP_ERROR_INVALID_PARAM;
  }

  remove(tmp_file);
  rc = oauth2_offline_store_destroy(loaded_store);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }
  rc = oauth2_offline_store_destroy(store);
  if (rc != OAUTH2_APP_OK) {
    return rc;
  }

  return OAUTH2_APP_OK;
}

int main(void) {
  enum oauth2_app_error rc;

  printf("Running test_offline_store...\n");
  rc = test_store_lifecycle();
  if (rc != OAUTH2_APP_OK) {
    fprintf(stderr, "test_offline_store failed with code %d\n", (int)rc);
    return 1;
  }

  printf("test_offline_store passed successfully!\n");
  return 0;
}
