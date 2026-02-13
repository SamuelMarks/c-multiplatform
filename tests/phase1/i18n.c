#include "cmpc/cmp_datetime.h"
#include "cmpc/cmp_i18n.h"
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
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(alloc, 0, sizeof(*alloc));
  return CMP_OK;
}

static int test_alloc_fn(void *ctx, cmp_usize size, void **out_ptr) {
  TestAlloc *alloc;
  void *ptr;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on_call > 0 &&
      alloc->alloc_calls == alloc->fail_alloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ptr = malloc((size_t)size);
  if (ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = ptr;
  return CMP_OK;
}

static int test_realloc_fn(void *ctx, void *ptr, cmp_usize size,
                           void **out_ptr) {
  TestAlloc *alloc;
  void *next;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_ptr = NULL;

  alloc = (TestAlloc *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on_call > 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on_call) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  next = realloc(ptr, (size_t)size);
  if (next == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = next;
  return CMP_OK;
}

static int test_free_fn(void *ctx, void *ptr) {
  TestAlloc *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAlloc *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_on_call > 0 &&
      alloc->free_calls == alloc->fail_free_on_call) {
    return CMP_ERR_IO;
  }

  free(ptr);
  return CMP_OK;
}

typedef struct TestIO {
  const char *data;
  cmp_usize size;
  int fail;
} TestIO;

static int test_io_read_file_alloc(void *io, const char *utf8_path,
                                   const CMPAllocator *allocator,
                                   void **out_data, cmp_usize *out_size) {
  TestIO *state;
  void *buffer;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestIO *)io;
  if (state->fail) {
    return CMP_ERR_IO;
  }

  if (state->size == 0) {
    *out_data = NULL;
    *out_size = 0;
    return CMP_OK;
  }

  rc = allocator->alloc(allocator->ctx, state->size, &buffer);
  if (rc != CMP_OK) {
    return rc;
  }

  memcpy(buffer, state->data, (size_t)state->size);
  *out_data = buffer;
  *out_size = state->size;
  return CMP_OK;
}

typedef struct TestFormatter {
  int mode;
} TestFormatter;

static int test_formatter_write(const char *text, char *out_text,
                                cmp_usize text_capacity, cmp_usize *out_len) {
  cmp_usize len;

  if (out_len == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (out_text == NULL && text_capacity != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  len = (cmp_usize)strlen(text);
  *out_len = len;
  if (text_capacity <= len) {
    return CMP_ERR_RANGE;
  }

  memcpy(out_text, text, (size_t)len);
  out_text[len] = '\0';
  return CMP_OK;
}

static int CMP_CALL test_formatter_number(void *ctx, const char *locale_tag,
                                          const CMPI18nLocale *locale,
                                          const CMPI18nNumber *number,
                                          char *out_text,
                                          cmp_usize text_capacity,
                                          cmp_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || number == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return CMP_ERR_IO;
  }

  return test_formatter_write("FMT-N", out_text, text_capacity, out_len);
}

static int CMP_CALL test_formatter_date(void *ctx, const char *locale_tag,
                                        const CMPI18nLocale *locale,
                                        const CMPDate *date, char *out_text,
                                        cmp_usize text_capacity,
                                        cmp_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || date == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return CMP_ERR_IO;
  }

  return test_formatter_write("FMT-D", out_text, text_capacity, out_len);
}

static int CMP_CALL test_formatter_time(void *ctx, const char *locale_tag,
                                        const CMPI18nLocale *locale,
                                        const CMPTime *time, char *out_text,
                                        cmp_usize text_capacity,
                                        cmp_usize *out_len) {
  TestFormatter *state;

  if (ctx == NULL || locale_tag == NULL || locale == NULL || time == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestFormatter *)ctx;
  if (state->mode == 0) {
    return CMP_ERR_UNSUPPORTED;
  }
  if (state->mode == 2) {
    return CMP_ERR_IO;
  }

  return test_formatter_write("FMT-T", out_text, text_capacity, out_len);
}

