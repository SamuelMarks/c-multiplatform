/**
 * @file ui_platform_detector.h
 * @brief OS and hardware capability detector (touch vs mouse vs keyboard).
 */

#ifndef UI_PLATFORM_DETECTOR_H
#define UI_PLATFORM_DETECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Platform capability flags.
 */
#define UI_PLATFORM_CAP_TOUCH (1 << 0) /**< Touch input is supported */
#define UI_PLATFORM_CAP_MOUSE (1 << 1) /**< Mouse input is supported */
#define UI_PLATFORM_CAP_KEYBOARD                                               \
  (1 << 2) /**< Hardware keyboard is supported                                 \
            */

/**
 * @brief Represents the platform detector instance.
 */
struct ui_platform_detector;

/**
 * @brief Creates a platform detector instance.
 * @param out_detector Pointer to receive the new instance.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_platform_detector_create(struct ui_platform_detector **out_detector);

/**
 * @brief Destroys a platform detector instance.
 * @param detector The instance to destroy.
 */
ui_error_t ui_platform_detector_destroy(struct ui_platform_detector *detector);

/**
 * @brief Gets the capabilities of the current platform.
 * @param detector The detector instance.
 * @param out_capabilities Pointer to receive the capabilities bitmask.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_platform_detector_get_capabilities(struct ui_platform_detector *detector,
                                      unsigned int *out_capabilities);

/**
 * @brief Checks if the platform has a specific capability.
 * @param detector The detector instance.
 * @param capability The capability flag to check.
 * @param out_has_capability Pointer to receive 1 if true, 0 if false.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_platform_detector_has_capability(struct ui_platform_detector *detector,
                                    unsigned int capability,
                                    int *out_has_capability);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_PLATFORM_DETECTOR_H */
