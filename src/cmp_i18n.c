/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_i18n_entry {
  const char *locale; /* e.g. "en-US", "ar-SA" */
  const char *key;
  const char *value;
} cmp_i18n_entry_t;

/* A trivial mock dictionary for demonstration */
static cmp_i18n_entry_t g_mock_dict[] = {
    {"en", "greeting", "Hello"},
    {"es", "greeting", "Hola"},
    {"ar", "greeting", "مرحبا"},
    {"en", "welcome", "Welcome, %s!"},
    {NULL, NULL, NULL}};

static char g_current_locale[16] = "en";

/**
 * @brief Initialize internationalization subsystem.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_init(void) {
  int rc = 0;
  /* Setup any required ICU/gettext bridging if needed in future */
  return rc;
}

/**
 * @brief Set the current active locale.
 *
 * @param locale The locale string (e.g. "en-US").
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_set_locale(const char *locale) {
  int rc = 0;
  if (!locale) {
    LOG_DEBUG("cmp_i18n_set_locale: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
#if defined(_MSC_VER)
  rc = strcpy_s(g_current_locale, sizeof(g_current_locale), locale);
  if (rc != 0) {
    LOG_DEBUG("cmp_i18n_set_locale: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(g_current_locale, locale, sizeof(g_current_locale) - 1);
  g_current_locale[sizeof(g_current_locale) - 1] = '\0';
#endif
  return 0;
}

/**
 * @brief Get the currently active locale.
 *
 * @param out_locale Pointer to receive the locale string pointer.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_get_locale(const char **out_locale) {
  if (!out_locale) {
    LOG_DEBUG("cmp_i18n_get_locale: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_locale = g_current_locale;
  return 0;
}

/**
 * @brief Retrieve a localized string by key.
 *
 * @param key The string key.
 * @param out_str Pointer to receive the localized string.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_get_string(const char *key, const char **out_str) {
  int i = 0;
  if (!key || !out_str) {
    LOG_DEBUG("cmp_i18n_get_string: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  *out_str = key; /* fallback to key */

  while (g_mock_dict[i].key != NULL) {
    if (strncmp(g_mock_dict[i].locale, g_current_locale, 2) == 0 &&
        strcmp(g_mock_dict[i].key, key) == 0) {
      *out_str = g_mock_dict[i].value;
      return 0;
    }
    i++;
  }
  return CMP_ERROR_NOT_FOUND;
}

/**
 * @brief Get text direction based on current locale.
 *
 * @param out_dir Pointer to receive the text direction.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_get_bidi_direction(cmp_text_direction_t *out_dir) {
  if (!out_dir) {
    LOG_DEBUG("cmp_i18n_get_bidi_direction: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  if (strncmp(g_current_locale, "ar", 2) == 0 ||
      strncmp(g_current_locale, "he", 2) == 0 ||
      strncmp(g_current_locale, "fa", 2) == 0 ||
      strncmp(g_current_locale, "ur", 2) == 0) {
    *out_dir = CMP_TEXT_DIR_RTL;
    return 0;
  }
  *out_dir = CMP_TEXT_DIR_LTR;
  return 0;
}

/**
 * @brief Check if the current locale is Right-To-Left.
 *
 * @param out_is_rtl Pointer to receive 1 if RTL, 0 otherwise.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_is_rtl(int *out_is_rtl) {
  int rc;
  cmp_text_direction_t dir;
  if (!out_is_rtl) {
    LOG_DEBUG("cmp_i18n_is_rtl: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  rc = cmp_i18n_get_bidi_direction(&dir);
  if (rc != 0) {
    *out_is_rtl = 0;
    return rc;
  }
  *out_is_rtl = (dir == CMP_TEXT_DIR_RTL) ? 1 : 0;
  return 0;
}

/**
 * @brief Format a localized string with arguments.
 *
 * @param key The localization key.
 * @param out_str Pointer to the string struct to receive formatted text.
 * @param ... Formatting arguments.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_i18n_format(const char *key, cmp_string_t *out_str, ...) {
  int rc = 0;
  const char *fmt = NULL;
  va_list args;
  char buffer[1024];
  int written;

  if (!key || !out_str) {
    LOG_DEBUG("cmp_i18n_format: invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = cmp_i18n_get_string(key, &fmt);
  if (rc != 0) {
    fmt = key; /* fallback to key */
  }

  va_start(args, out_str);
#if defined(_MSC_VER)
  written = vsprintf_s(buffer, sizeof(buffer), fmt, args);
#else
  written = vsnprintf(buffer, sizeof(buffer), fmt, args);
#endif
  va_end(args);

  if (written < 0 || (size_t)written >= sizeof(buffer)) {
    LOG_DEBUG("cmp_i18n_format: format error or truncation\n");
    return CMP_ERROR_GENERAL;
  }

  out_str->length = (size_t)written;
  out_str->capacity = out_str->length + 1;
  rc = CMP_MALLOC(out_str->capacity, (void **)&out_str->data);
  if (rc != 0) {
    LOG_DEBUG("Error in cmp_i18n_format: Out of memory\n");
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  rc = strcpy_s(out_str->data, out_str->capacity, buffer);
  if (rc != 0) {
    LOG_DEBUG("cmp_i18n_format: strcpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strcpy(out_str->data, buffer);
#endif

  return 0;
}
