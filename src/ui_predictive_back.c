/* clang-format off */
#include "ui_predictive_back.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_predictive_back {
  int edge_width_px;
  int screen_width_px;

  struct ui_signal *progress_signal;
  struct ui_signal *commit_signal;

  int is_tracking;
  int start_x;
  float current_progress;
};

/** \brief ui_error */
enum ui_error
ui_predictive_back_create(struct ui_predictive_back **out_tracker) {
  struct ui_predictive_back *t;

  if (!out_tracker) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  t = (struct ui_predictive_back *)UI_MALLOC(sizeof(struct ui_predictive_back));
  if (!t) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  t->edge_width_px = 40; /* Default Android/iOS edge threshold */
  t->screen_width_px = 1080;
  t->progress_signal = NULL;
  t->commit_signal = NULL;

  t->is_tracking = 0;
  t->start_x = 0;
  t->current_progress = 0.0f;

  *out_tracker = t;
  return UI_ERROR_NONE;
}

void ui_predictive_back_destroy(struct ui_predictive_back *tracker) {
  if (!tracker)
    return;
  UI_FREE(tracker);
}

enum ui_error ui_predictive_back_configure(struct ui_predictive_back *tracker,
                                           int edge_width_px,
                                           int screen_width_px) {
  if (!tracker)
    return UI_ERROR_INVALID_ARGUMENT;
  tracker->edge_width_px = edge_width_px;
  tracker->screen_width_px = screen_width_px;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_predictive_back_bind_progress(struct ui_predictive_back *tracker,
                                 struct ui_signal *progress_signal) {
  if (!tracker || !progress_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  tracker->progress_signal = progress_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_predictive_back_bind_commit(struct ui_predictive_back *tracker,
                                             struct ui_signal *commit_signal) {
  if (!tracker || !commit_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  tracker->commit_signal = commit_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_predictive_back_process_event(struct ui_predictive_back *tracker,
                                 const struct ui_event *event) {
  int tx = 0;

  if (!tracker || !event)
    return UI_ERROR_INVALID_ARGUMENT;

  if (event->type == UI_EVENT_TOUCH_START &&
      event->event_data.touch.num_points > 0) {
    tx = event->event_data.touch.points[0].x;
    if (tx <= tracker->edge_width_px) {
      tracker->is_tracking = 1;
      tracker->start_x = tx;
      tracker->current_progress = 0.0f;
    }
  } else if (event->type == UI_EVENT_TOUCH_MOVE && tracker->is_tracking) {
    if (event->event_data.touch.num_points > 0) {
      tx = event->event_data.touch.points[0].x;
      if (tracker->screen_width_px > 0) {
        tracker->current_progress =
            (float)(tx - tracker->start_x) / (float)tracker->screen_width_px;
        if (tracker->current_progress < 0.0f)
          tracker->current_progress = 0.0f;
        if (tracker->current_progress > 1.0f)
          tracker->current_progress = 1.0f;
      }

      /* Mock signal emit: ui_signal_emit(tracker->progress_signal,
       * &tracker->current_progress); */
    }
  } else if ((event->type == UI_EVENT_TOUCH_END ||
              event->type == UI_EVENT_TOUCH_CANCEL) &&
             tracker->is_tracking) {
    if (tracker->current_progress > 0.4f) {
      /* Commit swipe back */
      /* Mock signal emit: int v = 1; ui_signal_emit(tracker->commit_signal,
       * &v); */
    }
    tracker->is_tracking = 0;
    tracker->current_progress = 0.0f;
    /* Reset signal to 0.0f */
  }

  return UI_ERROR_NONE;
}
