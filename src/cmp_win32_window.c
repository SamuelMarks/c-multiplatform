/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif

#ifndef WINAPI
#define WINAPI __stdcall
#endif

__declspec(dllimport) HMODULE WINAPI LoadLibraryA(const char* lpLibFileName);
__declspec(dllimport) HMODULE WINAPI GetModuleHandleA(const char* lpModuleName);
__declspec(dllimport) void* WINAPI GetProcAddress(HMODULE hModule, const char* lpProcName);
__declspec(dllimport) BOOL WINAPI FreeLibrary(HMODULE hLibModule);

/* Fallback definitions for older SDKs */
#ifndef DWM_BB_ENABLE
#define DWM_BB_ENABLE 0x00000001
typedef struct _DWM_BLURBEHIND {
  DWORD dwFlags;
  BOOL fEnable;
  HRGN hRgnBlur;
  BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;
#endif

typedef HRESULT(WINAPI *pfnDwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND *);
typedef struct _MARGINS_DWM {
  int cxLeftWidth;
  int cxRightWidth;
  int cyTopHeight;
  int cyBottomHeight;
} MARGINS_DWM;

typedef HRESULT(WINAPI *pfnDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
typedef HRESULT(WINAPI *pfnDwmExtendFrameIntoClientArea)(HWND, const MARGINS_DWM *);

/* Mirror the internal structure from cmp_window.c */
struct cmp_window {
  cmp_window_config_t config;
  HWND hwnd;
};

/* Undocumented API for SetWindowCompositionAttribute (Acrylic/Blur on older
 * Windows 10) */
typedef enum _WINDOWCOMPOSITIONATTRIB {
  WCA_ACCENT_POLICY = 19
} WINDOWCOMPOSITIONATTRIB;

typedef enum _ACCENT_STATE {
  ACCENT_DISABLED = 0,
  ACCENT_ENABLE_GRADIENT = 1,
  ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
  ACCENT_ENABLE_BLURBEHIND = 3,
  ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
  ACCENT_INVALID_STATE = 5
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
  ACCENT_STATE AccentState;
  DWORD AccentFlags;
  DWORD GradientColor;
  DWORD AnimationId;
} ACCENT_POLICY;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attrib;
  PVOID pvData;
  SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef BOOL(WINAPI *pfnSetWindowCompositionAttribute)(
    HWND, WINDOWCOMPOSITIONATTRIBDATA *);
#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 1029
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif

#ifndef DWMSBT_TABBEDWINDOW
#define DWMSBT_TABBEDWINDOW 4
#endif

/**
 * @brief Requests a specific Windows material effect for a window.
 *
 * @param materials The materials context.
 * @param window The window to apply the material to.
 * @param material The desired material effect.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_win32_request_windows_material(cmp_materials_t *materials,
                                       cmp_window_t *window,
                                       cmp_windows_material_t material) {
  int rc;
  rc = 0;HWND hwnd;
  HMODULE hUser, hDwm;
  pfnSetWindowCompositionAttribute setWindowCompositionAttribute;
  pfnDwmSetWindowAttribute dwmSetWindowAttribute = NULL;
  pfnDwmEnableBlurBehindWindow dwmEnableBlurBehindWindow = NULL;
  pfnDwmExtendFrameIntoClientArea dwmExtendFrameIntoClientArea = NULL;

  int true_val = 1;
  int mica_type;

  (void)materials; /* Unused for now */

  if (!window) {
    LOG_DEBUG("cmp_win32_request_windows_material: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }
  hwnd = window->hwnd;
  if (!hwnd) {
    LOG_DEBUG("cmp_win32_request_windows_material: Invalid state (no HWND)\n");
    return CMP_ERROR_INVALID_STATE;
  }

  hDwm = LoadLibraryA("dwmapi.dll");
  if (hDwm) {
    dwmSetWindowAttribute = (pfnDwmSetWindowAttribute)GetProcAddress(
        hDwm, "DwmSetWindowAttribute");
    dwmEnableBlurBehindWindow = (pfnDwmEnableBlurBehindWindow)GetProcAddress(
        hDwm, "DwmEnableBlurBehindWindow");
    dwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea)GetProcAddress(
        hDwm, "DwmExtendFrameIntoClientArea");
  }

  if (dwmExtendFrameIntoClientArea && material != CMP_WINDOWS_MATERIAL_NONE) {
    MARGINS_DWM margins = {-1, -1, -1, -1};
    dwmExtendFrameIntoClientArea(hwnd, &margins);
  }

  if (material == CMP_WINDOWS_MATERIAL_NONE) {
    /* Disable all */
    if (dwmEnableBlurBehindWindow) {
      DWM_BLURBEHIND bb;
      bb.dwFlags = DWM_BB_ENABLE;
      bb.fEnable = FALSE;
      bb.hRgnBlur = NULL;
      bb.fTransitionOnMaximized = FALSE;
      dwmEnableBlurBehindWindow(hwnd, &bb);
    }
    if (hDwm)
      FreeLibrary(hDwm);
    return CMP_SUCCESS;
  }

  if (material == CMP_WINDOWS_MATERIAL_MICA ||
      material == CMP_WINDOWS_MATERIAL_MICA_ALT ||
      material == CMP_WINDOWS_MATERIAL_ACRYLIC_BASE ||
      material == CMP_WINDOWS_MATERIAL_ACRYLIC_THIN) {
    if (dwmSetWindowAttribute) {
      /* Try Windows 11 System Backdrop Type */
      if (material == CMP_WINDOWS_MATERIAL_MICA) {
        mica_type = DWMSBT_MAINWINDOW;
      } else if (material == CMP_WINDOWS_MATERIAL_MICA_ALT) {
        mica_type = DWMSBT_TABBEDWINDOW;
      } else {
        mica_type = 3; /* DWMSBT_TRANSIENTWINDOW */
      }

      /* First try newer Windows 11 API (System Backdrop Type) */
      if (dwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &mica_type,
                                sizeof(mica_type)) == S_OK) {
        if (hDwm)
          FreeLibrary(hDwm);
        return CMP_SUCCESS;
      }

      /* Fallback to older Windows 11 API (Mica Effect boolean) */
      if (dwmSetWindowAttribute(hwnd, DWMWA_MICA_EFFECT, &true_val,
                                sizeof(true_val)) == S_OK) {
        if (hDwm)
          FreeLibrary(hDwm);
        return CMP_SUCCESS;
      }
    }
  }

  /* Try Windows 10 Acrylic via SetWindowCompositionAttribute */
  hUser = GetModuleHandleA("user32.dll");
  if (hUser) {
    setWindowCompositionAttribute =
        (pfnSetWindowCompositionAttribute)GetProcAddress(
            hUser, "SetWindowCompositionAttribute");
    if (setWindowCompositionAttribute) {
      ACCENT_POLICY policy;
      WINDOWCOMPOSITIONATTRIBDATA data;

      policy.AccentState = (material == CMP_WINDOWS_MATERIAL_ACRYLIC_THIN ||
                            material == CMP_WINDOWS_MATERIAL_ACRYLIC_BASE)
                               ? ACCENT_ENABLE_ACRYLICBLURBEHIND
                               : ACCENT_ENABLE_BLURBEHIND;
      policy.AccentFlags = 0x20 | 0x40 | 0x80 | 0x100; /* Preserve borders and titlebar */
      /* Optional: Gradient color for Acrylic (AABBGGRR). Example: 0x01000000
       * for slightly dark */
      policy.GradientColor = 0x01000000;
      policy.AnimationId = 0;

      data.Attrib = WCA_ACCENT_POLICY;
      data.pvData = &policy;
      data.cbData = sizeof(policy);

      if (setWindowCompositionAttribute(hwnd, &data)) {
        if (hDwm)
          FreeLibrary(hDwm);
        return CMP_SUCCESS;
      }
    }
  }

  /* Ultimate fallback: Windows 7/Vista Blur Behind */
  if (dwmEnableBlurBehindWindow) {
    DWM_BLURBEHIND bb;
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;
    bb.fTransitionOnMaximized = FALSE;
    dwmEnableBlurBehindWindow(hwnd, &bb);
  }

  if (hDwm)
    FreeLibrary(hDwm);
  if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#endif
/* clang-format on */