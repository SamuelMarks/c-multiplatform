/**
 * @file cupertino_fullscreen_dialog.h
 * @brief Cupertino Fullscreen Dialog Transition
 * (CupertinoFullscreenDialogTransition & CupertinoPageRoute) conforming to
 * Apple HIG.
 */

#ifndef CUPERTINO_CUPERTINO_FULLSCREEN_DIALOG_H
#define CUPERTINO_CUPERTINO_FULLSCREEN_DIALOG_H

/* clang-format off */
#include "cupertino/cupertino_spring.h"
#include "ui_error.h"
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

struct ui_engine;
struct ui_component;

/**
 * @enum cupertino_dialog_nav_action
 * @brief Navigation bar action button type for fullscreen dialog presentation.
 */
enum cupertino_dialog_nav_action {
  CUPERTINO_DIALOG_NAV_ACTION_CANCEL =
      0, /**< Standard "Cancel" leading dismiss button. */
  CUPERTINO_DIALOG_NAV_ACTION_DONE,  /**< Standard "Done" trailing confirm
                                        button. */
  CUPERTINO_DIALOG_NAV_ACTION_CUSTOM /**< Custom label action button. */
};

/**
 * @enum cupertino_dialog_transition_state
 * @brief Current lifecycle state of fullscreen dialog transition.
 */
enum cupertino_dialog_transition_state {
  CUPERTINO_DIALOG_STATE_DISMISSED = 0, /**< Fully off-screen at bottom. */
  CUPERTINO_DIALOG_STATE_PRESENTING,    /**< Animating upward from bottom screen
                                           boundary. */
  CUPERTINO_DIALOG_STATE_PRESENTED,     /**< Fully presented fullscreen. */
  CUPERTINO_DIALOG_STATE_DRAGGING,  /**< User interactively dragging downward to
                                       dismiss. */
  CUPERTINO_DIALOG_STATE_DISMISSING /**< Animating downward to bottom screen
                                       boundary. */
};

/**
 * @struct cupertino_fullscreen_dialog_descriptor
 * @brief Configuration descriptor for Cupertino fullscreen dialog transition.
 */
struct cupertino_fullscreen_dialog_descriptor {
  float screen_height; /**< Total viewport height in points. */
  enum cupertino_dialog_nav_action action_type; /**< Nav action button type. */
  const char *action_title;     /**< Custom title if action_type is CUSTOM. */
  int swipe_to_dismiss_enabled; /**< Non-zero to enable downward
                                   swipe-to-dismiss. */
  float dismiss_velocity_threshold; /**< Points/sec velocity threshold to
                                       trigger dismiss (e.g. 500.0f). */
  float dismiss_distance_threshold; /**< Normalized displacement (0.0 to 1.0) to
                                       trigger dismiss (e.g. 0.35f). */
};

/**
 * @struct cupertino_fullscreen_dialog
 * @brief Instance managing fullscreen dialog presentation transition state.
 */
struct cupertino_fullscreen_dialog {
  float screen_height; /**< Height of screen in points. */
  enum cupertino_dialog_transition_state
      state; /**< Presentation transition state. */
  enum cupertino_dialog_nav_action
      action_type;              /**< Navigation bar button style. */
  char action_title[64];        /**< Button label text. */
  int swipe_to_dismiss_enabled; /**< Swipe to dismiss toggle. */
  float
      dismiss_velocity_threshold; /**< Minimum downward velocity for dismiss. */
  float dismiss_distance_threshold; /**< Fractional threshold for dismiss. */
  float current_progress; /**< 0.0f (dismissed) to 1.0f (fully presented). */
  float drag_offset_y;    /**< Current downward touch drag displacement. */
  float touch_velocity_y; /**< Measured finger release velocity (pt/s). */
  struct cupertino_spring_config
      spring_cfg;             /**< CASpringAnimation configuration. */
  float anim_elapsed_s;       /**< Elapsed animation time in seconds. */
  float parent_scale;         /**< Presentation scale factor of background view
                                 (e.g. 1.0 -> 0.92). */
  float parent_corner_radius; /**< Continuous corner radius applied to parent
                                 view (e.g. 0 -> 12pt). */
  float parent_dim_alpha; /**< Dimming backdrop alpha for parent view (0.0 ->
                             0.25). */
  struct ui_component
      *dialog_content; /**< Attached dialog content view component. */
  struct ui_component *parent_content; /**< Underlying parent view component. */
  int dismiss_invoked_count; /**< Counter for dismiss event emissions. */
};

