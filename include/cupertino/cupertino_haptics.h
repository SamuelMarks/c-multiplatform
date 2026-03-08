#ifndef CUPERTINO_HAPTICS_H
#define CUPERTINO_HAPTICS_H

/**
 * @file cupertino_haptics.h
 * @brief Apple Cupertino Haptics (UIFeedbackGenerator) mappings.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_core.h"
#include "cmpc/cmp_api_ws.h"

/** @brief iOS UIFeedbackGenerator equivalents */
typedef enum CupertinoHapticType {
    CUPERTINO_HAPTIC_SELECTION = 0,
    CUPERTINO_HAPTIC_IMPACT_LIGHT,
    CUPERTINO_HAPTIC_IMPACT_MEDIUM,
    CUPERTINO_HAPTIC_IMPACT_HEAVY,
    CUPERTINO_HAPTIC_IMPACT_RIGID,
    CUPERTINO_HAPTIC_IMPACT_SOFT,
    CUPERTINO_HAPTIC_NOTIFICATION_SUCCESS,
    CUPERTINO_HAPTIC_NOTIFICATION_WARNING,
    CUPERTINO_HAPTIC_NOTIFICATION_ERROR
} CupertinoHapticType;

/**
 * @brief Triggers a Cupertino haptic response using the window system backend.
 * @param ws Window system interface.
 * @param type The type of haptic feedback to trigger.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cupertino_haptic_trigger(const CMPWS *ws, CupertinoHapticType type);

#ifdef __cplusplus
}
#endif

#endif /* CUPERTINO_HAPTICS_H */
