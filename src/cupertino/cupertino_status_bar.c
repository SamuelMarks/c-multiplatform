/**
 * @file cupertino_status_bar.c
 * @brief iOS Safe Area & Translucent Status Bar component implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_status_bar.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_status_bar_mock_get_insets_fail = 0;
static ui_error_t
mock_safe_area_manager_get_insets(struct ui_safe_area_manager *manager,
                                  struct ui_safe_area_insets *out_insets) {
  if (g_cupertino_status_bar_mock_get_insets_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_safe_area_manager_get_insets)(manager, out_insets);
}
#undef ui_safe_area_manager_get_insets
/** @cond */
#define ui_safe_area_manager_get_insets mock_safe_area_manager_get_insets
/** @endcond */
#endif

static void cupertino_status_bar_apply_preset(
    struct cupertino_status_bar *bar,
    enum cupertino_hardware_device_preset preset) {
  switch (preset) {
  case CUPERTINO_DEVICE_PRESET_DYNAMIC_ISLAND:
    bar->insets.top = 54.0f;
    bar->insets.bottom = 34.0f;
    bar->insets.left = 0.0f;
    bar->insets.right = 0.0f;
    break;
  case CUPERTINO_DEVICE_PRESET_NOTCH:
    bar->insets.top = 47.0f;
    bar->insets.bottom = 34.0f;
    bar->insets.left = 0.0f;
    bar->insets.right = 0.0f;
    break;
  case CUPERTINO_DEVICE_PRESET_IPAD:
    bar->insets.top = 24.0f;
    bar->insets.bottom = 20.0f;
    bar->insets.left = 0.0f;
    bar->insets.right = 0.0f;
    break;
  case CUPERTINO_DEVICE_PRESET_CLASSIC:
  default:
    bar->insets.top = 20.0f;
    bar->insets.bottom = 0.0f;
    bar->insets.left = 0.0f;
    bar->insets.right = 0.0f;
    break;
  }
}

ui_error_t
cupertino_status_bar_create(struct ui_engine *engine,
                            const struct cupertino_status_bar_descriptor *desc,
                            struct cupertino_status_bar **out_status_bar) {
  struct cupertino_status_bar *bar;

  if (!engine || !desc || !out_status_bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar = (struct cupertino_status_bar *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_status_bar));
  if (!bar) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(bar, 0, sizeof(*bar));
  bar->style = desc->style;
  cupertino_status_bar_apply_preset(bar, desc->preset);

  if (desc->initial_time_text && desc->initial_time_text[0] != '\0') {
#if defined(_MSC_VER)
    strncpy_s(bar->time_text, sizeof(bar->time_text), desc->initial_time_text,
              _TRUNCATE);
#else
    strncpy(bar->time_text, desc->initial_time_text,
            sizeof(bar->time_text) - 1);
    bar->time_text[sizeof(bar->time_text) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strncpy_s(bar->time_text, sizeof(bar->time_text), "9:41", _TRUNCATE);
#else
    strncpy(bar->time_text, "9:41", sizeof(bar->time_text) - 1);
    bar->time_text[sizeof(bar->time_text) - 1] = '\0';
#endif
  }

  bar->battery_level = (desc->initial_battery_level >= 0.0f &&
                        desc->initial_battery_level <= 1.0f)
                           ? desc->initial_battery_level
                           : 1.0f;
  bar->is_charging = desc->is_charging ? 1 : 0;
  bar->cellular_bars = (desc->cellular_bars >= 0 && desc->cellular_bars <= 4)
                           ? desc->cellular_bars
                           : 4;
  bar->wifi_bars =
      (desc->wifi_bars >= 0 && desc->wifi_bars <= 3) ? desc->wifi_bars : 3;
  bar->is_hidden = 0;
  bar->manager = NULL;

  *out_status_bar = bar;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_status_bar_destroy(struct cupertino_status_bar *bar) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(bar);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_set_style(struct cupertino_status_bar *bar,
                               enum cupertino_status_bar_style style) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->style = style;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_style(const struct cupertino_status_bar *bar,
                               enum cupertino_status_bar_style *out_style) {
  if (!bar || !out_style) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_style = bar->style;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_apply_insets(struct cupertino_status_bar *bar,
                                  const struct ui_safe_area_insets *insets) {
  if (!bar || !insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->insets = *insets;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_insets(const struct cupertino_status_bar *bar,
                                struct ui_safe_area_insets *out_insets) {
  if (!bar || !out_insets) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_insets = bar->insets;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_status_bar_set_safe_area_manager(
    struct cupertino_status_bar *bar, struct ui_safe_area_manager *manager) {
  struct ui_safe_area_insets current_insets;
  ui_error_t rc;

  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->manager = manager;
  if (manager) {
    rc = ui_safe_area_manager_get_insets(manager, &current_insets);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    bar->insets = current_insets;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_status_bar_set_time_text(struct cupertino_status_bar *bar,
                                              const char *time_text) {
  if (!bar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (time_text && time_text[0] != '\0') {
#if defined(_MSC_VER)
    strncpy_s(bar->time_text, sizeof(bar->time_text), time_text, _TRUNCATE);
#else
    strncpy(bar->time_text, time_text, sizeof(bar->time_text) - 1);
    bar->time_text[sizeof(bar->time_text) - 1] = '\0';
#endif
  } else {
#if defined(_MSC_VER)
    strncpy_s(bar->time_text, sizeof(bar->time_text), "9:41", _TRUNCATE);
#else
    strncpy(bar->time_text, "9:41", sizeof(bar->time_text) - 1);
    bar->time_text[sizeof(bar->time_text) - 1] = '\0';
#endif
  }

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_time_text(const struct cupertino_status_bar *bar,
                                   const char **out_time_text) {
  if (!bar || !out_time_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_time_text = bar->time_text;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_status_bar_set_battery(struct cupertino_status_bar *bar,
                                            float level, int is_charging) {
  if (!bar || level < 0.0f || level > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->battery_level = level;
  bar->is_charging = is_charging ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_battery(const struct cupertino_status_bar *bar,
                                 float *out_level, int *out_is_charging) {
  if (!bar || !out_level || !out_is_charging) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_level = bar->battery_level;
  *out_is_charging = bar->is_charging;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_status_bar_set_signal(struct cupertino_status_bar *bar,
                                           int cellular_bars, int wifi_bars) {
  if (!bar || cellular_bars < 0 || cellular_bars > 4 || wifi_bars < 0 ||
      wifi_bars > 3) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  bar->cellular_bars = cellular_bars;
  bar->wifi_bars = wifi_bars;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_signal(const struct cupertino_status_bar *bar,
                                int *out_cellular_bars, int *out_wifi_bars) {
  if (!bar || !out_cellular_bars || !out_wifi_bars) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_cellular_bars = bar->cellular_bars;
  *out_wifi_bars = bar->wifi_bars;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_status_bar_get_bounds(const struct cupertino_status_bar *bar,
                                float screen_w, float *out_w, float *out_h) {
  if (!bar || screen_w <= 0.0f || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = screen_w;
  *out_h = bar->insets.top > 0.0f ? bar->insets.top : 20.0f;
  return UI_ERROR_NONE;
}
