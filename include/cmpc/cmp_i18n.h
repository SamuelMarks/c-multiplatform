#ifndef CMP_I18N_H
#define CMP_I18N_H

/**
 * @file cmp_i18n.h
 * @brief Localization string tables and formatting helpers for LibCMPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_api_env.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_datetime.h"

/** @brief Default entry capacity for I18N tables. */
#define CMP_I18N_DEFAULT_CAPACITY 32u

/** @brief Date order: month/day/year. */
#define CMP_I18N_DATE_ORDER_MDY 0u
/** @brief Date order: day/month/year. */
#define CMP_I18N_DATE_ORDER_DMY 1u
/** @brief Date order: year/month/day. */
#define CMP_I18N_DATE_ORDER_YMD 2u

/** @brief 24-hour time format. */
#define CMP_I18N_TIME_FORMAT_24H 0u
/** @brief 12-hour time format. */
#define CMP_I18N_TIME_FORMAT_12H 1u

/** @brief Maximum supported fractional digits for number formatting. */
#define CMP_I18N_MAX_FRACTION_DIGITS 9u

/** @brief Default locale tag used by the I18N system. */
#define CMP_I18N_DEFAULT_LOCALE_TAG "en-US"

/**
 * @brief Locale formatting description.
 */
typedef struct CMPI18nLocale {
  char decimal_separator;   /**< Decimal separator character. */
  char thousands_separator; /**< Thousands grouping separator (0 disables). */
  cmp_u32 grouping;         /**< Digits per group (0 disables). */
  char date_separator;      /**< Date separator character. */
  cmp_u32 date_order;       /**< Date order (CMP_I18N_DATE_ORDER_*). */
  char time_separator;      /**< Time separator character. */
  cmp_u32 time_format;      /**< Time format (CMP_I18N_TIME_FORMAT_*). */
  CMPBool pad_day;          /**< CMP_TRUE to pad day to 2 digits. */
  CMPBool pad_month;        /**< CMP_TRUE to pad month to 2 digits. */
  CMPBool pad_hour;         /**< CMP_TRUE to pad hour to 2 digits. */
  CMPBool pad_minute;       /**< CMP_TRUE to pad minute to 2 digits. */
  const char *am;           /**< AM designator for 12h format (may be NULL). */
  const char *pm;           /**< PM designator for 12h format (may be NULL). */
} CMPI18nLocale;

/**
 * @brief Fixed-point number descriptor.
 */
typedef struct CMPI18nNumber {
  cmp_i32 integer;  /**< Integer component. */
  cmp_u32 fraction; /**< Fractional component (scaled by fraction_digits). */
  cmp_u32 fraction_digits; /**< Number of fractional digits. */
} CMPI18nNumber;

struct CMPI18nFormatterVTable;

/**
 * @brief Locale-aware formatter interface.
 */
typedef struct CMPI18nFormatter {
  void *ctx; /**< Formatter context pointer. */
  const struct CMPI18nFormatterVTable *vtable; /**< Formatter virtual table. */
} CMPI18nFormatter;

/**
 * @brief Formatter virtual table.
 */
typedef struct CMPI18nFormatterVTable {
/**
 * @brief Format a number into a caller-provided buffer.
 * @param ctx Formatter context pointer.
 * @param locale_tag Locale tag string.
 * @param locale Locale formatting description.
 * @param number Number descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_UNSUPPORTED to fall back, or a failure
 * code.
 */
#ifdef CMP_DOXYGEN
  int format_number(void *ctx, const char *locale_tag,
                    const CMPI18nLocale *locale, const CMPI18nNumber *number,
                    char *out_text, cmp_usize text_capacity,
                    cmp_usize *out_len);
#else
  int(CMP_CALL *format_number)(void *ctx, const char *locale_tag,
                               const CMPI18nLocale *locale,
                               const CMPI18nNumber *number, char *out_text,
                               cmp_usize text_capacity, cmp_usize *out_len);
#endif
/**
 * @brief Format a date into a caller-provided buffer.
 * @param ctx Formatter context pointer.
 * @param locale_tag Locale tag string.
 * @param locale Locale formatting description.
 * @param date Date descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_UNSUPPORTED to fall back, or a failure
 * code.
 */
#ifdef CMP_DOXYGEN
  int format_date(void *ctx, const char *locale_tag,
                  const CMPI18nLocale *locale, const struct CMPDate *date,
                  char *out_text, cmp_usize text_capacity, cmp_usize *out_len);
#else
  int(CMP_CALL *format_date)(void *ctx, const char *locale_tag,
                             const CMPI18nLocale *locale,
                             const struct CMPDate *date, char *out_text,
                             cmp_usize text_capacity, cmp_usize *out_len);
#endif
/**
 * @brief Format a time into a caller-provided buffer.
 * @param ctx Formatter context pointer.
 * @param locale_tag Locale tag string.
 * @param locale Locale formatting description.
 * @param time Time descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_UNSUPPORTED to fall back, or a failure
 * code.
 */
#ifdef CMP_DOXYGEN
  int format_time(void *ctx, const char *locale_tag,
                  const CMPI18nLocale *locale, const struct CMPTime *time,
                  char *out_text, cmp_usize text_capacity, cmp_usize *out_len);
#else
  int(CMP_CALL *format_time)(void *ctx, const char *locale_tag,
                             const CMPI18nLocale *locale,
                             const struct CMPTime *time, char *out_text,
                             cmp_usize text_capacity, cmp_usize *out_len);
#endif
} CMPI18nFormatterVTable;

