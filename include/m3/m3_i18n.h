#ifndef M3_I18N_H
#define M3_I18N_H

/**
 * @file m3_i18n.h
 * @brief Localization string tables and formatting helpers for LibM3C.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "m3_api_env.h"
#include "m3_core.h"

/** @brief Default entry capacity for I18N tables. */
#define M3_I18N_DEFAULT_CAPACITY 32u

/** @brief Date order: month/day/year. */
#define M3_I18N_DATE_ORDER_MDY 0u
/** @brief Date order: day/month/year. */
#define M3_I18N_DATE_ORDER_DMY 1u
/** @brief Date order: year/month/day. */
#define M3_I18N_DATE_ORDER_YMD 2u

/** @brief 24-hour time format. */
#define M3_I18N_TIME_FORMAT_24H 0u
/** @brief 12-hour time format. */
#define M3_I18N_TIME_FORMAT_12H 1u

/** @brief Maximum supported fractional digits for number formatting. */
#define M3_I18N_MAX_FRACTION_DIGITS 9u

/** @brief Default locale tag used by the I18N system. */
#define M3_I18N_DEFAULT_LOCALE_TAG "en-US"

struct M3Date;
struct M3Time;

/**
 * @brief Locale formatting description.
 */
typedef struct M3I18nLocale {
  char decimal_separator;   /**< Decimal separator character. */
  char thousands_separator; /**< Thousands grouping separator (0 disables). */
  m3_u32 grouping;          /**< Digits per group (0 disables). */
  char date_separator;      /**< Date separator character. */
  m3_u32 date_order;        /**< Date order (M3_I18N_DATE_ORDER_*). */
  char time_separator;      /**< Time separator character. */
  m3_u32 time_format;       /**< Time format (M3_I18N_TIME_FORMAT_*). */
  M3Bool pad_day;           /**< M3_TRUE to pad day to 2 digits. */
  M3Bool pad_month;         /**< M3_TRUE to pad month to 2 digits. */
  M3Bool pad_hour;          /**< M3_TRUE to pad hour to 2 digits. */
  M3Bool pad_minute;        /**< M3_TRUE to pad minute to 2 digits. */
  const char *am;           /**< AM designator for 12h format (may be NULL). */
  const char *pm;           /**< PM designator for 12h format (may be NULL). */
} M3I18nLocale;

/**
 * @brief Fixed-point number descriptor.
 */
typedef struct M3I18nNumber {
  m3_i32 integer;  /**< Integer component. */
  m3_u32 fraction; /**< Fractional component (scaled by fraction_digits). */
  m3_u32 fraction_digits; /**< Number of fractional digits. */
} M3I18nNumber;

struct M3I18nFormatterVTable;

/**
 * @brief Locale-aware formatter interface.
 */
typedef struct M3I18nFormatter {
  void *ctx;                                  /**< Formatter context pointer. */
  const struct M3I18nFormatterVTable *vtable; /**< Formatter virtual table. */
} M3I18nFormatter;

/**
 * @brief Formatter virtual table.
 */
