#include "cmpc/cmp_i18n.h"

#include "cmpc/cmp_datetime.h"
#include "cmpc/cmp_utf8.h"

#include <string.h>

#define CMP_I18N_USIZE_MAX ((cmp_usize) ~(cmp_usize)0)

#define CMP_I18N_RETURN_IF_ERROR(rc)                                           \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#ifdef CMP_TESTING
static cmp_usize g_cmp_i18n_cstr_limit = 0;
static int g_cmp_i18n_force_pow10_error = 0;
static int g_cmp_i18n_force_leap_error = 0;
static int g_cmp_i18n_force_days_default = 0;
static cmp_u32 g_cmp_i18n_ascii_lower_fail_after = 0;
static int g_cmp_i18n_force_utf8_error = 0;
static cmp_u32 g_cmp_i18n_force_utf8_ok = 0u;
static int g_cmp_i18n_force_config_init_error = 0;
#endif

static int cmp_i18n_mul_overflow(cmp_usize a, cmp_usize b,
                                 cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = CMP_I18N_USIZE_MAX;
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_i18n_add_overflow(cmp_usize a, cmp_usize b,
                                 cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = CMP_I18N_USIZE_MAX;
  if (b > max_value - a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return CMP_OK;
}

static int cmp_i18n_validate_bool(CMPBool value) {
  if (value != CMP_FALSE && value != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_i18n_validate_locale(const CMPI18nLocale *locale) {
  int rc;

  if (locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (locale->decimal_separator == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (locale->date_separator == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (locale->time_separator == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (locale->grouping != 0 && locale->thousands_separator == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (locale->grouping > 9u) {
    return CMP_ERR_RANGE;
  }

  if (locale->date_order != CMP_I18N_DATE_ORDER_MDY &&
      locale->date_order != CMP_I18N_DATE_ORDER_DMY &&
      locale->date_order != CMP_I18N_DATE_ORDER_YMD) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (locale->time_format != CMP_I18N_TIME_FORMAT_24H &&
      locale->time_format != CMP_I18N_TIME_FORMAT_12H) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_bool(locale->pad_day);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_bool(locale->pad_month);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_bool(locale->pad_hour);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_bool(locale->pad_minute);
  CMP_I18N_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_i18n_pow10(cmp_u32 digits, cmp_u32 *out_value) {
  cmp_u32 value;
  cmp_u32 i;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (g_cmp_i18n_force_pow10_error != 0) {
    g_cmp_i18n_force_pow10_error = 0;
    return CMP_ERR_IO;
  }
#endif

  value = 1u;
  for (i = 0; i < digits; ++i) {
    if (value > 0xFFFFFFFFu / 10u) {
      return CMP_ERR_OVERFLOW;
    }
    value *= 10u;
  }

  *out_value = value;
  return CMP_OK;
}

static int cmp_i18n_validate_number(const CMPI18nNumber *number) {
  cmp_u32 limit;
  int rc;

  if (number == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (number->fraction_digits > CMP_I18N_MAX_FRACTION_DIGITS) {
    return CMP_ERR_RANGE;
  }

  if (number->fraction_digits == 0 && number->fraction != 0) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_i18n_pow10(number->fraction_digits, &limit);
  if (rc != CMP_OK) {
    return rc;
  }

  if (number->fraction_digits > 0 && number->fraction >= limit) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int cmp_i18n_is_leap_year(cmp_i32 year, CMPBool *out_leap) {
  if (out_leap == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_i18n_force_leap_error != 0) {
    g_cmp_i18n_force_leap_error = 0;
    return CMP_ERR_IO;
  }
#endif
  if ((year % 400) == 0) {
    *out_leap = CMP_TRUE;
  } else if ((year % 100) == 0) {
    *out_leap = CMP_FALSE;
  } else if ((year % 4) == 0) {
    *out_leap = CMP_TRUE;
  } else {
    *out_leap = CMP_FALSE;
  }
  return CMP_OK;
}

static int cmp_i18n_days_in_month(cmp_i32 year, cmp_u32 month,
                                  cmp_u32 *out_days) {
  CMPBool leap;
  int rc;

  if (out_days == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (month < 1u || month > 12u) {
    return CMP_ERR_RANGE;
  }
#ifdef CMP_TESTING /* GCOVR_EXCL_LINE */
  if (g_cmp_i18n_force_days_default != 0) {
    g_cmp_i18n_force_days_default = 0;
    month = 13u;
  }
#endif /* GCOVR_EXCL_LINE */

  switch (month) {
  case 1u: /* GCOVR_EXCL_LINE */
  case 3u:
  case 5u:
  case 7u:
  case 8u:
  case 10u:
  case 12u:
    *out_days = 31u;
    return CMP_OK;
  case 4u:
  case 6u:
  case 9u:
  case 11u:
    *out_days = 30u;
    return CMP_OK;
  case 2u:
    rc = cmp_i18n_is_leap_year(year, &leap);
    if (rc != CMP_OK) {
      return rc;
    }
    *out_days = (leap == CMP_TRUE) ? 29u : 28u;
    return CMP_OK;
  default:
    break;
  }

  return CMP_ERR_RANGE;
}

static int cmp_i18n_validate_date(const CMPDate *date) {
  cmp_u32 days_in_month;
  int rc;

  if (date == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (date->year < CMP_DATE_MIN_YEAR || date->year > CMP_DATE_MAX_YEAR) {
    return CMP_ERR_RANGE;
  }
  if (date->month < 1u || date->month > 12u) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_i18n_days_in_month(date->year, date->month, &days_in_month);
  if (rc != CMP_OK) {
    return rc;
  }
  if (date->day < 1u || date->day > days_in_month) {
    return CMP_ERR_RANGE;
  }

  return CMP_OK;
}

static int cmp_i18n_validate_time(const CMPTime *time) {
  if (time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (time->hour > 23u) {
    return CMP_ERR_RANGE;
  }
  if (time->minute > 59u) {
    return CMP_ERR_RANGE;
  }
  return CMP_OK;
}

static int cmp_i18n_cstrlen(const char *cstr, cmp_usize *out_len) {
  cmp_usize max_len;
  cmp_usize len;

  if (cstr == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_len = CMP_I18N_USIZE_MAX;
#ifdef CMP_TESTING
  if (g_cmp_i18n_cstr_limit != 0) {
    max_len = g_cmp_i18n_cstr_limit;
  }
#endif

  len = 0;
  while (cstr[len] != '\0') {
    if (len == max_len) {
      return CMP_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return CMP_OK;
}

static int cmp_i18n_ascii_lower(char ch, char *out_lower) {
  if (out_lower == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_i18n_ascii_lower_fail_after > 0u) {
    g_cmp_i18n_ascii_lower_fail_after -= 1u;
    if (g_cmp_i18n_ascii_lower_fail_after == 0u) {
      return CMP_ERR_IO;
    }
  }
#endif /* GCOVR_EXCL_LINE */

  if (ch >= 'A' && ch <= 'Z') {
    *out_lower = (char)(ch - 'A' + 'a');
  } else {
    *out_lower = ch;
  }
  return CMP_OK;
}

static int cmp_i18n_tag_equals(const char *a, const char *b,
                               CMPBool *out_equal) {
  cmp_usize index;
  char ca;
  char cb;
  int rc;

  if (a == NULL || b == NULL || out_equal == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  index = 0;
  while (a[index] != '\0' || b[index] != '\0') {
    if (a[index] == '\0' || b[index] == '\0') {
      *out_equal = CMP_FALSE;
      return CMP_OK;
    }

    ca = a[index];
    cb = b[index];
    if (ca == '_') {
      ca = '-';
    }
    if (cb == '_') {
      cb = '-';
    }

    rc = cmp_i18n_ascii_lower(ca, &ca);
    if (rc != CMP_OK) {
      return rc;
    }
    rc = cmp_i18n_ascii_lower(cb, &cb);
    if (rc != CMP_OK) {
      return rc;
    }

    if (ca != cb) {
      *out_equal = CMP_FALSE;
      return CMP_OK;
    }

    index += 1;
  }

  *out_equal = CMP_TRUE;
  return CMP_OK;
}

static int cmp_i18n_locale_preset_en_us(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = '.';
  out_locale->thousands_separator = ',';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = CMP_I18N_DATE_ORDER_MDY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_12H;
  out_locale->direction = CMP_I18N_DIRECTION_LTR;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_FALSE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return CMP_OK;
}

static int cmp_i18n_locale_preset_en_gb(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = '.';
  out_locale->thousands_separator = ',';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = CMP_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_24H;
  out_locale->direction = CMP_I18N_DIRECTION_LTR;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_TRUE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return CMP_OK;
}

static int cmp_i18n_locale_preset_fr_fr(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = ',';
  out_locale->thousands_separator = ' ';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = CMP_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_24H;
  out_locale->direction = CMP_I18N_DIRECTION_LTR;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_TRUE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return CMP_OK;
}

static int cmp_i18n_locale_preset_ar_sa(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = ',';
  out_locale->thousands_separator = '.';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = CMP_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_12H;
  out_locale->direction = CMP_I18N_DIRECTION_RTL;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_FALSE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "ص";
  out_locale->pm = "م";
  return CMP_OK;
}

static int cmp_i18n_locale_preset_he_il(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = '.';
  out_locale->thousands_separator = ',';
  out_locale->grouping = 3u;
  out_locale->date_separator = '.';
  out_locale->date_order = CMP_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_24H;
  out_locale->direction = CMP_I18N_DIRECTION_RTL;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_TRUE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return CMP_OK;
}

static int cmp_i18n_locale_preset_de_de(CMPI18nLocale *out_locale) {
  if (out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = ',';
  out_locale->thousands_separator = '.';
  out_locale->grouping = 3u;
  out_locale->date_separator = '.';
  out_locale->date_order = CMP_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = CMP_I18N_TIME_FORMAT_24H;
  out_locale->direction = CMP_I18N_DIRECTION_LTR;
  out_locale->pad_day = CMP_TRUE;
  out_locale->pad_month = CMP_TRUE;
  out_locale->pad_hour = CMP_TRUE;
  out_locale->pad_minute = CMP_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return CMP_OK;
}

static int cmp_i18n_validate_key(const char *key, cmp_usize key_len) {
  if (key == NULL || key_len == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_OK;
}

static int cmp_i18n_validate_utf8(const char *data, cmp_usize length) {
  CMPBool valid;
  int rc;

#ifdef CMP_TESTING
  if (g_cmp_i18n_force_utf8_ok > 0u) {
    g_cmp_i18n_force_utf8_ok -= 1u;
    return CMP_OK;
  }
#endif
  if (data == NULL && length != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (length == 0) {
    return CMP_OK;
  }

  rc = cmp_utf8_validate(data, length, &valid);
#ifdef CMP_TESTING
  if (g_cmp_i18n_force_utf8_error != 0) {
    g_cmp_i18n_force_utf8_error = 0;
    rc = CMP_ERR_IO;
  }
#endif
  if (rc != CMP_OK) {
    return rc;
  }
  if (valid != CMP_TRUE) {
    return CMP_ERR_CORRUPT;
  }

  return CMP_OK;
}

static int cmp_i18n_find_entry(const CMPI18n *i18n, const char *key,
                               cmp_usize key_len, cmp_usize *out_index,
                               CMPBool *out_found) {
  cmp_usize i;

  if (out_index == NULL || out_found == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_index = 0;
  *out_found = CMP_FALSE;

  for (i = 0; i < i18n->entry_count; ++i) {
    const CMPI18nEntry *entry = &i18n->entries[i];

    if (entry->key_len == key_len && entry->key != NULL &&
        memcmp(entry->key, key, (size_t)key_len) == 0) {
      *out_index = i;
      *out_found = CMP_TRUE;
      return CMP_OK;
    }
  }

  return CMP_OK;
}

static int cmp_i18n_entry_release(CMPI18n *i18n, CMPI18nEntry *entry) {
  int rc;
  int free_rc;

  rc = CMP_OK;

  if (entry->key != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, entry->key);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
    entry->key = NULL;
  }

  if (entry->value != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
    entry->value = NULL;
  }

  entry->key_len = 0;
  entry->value_len = 0;
  return rc;
}

static int cmp_i18n_grow(CMPI18n *i18n, cmp_usize min_capacity) {
  cmp_usize new_capacity;
  cmp_usize alloc_size;
  cmp_usize clear_count;
  void *new_entries;
  int rc;

  if (i18n->allocator.realloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  new_capacity = i18n->entry_capacity;
  while (new_capacity < min_capacity) {
    rc = cmp_i18n_mul_overflow(new_capacity, (cmp_usize)2, &new_capacity);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  rc = cmp_i18n_mul_overflow(new_capacity, (cmp_usize)sizeof(CMPI18nEntry),
                             &alloc_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = i18n->allocator.realloc(i18n->allocator.ctx, i18n->entries, alloc_size,
                               &new_entries);
  if (rc != CMP_OK) {
    return rc;
  }

  clear_count = new_capacity - i18n->entry_capacity;
  if (clear_count > 0) {
    memset((CMPI18nEntry *)new_entries + i18n->entry_capacity, 0,
           (size_t)(clear_count * sizeof(CMPI18nEntry)));
  }

  i18n->entries = (CMPI18nEntry *)new_entries;
  i18n->entry_capacity = new_capacity;
  return CMP_OK;
}

static int cmp_i18n_alloc_string(const CMPAllocator *allocator, const char *src,
                                 cmp_usize len, char **out_str) {
  cmp_usize bytes;
  void *mem;
  int rc;

  if (allocator == NULL || out_str == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (src == NULL && len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_add_overflow(len, (cmp_usize)1, &bytes);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, bytes, &mem);
  if (rc != CMP_OK) {
    return rc;
  }

  if (len > 0) {
    memcpy(mem, src, (size_t)len);
  }
  ((char *)mem)[len] = '\0';

  *out_str = (char *)mem;
  return CMP_OK;
}

static int cmp_i18n_write_uint(char *out_text, cmp_usize text_capacity,
                               cmp_usize *io_offset, cmp_u32 value,
                               cmp_u32 min_digits) {
  char digits[16];
  cmp_usize digit_count;
  cmp_usize i;
  cmp_usize required;

  if (out_text == NULL || io_offset == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  digit_count = 0;
  do {
    digits[digit_count] = (char)('0' + (value % 10u));
    value /= 10u;
    digit_count += 1;
  } while (value > 0u);

  while (digit_count < (cmp_usize)min_digits) {
    digits[digit_count] = '0';
    digit_count += 1;
  }

  required = *io_offset + digit_count;
  if (required >= text_capacity) {
    return CMP_ERR_RANGE;
  }

  for (i = 0; i < digit_count; ++i) {
    out_text[*io_offset + i] = digits[digit_count - 1 - i];
  }

  *io_offset += digit_count;
  return CMP_OK;
}

static int cmp_i18n_format_number_internal(const CMPI18nLocale *locale,
                                           const CMPI18nNumber *number,
                                           char *out_text,
                                           cmp_usize text_capacity,
                                           cmp_usize *out_len) {
  char digits[16];
  cmp_u32 magnitude;
  cmp_usize digit_count;
  cmp_usize sep_count;
  cmp_usize total_len;
  cmp_usize i;
  cmp_usize offset;
  cmp_usize group_size;
  int rc;

  if (locale == NULL || number == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_locale(locale);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_number(number);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (number->integer < 0) {
    magnitude = (cmp_u32)(-(number->integer + 1)) + 1u;
  } else {
    magnitude = (cmp_u32)number->integer;
  }

  digit_count = 0;
  do {
    digits[digit_count] = (char)('0' + (magnitude % 10u));
    magnitude /= 10u;
    digit_count += 1;
  } while (magnitude > 0u);

  for (i = 0; i < digit_count / 2u; ++i) {
    char tmp = digits[i];
    digits[i] = digits[digit_count - 1u - i];
    digits[digit_count - 1u - i] = tmp;
  }

  group_size = (cmp_usize)locale->grouping;
  sep_count = 0;
  if (group_size > 0u && locale->thousands_separator != '\0') {
    sep_count = (digit_count - 1u) / group_size;
  }

  total_len = digit_count + sep_count;
  if (number->integer < 0) {
    total_len += 1u;
  }
  if (number->fraction_digits > 0u) {
    total_len += 1u + (cmp_usize)number->fraction_digits;
  }

  *out_len = total_len;
  if (text_capacity <= total_len) {
    return CMP_ERR_RANGE;
  }

  offset = 0;
  if (number->integer < 0) {
    out_text[offset] = '-';
    offset += 1u;
  }

  for (i = 0; i < digit_count; ++i) {
    if (group_size > 0u && sep_count > 0u && i > 0u &&
        ((digit_count - i) % group_size) == 0u) {
      out_text[offset] = locale->thousands_separator;
      offset += 1u;
    }
    out_text[offset] = digits[i];
    offset += 1u;
  }

  if (number->fraction_digits > 0u) {
    cmp_u32 fraction;
    cmp_u32 scale;
    cmp_u32 digit;
    cmp_u32 idx;

    out_text[offset] = locale->decimal_separator;
    offset += 1u;

    fraction = number->fraction;
    rc = cmp_i18n_pow10(number->fraction_digits, &scale);
    CMP_I18N_RETURN_IF_ERROR(rc);

    for (idx = number->fraction_digits; idx > 0u; --idx) {
      scale /= 10u;
      digit = (fraction / scale) % 10u;
      out_text[offset] = (char)('0' + digit);
      offset += 1u;
    }
  }

  out_text[offset] = '\0';
  return CMP_OK;
}

static int cmp_i18n_format_date_internal(const CMPI18nLocale *locale,
                                         const CMPDate *date, char *out_text,
                                         cmp_usize text_capacity,
                                         cmp_usize *out_len) {
  cmp_u32 year_value;
  cmp_u32 year_digits;
  cmp_u32 year_min_digits;
  cmp_u32 day_digits;
  cmp_u32 month_digits;
  cmp_usize total_len;
  cmp_usize offset;
  int rc;

  if (locale == NULL || date == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_locale(locale);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_date(date);
  CMP_I18N_RETURN_IF_ERROR(rc);

  year_value = (cmp_u32)date->year;
  year_digits = 1u;
  while (year_value >= 10u) {
    year_value /= 10u;
    year_digits += 1u;
  }
  year_min_digits = 4u;
  if (year_digits > year_min_digits) {
    year_min_digits = year_digits;
  }

  day_digits = (locale->pad_day == CMP_TRUE) ? 2u : 1u;
  month_digits = (locale->pad_month == CMP_TRUE) ? 2u : 1u;

  total_len = (cmp_usize)year_min_digits + (cmp_usize)day_digits +
              (cmp_usize)month_digits + 2u;
  *out_len = total_len;
  if (text_capacity <= total_len) {
    return CMP_ERR_RANGE;
  }

  offset = 0;

  if (locale->date_order == CMP_I18N_DATE_ORDER_MDY) {
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                             month_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                             day_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset,
                             (cmp_u32)date->year, year_min_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
  } else if (locale->date_order == CMP_I18N_DATE_ORDER_DMY) {
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                             day_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                             month_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset,
                             (cmp_u32)date->year, year_min_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
  } else {
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset,
                             (cmp_u32)date->year, year_min_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                             month_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                             day_digits);
    CMP_I18N_RETURN_IF_ERROR(rc);
  }

  out_text[offset] = '\0';
  return CMP_OK;
}

static int cmp_i18n_format_time_internal(const CMPI18nLocale *locale,
                                         const CMPTime *time, char *out_text,
                                         cmp_usize text_capacity,
                                         cmp_usize *out_len) {
  cmp_u32 hour_value;
  cmp_u32 hour_digits;
  cmp_u32 minute_digits;
  cmp_usize total_len;
  cmp_usize offset;
  const char *designator;
  cmp_usize designator_len;
  int rc;

  if (locale == NULL || time == NULL || out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_locale(locale);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_time(time);
  CMP_I18N_RETURN_IF_ERROR(rc);

  designator = NULL;
  designator_len = 0;
  hour_value = time->hour;

  if (locale->time_format == CMP_I18N_TIME_FORMAT_12H) {
    if (hour_value < 12u) {
      designator = locale->am;
    } else {
      designator = locale->pm;
    }

    hour_value = hour_value % 12u;
    if (hour_value == 0u) {
      hour_value = 12u;
    }
  }

  if (locale->pad_hour == CMP_TRUE) {
    hour_digits = 2u;
  } else {
    hour_digits = (hour_value >= 10u) ? 2u : 1u;
  }
  minute_digits = (locale->pad_minute == CMP_TRUE) ? 2u : 1u;

  total_len = (cmp_usize)hour_digits + (cmp_usize)minute_digits + 1u;

  if (designator != NULL) {
    rc = cmp_i18n_cstrlen(designator, &designator_len);
    CMP_I18N_RETURN_IF_ERROR(rc);
    if (designator_len > 0) {
      total_len += 1u + designator_len;
    }
  }

  *out_len = total_len;
  if (text_capacity <= total_len) {
    return CMP_ERR_RANGE;
  }

  offset = 0;
  rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, hour_value,
                           hour_digits);
  CMP_I18N_RETURN_IF_ERROR(rc);
  out_text[offset] = locale->time_separator;
  offset += 1u;
  rc = cmp_i18n_write_uint(out_text, text_capacity, &offset, time->minute,
                           minute_digits);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (designator != NULL && designator_len > 0) {
    out_text[offset] = ' ';
    offset += 1u;
    memcpy(out_text + offset, designator, (size_t)designator_len);
    offset += designator_len;
  }

  out_text[offset] = '\0';
  return CMP_OK;
}

static int cmp_i18n_set_locale_internal(CMPI18n *i18n, const char *locale_tag,
                                        const CMPI18nLocale *locale) {
  CMPI18nLocale next_locale;
  char *next_tag;
  cmp_usize tag_len = 0u;
  int rc = CMP_OK;
  int free_rc = CMP_OK;
  int cleanup_rc = CMP_OK;

  if (i18n == NULL || locale_tag == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_cstrlen(locale_tag, &tag_len);
  if (rc != CMP_OK) {
    return rc;
  }
  if (tag_len == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_utf8(locale_tag, tag_len);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_i18n_alloc_string(&i18n->allocator, locale_tag, tag_len, &next_tag);
  if (rc != CMP_OK) {
    return rc;
  }

  if (locale != NULL) {
    rc = cmp_i18n_validate_locale(locale);
    if (rc != CMP_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      CMP_UNUSED(free_rc);
      return rc;
    }
    next_locale = *locale;
  } else {
    rc = cmp_i18n_locale_from_tag(locale_tag, &next_locale);
    if (rc != CMP_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      CMP_UNUSED(free_rc);
      return rc;
    }
  }

  if (i18n->locale_tag != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->locale_tag);
    if (free_rc != CMP_OK) {
      cleanup_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      CMP_UNUSED(cleanup_rc);
      return free_rc;
    }
  }

  i18n->locale_tag = next_tag;
  i18n->locale_tag_len = tag_len;
  i18n->locale = next_locale;
  return CMP_OK;
}

static int cmp_i18n_trim_span(const char *data, cmp_usize length,
                              cmp_usize *out_start, cmp_usize *out_length) {
  cmp_usize start;
  cmp_usize end;

  if (data == NULL || out_start == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  start = 0;
  while (start < length) {
    char ch = data[start];
    if (ch != ' ' && ch != '\t') {
      break;
    }
    start += 1u;
  }

  end = length;
  while (end > start) {
    char ch = data[end - 1u];
    if (ch != ' ' && ch != '\t') {
      break;
    }
    end -= 1u;
  }

  *out_start = start;
  *out_length = end - start;
  return CMP_OK;
}

static int cmp_i18n_parse_table(CMPI18n *i18n, const char *data, cmp_usize size,
                                CMPBool clear_existing, CMPBool overwrite) {
  cmp_usize offset;
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_validate_bool(clear_existing);
  CMP_I18N_RETURN_IF_ERROR(rc);
  rc = cmp_i18n_validate_bool(overwrite);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (clear_existing == CMP_TRUE) {
    rc = cmp_i18n_clear(i18n);
    CMP_I18N_RETURN_IF_ERROR(rc);
  }

  if (size == 0) {
    return CMP_OK;
  }

  offset = 0;
  if (size >= 3u && (cmp_u8)data[0] == 0xEFu && (cmp_u8)data[1] == 0xBBu &&
      (cmp_u8)data[2] == 0xBFu) {
    offset = 3u;
  }

  while (offset < size) {
    const char *line;
    cmp_usize line_start;
    cmp_usize line_len;
    cmp_usize trim_start;
    cmp_usize trim_len;
    cmp_usize eq_index;
    cmp_usize key_start;
    cmp_usize key_len;
    cmp_usize value_start;
    cmp_usize value_len;

    line_start = offset;
    while (offset < size && data[offset] != '\n') {
      offset += 1u;
    }
    line_len = offset - line_start;
    if (offset < size && data[offset] == '\n') {
      offset += 1u;
    }

    if (line_len == 0) {
      continue;
    }

    line = data + line_start;
    if (line_len > 0 && line[line_len - 1u] == '\r') {
      line_len -= 1u;
    }

    rc = cmp_i18n_trim_span(line, line_len, &trim_start, &trim_len);
    CMP_I18N_RETURN_IF_ERROR(rc);
    if (trim_len == 0) {
      continue;
    }

    if (line[trim_start] == '#' || line[trim_start] == ';') {
      continue;
    }

    eq_index = trim_start;
    while (eq_index < trim_start + trim_len && line[eq_index] != '=') {
      eq_index += 1u;
    }
    if (eq_index >= trim_start + trim_len) {
      return CMP_ERR_CORRUPT;
    }

    rc = cmp_i18n_trim_span(line + trim_start, eq_index - trim_start,
                            &key_start, &key_len);
    CMP_I18N_RETURN_IF_ERROR(rc);
    if (key_len == 0) {
      return CMP_ERR_CORRUPT;
    }
    key_start += trim_start;

    rc = cmp_i18n_trim_span(line + eq_index + 1u,
                            trim_start + trim_len - (eq_index + 1u),
                            &value_start, &value_len);
    CMP_I18N_RETURN_IF_ERROR(rc);
    value_start += eq_index + 1u;

    rc = cmp_i18n_put(i18n, line + key_start, key_len, line + value_start,
                      value_len, overwrite);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_i18n_locale_init(CMPI18nLocale *locale) {
  if (locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_i18n_locale_preset_en_us(locale);
}

int CMP_CALL cmp_i18n_locale_from_tag(const char *locale_tag,
                                      CMPI18nLocale *out_locale) {
  CMPBool match;
  int rc;

  if (locale_tag == NULL || out_locale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_tag_equals(locale_tag, "en-US", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_en_us(out_locale);
  }

  rc = cmp_i18n_tag_equals(locale_tag, "en-GB", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_en_gb(out_locale);
  }

  rc = cmp_i18n_tag_equals(locale_tag, "fr-FR", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_fr_fr(out_locale);
  }

  rc = cmp_i18n_tag_equals(locale_tag, "ar-SA", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_ar_sa(out_locale);
  }

  rc = cmp_i18n_tag_equals(locale_tag, "he-IL", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_he_il(out_locale);
  }

  rc = cmp_i18n_tag_equals(locale_tag, "de-DE", &match);
  if (rc != CMP_OK) {
    return rc;
  }
  if (match == CMP_TRUE) {
    return cmp_i18n_locale_preset_de_de(out_locale);
  }

  return CMP_ERR_NOT_FOUND;
}

int CMP_CALL cmp_i18n_config_init(CMPI18nConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#ifdef CMP_TESTING
  if (g_cmp_i18n_force_config_init_error != 0) {
    g_cmp_i18n_force_config_init_error = 0;
    return CMP_ERR_IO;
  }
#endif /* GCOVR_EXCL_LINE */

  config->allocator = NULL;
  config->entry_capacity = (cmp_usize)CMP_I18N_DEFAULT_CAPACITY;
  config->locale_tag = CMP_I18N_DEFAULT_LOCALE_TAG;
  config->locale = NULL;
  config->formatter = NULL;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_init(CMPI18n *i18n, const CMPI18nConfig *config) {
  CMPI18nConfig default_config;
  CMPAllocator allocator;
  CMPI18nEntry *entries;
  cmp_usize alloc_size;
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (i18n->entries != NULL) {
    return CMP_ERR_STATE;
  }

  if (config == NULL) {
    rc = cmp_i18n_config_init(&default_config);
    if (rc != CMP_OK) {
      return rc;
    }
    config = &default_config;
  }

  if (config->entry_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_mul_overflow(config->entry_capacity,
                             (cmp_usize)sizeof(CMPI18nEntry), &alloc_size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = allocator.alloc(allocator.ctx, alloc_size, (void **)&entries);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(entries, 0, (size_t)alloc_size);

  i18n->allocator = allocator;
  i18n->entries = entries;
  i18n->entry_capacity = config->entry_capacity;
  i18n->entry_count = 0;
  i18n->locale_tag = NULL;
  i18n->locale_tag_len = 0;
  memset(&i18n->locale, 0, sizeof(i18n->locale));
  memset(&i18n->formatter, 0, sizeof(i18n->formatter));

  if (config->formatter != NULL) {
    if (config->formatter->vtable == NULL) {
      rc = cmp_i18n_shutdown(i18n);
      CMP_UNUSED(rc);
      return CMP_ERR_INVALID_ARGUMENT;
    }
    i18n->formatter = *config->formatter;
  }

  if (config->locale != NULL || config->locale_tag != NULL) {
    const char *tag = config->locale_tag;
    if (tag == NULL) {
      tag = CMP_I18N_DEFAULT_LOCALE_TAG;
    }
    rc = cmp_i18n_set_locale_internal(i18n, tag, config->locale);
    if (rc != CMP_OK) {
      int shutdown_rc = cmp_i18n_shutdown(i18n);
      CMP_UNUSED(shutdown_rc);
      return rc;
    }
  } else {
    rc = cmp_i18n_set_locale_internal(i18n, CMP_I18N_DEFAULT_LOCALE_TAG, NULL);
    if (rc != CMP_OK) {
      int shutdown_rc = cmp_i18n_shutdown(i18n);
      CMP_UNUSED(shutdown_rc);
      return rc;
    }
  }

  return CMP_OK;
}

int CMP_CALL cmp_i18n_shutdown(CMPI18n *i18n) {
  cmp_usize i;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (i18n->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = CMP_OK;

  for (i = 0; i < i18n->entry_count; ++i) {
    free_rc = cmp_i18n_entry_release(i18n, &i18n->entries[i]);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
  }

  free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->entries);
  if (free_rc != CMP_OK && rc == CMP_OK) {
    rc = free_rc;
  }

  i18n->entries = NULL;
  i18n->entry_capacity = 0;
  i18n->entry_count = 0;

  if (i18n->locale_tag != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->locale_tag);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
  }

  i18n->locale_tag = NULL;
  i18n->locale_tag_len = 0;
  memset(&i18n->locale, 0, sizeof(i18n->locale));
  memset(&i18n->formatter, 0, sizeof(i18n->formatter));

  i18n->allocator.ctx = NULL;
  i18n->allocator.alloc = NULL;
  i18n->allocator.realloc = NULL;
  i18n->allocator.free = NULL;
  return rc;
}

int CMP_CALL cmp_i18n_set_locale(CMPI18n *i18n, const char *locale_tag,
                                 const CMPI18nLocale *locale) {
  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_i18n_set_locale_internal(i18n, locale_tag, locale);
}

int CMP_CALL cmp_i18n_set_formatter(CMPI18n *i18n,
                                    const CMPI18nFormatter *formatter) {
  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  if (formatter == NULL) {
    memset(&i18n->formatter, 0, sizeof(i18n->formatter));
    return CMP_OK;
  }
  if (formatter->vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  i18n->formatter = *formatter;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_i18n_get_direction(const CMPI18n *i18n,
                                            cmp_u32 *out_direction) {
  if (i18n == NULL || out_direction == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_direction = i18n->locale.direction;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_put(CMPI18n *i18n, const char *utf8_key,
                          cmp_usize key_len, const char *utf8_value,
                          cmp_usize value_len, CMPBool overwrite) {
  CMPI18nEntry *entry;
  cmp_usize index = 0u;
  CMPBool found; /* GCOVR_EXCL_LINE */
  char *key_copy;
  char *value_copy;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_i18n_validate_bool(overwrite);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_validate_key(utf8_key, key_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_validate_utf8(utf8_key, key_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_validate_utf8(utf8_value, value_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (utf8_value == NULL && value_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (found == CMP_TRUE) {
    if (overwrite == CMP_FALSE) {
      return CMP_ERR_BUSY;
    }

    entry = &i18n->entries[index];
    if (value_len == 0) {
      if (entry->value != NULL) {
        rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
        if (rc != CMP_OK) {
          return rc;
        }
      }
      entry->value = NULL;
      entry->value_len = 0;
      return CMP_OK;
    }

    rc = cmp_i18n_alloc_string(&i18n->allocator, utf8_value, value_len,
                               &value_copy);
    if (rc != CMP_OK) {
      return rc;
    }

    if (entry->value != NULL) {
      rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
      if (rc != CMP_OK) {
        free_rc = i18n->allocator.free(i18n->allocator.ctx, value_copy);
        CMP_UNUSED(free_rc);
        return rc;
      }
    }

    entry->value = value_copy;
    entry->value_len = value_len;
    return CMP_OK;
  }

  if (i18n->entry_count >= i18n->entry_capacity) {
    rc = cmp_i18n_grow(i18n, i18n->entry_count + 1u);
    CMP_I18N_RETURN_IF_ERROR(rc);
  }

  rc = cmp_i18n_alloc_string(&i18n->allocator, utf8_key, key_len, &key_copy);
  if (rc != CMP_OK) {
    return rc;
  }

  value_copy = NULL;
  if (value_len > 0) {
    rc = cmp_i18n_alloc_string(&i18n->allocator, utf8_value, value_len,
                               &value_copy);
    if (rc != CMP_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, key_copy);
      CMP_UNUSED(free_rc);
      return rc;
    }
  }

  entry = &i18n->entries[i18n->entry_count];
  entry->key = key_copy;
  entry->key_len = key_len;
  entry->value = value_copy;
  entry->value_len = value_len;
  i18n->entry_count += 1u;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_get(const CMPI18n *i18n, const char *utf8_key,
                          cmp_usize key_len, const char **out_value,
                          cmp_usize *out_value_len) {
  cmp_usize index;
  CMPBool found;
  int rc;

  if (i18n == NULL || out_value == NULL || out_value_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_i18n_validate_key(utf8_key, key_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (found == CMP_FALSE) {
    *out_value = NULL;
    *out_value_len = 0;
    return CMP_ERR_NOT_FOUND;
  }

  *out_value = i18n->entries[index].value;
  *out_value_len = i18n->entries[index].value_len;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_contains(const CMPI18n *i18n, const char *utf8_key,
                               cmp_usize key_len,
                               CMPBool *out_exists) { /* GCOVR_EXCL_LINE */
  cmp_usize index;                                    /* GCOVR_EXCL_LINE */
  CMPBool found;                                      /* GCOVR_EXCL_LINE */
  int rc;

  if (i18n == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_i18n_validate_key(utf8_key, key_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  CMP_I18N_RETURN_IF_ERROR(rc);

  *out_exists = found;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_remove(CMPI18n *i18n, const char *utf8_key,
                             cmp_usize key_len) {
  cmp_usize index;
  cmp_usize tail_count;
  CMPBool found;
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_i18n_validate_key(utf8_key, key_len);
  CMP_I18N_RETURN_IF_ERROR(rc);

  rc = cmp_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  CMP_I18N_RETURN_IF_ERROR(rc);

  if (found == CMP_FALSE) {
    return CMP_ERR_NOT_FOUND;
  }

  rc = cmp_i18n_entry_release(i18n, &i18n->entries[index]);
  if (rc != CMP_OK) {
    return rc;
  }

  tail_count = i18n->entry_count - index - 1u;
  if (tail_count > 0u) {
    memmove(&i18n->entries[index], &i18n->entries[index + 1u],
            (size_t)(tail_count * sizeof(CMPI18nEntry)));
  }

  i18n->entry_count -= 1u;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_clear(CMPI18n *i18n) {
  cmp_usize i;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  rc = CMP_OK;
  for (i = 0; i < i18n->entry_count; ++i) {
    free_rc = cmp_i18n_entry_release(i18n, &i18n->entries[i]);
    if (free_rc != CMP_OK && rc == CMP_OK) {
      rc = free_rc;
    }
  }

  i18n->entry_count = 0;
  return rc;
}

int CMP_CALL cmp_i18n_count(const CMPI18n *i18n, cmp_usize *out_count) {
  if (i18n == NULL || out_count == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  *out_count = i18n->entry_count;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_load_table(CMPI18n *i18n, CMPIO *io,
                                 const char *utf8_path, CMPBool clear_existing,
                                 CMPBool overwrite) {
  void *buffer;
  cmp_usize size;
  int rc;
  int free_rc;

  if (i18n == NULL || io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }
  if (io->vtable == NULL || io->vtable->read_file_alloc == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  buffer = NULL;
  size = 0;
  rc = io->vtable->read_file_alloc(io->ctx, utf8_path, &i18n->allocator,
                                   &buffer, &size);
  if (rc != CMP_OK) {
    return rc;
  }

  rc = cmp_i18n_load_table_buffer(i18n, (const char *)buffer, size,
                                  clear_existing, overwrite);
  free_rc = i18n->allocator.free(i18n->allocator.ctx, buffer);
  if (free_rc != CMP_OK && rc == CMP_OK) {
    rc = free_rc;
  }
  return rc;
}

int CMP_CALL cmp_i18n_load_table_buffer(
    CMPI18n *i18n, const char *data, cmp_usize size, CMPBool clear_existing,
    CMPBool overwrite) { /* GCOVR_EXCL_LINE */
  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  return cmp_i18n_parse_table(i18n, data, size, clear_existing, overwrite);
}

int CMP_CALL cmp_i18n_format_number(const CMPI18n *i18n,
                                    const CMPI18nNumber *number, char *out_text,
                                    cmp_usize text_capacity,
                                    cmp_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_number != NULL) {
    rc = i18n->formatter.vtable->format_number(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, number, out_text,
        text_capacity, out_len);
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return cmp_i18n_format_number_internal(&i18n->locale, number, out_text,
                                         text_capacity, out_len);
}

int CMP_CALL cmp_i18n_format_date(const CMPI18n *i18n, const CMPDate *date,
                                  char *out_text, cmp_usize text_capacity,
                                  cmp_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_date != NULL) {
    rc = i18n->formatter.vtable->format_date(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, date, out_text,
        text_capacity, out_len);
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return cmp_i18n_format_date_internal(&i18n->locale, date, out_text,
                                       text_capacity, out_len);
}

int CMP_CALL cmp_i18n_format_time(const CMPI18n *i18n, const CMPTime *time,
                                  char *out_text, cmp_usize text_capacity,
                                  cmp_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return CMP_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_time != NULL) {
    rc = i18n->formatter.vtable->format_time(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, time, out_text,
        text_capacity, out_len);
    if (rc != CMP_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return cmp_i18n_format_time_internal(&i18n->locale, time, out_text,
                                       text_capacity, out_len);
}

#ifdef CMP_TESTING
int CMP_CALL cmp_i18n_test_mul_overflow(cmp_usize a, cmp_usize b,
                                        cmp_usize *out_value) {
  return cmp_i18n_mul_overflow(a, b, out_value);
}

int CMP_CALL cmp_i18n_test_add_overflow(cmp_usize a, cmp_usize b,
                                        cmp_usize *out_value) {
  return cmp_i18n_add_overflow(a, b, out_value);
}

int CMP_CALL cmp_i18n_test_pow10(cmp_u32 digits, cmp_u32 *out_value) {
  return cmp_i18n_pow10(digits, out_value);
}

int CMP_CALL cmp_i18n_test_is_leap_year(cmp_i32 year, CMPBool *out_leap) {
  return cmp_i18n_is_leap_year(year, out_leap);
}

int CMP_CALL cmp_i18n_test_days_in_month(cmp_i32 year, cmp_u32 month,
                                         cmp_u32 *out_days) {
  return cmp_i18n_days_in_month(year, month, out_days);
}

int CMP_CALL cmp_i18n_test_validate_locale(const CMPI18nLocale *locale) {
  return cmp_i18n_validate_locale(locale);
}

int CMP_CALL cmp_i18n_test_validate_number(const CMPI18nNumber *number) {
  return cmp_i18n_validate_number(number);
}

int CMP_CALL cmp_i18n_test_validate_date(const CMPDate *date) {
  return cmp_i18n_validate_date(date);
}

int CMP_CALL cmp_i18n_test_validate_time(const CMPTime *time) {
  return cmp_i18n_validate_time(time);
}

int CMP_CALL cmp_i18n_test_validate_utf8(const char *data, cmp_usize length) {
  return cmp_i18n_validate_utf8(data, length);
}

int CMP_CALL cmp_i18n_test_cstrlen(const char *cstr, cmp_usize *out_len) {
  return cmp_i18n_cstrlen(cstr, out_len);
}

int CMP_CALL cmp_i18n_test_ascii_lower(char ch, char *out_lower) {
  return cmp_i18n_ascii_lower(ch, out_lower);
}

int CMP_CALL cmp_i18n_test_tag_equals(const char *a, const char *b,
                                      CMPBool *out_equal) {
  return cmp_i18n_tag_equals(a, b, out_equal);
}

int CMP_CALL cmp_i18n_test_trim_span(const char *data, cmp_usize length,
                                     cmp_usize *out_start,
                                     cmp_usize *out_length) {
  return cmp_i18n_trim_span(data, length, out_start, out_length);
}

int CMP_CALL cmp_i18n_test_alloc_string(const CMPAllocator *allocator,
                                        const char *src, cmp_usize len,
                                        char **out_str) {
  return cmp_i18n_alloc_string(allocator, src, len, out_str);
}

int CMP_CALL cmp_i18n_test_write_uint(char *out_text, cmp_usize text_capacity,
                                      cmp_usize *io_offset, cmp_u32 value,
                                      cmp_u32 min_digits) {
  return cmp_i18n_write_uint(out_text, text_capacity, io_offset, value,
                             min_digits);
}

int CMP_CALL cmp_i18n_test_find_entry(const CMPI18n *i18n, const char *key,
                                      cmp_usize key_len, cmp_usize *out_index,
                                      CMPBool *out_found) {
  return cmp_i18n_find_entry(i18n, key, key_len, out_index, out_found);
}

int CMP_CALL cmp_i18n_test_grow(CMPI18n *i18n, cmp_usize min_capacity) {
  return cmp_i18n_grow(i18n, min_capacity);
}

int CMP_CALL cmp_i18n_test_set_cstr_limit(cmp_usize max_len) {
  g_cmp_i18n_cstr_limit = max_len;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_pow10_error(CMPBool enable) {
  g_cmp_i18n_force_pow10_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_leap_error(CMPBool enable) {
  g_cmp_i18n_force_leap_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_days_default(CMPBool enable) {
  g_cmp_i18n_force_days_default = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_set_ascii_lower_fail_after(cmp_u32 call_count) {
  g_cmp_i18n_ascii_lower_fail_after = call_count;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_utf8_error(CMPBool enable) {
  g_cmp_i18n_force_utf8_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_utf8_ok(CMPBool enable) {
  g_cmp_i18n_force_utf8_ok = (enable == CMP_TRUE) ? 1u : 0u;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_utf8_ok_count(cmp_u32 count) {
  g_cmp_i18n_force_utf8_ok = count;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_force_config_init_error(CMPBool enable) {
  g_cmp_i18n_force_config_init_error = (enable == CMP_TRUE) ? 1 : 0;
  return CMP_OK;
}

int CMP_CALL cmp_i18n_test_locale_preset_en_us(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_en_us(out_locale);
}

int CMP_CALL cmp_i18n_test_locale_preset_en_gb(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_en_gb(out_locale);
}

int CMP_CALL cmp_i18n_test_locale_preset_fr_fr(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_fr_fr(out_locale);
}

int CMP_CALL cmp_i18n_test_locale_preset_ar_sa(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_ar_sa(out_locale);
}

int CMP_CALL cmp_i18n_test_locale_preset_he_il(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_he_il(out_locale);
}

int CMP_CALL cmp_i18n_test_locale_preset_de_de(CMPI18nLocale *out_locale) {
  return cmp_i18n_locale_preset_de_de(out_locale);
}

int CMP_CALL cmp_i18n_test_parse_table(CMPI18n *i18n,
                                       const char *data, /* GCOVR_EXCL_LINE */
                                       cmp_usize size, CMPBool clear_existing,
                                       CMPBool overwrite) {
  return cmp_i18n_parse_table(i18n, data, size, clear_existing,
                              overwrite); /* GCOVR_EXCL_LINE */
}
#endif
