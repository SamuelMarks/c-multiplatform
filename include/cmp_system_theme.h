
#ifndef CMP_SYSTEM_THEME_H
#define CMP_SYSTEM_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "cmp.h"
/* clang-format on */

/**
 * @brief Initialize the system theme listener
 * @return 0 on success.
 */
CMP_API int cmp_system_theme_init(void);

/**
 * @brief Cleanup the system theme listener
 * @return 0 on success.
 */
CMP_API int cmp_system_theme_shutdown(void);

/**
 * @brief Get the current system color scheme preference
 * @param out_is_dark Pointer to receive 1 if dark mode is preferred, 0 for
 * light.
 * @return 0 on success.
 */
CMP_API int cmp_system_theme_is_dark(int *out_is_dark);

/**
 * @brief Get the current system high contrast mode preference
 * @param out_is_high_contrast Pointer to receive 1 if high contrast is active,
 * 0 otherwise.
 * @return 0 on success.
 */
CMP_API int cmp_system_theme_is_high_contrast(int *out_is_high_contrast);

#ifdef __cplusplus
}
#endif

#endif /* CMP_SYSTEM_THEME_H */
