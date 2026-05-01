/* clang-format off */
#include "cmp_system_theme.h"

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/**
 * @brief Initializes the system theme detection module for Android and Web.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_init(void) {
int rc = CMP_SUCCESS;
  
  return rc;
}

/**
 * @brief Cleans up the system theme detection module for Android and Web.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_shutdown(void) {
int rc = CMP_SUCCESS;
  
  return rc;
}

/**
 * @brief Checks if the system prefers a dark color scheme.
 *
 * @param out_is_dark Pointer to store a boolean indicating if dark mode is active (1) or not (0).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_dark(int *out_is_dark) {
int rc = CMP_SUCCESS;if (!out_is_dark) return 2; /* CMP_ERROR_INVALID_ARG */
  *out_is_dark = 0;
#ifdef __EMSCRIPTEN__
  *out_is_dark = EM_ASM_INT({
    if (typeof window !== 'undefined' && window.matchMedia) {
      return window.matchMedia('(prefers-color-scheme: dark)').matches ? 1 : 0;
    }
    return rc;
  });
#elif defined(__ANDROID__)
  /* Android implementation via JNI would go here.
     For now, default to light or use some global state. */
#endif

  
  
  return rc;
}

/**
 * @brief Checks if the system prefers high contrast settings.
 *
 * @param out_is_high_contrast Pointer to store a boolean indicating if high contrast mode is active (1) or not (0).
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
int rc = CMP_SUCCESS;if (!out_is_high_contrast) return 2; /* CMP_ERROR_INVALID_ARG */
  *out_is_high_contrast = 0;
#ifdef __EMSCRIPTEN__
  *out_is_high_contrast = EM_ASM_INT({
    if (typeof window !== 'undefined' && window.matchMedia) {
      return window.matchMedia('(prefers-contrast: more)').matches ? 1 : 0;
    }
    return rc;
  });
#elif defined(__ANDROID__)
  /* Requires Android 14+ UI_MODE_NIGHT_MASK logic via JNI, mock to 0 */
#endif

  
  
  return rc;
}
#endif /* __ANDROID__ || __EMSCRIPTEN__ */
/* clang-format on */