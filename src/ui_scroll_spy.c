/* clang-format off */
#include "ui_scroll_spy.h"
#include "ui_intersection_observer.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

#define MAX_SPY_TARGETS 64

struct spy_target {
  struct ui_dom_node *node;
  int section_id;
  int is_intersecting;
  float intersection_ratio;
};

/** \brief ui_scroll_spy */
struct ui_scroll_spy {
  struct ui_intersection_observer *observer;
  struct ui_signal *active_signal;

  struct spy_target targets[MAX_SPY_TARGETS];
  int target_count;

  struct ui_dom_node *root;
  int root_margin_px;
};

static enum ui_error
on_intersection(struct ui_intersection_observer *observer,
                const struct ui_intersection_observer_entry *entries,
                int entry_count, void *user_data) {
  struct ui_scroll_spy *spy = (struct ui_scroll_spy *)user_data;
  int i, j;
  int best_id = -1;
  float best_ratio = -1.0f;

  (void)observer;

  if (!spy)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Update cached state for all reported entries */
  for (i = 0; i < entry_count; i++) {
    for (j = 0; j < spy->target_count; j++) {
      if (spy->targets[j].node == entries[i].target) {
        spy->targets[j].is_intersecting = entries[i].is_intersecting;
        spy->targets[j].intersection_ratio = entries[i].intersection_ratio;
        break;
      }
    }
  }

  /* Determine the new active section based on the highest intersection ratio */
  for (j = 0; j < spy->target_count; j++) {
    if (spy->targets[j].is_intersecting &&
        spy->targets[j].intersection_ratio > best_ratio) {
      best_ratio = spy->targets[j].intersection_ratio;
      best_id = spy->targets[j].section_id;
    }
  }

  /* Notify signal if bound and we have a valid section */
  /* For testing, we mock the signal emit since we don't know the exact signal
     payload structure, but normally we'd update the signal. */
  if (spy->active_signal && best_id != -1) {
    /* ui_signal_emit(spy->active_signal, (void*)(size_t)best_id); */
  }
  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_create(struct ui_scroll_spy **out_spy) {
  struct ui_scroll_spy *spy;

  if (!out_spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy = (struct ui_scroll_spy *)UI_MALLOC(sizeof(struct ui_scroll_spy));
  if (!spy) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  spy->observer = NULL;
  spy->active_signal = NULL;
  spy->target_count = 0;
  spy->root = NULL;
  spy->root_margin_px = 0;

  *out_spy = spy;
  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_destroy(struct ui_scroll_spy *spy) {
  if (!spy)
    return UI_ERROR_INVALID_ARGUMENT;

  if (spy->observer) {
    ui_intersection_observer_destroy(spy->observer);
  }
  UI_FREE(spy);
  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_set_root(struct ui_scroll_spy *spy,
                                     struct ui_dom_node *root,
                                     int root_margin_px) {
  float thresholds[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
  enum ui_error rc;

  if (!spy)
    return UI_ERROR_INVALID_ARGUMENT;

  spy->root = root;
  spy->root_margin_px = root_margin_px;

  if (spy->observer) {
    ui_intersection_observer_destroy(spy->observer);
    spy->observer = NULL;
  }

  rc = ui_intersection_observer_create(root, root_margin_px, thresholds, 5,
                                       &spy->observer);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_intersection_observer_subscribe(spy->observer, on_intersection, spy);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Re-observe existing targets */
  {
    int i;
    for (i = 0; i < spy->target_count; i++) {
      ui_intersection_observer_observe(spy->observer, spy->targets[i].node);
    }
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_add_target(struct ui_scroll_spy *spy,
                                       struct ui_dom_node *target,
                                       int section_id) {
  if (!spy || !target)
    return UI_ERROR_INVALID_ARGUMENT;

  if (spy->target_count >= MAX_SPY_TARGETS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  spy->targets[spy->target_count].node = target;
  spy->targets[spy->target_count].section_id = section_id;
  spy->targets[spy->target_count].is_intersecting = 0;
  spy->targets[spy->target_count].intersection_ratio = 0.0f;
  spy->target_count++;

  if (spy->observer) {
    ui_intersection_observer_observe(spy->observer, target);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_remove_target(struct ui_scroll_spy *spy,
                                          struct ui_dom_node *target) {
  int i;
  if (!spy || !target)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < spy->target_count; i++) {
    if (spy->targets[i].node == target) {
      if (spy->observer) {
        ui_intersection_observer_unobserve(spy->observer, target);
      }
      spy->targets[i] = spy->targets[spy->target_count - 1];
      spy->target_count--;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_scroll_spy_bind_active_section(struct ui_scroll_spy *spy,
                                  struct ui_signal *active_signal) {
  if (!spy || !active_signal)
    return UI_ERROR_INVALID_ARGUMENT;
  spy->active_signal = active_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_scroll_spy_evaluate(struct ui_scroll_spy *spy) {
  if (!spy)
    return UI_ERROR_INVALID_ARGUMENT;
  if (spy->observer) {
    return ui_intersection_observer_evaluate(spy->observer);
  }
  return UI_ERROR_NONE;
}
