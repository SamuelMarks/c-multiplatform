/* clang-format off */
#include "cmp_system_theme.h"
#include <stdio.h>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static int g_theme_is_dark = 0;

int cmp_system_theme_init(void) {
  int is_dark = 0;
  cmp_system_theme_is_dark(&is_dark);
  g_theme_is_dark = is_dark;
  /* In a real implementation we would listen to WM_SETTINGCHANGE here. */
  return 0;
}

int cmp_system_theme_shutdown(void) {
  return 0;
}

int cmp_system_theme_is_dark(int *out_is_dark) {
  HKEY hKey;
  LONG res;
  DWORD value = 1; /* Default to Light */
  DWORD dataSize = sizeof(value);

  if (!out_is_dark) return -1;

  res = RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey);

  if (res == ERROR_SUCCESS) {
    res = RegQueryValueExA(hKey, "AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &dataSize);
    RegCloseKey(hKey);
  }

  /* AppsUseLightTheme = 0 means Dark, 1 means Light */
  *out_is_dark = (value == 0) ? 1 : 0;
  return 0;
}

int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
  HIGHCONTRASTA hc = { sizeof(HIGHCONTRASTA) };
  if (!out_is_high_contrast) return -1;

  if (SystemParametersInfoA(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTA), &hc, 0)) {
    *out_is_high_contrast = (hc.dwFlags & HCF_HIGHCONTRASTON) ? 1 : 0;
    return 0;
  }
  
  *out_is_high_contrast = 0;
  return -1;
}

#endif /* _WIN32 */
/* clang-format on */