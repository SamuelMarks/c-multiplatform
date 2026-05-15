/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windef.h>
#include <winbase.h>

typedef int (__stdcall * EnableMouseInPointer_fn)(int);

/**
 * @brief Enable advanced Win32 Touch, Pen, and Windows Ink input.
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_win32_init_touch_ink(void) {
  int rc = CMP_SUCCESS;
  void *user32 = NULL;
  EnableMouseInPointer_fn enable_mouse_in_pointer = NULL;

  /* EnableMouseInPointer ensures WM_POINTER messages are generated for mouse as well,
     unifying the input stack for touch, pen, and mouse (Windows 8+). */
  user32 = (void *)GetModuleHandleA("user32.dll");
  if (user32) {
    enable_mouse_in_pointer =
        (EnableMouseInPointer_fn)(INT_PTR)GetProcAddress((HMODULE)user32, "EnableMouseInPointer");
    if (enable_mouse_in_pointer) {
      if (!enable_mouse_in_pointer(1)) {
        LOG_DEBUG("cmp_win32_init_touch_ink: EnableMouseInPointer failed\n");
      }
    } else {
      LOG_DEBUG("cmp_win32_init_touch_ink: EnableMouseInPointer not found\n");
    }
  } else {
    LOG_DEBUG("cmp_win32_init_touch_ink: user32.dll not found\n");
    rc = CMP_ERROR_GENERAL;
  }
  
  return rc;
}
#else

/**
 * @brief Enable advanced Win32 Touch, Pen, and Windows Ink input (stub).
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_win32_init_touch_ink(void) {
  int rc = CMP_SUCCESS;

  LOG_DEBUG("cmp_win32_init_touch_ink: Mocked Win32 Touch/Ink initialization\n");

  return rc;
}

#endif
/* clang-format on */
