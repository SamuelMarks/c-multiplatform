#ifndef CMP_CSS_SCROLL_ANIMATIONS_H
#define CMP_CSS_SCROLL_ANIMATIONS_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_scroll_animations.h
 * @brief CSS Scroll-Driven Animations & View Transitions (Level 1)
 * implementation.
 */

/**
 * @brief Represents the axis of a scroll timeline.
 */
typedef enum cmp_scroll_timeline_axis {
  CMP_SCROLL_TIMELINE_AXIS_BLOCK = 0,
  CMP_SCROLL_TIMELINE_AXIS_INLINE,
  CMP_SCROLL_TIMELINE_AXIS_X,
  CMP_SCROLL_TIMELINE_AXIS_Y
} cmp_scroll_timeline_axis_t;

/**
 * @brief Represents animation-timeline, animation-range, animation-range-start,
 * and animation-range-end.
 */
typedef struct cmp_prop_scroll_anim_group {
  char *
      timeline; /**< animation-timeline (e.g., "auto", "none", "my-timeline") */
  char *
      range_start; /**< animation-range-start (e.g., "cover 0%", "entry 25%") */
  char *range_end; /**< animation-range-end (e.g., "cover 100%", "exit 75%") */
} cmp_prop_scroll_anim_group_t;

/**
 * @brief Represents scroll-timeline-name, scroll-timeline-axis, and
 * scroll-timeline.
 */
typedef struct cmp_prop_scroll_timeline {
  char *name; /**< scroll-timeline-name (e.g., "none", "--my-timeline") */
  cmp_scroll_timeline_axis_t axis; /**< scroll-timeline-axis */
} cmp_prop_scroll_timeline_t;

/**
 * @brief Represents view-timeline-name, view-timeline-axis,
 * view-timeline-inset, and view-timeline.
 */
typedef struct cmp_prop_view_timeline {
  char *name; /**< view-timeline-name (e.g., "none", "--my-view-timeline") */
  cmp_scroll_timeline_axis_t axis; /**< view-timeline-axis */
  char *inset; /**< view-timeline-inset (e.g., "auto", "10% 20%") */
} cmp_prop_view_timeline_t;

/**
 * @brief Represents timeline-scope.
 */
typedef struct cmp_prop_timeline_scope {
  char *scope; /**< timeline-scope (e.g., "none", "all", "--my-timeline") */
} cmp_prop_timeline_scope_t;

/**
 * @brief Represents view-transition-name.
 */
typedef struct cmp_prop_view_transition_name {
  char *name; /**< view-transition-name (e.g., "none", "hero-image") */
} cmp_prop_view_transition_name_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes a scroll animation group.
 * @param group The scroll animation group to initialize.
 * @param timeline animation-timeline string (can be NULL).
 * @param range_start animation-range-start string (can be NULL).
 * @param range_end animation-range-end string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_scroll_anim_group_init(cmp_prop_scroll_anim_group_t *group,
                                    const char *timeline,
                                    const char *range_start,
                                    const char *range_end);

/**
 * @brief Frees resources in a scroll animation group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_scroll_anim_group_free(cmp_prop_scroll_anim_group_t *group);

/**
 * @brief Initializes a scroll timeline property group.
 * @param group The scroll timeline group to initialize.
 * @param name scroll-timeline-name string (can be NULL).
 * @param axis scroll-timeline-axis enum.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_scroll_timeline_init(cmp_prop_scroll_timeline_t *group,
                                  const char *name,
                                  cmp_scroll_timeline_axis_t axis);

/**
 * @brief Frees resources in a scroll timeline group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_scroll_timeline_free(cmp_prop_scroll_timeline_t *group);

/**
 * @brief Initializes a view timeline property group.
 * @param group The view timeline group to initialize.
 * @param name view-timeline-name string (can be NULL).
 * @param axis view-timeline-axis enum.
 * @param inset view-timeline-inset string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_view_timeline_init(cmp_prop_view_timeline_t *group,
                                const char *name,
                                cmp_scroll_timeline_axis_t axis,
                                const char *inset);

/**
 * @brief Frees resources in a view timeline group.
 * @param group The group to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_view_timeline_free(cmp_prop_view_timeline_t *group);

/**
 * @brief Initializes a timeline scope property.
 * @param scope_prop The timeline scope property to initialize.
 * @param scope timeline-scope string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_timeline_scope_init(cmp_prop_timeline_scope_t *scope_prop,
                                 const char *scope);

/**
 * @brief Frees resources in a timeline scope property.
 * @param scope_prop The property to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_timeline_scope_free(cmp_prop_timeline_scope_t *scope_prop);

/**
 * @brief Initializes a view transition name property.
 * @param name_prop The view transition name property to initialize.
 * @param name view-transition-name string (can be NULL).
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_view_transition_name_init(
    cmp_prop_view_transition_name_t *name_prop, const char *name);

/**
 * @brief Frees resources in a view transition name property.
 * @param name_prop The property to free.
 * @return 0 on success, non-zero on error.
 */
int cmp_prop_view_transition_name_free(
    cmp_prop_view_transition_name_t *name_prop);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_SCROLL_ANIMATIONS_H */