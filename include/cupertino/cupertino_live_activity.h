/**
 * @file cupertino_live_activity.h
 * @brief Cupertino Live Activity & StandBy Lock Screen Cards conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_LIVE_ACTIVITY_H
#define CUPERTINO_CUPERTINO_LIVE_ACTIVITY_H

/* clang-format off */
#include "ui_card_base.h"
#include "ui_color_space.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;
struct ui_component;

/**
 * @enum cupertino_live_activity_layout
 * @brief Lock screen card layout geometry modes.
 */
enum cupertino_live_activity_layout {
  CUPERTINO_LIVE_ACTIVITY_LAYOUT_COMPACT =
      0, /**< Standard Lock Screen glanceable card (360x88pt). */
  CUPERTINO_LIVE_ACTIVITY_LAYOUT_MINIMAL, /**< Minimal auxiliary circular/square
                                             tile (88x88pt). */
  CUPERTINO_LIVE_ACTIVITY_LAYOUT_EXPANDED /**< Expanded multi-metric rich card
                                             (360x160pt). */
};

/**
 * @struct cupertino_live_activity_descriptor
 * @brief Configuration descriptor for Live Activity & StandBy cards.
 */
struct cupertino_live_activity_descriptor {
  enum cupertino_live_activity_layout layout; /**< Initial layout geometry. */
  const char *title;                          /**< Primary activity title. */
  const char *subtitle;     /**< Subtitle or status summary. */
  float duration_seconds;   /**< Total timer duration in seconds. */
  float elapsed_seconds;    /**< Initial elapsed timer seconds. */
  int standby_mode_enabled; /**< 1 if StandBy mode is active. */
  float ambient_lux;        /**< Ambient light level in lux (StandBy night mode
                               triggers if < 1.0). */
};

/**
 * @struct cupertino_live_activity
 * @brief Instance managing Live Activity and StandBy lock screen widget card.
 */
struct cupertino_live_activity {
  enum cupertino_live_activity_layout layout; /**< Card layout mode. */
  char title[64];                             /**< Activity headline. */
  char subtitle[128];                         /**< Activity description. */
  float width;                 /**< Current card width in points. */
  float height;                /**< Current card height in points. */
  float corner_radius;         /**< Continuous G2 corner radius (e.g. 24pt). */
  float duration_seconds;      /**< Timer target total duration. */
  float elapsed_seconds;       /**< Current high-frequency elapsed time. */
  int is_timer_active;         /**< 1 if live timer is actively advancing. */
  int standby_mode_enabled;    /**< 1 if StandBy night adaptation enabled. */
  float ambient_lux;           /**< Ambient light reading in lux. */
  int is_night_mode;           /**< 1 if deep-red monochrome palette active. */
  ui_color_t background_color; /**< Card surface color. */
  ui_color_t primary_text_color;   /**< Title and timer text color. */
  ui_color_t secondary_text_color; /**< Subtitle and metric text color. */
  ui_color_t accent_color;         /**< Progress gauge / icon tint. */
  struct ui_card_base *base_card;  /**< Wrapped CDK base card primitive. */
};

/**
 * @brief Creates a new Cupertino Live Activity & StandBy card instance.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_activity Pointer to receive allocated Live Activity card.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_live_activity_create(
    struct ui_engine *engine,
    const struct cupertino_live_activity_descriptor *desc,
    struct cupertino_live_activity **out_activity);

/**
 * @brief Destroys a Cupertino Live Activity instance.
 *
 * @param activity Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_destroy(struct cupertino_live_activity *activity);

/**
 * @brief Sets the layout geometry mode of the Live Activity card.
 *
 * @param activity Target activity.
 * @param layout Target layout (Compact, Minimal, Expanded).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_set_layout(struct cupertino_live_activity *activity,
                                   enum cupertino_live_activity_layout layout);

/**
 * @brief Updates timer elapsed seconds with sub-second resolution.
 *
 * @param activity Target activity.
 * @param delta_seconds Seconds elapsed since last update (e.g. 0.016s).
 * @param out_progress Pointer to receive normalized progress in [0.0, 1.0].
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_tick_timer(struct cupertino_live_activity *activity,
                                   float delta_seconds, float *out_progress);

/**
 * @brief Sets the ambient light level and updates StandBy night mode palette.
 *
 * When ambient light drops below 1.0 lux in StandBy mode, activates deep-red
 * monochrome palette to protect night vision.
 *
 * @param activity Target activity.
 * @param ambient_lux Ambient light reading in lux.
 * @param out_is_night_mode Pointer to receive 1 if night mode activated, 0
 * otherwise.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_set_ambient_light(
    struct cupertino_live_activity *activity, float ambient_lux,
    int *out_is_night_mode);

/**
 * @brief Sets StandBy mode enabled state.
 *
 * @param activity Target activity.
 * @param enabled 1 to enable StandBy mode, 0 to disable.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_set_standby_mode(
    struct cupertino_live_activity *activity, int enabled);

/**
 * @brief Formats sub-second live timer string for display.
 *
 * @param activity Target activity.
 * @param buffer Output text buffer.
 * @param buffer_size Capacity of buffer in bytes.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_live_activity_get_timer_string(
    const struct cupertino_live_activity *activity, char *buffer,
    size_t buffer_size);

/**
 * @brief Retrieves underlying CDK card component primitive.
 *
 * @param activity Target activity.
 * @param out_card Pointer to receive struct ui_card_base *.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_live_activity_get_card_base(
    struct cupertino_live_activity *activity, struct ui_card_base **out_card);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_LIVE_ACTIVITY_H */