typedef struct M3I18nFormatterVTable {
  /**
   * @brief Format a number into a caller-provided buffer.
   * @param ctx Formatter context pointer.
   * @param locale_tag Locale tag string.
   * @param locale Locale formatting description.
   * @param number Number descriptor.
   * @param out_text Destination buffer.
   * @param text_capacity Size of the destination buffer.
   * @param out_len Receives number of bytes written (excluding null).
   * @return M3_OK on success, M3_ERR_UNSUPPORTED to fall back, or a failure
   * code.
   */
  int(M3_CALL *format_number)(void *ctx, const char *locale_tag,
                              const M3I18nLocale *locale,
                              const M3I18nNumber *number, char *out_text,
                              m3_usize text_capacity, m3_usize *out_len);
  /**
   * @brief Format a date into a caller-provided buffer.
   * @param ctx Formatter context pointer.
   * @param locale_tag Locale tag string.
   * @param locale Locale formatting description.
   * @param date Date descriptor.
   * @param out_text Destination buffer.
   * @param text_capacity Size of the destination buffer.
   * @param out_len Receives number of bytes written (excluding null).
   * @return M3_OK on success, M3_ERR_UNSUPPORTED to fall back, or a failure
   * code.
   */
  int(M3_CALL *format_date)(void *ctx, const char *locale_tag,
                            const M3I18nLocale *locale,
                            const struct M3Date *date, char *out_text,
                            m3_usize text_capacity, m3_usize *out_len);
  /**
   * @brief Format a time into a caller-provided buffer.
   * @param ctx Formatter context pointer.
   * @param locale_tag Locale tag string.
   * @param locale Locale formatting description.
   * @param time Time descriptor.
   * @param out_text Destination buffer.
   * @param text_capacity Size of the destination buffer.
   * @param out_len Receives number of bytes written (excluding null).
   * @return M3_OK on success, M3_ERR_UNSUPPORTED to fall back, or a failure
   * code.
   */
  int(M3_CALL *format_time)(void *ctx, const char *locale_tag,
                            const M3I18nLocale *locale,
                            const struct M3Time *time, char *out_text,
                            m3_usize text_capacity, m3_usize *out_len);
} M3I18nFormatterVTable;

/**
 * @brief I18N configuration.
 */
typedef struct M3I18nConfig {
  const M3Allocator *allocator; /**< Allocator for I18N memory; NULL uses
                                   default allocator. */
  m3_usize entry_capacity;      /**< Initial entry capacity (> 0). */
  const char *locale_tag;       /**< Locale tag string (NULL uses default). */
  const M3I18nLocale *locale;   /**< Locale settings override (NULL uses
                                   locale_tag preset). */
  const M3I18nFormatter *formatter; /**< Optional formatter override. */
} M3I18nConfig;

/**
 * @brief I18N string entry.
 */
typedef struct M3I18nEntry {
  char *key;          /**< UTF-8 key bytes (null-terminated copy). */
  m3_usize key_len;   /**< Key length in bytes (excluding null terminator). */
  char *value;        /**< UTF-8 value bytes (null-terminated copy). */
  m3_usize value_len; /**< Value length in bytes (excluding null terminator). */
} M3I18nEntry;

/**
 * @brief I18N instance.
 */
typedef struct M3I18n {
  M3Allocator allocator;     /**< Allocator used for I18N memory. */
  M3I18nEntry *entries;      /**< Entry array (capacity-sized). */
  m3_usize entry_count;      /**< Number of active entries. */
  m3_usize entry_capacity;   /**< Allocated entry capacity. */
  char *locale_tag;          /**< Locale tag string (null-terminated). */
  m3_usize locale_tag_len;   /**< Locale tag length in bytes. */
  M3I18nLocale locale;       /**< Locale formatting description. */
  M3I18nFormatter formatter; /**< Optional formatter override. */
} M3I18n;

/**
 * @brief Initialize a locale with default settings.
 * @param locale Locale descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_locale_init(M3I18nLocale *locale);

/**
 * @brief Initialize a locale from a preset locale tag.
 * @param locale_tag Locale tag string.
 * @param out_locale Receives the locale description.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if unknown, or a failure code.
 */
M3_API int M3_CALL m3_i18n_locale_from_tag(const char *locale_tag,
                                           M3I18nLocale *out_locale);

/**
 * @brief Initialize an I18N configuration with defaults.
 * @param config Configuration to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_config_init(M3I18nConfig *config);

/**
 * @brief Initialize an I18N instance.
 * @param i18n Instance to initialize.
 * @param config Configuration (NULL uses defaults).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_init(M3I18n *i18n, const M3I18nConfig *config);

/**
 * @brief Shut down an I18N instance and release resources.
 * @param i18n Instance to shut down.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_shutdown(M3I18n *i18n);

/**
 * @brief Update the locale settings for an I18N instance.
 * @param i18n Instance to update.
 * @param locale_tag Locale tag string.
 * @param locale Locale override (NULL uses locale_tag preset).
 * @return M3_OK on success, M3_ERR_NOT_FOUND if the preset is unknown, or a
 * failure code.
 */
