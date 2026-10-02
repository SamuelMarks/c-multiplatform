/**
 * @file cupertino_refresh.h
 * @brief Cupertino Pull-To-Refresh Control conforming to Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_REFRESH_H
#define CUPERTINO_CUPERTINO_REFRESH_H

/* clang-format off */
#include "ui_pull_to_refresh_base.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;

#define CUPERTINO_REFRESH_DEFAULT_TRIGGER_DISTANCE 60.0f
#define CUPERTINO_REFRESH_DEFAULT_RESTING_DISTANCE 44.0f
#define CUPERTINO_REFRESH_SPOKE_COUNT 8

/**
 * @struct cupertino_refresh_descriptor
 * @brief Configuration descriptor for creating a Cupertino pull-to-refresh
 * control.
 */
struct cupertino_refresh_descriptor {
  float trigger_distance; /**< Threshold in pt to trigger refresh (~60pt). */
  float resting_distance; /**< Detent height in pt while refreshing (~44pt). */
  ui_pull_to_refresh_on_refresh_t
      on_refresh;  /**< Callback fired when threshold is passed. */
  void *user_data; /**< User data context for callback. */
};

/**
 * @struct cupertino_refresh
 * @brief Cupertino Pull-To-Refresh instance wrapping ui_pull_to_refresh_base.
 */
struct cupertino_refresh {
  struct ui_pull_to_refresh_base *base; /**< CDK pull-to-refresh primitive. */
  float trigger_distance;               /**< Threshold in points. */
  float resting_distance;      /**< Held open detent height in points. */
  float current_pull_distance; /**< Current overscroll distance. */
  float progress;              /**< Normalized progress [0.0, 1.0]. */
  int active_spoke;            /**< Currently highlighted spoke index (0-7). */
  enum ui_pull_to_refresh_state state; /**< State machine state. */
};

/**
 * @brief Creates a new Cupertino pull-to-refresh control.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_refresh Pointer to receive newly created refresh control.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_refresh_create(
    struct ui_engine *engine, const struct cupertino_refresh_descriptor *desc,
    struct cupertino_refresh **out_refresh);

/**
 * @brief Destroys a Cupertino pull-to-refresh control.
 *
 * @param refresh Instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_refresh_destroy(struct cupertino_refresh *refresh);

/**
 * @brief Feeds overscroll pull gesture distance into the refresh control.
 *
 * Updates dynamic spoke rotation and threshold detection.
 *
 * @param refresh Target refresh control.
 * @param pull_distance Positive distance pulled past scroll edge in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_refresh_handle_pull(
    struct cupertino_refresh *refresh, float pull_distance);

/**
 * @brief Handles gesture release; triggers refresh if threshold reached, or
 * snaps back.
 *
 * @param refresh Target refresh control.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_refresh_release(struct cupertino_refresh *refresh);

/**
 * @brief Signals async refresh task completion and triggers damped recoil.
 *
 * @param refresh Target refresh control.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_refresh_complete(struct cupertino_refresh *refresh);

/**
 * @brief Retrieves normalized pull progress in [0.0, 1.0].
 *
 * @param refresh Target refresh control.
 * @param out_progress Pointer to receive progress.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_refresh_get_progress(
    const struct cupertino_refresh *refresh, float *out_progress);

/**
 * @brief Retrieves currently highlighted spoke index in [0, 7].
 *
 * @param refresh Target refresh control.
 * @param out_spoke Pointer to receive active spoke index.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_refresh_get_active_spoke(
    const struct cupertino_refresh *refresh, int *out_spoke);

/**
 * @brief Retrieves current state machine state.
 *
 * @param refresh Target refresh control.
 * @param out_state Pointer to receive state enum.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_refresh_get_state(const struct cupertino_refresh *refresh,
                            enum ui_pull_to_refresh_state *out_state);

/**
 * @brief Retrieves underlying CDK pull-to-refresh base primitive.
 *
 * @param refresh Target refresh control.
 * @param out_base Pointer to receive ui_pull_to_refresh_base pointer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_refresh_get_base(struct cupertino_refresh *refresh,
                           struct ui_pull_to_refresh_base **out_base);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_REFRESH_H */
