/* clang-format off */
#include "ui_platform_detector.h"
#include "ui_internal_mem.h"

#if defined(_WIN32)
#include <winsock2.h> /* Prevents windows.h bloat issues occasionally */
/* clang-format on */
/* For actual capability detection, GetSystemMetrics etc could be used,
   but we avoid direct windows.h include unless necessary or forward declare. */
#elif defined(__APPLE__)
/* Apple capability macros */
#elif defined(__linux__) || defined(__gnu_linux__) || defined(__CYGWIN__)
/* Linux capability macros */
#endif

struct ui_platform_detector {
  unsigned int current_capabilities;
};

/** \brief ui_error */
ui_error_t
ui_platform_detector_create(struct ui_platform_detector **out_detector) {
  struct ui_platform_detector *detector;
  unsigned int caps = 0;

  if (out_detector == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  detector = (struct ui_platform_detector *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_platform_detector));
  if (detector == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  /* Mock minimal logic for cross-platform baseline capabilities.
     Real environments would use platform-specific API calls to detect. */
  caps = UI_PLATFORM_CAP_MOUSE | UI_PLATFORM_CAP_KEYBOARD;

#if defined(__ANDROID__) || defined(__APPLE__) && defined(TARGET_OS_IOS)
  /* Mobile environments usually have touch. */
  caps |= UI_PLATFORM_CAP_TOUCH;
#endif

  detector->current_capabilities = caps;
  *out_detector = detector;

  return UI_ERROR_NONE;
}

ui_error_t ui_platform_detector_destroy(struct ui_platform_detector *detector) {
  if (detector != NULL) {
    C_MULTIPLATFORM_FREE(detector);
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_platform_detector_get_capabilities(struct ui_platform_detector *detector,
                                      unsigned int *out_capabilities) {
  if (detector == NULL || out_capabilities == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_capabilities = detector->current_capabilities;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_platform_detector_has_capability(struct ui_platform_detector *detector,
                                    unsigned int capability,
                                    int *out_has_capability) {
  if (detector == NULL || out_has_capability == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if ((detector->current_capabilities & capability) == capability) {
    *out_has_capability = 1;
  } else {
    *out_has_capability = 0;
  }

  return UI_ERROR_NONE;
}
