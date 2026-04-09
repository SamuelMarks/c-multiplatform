/* clang-format off */
#include "cmp.h"

#include <stdio.h>
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

static cmp_i18n_t *g_global_i18n = NULL;
static int g_i18n_initialized = 0;
static cmp_text_direction_t g_bidi_dir = CMP_TEXT_DIR_LTR;
static char g_current_locale[32] = "en-US";

int cmp_i18n_init(void) {
  if (g_i18n_initialized)
    return CMP_SUCCESS;
  if (cmp_i18n_create(&g_global_i18n) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  g_i18n_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_i18n_shutdown(void) {
  if (!g_i18n_initialized)
    return CMP_SUCCESS;
  cmp_i18n_destroy(g_global_i18n);
  g_global_i18n = NULL;
  g_i18n_initialized = 0;
  return CMP_SUCCESS;
}

int cmp_i18n_detect_os_locale(cmp_string_t *out_locale) {
  if (!out_locale)
    return CMP_ERROR_INVALID_ARG;
#if defined(_MSC_VER)
  str_duplicate("en-US", &out_locale->data);
#else
  str_duplicate("en-US", &out_locale->data);
#endif
  out_locale->length = 5;
  out_locale->capacity = 6;
  return CMP_SUCCESS;
}

int cmp_i18n_load_catalog(const char *virtual_path, const char *locale) {
  if (!g_i18n_initialized)
    return CMP_ERROR_INVALID_STATE;
  (void)virtual_path;
#if defined(_MSC_VER)
  strcpy_s(g_current_locale, 32, locale);
#else
  strncpy(g_current_locale, locale, 31);
  g_current_locale[31] = '\0';
#endif
  return CMP_SUCCESS;
}

int cmp_i18n_translate(const char *key, cmp_string_t *out_translated) {
  char *val = NULL;
  if (!g_i18n_initialized || !out_translated)
    return CMP_ERROR_INVALID_STATE;

  if (cmp_i18n_get_string(g_global_i18n, g_current_locale, key, &val) ==
      CMP_SUCCESS) {
    out_translated->data = val;
    out_translated->length = strlen(val);
    out_translated->capacity = out_translated->length + 1;
    return CMP_SUCCESS;
  }
  str_duplicate(key, &val);
  out_translated->data = val;
  out_translated->length = strlen(val);
  out_translated->capacity = out_translated->length + 1;
  return CMP_SUCCESS;
}

int cmp_i18n_translate_plural(const char *key, int count,
                              cmp_string_t *out_translated) {
  char plural_key[256];
  if (!key || !out_translated)
    return CMP_ERROR_INVALID_ARG;

  if (count == 1) {
    return cmp_i18n_translate(key, out_translated);
  }

#if defined(_MSC_VER)
  sprintf_s(plural_key, sizeof(plural_key), "%s_plural", key);
#else
  sprintf(plural_key, "%s_plural", key);
#endif

  return cmp_i18n_translate(plural_key, out_translated);
}

int cmp_i18n_set_bidi_direction(cmp_text_direction_t dir) {
  g_bidi_dir = dir;
  return CMP_SUCCESS;
}

int cmp_i18n_get_bidi_direction(void) { return g_bidi_dir; }
