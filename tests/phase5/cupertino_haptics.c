/* clang-format off */
#include "cupertino/cupertino_haptics.h"
#include "../phase1/test_utils.h"
#include <stddef.h>
/* clang-format on */

static int test_cupertino_haptics(void) {
  CMPWS ws;
  CMPWSVTable vtable = {0};
  ws.vtable = &vtable;

  CMP_TEST_EXPECT(cupertino_haptic_trigger(NULL, CUPERTINO_HAPTIC_SELECTION),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cupertino_haptic_trigger(&ws, (CupertinoHapticType)999),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Should return unsupported for now */
  CMP_TEST_EXPECT(cupertino_haptic_trigger(&ws, CUPERTINO_HAPTIC_SELECTION),
                  CMP_ERR_UNSUPPORTED);
  CMP_TEST_EXPECT(cupertino_haptic_trigger(&ws, CUPERTINO_HAPTIC_IMPACT_HEAVY),
                  CMP_ERR_UNSUPPORTED);

  return 0;
}

int main(void) {
  if (test_cupertino_haptics() != 0)
    return 1;
  return 0;
}
