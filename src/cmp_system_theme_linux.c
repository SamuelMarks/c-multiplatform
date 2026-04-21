/* clang-format off */
#include "cmp_system_theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__) && !defined(__ANDROID__)

int cmp_system_theme_init(void) {
  return 0;
}

int cmp_system_theme_shutdown(void) {
  return 0;
}

int cmp_system_theme_is_dark(int *out_is_dark) {
  FILE *fp;
  char buffer[128];
  
  if (!out_is_dark) return -1;
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
    return 0;
  }
  
  /* Fallback to checking GTK_THEME env var */
  {
    const char *gtk_theme = getenv("GTK_THEME");
    if (gtk_theme && strstr(gtk_theme, "dark")) {
      *out_is_dark = 1;
    }
  }

  return 0;
}

int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
  FILE *fp;
  char buffer[128];

  if (!out_is_high_contrast) return -1;
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

  return 0;
}

#endif /* __linux__ */
/* clang-format on */