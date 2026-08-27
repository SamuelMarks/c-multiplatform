/**
 * @file ui_web_animation.c
 * @brief Implementation of the Web Animation API component.
 */

/* clang-format off */
#include "ui_web_animation.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* Use MSVC Safe CRT internally, string.h is already included. */
#endif

/**
 * @struct ui_web_animation_effect
 * \brief ui_web_animation_effect
 */
struct ui_web_animation_effect {
  struct ui_dom_node *target;                  /**< target */
  struct ui_web_animation_keyframe *keyframes; /**< keyframes */
  struct ui_web_animation_timing timing;       /**< timing */
};

/**
 * @struct ui_web_animation_timeline
 * \brief ui_web_animation_timeline
 */
struct ui_web_animation_timeline {
  enum ui_web_animation_timeline_type type; /**< type */
  struct ui_dom_node *target;               /**< target */
  int axis;                                 /**< axis */
  double current_time;                      /**< current_time */
};

/**
 * @struct ui_web_animation
 * \brief ui_web_animation
 */
struct ui_web_animation {
  struct ui_web_animation_effect *effect;      /**< effect */
  struct ui_web_animation_timeline *timeline;  /**< timeline */
  enum ui_web_animation_play_state play_state; /**< play_state */
  double current_time;                         /**< current_time */
  double playback_rate;                        /**< playback_rate */
};

