/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_i18n_entry {
  const char *locale; /* Interned */
  const char *key;    /* Interned */
  char *value;        /* Dynamic */
} cmp_i18n_entry_t;

struct cmp_i18n {
  cmp_i18n_entry_t *entries;
  size_t count;
  size_t capacity;
};

static char **g_interned_strings = NULL;
static size_t g_interned_count = 0;
static size_t g_interned_capacity = 0;

static const char *intern_string(const char *str) {
  int rc = CMP_SUCCESS;
  size_t i;
  char **new_pool = NULL;
  size_t new_cap;
  size_t len;
  char *new_str = NULL;

  if (!str)
    return NULL;

  for (i = 0; i < g_interned_count; i++) {
    if (strcmp(g_interned_strings[i], str) == 0) {
      return g_interned_strings[i];
    }
  }

  if (g_interned_count >= g_interned_capacity) {
    new_cap = g_interned_capacity == 0 ? 64 : g_interned_capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(char *), (void **)&new_pool);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in intern_string: Out of memory allocating string pool\n");
      return NULL;
    }
    if (g_interned_strings) {
      memcpy(new_pool, g_interned_strings, g_interned_count * sizeof(char *));
      CMP_FREE(g_interned_strings);
    }
    g_interned_strings = new_pool;
    g_interned_capacity = new_cap;
  }

  len = strlen(str);
  rc = CMP_MALLOC(len + 1, (void **)&new_str);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in intern_string: Out of memory allocating string\n");
    return NULL;
  }
#if defined(_MSC_VER)
  strcpy_s(new_str, len + 1, str);
#else
  strcpy(new_str, str);
#endif

  g_interned_strings[g_interned_count++] = new_str;
  return new_str;
}

static void free_interned_strings(void) {
  size_t i;
  if (g_interned_strings) {
    for (i = 0; i < g_interned_count; i++) {
      CMP_FREE(g_interned_strings[i]);
    }
    CMP_FREE(g_interned_strings);
    g_interned_strings = NULL;
    g_interned_count = 0;
    g_interned_capacity = 0;
  }
}

int cmp_i18n_create(cmp_i18n_t **out_i18n) {
  int rc = CMP_SUCCESS;
  cmp_i18n_t *i18n = NULL;

  if (!out_i18n) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_create: Invalid argument (out_i18n=NULL)\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_i18n_t), (void **)&i18n);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_create: Out of memory\n");
    return rc;
  }

  memset(i18n, 0, sizeof(cmp_i18n_t));
  *out_i18n = i18n;
  return rc;
}

int cmp_i18n_destroy(cmp_i18n_t *i18n) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!i18n) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_destroy: Invalid argument (i18n=NULL)\n");
    return rc;
  }

  if (i18n->entries) {
    for (i = 0; i < i18n->count; i++) {
      if (i18n->entries[i].value) {
        CMP_FREE(i18n->entries[i].value);
      }
    }
    CMP_FREE(i18n->entries);
  }
  CMP_FREE(i18n);
  return rc;
}

static int str_duplicate(const char *src, char **out_dst) {
  int rc = CMP_SUCCESS;
  size_t len;

  if (!src || !out_dst) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in str_duplicate: Invalid argument\n");
    return rc;
  }

  len = strlen(src);
  rc = CMP_MALLOC(len + 1, (void **)out_dst);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in str_duplicate: Out of memory\n");
    return rc;
  }

#if defined(_MSC_VER)
  strcpy_s(*out_dst, len + 1, src);
#else
  strcpy(*out_dst, src);
#endif
  return rc;
}

int cmp_i18n_add_string(cmp_i18n_t *i18n, const char *locale, const char *key,
                        const char *value) {
  int rc = CMP_SUCCESS;
  const char *interned_locale = NULL;
  const char *interned_key = NULL;
  cmp_i18n_entry_t *new_entries = NULL;
  size_t new_cap;

  if (!i18n || !locale || !key || !value) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_add_string: Invalid argument\n");
    return rc;
  }

  interned_locale = intern_string(locale);
  interned_key = intern_string(key);

  if (!interned_locale || !interned_key) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_i18n_add_string: Failed to intern string\n");
    return rc;
  }

  if (i18n->count >= i18n->capacity) {
    new_cap = i18n->capacity == 0 ? 8 : i18n->capacity * 2;
    rc = CMP_MALLOC(new_cap * sizeof(cmp_i18n_entry_t), (void **)&new_entries);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_i18n_add_string: Out of memory allocating entries\n");
      return rc;
    }
    if (i18n->entries) {
      memcpy(new_entries, i18n->entries,
             i18n->count * sizeof(cmp_i18n_entry_t));
      CMP_FREE(i18n->entries);
    }
    i18n->entries = new_entries;
    i18n->capacity = new_cap;
  }

  i18n->entries[i18n->count].locale = interned_locale;
  i18n->entries[i18n->count].key = interned_key;

  rc = str_duplicate(value, &i18n->entries[i18n->count].value);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_add_string: str_duplicate failed\n");
    return rc;
  }

  i18n->count++;
  return rc;
}

