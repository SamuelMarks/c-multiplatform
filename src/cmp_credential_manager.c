/* clang-format off */
#include "cmp_credential_manager.h"
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
  cmp_credential_manager_t *manager;

  if (!out_manager) {
    return -1;
  }

  manager =
      (cmp_credential_manager_t *)malloc(sizeof(cmp_credential_manager_t));
  if (!manager) {
    return -2;
  }

  manager->dummy = 0;
  *out_manager = manager;
  return 0;
}

int cmp_credential_manager_destroy(cmp_credential_manager_t *manager) {
  if (!manager) {
    return -1;
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

  return 0;
}

int cmp_credential_manager_set_secret(cmp_credential_manager_t *manager,
                                      const char *service, const char *account,
                                      const char *secret) {
  size_t len;

  if (!manager || !service || !account || !secret) {
    return -1;
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
  if (!g_mock_secret)
    return -2;
  strncpy(g_mock_secret, secret, len);
  g_mock_secret[len] = '\0';

  len = strlen(service);
  g_mock_service = (char *)malloc(len + 1);
  if (!g_mock_service) {
    free(g_mock_secret);
    g_mock_secret = NULL;
    return -2;
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
    return -2;
  }
  strncpy(g_mock_account, account, len);
  g_mock_account[len] = '\0';

  return 0;
}

int cmp_credential_manager_get_secret(cmp_credential_manager_t *manager,
                                      const char *service, const char *account,
                                      char **out_secret) {
  size_t len;

  if (!manager || !service || !account || !out_secret) {
    return -1;
  }

  if (g_mock_service && strcmp(g_mock_service, service) == 0 &&
      g_mock_account && strcmp(g_mock_account, account) == 0 && g_mock_secret) {
    len = strlen(g_mock_secret);
    *out_secret = (char *)malloc(len + 1);
    if (!*out_secret)
      return -2;
    strncpy(*out_secret, g_mock_secret, len);
    (*out_secret)[len] = '\0';
    return 0;
  }

  *out_secret = NULL;
  return -3; /* Not found */
}

int cmp_credential_manager_delete_secret(cmp_credential_manager_t *manager,
                                         const char *service,
                                         const char *account) {
  if (!manager || !service || !account) {
    return -1;
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
    return 0;
  }

  return -3; /* Not found */
}

int cmp_credential_manager_free_secret(char *secret) {
  if (secret) {
    memset(secret, 0, strlen(secret));
    free(secret);
  }
  return 0;
}
