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
 * @brief Play state for Web Animations.
 */
enum ui_web_animation_play_state {
  UI_WEB_ANIMATION_PLAY_STATE_IDLE,
  UI_WEB_ANIMATION_PLAY_STATE_RUNNING,
  UI_WEB_ANIMATION_PLAY_STATE_PAUSED,
  UI_WEB_ANIMATION_PLAY_STATE_FINISHED
};

/**
 * @brief Fill mode for Web Animations.
 */
enum ui_web_animation_fill_mode {
  UI_WEB_ANIMATION_FILL_MODE_NONE,
  UI_WEB_ANIMATION_FILL_MODE_FORWARDS,
  UI_WEB_ANIMATION_FILL_MODE_BACKWARDS,
  UI_WEB_ANIMATION_FILL_MODE_BOTH,
  UI_WEB_ANIMATION_FILL_MODE_AUTO
};

/**
 * @brief Direction for Web Animations.
 */
enum ui_web_animation_direction {
  UI_WEB_ANIMATION_DIRECTION_NORMAL,
  UI_WEB_ANIMATION_DIRECTION_REVERSE,
  UI_WEB_ANIMATION_DIRECTION_ALTERNATE,
  UI_WEB_ANIMATION_DIRECTION_ALTERNATE_REVERSE
};

/**
 * @brief Timing options for a Web Animation Effect.
 */
struct ui_web_animation_timing {
  double delay;     /**< Delay in milliseconds */
  double end_delay; /**< End delay in milliseconds */
  enum ui_web_animation_fill_mode fill_mode;
  double iteration_start;
  double iterations; /**< Number of iterations, -1.0 for infinity */
  double duration;   /**< Duration in milliseconds */
  enum ui_web_animation_direction direction;
};

/**
 * @brief A single keyframe property-value pair.
 */
struct ui_web_animation_keyframe_property {
  char *property_name;
  char *value;
  struct ui_web_animation_keyframe_property *next;
};

/**
 * @brief A keyframe in a Web Animation Effect.
 */
struct ui_web_animation_keyframe {
  double offset; /* 0.0 to 1.0, or -1.0 for null/auto */
  struct ui_web_animation_keyframe_property *properties;
  struct ui_web_animation_keyframe *next;
};

/**
 * @brief An effect (like KeyframeEffect) for a Web Animation.
 */
struct ui_web_animation_effect;

/**
 * @brief Types of Web Animation timelines.
 */
enum ui_web_animation_timeline_type {
  UI_WEB_ANIMATION_TIMELINE_DOCUMENT,
  UI_WEB_ANIMATION_TIMELINE_SCROLL,
  UI_WEB_ANIMATION_TIMELINE_VIEW,
  UI_WEB_ANIMATION_TIMELINE_POINTER
};

/**
 * @brief Axis for scroll and view timelines.
 */
enum ui_web_animation_scroll_axis {
  UI_WEB_ANIMATION_SCROLL_AXIS_BLOCK,
  UI_WEB_ANIMATION_SCROLL_AXIS_INLINE,
  UI_WEB_ANIMATION_SCROLL_AXIS_X,
  UI_WEB_ANIMATION_SCROLL_AXIS_Y
};

/**
 * @brief Axis for pointer timelines.
 */
enum ui_web_animation_pointer_axis {
  UI_WEB_ANIMATION_POINTER_AXIS_X,
  UI_WEB_ANIMATION_POINTER_AXIS_Y
};

/**
 * @brief A timeline for a Web Animation.
 */
struct ui_web_animation_timeline;

/**
 * @brief A Web Animation instance.
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
enum ui_error ui_web_animation_effect_create_keyframe_effect(
    struct ui_dom_node *target, struct ui_web_animation_keyframe *keyframes,
    const struct ui_web_animation_timing *timing,
    struct ui_web_animation_effect **out_effect);

/**
 * @brief Destroys an effect.
 *
 * @param effect The effect to destroy.
 */
enum ui_error
ui_web_animation_effect_destroy(struct ui_web_animation_effect *effect);

/**
 * @brief Creates a Document Timeline.
 *
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_web_animation_timeline_create_document_timeline(
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Creates a Scroll Timeline.
 *
 * @param source The scrolling element.
 * @param axis The scroll axis.
 * @param out_timeline Pointer to receive the new timeline.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_web_animation_timeline_create_scroll_timeline(
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
enum ui_error ui_web_animation_timeline_create_view_timeline(
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
enum ui_error ui_web_animation_timeline_create_pointer_timeline(
    struct ui_dom_node *target, enum ui_web_animation_pointer_axis axis,
    struct ui_web_animation_timeline **out_timeline);

/**
 * @brief Explicitly sets the current time of a timeline (useful for
 * scroll/view/pointer timelines).
 *
 * @param timeline The timeline.
 * @param time The current progress time.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_web_animation_timeline_set_current_time(
    struct ui_web_animation_timeline *timeline, double time);

/**
 * @brief Destroys a timeline.
 *
 * @param timeline The timeline to destroy.
 */
enum ui_error
ui_web_animation_timeline_destroy(struct ui_web_animation_timeline *timeline);

/**
 * @brief Creates a Web Animation.
 *
 * @param effect The effect to run (takes ownership).
 * @param timeline The timeline to use (takes ownership).
 * @param out_animation Pointer to receive the new animation.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_web_animation_create(struct ui_web_animation_effect *effect,
                        struct ui_web_animation_timeline *timeline,
                        struct ui_web_animation **out_animation);

/**
 * @brief Destroys a Web Animation.
 *
 * @param animation The animation to destroy.
 */
enum ui_error ui_web_animation_destroy(struct ui_web_animation *animation);

/**
 * @brief Plays the animation.
 */
enum ui_error ui_web_animation_play(struct ui_web_animation *animation);

/**
 * @brief Pauses the animation.
 */
enum ui_error ui_web_animation_pause(struct ui_web_animation *animation);

/**
 * @brief Reverses the animation playback direction.
 */
enum ui_error ui_web_animation_reverse(struct ui_web_animation *animation);

/**
 * @brief Cancels the animation.
 */
enum ui_error ui_web_animation_cancel(struct ui_web_animation *animation);

/**
 * @brief Finishes the animation.
 */
enum ui_error ui_web_animation_finish(struct ui_web_animation *animation);

/**
 * @brief Gets the current play state.
 */
enum ui_error
ui_web_animation_get_play_state(const struct ui_web_animation *animation,
                                enum ui_web_animation_play_state *out_state);

/**
 * @brief Gets the current time of the animation.
 */
enum ui_error
ui_web_animation_get_current_time(const struct ui_web_animation *animation,
                                  double *out_time);

/**
 * @brief Sets the current time of the animation.
 */
enum ui_error
ui_web_animation_set_current_time(struct ui_web_animation *animation,
                                  double time);

/**
 * @brief Gets the playback rate.
 */
enum ui_error
ui_web_animation_get_playback_rate(const struct ui_web_animation *animation,
                                   double *out_rate);

/**
 * @brief Sets the playback rate.
 */
enum ui_error
ui_web_animation_set_playback_rate(struct ui_web_animation *animation,
                                   double rate);

/**
 * @brief Advances the timeline for the given animation.
 */
enum ui_error ui_web_animation_tick(struct ui_web_animation *animation,
                                    double delta_time_ms);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_WEB_ANIMATION_H */