/**
 * @brief Creates a new Cupertino Fullscreen Dialog transition coordinator.
 *
 * @param engine Pointer to ui_engine.
 * @param desc Configuration descriptor.
 * @param out_dialog Pointer to receive created dialog coordinator.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_fullscreen_dialog_create(
    struct ui_engine *engine,
    const struct cupertino_fullscreen_dialog_descriptor *desc,
    struct cupertino_fullscreen_dialog **out_dialog);

/**
 * @brief Destroys a Cupertino Fullscreen Dialog coordinator.
 *
 * @param dialog Coordinator instance to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_destroy(struct cupertino_fullscreen_dialog *dialog);

/**
 * @brief Presents the fullscreen dialog modally from bottom of screen.
 *
 * @param dialog Coordinator instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_present(struct cupertino_fullscreen_dialog *dialog);

/**
 * @brief Dismisses the fullscreen dialog sliding downwards.
 *
 * @param dialog Coordinator instance.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_dismiss(struct cupertino_fullscreen_dialog *dialog);

/**
 * @brief Advances spring physics animation state by time step delta_sec.
 *
 * @param dialog Coordinator instance.
 * @param delta_sec Time elapsed since last frame in seconds.
 * @param out_is_settled Pointer to receive 1 if animation completed, 0 if still
 * animating.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_tick(struct cupertino_fullscreen_dialog *dialog,
                                 float delta_sec, int *out_is_settled);

/**
 * @brief Begins interactive downward swipe-to-dismiss drag.
 *
 * @param dialog Coordinator instance.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_fullscreen_dialog_drag_start(
    struct cupertino_fullscreen_dialog *dialog);

/**
 * @brief Updates interactive downward swipe displacement.
 *
 * @param dialog Coordinator instance.
 * @param delta_y Downward touch offset displacement in points.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_drag_update(
    struct cupertino_fullscreen_dialog *dialog, float delta_y);

/**
 * @brief Completes interactive drag with touch release velocity.
 *
 * @param dialog Coordinator instance.
 * @param velocity_y Measured downward velocity in points/second.
 * @param out_will_dismiss Pointer to receive 1 if dialog will dismiss, 0 if it
 * springs back.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_drag_end(struct cupertino_fullscreen_dialog *dialog,
                                     float velocity_y, int *out_will_dismiss);

/**
 * @brief Retrieves current vertical translation offset of modal dialog in
 * points.
 *
 * @param dialog Coordinator instance.
 * @param out_translation_y Pointer to receive vertical translation in points
 * from top.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_get_translation_y(
    const struct cupertino_fullscreen_dialog *dialog, float *out_translation_y);

/**
 * @brief Retrieves visual transform properties applied to presenting parent
 * view.
 *
 * @param dialog Coordinator instance.
 * @param out_scale Pointer to receive parent scale (0.92f - 1.0f).
 * @param out_corner_radius Pointer to receive rounded corner radius (0.0f
 * - 12.0f pt).
 * @param out_dim_alpha Pointer to receive background dimming alpha (0.0f -
 * 0.25f).
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_get_parent_transform(
    const struct cupertino_fullscreen_dialog *dialog, float *out_scale,
    float *out_corner_radius, float *out_dim_alpha);

/**
 * @brief Retrieves current navigation action label button text.
 *
 * @param dialog Coordinator instance.
 * @param out_title Buffer to receive button title text.
 * @param title_size Size of out_title buffer.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t
cupertino_fullscreen_dialog_get_nav_action_title(
    const struct cupertino_fullscreen_dialog *dialog, char *out_title,
    size_t title_size);

/**
 * @brief Sets dialog content and parent content views.
 *
 * @param dialog Coordinator instance.
 * @param dialog_content Modal dialog root component.
 * @param parent_content Presenting parent view root component.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
extern C_MULTIPLATFORM_EXPORT ui_error_t cupertino_fullscreen_dialog_set_views(
    struct cupertino_fullscreen_dialog *dialog,
    struct ui_component *dialog_content, struct ui_component *parent_content);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CUPERTINO_CUPERTINO_FULLSCREEN_DIALOG_H */
