/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

#define MAX_BREADCRUMBS 32

struct cmp_breadcrumbs {
  cmp_breadcrumb_t segments[MAX_BREADCRUMBS];
  size_t count;
};

/**
 * @brief Creates a new breadcrumbs context.
 *
 * @param out_crumbs Pointer to store the newly allocated breadcrumbs context.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_breadcrumbs_create(cmp_breadcrumbs_t **out_crumbs) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  cmp_breadcrumbs_t *crumbs = NULL;

  if (out_crumbs == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_breadcrumbs_create: Invalid argument (out_crumbs=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_breadcrumbs_t), (void **)&crumbs);
  if (rc != CMP_SUCCESS) {
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_breadcrumbs_create: Out of memory: %s\n", err_str);

    return rc;
  }

  crumbs->count = 0;

  *out_crumbs = crumbs;
  cmp_log_debug(
      "cmp_breadcrumbs_create: Successfully created breadcrumbs context\n");

  return rc;
}

/**
 * @brief Destroys a breadcrumbs context.
 *
 * @param crumbs Pointer to the breadcrumbs context to destroy.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_breadcrumbs_destroy(cmp_breadcrumbs_t *crumbs) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (crumbs == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_breadcrumbs_destroy: Invalid argument (crumbs=NULL): %s\n",
        err_str);

    return rc;
  }

  rc = CMP_FREE(crumbs);
  if (rc != CMP_SUCCESS) {
    cmp_log_debug("cmp_breadcrumbs_destroy: CMP_FREE failed\n");

    return rc;
  }

  cmp_log_debug(
      "cmp_breadcrumbs_destroy: Successfully destroyed breadcrumbs context\n");
  return rc;
}

/**
 * @brief Parses a path string and populates the breadcrumbs.
 *
 * @param crumbs Pointer to the breadcrumbs context.
 * @param full_path The path string to parse (e.g., "/usr/local/bin").
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_breadcrumbs_set_path(cmp_breadcrumbs_t *crumbs, const char *full_path) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;
  const char *start;
  const char *p;
  size_t len;

  if (crumbs == NULL || full_path == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_breadcrumbs_set_path: Invalid argument: %s\n", err_str);

    return rc;
  }

  crumbs->count = 0;
  start = full_path;

  while (*start != '\0' && crumbs->count < MAX_BREADCRUMBS) {
    /* Skip leading slashes */
    while (*start == '/' || *start == '\\') {
      start++;
    }

    if (*start == '\0') {
      break;
    }

    /* Find next slash or end */
    p = start;
    while (*p != '\0' && *p != '/' && *p != '\\') {
      p++;
    }

    len = (size_t)(p - start);
    if (len > 0) {
      if (len >= sizeof(crumbs->segments[0].label)) {
        len = sizeof(crumbs->segments[0].label) - 1;
      }
#if defined(_MSC_VER)
      if (strncpy_s(crumbs->segments[crumbs->count].label,
                    sizeof(crumbs->segments[0].label), start, len) != 0) {
        cmp_log_debug("cmp_breadcrumbs_set_path: strncpy_s failed\n");
        return CMP_ERROR_GENERAL;
      }
#else
      strncpy(crumbs->segments[crumbs->count].label, start, len);
      crumbs->segments[crumbs->count].label[len] = '\0';
#endif
      crumbs->segments[crumbs->count].is_active = (*p == '\0') ? 1 : 0;
      crumbs->count++;
    }

    start = p;
  }

  cmp_log_debug("cmp_breadcrumbs_set_path: Parsed path to %d segments\n",
                (int)crumbs->count);

  return rc;
}

/**
 * @brief Gets the number of parsed breadcrumb segments.
 *
 * @param crumbs Pointer to the breadcrumbs context.
 * @param out_count Pointer to store the count.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_breadcrumbs_get_count(const cmp_breadcrumbs_t *crumbs,
                              size_t *out_count) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (crumbs == NULL || out_count == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug("cmp_breadcrumbs_get_count: Invalid argument: %s\n", err_str);

    return rc;
  }

  *out_count = crumbs->count;
  cmp_log_debug("cmp_breadcrumbs_get_count: Count is %d\n", (int)crumbs->count);

  return rc;
}

/**
 * @brief Gets a specific breadcrumb segment by index.
 *
 * @param crumbs Pointer to the breadcrumbs context.
 * @param index The 0-based index of the segment.
 * @param out_segment Pointer to store the segment pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_breadcrumbs_get_segment(const cmp_breadcrumbs_t *crumbs, size_t index,
                                cmp_breadcrumb_t **out_segment) {
  int rc = CMP_SUCCESS;
  int err_rc;
  const char *err_str;

  if (crumbs == NULL || out_segment == NULL || index >= crumbs->count) {
    rc = CMP_ERROR_INVALID_ARG;
    err_rc = cmp_strerror(rc, &err_str);
    if (err_rc != CMP_SUCCESS) {
      err_str = "Unknown";
    }
    cmp_log_debug(
        "cmp_breadcrumbs_get_segment: Invalid argument or bounds: %s\n",
        err_str);

    return rc;
  }

  *out_segment = (cmp_breadcrumb_t *)&crumbs->segments[index];
  cmp_log_debug("cmp_breadcrumbs_get_segment: Fetched segment at index %d\n",
                (int)index);

  return rc;
}
