/**
 * @file i18n_dict.h
 * @brief Internationalization translation dictionary header.
 */
#ifndef I18N_DICT_H
#define I18N_DICT_H

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the translated string for a given key and locale.
 *
 * @param locale The locale identifier (e.g. "en-US", "es-ES").
 * @param key The translation dictionary key.
 * @param out_str Output pointer for the translated string.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if out_str is
 * NULL, UI_ERROR_NOT_FOUND if the key doesn't exist.
 */
enum ui_error get_translated_string(const char *locale, const char *key,
                                    const char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* I18N_DICT_H */
