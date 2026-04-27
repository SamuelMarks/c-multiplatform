/* clang-format off */
#include "cmp_system_theme.h"
#include "cmp_log.h"
#include <stdio.h>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/* Include specific headers instead of <windows.h> if possible, 
   but since cmp.h eventually includes <winsock2.h> which includes windows.h, 
   the types are already defined. */

__declspec(dllimport) LONG __stdcall RegOpenKeyExA(HKEY hKey, const char* lpSubKey, DWORD ulOptions, DWORD samDesired, HKEY* phkResult);
__declspec(dllimport) LONG __stdcall RegQueryValueExA(HKEY hKey, const char* lpValueName, DWORD* lpReserved, DWORD* lpType, BYTE* lpData, DWORD* lpcbData);
__declspec(dllimport) LONG __stdcall RegCloseKey(HKEY hKey);
#pragma comment(lib, "advapi32.lib")

#ifndef SPI_GETHIGHCONTRAST
#define SPI_GETHIGHCONTRAST 0x0042
#endif

#ifndef HCF_HIGHCONTRASTON
#define HCF_HIGHCONTRASTON  0x00000001
#endif

#ifndef HKEY_CURRENT_USER
#define HKEY_CURRENT_USER ((HKEY)(ULONG_PTR)((LONG)0x80000001))
#endif

#ifndef KEY_READ
#define KEY_READ 0x20019
#endif

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif

typedef struct tagHIGHCONTRASTA {
    unsigned int cbSize;
    DWORD dwFlags;
    char *lpszDefaultScheme;
} HIGHCONTRASTA, *LPHIGHCONTRASTA;

__declspec(dllimport) BOOL __stdcall SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
#pragma comment(lib, "user32.lib")

static int g_theme_is_dark = 0;

/**
 * @brief Initializes the Windows system theme detector.
 *
 * @return Returns CMP_SUCCESS on success.
 */
int cmp_system_theme_init(void) {
  int rc;
  rc = 0;int is_dark = 0;
  cmp_system_theme_is_dark(&is_dark);
  g_theme_is_dark = is_dark;
  /* In a real implementation we would listen to WM_SETTINGCHANGE here. */
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Shuts down the Windows system theme detector.
 *
 * @return Returns CMP_SUCCESS on success.
 */
int cmp_system_theme_shutdown(void) {
  int rc;
  rc = 0;if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Checks if the system theme is set to dark mode.
 *
 * @param out_is_dark Pointer to an int that will be set to 1 if dark, 0 if light.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_system_theme_is_dark(int *out_is_dark) {
  int rc;
  rc = 0;HKEY hKey;
  LONG res;
  DWORD value = 1; /* Default to Light */
  DWORD dataSize = sizeof(value);

  if (!out_is_dark) {
    LOG_DEBUG("cmp_system_theme_is_dark: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  res = RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey);

  if (res == ERROR_SUCCESS) {
    res = RegQueryValueExA(hKey, "AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &dataSize);
    RegCloseKey(hKey);
  }

  /* AppsUseLightTheme = 0 means Dark, 1 means Light */
  *out_is_dark = (value == 0) ? 1 : 0;
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}

/**
 * @brief Checks if the system is in high contrast mode.
 *
 * @param out_is_high_contrast Pointer to an int that will be set to 1 if high contrast, 0 otherwise.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_system_theme_is_high_contrast(int *out_is_high_contrast) {
  int rc;
  rc = 0;HIGHCONTRASTA hc = { sizeof(HIGHCONTRASTA), 0, 0 };
  if (!out_is_high_contrast) {
    LOG_DEBUG("cmp_system_theme_is_high_contrast: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  if (SystemParametersInfoA(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTA), &hc, 0)) {
    *out_is_high_contrast = (hc.dwFlags & HCF_HIGHCONTRASTON) ? 1 : 0;
    return CMP_SUCCESS;
  }

  *out_is_high_contrast = 0;
  LOG_DEBUG("cmp_system_theme_is_high_contrast: SystemParametersInfoA failed\n");
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  return CMP_ERROR_GENERAL;}
#endif /* _WIN32 */
/* clang-format on */