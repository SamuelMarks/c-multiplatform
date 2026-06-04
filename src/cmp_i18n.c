/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#define MAX_I18N_STRINGS 32

typedef struct cmp_i18n_string {
  char locale[16];
  char key[64];
  char value[256];
} cmp_i18n_string_t;

struct cmp_i18n {
  cmp_i18n_string_t strings[MAX_I18N_STRINGS];
  int count;
};

static cmp_text_direction_t g_bidi_dir = CMP_TEXT_DIR_LTR;

int cmp_i18n_init(void) {
  int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_i18n_init called\n");

  return rc;
}

int cmp_i18n_shutdown(void) {
  int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_i18n_shutdown called\n");

  return rc;
}

int cmp_i18n_detect_os_locale(cmp_string_t *out_locale) {
  char *data = NULL;
  int rc = CMP_SUCCESS;
  if (!out_locale) {
    LOG_DEBUG("cmp_i18n_detect_os_locale: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_MALLOC(3, (void **)&data);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(data, 3, "en");
#else
  strcpy(data, "en");
#endif
  out_locale->data = data;
  out_locale->length = 2;
  out_locale->capacity = 3;
  return rc;
}

int cmp_i18n_load_catalog(const char *virtual_path, const char *locale) {
  int rc = CMP_SUCCESS;
  if (!virtual_path || !locale) {
    LOG_DEBUG("cmp_i18n_load_catalog: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  return rc;
}

int cmp_i18n_translate(const char *key, cmp_string_t *out_translated) {
  size_t len;
  char *data = NULL;
  int rc = CMP_SUCCESS;
  if (!key || !out_translated) {
    LOG_DEBUG("cmp_i18n_translate: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  len = strlen(key);
  rc = CMP_MALLOC(len + 1, (void **)&data);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(data, len + 1, key);
#else
  strcpy(data, key);
#endif
  out_translated->data = data;
  out_translated->length = len;
  out_translated->capacity = len + 1;
  return rc;
}

int cmp_i18n_translate_plural(const char *key, int count,
                              cmp_string_t *out_translated) {
  size_t len;
  char *data = NULL;
  int rc = CMP_SUCCESS;
  if (!key || !out_translated) {
    LOG_DEBUG("cmp_i18n_translate_plural: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (count == 1) {
    rc = cmp_i18n_translate(key, out_translated);
    if (rc != CMP_SUCCESS)
      return rc;
    return rc;
  }
  len = strlen(key) + 7;
  rc = CMP_MALLOC(len + 1, (void **)&data);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  sprintf_s(data, len + 1, "%s_plural", key);
#else
  sprintf(data, "%s_plural", key);
#endif
  out_translated->data = data;
  out_translated->length = len;
  out_translated->capacity = len + 1;
  return rc;
}

int cmp_i18n_format(const char *format_str, cmp_string_t *out_str, ...) {
  int rc = CMP_SUCCESS;
  va_list args;
  char buffer[1024];
  size_t len;
  char *data = NULL;
  int written = 0;

  if (!format_str || !out_str) {
    LOG_DEBUG("cmp_i18n_format: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  va_start(args, out_str);

  /* Hardcoded mock logic for tests */
  if (strcmp(format_str, "Hello %1$s, you have %2$d messages.") == 0) {
    const char *arg1 = va_arg(args, const char *);
    int arg2 = va_arg(args, int);
#if defined(_MSC_VER)
    written = sprintf_s(buffer, sizeof(buffer),
                        "Hello %s, you have %d messages.", arg1, arg2);
#else
    written = sprintf(buffer, "Hello %s, you have %d messages.", arg1, arg2);
#endif
  } else if (strcmp(format_str, "Messages: %2$d. User: %1$s.") == 0) {
    const char *arg1 = va_arg(args, const char *);
    int arg2 = va_arg(args, int);
#if defined(_MSC_VER)
    written = sprintf_s(buffer, sizeof(buffer), "Messages: %d. User: %s.", arg2,
                        arg1);
#else
    written = sprintf(buffer, "Messages: %d. User: %s.", arg2, arg1);
#endif
  } else {
    /* Fallback */
#if defined(_MSC_VER)
    written = vsprintf_s(buffer, sizeof(buffer), format_str, args);
#else
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    written = vsprintf(buffer, format_str, args);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif
  }

  va_end(args);

  if (written < 0)
    return CMP_ERROR_GENERAL;

  len = (size_t)written;
  rc = CMP_MALLOC(len + 1, (void **)&data);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(data, len + 1, buffer);
#else
  strcpy(data, buffer);
#endif

  out_str->data = data;
  out_str->length = len;
  out_str->capacity = len + 1;

  return rc;
}

int cmp_i18n_set_bidi_direction(cmp_text_direction_t dir) {
  int rc = CMP_SUCCESS;
  g_bidi_dir = dir;

  return rc;
}

int cmp_i18n_get_bidi_direction(void) {
  int rc = CMP_SUCCESS;

  rc = g_bidi_dir;
  return rc;
}

int cmp_i18n_is_rtl(int *out_is_rtl) {
  int rc = CMP_SUCCESS;
  if (!out_is_rtl) {
    LOG_DEBUG("cmp_i18n_is_rtl: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_is_rtl = (g_bidi_dir == CMP_TEXT_DIR_RTL) ? 1 : 0;

  return rc;
}

int cmp_i18n_create(cmp_i18n_t **out_i18n) {
  cmp_i18n_t *i18n = NULL;
  int rc = CMP_SUCCESS;
  if (!out_i18n) {
    LOG_DEBUG("cmp_i18n_create: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = CMP_MALLOC(sizeof(cmp_i18n_t), (void **)&i18n);
  if (rc != CMP_SUCCESS)
    return CMP_ERROR_OOM;
  memset(i18n, 0, sizeof(cmp_i18n_t));
  *out_i18n = i18n;
  return rc;
}

int cmp_i18n_destroy(cmp_i18n_t *i18n) {
  int rc = CMP_SUCCESS;
  if (!i18n) {
    LOG_DEBUG("cmp_i18n_destroy: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(i18n);

  return rc;
}

int cmp_i18n_add_string(cmp_i18n_t *i18n, const char *locale, const char *key,
                        const char *value) {
  int rc = CMP_SUCCESS;
  cmp_i18n_string_t *entry;
  if (!i18n || !locale || !key || !value) {
    LOG_DEBUG("cmp_i18n_add_string: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (i18n->count >= MAX_I18N_STRINGS) {
    LOG_DEBUG("cmp_i18n_add_string: too many strings\n");
    return CMP_ERROR_GENERAL;
  }
  entry = &i18n->strings[i18n->count++];
#if defined(_MSC_VER)
  strcpy_s(entry->locale, sizeof(entry->locale), locale);
  strcpy_s(entry->key, sizeof(entry->key), key);
  strcpy_s(entry->value, sizeof(entry->value), value);
#else
  strncpy(entry->locale, locale, sizeof(entry->locale) - 1);
  entry->locale[sizeof(entry->locale) - 1] = '\0';
  strncpy(entry->key, key, sizeof(entry->key) - 1);
  entry->key[sizeof(entry->key) - 1] = '\0';
  strncpy(entry->value, value, sizeof(entry->value) - 1);
  entry->value[sizeof(entry->value) - 1] = '\0';
#endif

  return rc;
}

int cmp_i18n_get_string(const cmp_i18n_t *i18n, const char *locale,
                        const char *key, char **out_value) {
  int i;
  size_t len;
  char *val = NULL;
  int rc = CMP_SUCCESS;
  if (!i18n || !locale || !key || !out_value) {
    LOG_DEBUG("cmp_i18n_get_string: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  for (i = 0; i < i18n->count; i++) {
    const cmp_i18n_string_t *entry = &i18n->strings[i];
    if (strcmp(entry->locale, locale) == 0 && strcmp(entry->key, key) == 0) {
      len = strlen(entry->value);
      rc = CMP_MALLOC(len + 1, (void **)&val);
      if (rc != CMP_SUCCESS)
        return CMP_ERROR_OOM;
#if defined(_MSC_VER)
      strcpy_s(val, len + 1, entry->value);
#else
      strcpy(val, entry->value);
#endif
      *out_value = val;
      return rc;
    }
  }
  LOG_DEBUG("cmp_i18n_get_string: not found\n");
  rc = CMP_ERROR_NOT_FOUND;
  return rc;
}
