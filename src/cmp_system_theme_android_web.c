/* clang-format off */
#include "cmp_system_theme.h"

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/**
 * @brief cmp_system_theme_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_init(void) {
  return 0;
}

/**
 * @brief cmp_system_theme_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_shutdown(void) {
  return 0;
}

/**
 * @brief cmp_system_theme_is_dark
 *
 * @param out_is_dark Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_dark(int *out_is_dark) {
  if (!out_is_dark) return -1;
  *out_is_dark = 0;

#ifdef __EMSCRIPTEN__
  *out_is_dark = EM_ASM_INT({
    if (typeof window !== 'undefined' && window.matchMedia) {
      return window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
    }
    return 0;
  });
#elif defined(__ANDROID__)
  /* Android implementation via JNI would go here.
     For now, default to light or use some global state. */
#endif

  return 0;
}

/**
 * @brief cmp_system_theme_is_high_contrast
 *
 * @param out_is_high_contrast Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
  if (!out_is_high_contrast) return -1;
  *out_is_high_contrast = 0;

#ifdef __EMSCRIPTEN__
  *out_is_high_contrast = EM_ASM_INT({
    if (typeof window !== 'undefined' && window.matchMedia) {
      return window.matchMedia('(prefers-contrast: more)').matches ? 1 : 0;
    }
    return 0;
  });
#elif defined(__ANDROID__)
  /* Requires Android 14+ UI_MODE_NIGHT_MASK logic via JNI, mock to 0 */
#endif

  return 0;
}
#endif /* __ANDROID__ || __EMSCRIPTEN__ */
/* clang-format on */