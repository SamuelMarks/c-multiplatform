/* clang-format off */
#include "cmp_regex.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static char *codex_strdup(const char *s) {
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  d = (char *)malloc(len + 1);
  if (!d)
    return NULL;
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

int cmp_regex_compile(cmp_regex_t **out_regex, const char *pattern) {
  cmp_regex_t *r;

  if (!out_regex || !pattern) {
    return -1;
  }

  r = (cmp_regex_t *)malloc(sizeof(cmp_regex_t));
  if (!r) {
    return -2;
  }

  r->pattern = strdup(pattern);
  if (!r->pattern) {
    free(r);
    return -2;
  }

  *out_regex = r;
  return 0;
}

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

int cmp_regex_free(cmp_regex_t *regex) {
  if (!regex) {
    return -1;
  }
  if (regex->pattern) {
    memset(regex->pattern, 0, strlen(regex->pattern));
    free(regex->pattern);
  }
  free(regex);
  return 0;
}
