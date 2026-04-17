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

int cmp_credential_manager_create(cmp_credential_manager_t **out_manager) {
  int rc = CMP_SUCCESS;
  cmp_credential_manager_t *manager = NULL;

  if (!out_manager) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_create: Invalid argument\n");
    return rc;
  }

  manager =
      (cmp_credential_manager_t *)malloc(sizeof(cmp_credential_manager_t));
  if (!manager) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_credential_manager_create: Out of memory\n");
    return rc;
  }

  manager->dummy = 0;
  *out_manager = manager;
  return rc;
}

int cmp_credential_manager_destroy(cmp_credential_manager_t *manager) {
  int rc = CMP_SUCCESS;

  if (!manager) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_credential_manager_destroy: Invalid argument\n");
    return rc;
  }

  free(manager);

  /* Clean up mock static state for tests */
  if (g_mock_secret) {
    memset(g_mock_secret, 0, strlen(g_mock_secret));
    free(g_mock_secret);
    g_mock_secret = NULL;
  }
  if (g_mock_service) {
    free(g_mock_service);
    g_mock_service = NULL;
  }
  if (g_mock_account) {
    free(g_mock_account);
    g_mock_account = NULL;
  }

  return rc;
}

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
    free(g_mock_secret);
    g_mock_secret = NULL;
  }
  if (g_mock_service) {
    free(g_mock_service);
    g_mock_service = NULL;
  }
  if (g_mock_account) {
    free(g_mock_account);
    g_mock_account = NULL;
  }

  len = strlen(secret);
  g_mock_secret = (char *)malloc(len + 1);
  if (!g_mock_secret) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating secret\n");
    return rc;
  }
  strncpy(g_mock_secret, secret, len);
  g_mock_secret[len] = '\0';

  len = strlen(service);
  g_mock_service = (char *)malloc(len + 1);
  if (!g_mock_service) {
    free(g_mock_secret);
    g_mock_secret = NULL;
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating service\n");
    return rc;
  }
  strncpy(g_mock_service, service, len);
  g_mock_service[len] = '\0';

  len = strlen(account);
  g_mock_account = (char *)malloc(len + 1);
  if (!g_mock_account) {
    free(g_mock_secret);
    free(g_mock_service);
    g_mock_secret = NULL;
    g_mock_service = NULL;
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_credential_manager_set_secret: Out of memory "
              "allocating account\n");
    return rc;
  }
  strncpy(g_mock_account, account, len);
  g_mock_account[len] = '\0';

  return rc;
}

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
    *out_secret = (char *)malloc(len + 1);
    if (!*out_secret) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("Error in cmp_credential_manager_get_secret: Out of memory\n");
      return rc;
    }
    strncpy(*out_secret, g_mock_secret, len);
    (*out_secret)[len] = '\0';
    return rc;
  }

  *out_secret = NULL;
  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_credential_manager_get_secret: Secret not found\n");
  return rc; /* Not found */
}

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
      free(g_mock_secret);
      g_mock_secret = NULL;
    }
    if (g_mock_service) {
      free(g_mock_service);
      g_mock_service = NULL;
    }
    if (g_mock_account) {
      free(g_mock_account);
      g_mock_account = NULL;
    }
    return rc;
  }

  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG(
      "Error in cmp_credential_manager_delete_secret: Secret not found\n");
  return rc; /* Not found */
}

int cmp_credential_manager_free_secret(char *secret) {
  int rc = CMP_SUCCESS;

  if (secret) {
    memset(secret, 0, strlen(secret));
    free(secret);
  }
  return rc;
}