M3_API int M3_CALL m3_i18n_set_locale(M3I18n *i18n, const char *locale_tag,
                                      const M3I18nLocale *locale);

/**
 * @brief Update the formatter override for an I18N instance.
 * @param i18n Instance to update.
 * @param formatter Formatter override (NULL clears it).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_set_formatter(M3I18n *i18n,
                                         const M3I18nFormatter *formatter);

/**
 * @brief Insert or update a localized string.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param utf8_value UTF-8 value bytes (may be NULL when value_len is 0).
 * @param value_len Value length in bytes.
 * @param overwrite M3_TRUE to overwrite existing values.
 * @return M3_OK on success, M3_ERR_BUSY if the key exists and overwrite is
 * false, or a failure code.
 */
M3_API int M3_CALL m3_i18n_put(M3I18n *i18n, const char *utf8_key,
                               m3_usize key_len, const char *utf8_value,
                               m3_usize value_len, M3Bool overwrite);

/**
 * @brief Retrieve a localized string pointer.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_value Receives the value pointer.
 * @param out_value_len Receives the value length in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if missing, or a failure code.
 */
M3_API int M3_CALL m3_i18n_get(const M3I18n *i18n, const char *utf8_key,
                               m3_usize key_len, const char **out_value,
                               m3_usize *out_value_len);

/**
 * @brief Check whether a key exists in the I18N table.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @param out_exists Receives M3_TRUE if the key exists.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_contains(const M3I18n *i18n, const char *utf8_key,
                                    m3_usize key_len, M3Bool *out_exists);

/**
 * @brief Remove a localized string.
 * @param i18n I18N instance.
 * @param utf8_key UTF-8 key bytes.
 * @param key_len Key length in bytes.
 * @return M3_OK on success, M3_ERR_NOT_FOUND if missing, or a failure code.
 */
M3_API int M3_CALL m3_i18n_remove(M3I18n *i18n, const char *utf8_key,
                                  m3_usize key_len);

/**
 * @brief Remove all localized strings.
 * @param i18n I18N instance.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_clear(M3I18n *i18n);

/**
 * @brief Get the number of localized strings.
 * @param i18n I18N instance.
 * @param out_count Receives the entry count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_count(const M3I18n *i18n, m3_usize *out_count);

/**
 * @brief Load a localization table from a file.
 * @param i18n I18N instance.
 * @param io IO backend.
 * @param utf8_path Input file path in UTF-8.
 * @param clear_existing M3_TRUE to clear existing entries first.
 * @param overwrite M3_TRUE to overwrite existing keys.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_load_table(M3I18n *i18n, M3IO *io,
                                      const char *utf8_path,
                                      M3Bool clear_existing, M3Bool overwrite);

/**
 * @brief Load a localization table from memory.
 * @param i18n I18N instance.
 * @param data Table buffer (may be NULL when size is 0).
 * @param size Table size in bytes.
 * @param clear_existing M3_TRUE to clear existing entries first.
 * @param overwrite M3_TRUE to overwrite existing keys.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_load_table_buffer(M3I18n *i18n, const char *data,
                                             m3_usize size,
                                             M3Bool clear_existing,
                                             M3Bool overwrite);

/**
 * @brief Format a number using locale settings.
 * @param i18n I18N instance.
 * @param number Number descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return M3_OK on success, M3_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
M3_API int M3_CALL m3_i18n_format_number(const M3I18n *i18n,
                                         const M3I18nNumber *number,
                                         char *out_text, m3_usize text_capacity,
                                         m3_usize *out_len);

/**
 * @brief Format a date using locale settings.
 * @param i18n I18N instance.
 * @param date Date descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return M3_OK on success, M3_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
M3_API int M3_CALL m3_i18n_format_date(const M3I18n *i18n,
                                       const struct M3Date *date,
                                       char *out_text, m3_usize text_capacity,
                                       m3_usize *out_len);

/**
 * @brief Format a time using locale settings.
 * @param i18n I18N instance.
 * @param time Time descriptor.
 * @param out_text Destination buffer.
 * @param text_capacity Size of the destination buffer.
 * @param out_len Receives number of bytes written (excluding null).
 * @return M3_OK on success, M3_ERR_RANGE if the buffer is too small, or a
 * failure code.
 */
