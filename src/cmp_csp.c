/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_csp_domain {
  char *domain;
} cmp_csp_domain_t;

struct cmp_csp {
  cmp_csp_domain_t *domains;
  size_t count;
  size_t capacity;
};

int cmp_csp_create(cmp_csp_t **out_csp) {
  int rc = CMP_SUCCESS;
  cmp_csp_t *csp = NULL;

  if (!out_csp) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_csp_create: Invalid argument (out_csp=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_csp_t), (void **)&csp);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_csp_create: Out of memory\n");
    return rc;
  }

  memset(csp, 0, sizeof(cmp_csp_t));
  *out_csp = csp;
  return rc;
}

int cmp_csp_destroy(cmp_csp_t *csp) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!csp) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_csp_destroy: Invalid argument (csp=NULL)\n");
    return rc;
  }

  if (csp->domains) {
    for (i = 0; i < csp->count; i++) {
      if (csp->domains[i].domain) {
        CMP_FREE(csp->domains[i].domain);
      }
    }
    CMP_FREE(csp->domains);
  }

  CMP_FREE(csp);
  return rc;
}

static int str_duplicate(const char *src, char **out_dst) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!src || !out_dst) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in str_duplicate: Invalid argument\n");
    return rc;
  }

  len = strlen(src);
  rc = CMP_MALLOC(len + 1, (void **)out_dst);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in str_duplicate: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_dst, len + 1, src);
#else
  strcpy(*out_dst, src);
#endif

  return rc;
}

int cmp_csp_add_domain(cmp_csp_t *csp, const char *domain) {
  int rc = CMP_SUCCESS;
  size_t new_cap;
  cmp_csp_domain_t *new_domains = NULL;

  if (!csp || !domain) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_csp_add_domain: Invalid argument\n");
    return rc;
  }

  if (csp->count >= csp->capacity) {
    new_cap = csp->capacity == 0 ? 8 : csp->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_csp_domain_t), (void **)&new_domains);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_csp_add_domain: Out of memory allocating domains\n");
      return rc;
    }

    if (csp->domains) {
      memcpy(new_domains, csp->domains, csp->count * sizeof(cmp_csp_domain_t));
      CMP_FREE(csp->domains);
    }
    csp->domains = new_domains;
    csp->capacity = new_cap;
  }

  rc = str_duplicate(domain, &csp->domains[csp->count].domain);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_csp_add_domain: str_duplicate failed\n");
    return rc;
  }

  csp->count++;
  return rc;
}

int cmp_csp_check_domain(const cmp_csp_t *csp, const char *domain,
                         cmp_csp_resource_type_t type, int *out_allowed) {
  int rc = CMP_SUCCESS;
  size_t i;
  (void)type; /* For this basic implementation, type does not change outcome */

  if (!csp || !domain || !out_allowed) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_csp_check_domain: Invalid argument\n");
    return rc;
  }

  *out_allowed = 0;

  for (i = 0; i < csp->count; i++) {
    if (strcmp(csp->domains[i].domain, domain) == 0) {
      *out_allowed = 1;
      break;
    }
  }

  return rc;
}
