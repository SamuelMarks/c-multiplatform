/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define MAX_BREADCRUMBS 32

struct cmp_breadcrumbs {
  cmp_breadcrumb_t segments[MAX_BREADCRUMBS];
  size_t count;
};

int cmp_breadcrumbs_create(cmp_breadcrumbs_t **out_crumbs) {
  cmp_breadcrumbs_t *crumbs;

  if (!out_crumbs) {
    return CMP_ERROR_INVALID_ARG;
  }

  crumbs = (cmp_breadcrumbs_t *)malloc(sizeof(cmp_breadcrumbs_t));
  if (!crumbs) {
    return CMP_ERROR_OOM;
  }

  crumbs->count = 0;

  *out_crumbs = crumbs;
  return CMP_SUCCESS;
}

int cmp_breadcrumbs_destroy(cmp_breadcrumbs_t *crumbs) {
  if (!crumbs) {
    return CMP_ERROR_INVALID_ARG;
  }
  free(crumbs);
  return CMP_SUCCESS;
}

int cmp_breadcrumbs_set_path(cmp_breadcrumbs_t *crumbs, const char *full_path) {
  const char *start;
  const char *p;
  size_t len;

  if (!crumbs || !full_path) {
    return CMP_ERROR_INVALID_ARG;
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

  return CMP_SUCCESS;
}

int cmp_breadcrumbs_get_count(const cmp_breadcrumbs_t *crumbs,
                              size_t *out_count) {
  if (!crumbs || !out_count) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_count = crumbs->count;
  return CMP_SUCCESS;
}

int cmp_breadcrumbs_get_segment(const cmp_breadcrumbs_t *crumbs, size_t index,
                                cmp_breadcrumb_t **out_segment) {
  if (!crumbs || !out_segment || index >= crumbs->count) {
    return CMP_ERROR_INVALID_ARG;
  }

  *out_segment = (cmp_breadcrumb_t *)&crumbs->segments[index];
  return CMP_SUCCESS;
}
