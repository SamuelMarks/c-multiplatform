#include "m3/m3_i18n.h"

#include "m3/m3_date_picker.h"
#include "m3/m3_time_picker.h"
#include "m3/m3_utf8.h"

#include <string.h>

#define M3_I18N_USIZE_MAX ((m3_usize) ~(m3_usize)0)

#define M3_I18N_RETURN_IF_ERROR(rc)                                            \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#ifdef M3_TESTING
static m3_usize g_m3_i18n_cstr_limit = 0;
#endif

static int m3_i18n_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = M3_I18N_USIZE_MAX;
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_i18n_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = M3_I18N_USIZE_MAX;
  if (b > max_value - a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a + b;
  return M3_OK;
}

static int m3_i18n_validate_bool(M3Bool value) {
  if (value != M3_FALSE && value != M3_TRUE) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_i18n_validate_locale(const M3I18nLocale *locale) {
  int rc;

  if (locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (locale->decimal_separator == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (locale->date_separator == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (locale->time_separator == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (locale->grouping != 0 && locale->thousands_separator == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (locale->grouping > 9u) {
    return M3_ERR_RANGE;
  }

  if (locale->date_order != M3_I18N_DATE_ORDER_MDY &&
      locale->date_order != M3_I18N_DATE_ORDER_DMY &&
      locale->date_order != M3_I18N_DATE_ORDER_YMD) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (locale->time_format != M3_I18N_TIME_FORMAT_24H &&
      locale->time_format != M3_I18N_TIME_FORMAT_12H) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_bool(locale->pad_day);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_bool(locale->pad_month);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_bool(locale->pad_hour);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_bool(locale->pad_minute);
  M3_I18N_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_i18n_pow10(m3_u32 digits, m3_u32 *out_value) {
  m3_u32 value;
  m3_u32 i;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  value = 1u;
  for (i = 0; i < digits; ++i) {
    if (value > 0xFFFFFFFFu / 10u) {
      return M3_ERR_OVERFLOW;
    }
    value *= 10u;
  }

  *out_value = value;
  return M3_OK;
}

static int m3_i18n_validate_number(const M3I18nNumber *number) {
  m3_u32 limit;
  int rc;

  if (number == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (number->fraction_digits > M3_I18N_MAX_FRACTION_DIGITS) {
    return M3_ERR_RANGE;
  }

  if (number->fraction_digits == 0 && number->fraction != 0) {
    return M3_ERR_RANGE;
  }

  rc = m3_i18n_pow10(number->fraction_digits, &limit);
  if (rc != M3_OK) {
    return rc;
  }

  if (number->fraction_digits > 0 && number->fraction >= limit) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_i18n_is_leap_year(m3_i32 year, M3Bool *out_leap) {
  if (out_leap == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if ((year % 400) == 0) {
    *out_leap = M3_TRUE;
  } else if ((year % 100) == 0) {
    *out_leap = M3_FALSE;
  } else if ((year % 4) == 0) {
    *out_leap = M3_TRUE;
  } else {
    *out_leap = M3_FALSE;
  }
  return M3_OK;
}

static int m3_i18n_days_in_month(m3_i32 year, m3_u32 month, m3_u32 *out_days) {
  M3Bool leap;
  int rc;

  if (out_days == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (month < 1u || month > 12u) {
    return M3_ERR_RANGE;
  }

  switch (month) {
  case 1u:
  case 3u:
  case 5u:
  case 7u:
  case 8u:
  case 10u:
  case 12u:
    *out_days = 31u;
    return M3_OK;
  case 4u:
  case 6u:
  case 9u:
  case 11u:
    *out_days = 30u;
    return M3_OK;
  case 2u:
    rc = m3_i18n_is_leap_year(year, &leap);
    if (rc != M3_OK) {
      return rc;
    }
    *out_days = (leap == M3_TRUE) ? 29u : 28u;
    return M3_OK;
  default:
    break;
  }

  return M3_ERR_RANGE;
}

static int m3_i18n_validate_date(const M3Date *date) {
  m3_u32 days_in_month;
  int rc;

  if (date == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (date->year < M3_DATE_MIN_YEAR || date->year > M3_DATE_MAX_YEAR) {
    return M3_ERR_RANGE;
  }
  if (date->month < 1u || date->month > 12u) {
    return M3_ERR_RANGE;
  }

  rc = m3_i18n_days_in_month(date->year, date->month, &days_in_month);
  if (rc != M3_OK) {
    return rc;
  }
  if (date->day < 1u || date->day > days_in_month) {
    return M3_ERR_RANGE;
  }

  return M3_OK;
}

static int m3_i18n_validate_time(const M3Time *time) {
  if (time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (time->hour > 23u) {
    return M3_ERR_RANGE;
  }
  if (time->minute > 59u) {
    return M3_ERR_RANGE;
  }
  return M3_OK;
}

static int m3_i18n_cstrlen(const char *cstr, m3_usize *out_len) {
  m3_usize max_len;
  m3_usize len;

  if (cstr == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_len = M3_I18N_USIZE_MAX;
#ifdef M3_TESTING
  if (g_m3_i18n_cstr_limit != 0) {
    max_len = g_m3_i18n_cstr_limit;
  }
#endif

  len = 0;
  while (cstr[len] != '\0') {
    if (len == max_len) {
      return M3_ERR_OVERFLOW;
    }
    len += 1;
  }

  *out_len = len;
  return M3_OK;
}

static int m3_i18n_ascii_lower(char ch, char *out_lower) {
  if (out_lower == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (ch >= 'A' && ch <= 'Z') {
    *out_lower = (char)(ch - 'A' + 'a');
  } else {
    *out_lower = ch;
  }
  return M3_OK;
}

static int m3_i18n_tag_equals(const char *a, const char *b, M3Bool *out_equal) {
  m3_usize index;
  char ca;
  char cb;
  int rc;

  if (a == NULL || b == NULL || out_equal == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  index = 0;
  while (a[index] != '\0' || b[index] != '\0') {
    if (a[index] == '\0' || b[index] == '\0') {
      *out_equal = M3_FALSE;
      return M3_OK;
    }

    ca = a[index];
    cb = b[index];
    if (ca == '_') {
      ca = '-';
    }
    if (cb == '_') {
      cb = '-';
    }

    rc = m3_i18n_ascii_lower(ca, &ca);
    if (rc != M3_OK) {
      return rc;
    }
    rc = m3_i18n_ascii_lower(cb, &cb);
    if (rc != M3_OK) {
      return rc;
    }

    if (ca != cb) {
      *out_equal = M3_FALSE;
      return M3_OK;
    }

    index += 1;
  }

  *out_equal = M3_TRUE;
  return M3_OK;
}

static int m3_i18n_locale_preset_en_us(M3I18nLocale *out_locale) {
  if (out_locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = '.';
  out_locale->thousands_separator = ',';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = M3_I18N_DATE_ORDER_MDY;
  out_locale->time_separator = ':';
  out_locale->time_format = M3_I18N_TIME_FORMAT_12H;
  out_locale->pad_day = M3_TRUE;
  out_locale->pad_month = M3_TRUE;
  out_locale->pad_hour = M3_FALSE;
  out_locale->pad_minute = M3_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return M3_OK;
}

static int m3_i18n_locale_preset_en_gb(M3I18nLocale *out_locale) {
  if (out_locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = '.';
  out_locale->thousands_separator = ',';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = M3_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = M3_I18N_TIME_FORMAT_24H;
  out_locale->pad_day = M3_TRUE;
  out_locale->pad_month = M3_TRUE;
  out_locale->pad_hour = M3_TRUE;
  out_locale->pad_minute = M3_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return M3_OK;
}

static int m3_i18n_locale_preset_fr_fr(M3I18nLocale *out_locale) {
  if (out_locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = ',';
  out_locale->thousands_separator = ' ';
  out_locale->grouping = 3u;
  out_locale->date_separator = '/';
  out_locale->date_order = M3_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = M3_I18N_TIME_FORMAT_24H;
  out_locale->pad_day = M3_TRUE;
  out_locale->pad_month = M3_TRUE;
  out_locale->pad_hour = M3_TRUE;
  out_locale->pad_minute = M3_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return M3_OK;
}

static int m3_i18n_locale_preset_de_de(M3I18nLocale *out_locale) {
  if (out_locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  out_locale->decimal_separator = ',';
  out_locale->thousands_separator = '.';
  out_locale->grouping = 3u;
  out_locale->date_separator = '.';
  out_locale->date_order = M3_I18N_DATE_ORDER_DMY;
  out_locale->time_separator = ':';
  out_locale->time_format = M3_I18N_TIME_FORMAT_24H;
  out_locale->pad_day = M3_TRUE;
  out_locale->pad_month = M3_TRUE;
  out_locale->pad_hour = M3_TRUE;
  out_locale->pad_minute = M3_TRUE;
  out_locale->am = "AM";
  out_locale->pm = "PM";
  return M3_OK;
}

static int m3_i18n_validate_key(const char *key, m3_usize key_len) {
  if (key == NULL || key_len == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_OK;
}

static int m3_i18n_validate_utf8(const char *data, m3_usize length) {
  M3Bool valid;
  int rc;

  if (data == NULL && length != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (length == 0) {
    return M3_OK;
  }

  rc = m3_utf8_validate(data, length, &valid);
  if (rc != M3_OK) {
    return rc;
  }
  if (valid != M3_TRUE) {
    return M3_ERR_CORRUPT;
  }

  return M3_OK;
}

static int m3_i18n_find_entry(const M3I18n *i18n, const char *key,
                              m3_usize key_len, m3_usize *out_index,
                              M3Bool *out_found) {
  m3_usize i;

  if (out_index == NULL || out_found == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_index = 0;
  *out_found = M3_FALSE;

  for (i = 0; i < i18n->entry_count; ++i) {
    const M3I18nEntry *entry = &i18n->entries[i];

    if (entry->key_len == key_len && entry->key != NULL &&
        memcmp(entry->key, key, (size_t)key_len) == 0) {
      *out_index = i;
      *out_found = M3_TRUE;
      return M3_OK;
    }
  }

  return M3_OK;
}

static int m3_i18n_entry_release(M3I18n *i18n, M3I18nEntry *entry) {
  int rc;
  int free_rc;

  rc = M3_OK;

  if (entry->key != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, entry->key);
    if (free_rc != M3_OK && rc == M3_OK) {
      rc = free_rc;
    }
    entry->key = NULL;
  }

  if (entry->value != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
    if (free_rc != M3_OK && rc == M3_OK) {
      rc = free_rc;
    }
    entry->value = NULL;
  }

  entry->key_len = 0;
  entry->value_len = 0;
  return rc;
}

static int m3_i18n_grow(M3I18n *i18n, m3_usize min_capacity) {
  m3_usize new_capacity;
  m3_usize alloc_size;
  m3_usize clear_count;
  void *new_entries;
  int rc;

  if (i18n->allocator.realloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  new_capacity = i18n->entry_capacity;
  while (new_capacity < min_capacity) {
    rc = m3_i18n_mul_overflow(new_capacity, (m3_usize)2, &new_capacity);
    if (rc != M3_OK) {
      return rc;
    }
  }

  rc = m3_i18n_mul_overflow(new_capacity, (m3_usize)sizeof(M3I18nEntry),
                            &alloc_size);
  if (rc != M3_OK) {
    return rc;
  }

  rc = i18n->allocator.realloc(i18n->allocator.ctx, i18n->entries, alloc_size,
                               &new_entries);
  if (rc != M3_OK) {
    return rc;
  }

  clear_count = new_capacity - i18n->entry_capacity;
  if (clear_count > 0) {
    memset((M3I18nEntry *)new_entries + i18n->entry_capacity, 0,
           (size_t)(clear_count * sizeof(M3I18nEntry)));
  }

  i18n->entries = (M3I18nEntry *)new_entries;
  i18n->entry_capacity = new_capacity;
  return M3_OK;
}

static int m3_i18n_alloc_string(const M3Allocator *allocator, const char *src,
                                m3_usize len, char **out_str) {
  m3_usize bytes;
  void *mem;
  int rc;

  if (allocator == NULL || out_str == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (src == NULL && len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_add_overflow(len, (m3_usize)1, &bytes);
  if (rc != M3_OK) {
    return rc;
  }

  rc = allocator->alloc(allocator->ctx, bytes, &mem);
  if (rc != M3_OK) {
    return rc;
  }

  if (len > 0) {
    memcpy(mem, src, (size_t)len);
  }
  ((char *)mem)[len] = '\0';

  *out_str = (char *)mem;
  return M3_OK;
}

static int m3_i18n_write_uint(char *out_text, m3_usize text_capacity,
                              m3_usize *io_offset, m3_u32 value,
                              m3_u32 min_digits) {
  char digits[16];
  m3_usize digit_count;
  m3_usize i;
  m3_usize required;

  if (out_text == NULL || io_offset == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  digit_count = 0;
  do {
    digits[digit_count] = (char)('0' + (value % 10u));
    value /= 10u;
    digit_count += 1;
  } while (value > 0u);

  while (digit_count < (m3_usize)min_digits) {
    digits[digit_count] = '0';
    digit_count += 1;
  }

  required = *io_offset + digit_count;
  if (required >= text_capacity) {
    return M3_ERR_RANGE;
  }

  for (i = 0; i < digit_count; ++i) {
    out_text[*io_offset + i] = digits[digit_count - 1 - i];
  }

  *io_offset += digit_count;
  return M3_OK;
}

static int m3_i18n_format_number_internal(const M3I18nLocale *locale,
                                          const M3I18nNumber *number,
                                          char *out_text,
                                          m3_usize text_capacity,
                                          m3_usize *out_len) {
  char digits[16];
  m3_u32 magnitude;
  m3_usize digit_count;
  m3_usize sep_count;
  m3_usize total_len;
  m3_usize i;
  m3_usize offset;
  m3_usize group_size;
  int rc;

  if (locale == NULL || number == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_locale(locale);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_number(number);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (number->integer < 0) {
    magnitude = (m3_u32)(-(number->integer + 1)) + 1u;
  } else {
    magnitude = (m3_u32)number->integer;
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

  group_size = (m3_usize)locale->grouping;
  sep_count = 0;
  if (group_size > 0u && locale->thousands_separator != '\0') {
    sep_count = (digit_count - 1u) / group_size;
  }

  total_len = digit_count + sep_count;
  if (number->integer < 0) {
    total_len += 1u;
  }
  if (number->fraction_digits > 0u) {
    total_len += 1u + (m3_usize)number->fraction_digits;
  }

  *out_len = total_len;
  if (text_capacity <= total_len) {
    return M3_ERR_RANGE;
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
    m3_u32 fraction;
    m3_u32 scale;
    m3_u32 digit;
    m3_u32 idx;

    out_text[offset] = locale->decimal_separator;
    offset += 1u;

    fraction = number->fraction;
    rc = m3_i18n_pow10(number->fraction_digits, &scale);
    M3_I18N_RETURN_IF_ERROR(rc);

    for (idx = number->fraction_digits; idx > 0u; --idx) {
      scale /= 10u;
      digit = (fraction / scale) % 10u;
      out_text[offset] = (char)('0' + digit);
      offset += 1u;
    }
  }

  out_text[offset] = '\0';
  return M3_OK;
}

static int m3_i18n_format_date_internal(const M3I18nLocale *locale,
                                        const M3Date *date, char *out_text,
                                        m3_usize text_capacity,
                                        m3_usize *out_len) {
  m3_u32 year_value;
  m3_u32 year_digits;
  m3_u32 year_min_digits;
  m3_u32 day_digits;
  m3_u32 month_digits;
  m3_usize total_len;
  m3_usize offset;
  int rc;

  if (locale == NULL || date == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_locale(locale);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_date(date);
  M3_I18N_RETURN_IF_ERROR(rc);

  year_value = (m3_u32)date->year;
  year_digits = 1u;
  while (year_value >= 10u) {
    year_value /= 10u;
    year_digits += 1u;
  }
  year_min_digits = 4u;
  if (year_digits > year_min_digits) {
    year_min_digits = year_digits;
  }

  day_digits = (locale->pad_day == M3_TRUE) ? 2u : 1u;
  month_digits = (locale->pad_month == M3_TRUE) ? 2u : 1u;

  total_len = (m3_usize)year_min_digits + (m3_usize)day_digits +
              (m3_usize)month_digits + 2u;
  *out_len = total_len;
  if (text_capacity <= total_len) {
    return M3_ERR_RANGE;
  }

  offset = 0;

  if (locale->date_order == M3_I18N_DATE_ORDER_MDY) {
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                            month_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                            day_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset,
                            (m3_u32)date->year, year_min_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
  } else if (locale->date_order == M3_I18N_DATE_ORDER_DMY) {
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                            day_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                            month_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset,
                            (m3_u32)date->year, year_min_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
  } else {
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset,
                            (m3_u32)date->year, year_min_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->month,
                            month_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
    out_text[offset] = locale->date_separator;
    offset += 1u;
    rc = m3_i18n_write_uint(out_text, text_capacity, &offset, date->day,
                            day_digits);
    M3_I18N_RETURN_IF_ERROR(rc);
  }

  out_text[offset] = '\0';
  return M3_OK;
}

static int m3_i18n_format_time_internal(const M3I18nLocale *locale,
                                        const M3Time *time, char *out_text,
                                        m3_usize text_capacity,
                                        m3_usize *out_len) {
  m3_u32 hour_value;
  m3_u32 hour_digits;
  m3_u32 minute_digits;
  m3_usize total_len;
  m3_usize offset;
  const char *designator;
  m3_usize designator_len;
  int rc;

  if (locale == NULL || time == NULL || out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_locale(locale);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_time(time);
  M3_I18N_RETURN_IF_ERROR(rc);

  designator = NULL;
  designator_len = 0;
  hour_value = time->hour;

  if (locale->time_format == M3_I18N_TIME_FORMAT_12H) {
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

  if (locale->pad_hour == M3_TRUE) {
    hour_digits = 2u;
  } else {
    hour_digits = (hour_value >= 10u) ? 2u : 1u;
  }
  minute_digits = (locale->pad_minute == M3_TRUE) ? 2u : 1u;

  total_len = (m3_usize)hour_digits + (m3_usize)minute_digits + 1u;

  if (designator != NULL) {
    rc = m3_i18n_cstrlen(designator, &designator_len);
    M3_I18N_RETURN_IF_ERROR(rc);
    if (designator_len > 0) {
      total_len += 1u + designator_len;
    }
  }

  *out_len = total_len;
  if (text_capacity <= total_len) {
    return M3_ERR_RANGE;
  }

  offset = 0;
  rc = m3_i18n_write_uint(out_text, text_capacity, &offset, hour_value,
                          hour_digits);
  M3_I18N_RETURN_IF_ERROR(rc);
  out_text[offset] = locale->time_separator;
  offset += 1u;
  rc = m3_i18n_write_uint(out_text, text_capacity, &offset, time->minute,
                          minute_digits);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (designator != NULL && designator_len > 0) {
    out_text[offset] = ' ';
    offset += 1u;
    memcpy(out_text + offset, designator, (size_t)designator_len);
    offset += designator_len;
  }

  out_text[offset] = '\0';
  return M3_OK;
}

static int m3_i18n_set_locale_internal(M3I18n *i18n, const char *locale_tag,
                                       const M3I18nLocale *locale) {
  M3I18nLocale next_locale;
  char *next_tag;
  m3_usize tag_len;
  int rc;
  int free_rc;

  if (i18n == NULL || locale_tag == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_cstrlen(locale_tag, &tag_len);
  if (rc != M3_OK) {
    return rc;
  }
  if (tag_len == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_utf8(locale_tag, tag_len);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_i18n_alloc_string(&i18n->allocator, locale_tag, tag_len, &next_tag);
  if (rc != M3_OK) {
    return rc;
  }

  if (locale != NULL) {
    rc = m3_i18n_validate_locale(locale);
    if (rc != M3_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      M3_UNUSED(free_rc);
      return rc;
    }
    next_locale = *locale;
  } else {
    rc = m3_i18n_locale_from_tag(locale_tag, &next_locale);
    if (rc != M3_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      M3_UNUSED(free_rc);
      return rc;
    }
  }

  if (i18n->locale_tag != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->locale_tag);
    if (free_rc != M3_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, next_tag);
      M3_UNUSED(free_rc);
      return free_rc;
    }
  }

  i18n->locale_tag = next_tag;
  i18n->locale_tag_len = tag_len;
  i18n->locale = next_locale;
  return M3_OK;
}

static int m3_i18n_trim_span(const char *data, m3_usize length,
                             m3_usize *out_start, m3_usize *out_length) {
  m3_usize start;
  m3_usize end;

  if (data == NULL || out_start == NULL || out_length == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
  return M3_OK;
}

static int m3_i18n_parse_table(M3I18n *i18n, const char *data, m3_usize size,
                               M3Bool clear_existing, M3Bool overwrite) {
  m3_usize offset;
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_validate_bool(clear_existing);
  M3_I18N_RETURN_IF_ERROR(rc);
  rc = m3_i18n_validate_bool(overwrite);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (data == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (clear_existing == M3_TRUE) {
    rc = m3_i18n_clear(i18n);
    M3_I18N_RETURN_IF_ERROR(rc);
  }

  if (size == 0) {
    return M3_OK;
  }

  offset = 0;
  if (size >= 3u && (m3_u8)data[0] == 0xEFu && (m3_u8)data[1] == 0xBBu &&
      (m3_u8)data[2] == 0xBFu) {
    offset = 3u;
  }

  while (offset < size) {
    const char *line;
    m3_usize line_start;
    m3_usize line_len;
    m3_usize trim_start;
    m3_usize trim_len;
    m3_usize eq_index;
    m3_usize key_start;
    m3_usize key_len;
    m3_usize value_start;
    m3_usize value_len;

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

    rc = m3_i18n_trim_span(line, line_len, &trim_start, &trim_len);
    M3_I18N_RETURN_IF_ERROR(rc);
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
      return M3_ERR_CORRUPT;
    }

    rc = m3_i18n_trim_span(line + trim_start, eq_index - trim_start, &key_start,
                           &key_len);
    M3_I18N_RETURN_IF_ERROR(rc);
    if (key_len == 0) {
      return M3_ERR_CORRUPT;
    }
    key_start += trim_start;

    rc = m3_i18n_trim_span(line + eq_index + 1u,
                           trim_start + trim_len - (eq_index + 1u),
                           &value_start, &value_len);
    M3_I18N_RETURN_IF_ERROR(rc);
    value_start += eq_index + 1u;

    rc = m3_i18n_put(i18n, line + key_start, key_len, line + value_start,
                     value_len, overwrite);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_i18n_locale_init(M3I18nLocale *locale) {
  if (locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return m3_i18n_locale_preset_en_us(locale);
}

int M3_CALL m3_i18n_locale_from_tag(const char *locale_tag,
                                    M3I18nLocale *out_locale) {
  M3Bool match;
  int rc;

  if (locale_tag == NULL || out_locale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_tag_equals(locale_tag, "en-US", &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    return m3_i18n_locale_preset_en_us(out_locale);
  }

  rc = m3_i18n_tag_equals(locale_tag, "en-GB", &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    return m3_i18n_locale_preset_en_gb(out_locale);
  }

  rc = m3_i18n_tag_equals(locale_tag, "fr-FR", &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    return m3_i18n_locale_preset_fr_fr(out_locale);
  }

  rc = m3_i18n_tag_equals(locale_tag, "de-DE", &match);
  if (rc != M3_OK) {
    return rc;
  }
  if (match == M3_TRUE) {
    return m3_i18n_locale_preset_de_de(out_locale);
  }

  return M3_ERR_NOT_FOUND;
}

int M3_CALL m3_i18n_config_init(M3I18nConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->entry_capacity = (m3_usize)M3_I18N_DEFAULT_CAPACITY;
  config->locale_tag = M3_I18N_DEFAULT_LOCALE_TAG;
  config->locale = NULL;
  config->formatter = NULL;
  return M3_OK;
}

int M3_CALL m3_i18n_init(M3I18n *i18n, const M3I18nConfig *config) {
  M3I18nConfig default_config;
  M3Allocator allocator;
  M3I18nEntry *entries;
  m3_usize alloc_size;
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (i18n->entries != NULL) {
    return M3_ERR_STATE;
  }

  if (config == NULL) {
    rc = m3_i18n_config_init(&default_config);
    if (rc != M3_OK) {
      return rc;
    }
    config = &default_config;
  }

  if (config->entry_capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    if (rc != M3_OK) {
      return rc;
    }
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_mul_overflow(config->entry_capacity,
                            (m3_usize)sizeof(M3I18nEntry), &alloc_size);
  if (rc != M3_OK) {
    return rc;
  }

  rc = allocator.alloc(allocator.ctx, alloc_size, (void **)&entries);
  if (rc != M3_OK) {
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
      rc = m3_i18n_shutdown(i18n);
      M3_UNUSED(rc);
      return M3_ERR_INVALID_ARGUMENT;
    }
    i18n->formatter = *config->formatter;
  }

  if (config->locale != NULL || config->locale_tag != NULL) {
    const char *tag = config->locale_tag;
    if (tag == NULL) {
      tag = M3_I18N_DEFAULT_LOCALE_TAG;
    }
    rc = m3_i18n_set_locale_internal(i18n, tag, config->locale);
    if (rc != M3_OK) {
      int shutdown_rc = m3_i18n_shutdown(i18n);
      M3_UNUSED(shutdown_rc);
      return rc;
    }
  } else {
    rc = m3_i18n_set_locale_internal(i18n, M3_I18N_DEFAULT_LOCALE_TAG, NULL);
    if (rc != M3_OK) {
      int shutdown_rc = m3_i18n_shutdown(i18n);
      M3_UNUSED(shutdown_rc);
      return rc;
    }
  }

  return M3_OK;
}

int M3_CALL m3_i18n_shutdown(M3I18n *i18n) {
  m3_usize i;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }
  if (i18n->allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = M3_OK;

  for (i = 0; i < i18n->entry_count; ++i) {
    free_rc = m3_i18n_entry_release(i18n, &i18n->entries[i]);
    if (free_rc != M3_OK && rc == M3_OK) {
      rc = free_rc;
    }
  }

  free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->entries);
  if (free_rc != M3_OK && rc == M3_OK) {
    rc = free_rc;
  }

  i18n->entries = NULL;
  i18n->entry_capacity = 0;
  i18n->entry_count = 0;

  if (i18n->locale_tag != NULL) {
    free_rc = i18n->allocator.free(i18n->allocator.ctx, i18n->locale_tag);
    if (free_rc != M3_OK && rc == M3_OK) {
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

int M3_CALL m3_i18n_set_locale(M3I18n *i18n, const char *locale_tag,
                               const M3I18nLocale *locale) {
  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }
  return m3_i18n_set_locale_internal(i18n, locale_tag, locale);
}

int M3_CALL m3_i18n_set_formatter(M3I18n *i18n,
                                  const M3I18nFormatter *formatter) {
  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  if (formatter == NULL) {
    memset(&i18n->formatter, 0, sizeof(i18n->formatter));
    return M3_OK;
  }
  if (formatter->vtable == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  i18n->formatter = *formatter;
  return M3_OK;
}

int M3_CALL m3_i18n_put(M3I18n *i18n, const char *utf8_key, m3_usize key_len,
                        const char *utf8_value, m3_usize value_len,
                        M3Bool overwrite) {
  M3I18nEntry *entry;
  m3_usize index;
  M3Bool found;
  char *key_copy;
  char *value_copy;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_i18n_validate_bool(overwrite);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_validate_key(utf8_key, key_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_validate_utf8(utf8_key, key_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_validate_utf8(utf8_value, value_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (utf8_value == NULL && value_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (found == M3_TRUE) {
    if (overwrite == M3_FALSE) {
      return M3_ERR_BUSY;
    }

    entry = &i18n->entries[index];
    if (value_len == 0) {
      if (entry->value != NULL) {
        rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
        if (rc != M3_OK) {
          return rc;
        }
      }
      entry->value = NULL;
      entry->value_len = 0;
      return M3_OK;
    }

    rc = m3_i18n_alloc_string(&i18n->allocator, utf8_value, value_len,
                              &value_copy);
    if (rc != M3_OK) {
      return rc;
    }

    if (entry->value != NULL) {
      rc = i18n->allocator.free(i18n->allocator.ctx, entry->value);
      if (rc != M3_OK) {
        free_rc = i18n->allocator.free(i18n->allocator.ctx, value_copy);
        M3_UNUSED(free_rc);
        return rc;
      }
    }

    entry->value = value_copy;
    entry->value_len = value_len;
    return M3_OK;
  }

  if (i18n->entry_count >= i18n->entry_capacity) {
    rc = m3_i18n_grow(i18n, i18n->entry_count + 1u);
    M3_I18N_RETURN_IF_ERROR(rc);
  }

  rc = m3_i18n_alloc_string(&i18n->allocator, utf8_key, key_len, &key_copy);
  if (rc != M3_OK) {
    return rc;
  }

  value_copy = NULL;
  if (value_len > 0) {
    rc = m3_i18n_alloc_string(&i18n->allocator, utf8_value, value_len,
                              &value_copy);
    if (rc != M3_OK) {
      free_rc = i18n->allocator.free(i18n->allocator.ctx, key_copy);
      M3_UNUSED(free_rc);
      return rc;
    }
  }

  entry = &i18n->entries[i18n->entry_count];
  entry->key = key_copy;
  entry->key_len = key_len;
  entry->value = value_copy;
  entry->value_len = value_len;
  i18n->entry_count += 1u;
  return M3_OK;
}

int M3_CALL m3_i18n_get(const M3I18n *i18n, const char *utf8_key,
                        m3_usize key_len, const char **out_value,
                        m3_usize *out_value_len) {
  m3_usize index;
  M3Bool found;
  int rc;

  if (i18n == NULL || out_value == NULL || out_value_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_i18n_validate_key(utf8_key, key_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (found == M3_FALSE) {
    *out_value = NULL;
    *out_value_len = 0;
    return M3_ERR_NOT_FOUND;
  }

  *out_value = i18n->entries[index].value;
  *out_value_len = i18n->entries[index].value_len;
  return M3_OK;
}

int M3_CALL m3_i18n_contains(const M3I18n *i18n, const char *utf8_key,
                             m3_usize key_len, M3Bool *out_exists) {
  m3_usize index;
  M3Bool found;
  int rc;

  if (i18n == NULL || out_exists == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_i18n_validate_key(utf8_key, key_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  M3_I18N_RETURN_IF_ERROR(rc);

  *out_exists = found;
  return M3_OK;
}

int M3_CALL m3_i18n_remove(M3I18n *i18n, const char *utf8_key,
                           m3_usize key_len) {
  m3_usize index;
  m3_usize tail_count;
  M3Bool found;
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_i18n_validate_key(utf8_key, key_len);
  M3_I18N_RETURN_IF_ERROR(rc);

  rc = m3_i18n_find_entry(i18n, utf8_key, key_len, &index, &found);
  M3_I18N_RETURN_IF_ERROR(rc);

  if (found == M3_FALSE) {
    return M3_ERR_NOT_FOUND;
  }

  rc = m3_i18n_entry_release(i18n, &i18n->entries[index]);
  if (rc != M3_OK) {
    return rc;
  }

  tail_count = i18n->entry_count - index - 1u;
  if (tail_count > 0u) {
    memmove(&i18n->entries[index], &i18n->entries[index + 1u],
            (size_t)(tail_count * sizeof(M3I18nEntry)));
  }

  i18n->entry_count -= 1u;
  return M3_OK;
}

int M3_CALL m3_i18n_clear(M3I18n *i18n) {
  m3_usize i;
  int rc;
  int free_rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  rc = M3_OK;
  for (i = 0; i < i18n->entry_count; ++i) {
    free_rc = m3_i18n_entry_release(i18n, &i18n->entries[i]);
    if (free_rc != M3_OK && rc == M3_OK) {
      rc = free_rc;
    }
  }

  i18n->entry_count = 0;
  return rc;
}

int M3_CALL m3_i18n_count(const M3I18n *i18n, m3_usize *out_count) {
  if (i18n == NULL || out_count == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  *out_count = i18n->entry_count;
  return M3_OK;
}

int M3_CALL m3_i18n_load_table(M3I18n *i18n, M3IO *io, const char *utf8_path,
                               M3Bool clear_existing, M3Bool overwrite) {
  void *buffer;
  m3_usize size;
  int rc;
  int free_rc;

  if (i18n == NULL || io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }
  if (io->vtable == NULL || io->vtable->read_file_alloc == NULL) {
    return M3_ERR_UNSUPPORTED;
  }

  buffer = NULL;
  size = 0;
  rc = io->vtable->read_file_alloc(io->ctx, utf8_path, &i18n->allocator,
                                   &buffer, &size);
  if (rc != M3_OK) {
    return rc;
  }

  rc = m3_i18n_load_table_buffer(i18n, (const char *)buffer, size,
                                 clear_existing, overwrite);
  free_rc = i18n->allocator.free(i18n->allocator.ctx, buffer);
  if (free_rc != M3_OK && rc == M3_OK) {
    rc = free_rc;
  }
  return rc;
}

int M3_CALL m3_i18n_load_table_buffer(M3I18n *i18n, const char *data,
                                      m3_usize size, M3Bool clear_existing,
                                      M3Bool overwrite) {
  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  return m3_i18n_parse_table(i18n, data, size, clear_existing, overwrite);
}

int M3_CALL m3_i18n_format_number(const M3I18n *i18n,
                                  const M3I18nNumber *number, char *out_text,
                                  m3_usize text_capacity, m3_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_number != NULL) {
    rc = i18n->formatter.vtable->format_number(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, number, out_text,
        text_capacity, out_len);
    if (rc != M3_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return m3_i18n_format_number_internal(&i18n->locale, number, out_text,
                                        text_capacity, out_len);
}

int M3_CALL m3_i18n_format_date(const M3I18n *i18n, const M3Date *date,
                                char *out_text, m3_usize text_capacity,
                                m3_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_date != NULL) {
    rc = i18n->formatter.vtable->format_date(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, date, out_text,
        text_capacity, out_len);
    if (rc != M3_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return m3_i18n_format_date_internal(&i18n->locale, date, out_text,
                                      text_capacity, out_len);
}

int M3_CALL m3_i18n_format_time(const M3I18n *i18n, const M3Time *time,
                                char *out_text, m3_usize text_capacity,
                                m3_usize *out_len) {
  int rc;

  if (i18n == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (i18n->entries == NULL) {
    return M3_ERR_STATE;
  }

  if (i18n->formatter.vtable != NULL &&
      i18n->formatter.vtable->format_time != NULL) {
    rc = i18n->formatter.vtable->format_time(
        i18n->formatter.ctx, i18n->locale_tag, &i18n->locale, time, out_text,
        text_capacity, out_len);
    if (rc != M3_ERR_UNSUPPORTED) {
      return rc;
    }
  }

  return m3_i18n_format_time_internal(&i18n->locale, time, out_text,
                                      text_capacity, out_len);
}

#ifdef M3_TESTING
int M3_CALL m3_i18n_test_mul_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  return m3_i18n_mul_overflow(a, b, out_value);
}

int M3_CALL m3_i18n_test_add_overflow(m3_usize a, m3_usize b,
                                      m3_usize *out_value) {
  return m3_i18n_add_overflow(a, b, out_value);
}

int M3_CALL m3_i18n_test_pow10(m3_u32 digits, m3_u32 *out_value) {
  return m3_i18n_pow10(digits, out_value);
}

int M3_CALL m3_i18n_test_is_leap_year(m3_i32 year, M3Bool *out_leap) {
  return m3_i18n_is_leap_year(year, out_leap);
}

int M3_CALL m3_i18n_test_days_in_month(m3_i32 year, m3_u32 month,
                                       m3_u32 *out_days) {
  return m3_i18n_days_in_month(year, month, out_days);
}

int M3_CALL m3_i18n_test_validate_locale(const M3I18nLocale *locale) {
  return m3_i18n_validate_locale(locale);
}

int M3_CALL m3_i18n_test_validate_number(const M3I18nNumber *number) {
  return m3_i18n_validate_number(number);
}

int M3_CALL m3_i18n_test_validate_date(const M3Date *date) {
  return m3_i18n_validate_date(date);
}

int M3_CALL m3_i18n_test_validate_time(const M3Time *time) {
  return m3_i18n_validate_time(time);
}

int M3_CALL m3_i18n_test_cstrlen(const char *cstr, m3_usize *out_len) {
  return m3_i18n_cstrlen(cstr, out_len);
}

int M3_CALL m3_i18n_test_ascii_lower(char ch, char *out_lower) {
  return m3_i18n_ascii_lower(ch, out_lower);
}

int M3_CALL m3_i18n_test_tag_equals(const char *a, const char *b,
                                    M3Bool *out_equal) {
  return m3_i18n_tag_equals(a, b, out_equal);
}

int M3_CALL m3_i18n_test_trim_span(const char *data, m3_usize length,
                                   m3_usize *out_start, m3_usize *out_length) {
  return m3_i18n_trim_span(data, length, out_start, out_length);
}

int M3_CALL m3_i18n_test_alloc_string(const M3Allocator *allocator,
                                      const char *src, m3_usize len,
                                      char **out_str) {
  return m3_i18n_alloc_string(allocator, src, len, out_str);
}

int M3_CALL m3_i18n_test_write_uint(char *out_text, m3_usize text_capacity,
                                    m3_usize *io_offset, m3_u32 value,
                                    m3_u32 min_digits) {
  return m3_i18n_write_uint(out_text, text_capacity, io_offset, value,
                            min_digits);
}

int M3_CALL m3_i18n_test_find_entry(const M3I18n *i18n, const char *key,
                                    m3_usize key_len, m3_usize *out_index,
                                    M3Bool *out_found) {
  return m3_i18n_find_entry(i18n, key, key_len, out_index, out_found);
}

int M3_CALL m3_i18n_test_grow(M3I18n *i18n, m3_usize min_capacity) {
  return m3_i18n_grow(i18n, min_capacity);
}

int M3_CALL m3_i18n_test_set_cstr_limit(m3_usize max_len) {
  g_m3_i18n_cstr_limit = max_len;
  return M3_OK;
}
#endif
