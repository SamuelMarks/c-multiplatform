/**
 * \file ui_layout_observer.c
 * \brief Implementation of responsive layout observations and breakpoints.
 */
/* clang-format off */
#include "ui_layout_observer.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * \struct ui_layout_breakpoint
 * \brief Defines a dimensional range for triggering layout changes.
 */
struct ui_layout_breakpoint {
  int id;
  int min_width;
  int max_width;
  int min_height;
  int max_height;
  int is_active;
};

/** \brief ui_layout_subscriber */
struct ui_layout_subscriber {
  ui_layout_observer_cb_t callback;
  void *user_data;
};

/** \brief ui_layout_observer */
struct ui_layout_observer {
  struct ui_layout_breakpoint *breakpoints;
  int breakpoint_count;
  int breakpoint_capacity;
  int next_id;

  struct ui_layout_subscriber *subscribers;
  int subscriber_count;
  int subscriber_capacity;

  int last_width;
  int last_height;
};

/** \brief ui_error */
ui_error_t ui_layout_observer_create(struct ui_layout_observer **out_observer) {
  struct ui_layout_observer *obs;

  if (!out_observer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  obs = (struct ui_layout_observer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_layout_observer));
  if (!obs) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(obs, 0, sizeof(struct ui_layout_observer));
  obs->last_width = -1;
  obs->last_height = -1;
  obs->next_id = 1;

  *out_observer = obs;
  return UI_ERROR_NONE;
}

/**
 * \brief Destroys a layout observer.
 * \param[in,out] observer The observer to destroy.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_layout_observer_destroy(struct ui_layout_observer *observer) {
  if (!observer) {
    return UI_ERROR_NONE;
  }

  if (observer->breakpoints) {
    C_MULTIPLATFORM_FREE(observer->breakpoints);
  }
  if (observer->subscribers) {
    C_MULTIPLATFORM_FREE(observer->subscribers);
  }

  C_MULTIPLATFORM_FREE(observer);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_layout_observer_add_breakpoint(struct ui_layout_observer *observer,
                                  int min_width, int max_width, int min_height,
                                  int max_height, int *out_breakpoint_id) {

  struct ui_layout_breakpoint *bp;

  if (!observer || !out_breakpoint_id) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (observer->breakpoint_count >= observer->breakpoint_capacity) {
    int new_cap = observer->breakpoint_capacity == 0
                      ? 4
                      : observer->breakpoint_capacity * 2;
    struct ui_layout_breakpoint *new_bps =
        (struct ui_layout_breakpoint *)C_MULTIPLATFORM_REALLOC(
            observer->breakpoints,
            sizeof(struct ui_layout_breakpoint) * new_cap);
    if (!new_bps) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    observer->breakpoints = new_bps;
    observer->breakpoint_capacity = new_cap;
  }

  bp = &observer->breakpoints[observer->breakpoint_count++];
  bp->id = observer->next_id++;
  bp->min_width = min_width;
  bp->max_width = max_width;
  bp->min_height = min_height;
  bp->max_height = max_height;
  bp->is_active = 0;

  /* Pre-evaluate if we already have a dimension */
  if (observer->last_width >= 0) {
    int active = 1;
    if (min_width >= 0 && observer->last_width < min_width)
      active = 0;
    if (max_width >= 0 && observer->last_width > max_width)
      active = 0;
    if (min_height >= 0 && observer->last_height < min_height)
      active = 0;
    if (max_height >= 0 && observer->last_height > max_height)
      active = 0;
    bp->is_active = active;
  }

  *out_breakpoint_id = bp->id;
  return UI_ERROR_NONE;
}

/**
 * \brief Subscribes a callback to layout breakpoint changes.
 * \param[in,out] observer The layout observer.
 * \param[in] callback The function to call when a breakpoint triggers.
 * \param[in] user_data Optional user data to pass to the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_layout_observer_subscribe(struct ui_layout_observer *observer,
                                        ui_layout_observer_cb_t callback,
                                        void *user_data) {

  struct ui_layout_subscriber *sub;

  if (!observer || !callback) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (observer->subscriber_count >= observer->subscriber_capacity) {
    int new_cap = observer->subscriber_capacity == 0
                      ? 4
                      : observer->subscriber_capacity * 2;
    struct ui_layout_subscriber *new_subs =
        (struct ui_layout_subscriber *)C_MULTIPLATFORM_REALLOC(
            observer->subscribers,
            sizeof(struct ui_layout_subscriber) * new_cap);
    if (!new_subs) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    observer->subscribers = new_subs;
    observer->subscriber_capacity = new_cap;
  }

  sub = &observer->subscribers[observer->subscriber_count++];
  sub->callback = callback;
  sub->user_data = user_data;

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_layout_observer_notify_resize(struct ui_layout_observer *observer,
                                            int width, int height) {
  int i, j;

  if (!observer || width < 0 || height < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (observer->last_width == width && observer->last_height == height) {
    return UI_ERROR_NONE; /* No change */
  }

  observer->last_width = width;
  observer->last_height = height;

  for (i = 0; i < observer->breakpoint_count; i++) {
    struct ui_layout_breakpoint *bp = &observer->breakpoints[i];
    int active = 1;

    if (bp->min_width >= 0 && width < bp->min_width)
      active = 0;
    if (bp->max_width >= 0 && width > bp->max_width)
      active = 0;
    if (bp->min_height >= 0 && height < bp->min_height)
      active = 0;
    if (bp->max_height >= 0 && height > bp->max_height)
      active = 0;

    if (active != bp->is_active) {
      bp->is_active = active;
      for (j = 0; j < observer->subscriber_count; j++) {
        ui_error_t cb_rc = observer->subscribers[j].callback(
            observer, bp->id, active, observer->subscribers[j].user_data);
        if (cb_rc != UI_ERROR_NONE) {
          return cb_rc;
        }
      }
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_layout_observer_is_active(const struct ui_layout_observer *observer,
                             int breakpoint_id, int *out_is_active) {
  int i;
  if (!observer || !out_is_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_active = 0;
  if (breakpoint_id <= 0) {
    return UI_ERROR_NONE;
  }

  for (i = 0; i < observer->breakpoint_count; i++) {
    if (observer->breakpoints[i].id == breakpoint_id) {
      *out_is_active = observer->breakpoints[i].is_active;
      break;
    }
  }

  return UI_ERROR_NONE;
}
