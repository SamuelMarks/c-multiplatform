#include "m3/m3_date_picker.h"
#include "m3/m3_i18n.h"
#include "m3/m3_time_picker.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAlloc {
  int fail_alloc_on_call;
  int fail_realloc_on_call;
  int fail_free_on_call;
  int alloc_calls;
  int realloc_calls;
  int free_calls;
} TestAlloc;

static int test_alloc_reset(TestAlloc *alloc) {
  if (alloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(alloc, 0, sizeof(*alloc));
  return M3_OK;
}

static int test_alloc_fn(void *ctx, m3_usize size, void **out_ptr) {
  TestAlloc *alloc;
  void *ptr;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on_call > 0 &&
      alloc->alloc_calls == alloc->fail_alloc_on_call) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  ptr = malloc((size_t)size);
  if (ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = ptr;
  return M3_OK;
}

static int test_realloc_fn(void *ctx, void *ptr, m3_usize size,
                           void **out_ptr) {
  TestAlloc *alloc;
  void *next;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on_call > 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on_call) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  next = realloc(ptr, (size_t)size);
  if (next == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = next;
  return M3_OK;
}

static int test_free_fn(void *ctx, void *ptr) {
  TestAlloc *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on_call > 0 &&
      alloc->free_calls == alloc->fail_free_on_call) {
    return M3_ERR_IO;
  }

  free(ptr);
  return M3_OK;
}

typedef struct TestIO {
  const char *data;
  m3_usize size;
  int fail;
} TestIO;

static int test_io_read_file_alloc(void *io, const char *utf8_path,
                                   const M3Allocator *allocator,
                                   void **out_data, m3_usize *out_size) {
  TestIO *state;
  void *buffer;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->fail) {
    return M3_ERR_IO;
  }

  if (state->size == 0) {
    *out_data = NULL;
    *out_size = 0;
    return M3_OK;
  }

  rc = allocator->alloc(allocator->ctx, state->size, &buffer);
  if (rc != M3_OK) {
    return rc;
  }

  memcpy(buffer, state->data, (size_t)state->size);
  *out_data = buffer;
  *out_size = state->size;
  return M3_OK;
}

typedef struct TestFormatter {
  int mode;
} TestFormatter;

static int test_formatter_write(const char *text, char *out_text,
                                m3_usize text_capacity, m3_usize *out_len) {
  m3_usize len;

  if (out_len == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  len = (m3_usize)strlen(text);
  *out_len = len;
  if (text_capacity <= len) {
    return M3_ERR_RANGE;
  }

  memcpy(out_text, text, (size_t)len);
  out_text[len] = '\0';
  return M3_OK;
}

static int M3_CALL test_formatter_number(void *ctx, const char *locale_tag,
                                         const M3I18nLocale *locale,
                                         const M3I18nNumber *number,
                                         char *out_text, m3_usize text_capacity,
                                         m3_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || number == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return M3_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return M3_ERR_IO;
  }

  return test_formatter_write("FMT-N", out_text, text_capacity, out_len);
}

static int M3_CALL test_formatter_date(void *ctx, const char *locale_tag,
                                       const M3I18nLocale *locale,
                                       const M3Date *date, char *out_text,
                                       m3_usize text_capacity,
                                       m3_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || date == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return M3_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return M3_ERR_IO;
  }

  return test_formatter_write("FMT-D", out_text, text_capacity, out_len);
}

static int M3_CALL test_formatter_time(void *ctx, const char *locale_tag,
                                       const M3I18nLocale *locale,
                                       const M3Time *time, char *out_text,
                                       m3_usize text_capacity,
                                       m3_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || time == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return M3_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return M3_ERR_IO;
  }

  return test_formatter_write("FMT-T", out_text, text_capacity, out_len);
}

