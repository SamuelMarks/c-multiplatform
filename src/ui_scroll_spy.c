/**
 * @file ui_scroll_spy.c
 * @brief ui_scroll_spy.c implementation.
 */
/*
 * \file ui_scroll_spy.c
 * \brief Implementation of the UI Scroll Spy component.
 */

/* clang-format off */
#include "ui_scroll_spy.h"
#include "ui_intersection_observer.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_scroll_spy_mock_fail = 0;

/**
 * @brief mock_scroll_spy_signal_set.
 * @param signal Parameter signal.
 * @param payload Parameter payload.
 * @return Return value.
 */
static ui_error_t mock_scroll_spy_signal_set(struct ui_signal *signal,
                                             union ui_signal_payload payload) {
  if (g_scroll_spy_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_set)(signal, payload);
}
#undef ui_signal_set
/** @cond */
#define ui_signal_set mock_scroll_spy_signal_set
/** @endcond */

/**
 * @brief mock_scroll_spy_intersection_observer_destroy.
 * @param obs Parameter obs.
 * @return Return value.
 */
static ui_error_t mock_scroll_spy_intersection_observer_destroy(
    struct ui_intersection_observer *obs) {
  if (g_scroll_spy_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_intersection_observer_destroy)(obs);
}
#undef ui_intersection_observer_destroy
/** @cond */
#define ui_intersection_observer_destroy                                       \
  mock_scroll_spy_intersection_observer_destroy
/** @endcond */

/**
 * @brief mock_scroll_spy_subscribe.
 * @param obs Parameter obs.
 * @param cb Parameter cb.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t
mock_scroll_spy_subscribe(struct ui_intersection_observer *obs,
                          ui_intersection_observer_cb_t cb, void *user_data) {
  if (g_scroll_spy_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_intersection_observer_subscribe)(obs, cb, user_data);
}
#undef ui_intersection_observer_subscribe
/** @cond */
#define ui_intersection_observer_subscribe mock_scroll_spy_subscribe
/** @endcond */

/**
 * @brief mock_scroll_spy_observe.
 * @param obs Parameter obs.
 * @param target Parameter target.
 * @return Return value.
 */
static ui_error_t mock_scroll_spy_observe(struct ui_intersection_observer *obs,
                                          struct ui_dom_node *target) {
  if (g_scroll_spy_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_intersection_observer_observe)(obs, target);
}
#undef ui_intersection_observer_observe
/** @cond */
#define ui_intersection_observer_observe mock_scroll_spy_observe
/** @endcond */

/**
 * @brief mock_scroll_spy_unobserve.
 * @param obs Parameter obs.
 * @param target Parameter target.
 * @return Return value.
 */
static ui_error_t
mock_scroll_spy_unobserve(struct ui_intersection_observer *obs,
                          struct ui_dom_node *target) {
  if (g_scroll_spy_mock_fail == 5) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_intersection_observer_unobserve)(obs, target);
}
#undef ui_intersection_observer_unobserve
/** @cond */
#define ui_intersection_observer_unobserve mock_scroll_spy_unobserve
/** @endcond */
#endif

/* \brief Maximum number of targets a scroll spy can track */
/** @def MAX_SPY_TARGETS
 * @brief Maximum spy targets
 */
#define MAX_SPY_TARGETS 64

/**
 * @struct spy_target
 * \brief Internal structure representing a tracked target section.
 */
struct spy_target {
  struct ui_dom_node *node; /**< Tracked DOM node */
  int section_id;           /**< User-defined section ID */
  int is_intersecting;      /**< True if currently intersecting */
  float intersection_ratio; /**< Current intersection ratio */
};

/**
 * @struct ui_scroll_spy
 * \brief Internal structure representing the scroll spy instance.
 */
struct ui_scroll_spy {
  struct ui_intersection_observer *observer; /**< Intersection observer */
  struct ui_signal *active_signal;           /**< Bound active signal */

  struct spy_target targets[MAX_SPY_TARGETS]; /**< Tracked targets array */
  int target_count;                           /**< Current number of targets */

  struct ui_dom_node *root; /**< Root scrolling container */
  int root_margin_px;       /**< Root margin in pixels */
};

/**
 * \brief Intersection observer callback to handle visibility changes.
 *
 * \param observer The intersection observer.
 * \param entries The changed entries.
 * \param entry_count Number of entries.
 * \param user_data Opaque pointer to the scroll spy instance.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
static ui_error_t
/**
 * @brief on_intersection.
 * @param observer Parameter observer.
 * @param entries Parameter entries.
 * @param entry_count Parameter entry_count.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
on_intersection(struct ui_intersection_observer *observer,
                const struct ui_intersection_observer_entry *entries,
                int entry_count, void *user_data) {
  struct ui_scroll_spy *spy;
  int i;
  int j;
  int best_id = -1;
  float best_ratio = -1.0f;

  if (!observer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy = (struct ui_scroll_spy *)user_data;

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
  if (spy->active_signal && best_id != -1) {
    union ui_signal_payload payload;
    ui_error_t s_rc;
    payload.int_val = best_id;
    s_rc = ui_signal_set(spy->active_signal, payload);
    if (s_rc != UI_ERROR_NONE) {
      /* Reactive effect failure does not disrupt scroll tracking */
    }
  }
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
/**
 * @brief test_ui_scroll_spy_call_on_intersection.
 * @param spy Parameter spy.
 * @param obs Parameter obs.
 * @return Return value.
 */
