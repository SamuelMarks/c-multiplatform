/**
 * @file cupertino_live_activity.c
 * @brief Cupertino Live Activity & StandBy Lock Screen Cards implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_live_activity.h"
#include "cupertino/cupertino_tokens.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_live_activity_mock_card_create_fail = 0;
int g_cupertino_live_activity_mock_card_set_title_fail = 0;
int g_cupertino_live_activity_mock_card_set_subtitle_fail = 0;
int g_cupertino_live_activity_mock_card_destroy_fail = 0;
int g_cupertino_live_activity_mock_apply_geometry_fail = 0;
int g_cupertino_live_activity_mock_update_palette_fail = 0;
struct ui_card_base *g_cupertino_live_activity_last_created_card = NULL;

static ui_error_t mock_card_base_create(struct ui_card_base **out_card) {
  ui_error_t rc;
  if (g_cupertino_live_activity_mock_card_create_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  rc = ui_card_base_create(out_card);
  if (rc == UI_ERROR_NONE) {
    g_cupertino_live_activity_last_created_card = *out_card;
  }
  return rc;
}
#undef ui_card_base_create
/** @cond */
#define ui_card_base_create mock_card_base_create
/** @endcond */

static ui_error_t mock_card_base_set_title(struct ui_card_base *card,
                                           const char *title) {
  if (g_cupertino_live_activity_mock_card_set_title_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_card_base_set_title(card, title);
}
#undef ui_card_base_set_title
/** @cond */
#define ui_card_base_set_title mock_card_base_set_title
/** @endcond */

static ui_error_t mock_card_base_set_subtitle(struct ui_card_base *card,
                                              const char *subtitle) {
  if (g_cupertino_live_activity_mock_card_set_subtitle_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_card_base_set_subtitle(card, subtitle);
}
#undef ui_card_base_set_subtitle
/** @cond */
#define ui_card_base_set_subtitle mock_card_base_set_subtitle
/** @endcond */

static ui_error_t mock_card_base_destroy(struct ui_card_base *card) {
  if (g_cupertino_live_activity_mock_card_destroy_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_card_base_destroy)(card);
}
#undef ui_card_base_destroy
/** @cond */
#define ui_card_base_destroy mock_card_base_destroy
/** @endcond */
#endif

#define CUPERTINO_CARD_COMPACT_WIDTH 360.0f
#define CUPERTINO_CARD_COMPACT_HEIGHT 88.0f
#define CUPERTINO_CARD_MINIMAL_WIDTH 88.0f
#define CUPERTINO_CARD_MINIMAL_HEIGHT 88.0f
#define CUPERTINO_CARD_EXPANDED_WIDTH 360.0f
#define CUPERTINO_CARD_EXPANDED_HEIGHT 160.0f

static ui_error_t cupertino_live_activity_update_palette(
    struct cupertino_live_activity *activity) {
  if (!activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_live_activity_mock_update_palette_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (activity->standby_mode_enabled && activity->ambient_lux < 1.0f) {
    activity->is_night_mode = 1;
    /* StandBy Deep-Red Night Mode */
    activity->background_color = UI_COLOR_ARGB(245, 15, 0, 0);
    activity->primary_text_color = UI_COLOR_ARGB(255, 255, 45, 35);
    activity->secondary_text_color = UI_COLOR_ARGB(200, 180, 20, 20);
    activity->accent_color = UI_COLOR_ARGB(255, 255, 59, 48);
  } else {
    activity->is_night_mode = 0;
    /* Standard iOS Lock Screen Translucent Dark Glass */
    activity->background_color = UI_COLOR_ARGB(220, 30, 30, 32);
    activity->primary_text_color = UI_COLOR_ARGB(255, 255, 255, 255);
    activity->secondary_text_color = UI_COLOR_ARGB(153, 235, 235, 245);
    activity->accent_color = UI_COLOR_ARGB(255, 10, 132, 255);
  }

  return UI_ERROR_NONE;
}

static ui_error_t cupertino_live_activity_apply_geometry(
    struct cupertino_live_activity *activity) {
  if (!activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_live_activity_mock_apply_geometry_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  switch (activity->layout) {
  case CUPERTINO_LIVE_ACTIVITY_LAYOUT_MINIMAL:
    activity->width = CUPERTINO_CARD_MINIMAL_WIDTH;
    activity->height = CUPERTINO_CARD_MINIMAL_HEIGHT;
    activity->corner_radius = 22.0f;
    break;
  case CUPERTINO_LIVE_ACTIVITY_LAYOUT_EXPANDED:
    activity->width = CUPERTINO_CARD_EXPANDED_WIDTH;
    activity->height = CUPERTINO_CARD_EXPANDED_HEIGHT;
    activity->corner_radius = 28.0f;
    break;
  case CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT:
  default:
    activity->width = CUPERTINO_CARD_COMPACT_WIDTH;
    activity->height = CUPERTINO_CARD_COMPACT_HEIGHT;
    activity->corner_radius = 24.0f;
    break;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_live_activity_create(
    struct ui_engine *engine,
    const struct cupertino_live_activity_descriptor *desc,
    struct cupertino_live_activity **out_activity) {
  struct cupertino_live_activity *activity;
  ui_error_t rc;

  if (!engine || !desc || !out_activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  activity = (struct cupertino_live_activity *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_live_activity));
  if (!activity) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(activity, 0, sizeof(*activity));
  activity->layout = desc->layout;
  activity->duration_seconds =
      (desc->duration_seconds > 0.0f) ? desc->duration_seconds : 60.0f;
  activity->elapsed_seconds =
      (desc->elapsed_seconds >= 0.0f) ? desc->elapsed_seconds : 0.0f;
  activity->is_timer_active =
      (activity->elapsed_seconds < activity->duration_seconds) ? 1 : 0;
  activity->standby_mode_enabled = desc->standby_mode_enabled;
  activity->ambient_lux =
      (desc->ambient_lux >= 0.0f) ? desc->ambient_lux : 100.0f;

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(activity->title, sizeof(activity->title), desc->title,
              sizeof(activity->title) - 1);
#else
    strncpy(activity->title, desc->title, sizeof(activity->title) - 1);
    activity->title[sizeof(activity->title) - 1] = '\0';
#endif
  }

  if (desc->subtitle) {
#if defined(_MSC_VER)
    strncpy_s(activity->subtitle, sizeof(activity->subtitle), desc->subtitle,
              sizeof(activity->subtitle) - 1);
#else
    strncpy(activity->subtitle, desc->subtitle, sizeof(activity->subtitle) - 1);
    activity->subtitle[sizeof(activity->subtitle) - 1] = '\0';
#endif
  }

  rc = ui_card_base_create(&activity->base_card);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(activity);
    return rc;
  }

  rc = ui_card_base_set_title(activity->base_card, activity->title);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_card_base_destroy(activity->base_card);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(activity);
    return rc;
  }

  rc = ui_card_base_set_subtitle(activity->base_card, activity->subtitle);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_card_base_destroy(activity->base_card);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(activity);
    return rc;
  }

  rc = cupertino_live_activity_apply_geometry(activity);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_card_base_destroy(activity->base_card);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(activity);
    return rc;
  }

  rc = cupertino_live_activity_update_palette(activity);
  if (rc != UI_ERROR_NONE) {
    ui_error_t destroy_rc;
    destroy_rc = ui_card_base_destroy(activity->base_card);
    if (destroy_rc != UI_ERROR_NONE) {
      /* Keep original error */
    }
    C_MULTIPLATFORM_FREE(activity);
    return rc;
  }

  *out_activity = activity;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_live_activity_destroy(struct cupertino_live_activity *activity) {
  ui_error_t rc;

  if (!activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (activity->base_card) {
    rc = ui_card_base_destroy(activity->base_card);
    if (rc != UI_ERROR_NONE) {
      /* Continue freeing instance */
    }
  }

  C_MULTIPLATFORM_FREE(activity);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_live_activity_set_layout(struct cupertino_live_activity *activity,
                                   enum cupertino_live_activity_layout layout) {
  if (!activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  activity->layout = layout;
  return cupertino_live_activity_apply_geometry(activity);
}

ui_error_t
cupertino_live_activity_tick_timer(struct cupertino_live_activity *activity,
                                   float delta_seconds, float *out_progress) {
  float p;

  if (!activity || delta_seconds < 0.0f || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (activity->is_timer_active) {
    activity->elapsed_seconds += delta_seconds;
    if (activity->elapsed_seconds >= activity->duration_seconds) {
      activity->elapsed_seconds = activity->duration_seconds;
      activity->is_timer_active = 0;
    }
  }

  p = 0.0f;
  if (activity->duration_seconds > 0.0f) {
    p = activity->elapsed_seconds / activity->duration_seconds;
    if (p > 1.0f) {
      p = 1.0f;
    }
  }

  *out_progress = p;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_live_activity_set_ambient_light(
    struct cupertino_live_activity *activity, float ambient_lux,
    int *out_is_night_mode) {
  ui_error_t rc;

  if (!activity || ambient_lux < 0.0f || !out_is_night_mode) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  activity->ambient_lux = ambient_lux;
  rc = cupertino_live_activity_update_palette(activity);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_is_night_mode = activity->is_night_mode;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_live_activity_set_standby_mode(
    struct cupertino_live_activity *activity, int enabled) {
  if (!activity) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  activity->standby_mode_enabled = enabled;
  return cupertino_live_activity_update_palette(activity);
}

ui_error_t cupertino_live_activity_get_timer_string(
    const struct cupertino_live_activity *activity, char *buffer,
    size_t buffer_size) {
  unsigned int minutes;
  unsigned int seconds;
  unsigned int centiseconds;
  float rem;

  if (!activity || !buffer || buffer_size == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rem = activity->duration_seconds - activity->elapsed_seconds;
  if (rem < 0.0f) {
    rem = 0.0f;
  }

  minutes = (unsigned int)(rem / 60.0f);
  seconds = (unsigned int)(rem) % 60;
  centiseconds = (unsigned int)((rem - (float)((unsigned int)rem)) * 100.0f);

#if defined(_MSC_VER)
  sprintf_s(buffer, buffer_size, "%02u:%02u.%02u", minutes, seconds,
            centiseconds);
#else
  snprintf(buffer, buffer_size, "%02u:%02u.%02u", minutes, seconds,
           centiseconds);
#endif

  return UI_ERROR_NONE;
}

ui_error_t
cupertino_live_activity_get_card_base(struct cupertino_live_activity *activity,
                                      struct ui_card_base **out_card) {
  if (!activity || !out_card) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_card = activity->base_card;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
ui_error_t test_cupertino_live_activity_update_palette(
    struct cupertino_live_activity *activity);
ui_error_t test_cupertino_live_activity_apply_geometry(
    struct cupertino_live_activity *activity);

ui_error_t test_cupertino_live_activity_update_palette(
    struct cupertino_live_activity *activity) {
  return cupertino_live_activity_update_palette(activity);
}

ui_error_t test_cupertino_live_activity_apply_geometry(
    struct cupertino_live_activity *activity) {
  return cupertino_live_activity_apply_geometry(activity);
}
#endif
