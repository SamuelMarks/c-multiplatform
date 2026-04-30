/* clang-format off */
#include "cmp.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windef.h>
#include <winbase.h>

/**
 * @brief cmp_win32_init_touch_ink
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_win32_init_touch_ink(void) {
int rc = CMP_SUCCESS;/* EnableMouseInPointer ensures WM_POINTER messages are generated for mouse as well,
     unifying the input stack for touch, pen, and mouse (Windows 8+). */
  void* user32 = GetModuleHandleA("user32.dll");
  if (user32) {
    typedef int (__stdcall * EnableMouseInPointer_fn)(int);
    EnableMouseInPointer_fn enable_mouse_in_pointer =
        (EnableMouseInPointer_fn)GetProcAddress(user32, "EnableMouseInPointer");
    if (enable_mouse_in_pointer) {
      enable_mouse_in_pointer(1); /* TRUE */
    }
  }
  
  return rc; /* CMP_SUCCESS */}
#else

/**
 * @brief cmp_win32_init_touch_ink
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_win32_init_touch_ink(void) {
  int rc = CMP_SUCCESS;
  
  return rc;
}
#endif
/* clang-format on */
