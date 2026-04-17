/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* clang-format on */

#define MAX_BREADCRUMBS 32

struct cmp_breadcrumbs {
  cmp_breadcrumb_t segments[MAX_BREADCRUMBS];
  size_t count;
};

int cmp_breadcrumbs_create(cmp_breadcrumbs_t **out_crumbs) {
  int rc = CMP_SUCCESS;
  cmp_breadcrumbs_t *crumbs = NULL;

  if (!out_crumbs) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_breadcrumbs_create: Invalid argument "
                    "(out_crumbs=NULL)\n");
    return rc;
  }

  crumbs = (cmp_breadcrumbs_t *)malloc(sizeof(cmp_breadcrumbs_t));
  if (!crumbs) {
    rc = CMP_ERROR_OOM;
    fprintf(stderr, "Error in cmp_breadcrumbs_create: Out of memory\n");
    return rc;
  }

  crumbs->count = 0;

  *out_crumbs = crumbs;
  return rc;
}

int cmp_breadcrumbs_destroy(cmp_breadcrumbs_t *crumbs) {
  int rc = CMP_SUCCESS;

  if (!crumbs) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(
        stderr,
        "Error in cmp_breadcrumbs_destroy: Invalid argument (crumbs=NULL)\n");
    return rc;
  }
  free(crumbs);
  return rc;
}

int cmp_breadcrumbs_set_path(cmp_breadcrumbs_t *crumbs, const char *full_path) {
  int rc = CMP_SUCCESS;
  const char *start;
  const char *p;
  size_t len;

  if (!crumbs || !full_path) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_breadcrumbs_set_path: Invalid argument\n");
    return rc;
  }

  crumbs->count = 0;
  start = full_path;

  while (*start && crumbs->count < MAX_BREADCRUMBS) {
    /* Skip leading slashes */
    while (*start == '/' || *start == '\\') {
      start++;
    }

    if (!*start) {
      break;
    }

    /* Find next slash or end */
    p = start;
    while (*p && *p != '/' && *p != '\\') {
      p++;
    }

    len = (size_t)(p - start);
    if (len > 0) {
      if (len >= sizeof(crumbs->segments[0].label)) {
        len = sizeof(crumbs->segments[0].label) - 1;
      }
      strncpy(crumbs->segments[crumbs->count].label, start, len);
      crumbs->segments[crumbs->count].label[len] = '\0';
      crumbs->segments[crumbs->count].is_active = (*p == '\0') ? 1 : 0;
      crumbs->count++;
    }

    start = p;
  }

  return rc;
}

int cmp_breadcrumbs_get_count(const cmp_breadcrumbs_t *crumbs,
                              size_t *out_count) {
  int rc = CMP_SUCCESS;

  if (!crumbs || !out_count) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_breadcrumbs_get_count: Invalid argument\n");
    return rc;
  }

  *out_count = crumbs->count;
  return rc;
}

int cmp_breadcrumbs_get_segment(const cmp_breadcrumbs_t *crumbs, size_t index,
                                cmp_breadcrumb_t **out_segment) {
  int rc = CMP_SUCCESS;

  if (!crumbs || !out_segment || index >= crumbs->count) {
    rc = CMP_ERROR_INVALID_ARG;
    fprintf(stderr, "Error in cmp_breadcrumbs_get_segment: Invalid argument or "
                    "index out of bounds\n");
    return rc;
  }

  *out_segment = (cmp_breadcrumb_t *)&crumbs->segments[index];
  return rc;
}