static int test_i18n_coverage_hooks(TestAlloc *alloc_state,
                                    const CMPAllocator *allocator) {
  CMPI18n i18n_local;
  CMPI18nConfig config;
  CMPI18nLocale locale;
  CMPI18nNumber number;
  CMPDate date;
  CMPBool equal;
  cmp_u32 days;
  char buffer[128];
  cmp_usize out_len;

  if (alloc_state == NULL || allocator == NULL) {
    return 1;
  }

  number.integer = 1;
  number.fraction = 0;
  number.fraction_digits = 1;
  CMP_TEST_OK(cmp_i18n_test_force_pow10_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_IO);

  CMP_TEST_OK(cmp_i18n_test_force_leap_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_test_days_in_month(2024, 2u, &days), CMP_ERR_IO);

  date.year = 2024;
  date.month = 2u;
  date.day = 1u;
  CMP_TEST_OK(cmp_i18n_test_force_leap_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_IO);

  CMP_TEST_OK(cmp_i18n_test_force_days_default(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_test_days_in_month(2024, 2u, &days), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_i18n_test_days_in_month(2024, 1u, &days));
  CMP_TEST_ASSERT(days == 31u);

  CMP_TEST_OK(cmp_i18n_test_tag_equals("en-US", "en_US", &equal));
  CMP_TEST_ASSERT(equal == CMP_TRUE);

  CMP_TEST_EXPECT(cmp_i18n_test_locale_preset_en_us(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_locale_preset_en_gb(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_locale_preset_fr_fr(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_locale_preset_de_de(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(1u));
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("zz-ZZ", &locale), CMP_ERR_IO);
  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(2u));
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals("aa", "aa", &equal), CMP_ERR_IO);
  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(3u));
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("zz-ZZ", &locale), CMP_ERR_IO);
  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(5u));
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("zz-ZZ", &locale), CMP_ERR_IO);
  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(7u));
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("zz-ZZ", &locale), CMP_ERR_IO);
  CMP_TEST_OK(cmp_i18n_test_set_ascii_lower_fail_after(0u));

  CMP_TEST_OK(cmp_i18n_test_force_utf8_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_test_validate_utf8("a", 1u), CMP_ERR_IO);

  CMP_TEST_EXPECT(cmp_i18n_test_parse_table(NULL, "", 0u, CMP_FALSE, CMP_FALSE),
                  CMP_ERR_INVALID_ARGUMENT);

  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 4u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  {
    const char *data = "\n\r\n   \t\r\n#comment\n;comment\nkey=value\r\n";
    CMP_TEST_OK(cmp_i18n_test_parse_table(
        &i18n_local, data, (cmp_usize)strlen(data), CMP_TRUE, CMP_TRUE));
  }
  {
    const char *data = "badline\n";
    CMP_TEST_EXPECT(cmp_i18n_test_parse_table(&i18n_local, data,
                                              (cmp_usize)strlen(data), CMP_TRUE,
                                              CMP_TRUE),
                    CMP_ERR_CORRUPT);
  }
  {
    const char *data = "=value\n";
    CMP_TEST_EXPECT(cmp_i18n_test_parse_table(&i18n_local, data,
                                              (cmp_usize)strlen(data), CMP_TRUE,
                                              CMP_TRUE),
                    CMP_ERR_CORRUPT);
  }
  {
    const char *data = "k=v\nk=w\n";
    CMP_TEST_EXPECT(cmp_i18n_test_parse_table(&i18n_local, data,
                                              (cmp_usize)strlen(data), CMP_TRUE,
                                              CMP_FALSE),
                    CMP_ERR_BUSY);
  }
  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_local));

  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_test_force_config_init_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n_local, NULL), CMP_ERR_IO);

  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = (cmp_usize) ~(cmp_usize)0;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n_local, &config), CMP_ERR_OVERFLOW);

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  alloc_state->fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n_local, &config), CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  config.locale_tag = NULL;
  config.locale = NULL;
  alloc_state->fail_alloc_on_call = 2;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n_local, &config), CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;

  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.locale = &locale;
  config.locale_tag = NULL;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_local));

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  alloc_state->fail_alloc_on_call = alloc_state->alloc_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n_local, &config), CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));

  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(1u));
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_local, "en-US", NULL),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(0u));

  CMP_TEST_OK(cmp_i18n_test_force_utf8_error(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_local, "en-US", NULL), CMP_ERR_IO);

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  alloc_state->fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_local, "en-US", NULL),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;

  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.decimal_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_local, "en-US", &locale),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_test_force_utf8_ok(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_local, "a", 1u, NULL, 1u, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);

  date.year = 10000;
  date.month = 1u;
  date.day = 1u;
  CMP_TEST_OK(cmp_i18n_format_date(&i18n_local, &date, buffer, sizeof(buffer),
                                   &out_len));

  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_local));

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  CMP_TEST_OK(cmp_i18n_put(&i18n_local, "a", 1u, "b", 1u, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_put(&i18n_local, "c", 1u, "d", 1u, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_remove(&i18n_local, "a", 1u));
  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_local));

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  CMP_TEST_OK(cmp_i18n_put(&i18n_local, "k", 1u, "v", 1u, CMP_TRUE));
  alloc_state->fail_free_on_call = alloc_state->free_calls + 2;
  CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_local), CMP_ERR_IO);
  alloc_state->fail_free_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  CMP_TEST_OK(cmp_i18n_put(&i18n_local, "k", 1u, "v", 1u, CMP_TRUE));
  alloc_state->fail_free_on_call = alloc_state->free_calls + 3;
  CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_local), CMP_ERR_IO);
  alloc_state->fail_free_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  alloc_state->fail_alloc_on_call = alloc_state->alloc_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_local, "x", 1u, "y", 1u, CMP_TRUE),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;
  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_local));

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  memset(&i18n_local, 0, sizeof(i18n_local));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_local, &config));
  CMP_TEST_OK(cmp_i18n_put(&i18n_local, "k", 1u, "v", 1u, CMP_TRUE));
  alloc_state->fail_free_on_call = alloc_state->free_calls + 4;
  CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_local), CMP_ERR_IO);
  alloc_state->fail_free_on_call = 0;

  return 0;
}

