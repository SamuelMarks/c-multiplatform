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

/**
 * @brief str_duplicate
 *
 * @param src Parameter description.
 * @param out_dst Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int str_duplicate(const char *src, char **out_dst) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t len;

  if (src == NULL || out_dst == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("str_duplicate: Invalid argument: %s\n", err_str);

    return rc;
  }

  len = strlen(src);
  rc = CMP_MALLOC(len + 1, (void **)out_dst);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("str_duplicate: Out of memory: %s\n", err_str);

    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(*out_dst, len + 1, src) != 0) {
    cmp_log_debug("str_duplicate: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(*out_dst, src);
#endif

  return rc;
}

/**
 * @brief cmp_csp_create
 *
 * @param out_csp Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_csp_create(cmp_csp_t **out_csp) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_csp_t *csp = NULL;

  if (out_csp == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_csp_create: Invalid argument (out_csp=NULL): %s\n",
                  err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_csp_t), (void **)&csp);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_csp_create: Out of memory: %s\n", err_str);

    return rc;
  }

  memset(csp, 0, sizeof(cmp_csp_t));
  *out_csp = csp;
  cmp_log_debug("cmp_csp_create: Successfully created CSP context\n");

  return rc;
}

/**
 * @brief cmp_csp_destroy
 *
 * @param csp Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_csp_destroy(cmp_csp_t *csp) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;

  if (csp == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_csp_destroy: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (csp->domains != NULL) {
    for (i = 0; i < csp->count; i++) {
      if (csp->domains[i].domain != NULL) {
        rc = CMP_FREE(csp->domains[i].domain);
        if (rc != CMP_SUCCESS) {
          cmp_log_debug("cmp_csp_destroy: CMP_FREE domain failed\n");
        }
      }
    }
    rc = CMP_FREE(csp->domains);
    if (rc != CMP_SUCCESS) {
      cmp_log_debug("cmp_csp_destroy: CMP_FREE domains array failed\n");
    }
  }

  rc = CMP_FREE(csp);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_csp_destroy: CMP_FREE ctx failed\n");
  }

  cmp_log_debug("cmp_csp_destroy: Successfully destroyed CSP context\n");
  return rc;
}

/**
 * @brief cmp_csp_add_domain
 *
 * @param csp Parameter description.
 * @param domain Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_csp_add_domain(cmp_csp_t *csp, const char *domain) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t new_cap;
  cmp_csp_domain_t *new_domains = NULL;

  if (csp == NULL || domain == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_csp_add_domain: Invalid argument: %s\n", err_str);

    return rc;
  }

  if (csp->count >= csp->capacity) {
    new_cap = csp->capacity == 0 ? 8 : csp->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_csp_domain_t), (void **)&new_domains);
    if (rc != CMP_SUCCESS) {
      err_rc = cmp_strerror(rc, &err_str);
      if (err_rc != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      cmp_log_debug(
          "cmp_csp_add_domain: Out of memory allocating domains: %s\n",
          err_str);

      return rc;
    }

    if (csp->domains != NULL) {
      memcpy(new_domains, csp->domains, csp->count * sizeof(cmp_csp_domain_t));
      rc = CMP_FREE(csp->domains);
      if (rc != CMP_SUCCESS) {
        cmp_log_debug("cmp_csp_add_domain: CMP_FREE old domains failed\n");
      }
    }
    csp->domains = new_domains;
    csp->capacity = new_cap;
  }

  rc = str_duplicate(domain, &csp->domains[csp->count].domain);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_csp_add_domain: str_duplicate failed\n");

    return rc;
  }

  csp->count++;
  cmp_log_debug("cmp_csp_add_domain: Added domain %s\n", domain);
  return rc;
}

/**
 * @brief cmp_csp_check_domain
 *
 * @param csp Parameter description.
 * @param domain Parameter description.
 * @param type Parameter description.
 * @param out_allowed Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_csp_check_domain(const cmp_csp_t *csp, const char *domain,
                         cmp_csp_resource_type_t type, int *out_allowed) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  size_t i;
  (void)type; /* For this basic implementation, type does not change outcome */

  if (csp == NULL || domain == NULL || out_allowed == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_csp_check_domain: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_allowed = 0;

  for (i = 0; i < csp->count; i++) {
    if (strcmp(csp->domains[i].domain, domain) == 0) {
      *out_allowed = 1;
      cmp_log_debug("cmp_csp_check_domain: Allowed domain %s\n", domain);
      return rc;
    }
  }

  cmp_log_debug("cmp_csp_check_domain: Rejected domain %s\n", domain);

  return rc;
}
