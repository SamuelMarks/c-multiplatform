/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_i18n_entry {
  char *locale;
  char *key;
  char *value;
} cmp_i18n_entry_t;

struct cmp_i18n {
  cmp_i18n_entry_t *entries;
  size_t count;
  size_t capacity;
};

int cmp_i18n_create(cmp_i18n_t **out_i18n) {
  cmp_i18n_t *i18n;
  if (!out_i18n) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_i18n_t), (void **)&i18n) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(i18n, 0, sizeof(cmp_i18n_t));
  *out_i18n = i18n;
  return CMP_SUCCESS;
}

int cmp_i18n_destroy(cmp_i18n_t *i18n) {
  size_t i;
  if (!i18n) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (i18n->entries) {
    for (i = 0; i < i18n->count; i++) {
      if (i18n->entries[i].locale) {
        CMP_FREE(i18n->entries[i].locale);
      }
      if (i18n->entries[i].key) {
        CMP_FREE(i18n->entries[i].key);
      }
      if (i18n->entries[i].value) {
        CMP_FREE(i18n->entries[i].value);
      }
    }
    CMP_FREE(i18n->entries);
  }
  CMP_FREE(i18n);
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

int cmp_i18n_add_string(cmp_i18n_t *i18n, const char *locale, const char *key,
                        const char *value) {
  if (!i18n || !locale || !key || !value) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (i18n->count >= i18n->capacity) {
    size_t new_cap = i18n->capacity == 0 ? 8 : i18n->capacity * 2;
    cmp_i18n_entry_t *new_entries;
    if (CMP_MALLOC(new_cap * sizeof(cmp_i18n_entry_t), (void **)&new_entries) !=
        CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    if (i18n->entries) {
      memcpy(new_entries, i18n->entries,
             i18n->count * sizeof(cmp_i18n_entry_t));
      CMP_FREE(i18n->entries);
    }
    i18n->entries = new_entries;
    i18n->capacity = new_cap;
  }

  if (str_duplicate(locale, &i18n->entries[i18n->count].locale) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  if (str_duplicate(key, &i18n->entries[i18n->count].key) != CMP_SUCCESS) {
    CMP_FREE(i18n->entries[i18n->count].locale);
    return CMP_ERROR_OOM;
  }
  if (str_duplicate(value, &i18n->entries[i18n->count].value) != CMP_SUCCESS) {
    CMP_FREE(i18n->entries[i18n->count].locale);
    CMP_FREE(i18n->entries[i18n->count].key);
    return CMP_ERROR_OOM;
  }

  i18n->count++;
  return CMP_SUCCESS;
}

int cmp_i18n_get_string(const cmp_i18n_t *i18n, const char *locale,
                        const char *key, char **out_value) {
  size_t i;
  if (!i18n || !locale || !key || !out_value) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < i18n->count; i++) {
    if (strcmp(i18n->entries[i].locale, locale) == 0 &&
        strcmp(i18n->entries[i].key, key) == 0) {
      return str_duplicate(i18n->entries[i].value, out_value);
    }
  }

  *out_value = NULL;
  return CMP_ERROR_NOT_FOUND;
}
