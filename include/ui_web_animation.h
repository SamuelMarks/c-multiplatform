/**
 * @file ui_web_animation.h
 * @brief Web Animation API implementation for UI effects.
 */
#ifndef UI_WEB_ANIMATION_H
#define UI_WEB_ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_dom_node.h"
/* clang-format on */

/**
 * @enum ui_web_animation_play_state
 * @brief Play state for Web Animations.
 */
enum ui_web_animation_play_state {
  /** @brief Animation is idle. */
  UI_WEB_ANIMATION_PLAY_STATE_IDLE,
  /** @brief Animation is currently running. */
  UI_WEB_ANIMATION_PLAY_STATE_RUNNING,
  /** @brief Animation is paused. */
  UI_WEB_ANIMATION_PLAY_STATE_PAUSED,
  /** @brief Animation has finished playing. */
  UI_WEB_ANIMATION_PLAY_STATE_FINISHED
};

/**
 * @enum ui_web_animation_fill_mode
 * @brief Fill mode for Web Animations.
 */
enum ui_web_animation_fill_mode {
  /** @brief No fill mode. */
  UI_WEB_ANIMATION_FILL_MODE_NONE,
  /** @brief Forwards fill mode. */
  UI_WEB_ANIMATION_FILL_MODE_FORWARDS,
  /** @brief Backwards fill mode. */
  UI_WEB_ANIMATION_FILL_MODE_BACKWARDS,
  /** @brief Both forwards and backwards fill mode. */
  UI_WEB_ANIMATION_FILL_MODE_BOTH,
  /** @brief Auto fill mode. */
  UI_WEB_ANIMATION_FILL_MODE_AUTO
};

/**
 * @enum ui_web_animation_direction
 * @brief Direction for Web Animations.
 */
enum ui_web_animation_direction {
  /** @brief Normal direction. */
  UI_WEB_ANIMATION_DIRECTION_NORMAL,
  /** @brief Reverse direction. */
  UI_WEB_ANIMATION_DIRECTION_REVERSE,
  /** @brief Alternate direction. */
  UI_WEB_ANIMATION_DIRECTION_ALTERNATE,
  /** @brief Alternate reverse direction. */
  UI_WEB_ANIMATION_DIRECTION_ALTERNATE_REVERSE
};

/**
 * @struct ui_web_animation_timing
 * @brief Timing options for a Web Animation Effect.
 */
struct ui_web_animation_timing {
  /** @brief Delay in milliseconds. */
  double delay;
  /** @brief End delay in milliseconds. */
  double end_delay;
  /** @brief Fill mode. */
  enum ui_web_animation_fill_mode fill_mode;
  /** @brief Iteration start offset. */
  double iteration_start;
  /** @brief Number of iterations, -1.0 for infinity. */
  double iterations;
  /** @brief Duration in milliseconds. */
  double duration;
  /** @brief Playback direction. */
  enum ui_web_animation_direction direction;
};

/**
 * @struct ui_web_animation_keyframe_property
 * @brief A single keyframe property-value pair.
 */
struct ui_web_animation_keyframe_property {
  /** @brief The CSS property name. */
  char *property_name;
  /** @brief The value as a string. */
  char *value;
  /** @brief Pointer to the next property. */
  struct ui_web_animation_keyframe_property *next;
};

/**
 * @struct ui_web_animation_keyframe
 * @brief A keyframe in a Web Animation Effect.
 */
struct ui_web_animation_keyframe {
  /** @brief 0.0 to 1.0, or -1.0 for null/auto. */
  double offset;
  /** @brief List of properties for this keyframe. */
  struct ui_web_animation_keyframe_property *properties;
  /** @brief Pointer to the next keyframe. */
  struct ui_web_animation_keyframe *next;
};

/**
 * @struct ui_web_animation_effect
 * @brief An opaque effect (like KeyframeEffect) for a Web Animation.
 */
struct ui_web_animation_effect;

/**
 * @enum ui_web_animation_timeline_type
 * @brief Types of Web Animation timelines.
 */
enum ui_web_animation_timeline_type {
  /** @brief Document timeline. */
  UI_WEB_ANIMATION_TIMELINE_DOCUMENT,
  /** @brief Scroll timeline. */
  UI_WEB_ANIMATION_TIMELINE_SCROLL,
  /** @brief View timeline. */
  UI_WEB_ANIMATION_TIMELINE_VIEW,
  /** @brief Pointer timeline. */
  UI_WEB_ANIMATION_TIMELINE_POINTER
};

/**
 * @enum ui_web_animation_scroll_axis
 * @brief Axis for scroll and view timelines.
 */
enum ui_web_animation_scroll_axis {
  /** @brief Block scroll axis. */
  UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK,
  /** @brief Inline scroll axis. */
  UI_WEB_ANIMATION_SCROLL_AXIS_INLINE,
  /** @brief Horizontal scroll axis. */
  UI_WEB_ANIMATION_SCROLL_AXIS_X,
  /** @brief Vertical scroll axis. */
  UI_WEB_ANIMATION_SCROLL_AXIS_Y
};

