/* clang-format off */
#include "cmp_regex.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

/**
 * @brief codex_strdup
 *
 * @param s Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static char *codex_strdup(const char *s) {
  int rc = CMP_SUCCESS;
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  rc = CMP_MALLOC(len + 1, (void **)&(d));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return NULL;
  }
#if defined(_MSC_VER)
  strcpy_s(d, len + 1, s);
#else
  strcpy(d, s);
#endif
  return d;
}
#define strdup codex_strdup

struct cmp_regex {
  char *pattern;
};

/**
 * @brief cmp_regex_compile
 *
 * @param out_regex Parameter description.
 * @param pattern Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_compile(cmp_regex_t **out_regex, const char *pattern) {
  int rc = CMP_SUCCESS;
  cmp_regex_t *r;

  if (!out_regex || !pattern) {
    return -1;
  }

  rc = CMP_MALLOC(sizeof(cmp_regex_t), (void **)&(r));
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("OOM\n");
    return CMP_ERROR_OOM;
  }

  r->pattern = strdup(pattern);
  if (!r->pattern) {
    rc = CMP_FREE(r);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
    return -2;
  }

  *out_regex = r;
  return 0;
}

/**
 * @brief cmp_regex_match
 *
 * @param regex Parameter description.
 * @param string Parameter description.
 * @param out_matched Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_match(cmp_regex_t *regex, const char *string, int *out_matched) {
  if (!regex || !string || !out_matched) {
    return -1;
  }

  /* Mock regex logic for tests: Basic substring search */
  if (strstr(string, regex->pattern) != NULL) {
    *out_matched = 1;
  } else {
    *out_matched = 0;
  }

  return 0;
}

/**
 * @brief cmp_regex_free
 *
 * @param regex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_free(cmp_regex_t *regex) {
  int rc = CMP_SUCCESS;
  if (!regex) {
    return -1;
  }
  if (regex->pattern) {
    memset(regex->pattern, 0, strlen(regex->pattern));
    rc = CMP_FREE(regex->pattern);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Free failed\n");
    }
  }
  rc = CMP_FREE(regex);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Free failed\n");
  }
  return 0;
}
