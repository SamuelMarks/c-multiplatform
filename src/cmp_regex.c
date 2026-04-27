/* clang-format off */
#include "cmp_regex.h"
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
#include "cmp_log.h"
/* clang-format on */

struct cmp_regex {
  char *pattern;
};

/**
 * @brief codex_strdup
 *
 * @param s Parameter description.
 * @param out_str Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int codex_strdup(const char *s, char **out_str) {
  int rc;
  rc = CMP_SUCCESS;
  size_t len;
  char *d = NULL;

  if (!s || !out_str) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in codex_strdup: Invalid argument\n");
    return rc;
  }

  len = strlen(s);
  rc = CMP_MALLOC(len + 1, (void **)&d);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in codex_strdup: Out of memory\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(d, len + 1, s);
#else
  strcpy(d, s);
#endif

  *out_str = d;
  return CMP_SUCCESS;
}

/**
 * @brief cmp_regex_compile
 *
 * @param out_regex Parameter description.
 * @param pattern Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_compile(cmp_regex_t **out_regex, const char *pattern) {
  int rc;
  rc = CMP_SUCCESS;
  cmp_regex_t *r = NULL;

  if (!out_regex || !pattern) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_regex_compile: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_regex_t), (void **)&r);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_regex_compile: Out of memory for regex\n");
    return CMP_ERROR_OOM;
  }

  rc = codex_strdup(pattern, &r->pattern);
  if (rc != CMP_SUCCESS) {
    int free_rc = CMP_FREE(r);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_regex_compile: CMP_FREE failed\n");
    }
    LOG_DEBUG("Error in cmp_regex_compile: strdup failed\n");
    return rc;
  }

  *out_regex = r;
  return CMP_SUCCESS;
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
  int rc;
  rc = CMP_SUCCESS;

  if (!regex || !string || !out_matched) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_regex_match: Invalid argument\n");
    return rc;
  }

  /* Mock regex logic for tests: Basic substring search */
  if (strstr(string, regex->pattern) != NULL) {
    *out_matched = 1;
  } else {
    *out_matched = 0;
  }
  return CMP_SUCCESS;
}

/**
 * @brief cmp_regex_free
 *
 * @param regex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_free(cmp_regex_t *regex) {
  int rc;
  rc = CMP_SUCCESS;

  if (!regex) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_regex_free: Invalid argument\n");
    return rc;
  }

  if (regex->pattern) {
    memset(regex->pattern, 0, strlen(regex->pattern));
    rc = CMP_FREE(regex->pattern);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_regex_free: CMP_FREE pattern failed\n");
      return rc;
    }
  }

  rc = CMP_FREE(regex);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_regex_free: CMP_FREE regex failed\n");
    return rc;
  }

  return CMP_SUCCESS;
}
