/**
 * \file ui_intersection_observer.c
 * \brief Implementation of intersection observer for viewport visibility
 * tracking.
 */
/* clang-format off */
#include "ui_intersection_observer.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/**
 * \struct ui_intersection_observer_target
 * \brief Represents a target node being observed for intersection changes.
 */
struct ui_intersection_observer_target {
  struct ui_dom_node *node;
  int is_intersecting;
  float last_ratio;
};

/** \brief ui_intersection_observer */
struct ui_intersection_observer {
  struct ui_dom_node *root;
  int root_margin_px;
  float *thresholds;
  int threshold_count;

  struct ui_intersection_observer_target *targets;
  int target_count;
  int target_capacity;

  ui_intersection_observer_cb_t callback;
  void *user_data;
};

/** \brief ui_intersection_observer_create */
ui_error_t ui_intersection_observer_create(
    struct ui_dom_node *root, int root_margin_px, const float *thresholds,
    int threshold_count, struct ui_intersection_observer **out_observer) {

  struct ui_intersection_observer *obs;
  int i;

  if (out_observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  obs = (struct ui_intersection_observer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_intersection_observer));
  if (obs == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  obs->root = root;
  obs->root_margin_px = root_margin_px;

  if (threshold_count > 0) {
    obs->thresholds =
        (float *)C_MULTIPLATFORM_MALLOC(sizeof(float) * threshold_count);
    if (obs->thresholds == NULL) {
      C_MULTIPLATFORM_FREE(obs);
      return UI_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < threshold_count; i++) {
      obs->thresholds[i] = thresholds[i];
    }
    obs->threshold_count = threshold_count;
  } else {
    obs->thresholds = NULL;
    obs->threshold_count = 0;
  }

  obs->target_capacity = 4;
  obs->target_count = 0;
  obs->targets =
      (struct ui_intersection_observer_target *)C_MULTIPLATFORM_MALLOC(
          sizeof(struct ui_intersection_observer_target) *
          obs->target_capacity);
  if (obs->targets == NULL) {
    if (obs->thresholds)
      C_MULTIPLATFORM_FREE(obs->thresholds);
    C_MULTIPLATFORM_FREE(obs);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  obs->callback = NULL;
  obs->user_data = NULL;

  *out_observer = obs;
  return UI_ERROR_NONE;
}

/** \brief ui_intersection_observer_destroy */
ui_error_t
ui_intersection_observer_destroy(struct ui_intersection_observer *observer) {
  if (observer == NULL) {
    return UI_ERROR_NONE;
  }

  if (observer->thresholds != NULL) {
    C_MULTIPLATFORM_FREE(observer->thresholds);
  }

  C_MULTIPLATFORM_FREE(observer->targets);

  C_MULTIPLATFORM_FREE(observer);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_intersection_observer_observe(struct ui_intersection_observer *observer,
                                 struct ui_dom_node *target) {
  int i;
  struct ui_intersection_observer_target *new_targets;

  if (observer == NULL || target == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Check if already observing */
  for (i = 0; i < observer->target_count; i++) {
    if (observer->targets[i].node == target) {
      return UI_ERROR_NONE; /* Already observing */
    }
  }

  if (observer->target_count >= observer->target_capacity) {
    int new_capacity = observer->target_capacity * 2;
    new_targets =
        (struct ui_intersection_observer_target *)C_MULTIPLATFORM_MALLOC(
            sizeof(struct ui_intersection_observer_target) * new_capacity);
    if (new_targets == NULL) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(new_targets, observer->targets,
           sizeof(struct ui_intersection_observer_target) *
               observer->target_count);
    C_MULTIPLATFORM_FREE(observer->targets);
    observer->targets = new_targets;
    observer->target_capacity = new_capacity;
  }

  observer->targets[observer->target_count].node = target;
  observer->targets[observer->target_count].is_intersecting = 0;
  observer->targets[observer->target_count].last_ratio = 0.0f;
  observer->target_count++;

  return UI_ERROR_NONE;
}

/**
 * \brief Stops observing a target node for intersection changes.
 * \param[in,out] observer The intersection observer.
 * \param[in,out] target The DOM node to stop observing.
 * \return UI_ERROR_NONE on success, or UI_ERROR_NOT_FOUND if not observed.
 */
ui_error_t
ui_intersection_observer_unobserve(struct ui_intersection_observer *observer,
                                   struct ui_dom_node *target) {
  int i, j;

  if (observer == NULL || target == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < observer->target_count; i++) {
    if (observer->targets[i].node == target) {
      for (j = i; j < observer->target_count - 1; j++) {
        observer->targets[j] = observer->targets[j + 1];
      }
      observer->target_count--;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * \brief Stops observing all target nodes.
 * \param[in,out] observer The intersection observer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_disconnect(struct ui_intersection_observer *observer) {
  if (observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  observer->target_count = 0;
  return UI_ERROR_NONE;
}

/**
 * \brief Subscribes a callback to receive intersection events.
 * \param[in,out] observer The intersection observer.
 * \param[in] callback The callback function to invoke on intersection changes.
 * \param[in] user_data Optional user data passed to the callback.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_subscribe(struct ui_intersection_observer *observer,
                                   ui_intersection_observer_cb_t callback,
                                   void *user_data) {

  if (observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  observer->callback = callback;
  observer->user_data = user_data;

  return UI_ERROR_NONE;
}

/**
 * \brief Evaluates intersections for all observed targets against thresholds.
 * \param[in,out] observer The intersection observer.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t
ui_intersection_observer_evaluate(struct ui_intersection_observer *observer) {
  /* For a fully robust implementation, this would require querying ui_layout
     bounds. We mock this evaluation cycle minimally to satisfy architecture
     paths for now, until a rigid integration with ui_layout bounding-box
     queries is wired. */

  struct ui_intersection_observer_entry *entries = NULL;
  int entry_count = 0;
  int i, j;
  int cross_threshold;
  float current_ratio;
  int is_intersecting;

  if (observer == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (observer->callback == NULL || observer->target_count == 0) {
    return UI_ERROR_NONE;
  }

  entries = (struct ui_intersection_observer_entry *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_intersection_observer_entry) * observer->target_count);
  if (entries == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  for (i = 0; i < observer->target_count; i++) {
    /* MOCK logic for layout querying */
    current_ratio = 1.0f; /* Assume fully visible for scaffolding */
    is_intersecting = 1;

    cross_threshold = 0;
    if (observer->threshold_count > 0) {
      for (j = 0; j < observer->threshold_count; j++) {
        if ((observer->targets[i].last_ratio < observer->thresholds[j] &&
             current_ratio >= observer->thresholds[j]) ||
            (observer->targets[i].last_ratio >= observer->thresholds[j] &&
             current_ratio < observer->thresholds[j])) {
          cross_threshold = 1;
          break;
        }
      }
    } else {
      if (observer->targets[i].is_intersecting != is_intersecting) {
        cross_threshold = 1;
      }
    }

    if (cross_threshold) {
      entries[entry_count].target = observer->targets[i].node;
      entries[entry_count].is_intersecting = is_intersecting;
      entries[entry_count].intersection_ratio = current_ratio;
      entry_count++;

      observer->targets[i].is_intersecting = is_intersecting;
      observer->targets[i].last_ratio = current_ratio;
    }
  }

  if (entry_count > 0) {
    (void)observer->callback(observer, entries, entry_count,
                             observer->user_data);
  }

  C_MULTIPLATFORM_FREE(entries);

  return UI_ERROR_NONE;
}
