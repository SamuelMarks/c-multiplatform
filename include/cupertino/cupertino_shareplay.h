/**
 * @file cupertino_shareplay.h
 * @brief Cupertino SharePlay Group Activities Control conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_SHAREPLAY_H
#define CUPERTINO_CUPERTINO_SHAREPLAY_H

/* clang-format off */
#include "ui_avatar_group_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_SHAREPLAY_PILL_HEIGHT 36.0f
#define CUPERTINO_SHAREPLAY_BASE_WIDTH 120.0f
#define CUPERTINO_SHAREPLAY_WAVE_COUNT 3

/**
 * @enum cupertino_shareplay_action
 * @brief Action options surfaced when tapping the SharePlay activity pill.
 */
enum cupertino_shareplay_action {
  CUPERTINO_SHAREPLAY_ACTION_NONE = 0, /**< No action. */
  CUPERTINO_SHAREPLAY_ACTION_LEAVE,    /**< 'Leave SharePlay' (user leaves
                                          session). */
  CUPERTINO_SHAREPLAY_ACTION_END_FOR_EVERYONE /**< 'End for Everyone' (host
                                                 terminates session). */
};

/**
 * @struct cupertino_shareplay_descriptor
 * @brief Configuration descriptor for SharePlay control.
 */
struct cupertino_shareplay_descriptor {
  int initial_participant_count; /**< Number of connected FaceTime peers. */
  int is_session_active;         /**< 1 if active, 0 if idle. */
};

/**
 * @struct cupertino_shareplay_control
 * @brief SharePlay indicator pill instance wrapping ui_avatar_group_base.
 */
struct cupertino_shareplay_control {
  struct ui_avatar_group_base *avatar_group;
  int is_session_active;
  int participant_count;
  float wave_time;
  float wave_heights[CUPERTINO_SHAREPLAY_WAVE_COUNT];
  enum cupertino_shareplay_action last_action;
  float width;
  float height;
};

/**
 * @brief Creates a new Cupertino SharePlay control.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Pointer to configuration descriptor.
 * @param out_control Pointer to receive newly created control instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_create(
    struct ui_engine *engine, const struct cupertino_shareplay_descriptor *desc,
    struct cupertino_shareplay_control **out_control);

/**
 * @brief Destroys a SharePlay control instance.
 *
 * @param control Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_shareplay_destroy(struct cupertino_shareplay_control *control);

/**
 * @brief Sets session active state.
 *
 * @param control Target control.
 * @param is_active 1 for active stream, 0 for idle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_set_session_active(
    struct cupertino_shareplay_control *control, int is_active);

/**
 * @brief Checks if session is active.
 *
 * @param control Target control.
 * @param out_active Pointer to receive active flag (1 or 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_is_session_active(
    const struct cupertino_shareplay_control *control, int *out_active);

/**
 * @brief Updates count of connected participants.
 *
 * @param control Target control.
 * @param count Participant count (>= 0).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_shareplay_set_participant_count(
    struct cupertino_shareplay_control *control, int count);

/**
 * @brief Gets count of connected participants.
 *
 * @param control Target control.
 * @param out_count Pointer to receive count.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_shareplay_get_participant_count(
    const struct cupertino_shareplay_control *control, int *out_count);

/**
 * @brief Advances green waveform animation frame.
 *
 * @param control Target control.
 * @param delta_ms Elapsed time in milliseconds.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_tick(
    struct cupertino_shareplay_control *control, float delta_ms);

/**
 * @brief Retrieves animated heights for the 3 green audio/video stream waveform
 * bars.
 *
 * @param control Target control.
 * @param out_heights Array of 3 floats to receive waveform heights.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_get_wave_heights(
    const struct cupertino_shareplay_control *control, float out_heights[3]);

/**
 * @brief Triggers a session management action.
 *
 * @param control Target control.
 * @param action Action identifier.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_shareplay_trigger_action(struct cupertino_shareplay_control *control,
                                   enum cupertino_shareplay_action action);

/**
 * @brief Gets dimensions of the SharePlay pill container.
 *
 * @param control Target control.
 * @param out_width Pointer to receive width.
 * @param out_height Pointer to receive height.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_get_dimensions(
    const struct cupertino_shareplay_control *control, float *out_width,
    float *out_height);

/**
 * @brief Retrieves underlying CDK avatar group base.
 *
 * @param control Target control.
 * @param out_avatar_group Pointer to receive ui_avatar_group_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_shareplay_get_avatar_group(
    struct cupertino_shareplay_control *control,
    struct ui_avatar_group_base **out_avatar_group);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_SHAREPLAY_H */
