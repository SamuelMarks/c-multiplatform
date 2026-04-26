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
  int rc = CMP_SUCCESS;
  cmp_credential_manager_t *manager = NULL;

  if (!out_manager) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_credential_manager_t), (void **)&manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_credential_manager_create: Out of memory\n");
    return rc;
  }

  manager->dummy = 0;
  *out_manager = manager;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_credential_manager_destroy
 *
 * @param manager Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_destroy(cmp_credential_manager_t *manager) {
  int rc = CMP_SUCCESS;

  if (!manager) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_destroy: Invalid argument\n");
    return rc;
  }

  rc = CMP_FREE(manager);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_credential_manager_destroy: CMP_FREE failed\n");
  }

  /* Clean up mock static state for tests */
  if (g_mock_secret) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    CMP_FREE(g_mock_secret);
    g_mock_secret = NULL;
  }
  if (g_mock_service) {
    CMP_FREE(g_mock_service);
    g_mock_service = NULL;
  }
  if (g_mock_account) {
    CMP_FREE(g_mock_account);
    g_mock_account = NULL;
  }

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
  int rc = CMP_SUCCESS;
  size_t len;

  if (!manager || !service || !account || !secret) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Invalid argument\n");
    return rc;
  }

  /* Simple mock for tests */
  if (g_mock_secret) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    CMP_FREE(g_mock_secret);
    g_mock_secret = NULL;
  }
  if (g_mock_service) {
    CMP_FREE(g_mock_service);
    g_mock_service = NULL;
  }
  if (g_mock_account) {
    CMP_FREE(g_mock_account);
    g_mock_account = NULL;
  }

  len = strlen(secret);
  rc = CMP_MALLOC(len + 1, (void **)&g_mock_secret);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating secret\n");
    return rc;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_secret, len + 1, secret, len + 1) != 0) {
    CMP_FREE(g_mock_secret);
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
    CMP_FREE(g_mock_secret);
    g_mock_secret = NULL;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating service\n");
    return rc;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_service, len + 1, service, len + 1) != 0) {
    CMP_FREE(g_mock_service);
    g_mock_service = NULL;
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    CMP_FREE(g_mock_secret);
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
    CMP_FREE(g_mock_secret);
    CMP_FREE(g_mock_service);
    g_mock_secret = NULL;
    g_mock_service = NULL;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating account\n");
    return rc;
  }
#if defined(_MSC_VER)
  if (memcpy_s(g_mock_account, len + 1, account, len + 1) != 0) {
    CMP_FREE(g_mock_account);
    g_mock_account = NULL;
    CMP_FREE(g_mock_service);
    g_mock_service = NULL;
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    CMP_FREE(g_mock_secret);
    g_mock_secret = NULL;
    return CMP_ERROR_GENERAL;
  }
#else
  memcpy(g_mock_account, account, len + 1);
#endif

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
  int rc = CMP_SUCCESS;
  size_t len;

  if (!manager || !service || !account || !out_secret) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_get_secret: Invalid argument\n");
    return rc;
  }

  if (g_mock_service && strcmp(g_mock_service, service) == 0 &&
      g_mock_account && strcmp(g_mock_account, account) == 0 && g_mock_secret) {
    len = strlen(g_mock_secret);
    rc = CMP_MALLOC(len + 1, (void **)out_secret);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_credential_manager_get_secret: Out of memory\n");
      return rc;
    }
#if defined(_MSC_VER)
    if (memcpy_s(*out_secret, len + 1, g_mock_secret, len + 1) != 0) {
      CMP_FREE(*out_secret);
      *out_secret = NULL;
      return CMP_ERROR_GENERAL;
    }
#else
    memcpy(*out_secret, g_mock_secret, len + 1);
#endif
    return CMP_SUCCESS;
  }

  *out_secret = NULL;
  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_credential_manager_get_secret: Secret not found\n");
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
  int rc = CMP_SUCCESS;

  if (!manager || !service || !account) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG(
        "Error in cmp_credential_manager_delete_secret: Invalid argument\n");
    return rc;
  }

  if (g_mock_service && strcmp(g_mock_service, service) == 0 &&
      g_mock_account && strcmp(g_mock_account, account) == 0) {
    if (g_mock_secret) {
      memset(g_mock_secret, 0, strlen(g_mock_secret));
      CMP_FREE(g_mock_secret);
      g_mock_secret = NULL;
    }
    if (g_mock_service) {
      CMP_FREE(g_mock_service);
      g_mock_service = NULL;
    }
    if (g_mock_account) {
      CMP_FREE(g_mock_account);
      g_mock_account = NULL;
    }
    return CMP_SUCCESS;
  }

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG(
      "Error in cmp_credential_manager_delete_secret: Secret not found\n");
  return rc; /* Not found */
}

/**
 * @brief cmp_credential_manager_free_secret
 *
 * @param secret Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_credential_manager_free_secret(char *secret) {
  int rc = CMP_SUCCESS;

  if (secret) {
    memset(secret, 0, strlen(secret));
    rc = CMP_FREE(secret);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_credential_manager_free_secret: CMP_FREE failed\n");
    }
  }
  return CMP_SUCCESS;
}