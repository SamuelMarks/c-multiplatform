/* clang-format off */
#include "cupertino/cupertino_haptics.h"
/* clang-format on */

CMP_API int CMP_CALL cupertino_haptic_trigger(const CMPWS *ws,
                                              CupertinoHapticType type) {
  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (type > CUPERTINO_HAPTIC_NOTIFICATION_ERROR) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  /* The core CMPWS interface currently lacks a dedicated haptics method.
     Until backends support this, we return UNSUPPORTED. */
  return CMP_ERR_UNSUPPORTED;
}