/**
 * @enum ui_web_animation_pointer_axis
 * @brief Axis for pointer timelines.
 */
enum ui_web_animation_pointer_axis {
  /** @brief Horizontal pointer axis. */
  UI_WEB_ANIMATION_POINTER_AXIS_X,
  /** @brief Vertical pointer axis. */
  UI_WEB_ANIMATION_POINTER_AXIS_Y
};

/**
 * @struct ui_web_animation_timeline
 * @brief An opaque timeline for a Web Animation.
 */
struct ui_web_animation_timeline;

/**
 * @struct ui_web_animation
 * @brief An opaque Web Animation instance.
 */
struct ui_web_animation;

/**
 * @brief Creates a Keyframe Effect.
 *
 * @param target The target DOM node.
 * @param keyframes Linked list of keyframes (takes ownership if successful).
 * @param timing The timing options.
 * @param out_effect Pointer to receive the new effect.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_effect_create_keyframe_effect(
    struct ui_dom_node *target, struct ui_web_animation_keyframe *keyframes,
    const struct ui_web_animation_timing *timing,
    struct ui_web_animation_effect **out_effect);

/**
 * @brief Destroys an effect.
 *
 * @param effect The effect to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_effect_destroy(struct ui_web_animation_effect *effect);

/**
 * @brief Creates a Document Timeline.
 *
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_timeline_create_document_timeline(
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Creates a Scroll Timeline.
 *
 * @param source The scrolling element.
 * @param axis The scroll axis.
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_timeline_create_scroll_timeline(
    struct ui_dom_node *source, enum ui_web_animation_scroll_axis axis,
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Creates a View Timeline.
 *
 * @param subject The element intersecting the viewport.
 * @param axis The scroll axis.
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_timeline_create_view_timeline(
    struct ui_dom_node *subject, enum ui_web_animation_scroll_axis axis,
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Creates a Pointer Timeline.
 *
 * @param target The element tracking pointer movements.
 * @param axis The pointer axis.
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_timeline_create_pointer_timeline(
    struct ui_dom_node *target, enum ui_web_animation_pointer_axis axis,
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Explicitly sets the current time of a timeline (useful for
 * scroll/view/pointer timelines).
 *
 * @param timeline The timeline.
 * @param time The current progress time.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_timeline_set_current_time(
    struct ui_web_animation_timeline *timeline, double time);

/**
 * @brief Destroys a timeline.
 *
 * @param timeline The timeline to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_timeline_destroy(struct ui_web_animation_timeline *timeline);

/**
 * @brief Creates a Web Animation.
 *
 * @param effect The effect to run (takes ownership).
 * @param timeline The timeline to use (takes ownership).
 * @param out_animation Pointer to receive the new animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_create(struct ui_web_animation_effect *effect,
                                   struct ui_web_animation_timeline *timeline,
                                   struct ui_web_animation **out_animation);

/**
 * @brief Destroys a Web Animation.
 *
 * @param animation The animation to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_destroy(struct ui_web_animation *animation);

/**
 * @brief Plays the animation.
 *
 * @param animation The animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_play(struct ui_web_animation *animation);

/**
 * @brief Pauses the animation.
 *
 * @param animation The animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_pause(struct ui_web_animation *animation);

/**
 * @brief Reverses the animation playback direction.
 *
 * @param animation The animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_reverse(struct ui_web_animation *animation);

/**
 * @brief Cancels the animation.
 *
 * @param animation The animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_cancel(struct ui_web_animation *animation);

/**
 * @brief Finishes the animation.
 *
 * @param animation The animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_finish(struct ui_web_animation *animation);

/**
 * @brief Gets the current play state.
 *
 * @param animation The animation.
 * @param out_state Pointer to receive the state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_get_play_state(const struct ui_web_animation *animation,
                                enum ui_web_animation_play_state *out_state);

/**
 * @brief Gets the current time of the animation.
 *
 * @param animation The animation.
 * @param out_time Pointer to receive the current time.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_get_current_time(const struct ui_web_animation *animation,
                                  double *out_time);

/**
 * @brief Sets the current time of the animation.
 *
 * @param animation The animation.
 * @param time The new current time.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_set_current_time(struct ui_web_animation *animation,
                                             double time);

/**
 * @brief Gets the playback rate.
 *
 * @param animation The animation.
 * @param out_rate Pointer to receive the playback rate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_get_playback_rate(const struct ui_web_animation *animation,
                                   double *out_rate);

/**
 * @brief Sets the playback rate.
 *
 * @param animation The animation.
 * @param rate The new playback rate.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_web_animation_set_playback_rate(struct ui_web_animation *animation,
                                   double rate);

/**
 * @brief Advances the timeline for the given animation.
 *
 * @param animation The animation.
 * @param delta_time_ms Delta time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_web_animation_tick(struct ui_web_animation *animation,
                                 double delta_time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WEB_ANIMATION_H */