ui_error_t ui_web_animation_effect_create_keyframe_effect(
    struct ui_dom_node *target, struct ui_web_animation_keyframe *keyframes,
    const struct ui_web_animation_timing *timing,
    struct ui_web_animation_effect **out_effect) {
  struct ui_web_animation_effect *effect;

  if (!out_effect || !timing) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  effect = (struct ui_web_animation_effect *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation_effect));
  if (!effect) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  effect->target = target;
  effect->keyframes = keyframes;
  effect->timing = *timing;

  *out_effect = effect;
  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_effect_destroy(struct ui_web_animation_effect *effect) {
  struct ui_web_animation_keyframe *kf;
  struct ui_web_animation_keyframe *next_kf;
  struct ui_web_animation_keyframe_property *prop;
  struct ui_web_animation_keyframe_property *next_prop;

  if (!effect) {
    return UI_ERROR_NONE;
  }

  kf = effect->keyframes;
  while (kf) {
    next_kf = kf->next;

    prop = kf->properties;
    while (prop) {
      next_prop = prop->next;
      C_MULTIPLATFORM_FREE(prop->property_name);
      C_MULTIPLATFORM_FREE(prop->value);
      C_MULTIPLATFORM_FREE(prop);
      prop = next_prop;
    }

    C_MULTIPLATFORM_FREE(kf);
    kf = next_kf;
  }

  C_MULTIPLATFORM_FREE(effect);
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_timeline_create_document_timeline(
    struct ui_web_animation_timeline **out_timeline) {
  struct ui_web_animation_timeline *timeline;

  if (!out_timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timeline = (struct ui_web_animation_timeline *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation_timeline));
  if (!timeline) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timeline->type = UI_WEB_ANIMATION_TIMELINE_DOCUMENT;
  timeline->target = NULL;
  timeline->axis = 0;
  timeline->current_time = 0.0;
  *out_timeline = timeline;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_timeline_create_scroll_timeline(
    struct ui_dom_node *source, enum ui_web_animation_scroll_axis axis,
    struct ui_web_animation_timeline **out_timeline) {
  struct ui_web_animation_timeline *timeline;

  if (!out_timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timeline = (struct ui_web_animation_timeline *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation_timeline));
  if (!timeline) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timeline->type = UI_WEB_ANIMATION_TIMELINE_SCROLL;
  timeline->target = source;
  timeline->axis = (int)axis;
  timeline->current_time = 0.0;
  *out_timeline = timeline;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_timeline_create_view_timeline(
    struct ui_dom_node *subject, enum ui_web_animation_scroll_axis axis,
    struct ui_web_animation_timeline **out_timeline) {
  struct ui_web_animation_timeline *timeline;

  if (!out_timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timeline = (struct ui_web_animation_timeline *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation_timeline));
  if (!timeline) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timeline->type = UI_WEB_ANIMATION_TIMELINE_VIEW;
  timeline->target = subject;
  timeline->axis = (int)axis;
  timeline->current_time = 0.0;
  *out_timeline = timeline;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_timeline_create_pointer_timeline(
    struct ui_dom_node *target, enum ui_web_animation_pointer_axis axis,
    struct ui_web_animation_timeline **out_timeline) {
  struct ui_web_animation_timeline *timeline;

  if (!out_timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  timeline = (struct ui_web_animation_timeline *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation_timeline));
  if (!timeline) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  timeline->type = UI_WEB_ANIMATION_TIMELINE_POINTER;
  timeline->target = target;
  timeline->axis = (int)axis;
  timeline->current_time = 0.0;
  *out_timeline = timeline;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_timeline_set_current_time(
    struct ui_web_animation_timeline *timeline, double time) {
  if (!timeline) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  timeline->current_time = time;
  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_timeline_destroy(struct ui_web_animation_timeline *timeline) {
  if (!timeline) {
    return UI_ERROR_NONE;
  }
  C_MULTIPLATFORM_FREE(timeline);
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_create(struct ui_web_animation_effect *effect,
                                   struct ui_web_animation_timeline *timeline,
                                   struct ui_web_animation **out_animation) {
  struct ui_web_animation *animation;

  if (!out_animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  animation = (struct ui_web_animation *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_web_animation));
  if (!animation) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  animation->effect = effect;
  animation->timeline = timeline;
  animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_IDLE;
  animation->current_time = 0.0;
  animation->playback_rate = 1.0;

  *out_animation = animation;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_destroy(struct ui_web_animation *animation) {
  if (!animation) {
    return UI_ERROR_NONE;
  }

  if (animation->effect) {
    ui_web_animation_effect_destroy(animation->effect);
  }
  if (animation->timeline) {
    ui_web_animation_timeline_destroy(animation->timeline);
  }

  C_MULTIPLATFORM_FREE(animation);
  return UI_ERROR_NONE;
}

static double get_active_duration(const struct ui_web_animation *anim) {
  if (!anim->effect) {
    return 0.0;
  }
  if (anim->effect->timing.iterations < 0.0) {
    return -1.0; /* Represents infinity */
  }
  return anim->effect->timing.duration * anim->effect->timing.iterations;
}

ui_error_t ui_web_animation_play(struct ui_web_animation *animation) {
  double active_duration;

  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  active_duration = get_active_duration(animation);

  if (animation->playback_rate > 0.0) {
    if (animation->play_state == UI_WEB_ANIMATION_PLAY_STATE_FINISHED ||
        (active_duration >= 0.0 &&
         animation->current_time >= active_duration) ||
        animation->current_time < 0.0) {
      animation->current_time = 0.0;
    }
  } else {
    if (animation->play_state == UI_WEB_ANIMATION_PLAY_STATE_FINISHED ||
        animation->current_time <= 0.0 ||
        (active_duration >= 0.0 && animation->current_time > active_duration)) {
      if (active_duration >= 0.0) {
        animation->current_time = active_duration;
      }
    }
  }

  animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_RUNNING;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_pause(struct ui_web_animation *animation) {
  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (animation->play_state != UI_WEB_ANIMATION_PLAY_STATE_IDLE &&
      animation->play_state != UI_WEB_ANIMATION_PLAY_STATE_FINISHED) {
    animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_PAUSED;
  }
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_reverse(struct ui_web_animation *animation) {
  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  animation->playback_rate = -animation->playback_rate;
  return ui_web_animation_play(animation);
}

ui_error_t ui_web_animation_cancel(struct ui_web_animation *animation) {
  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_IDLE;
  animation->current_time = 0.0;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_finish(struct ui_web_animation *animation) {
  double active_duration;

  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  active_duration = get_active_duration(animation);

  if (animation->playback_rate > 0.0) {
    if (active_duration >= 0.0) {
      animation->current_time = active_duration;
    }
  } else {
    animation->current_time = 0.0;
  }

  animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_FINISHED;
  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_get_play_state(const struct ui_web_animation *animation,
                                enum ui_web_animation_play_state *out_state) {
  if (!animation || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = animation->play_state;
  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_get_current_time(const struct ui_web_animation *animation,
                                  double *out_time) {
  if (!animation || !out_time) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_time = animation->current_time;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_set_current_time(struct ui_web_animation *animation,
                                             double time) {
  double active_duration;

  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  animation->current_time = time;
  active_duration = get_active_duration(animation);

  if (animation->play_state == UI_WEB_ANIMATION_PLAY_STATE_FINISHED) {
    if ((animation->playback_rate > 0.0 && time < active_duration) ||
        (animation->playback_rate <= 0.0 && time > 0.0)) {
      animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_RUNNING;
    }
  } else if (animation->play_state == UI_WEB_ANIMATION_PLAY_STATE_RUNNING) {
    if ((animation->playback_rate > 0.0 && active_duration >= 0.0 &&
         time >= active_duration) ||
        (animation->playback_rate <= 0.0 && time <= 0.0)) {
      animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_FINISHED;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_get_playback_rate(const struct ui_web_animation *animation,
                                   double *out_rate) {
  if (!animation || !out_rate) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_rate = animation->playback_rate;
  return UI_ERROR_NONE;
}

ui_error_t
ui_web_animation_set_playback_rate(struct ui_web_animation *animation,
                                   double rate) {
  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  animation->playback_rate = rate;
  return UI_ERROR_NONE;
}

ui_error_t ui_web_animation_tick(struct ui_web_animation *animation,
                                 double delta_time_ms) {
  double new_time;
  double active_duration;

  if (!animation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (animation->play_state != UI_WEB_ANIMATION_PLAY_STATE_RUNNING) {
    return UI_ERROR_NONE;
  }

  if (animation->timeline) {
    animation->timeline->current_time += delta_time_ms;
  }

  new_time =
      animation->current_time + (delta_time_ms * animation->playback_rate);
  active_duration = get_active_duration(animation);

  if (animation->playback_rate > 0.0) {
    if (active_duration >= 0.0 && new_time >= active_duration) {
      new_time = active_duration;
      animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_FINISHED;
    }
  } else {
    if (new_time <= 0.0) {
      new_time = 0.0;
      animation->play_state = UI_WEB_ANIMATION_PLAY_STATE_FINISHED;
    }
  }

  animation->current_time = new_time;
  return UI_ERROR_NONE;
}
