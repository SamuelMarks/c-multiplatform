/* clang-format off */
#include "cmp.h"
#include "cmp.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * \brief Enable advanced Win32 Touch, Pen, and Windows Ink input.
 * \return 0 on success, or an error code.
 */
int cmp_win32_init_touch_ink(void) {
  /* EnableMouseInPointer ensures WM_POINTER messages are generated for mouse as well,
     unifying the input stack for touch, pen, and mouse (Windows 8+). */
  HMODULE user32 = GetModuleHandleA("user32.dll");
  if (user32) {
    typedef BOOL(WINAPI * EnableMouseInPointer_fn)(BOOL);
    EnableMouseInPointer_fn enable_mouse_in_pointer =
        (EnableMouseInPointer_fn)GetProcAddress(user32, "EnableMouseInPointer");
    if (enable_mouse_in_pointer) {
      enable_mouse_in_pointer(TRUE);
    }
  }
  return 0; /* CMP_SUCCESS */
}

#else

/**
 * \brief Stub for non-Windows platforms.
 * \return 0 on success.
 */
int cmp_win32_init_touch_ink(void) {
  return 0;
}

#endif
/* clang-format on */