ui_error_t
test_ui_scroll_spy_call_on_intersection(struct ui_scroll_spy *spy,
                                        struct ui_intersection_observer *obs);
ui_error_t
test_ui_scroll_spy_call_on_intersection(struct ui_scroll_spy *spy,
                                        struct ui_intersection_observer *obs) {
  struct ui_intersection_observer_entry entry;
  entry.target = (struct ui_dom_node *)0xdead;
  entry.is_intersecting = 0;
  entry.intersection_ratio = 0.0f;
  return on_intersection(obs, &entry, 1, spy);
}
#endif

/**
 * \brief Creates a new scroll spy behavior instance.
 *
 * \param out_spy Pointer to receive the allocated scroll spy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_create(struct ui_scroll_spy **out_spy) {
  struct ui_scroll_spy *spy;

  if (!out_spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy = (struct ui_scroll_spy *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_scroll_spy));
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

/**
 * \brief Destroys a scroll spy instance.
 *
 * \param spy The scroll spy to destroy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_destroy(struct ui_scroll_spy *spy) {
  ui_error_t rc = UI_ERROR_NONE;

  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (spy->observer) {
    rc = ui_intersection_observer_destroy(spy->observer);
  }
  C_MULTIPLATFORM_FREE(spy);
  return rc;
}

/**
 * \brief Sets the root scrolling container and its observation margin.
 *
 * \param spy The scroll spy.
 * \param root The scrolling container DOM node (or NULL for viewport).
 * \param root_margin_px Margin to apply to the root bounds (usually negative to
 * trigger early).
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_set_root(struct ui_scroll_spy *spy,
                                  struct ui_dom_node *root,
                                  int root_margin_px) {
  float thresholds[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
  ui_error_t rc;
  int i;

  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  spy->root = root;
  spy->root_margin_px = root_margin_px;

  if (spy->observer) {
    rc = ui_intersection_observer_destroy(spy->observer);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    spy->observer = NULL;
  }

  rc = ui_intersection_observer_create(root, root_margin_px, thresholds, 5,
                                       &spy->observer);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* subscribe only fails on NULL observer, which is guaranteed non-NULL here */
  rc = ui_intersection_observer_subscribe(spy->observer, on_intersection, spy);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  /* Re-observe existing targets */
  for (i = 0; i < spy->target_count; i++) {
    rc = ui_intersection_observer_observe(spy->observer, spy->targets[i].node);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Adds a target section to be tracked by the scroll spy.
 *
 * \param spy The scroll spy.
 * \param target The target DOM node representing the content section.
 * \param section_id The unique user-defined ID for this section.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_add_target(struct ui_scroll_spy *spy,
                                    struct ui_dom_node *target,
                                    int section_id) {
  if (!spy || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (spy->target_count >= MAX_SPY_TARGETS) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  spy->targets[spy->target_count].node = target;
  spy->targets[spy->target_count].section_id = section_id;
  spy->targets[spy->target_count].is_intersecting = 0;
  spy->targets[spy->target_count].intersection_ratio = 0.0f;
  spy->target_count++;

  if (spy->observer) {
    return ui_intersection_observer_observe(spy->observer, target);
  }

  return UI_ERROR_NONE;
}

/**
 * \brief Removes a target section from the scroll spy.
 *
 * \param spy The scroll spy.
 * \param target The target DOM node to stop tracking.
 * \return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND, or an appropriate error
 * code.
 */
ui_error_t ui_scroll_spy_remove_target(struct ui_scroll_spy *spy,
                                       struct ui_dom_node *target) {
  int i;
  ui_error_t rc;

  if (!spy || !target) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < spy->target_count; i++) {
    if (spy->targets[i].node == target) {
      if (spy->observer) {
        rc = ui_intersection_observer_unobserve(spy->observer, target);
        if (rc != UI_ERROR_NONE) {
          return rc;
        }
      }
      spy->targets[i] = spy->targets[spy->target_count - 1];
      spy->target_count--;
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}

/**
 * \brief Binds a signal that will receive the active section ID.
 * The payload of the signal should be castable to (int).
 *
 * \param spy The scroll spy.
 * \param active_signal The signal to update when the active section changes.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_bind_active_section(struct ui_scroll_spy *spy,
                                             struct ui_signal *active_signal) {
  if (!spy || !active_signal) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  spy->active_signal = active_signal;
  return UI_ERROR_NONE;
}

/**
 * \brief Triggers an evaluation of the underlying intersection observer.
 * Typically called during a layout or scroll event.
 *
 * \param spy The scroll spy.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_scroll_spy_evaluate(struct ui_scroll_spy *spy) {
  if (!spy) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (spy->observer) {
    return ui_intersection_observer_evaluate(spy->observer);
  }
  return UI_ERROR_NONE;
}
