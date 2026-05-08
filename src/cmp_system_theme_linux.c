/* clang-format off */
#include "cmp_system_theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

/**
 * @brief cmp_system_theme_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_init(void) {
int rc = CMP_SUCCESS;
  
  return rc;
}

/**
 * @brief cmp_system_theme_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_shutdown(void) {
int rc = CMP_SUCCESS;
  
  return rc;
}

/**
 * @brief cmp_system_theme_is_dark
 *
 * @param out_is_dark Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_dark(int *out_is_dark) {
int rc = CMP_SUCCESS;FILE *fp;
  char buffer[128];
  
  if (!out_is_dark) return CMP_ERROR_INVALID_ARG;
  *out_is_dark = 0; /* Default to light */
  
  /* Try gsettings first, which is common on GNOME/GTK environments */
  fp = popen("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      if (strstr(buffer, "prefer-dark") != NULL) {
        *out_is_dark = 1;
      }
    }
    pclose(fp);
    return rc;
  }
  
  /* Fallback to checking GTK_THEME env var */
  {
    const char *gtk_theme = getenv("GTK_THEME");
    if (gtk_theme && strstr(gtk_theme, "dark")) {
      *out_is_dark = 1;
    }
  }

  
  
  return rc;
}

/**
 * @brief cmp_system_theme_is_high_contrast
 *
 * @param out_is_high_contrast Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
int rc = CMP_SUCCESS;FILE *fp;
  char buffer[128];

  if (!out_is_high_contrast) return CMP_ERROR_INVALID_ARG;
  *out_is_high_contrast = 0; /* Default to off */

  fp = popen("gsettings get org.gnome.desktop.a11y.interface high-contrast 2>/dev/null", "r");
  if (fp) {
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      if (strstr(buffer, "true") != NULL) {
        *out_is_high_contrast = 1;
      }
    }
    pclose(fp);
  }

  
  
  return rc;
}
#endif /* !defined(_WIN32) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__) */
/* clang-format on */int cmp_dummy_var_linux = 0;
