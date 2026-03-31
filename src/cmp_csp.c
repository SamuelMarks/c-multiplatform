/* clang-format off */
#include "cmp.h"

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
  cmp_csp_t *csp;
  if (!out_csp) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_csp_t), (void **)&csp) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(csp, 0, sizeof(cmp_csp_t));
  *out_csp = csp;
  return CMP_SUCCESS;
}

int cmp_csp_destroy(cmp_csp_t *csp) {
  size_t i;
  if (!csp) {
    return CMP_ERROR_INVALID_ARG;
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
  return CMP_SUCCESS;
}

static int str_duplicate(const char *src, char **out_dst) {
  size_t len = strlen(src);
  if (CMP_MALLOC(len + 1, (void **)out_dst) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(*out_dst, len + 1, src);
#else
  strcpy(*out_dst, src);
#endif
  return CMP_SUCCESS;
}

int cmp_csp_add_domain(cmp_csp_t *csp, const char *domain) {
  if (!csp || !domain) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (csp->count >= csp->capacity) {
    size_t new_cap = csp->capacity == 0 ? 8 : csp->capacity * 2;
    cmp_csp_domain_t *new_domains;
    if (CMP_MALLOC(new_cap * sizeof(cmp_csp_domain_t), (void **)&new_domains) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    if (csp->domains) {
      memcpy(new_domains, csp->domains, csp->count * sizeof(cmp_csp_domain_t));
      CMP_FREE(csp->domains);
    }
    csp->domains = new_domains;
    csp->capacity = new_cap;
  }

  if (str_duplicate(domain, &csp->domains[csp->count].domain) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  csp->count++;
  return CMP_SUCCESS;
}

int cmp_csp_check_domain(const cmp_csp_t *csp, const char *domain,
                         cmp_csp_resource_type_t type, int *out_allowed) {
  size_t i;
  (void)type; /* For this basic implementation, type does not change outcome */

  if (!csp || !domain || !out_allowed) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_allowed = 0;

  for (i = 0; i < csp->count; i++) {
    if (strcmp(csp->domains[i].domain, domain) == 0) {
      *out_allowed = 1;
      break;
    }
  }

  return CMP_SUCCESS;
}