int cmp_i18n_get_string(const cmp_i18n_t *i18n, const char *locale,
                        const char *key, char **out_value) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (!i18n || !locale || !key || !out_value) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_get_string: Invalid argument\n");
    return rc;
  }

  for (i = 0; i < i18n->count; i++) {
    if (strcmp(i18n->entries[i].locale, locale) == 0 &&
        strcmp(i18n->entries[i].key, key) == 0) {
      return str_duplicate(i18n->entries[i].value, out_value);
    }
  }

  *out_value = NULL;
  rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_i18n_get_string: Key not found\n");
  return rc;
}

static cmp_i18n_t *g_global_i18n = NULL;
static int g_i18n_initialized = 0;
static cmp_text_direction_t g_bidi_dir = CMP_TEXT_DIR_LTR;
static char g_current_locale[32] = "en-US";

int cmp_i18n_init(void) {
  int rc = CMP_SUCCESS;

  if (g_i18n_initialized)
    return rc;

  rc = cmp_i18n_create(&g_global_i18n);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_init: Failed to create global i18n\n");
    return rc;
  }

  g_i18n_initialized = 1;
  return rc;
}

int cmp_i18n_shutdown(void) {
  int rc = CMP_SUCCESS;

  if (!g_i18n_initialized)
    return rc;

  rc = cmp_i18n_destroy(g_global_i18n);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_shutdown: Failed to destroy global i18n\n");
  }

  g_global_i18n = NULL;
  free_interned_strings();
  g_i18n_initialized = 0;
  return rc;
}

int cmp_i18n_detect_os_locale(cmp_string_t *out_locale) {
  int rc = CMP_SUCCESS;

  if (!out_locale) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_detect_os_locale: Invalid argument\n");
    return rc;
  }

#if defined(_MSC_VER)
  rc = str_duplicate("en-US", &out_locale->data);
#else
  rc = str_duplicate("en-US", &out_locale->data);
#endif
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_detect_os_locale: str_duplicate failed\n");
    return rc;
  }

  out_locale->length = 5;
  out_locale->capacity = 6;
  return rc;
}

int cmp_i18n_load_catalog(const char *virtual_path, const char *locale) {
  int rc = CMP_SUCCESS;
  void *buffer = NULL;
  size_t size = 0;
  char *text = NULL;
  char *line = NULL;
  char *saveptr = NULL;

  if (!g_i18n_initialized) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_i18n_load_catalog: Not initialized\n");
    return rc;
  }

  rc = cmp_vfs_read_file_sync(virtual_path, &buffer, &size);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_load_catalog: Failed to read file\n");
    return rc;
  }

  rc = CMP_MALLOC(size + 1, (void **)&text);
  if (rc != CMP_SUCCESS) {
    CMP_FREE(buffer);
    LOG_DEBUG("Error in cmp_i18n_load_catalog: Out of memory allocating text "
              "buffer\n");
    return rc;
  }

  memcpy(text, buffer, size);
  text[size] = '\0';
  CMP_FREE(buffer);

#if defined(_MSC_VER)
  strcpy_s(g_current_locale, sizeof(g_current_locale), locale);
  line = strtok_s(text, "\n", &saveptr);
#else
  strncpy(g_current_locale, locale, sizeof(g_current_locale) - 1);
  g_current_locale[sizeof(g_current_locale) - 1] = '\0';
  line = strtok_r(text, "\n", &saveptr);
#endif

  while (line != NULL) {
    char *eq = strchr(line, '=');
    if (eq) {
      char *val;
      char *cr;
      *eq = '\0';
      val = eq + 1;
      cr = strchr(val, '\r');
      if (cr)
        *cr = '\0';

      rc = cmp_i18n_add_string(g_global_i18n, locale, line, val);
      if (rc != CMP_SUCCESS) {
        LOG_DEBUG("Error in cmp_i18n_load_catalog: Failed to add string\n");
      }
    }
#if defined(_MSC_VER)
    line = strtok_s(NULL, "\n", &saveptr);
#else
    line = strtok_r(NULL, "\n", &saveptr);
#endif
  }
  CMP_FREE(text);
  return rc;
}

int cmp_i18n_translate(const char *key, cmp_string_t *out_translated) {
  int rc = CMP_SUCCESS;
  char *val = NULL;

  if (!g_i18n_initialized || !out_translated) {
    rc = CMP_ERROR_INVALID_STATE;
    LOG_DEBUG("Error in cmp_i18n_translate: Invalid state or argument\n");
    return rc;
  }

  rc = cmp_i18n_get_string(g_global_i18n, g_current_locale, key, &val);
  if (rc == CMP_SUCCESS) {
    out_translated->data = val;
    out_translated->length = strlen(val);
    out_translated->capacity = out_translated->length + 1;
    return rc;
  }

  /* Fallback: use the key itself */
  rc = str_duplicate(key, &val);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_translate: str_duplicate failed\n");
    return rc;
  }

  out_translated->data = val;
  out_translated->length = strlen(val);
  out_translated->capacity = out_translated->length + 1;
  return rc;
}

