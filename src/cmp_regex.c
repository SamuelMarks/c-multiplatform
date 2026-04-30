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
  int rc = CMP_SUCCESS;
  size_t len;
  char *d = NULL;

  if (s == NULL || out_str == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in codex_strdup: Invalid argument\n");
    return rc;
  }

  len = strlen(s);
  rc = CMP_MALLOC(len + 1, (void **)&d);
  if (rc != CMP_SUCCESS || d == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in codex_strdup: Out of memory\n");
    return rc;
  }
#if defined(_MSC_VER)
  if (strcpy_s(d, len + 1, s) != 0) {
    int free_rc = CMP_FREE(d);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in codex_strdup: CMP_FREE failed during recovery\n");
    }
    rc = CMP_ERROR_GENERAL;
    LOG_DEBUG("Error in codex_strdup: strcpy_s failed\n");
    return rc;
  }
#else
  strcpy(d, s);
#endif

  *out_str = d;
  return rc;
}

/**
 * @brief cmp_regex_compile
 *
 * @param out_regex Parameter description.
 * @param pattern Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_compile(cmp_regex_t **out_regex, const char *pattern) {
  int rc = CMP_SUCCESS;
  cmp_regex_t *r = NULL;
  int free_rc;

  if (out_regex == NULL || pattern == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_regex_compile: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_regex_t), (void **)&r);
  if (rc != CMP_SUCCESS || r == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_regex_compile: Out of memory for regex\n");
    return rc;
  }

  rc = codex_strdup(pattern, &r->pattern);
  if (rc != CMP_SUCCESS) {
    free_rc = CMP_FREE(r);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_regex_compile: CMP_FREE failed\n");
    }
    LOG_DEBUG("Error in cmp_regex_compile: strdup failed\n");
    return rc;
  }

  *out_regex = r;
  return rc;
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
  int rc = CMP_SUCCESS;

  if (regex == NULL || string == NULL || out_matched == NULL) {
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
  return rc;
}

/**
 * @brief cmp_regex_free
 *
 * @param regex Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_regex_free(cmp_regex_t *regex) {
  int rc = CMP_SUCCESS;
  int free_rc;

  if (regex == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_regex_free: Invalid argument\n");
    return rc;
  }

  if (regex->pattern != NULL) {
    memset(regex->pattern, 0, strlen(regex->pattern));
    free_rc = CMP_FREE(regex->pattern);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_regex_free: CMP_FREE pattern failed\n");
      rc = free_rc;
    }
  }

  free_rc = CMP_FREE(regex);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_regex_free: CMP_FREE regex failed\n");
    /* Do not override rc if it's already an error, but here it might just be
     * CMP_SUCCESS. We should return error */
    rc = free_rc;
  }

  return rc;
}