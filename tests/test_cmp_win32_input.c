/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
/* clang-format on */

#define ASSERT_EQ(expected, actual)                                            \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      printf("%s:%d: Assertion failed: expected %d, got %d\n", __FILE__,       \
             __LINE__, (int)(expected), (int)(actual));                        \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main(void) {
  int rc;

  rc = cmp_win32_init_touch_ink();

#if defined(_WIN32)
  /* On older Windows, it might log a missing DLL or function and return
   * CMP_ERROR_GENERAL or CMP_SUCCESS */
  if (rc != CMP_SUCCESS && rc != CMP_ERROR_GENERAL) {
    ASSERT_EQ(CMP_SUCCESS, rc);
  }
#else
  ASSERT_EQ(CMP_SUCCESS, rc);
#endif

  printf("test_cmp_win32_input passed.\n");
  return 0;
}