static int test_i18n_branch_sweep(TestAlloc *alloc_state,
                                  const CMPAllocator *allocator) {
  CMPI18n i18n;
  CMPI18nConfig config;
  CMPI18nLocale locale;
  CMPI18nNumber number;
  CMPDate date;
  CMPTime time;
  cmp_usize out_len;
  cmp_usize out_usize;
  cmp_usize index;
  CMPBool equal;
  CMPBool found;
  cmp_u32 out_u32;
  char buffer[64];
  char lower;
  char *allocated;

  if (alloc_state == NULL || allocator == NULL) {
    return 1;
  }

  CMP_TEST_EXPECT(cmp_i18n_test_mul_overflow(1u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_test_mul_overflow((cmp_usize) ~(cmp_usize)0, 2u, &out_usize),
      CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_mul_overflow(3u, 4u, &out_usize));

  CMP_TEST_EXPECT(cmp_i18n_test_add_overflow(1u, 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_test_add_overflow((cmp_usize) ~(cmp_usize)0, 1u, &out_usize),
      CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_add_overflow(5u, 6u, &out_usize));

  CMP_TEST_EXPECT(cmp_i18n_test_pow10(1u, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_pow10(10u, &out_u32), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_pow10(3u, &out_u32));

  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.decimal_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.decimal_separator = '.';
  locale.date_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.date_separator = '/';
  locale.time_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.time_separator = ':';
  locale.grouping = 3u;
  locale.thousands_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.thousands_separator = ',';
  locale.grouping = 10u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale), CMP_ERR_RANGE);
  locale.grouping = 3u;
  locale.date_order = (cmp_u32)99;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.date_order = CMP_I18N_DATE_ORDER_MDY;
  locale.time_format = (cmp_u32)99;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.time_format = CMP_I18N_TIME_FORMAT_24H;
  locale.pad_day = (CMPBool)2;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.pad_day = CMP_TRUE;
  locale.pad_month = (CMPBool)2;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.pad_month = CMP_TRUE;
  locale.pad_hour = (CMPBool)2;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.pad_hour = CMP_TRUE;
  locale.pad_minute = (CMPBool)2;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  locale.pad_minute = CMP_TRUE;
  CMP_TEST_OK(cmp_i18n_test_validate_locale(&locale));

  memset(&number, 0, sizeof(number));
  number.fraction_digits = CMP_I18N_MAX_FRACTION_DIGITS + 1u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.fraction_digits = 0u;
  number.fraction = 1u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.fraction_digits = 2u;
  number.fraction = 100u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.integer = -1234;
  number.fraction = 5u;
  number.fraction_digits = 1u;
  CMP_TEST_OK(cmp_i18n_test_validate_number(&number));

  memset(&date, 0, sizeof(date));
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(NULL), CMP_ERR_INVALID_ARGUMENT);
  date.year = 2024;
  date.month = 0u;
  date.day = 1u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.month = 2u;
  date.day = 31u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.day = 29u;
  CMP_TEST_OK(cmp_i18n_test_validate_date(&date));

  memset(&time, 0, sizeof(time));
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(NULL), CMP_ERR_INVALID_ARGUMENT);
  time.hour = 24u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(&time), CMP_ERR_RANGE);
  time.hour = 23u;
  time.minute = 60u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(&time), CMP_ERR_RANGE);
  time.minute = 59u;
  CMP_TEST_OK(cmp_i18n_test_validate_time(&time));

  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen(NULL, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen("abc", NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(3u));
  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen("abcd", &out_len), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(0u));

  CMP_TEST_EXPECT(cmp_i18n_test_ascii_lower('A', NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_ascii_lower('A', &lower));
  CMP_TEST_ASSERT(lower == 'a');
  CMP_TEST_OK(cmp_i18n_test_ascii_lower('?', &lower));
  CMP_TEST_ASSERT(lower == '?');

  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals(NULL, "a", &equal),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals("a", NULL, &equal),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals("a", "b", NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_tag_equals("en-US", "en-US", &equal));
  CMP_TEST_ASSERT(equal == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_test_tag_equals("en-US", "fr-FR", &equal));
  CMP_TEST_ASSERT(equal == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_i18n_test_trim_span(NULL, 0u, &index, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_trim_span("x", 1u, NULL, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_trim_span("x", 1u, &index, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_trim_span("  a  ", 5u, &index, &out_len));
  CMP_TEST_ASSERT(index == 2u);
  CMP_TEST_ASSERT(out_len == 1u);
  CMP_TEST_OK(cmp_i18n_test_trim_span("   ", 3u, &index, &out_len));
  CMP_TEST_ASSERT(out_len == 0u);

  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(NULL, "x", 1u, &allocated),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(allocator, NULL, 1u, &allocated),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(allocator, "x", 1u, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  allocated = NULL;
  alloc_state->fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(allocator, "x", 1u, &allocated),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_alloc_on_call = 0;
  CMP_TEST_OK(cmp_i18n_test_alloc_string(allocator, "hi", 2u, &allocated));
  CMP_TEST_OK(allocator->free(allocator->ctx, allocated));

  out_len = 0u;
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(NULL, 1u, &out_len, 10u, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(buffer, 2u, NULL, 10u, 1u),
                  CMP_ERR_INVALID_ARGUMENT);
  out_len = 0u;
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(buffer, 1u, &out_len, 10u, 1u),
                  CMP_ERR_RANGE);
  out_len = 0u;
  CMP_TEST_OK(
      cmp_i18n_test_write_uint(buffer, sizeof(buffer), &out_len, 10u, 4u));

  memset(&i18n, 0, sizeof(i18n));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n, &config));
  CMP_TEST_EXPECT(cmp_i18n_test_find_entry(&i18n, "a", 1u, NULL, &found),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_find_entry(&i18n, "a", 1u, &index, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_find_entry(&i18n, "missing", 7u, &index, &found));
  CMP_TEST_ASSERT(found == CMP_FALSE);
  CMP_TEST_OK(cmp_i18n_put(&i18n, "key", 3u, "value", 5u, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_test_find_entry(&i18n, "key", 3u, &index, &found));
  CMP_TEST_ASSERT(found == CMP_TRUE);

  CMP_TEST_OK(test_alloc_reset(alloc_state));
  alloc_state->fail_realloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_test_grow(&i18n, 8u), CMP_ERR_OUT_OF_MEMORY);
  alloc_state->fail_realloc_on_call = 0;
  CMP_TEST_OK(cmp_i18n_test_grow(&i18n, 2u));

  CMP_TEST_OK(cmp_i18n_shutdown(&i18n));

  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  memset(&i18n, 0, sizeof(i18n));
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = allocator;
  config.locale = &locale;
  config.locale_tag = "zz-ZZ";
  CMP_TEST_OK(cmp_i18n_init(&i18n, &config));

  number.integer = -12345;
  number.fraction = 42u;
  number.fraction_digits = 2u;
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_EXPECT(cmp_i18n_format_number(&i18n, &number, buffer, 2u, &out_len),
                  CMP_ERR_RANGE);

  date.year = 2025;
  date.month = 12u;
  date.day = 31u;
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  locale.date_order = CMP_I18N_DATE_ORDER_DMY;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "zz-ZZ", &locale));
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  locale.date_order = CMP_I18N_DATE_ORDER_YMD;
  locale.pad_day = CMP_FALSE;
  locale.pad_month = CMP_FALSE;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "zz-ZZ", &locale));
  CMP_TEST_EXPECT(cmp_i18n_format_date(&i18n, &date, buffer, 4u, &out_len),
                  CMP_ERR_RANGE);

  time.hour = 13u;
  time.minute = 5u;
  locale.time_format = CMP_I18N_TIME_FORMAT_12H;
  locale.pad_hour = CMP_TRUE;
  locale.pad_minute = CMP_FALSE;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "zz-ZZ", &locale));
  CMP_TEST_OK(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  locale.time_format = CMP_I18N_TIME_FORMAT_24H;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "zz-ZZ", &locale));
  CMP_TEST_EXPECT(cmp_i18n_format_time(&i18n, &time, buffer, 3u, &out_len),
                  CMP_ERR_RANGE);

  CMP_TEST_OK(cmp_i18n_shutdown(&i18n));

  return 0;
}