M3_API int M3_CALL m3_i18n_format_time(const M3I18n *i18n,
                                       const struct M3Time *time,
                                       char *out_text, m3_usize text_capacity,
                                       m3_usize *out_len);

#ifdef M3_TESTING
/**
 * @brief Test wrapper for overflow-safe multiplication.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives product.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_mul_overflow(m3_usize a, m3_usize b,
                                             m3_usize *out_value);

/**
 * @brief Test wrapper for overflow-safe addition.
 * @param a First operand.
 * @param b Second operand.
 * @param out_value Receives sum.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_add_overflow(m3_usize a, m3_usize b,
                                             m3_usize *out_value);

/**
 * @brief Test wrapper for base-10 exponent helper.
 * @param digits Exponent value.
 * @param out_value Receives the computed power.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_pow10(m3_u32 digits, m3_u32 *out_value);

/**
 * @brief Test wrapper for leap year detection.
 * @param year Year to evaluate.
 * @param out_leap Receives M3_TRUE if leap year.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_is_leap_year(m3_i32 year, M3Bool *out_leap);

/**
 * @brief Test wrapper for days-in-month helper.
 * @param year Year component.
 * @param month Month component.
 * @param out_days Receives days in month.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_days_in_month(m3_i32 year, m3_u32 month,
                                              m3_u32 *out_days);

/**
 * @brief Test wrapper for locale validation.
 * @param locale Locale descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_validate_locale(const M3I18nLocale *locale);

/**
 * @brief Test wrapper for number validation.
 * @param number Number descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_validate_number(const M3I18nNumber *number);

/**
 * @brief Test wrapper for date validation.
 * @param date Date descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_validate_date(const struct M3Date *date);

/**
 * @brief Test wrapper for time validation.
 * @param time Time descriptor.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_validate_time(const struct M3Time *time);

/**
 * @brief Test wrapper for UTF-8 validation.
 * @param data UTF-8 data pointer (may be NULL when length is 0).
 * @param length Data length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_validate_utf8(const char *data,
                                              m3_usize length);

/**
 * @brief Test wrapper for C-string length helper.
 * @param cstr Null-terminated string.
 * @param out_len Receives string length in bytes.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_cstrlen(const char *cstr, m3_usize *out_len);

/**
 * @brief Test wrapper for ASCII lowercase conversion.
 * @param ch Input character.
 * @param out_lower Receives lowercase value.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_ascii_lower(char ch, char *out_lower);

/**
 * @brief Test wrapper for locale tag comparison.
 * @param a First tag.
 * @param b Second tag.
 * @param out_equal Receives M3_TRUE when equal.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_tag_equals(const char *a, const char *b,
                                           M3Bool *out_equal);

/**
 * @brief Test wrapper for trimming span helper.
 * @param data Input buffer.
 * @param length Input length.
 * @param out_start Receives trimmed start index.
 * @param out_length Receives trimmed length.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_trim_span(const char *data, m3_usize length,
                                          m3_usize *out_start,
                                          m3_usize *out_length);

/**
 * @brief Test wrapper for string allocation helper.
 * @param allocator Allocator interface.
 * @param src Source bytes.
 * @param len Source length in bytes.
 * @param out_str Receives allocated string.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_alloc_string(const M3Allocator *allocator,
                                             const char *src, m3_usize len,
                                             char **out_str);

/**
 * @brief Test wrapper for unsigned integer formatting helper.
 * @param out_text Destination buffer.
 * @param text_capacity Destination buffer size.
 * @param io_offset In/out offset into the buffer.
 * @param value Value to format.
 * @param min_digits Minimum digit count.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_write_uint(char *out_text,
                                           m3_usize text_capacity,
                                           m3_usize *io_offset, m3_u32 value,
                                           m3_u32 min_digits);

/**
 * @brief Test wrapper for entry lookup helper.
 * @param i18n I18N instance.
 * @param key Key bytes.
 * @param key_len Key length in bytes.
 * @param out_index Receives entry index.
 * @param out_found Receives M3_TRUE when found.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_find_entry(const M3I18n *i18n, const char *key,
                                           m3_usize key_len,
                                           m3_usize *out_index,
                                           M3Bool *out_found);

/**
 * @brief Test wrapper for entry growth helper.
 * @param i18n I18N instance.
 * @param min_capacity Minimum entry capacity.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_grow(M3I18n *i18n, m3_usize min_capacity);

/**
 * @brief Test helper for setting the maximum C-string length.
 * @param max_len Maximum allowed length (0 resets to default).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_set_cstr_limit(m3_usize max_len);

/**
 * @brief Force the pow10 helper to return an error once (testing only).
 * @param enable M3_TRUE to force an error on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_pow10_error(M3Bool enable);

/**
 * @brief Force leap-year detection to return an error once (testing only).
 * @param enable M3_TRUE to force an error on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_leap_error(M3Bool enable);

/**
 * @brief Force the days-in-month helper to hit the default branch once.
 * @param enable M3_TRUE to force the default path on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_days_default(M3Bool enable);

/**
 * @brief Fail the ASCII lowercase helper after a number of calls.
 * @param call_count Number of calls before failing (0 to disable).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_set_ascii_lower_fail_after(m3_u32 call_count);

/**
 * @brief Force UTF-8 validation to return an error once (testing only).
 * @param enable M3_TRUE to force an error on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_utf8_error(M3Bool enable);

/**
 * @brief Force UTF-8 validation to return success once (testing only).
 * @param enable M3_TRUE to force success on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_utf8_ok(M3Bool enable);

/**
 * @brief Force UTF-8 validation to return success for a number of calls.
 * @param count Number of calls to force success for (0 to disable).
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_utf8_ok_count(m3_u32 count);

/**
 * @brief Force config initialization to return an error once (testing only).
 * @param enable M3_TRUE to force an error on the next call.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_force_config_init_error(M3Bool enable);

/**
 * @brief Test wrapper for en-US locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_locale_preset_en_us(M3I18nLocale *out_locale);

/**
 * @brief Test wrapper for en-GB locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_locale_preset_en_gb(M3I18nLocale *out_locale);

/**
 * @brief Test wrapper for fr-FR locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_locale_preset_fr_fr(M3I18nLocale *out_locale);

/**
 * @brief Test wrapper for de-DE locale preset.
 * @param out_locale Locale descriptor to initialize.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_locale_preset_de_de(M3I18nLocale *out_locale);

/**
 * @brief Test wrapper for parsing translation tables.
 * @param i18n I18N instance.
 * @param data Table text data.
 * @param size Size of the text buffer.
 * @param clear_existing M3_TRUE to clear existing entries.
 * @param overwrite M3_TRUE to overwrite existing keys.
 * @return M3_OK on success or a failure code.
 */
M3_API int M3_CALL m3_i18n_test_parse_table(M3I18n *i18n, const char *data,
                                            m3_usize size,
                                            M3Bool clear_existing,
                                            M3Bool overwrite);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* M3_I18N_H */