/**
 * @brief I18N configuration.
 */
typedef struct CMPI18nConfig {
  const CMPAllocator *allocator; /**< Allocator for I18N memory; NULL uses
                                   default allocator. */
  cmp_usize entry_capacity;      /**< Initial entry capacity (> 0). */
  const char *locale_tag;        /**< Locale tag string (NULL uses default). */
  const CMPI18nLocale *locale;   /**< Locale settings override (NULL uses
                                   locale_tag preset). */
  const CMPI18nFormatter *formatter; /**< Optional formatter override. */
} CMPI18nConfig;

/**
 * @brief I18N string entry.
 */
typedef struct CMPI18nEntry {
  char *key;         /**< UTF-8 key bytes (null-terminated copy). */
  cmp_usize key_len; /**< Key length in bytes (excluding null terminator). */
  char *value;       /**< UTF-8 value bytes (null-terminated copy). */
  cmp_usize
      value_len; /**< Value length in bytes (excluding null terminator). */
} CMPI18nEntry;

/**
 * @brief I18N instance.
 */
typedef struct CMPI18n {
  CMPAllocator allocator;     /**< Allocator used for I18N memory. */
  CMPI18nEntry *entries;      /**< Entry array (capacity-sized). */
  cmp_usize entry_count;      /**< Number of active entries. */
  cmp_usize entry_capacity;   /**< Allocated entry capacity. */
  char *locale_tag;           /**< Locale tag string (null-terminated). */
  cmp_usize locale_tag_len;   /**< Locale tag length in bytes. */
  CMPI18nLocale locale;       /**< Locale formatting description. */
  CMPI18nFormatter formatter; /**< Optional formatter override. */
} CMPI18n;