int main(void) {
  CMPI18n i18n;
  CMPI18n i18n_fail;
  CMPI18nConfig config;
  CMPI18nLocale locale;
  CMPI18nLocale custom_locale;
  CMPI18nNumber number;
  CMPDate date;
  CMPTime time;
  CMPAllocator default_alloc;
  CMPAllocator test_allocator;
  CMPI18nFormatter formatter;
  CMPI18nFormatterVTable formatter_vtable;
  TestFormatter formatter_state;
  TestAlloc alloc_state;
  TestAlloc alloc_state2;
  TestIO io_state;
  CMPIO io;
  const char *value;
  cmp_usize value_len;
  cmp_usize count;
  cmp_usize out_len;
  cmp_usize index;
  cmp_usize start;
  cmp_usize span_len;
  cmp_usize max_size;
  char buffer[128];
  char small_buffer[8];
  char lower;
  char *allocated;
  CMPBool flag;
  cmp_u32 u32_value;
  cmp_u32 days;
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

  max_size = (cmp_usize) ~(cmp_usize)0;

  CMP_TEST_EXPECT(cmp_i18n_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_locale_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag(NULL, &locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("en-US", NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_from_tag("EN_us", &locale));
  CMP_TEST_ASSERT(locale.date_order == CMP_I18N_DATE_ORDER_MDY);
  CMP_TEST_EXPECT(cmp_i18n_locale_from_tag("zz-ZZ", &locale),
                  CMP_ERR_NOT_FOUND);

  CMP_TEST_EXPECT(cmp_i18n_test_mul_overflow(1, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_add_overflow(1, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_pow10(1, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_is_leap_year(2000, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_days_in_month(2020, 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen(NULL, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen("x", NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_ascii_lower('A', NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals(NULL, "en", &flag),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals("en", NULL, &flag),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_tag_equals("en", "en", NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_trim_span(NULL, 0, &start, &span_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(NULL, "x", 1, &allocated),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_alloc_string(&default_alloc, "x", 1, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_test_alloc_string(&default_alloc, NULL, 1, &allocated),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(NULL, 4, &out_len, 1, 1),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(buffer, 4, NULL, 1, 1),
                  CMP_ERR_INVALID_ARGUMENT);

  number.integer = 0;
  number.fraction = 0;
  number.fraction_digits = 0u;
  date.year = 2024;
  date.month = 1;
  date.day = 1;
  time.hour = 0u;
  time.minute = 0u;
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_fail, "en-US", NULL),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_set_formatter(&i18n_fail, NULL), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "k", 1, "v", 1, CMP_TRUE),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_get(&i18n_fail, "k", 1, &value, &value_len),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_contains(&i18n_fail, "k", 1, &flag), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_remove(&i18n_fail, "k", 1), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_clear(&i18n_fail), CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_count(&i18n_fail, &count), CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_i18n_load_table(&i18n_fail, &io, "path", CMP_FALSE, CMP_TRUE),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_i18n_load_table_buffer(&i18n_fail, "x", 1, CMP_FALSE, CMP_TRUE),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(cmp_i18n_format_number(&i18n_fail, &number, buffer,
                                         sizeof(buffer), &out_len),
                  CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_i18n_format_date(&i18n_fail, &date, buffer, sizeof(buffer), &out_len),
      CMP_ERR_STATE);
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(&i18n_fail, &time, buffer, sizeof(buffer), &out_len),
      CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_i18n_test_mul_overflow(max_size, 2, &out_len),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_mul_overflow(2, 3, &out_len));
  CMP_TEST_ASSERT(out_len == 6u);
  CMP_TEST_EXPECT(cmp_i18n_test_add_overflow(max_size, 1, &out_len),
                  CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_add_overflow(2, 3, &out_len));
  CMP_TEST_ASSERT(out_len == 5u);

  CMP_TEST_EXPECT(cmp_i18n_test_pow10(10u, &u32_value), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_pow10(0u, &u32_value));
  CMP_TEST_ASSERT(u32_value == 1u);

  CMP_TEST_OK(cmp_i18n_test_is_leap_year(2000, &flag));
  CMP_TEST_ASSERT(flag == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_test_is_leap_year(1900, &flag));
  CMP_TEST_ASSERT(flag == CMP_FALSE);
  CMP_TEST_OK(cmp_i18n_test_is_leap_year(2004, &flag));
  CMP_TEST_ASSERT(flag == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_test_is_leap_year(2001, &flag));
  CMP_TEST_ASSERT(flag == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_i18n_test_days_in_month(2020, 0, &days), CMP_ERR_RANGE);
  CMP_TEST_EXPECT(cmp_i18n_test_days_in_month(2020, 13, &days), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_i18n_test_days_in_month(2020, 1, &days));
  CMP_TEST_ASSERT(days == 31u);
  CMP_TEST_OK(cmp_i18n_test_days_in_month(2020, 4, &days));
  CMP_TEST_ASSERT(days == 30u);
  CMP_TEST_OK(cmp_i18n_test_days_in_month(2000, 2, &days));
  CMP_TEST_ASSERT(days == 29u);
  CMP_TEST_OK(cmp_i18n_test_days_in_month(2001, 2, &days));
  CMP_TEST_ASSERT(days == 28u);

  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.decimal_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.date_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.time_separator = '\0';
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.thousands_separator = '\0';
  locale.grouping = 3u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.grouping = 10u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale), CMP_ERR_RANGE);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.date_order = 99u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.time_format = 99u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  locale.pad_day = 2;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_locale(&locale),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_locale_init(&locale));
  CMP_TEST_OK(cmp_i18n_test_validate_locale(&locale));

  number.integer = 1;
  number.fraction = 0;
  number.fraction_digits = CMP_I18N_MAX_FRACTION_DIGITS + 1u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.integer = 1;
  number.fraction = 1;
  number.fraction_digits = 0u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.integer = 1;
  number.fraction = 100u;
  number.fraction_digits = 2u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_number(&number), CMP_ERR_RANGE);
  number.integer = 1;
  number.fraction = 5u;
  number.fraction_digits = 2u;
  CMP_TEST_OK(cmp_i18n_test_validate_number(&number));

  date.year = -1;
  date.month = 1;
  date.day = 1;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = CMP_DATE_MAX_YEAR + 1;
  date.month = 1;
  date.day = 1;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = 2024;
  date.month = 13;
  date.day = 1;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = 2024;
  date.month = 4;
  date.day = 31;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_date(&date), CMP_ERR_RANGE);
  date.year = 2024;
  date.month = 3;
  date.day = 4;
  CMP_TEST_OK(cmp_i18n_test_validate_date(&date));

  time.hour = 24u;
  time.minute = 0u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(&time), CMP_ERR_RANGE);
  time.hour = 0u;
  time.minute = 60u;
  CMP_TEST_EXPECT(cmp_i18n_test_validate_time(&time), CMP_ERR_RANGE);
  time.hour = 9u;
  time.minute = 5u;
  CMP_TEST_OK(cmp_i18n_test_validate_time(&time));

  CMP_TEST_OK(cmp_i18n_test_ascii_lower('A', &lower));
  CMP_TEST_ASSERT(lower == 'a');
  CMP_TEST_OK(cmp_i18n_test_ascii_lower('z', &lower));
  CMP_TEST_ASSERT(lower == 'z');

  CMP_TEST_OK(cmp_i18n_test_tag_equals("en_US", "EN-us", &flag));
  CMP_TEST_ASSERT(flag == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_test_tag_equals("en", "en-us", &flag));
  CMP_TEST_ASSERT(flag == CMP_FALSE);

  CMP_TEST_OK(cmp_i18n_test_trim_span("  x  ", 5, &start, &span_len));
  CMP_TEST_ASSERT(start == 2u);
  CMP_TEST_ASSERT(span_len == 1u);

  CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
  CMP_TEST_OK(cmp_i18n_test_alloc_string(&default_alloc, "hi", 2, &allocated));
  CMP_TEST_ASSERT(strcmp(allocated, "hi") == 0);
  CMP_TEST_OK(default_alloc.free(default_alloc.ctx, allocated));

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = test_realloc_fn;
  test_allocator.free = test_free_fn;
  alloc_state.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(
      cmp_i18n_test_alloc_string(&test_allocator, "x", 1, &allocated),
      CMP_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 0;

  out_len = 0;
  CMP_TEST_EXPECT(cmp_i18n_test_write_uint(small_buffer, 2, &out_len, 999u, 3u),
                  CMP_ERR_RANGE);
  out_len = 0;
  CMP_TEST_OK(
      cmp_i18n_test_write_uint(buffer, sizeof(buffer), &out_len, 42u, 4u));
  CMP_TEST_ASSERT(strcmp(buffer, "0042") == 0);

  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen(NULL, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(1));
  CMP_TEST_EXPECT(cmp_i18n_test_cstrlen("ab", &out_len), CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(0));

  CMP_TEST_OK(cmp_i18n_config_init(&config));
  CMP_TEST_ASSERT(config.entry_capacity ==
                  (cmp_usize)CMP_I18N_DEFAULT_CAPACITY);
  CMP_TEST_ASSERT(config.locale_tag != NULL);

#ifdef CMP_TESTING
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, NULL), CMP_ERR_UNKNOWN);
  CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));
#endif

  CMP_TEST_EXPECT(cmp_i18n_init(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.entry_capacity = 0;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, &config), CMP_ERR_INVALID_ARGUMENT);

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = NULL;
  test_allocator.free = test_free_fn;
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, &config), CMP_ERR_INVALID_ARGUMENT);

  test_allocator.realloc = test_realloc_fn;
  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  formatter.vtable = NULL;
  formatter.ctx = NULL;
  config.formatter = &formatter;
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, &config), CMP_ERR_INVALID_ARGUMENT);

  config.formatter = NULL;
  config.locale_tag = "zz-ZZ";
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, &config), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 2u;
  CMP_TEST_OK(cmp_i18n_init(&i18n, &config));
  CMP_TEST_EXPECT(cmp_i18n_init(&i18n, &config), CMP_ERR_STATE);

  CMP_TEST_EXPECT(cmp_i18n_set_locale(NULL, "en-US", NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_set_formatter(NULL, NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_put(NULL, "k", 1, "v", 1, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_get(NULL, "k", 1, &value, &value_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_contains(NULL, "k", 1, &flag),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_remove(NULL, "k", 1), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_clear(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_count(NULL, &count), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_load_table(NULL, &io, "path", CMP_FALSE, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_load_table_buffer(NULL, "x", 1, CMP_FALSE, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_number(NULL, &number, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_date(NULL, &date, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(NULL, &time, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n, NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n, "", NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_i18n_set_formatter(&i18n, &formatter),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_locale_init(&custom_locale));
  custom_locale.decimal_separator = '.';
  custom_locale.thousands_separator = ',';
  custom_locale.grouping = 3u;
  custom_locale.date_separator = '-';
  custom_locale.date_order = CMP_I18N_DATE_ORDER_YMD;
  custom_locale.time_separator = ':';
  custom_locale.time_format = CMP_I18N_TIME_FORMAT_24H;
  custom_locale.pad_day = CMP_TRUE;
  custom_locale.pad_month = CMP_TRUE;
  custom_locale.pad_hour = CMP_TRUE;
  custom_locale.pad_minute = CMP_TRUE;
  custom_locale.am = "AM";
  custom_locale.pm = "PM";

  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n, "zz-ZZ", NULL), CMP_ERR_NOT_FOUND);

  formatter_vtable.format_number = test_formatter_number;
  formatter_vtable.format_date = test_formatter_date;
  formatter_vtable.format_time = test_formatter_time;
  formatter.vtable = &formatter_vtable;
  formatter.ctx = &formatter_state;

  CMP_TEST_OK(cmp_i18n_set_formatter(&i18n, &formatter));
  CMP_TEST_OK(cmp_i18n_set_formatter(&i18n, NULL));

  CMP_TEST_EXPECT(cmp_i18n_put(&i18n, NULL, 1, "v", 1, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n, "k", 0, "v", 1, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n, "k", 1, NULL, 1, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    char invalid_utf8[2];
    invalid_utf8[0] = (char)0xC0;
    invalid_utf8[1] = '\0';
    CMP_TEST_EXPECT(cmp_i18n_put(&i18n, invalid_utf8, 1, "v", 1, CMP_TRUE),
                    CMP_ERR_CORRUPT);
  }

  CMP_TEST_OK(cmp_i18n_put(&i18n, "greeting", 8, "hello", 5, CMP_FALSE));
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n, "greeting", 8, "hi", 2, CMP_FALSE),
                  CMP_ERR_BUSY);
  CMP_TEST_OK(cmp_i18n_put(&i18n, "greeting", 8, "hi", 2, CMP_TRUE));

  CMP_TEST_OK(cmp_i18n_get(&i18n, "greeting", 8, &value, &value_len));
  CMP_TEST_ASSERT(value_len == 2u);
  CMP_TEST_ASSERT(strncmp(value, "hi", 2) == 0);

  CMP_TEST_OK(cmp_i18n_contains(&i18n, "greeting", 8, &flag));
  CMP_TEST_ASSERT(flag == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_contains(&i18n, "missing", 7, &flag));
  CMP_TEST_ASSERT(flag == CMP_FALSE);

  CMP_TEST_EXPECT(cmp_i18n_get(&i18n, "missing", 7, &value, &value_len),
                  CMP_ERR_NOT_FOUND);
  CMP_TEST_ASSERT(value == NULL);
  CMP_TEST_ASSERT(value_len == 0u);

  CMP_TEST_OK(cmp_i18n_count(&i18n, &count));
  CMP_TEST_ASSERT(count == 1u);

  CMP_TEST_OK(cmp_i18n_remove(&i18n, "greeting", 8));
  CMP_TEST_EXPECT(cmp_i18n_remove(&i18n, "greeting", 8), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_i18n_put(&i18n, "empty", 5, NULL, 0, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_get(&i18n, "empty", 5, &value, &value_len));
  CMP_TEST_ASSERT(value_len == 0u);

  CMP_TEST_OK(cmp_i18n_clear(&i18n));
  CMP_TEST_OK(cmp_i18n_count(&i18n, &count));
  CMP_TEST_ASSERT(count == 0u);

  CMP_TEST_OK(cmp_i18n_put(&i18n, "one", 3, "1", 1, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_put(&i18n, "two", 3, "2", 1, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_test_find_entry(&i18n, "one", 3, &index, &flag));
  CMP_TEST_ASSERT(flag == CMP_TRUE);
  CMP_TEST_OK(cmp_i18n_test_find_entry(&i18n, "missing", 7, &index, &flag));
  CMP_TEST_ASSERT(flag == CMP_FALSE);
  CMP_TEST_EXPECT(cmp_i18n_test_find_entry(&i18n, "one", 3, NULL, &flag),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_test_find_entry(&i18n, "one", 3, &index, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_clear(&i18n));

  rc = cmp_i18n_test_grow(&i18n, i18n.entry_capacity + 1u);
  CMP_TEST_OK(rc);

  {
    CMPI18n grow_i18n;
    CMPI18nEntry *entries;

    memset(&grow_i18n, 0, sizeof(grow_i18n));
    test_alloc_reset(&alloc_state2);
    test_allocator.ctx = &alloc_state2;
    test_allocator.alloc = test_alloc_fn;
    test_allocator.realloc = test_realloc_fn;
    test_allocator.free = test_free_fn;

    CMP_TEST_OK(test_allocator.alloc(test_allocator.ctx,
                                     (cmp_usize)sizeof(CMPI18nEntry),
                                     (void **)&entries));
    memset(entries, 0, sizeof(CMPI18nEntry));
    grow_i18n.entries = entries;
    grow_i18n.entry_capacity = 1u;
    grow_i18n.allocator = test_allocator;

    alloc_state2.fail_realloc_on_call = 1;
    CMP_TEST_EXPECT(cmp_i18n_test_grow(&grow_i18n, 2u), CMP_ERR_OUT_OF_MEMORY);
    alloc_state2.fail_realloc_on_call = 0;
    CMP_TEST_OK(cmp_i18n_test_grow(&grow_i18n, 2u));
    CMP_TEST_ASSERT(grow_i18n.entry_capacity >= 2u);

    CMP_TEST_OK(test_allocator.free(test_allocator.ctx, grow_i18n.entries));
  }

  {
    CMPI18n grow_i18n;

    memset(&grow_i18n, 0, sizeof(grow_i18n));
    grow_i18n.entries = (CMPI18nEntry *)buffer;
    grow_i18n.entry_capacity = 1u;
    grow_i18n.allocator.realloc = NULL;
    CMP_TEST_EXPECT(cmp_i18n_test_grow(&grow_i18n, 2u),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  {
    const char *table_data = "# comment\n"
                             " greeting = hello \n"
                             "empty=\n"
                             "; more comment\n"
                             "farewell=bye\n";
    CMP_TEST_OK(cmp_i18n_load_table_buffer(
        &i18n, table_data, (cmp_usize)strlen(table_data), CMP_TRUE, CMP_TRUE));
    CMP_TEST_OK(cmp_i18n_get(&i18n, "greeting", 8, &value, &value_len));
    CMP_TEST_ASSERT(strncmp(value, "hello", value_len) == 0);
    CMP_TEST_OK(cmp_i18n_get(&i18n, "empty", 5, &value, &value_len));
    CMP_TEST_ASSERT(value_len == 0u);
  }

  {
    const char *bad_table = "badline\n";
    CMP_TEST_EXPECT(cmp_i18n_load_table_buffer(&i18n, bad_table,
                                               (cmp_usize)strlen(bad_table),
                                               CMP_FALSE, CMP_TRUE),
                    CMP_ERR_CORRUPT);
  }

  {
    const char *bad_table = "=value\n";
    CMP_TEST_EXPECT(cmp_i18n_load_table_buffer(&i18n, bad_table,
                                               (cmp_usize)strlen(bad_table),
                                               CMP_FALSE, CMP_TRUE),
                    CMP_ERR_CORRUPT);
  }

  {
    char bom_table[] = "\xEF\xBB\xBFkey=value\n";
    CMP_TEST_OK(cmp_i18n_clear(&i18n));
    CMP_TEST_OK(cmp_i18n_load_table_buffer(
        &i18n, bom_table, (cmp_usize)strlen(bom_table), CMP_FALSE, CMP_TRUE));
    CMP_TEST_OK(cmp_i18n_get(&i18n, "key", 3, &value, &value_len));
    CMP_TEST_ASSERT(strncmp(value, "value", value_len) == 0);
  }

  CMP_TEST_EXPECT(
      cmp_i18n_load_table_buffer(&i18n, "x", 1, (CMPBool)2, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_load_table_buffer(&i18n, "x", 1, CMP_FALSE, (CMPBool)2),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_load_table_buffer(&i18n, NULL, 1, CMP_FALSE, CMP_TRUE),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_i18n_load_table_buffer(&i18n, NULL, 0, CMP_FALSE, CMP_FALSE));

  io.vtable = NULL;
  CMP_TEST_EXPECT(cmp_i18n_load_table(&i18n, &io, "path", CMP_FALSE, CMP_TRUE),
                  CMP_ERR_UNSUPPORTED);

  {
    static const CMPIOVTable io_vtable = {
        NULL, test_io_read_file_alloc, NULL, NULL, NULL, NULL};
    io.vtable = &io_vtable;
    io_state.data = "key=value\n";
    io_state.size = (cmp_usize)strlen(io_state.data);
    io_state.fail = 0;
    io.ctx = &io_state;
    CMP_TEST_OK(cmp_i18n_load_table(&i18n, &io, "path", CMP_TRUE, CMP_TRUE));
    CMP_TEST_OK(cmp_i18n_get(&i18n, "key", 3, &value, &value_len));
    CMP_TEST_ASSERT(strncmp(value, "value", value_len) == 0);

    io_state.fail = 1;
    CMP_TEST_EXPECT(
        cmp_i18n_load_table(&i18n, &io, "path", CMP_FALSE, CMP_TRUE),
        CMP_ERR_IO);
    io_state.fail = 0;
  }

  formatter_state.mode = 1;
  formatter.vtable = &formatter_vtable;
  formatter.ctx = &formatter_state;
  CMP_TEST_OK(cmp_i18n_set_formatter(&i18n, &formatter));

  number.integer = 123;
  number.fraction = 0;
  number.fraction_digits = 0u;
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "FMT-N") == 0);

  formatter_state.mode = 0;
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "123") == 0);

  formatter_state.mode = 2;
  CMP_TEST_EXPECT(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len),
      CMP_ERR_IO);

  formatter_state.mode = 0;
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  CMP_TEST_OK(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));

  CMP_TEST_OK(cmp_i18n_set_formatter(&i18n, NULL));

  number.integer = -12345;
  number.fraction = 67u;
  number.fraction_digits = 2u;
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "-12,345.67") == 0);

  number.integer = 1;
  number.fraction = 1u;
  number.fraction_digits = 0u;
  CMP_TEST_EXPECT(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len),
      CMP_ERR_RANGE);

  number.integer = 1;
  number.fraction = 0u;
  number.fraction_digits = 0u;
  CMP_TEST_EXPECT(cmp_i18n_format_number(&i18n, &number, NULL, 1, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_format_number(&i18n, &number, buffer, 1, &out_len),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_i18n_format_number(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  date.year = 2024;
  date.month = 3;
  date.day = 4;
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "2024-03-04") == 0);

  custom_locale = i18n.locale;
  custom_locale.date_order = CMP_I18N_DATE_ORDER_MDY;
  custom_locale.date_separator = '/';
  custom_locale.pad_day = CMP_TRUE;
  custom_locale.pad_month = CMP_TRUE;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "03/04/2024") == 0);

  custom_locale.date_order = CMP_I18N_DATE_ORDER_DMY;
  custom_locale.pad_day = CMP_FALSE;
  custom_locale.pad_month = CMP_FALSE;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_OK(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "4/3/2024") == 0);

  date.month = 2;
  date.day = 30;
  CMP_TEST_EXPECT(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), &out_len),
      CMP_ERR_RANGE);
  date.month = 3;
  date.day = 4;

  CMP_TEST_EXPECT(cmp_i18n_format_date(&i18n, &date, buffer, 4, &out_len),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_i18n_format_date(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_format_date(&i18n, &date, NULL, 1, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_date(&i18n, &date, buffer, sizeof(buffer), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  time.hour = 0u;
  time.minute = 5u;
  custom_locale.time_format = CMP_I18N_TIME_FORMAT_12H;
  custom_locale.pad_hour = CMP_TRUE;
  custom_locale.pad_minute = CMP_TRUE;
  custom_locale.time_separator = ':';
  custom_locale.am = "AM";
  custom_locale.pm = "PM";
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_OK(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "12:05 AM") == 0);

  time.hour = 23u;
  time.minute = 7u;
  CMP_TEST_OK(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "11:07 PM") == 0);

  custom_locale.time_format = CMP_I18N_TIME_FORMAT_24H;
  custom_locale.pad_hour = CMP_FALSE;
  custom_locale.pad_minute = CMP_TRUE;
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  time.hour = 9u;
  time.minute = 5u;
  CMP_TEST_OK(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "9:05") == 0);

  time.hour = 24u;
  time.minute = 0u;
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len),
      CMP_ERR_RANGE);
  time.hour = 9u;
  time.minute = 5u;

  CMP_TEST_EXPECT(cmp_i18n_format_time(&i18n, &time, buffer, 4, &out_len),
                  CMP_ERR_RANGE);
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(&i18n, NULL, buffer, sizeof(buffer), &out_len),
      CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_i18n_format_time(&i18n, &time, NULL, 1, &out_len),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), NULL),
      CMP_ERR_INVALID_ARGUMENT);

  custom_locale.time_format = CMP_I18N_TIME_FORMAT_12H;
  custom_locale.am = "AM";
  custom_locale.pm = "PM";
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(1));
  time.hour = 1u;
  time.minute = 0u;
  CMP_TEST_EXPECT(
      cmp_i18n_format_time(&i18n, &time, buffer, sizeof(buffer), &out_len),
      CMP_ERR_OVERFLOW);
  CMP_TEST_OK(cmp_i18n_test_set_cstr_limit(max_size));

  number.integer = 1234;
  number.fraction = 0u;
  number.fraction_digits = 0u;
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "1,234") == 0);

  custom_locale.grouping = 0u;
  custom_locale.thousands_separator = '\0';
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-US", &custom_locale));
  CMP_TEST_OK(
      cmp_i18n_format_number(&i18n, &number, buffer, sizeof(buffer), &out_len));
  CMP_TEST_ASSERT(strcmp(buffer, "1234") == 0);

  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "en-GB", NULL));
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "fr-FR", NULL));
  CMP_TEST_OK(cmp_i18n_set_locale(&i18n, "de-DE", NULL));
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n, "zz-ZZ", NULL), CMP_ERR_NOT_FOUND);

  CMP_TEST_OK(cmp_i18n_shutdown(&i18n));

  {
    CMPI18n i18n_fmt;
    CMPI18nConfig fmt_config;
    CMPI18nFormatter formatter;
    CMPI18nFormatterVTable formatter_vtable;
    TestFormatter fmt_state;

    memset(&i18n_fmt, 0, sizeof(i18n_fmt));
    fmt_state.mode = 2;
    formatter_vtable.format_number = test_formatter_number;
    formatter_vtable.format_date = test_formatter_date;
    formatter_vtable.format_time = test_formatter_time;
    formatter.ctx = &fmt_state;
    formatter.vtable = &formatter_vtable;

    CMP_TEST_OK(cmp_i18n_config_init(&fmt_config));
    fmt_config.formatter = &formatter;
    CMP_TEST_OK(cmp_i18n_init(&i18n_fmt, &fmt_config));
    CMP_TEST_EXPECT(cmp_i18n_format_number(&i18n_fmt, &number, buffer,
                                           sizeof(buffer), &out_len),
                    CMP_ERR_IO);
    CMP_TEST_EXPECT(cmp_i18n_format_date(&i18n_fmt, &date, buffer,
                                         sizeof(buffer), &out_len),
                    CMP_ERR_IO);
    CMP_TEST_EXPECT(cmp_i18n_format_time(&i18n_fmt, &time, buffer,
                                         sizeof(buffer), &out_len),
                    CMP_ERR_IO);
    CMP_TEST_OK(cmp_i18n_shutdown(&i18n_fmt));

    formatter.vtable = NULL;
    CMP_TEST_OK(cmp_i18n_config_init(&fmt_config));
    fmt_config.formatter = &formatter;
    CMP_TEST_EXPECT(cmp_i18n_init(&i18n_fmt, &fmt_config),
                    CMP_ERR_INVALID_ARGUMENT);
  }

  test_alloc_reset(&alloc_state);
  test_allocator.ctx = &alloc_state;
  test_allocator.alloc = test_alloc_fn;
  test_allocator.realloc = test_realloc_fn;
  test_allocator.free = test_free_fn;

  CMP_TEST_OK(cmp_i18n_config_init(&config));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_fail, &config));
  CMP_TEST_OK(test_alloc_reset(&alloc_state));

  {
    CMPI18n i18n_bad;
    memset(&i18n_bad, 0, sizeof(i18n_bad));
    i18n_bad.entries = (CMPI18nEntry *)&i18n_bad;
    i18n_bad.allocator.free = NULL;
    CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_bad), CMP_ERR_INVALID_ARGUMENT);
  }

  CMP_TEST_EXPECT(cmp_i18n_test_validate_utf8(NULL, 1u),
                  CMP_ERR_INVALID_ARGUMENT);

  {
    CMPI18n i18n_overflow;
    CMPI18nEntry entries[1];
    cmp_usize max_size;
    cmp_usize min_capacity;

    max_size = (cmp_usize) ~(cmp_usize)0;
    memset(&i18n_overflow, 0, sizeof(i18n_overflow));
    i18n_overflow.entries = entries;
    i18n_overflow.allocator = test_allocator;

    i18n_overflow.entry_capacity = (max_size / 2u) + 1u;
    min_capacity = i18n_overflow.entry_capacity + 1u;
    CMP_TEST_EXPECT(cmp_i18n_test_grow(&i18n_overflow, min_capacity),
                    CMP_ERR_OVERFLOW);

    i18n_overflow.entry_capacity =
        (max_size / (cmp_usize)sizeof(CMPI18nEntry)) + 1u;
    CMP_TEST_EXPECT(
        cmp_i18n_test_grow(&i18n_overflow, i18n_overflow.entry_capacity),
        CMP_ERR_OVERFLOW);
  }

  {
    char *out_string = NULL;
    cmp_usize max_size;

    max_size = (cmp_usize) ~(cmp_usize)0;
    CMP_TEST_EXPECT(
        cmp_i18n_test_alloc_string(&test_allocator, "x", max_size, &out_string),
        CMP_ERR_OVERFLOW);
  }

  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_set_locale(&i18n_fail, "en-US", NULL), CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_i18n_test_force_utf8_ok_count(2u));
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "a", 1, NULL, 1, CMP_TRUE),
                  CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_i18n_put(&i18n_fail, "key", 3, "val", 3, CMP_TRUE));
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "key", 3, "new", 3, CMP_FALSE),
                  CMP_ERR_BUSY);
  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "key", 3, NULL, 0, CMP_TRUE),
                  CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;
  CMP_TEST_OK(cmp_i18n_put(&i18n_fail, "key", 3, NULL, 0, CMP_TRUE));
  CMP_TEST_OK(cmp_i18n_put(&i18n_fail, "key", 3, "val", 3, CMP_TRUE));
  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "key", 3, "new", 3, CMP_TRUE),
                  CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  CMP_TEST_OK(test_alloc_reset(&alloc_state));
  alloc_state.fail_alloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "a", 1, "b", 1, CMP_TRUE),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 2;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "a", 1, "b", 1, CMP_TRUE),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state.fail_alloc_on_call = 0;

  {
    CMPI18n i18n_update;
    CMPI18nConfig update_config;

    memset(&i18n_update, 0, sizeof(i18n_update));
    CMP_TEST_OK(cmp_i18n_config_init(&update_config));
    update_config.allocator = &test_allocator;
    update_config.entry_capacity = 2u;
    CMP_TEST_OK(cmp_i18n_init(&i18n_update, &update_config));
    CMP_TEST_OK(cmp_i18n_put(&i18n_update, "k", 1, "v", 1, CMP_TRUE));

    CMP_TEST_OK(test_alloc_reset(&alloc_state));
    alloc_state.fail_alloc_on_call = 1;
    CMP_TEST_EXPECT(cmp_i18n_put(&i18n_update, "k", 1, "n", 1, CMP_TRUE),
                    CMP_ERR_OUT_OF_MEMORY);
    alloc_state.fail_alloc_on_call = 0;

    CMP_TEST_OK(test_alloc_reset(&alloc_state));
    alloc_state.fail_alloc_on_call = 2;
    CMP_TEST_EXPECT(cmp_i18n_put(&i18n_update, "x", 1, "y", 1, CMP_TRUE),
                    CMP_ERR_OUT_OF_MEMORY);
    alloc_state.fail_alloc_on_call = 0;

    CMP_TEST_OK(cmp_i18n_shutdown(&i18n_update));
  }

  CMP_TEST_OK(cmp_i18n_put(&i18n_fail, "a", 1, "b", 1, CMP_TRUE));
  CMP_TEST_OK(test_alloc_reset(&alloc_state));
  alloc_state.fail_realloc_on_call = 1;
  CMP_TEST_EXPECT(cmp_i18n_put(&i18n_fail, "c", 1, "d", 1, CMP_TRUE),
                  CMP_ERR_OUT_OF_MEMORY);
  alloc_state.fail_realloc_on_call = 0;

  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_clear(&i18n_fail), CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  CMP_TEST_OK(cmp_i18n_clear(&i18n_fail));
  CMP_TEST_OK(cmp_i18n_put(&i18n_fail, "x", 1, "y", 1, CMP_TRUE));
  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_remove(&i18n_fail, "x", 1), CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
  CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_fail), CMP_ERR_IO);
  alloc_state.fail_free_on_call = 0;

  memset(&i18n_fail, 0, sizeof(i18n_fail));
  config.allocator = &test_allocator;
  config.entry_capacity = 1u;
  CMP_TEST_OK(cmp_i18n_init(&i18n_fail, &config));

  {
    static const CMPIOVTable io_vtable = {
        NULL, test_io_read_file_alloc, NULL, NULL, NULL, NULL};
    io.vtable = &io_vtable;
    io_state.data = "k=v\n";
    io_state.size = (cmp_usize)strlen(io_state.data);
    io_state.fail = 0;
    io.ctx = &io_state;

    alloc_state.fail_free_on_call = alloc_state.free_calls + 1;
    CMP_TEST_EXPECT(
        cmp_i18n_load_table(&i18n_fail, &io, "path", CMP_TRUE, CMP_TRUE),
        CMP_ERR_IO);
    alloc_state.fail_free_on_call = 0;
  }

  CMP_TEST_OK(cmp_i18n_shutdown(&i18n_fail));

  CMP_TEST_EXPECT(cmp_i18n_shutdown(&i18n_fail), CMP_ERR_STATE);

  if (test_i18n_coverage_hooks(&alloc_state, &test_allocator) != 0) {
    return 1;
  }
  if (test_i18n_branch_sweep(&alloc_state, &test_allocator) != 0) {
    return 1;
  }

  return 0;
}