int main(void) {
  M3I18n i18n;
  M3I18n i18n_fail;
  M3I18nConfig config;
  M3I18nLocale locale;
  M3I18nLocale custom_locale;
  M3I18nNumber number;
  M3Date date;
  M3Time time;
  M3Allocator default_alloc;
  M3Allocator test_allocator;
  M3I18nFormatter formatter;
  M3I18nFormatterVTable formatter_vtable;
  TestFormatter formatter_state;
  TestAlloc alloc_state;
  TestAlloc alloc_state2;
  TestIO io_state;
  M3IO io;
  const char *value;
  m3_usize value_len;
  m3_usize count;
  m3_usize out_len;
  m3_usize index;
  m3_usize start;
  m3_usize span_len;
  m3_usize max_size;
  char buffer[128];
  char small_buffer[8];
  char lower;
  char *allocated;
  M3Bool flag;
  m3_u32 u32_value;
  m3_u32 days;
  int rc;

  memset(&i18n, 0, sizeof(i18n));
  memset(&i18n_fail, 0, sizeof(i18n_fail));
  memset(&config, 0, sizeof(config));
  memset(&locale, 0, sizeof(locale));
  memset(&custom_locale, 0, sizeof(custom_locale));
  memset(&number, 0, sizeof(number));
  memset(&date, 0, sizeof(date));
  memset(&time, 0, sizeof(time));
  memset(&default_alloc, 0, sizeof(default_alloc));
  memset(&test_allocator, 0, sizeof(test_allocator));
  memset(&formatter, 0, sizeof(formatter));
  memset(&formatter_vtable, 0, sizeof(formatter_vtable));
  memset(&formatter_state, 0, sizeof(formatter_state));
  memset(&alloc_state, 0, sizeof(alloc_state));
  memset(&alloc_state2, 0, sizeof(alloc_state2));
  memset(&io_state, 0, sizeof(io_state));
  memset(&io, 0, sizeof(io));
  memset(buffer, 0, sizeof(buffer));
  memset(small_buffer, 0, sizeof(small_buffer));

  max_size = (m3_usize) ~(m3_usize)0;

  M3_TEST_EXPECT(m3_i18n_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_locale_init(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_locale_from_tag(NULL, &locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_locale_from_tag("en-US", NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_from_tag("EN_us", &locale));
  M3_TEST_ASSERT(locale.date_order == M3_I18N_DATE_ORDER_MDY);
  M3_TEST_EXPECT(m3_i18n_locale_from_tag("zz-ZZ", &locale), M3_ERR_NOT_FOUND);

  M3_TEST_EXPECT(m3_i18n_test_mul_overflow(1, 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_add_overflow(1, 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_pow10(1, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_is_leap_year(2000, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_days_in_month(2020, 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_validate_number(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_validate_date(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_validate_time(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_cstrlen(NULL, &out_len), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_cstrlen("x", NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_ascii_lower('A', NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_tag_equals(NULL, "en", &flag),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_tag_equals("en", NULL, &flag),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_tag_equals("en", "en", NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_trim_span(NULL, 0, &start, &span_len),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_alloc_string(NULL, "x", 1, &allocated),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_alloc_string(&default_alloc, "x", 1, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_alloc_string(&default_alloc, NULL, 1, &allocated),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_write_uint(NULL, 4, &out_len, 1, 1),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_write_uint(buffer, 4, NULL, 1, 1),
                 M3_ERR_INVALID_ARGUMENT);

  number.integer = 0;
  number.fraction = 0;
  number.fraction_digits = 0u;
  date.year = 2024;
  date.month = 1;
  date.day = 1;
  time.hour = 0u;
  time.minute = 0u;
  M3_TEST_EXPECT(m3_i18n_set_locale(&i18n_fail, "en-US", NULL), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_set_formatter(&i18n_fail, NULL), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_put(&i18n_fail, "k", 1, "v", 1, M3_TRUE),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_get(&i18n_fail, "k", 1, &value, &value_len),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_contains(&i18n_fail, "k", 1, &flag), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_remove(&i18n_fail, "k", 1), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_clear(&i18n_fail), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_count(&i18n_fail, &count), M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_load_table(&i18n_fail, &io, "path", M3_FALSE, M3_TRUE),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(
      m3_i18n_load_table_buffer(&i18n_fail, "x", 1, M3_FALSE, M3_TRUE),
      M3_ERR_STATE);
  M3_TEST_EXPECT(m3_i18n_format_number(&i18n_fail, &number, buffer,
                                       sizeof(buffer), &out_len),
                 M3_ERR_STATE);
  M3_TEST_EXPECT(
      m3_i18n_format_date(&i18n_fail, &date, buffer, sizeof(buffer), &out_len),
      M3_ERR_STATE);
  M3_TEST_EXPECT(
      m3_i18n_format_time(&i18n_fail, &time, buffer, sizeof(buffer), &out_len),
      M3_ERR_STATE);

  M3_TEST_EXPECT(m3_i18n_test_mul_overflow(max_size, 2, &out_len),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_i18n_test_mul_overflow(2, 3, &out_len));
  M3_TEST_ASSERT(out_len == 6u);
  M3_TEST_EXPECT(m3_i18n_test_add_overflow(max_size, 1, &out_len),
                 M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_i18n_test_add_overflow(2, 3, &out_len));
  M3_TEST_ASSERT(out_len == 5u);

  M3_TEST_EXPECT(m3_i18n_test_pow10(10u, &u32_value), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_i18n_test_pow10(0u, &u32_value));
  M3_TEST_ASSERT(u32_value == 1u);

  M3_TEST_OK(m3_i18n_test_is_leap_year(2000, &flag));
  M3_TEST_ASSERT(flag == M3_TRUE);
  M3_TEST_OK(m3_i18n_test_is_leap_year(1900, &flag));
  M3_TEST_ASSERT(flag == M3_FALSE);
  M3_TEST_OK(m3_i18n_test_is_leap_year(2004, &flag));
  M3_TEST_ASSERT(flag == M3_TRUE);
  M3_TEST_OK(m3_i18n_test_is_leap_year(2001, &flag));
  M3_TEST_ASSERT(flag == M3_FALSE);

  M3_TEST_EXPECT(m3_i18n_test_days_in_month(2020, 0, &days), M3_ERR_RANGE);
  M3_TEST_EXPECT(m3_i18n_test_days_in_month(2020, 13, &days), M3_ERR_RANGE);
  M3_TEST_OK(m3_i18n_test_days_in_month(2020, 1, &days));
  M3_TEST_ASSERT(days == 31u);
  M3_TEST_OK(m3_i18n_test_days_in_month(2020, 4, &days));
  M3_TEST_ASSERT(days == 30u);
  M3_TEST_OK(m3_i18n_test_days_in_month(2000, 2, &days));
  M3_TEST_ASSERT(days == 29u);
  M3_TEST_OK(m3_i18n_test_days_in_month(2001, 2, &days));
  M3_TEST_ASSERT(days == 28u);

  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.decimal_separator = '\0';
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.thousands_separator = '\0';
  locale.grouping = 3u;
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.grouping = 10u;
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale), M3_ERR_RANGE);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.date_order = 99u;
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.time_format = 99u;
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  locale.pad_day = 2;
  M3_TEST_EXPECT(m3_i18n_test_validate_locale(&locale),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_locale_init(&locale));
  M3_TEST_OK(m3_i18n_test_validate_locale(&locale));

  number.integer = 1;
  number.fraction = 0;
  number.fraction_digits = M3_I18N_MAX_FRACTION_DIGITS + 1u;
  M3_TEST_EXPECT(m3_i18n_test_validate_number(&number), M3_ERR_RANGE);
  number.integer = 1;
  number.fraction = 1;
  number.fraction_digits = 0u;
  M3_TEST_EXPECT(m3_i18n_test_validate_number(&number), M3_ERR_RANGE);
  number.integer = 1;
  number.fraction = 100u;
  number.fraction_digits = 2u;
  M3_TEST_EXPECT(m3_i18n_test_validate_number(&number), M3_ERR_RANGE);
  number.integer = 1;
  number.fraction = 5u;
  number.fraction_digits = 2u;
  M3_TEST_OK(m3_i18n_test_validate_number(&number));

  date.year = -1;
  date.month = 1;
  date.day = 1;
  M3_TEST_EXPECT(m3_i18n_test_validate_date(&date), M3_ERR_RANGE);
  date.year = M3_DATE_MAX_YEAR + 1;
  date.month = 1;
  date.day = 1;
  M3_TEST_EXPECT(m3_i18n_test_validate_date(&date), M3_ERR_RANGE);
  date.year = 2024;
  date.month = 13;
  date.day = 1;
  M3_TEST_EXPECT(m3_i18n_test_validate_date(&date), M3_ERR_RANGE);
  date.year = 2024;
  date.month = 4;
  date.day = 31;
  M3_TEST_EXPECT(m3_i18n_test_validate_date(&date), M3_ERR_RANGE);
  date.year = 2024;
  date.month = 3;
  date.day = 4;
  M3_TEST_OK(m3_i18n_test_validate_date(&date));

  time.hour = 24u;
  time.minute = 0u;
  M3_TEST_EXPECT(m3_i18n_test_validate_time(&time), M3_ERR_RANGE);
  time.hour = 0u;
  time.minute = 60u;
  M3_TEST_EXPECT(m3_i18n_test_validate_time(&time), M3_ERR_RANGE);
  time.hour = 9u;
  time.minute = 5u;
  M3_TEST_OK(m3_i18n_test_validate_time(&time));

  M3_TEST_OK(m3_i18n_test_ascii_lower('A', &lower));
  M3_TEST_ASSERT(lower == 'a');
  M3_TEST_OK(m3_i18n_test_ascii_lower('z', &lower));
  M3_TEST_ASSERT(lower == 'z');

  M3_TEST_OK(m3_i18n_test_tag_equals("en_US", "EN-us", &flag));
  M3_TEST_ASSERT(flag == M3_TRUE);
  M3_TEST_OK(m3_i18n_test_tag_equals("en", "en-us", &flag));
  M3_TEST_ASSERT(flag == M3_FALSE);

  M3_TEST_OK(m3_i18n_test_trim_span("  x  ", 5, &start, &span_len));
  M3_TEST_ASSERT(start == 2u);
  M3_TEST_ASSERT(span_len == 1u);

  M3_TEST_OK(m3_get_default_allocator(&default_alloc));
  M3_TEST_OK(m3_i18n_test_alloc_string(&default_alloc, "hi", 2, &allocated));
  M3_TEST_ASSERT(strcmp(allocated, "hi") == 0);
  M3_TEST_OK(default_alloc.free(default_alloc.ctx, allocated));

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = test_realloc_fn;
  test_allocator.free = test_free_fn;
  alloc_state.fail_alloc_on_call = 1;
  M3_TEST_EXPECT(m3_i18n_test_alloc_string(&test_allocator, "x", 1, &allocated),
                 M3_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 0;

  out_len = 0;
  M3_TEST_EXPECT(m3_i18n_test_write_uint(small_buffer, 2, &out_len, 999u, 3u),
                 M3_ERR_RANGE);
  out_len = 0;
  M3_TEST_OK(
      m3_i18n_test_write_uint(buffer, sizeof(buffer), &out_len, 42u, 4u));
  M3_TEST_ASSERT(strcmp(buffer, "0042") == 0);

  M3_TEST_EXPECT(m3_i18n_test_cstrlen(NULL, &out_len), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_test_set_cstr_limit(1));
  M3_TEST_EXPECT(m3_i18n_test_cstrlen("ab", &out_len), M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_i18n_test_set_cstr_limit(0));

  M3_TEST_OK(m3_i18n_config_init(&config));
  M3_TEST_ASSERT(config.entry_capacity == (m3_usize)M3_I18N_DEFAULT_CAPACITY);
  M3_TEST_ASSERT(config.locale_tag != NULL);

#ifdef M3_TESTING
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
  M3_TEST_EXPECT(m3_i18n_init(&i18n, NULL), M3_ERR_UNKNOWN);
  M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));
#endif

  M3_TEST_EXPECT(m3_i18n_init(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_i18n_config_init(&config));
  config.entry_capacity = 0;
  M3_TEST_EXPECT(m3_i18n_init(&i18n, &config), M3_ERR_INVALID_ARGUMENT);

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = NULL;
  test_allocator.free = test_free_fn;
  M3_TEST_OK(m3_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  M3_TEST_EXPECT(m3_i18n_init(&i18n, &config), M3_ERR_INVALID_ARGUMENT);

  test_allocator.realloc = test_realloc_fn;
  M3_TEST_OK(m3_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  formatter.vtable = NULL;
  formatter.ctx = NULL;
  config.formatter = &formatter;
  M3_TEST_EXPECT(m3_i18n_init(&i18n, &config), M3_ERR_INVALID_ARGUMENT);

  config.formatter = NULL;
  config.locale_tag = "zz-ZZ";
  M3_TEST_EXPECT(m3_i18n_init(&i18n, &config), M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 2u;
  M3_TEST_OK(m3_i18n_init(&i18n, &config));
  M3_TEST_EXPECT(m3_i18n_init(&i18n, &config), M3_ERR_STATE);

  M3_TEST_EXPECT(m3_i18n_set_locale(NULL, "en-US", NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_set_formatter(NULL, NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_put(NULL, "k", 1, "v", 1, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_get(NULL, "k", 1, &value, &value_len),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_contains(NULL, "k", 1, &flag),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_remove(NULL, "k", 1), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_clear(NULL), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_count(NULL, &count), M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_load_table(NULL, &io, "path", M3_FALSE, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_load_table_buffer(NULL, "x", 1, M3_FALSE, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_number(NULL, &number, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_date(NULL, &date, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_time(NULL, &time, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_i18n_set_locale(&i18n, NULL, NULL),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_set_locale(&i18n, "", NULL), M3_ERR_INVALID_ARGUMENT);

  M3_TEST_EXPECT(m3_i18n_set_formatter(&i18n, &formatter),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_i18n_locale_init(&custom_locale));
  custom_locale.decimal_separator = '.';
  custom_locale.thousands_separator = ',';
  custom_locale.grouping = 3u;
  custom_locale.date_separator = '-';
  custom_locale.date_order = M3_I18N_DATE_ORDER_YMD;
  custom_locale.time_separator = ':';
  custom_locale.time_format = M3_I18N_TIME_FORMAT_24H;
  custom_locale.pad_day = M3_TRUE;
  custom_locale.pad_month = M3_TRUE;
  custom_locale.pad_hour = M3_TRUE;
  custom_locale.pad_minute = M3_TRUE;
  custom_locale.am = "AM";
  custom_locale.pm = "PM";

  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_EXPECT(m3_i18n_set_locale(&i18n, "zz-ZZ", NULL), M3_ERR_NOT_FOUND);

  formatter_vtable.format_number = test_formatter_number;
  formatter_vtable.format_date = test_formatter_date;
  formatter_vtable.format_time = test_formatter_time;
  formatter.vtable = &formatter_vtable;
  formatter.ctx = &formatter_state;

  M3_TEST_OK(m3_i18n_set_formatter(&i18n, &formatter));
  M3_TEST_OK(m3_i18n_set_formatter(&i18n, NULL));

  M3_TEST_EXPECT(m3_i18n_put(&i18n, NULL, 1, "v", 1, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_put(&i18n, "k", 0, "v", 1, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_put(&i18n, "k", 1, NULL, 1, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);

  {
    char invalid_utf8[2];
    invalid_utf8[0] = (char)0xC0;
    invalid_utf8[1] = '\0';
    M3_TEST_EXPECT(m3_i18n_put(&i18n, invalid_utf8, 1, "v", 1, M3_TRUE),
                   M3_ERR_CORRUPT);
  }

  M3_TEST_OK(m3_i18n_put(&i18n, "greeting", 8, "hello", 5, M3_FALSE));
  M3_TEST_EXPECT(m3_i18n_put(&i18n, "greeting", 8, "hi", 2, M3_FALSE),
                 M3_ERR_BUSY);
  M3_TEST_OK(m3_i18n_put(&i18n, "greeting", 8, "hi", 2, M3_TRUE));

  M3_TEST_OK(m3_i18n_get(&i18n, "greeting", 8, &value, &value_len));
  M3_TEST_ASSERT(value_len == 2u);
  M3_TEST_ASSERT(strncmp(value, "hi", 2) == 0);

  M3_TEST_OK(m3_i18n_contains(&i18n, "greeting", 8, &flag));
  M3_TEST_ASSERT(flag == M3_TRUE);
  M3_TEST_OK(m3_i18n_contains(&i18n, "missing", 7, &flag));
  M3_TEST_ASSERT(flag == M3_FALSE);

  M3_TEST_EXPECT(m3_i18n_get(&i18n, "missing", 7, &value, &value_len),
                 M3_ERR_NOT_FOUND);
  M3_TEST_ASSERT(value == NULL);
  M3_TEST_ASSERT(value_len == 0u);

  M3_TEST_OK(m3_i18n_count(&i18n, &count));
  M3_TEST_ASSERT(count == 1u);

  M3_TEST_OK(m3_i18n_remove(&i18n, "greeting", 8));
  M3_TEST_EXPECT(m3_i18n_remove(&i18n, "greeting", 8), M3_ERR_NOT_FOUND);

  M3_TEST_OK(m3_i18n_put(&i18n, "empty", 5, NULL, 0, M3_TRUE));
  M3_TEST_OK(m3_i18n_get(&i18n, "empty", 5, &value, &value_len));
  M3_TEST_ASSERT(value_len == 0u);

  M3_TEST_OK(m3_i18n_clear(&i18n));
  M3_TEST_OK(m3_i18n_count(&i18n, &count));
  M3_TEST_ASSERT(count == 0u);

  M3_TEST_OK(m3_i18n_put(&i18n, "one", 3, "1", 1, M3_TRUE));
  M3_TEST_OK(m3_i18n_put(&i18n, "two", 3, "2", 1, M3_TRUE));
  M3_TEST_OK(m3_i18n_test_find_entry(&i18n, "one", 3, &index, &flag));
  M3_TEST_ASSERT(flag == M3_TRUE);
  M3_TEST_OK(m3_i18n_test_find_entry(&i18n, "missing", 7, &index, &flag));
  M3_TEST_ASSERT(flag == M3_FALSE);
  M3_TEST_EXPECT(m3_i18n_test_find_entry(&i18n, "one", 3, NULL, &flag),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_test_find_entry(&i18n, "one", 3, &index, NULL),
                 M3_ERR_INVALID_ARGUMENT);

  M3_TEST_OK(m3_i18n_clear(&i18n));

  rc = m3_i18n_test_grow(&i18n, i18n.entry_capacity + 1u);
  M3_TEST_OK(rc);

  {
    M3I18n grow_i18n;
    M3I18nEntry *entries;

    memset(&grow_i18n, 0, sizeof(grow_i18n));
    test_alloc_reset(&alloc_state2);
    test_allocator.ctx = &alloc_state2;
    test_allocator.alloc = test_alloc_fn;
    test_allocator.realloc = test_realloc_fn;
    test_allocator.free = test_free_fn;

    M3_TEST_OK(test_allocator.alloc(
        test_allocator.ctx, (m3_usize)sizeof(M3I18nEntry), (void **)&entries));
    memset(entries, 0, sizeof(M3I18nEntry));
    grow_i18n.entries = entries;
    grow_i18n.entry_capacity = 1u;
    grow_i18n.allocator = test_allocator;

    alloc_state2.fail_realloc_on_call = 1;
    M3_TEST_EXPECT(m3_i18n_test_grow(&grow_i18n, 2u), M3_ERR_OUT_OF_MEMORY);
    alloc_state2.fail_realloc_on_call = 0;
    M3_TEST_OK(m3_i18n_test_grow(&grow_i18n, 2u));
    M3_TEST_ASSERT(grow_i18n.entry_capacity >= 2u);

    M3_TEST_OK(test_allocator.free(test_allocator.ctx, grow_i18n.entries));
  }

  {
    M3I18n grow_i18n;

    memset(&grow_i18n, 0, sizeof(grow_i18n));
    grow_i18n.entries = (M3I18nEntry *)buffer;
    grow_i18n.entry_capacity = 1u;
    grow_i18n.allocator.realloc = NULL;
    M3_TEST_EXPECT(m3_i18n_test_grow(&grow_i18n, 2u), M3_ERR_INVALID_ARGUMENT);
  }

  {
    const char *table_data = "# comment\n"
                             " greeting = hello \n"
                             "empty=\n"
                             "; more comment\n"
                             "farewell=bye\n";
    M3_TEST_OK(m3_i18n_load_table_buffer(
        &i18n, table_data, (m3_usize)strlen(table_data), M3_TRUE, M3_TRUE));
    M3_TEST_OK(m3_i18n_get(&i18n, "greeting", 8, &value, &value_len));
    M3_TEST_ASSERT(strncmp(value, "hello", value_len) == 0);
    M3_TEST_OK(m3_i18n_get(&i18n, "empty", 5, &value, &value_len));
    M3_TEST_ASSERT(value_len == 0u);
  }

  {
    const char *bad_table = "badline\n";
    M3_TEST_EXPECT(m3_i18n_load_table_buffer(&i18n, bad_table,
                                             (m3_usize)strlen(bad_table),
                                             M3_FALSE, M3_TRUE),
                   M3_ERR_CORRUPT);
  }

  {
    const char *bad_table = "=value\n";
    M3_TEST_EXPECT(m3_i18n_load_table_buffer(&i18n, bad_table,
                                             (m3_usize)strlen(bad_table),
                                             M3_FALSE, M3_TRUE),
                   M3_ERR_CORRUPT);
  }

  {
    char bom_table[] = "\xEF\xBB\xBFkey=value\n";
    M3_TEST_OK(m3_i18n_clear(&i18n));
    M3_TEST_OK(m3_i18n_load_table_buffer(
        &i18n, bom_table, (m3_usize)strlen(bom_table), M3_FALSE, M3_TRUE));
    M3_TEST_OK(m3_i18n_get(&i18n, "key", 3, &value, &value_len));
    M3_TEST_ASSERT(strncmp(value, "value", value_len) == 0);
  }

  M3_TEST_EXPECT(m3_i18n_load_table_buffer(&i18n, "x", 1, (M3Bool)2, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_load_table_buffer(&i18n, "x", 1, M3_FALSE, (M3Bool)2),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_load_table_buffer(&i18n, NULL, 1, M3_FALSE, M3_TRUE),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_OK(m3_i18n_load_table_buffer(&i18n, NULL, 0, M3_FALSE, M3_FALSE));

  io.vtable = NULL;
  M3_TEST_EXPECT(m3_i18n_load_table(&i18n, &io, "path", M3_FALSE, M3_TRUE),
                 M3_ERR_UNSUPPORTED);

  {
    static const M3IOVTable io_vtable = {
        NULL, test_io_read_file_alloc, NULL, NULL, NULL, NULL};
    io.vtable = &io_vtable;
    io_state.data = "key=value\n";
    io_state.size = (m3_usize)strlen(io_state.data);
    io_state.fail = 0;
    io.ctx = &io_state;
    M3_TEST_OK(m3_i18n_load_table(&i18n, &io, "path", M3_TRUE, M3_TRUE));
    M3_TEST_OK(m3_i18n_get(&i18n, "key", 3, &value, &value_len));
    M3_TEST_ASSERT(strncmp(value, "value", value_len) == 0);

    io_state.fail = 1;
    M3_TEST_EXPECT(m3_i18n_load_table(&i18n, &io, "path", M3_FALSE, M3_TRUE),
                   M3_ERR_IO);
    io_state.fail = 0;
  }

  formatter_state.mode = 1;
  formatter.vtable = &formatter_vtable;
  formatter.ctx = &formatter_state;
  M3_TEST_OK(m3_i18n_set_formatter(&i18n, &formatter));

  number.integer = 123;
  number.fraction = 0;
  number.fraction_digits = 0u;
  M3_TEST_OK(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "FMT-N") == 0);

  formatter_state.mode = 0;
  M3_TEST_OK(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "123") == 0);

  formatter_state.mode = 2;
  M3_TEST_EXPECT(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len),
      M3_ERR_IO);

  formatter_state.mode = 0;
  M3_TEST_OK(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  M3_TEST_OK(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));

  M3_TEST_OK(m3_i18n_set_formatter(&i18n, NULL));

  number.integer = -12345;
  number.fraction = 67u;
  number.fraction_digits = 2u;
  M3_TEST_OK(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "-12,345.67") == 0);

  number.integer = 1;
  number.fraction = 1u;
  number.fraction_digits = 0u;
  M3_TEST_EXPECT(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len),
      M3_ERR_RANGE);

  number.integer = 1;
  number.fraction = 0u;
  number.fraction_digits = 0u;
  M3_TEST_EXPECT(m3_i18n_format_number(&i18n, &number, NULL, 1, &out_len),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_format_number(&i18n, &number, buffer, 1, &out_len),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_i18n_format_number(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), NULL),
      M3_ERR_INVALID_ARGUMENT);

  date.year = 2024;
  date.month = 3;
  date.day = 4;
  M3_TEST_OK(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "2024-03-04") == 0);

  custom_locale = i18n.locale;
  custom_locale.date_order = M3_I18N_DATE_ORDER_MDY;
  custom_locale.date_separator = '/';
  custom_locale.pad_day = M3_TRUE;
  custom_locale.pad_month = M3_TRUE;
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_OK(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "03/04/2024") == 0);

  custom_locale.date_order = M3_I18N_DATE_ORDER_DMY;
  custom_locale.pad_day = M3_FALSE;
  custom_locale.pad_month = M3_FALSE;
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_OK(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "4/3/2024") == 0);

  date.month = 2;
  date.day = 30;
  M3_TEST_EXPECT(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len),
      M3_ERR_RANGE);
  date.month = 3;
  date.day = 4;

  M3_TEST_EXPECT(m3_i18n_format_date(&i18n, &date, buffer, 4, &out_len),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_i18n_format_date(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_format_date(&i18n, &date, NULL, 1, &out_len),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), NULL),
      M3_ERR_INVALID_ARGUMENT);

  time.hour = 0u;
  time.minute = 5u;
  custom_locale.time_format = M3_I18N_TIME_FORMAT_12H;
  custom_locale.pad_hour = M3_TRUE;
  custom_locale.pad_minute = M3_TRUE;
  custom_locale.time_separator = ':';
  custom_locale.am = "AM";
  custom_locale.pm = "PM";
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_OK(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "12:05 AM") == 0);

  time.hour = 23u;
  time.minute = 7u;
  M3_TEST_OK(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "11:07 PM") == 0);

  custom_locale.time_format = M3_I18N_TIME_FORMAT_24H;
  custom_locale.pad_hour = M3_FALSE;
  custom_locale.pad_minute = M3_TRUE;
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  time.hour = 9u;
  time.minute = 5u;
  M3_TEST_OK(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "9:05") == 0);

  time.hour = 24u;
  time.minute = 0u;
  M3_TEST_EXPECT(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len),
      M3_ERR_RANGE);
  time.hour = 9u;
  time.minute = 5u;

  M3_TEST_EXPECT(m3_i18n_format_time(&i18n, &time, buffer, 4, &out_len),
                 M3_ERR_RANGE);
  M3_TEST_EXPECT(
      m3_i18n_format_time(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(m3_i18n_format_time(&i18n, &time, NULL, 1, &out_len),
                 M3_ERR_INVALID_ARGUMENT);
  M3_TEST_EXPECT(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), NULL),
      M3_ERR_INVALID_ARGUMENT);

  custom_locale.time_format = M3_I18N_TIME_FORMAT_12H;
  custom_locale.am = "AM";
  custom_locale.pm = "PM";
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_OK(m3_i18n_test_set_cstr_limit(1));
  time.hour = 1u;
  time.minute = 0u;
  M3_TEST_EXPECT(
      m3_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len),
      M3_ERR_OVERFLOW);
  M3_TEST_OK(m3_i18n_test_set_cstr_limit(max_size));

  number.integer = 1234;
  number.fraction = 0u;
  number.fraction_digits = 0u;
  M3_TEST_OK(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "1,234") == 0);

  custom_locale.grouping = 0u;
  custom_locale.thousands_separator = '\0';
  M3_TEST_OK(m3_i18n_set_locale(&i18n, "en-US", &custom_locale));
  M3_TEST_OK(
      m3_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  M3_TEST_ASSERT(strcmp(buffer, "1234") == 0);

  M3_TEST_OK(m3_i18n_shutdown(&i18n));

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = test_realloc_fn;
  test_allocator.free = test_free_fn;

  M3_TEST_OK(m3_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  M3_TEST_OK(m3_i18n_init(&i18n_fail, &config));
  M3_TEST_OK(test_alloc_reset(&alloc_state));

  alloc_state.fail_alloc_on_call = 1;
  M3_TEST_EXPECT(m3_i18n_put(&i18n_fail, "a", 1, "b", 1, M3_TRUE),
                 M3_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 2;
  M3_TEST_EXPECT(m3_i18n_put(&i18n_fail, "a", 1, "b", 1, M3_TRUE),
                 M3_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 0;

  M3_TEST_OK(m3_i18n_put(&i18n_fail, "a", 1, "b", 1, M3_TRUE));
  alloc_state.fail_realloc_on_call = 1;
  M3_TEST_EXPECT(m3_i18n_put(&i18n_fail, "c", 1, "d", 1, M3_TRUE),
                 M3_ERR_OUT_OF_MEMORY);
  alloc_state.fail_realloc_on_call = 0;

  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  M3_TEST_EXPECT(m3_i18n_clear(&i18n_fail), M3_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  M3_TEST_OK(m3_i18n_clear(&i18n_fail));
  M3_TEST_OK(m3_i18n_put(&i18n_fail, "x", 1, "y", 1, M3_TRUE));

  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  M3_TEST_EXPECT(m3_i18n_shutdown(&i18n_fail), M3_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  memset(&i18n_fail, 0, sizeof(i18n_fail));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  M3_TEST_OK(m3_i18n_init(&i18n_fail, &config));

  {
    static const M3IOVTable io_vtable = {
        NULL, test_io_read_file_alloc, NULL, NULL, NULL, NULL};
    io.vtable = &io_vtable;
    io_state.data = "k=v\n";
    io_state.size = (m3_usize)strlen(io_state.data);
    io_state.fail = 0;
    io.ctx = &io_state;

    alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
    M3_TEST_EXPECT(
        m3_i18n_load_table(&i18n_fail, &io, "path", M3_TRUE, M3_TRUE),
        M3_ERR_IO);
    alloc_state.fail_free_on_call = 0;
  }

  M3_TEST_OK(m3_i18n_shutdown(&i18n_fail));

  M3_TEST_EXPECT(m3_i18n_shutdown(&i18n_fail), M3_ERR_STATE);

  return 0;
}