/**
 * @brief Initialize a locale with default settings.
 * @param locale Locale descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_locale_init(CMPI18nLocale *locale);

/**
 * @brief Initialize a locale from a preset locale tag.
 * @param locale_tag Locale tag string.
 * @param out_locale Receives the locale description.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if unknown, or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_locale_from_tag(const char *locale_tag,
                                              CMPI18nLocale *out_locale);

/**
 * @brief Initialize an I18N configuration with defaults.
 * @param config Configuration to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_config_init(CMPI18nConfig *config);

/**
 * @brief Initialize an I18N instance.
 * @param i18n Instance to initialize.
 * @param config Configuration (NULL uses defaults).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_init(CMPI18n *i18n, const CMPI18nConfig *config);

/**
 * @brief Shut down an I18N instance and release resources.
 * @param i18n Instance to shut down.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_shutdown(CMPI18n *i18n);

/**
 * @brief Update the locale settings for an I18N instance.
 * @param i18n Instance to update.
 * @param locale_tag Locale tag string.
 * @param locale Locale override (NULL uses locale_tag preset).
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if the preset is unknown, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_i18n_set_locale(CMPI18n *i18n, const char *locale_tag,
                                         const CMPI18nLocale *locale);

/**
 * @brief Update the formatter override for an I18N instance.
 * @param i18n Instance to update.
 * @param formatter Formatter override (NULL clears it).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_set_formatter(CMPI18n *i18n,
                                            const CMPI18nFormatter *formatter);

/**
 * @brief Insert or update a localized string.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param utf8_value UTF-8 value bytes (may be NULL when value_len is 0).
 * @param value_len Value length in bytes.
 * @param overwrite CMP_TRUE to overwrite existing values.
 * @return CMP_OK on success, CMP_ERR_BUSY if the key exists and overwrite is
 * false, or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_put(CMPI18n *i18n, const char *utf8_key,
                                  cmp_usize key_len, const char *utf8_value,
                                  cmp_usize value_len, CMPBool overwrite);

/**
 * @brief Retrieve a localized string pointer.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_value Receives the value pointer.
 * @param out_value_len Receives the value length in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if missing, or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_get(const CMPI18n *i18n, const char *utf8_key,
                                  cmp_usize key_len, const char **out_value,
                                  cmp_usize *out_value_len);

/**
 * @brief Check whether a key exists in the I18N table.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_exists Receives CMP_TRUE if the key exists.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_contains(const CMPI18n *i18n,
                                       const char *utf8_key, cmp_usize key_len,
                                       CMPBool *out_exists);

/**
 * @brief Remove a localized string.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @return CMP_OK on success, CMP_ERR_NOT_FOUND if missing, or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_remove(CMPI18n *i18n, const char *utf8_key,
                                     cmp_usize key_len);

/**
 * @brief Remove all localized strings.
 * @param i18n I18N instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_clear(CMPI18n *i18n);

/**
 * @brief Get the number of localized strings.
 * @param i18n I18N instance.
 * @param out_count Receives the entry count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_count(const CMPI18n *i18n, cmp_usize *out_count);

/**
 * @brief Load a localization table from a file.
 * @param i18n I18N instance.
 * @param io IO backend.
 * @param utf8_path Input file path in UTF-8.
 * @param clear_existing CMP_TRUE to clear existing entries first.
 * @param overwrite CMP_TRUE to overwrite existing keys.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_load_table(CMPI18n *i18n, CMPIO *io,
                                         const char *utf8_path,
                                         CMPBool clear_existing,
                                         CMPBool overwrite);

/**
 * @brief Load a localization table from memory.
 * @param i18n I18N instance.
 * @param data Table buffer (may be NULL when size is 0).
 * @param size Table size in bytes.
 * @param clear_existing CMP_TRUE to clear existing entries first.
 * @param overwrite CMP_TRUE to overwrite existing keys.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_load_table_buffer(CMPI18n *i18n, const char *data,
                                                cmp_usize size,
                                                CMPBool clear_existing,
                                                CMPBool overwrite);

/**
 * @brief Format a number using locale settings.
 * @param i18n I18N instance.
 * @param number Number descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_i18n_format_number(const CMPI18n *i18n,
                                            const CMPI18nNumber *number,
                                            char *out_text,
                                            cmp_usize text_capacity,
                                            cmp_usize *out_len);

/**
 * @brief Format a date using locale settings.
 * @param i18n I18N instance.
 * @param date Date descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_i18n_format_date(const CMPI18n *i18n,
                                          const struct CMPDate *date,
                                          char *out_text,
                                          cmp_usize text_capacity,
                                          cmp_usize *out_len);

/**
 * @brief Format a time using locale settings.
 * @param i18n I18N instance.
 * @param time Time descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return CMP_OK on success, CMP_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
CMP_API int CMP_CALL cmp_i18n_format_time(const CMPI18n *i18n,
                                          const struct CMPTime *time,
                                          char *out_text,
                                          cmp_usize text_capacity,
                                          cmp_usize *out_len);

#ifdef CMP_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_mul_overflow(cmp_usize a, cmp_usize b,
                                                cmp_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_add_overflow(cmp_usize a, cmp_usize b,
                                                cmp_usize *out_value);

/**
 * @brief Test wrapper for base-10 exponent helper.
 * @param digits Exponent value.
 * @param out_value Receives the computed power.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_pow10(cmp_u32 digits, cmp_u32 *out_value);

/**
 * @brief Test wrapper for leap year detection.
 * @param year Year to evaluate.
 * @param out_leap Receives CMP_TRUE if leap year.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_is_leap_year(cmp_i32 year,
                                                CMPBool *out_leap);

/**
 * @brief Test wrapper for days-in-month helper.
 * @param year Year component.
 * @param month Month component.
 * @param out_days Receives days in month.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_days_in_month(cmp_i32 year, cmp_u32 month,
                                                 cmp_u32 *out_days);

/**
 * @brief Test wrapper for locale validation.
 * @param locale Locale descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_validate_locale(const CMPI18nLocale *locale);

/**
 * @brief Test wrapper for number validation.
 * @param number Number descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_validate_number(const CMPI18nNumber *number);

/**
 * @brief Test wrapper for date validation.
 * @param date Date descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_validate_date(const struct CMPDate *date);

/**
 * @brief Test wrapper for time validation.
 * @param time Time descriptor.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_validate_time(const struct CMPTime *time);

/**
 * @brief Test wrapper for UTF-8 validation.
 * @param data UTF-8 data pointer (may be NULL when length is 0).
 * @param length Data length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_validate_utf8(const char *data,
                                                 cmp_usize length);

/**
 * @brief Test wrapper for C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives string length in bytes.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_cstrlen(const char *cstr,
                                           cmp_usize *out_len);

/**
 * @brief Test wrapper for ASCII lowercase conversion.
 * @param ch Input character.
 * @param out_lower Receives lowercase value.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_ascii_lower(char ch, char *out_lower);

/**
 * @brief Test wrapper for locale tag comparison.
 * @param a First tag.
 * @param b Second tag.
 * @param out_equal Receives CMP_TRUE when equal.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_tag_equals(const char *a, const char *b,
                                              CMPBool *out_equal);

/**
 * @brief Test wrapper for trimming span helper.
 * @param data Input buffer.
 * @param length Input length.
 * @param out_start Receives trimmed start index.
 * @param out_length Receives trimmed length.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_trim_span(const char *data, cmp_usize length,
                                             cmp_usize *out_start,
                                             cmp_usize *out_length);

/**
 * @brief Test wrapper for string allocation helper.
 * @param allocator Allocator interface.
 * @param src Source bytes.
 * @param len Source length in bytes.
 * @param out_str Receives allocated string.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_alloc_string(const CMPAllocator *allocator,
                                                const char *src, cmp_usize len,
                                                char **out_str);

/**
 * @brief Test wrapper for unsigned integer formatting helper.
 * @param out_text Destination buffer.
 * @param text_capacity Destination buffer size.
 * @param io_offset In/out offset into the buffer.
 * @param value Value to format.
 * @param min_digits Minimum digit count.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_write_uint(char *out_text,
                                              cmp_usize text_capacity,
                                              cmp_usize *io_offset,
                                              cmp_u32 value,
                                              cmp_u32 min_digits);

/**
 * @brief Test wrapper for entry lookup helper.
 * @param i18n I18N instance.
 * @param key Key bytes.
 * @param key_len Key length in bytes.
 * @param out_index Receives entry index.
 * @param out_found Receives CMP_TRUE when found.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_find_entry(const CMPI18n *i18n,
                                              const char *key,
                                              cmp_usize key_len,
                                              cmp_usize *out_index,
                                              CMPBool *out_found);

/**
 * @brief Test wrapper for entry growth helper.
 * @param i18n I18N instance.
 * @param min_capacity Minimum entry capacity.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_grow(CMPI18n *i18n, cmp_usize min_capacity);

/**
 * @brief Test helper for setting the maximum C-string length.
 * @param max_len Maximum allowed length (0 resets to default).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_set_cstr_limit(cmp_usize max_len);

/**
 * @brief Force the pow10 helper to return an error once (testing only).
 * @param enable CMP_TRUE to force an error on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_pow10_error(CMPBool enable);

/**
 * @brief Force leap-year detection to return an error once (testing only).
 * @param enable CMP_TRUE to force an error on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_leap_error(CMPBool enable);

/**
 * @brief Force the days-in-month helper to hit the default branch once.
 * @param enable CMP_TRUE to force the default path on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_days_default(CMPBool enable);

/**
 * @brief Fail the ASCII lowercase helper after a number of calls.
 * @param call_count Number of calls before failing (0 to disable).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_i18n_test_set_ascii_lower_fail_after(cmp_u32 call_count);

/**
 * @brief Force UTF-8 validation to return an error once (testing only).
 * @param enable CMP_TRUE to force an error on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_utf8_error(CMPBool enable);

/**
 * @brief Force UTF-8 validation to return success once (testing only).
 * @param enable CMP_TRUE to force success on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_utf8_ok(CMPBool enable);

/**
 * @brief Force UTF-8 validation to return success for a number of calls.
 * @param count Number of calls to force success for (0 to disable).
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_utf8_ok_count(cmp_u32 count);

/**
 * @brief Force config initialization to return an error once (testing only).
 * @param enable CMP_TRUE to force an error on the next call.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_force_config_init_error(CMPBool enable);

/**
 * @brief Test wrapper for en-US locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_i18n_test_locale_preset_en_us(CMPI18nLocale *out_locale);

/**
 * @brief Test wrapper for en-GB locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_i18n_test_locale_preset_en_gb(CMPI18nLocale *out_locale);

/**
 * @brief Test wrapper for fr-FR locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_i18n_test_locale_preset_fr_fr(CMPI18nLocale *out_locale);

/**
 * @brief Test wrapper for de-DE locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_i18n_test_locale_preset_de_de(CMPI18nLocale *out_locale);

/**
 * @brief Test wrapper for parsing translation tables.
 * @param i18n I18N instance.
 * @param data Table text data.
 * @param size Size of the text buffer.
 * @param clear_existing CMP_TRUE to clear existing entries.
 * @param overwrite CMP_TRUE to overwrite existing keys.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_i18n_test_parse_table(CMPI18n *i18n, const char *data,
                                               cmp_usize size,
                                               CMPBool clear_existing,
                                               CMPBool overwrite);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_I18N_H */
