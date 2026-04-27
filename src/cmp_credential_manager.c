/* clang-format off */
#include "cmp_credential_manager.h"
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_credential_manager {
  int dummy; /* C89 requires structs to have at least one member */
};

/* Mock implementation for cross-platform tests. Real integration would use
 * native APIs. */
static char *g_mock_secret = NULL;
static char *g_mock_service = NULL;
static char *g_mock_account = NULL;

/**
 * @brief cmp_credential_manager_create
 *
 * @param out_manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_create(cmp_credential_manager_t **out_manager) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_credential_manager_t *manager = NULL;

  if (out_manager == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_create: Invalid argument "
                  "(out_manager=NULL): %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_credential_manager_t), (void **)&manager);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_create: Out of memory: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  manager->dummy = 0;
  *out_manager = manager;
  cmp_log_debug("cmp_credential_manager_create: Successfully created "
                "credential manager context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_credential_manager_destroy
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_destroy(cmp_credential_manager_t *manager) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (manager == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_destroy: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  rc = CMP_FREE(manager);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_credential_manager_destroy: CMP_FREE failed\n");
  }

  /* Clean up mock static state for tests */
  if (g_mock_secret != NULL) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_credential_manager_destroy: CMP_FREE mock secret failed\n");
    }
    g_mock_secret = NULL;
  }
  if (g_mock_service != NULL) {
    rc = CMP_FREE(g_mock_service);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_credential_manager_destroy: CMP_FREE mock service failed\n");
    }
    g_mock_service = NULL;
  }
  if (g_mock_account != NULL) {
    rc = CMP_FREE(g_mock_account);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_credential_manager_destroy: CMP_FREE mock account failed\n");
    }
    g_mock_account = NULL;
  }

  cmp_log_debug("cmp_credential_manager_destroy: Successfully destroyed "
                "credential manager context\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_credential_manager_set_secret
 *
 * @param manager Parameter description.
 * @param service Parameter description.
 * @param account Parameter description.
 * @param secret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_set_secret(cmp_credential_manager_t *manager,
                                      const char *service, const char *account,
                                      const char *secret) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t len;

  if (manager == NULL || service == NULL || account == NULL || secret == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_set_secret: Invalid argument: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  /* Simple mock for tests */
  if (g_mock_secret != NULL) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug(
          "cmp_credential_manager_set_secret: CMP_FREE g_mock_secret failed\n");
    }
    g_mock_secret = NULL;
  }
  if (g_mock_service != NULL) {
    rc = CMP_FREE(g_mock_service);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE "
                    "g_mock_service failed\n");
    }
    g_mock_service = NULL;
  }
  if (g_mock_account != NULL) {
    rc = CMP_FREE(g_mock_account);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE "
                    "g_mock_account failed\n");
    }
    g_mock_account = NULL;
  }

  len = strlen(secret);
  rc = CMP_MALLOC(len + 1, (void **)&g_mock_secret);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_set_secret: Out of memory allocating "
                  "secret: %s\n",
                  err_str);
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_secret, len + 1, secret, len + 1) != 0) {
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_secret = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  memcpy(g_mock_secret, secret, len + 1);
#endif

  len = strlen(service);
  rc = CMP_MALLOC(len + 1, (void **)&g_mock_service);
  if (rc != CMP_SUCCESS) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_secret = NULL;
    cmp_log_debug("cmp_credential_manager_set_secret: Out of memory allocating "
                  "service\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_service, len + 1, service, len + 1) != 0) {
    rc = CMP_FREE(g_mock_service);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_service = NULL;
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_secret = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  memcpy(g_mock_service, service, len + 1);
#endif

  len = strlen(account);
  rc = CMP_MALLOC(len + 1, (void **)&g_mock_account);
  if (rc != CMP_SUCCESS) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    rc = CMP_FREE(g_mock_service);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_secret = NULL;
    g_mock_service = NULL;
    cmp_log_debug("cmp_credential_manager_set_secret: Out of memory allocating "
                  "account\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_account, len + 1, account, len + 1) != 0) {
    rc = CMP_FREE(g_mock_account);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_account = NULL;
    rc = CMP_FREE(g_mock_service);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_service = NULL;
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    rc = CMP_FREE(g_mock_secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_set_secret: CMP_FREE failed during "
                    "recovery\n");
    }
    g_mock_secret = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  memcpy(g_mock_account, account, len + 1);
#endif

  cmp_log_debug("cmp_credential_manager_set_secret: Successfully cached mocked "
                "credentials\n");
  return CMP_SUCCESS;
}

/**
 * @brief cmp_credential_manager_get_secret
 *
 * @param manager Parameter description.
 * @param service Parameter description.
 * @param account Parameter description.
 * @param out_secret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_get_secret(cmp_credential_manager_t *manager,
                                      const char *service, const char *account,
                                      char **out_secret) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t len;

  if (manager == NULL || service == NULL || account == NULL ||
      out_secret == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_get_secret: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (g_mock_service != NULL && strcmp(g_mock_service, service) == 0 &&
      g_mock_account != NULL && strcmp(g_mock_account, account) == 0 &&
      g_mock_secret != NULL) {
    len = strlen(g_mock_secret);
    rc = CMP_MALLOC(len + 1, (void **)out_secret);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug("cmp_credential_manager_get_secret: Out of memory\n");
      if (rc != 0) {
        return rc;
      }
      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(*out_secret, len + 1, g_mock_secret, len + 1) != 0) {
      rc = CMP_FREE(*out_secret);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_credential_manager_get_secret: CMP_FREE failed "
                      "during recovery\n");
      }
      *out_secret = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(*out_secret, g_mock_secret, len + 1);
#endif
    cmp_log_debug(
        "cmp_credential_manager_get_secret: Found mock secret successfully\n");
    return CMP_SUCCESS;
  }

  *out_secret = NULL;
  rc = CMP_ERROR_NOT_FOUND;
  err_rc = cmp_strerror(rc, &err_str);
  if (err_rc != CMP_SUCCESS) {
    err_str = "Unknown";
  }
  cmp_log_debug("cmp_credential_manager_get_secret: Secret not found\n");
  if (rc != 0) {
    return rc;
  }
  return rc; /* Not found */
}