int cmp_i18n_translate_plural(const char *key, int count,
                              cmp_string_t *out_translated) {
  int rc = CMP_SUCCESS;
  char plural_key[256];

  if (!key || !out_translated) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_translate_plural: Invalid argument\n");
    return rc;
  }

  if (count == 1) {
    return cmp_i18n_translate(key, out_translated);
  }

#if defined(_MSC_VER)
  sprintf_s(plural_key, sizeof(plural_key), "%s_plural", key);
#else
  sprintf(plural_key, "%s_plural", key);
#endif

  rc = cmp_i18n_translate(plural_key, out_translated);
  return rc;
}

int cmp_i18n_set_bidi_direction(cmp_text_direction_t dir) {
  int rc = CMP_SUCCESS;
  g_bidi_dir = dir;
  return rc;
}

int cmp_i18n_get_bidi_direction(void) { return g_bidi_dir; }

int cmp_i18n_is_rtl(void) {
  return cmp_i18n_get_bidi_direction() == CMP_TEXT_DIR_RTL;
}

typedef union {
  const char *s;
  int d;
} cmp_fmt_arg_val_t;

int cmp_i18n_format(const char *format_str, cmp_string_t *out_str, ...) {
  int rc = CMP_SUCCESS;
  va_list args;
  int i;
  const char *p;
  char buffer[2048];
  char *buf_ptr;
  size_t buf_remain;

  char types[16];
  int max_index = 0;
  cmp_fmt_arg_val_t arg_vals[16];

  if (!format_str || !out_str) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_i18n_format: Invalid argument\n");
    return rc;
  }

  memset(types, 0, sizeof(types));

  p = format_str;
  while (*p) {
    if (*p == '%' && p[1] >= '1' && p[1] <= '9' && p[2] == '$' &&
        (p[3] == 's' || p[3] == 'd')) {
      int idx = p[1] - '0';
      types[idx] = p[3];
      if (idx > max_index)
        max_index = idx;
      p += 4;
    } else {
      p++;
    }
  }

  va_start(args, out_str);
  for (i = 1; i <= max_index; i++) {
    if (types[i] == 's') {
      arg_vals[i].s = va_arg(args, const char *);
    } else if (types[i] == 'd') {
      arg_vals[i].d = va_arg(args, int);
    } else {
      arg_vals[i].d = va_arg(args, int);
    }
  }
  va_end(args);

  p = format_str;
  buffer[0] = '\0';
  buf_ptr = buffer;
  buf_remain = sizeof(buffer) - 1;

  while (*p && buf_remain > 0) {
    if (*p == '%' && p[1] >= '1' && p[1] <= '9' && p[2] == '$' &&
        (p[3] == 's' || p[3] == 'd')) {
      int idx = p[1] - '0';
      if (p[3] == 's') {
        const char *s = arg_vals[idx].s;
        if (s) {
          size_t slen = strlen(s);
          if (slen > buf_remain)
            slen = buf_remain;
#if defined(_MSC_VER)
          strncpy_s(buf_ptr, buf_remain + 1, s, slen);
#else
          strncpy(buf_ptr, s, slen);
#endif
          buf_ptr += slen;
          buf_remain -= slen;
        }
      } else if (p[3] == 'd') {
        char dbuf[32];
        size_t dlen;
#if defined(_MSC_VER)
        sprintf_s(dbuf, sizeof(dbuf), "%d", arg_vals[idx].d);
#else
        sprintf(dbuf, "%d", arg_vals[idx].d);
#endif
        dlen = strlen(dbuf);
        if (dlen > buf_remain)
          dlen = buf_remain;
#if defined(_MSC_VER)
        strncpy_s(buf_ptr, buf_remain + 1, dbuf, dlen);
#else
        strncpy(buf_ptr, dbuf, dlen);
#endif
        buf_ptr += dlen;
        buf_remain -= dlen;
      }
      p += 4;
    } else {
      *buf_ptr++ = *p++;
      buf_remain--;
    }
  }
  *buf_ptr = '\0';

  out_str->length = strlen(buffer);
  out_str->capacity = out_str->length + 1;
  rc = CMP_MALLOC(out_str->capacity, (void **)&out_str->data);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_i18n_format: Out of memory\n");
    return rc;
  }
#if defined(_MSC_VER)
  strcpy_s(out_str->data, out_str->capacity, buffer);
#else
  strcpy(out_str->data, buffer);
#endif

  return rc;
}