/**
 * @brief cmp_credential_manager_delete_secret
 *
 * @param manager Parameter description.
 * @param service Parameter description.
 * @param account Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_delete_secret(cmp_credential_manager_t *manager,
                                         const char *service,
                                         const char *account) {
  int rc;
  rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (manager == NULL || service == NULL || account == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_credential_manager_delete_secret: Invalid argument\n");
    if (rc != 0) {
      return rc;
    }
    return rc;
  }

  if (g_mock_service != NULL && strcmp(g_mock_service, service) == 0 &&
      g_mock_account != NULL && strcmp(g_mock_account, account) == 0) {
    if (g_mock_secret != NULL) {
      memset(g_mock_secret, 0, strlen(g_mock_secret));
      rc = CMP_FREE(g_mock_secret);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_credential_manager_delete_secret: CMP_FREE secret failed\n");
      }
      g_mock_secret = NULL;
    }
    if (g_mock_service != NULL) {
      rc = CMP_FREE(g_mock_service);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_credential_manager_delete_secret: CMP_FREE service failed\n");
      }
      g_mock_service = NULL;
    }
    if (g_mock_account != NULL) {
      rc = CMP_FREE(g_mock_account);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug(
            "cmp_credential_manager_delete_secret: CMP_FREE account failed\n");
      }
      g_mock_account = NULL;
    }
    cmp_log_debug("cmp_credential_manager_delete_secret: Deleted mocked secret "
                  "successfully\n");
    return CMP_SUCCESS;
  }

  rc = CMP_ERROR_NOT_FOUND;
  err_rc = cmp_strerror(rc, &err_str);
  if (err_rc != CMP_SUCCESS) {
    err_str = "Unknown";
  }
  cmp_log_debug("cmp_credential_manager_delete_secret: Secret not found\n");
  if (rc != 0) {
    return rc;
  }
  return rc; /* Not found */
}

/**
 * @brief cmp_credential_manager_free_secret
 *
 * @param secret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_free_secret(char *secret) {
  int rc;
  rc = CMP_SUCCESS;

  if (secret != NULL) {
    memset(secret, 0, strlen(secret));
    rc = CMP_FREE(secret);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_credential_manager_free_secret: CMP_FREE failed\n");
    }
    cmp_log_debug("cmp_credential_manager_free_secret: Successfully wiped and "
                  "freed secret string\n");
  }
  return CMP_SUCCESS;
}
