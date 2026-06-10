#ifndef CMP_CSS_OVERFLOW_H
#define CMP_CSS_OVERFLOW_H

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_overflow.h
 * @brief CSS Overflow, Scrolling & Overscroll implementation.
 */

/**
 * @brief Represents overflow value.
 */
typedef enum cmp_overflow_value {
  CMP_OVERFLOW_VISIBLE = 0,
  CMP_OVERFLOW_HIDDEN,
  CMP_OVERFLOW_CLIP,
  CMP_OVERFLOW_SCROLL,
  CMP_OVERFLOW_AUTO
} cmp_overflow_value_t;

/**
 * @brief Represents overflow properties group.
 */
typedef struct cmp_prop_overflow_group {
  cmp_overflow_value_t overflow_x;
  cmp_overflow_value_t overflow_y;
  char *overflow_clip_margin; /**< String representation of clip margin */
} cmp_prop_overflow_group_t;

/**
 * @brief Represents overscroll-behavior value.
 */
typedef enum cmp_overscroll_behavior_value {
  CMP_OVERSCROLL_BEHAVIOR_AUTO = 0,
  CMP_OVERSCROLL_BEHAVIOR_CONTAIN,
  CMP_OVERSCROLL_BEHAVIOR_NONE
} cmp_overscroll_behavior_value_t;

/**
 * @brief Represents overscroll-behavior properties.
 */
typedef struct cmp_prop_overscroll {
  cmp_overscroll_behavior_value_t overscroll_x;
  cmp_overscroll_behavior_value_t overscroll_y;
} cmp_prop_overscroll_t;

/**
 * @brief Represents scroll-behavior property.
 */
typedef enum cmp_scroll_behavior {
  CMP_SCROLL_BEHAVIOR_AUTO = 0,
  CMP_SCROLL_BEHAVIOR_SMOOTH
} cmp_scroll_behavior_t;

/**
 * @brief Represents scroll-behavior.
 */
typedef struct cmp_prop_scroll_behavior {
  cmp_scroll_behavior_t behavior;
} cmp_prop_scroll_behavior_t;

/**
 * @brief Represents scroll-snap-type property.
 */
typedef struct cmp_prop_scroll_snap_type {
  char *snap_type; /**< String representation, e.g., "x mandatory", "none" */
} cmp_prop_scroll_snap_type_t;

/**
 * @brief Represents scroll-snap-align property.
 */
typedef struct cmp_prop_scroll_snap_align {
  char *snap_align; /**< String representation, e.g., "start end", "none" */
} cmp_prop_scroll_snap_align_t;

/**
 * @brief Represents scroll-snap-stop property.
 */
typedef enum cmp_scroll_snap_stop {
  CMP_SCROLL_SNAP_STOP_NORMAL = 0,
  CMP_SCROLL_SNAP_STOP_ALWAYS
} cmp_scroll_snap_stop_t;

/**
 * @brief Represents scroll-snap group.
 */
typedef struct cmp_prop_scroll_snap_group {
  cmp_prop_scroll_snap_type_t type;
  cmp_prop_scroll_snap_align_t align;
  cmp_scroll_snap_stop_t stop;
} cmp_prop_scroll_snap_group_t;

/**
 * @brief Represents scroll-padding and scroll-margin properties.
 */
typedef struct cmp_prop_scroll_padding_margin {
  char *scroll_padding; /**< Blob representation for padding */
  char *scroll_margin;  /**< Blob representation for margin */
} cmp_prop_scroll_padding_margin_t;

/**
 * @brief Represents scrollbar-gutter property.
 */
typedef enum cmp_scrollbar_gutter {
  CMP_SCROLLBAR_GUTTER_AUTO = 0,
  CMP_SCROLLBAR_GUTTER_STABLE,
  CMP_SCROLLBAR_GUTTER_STABLE_BOTH_EDGES
} cmp_scrollbar_gutter_t;

/**
 * @brief Represents scrollbar-gutter.
 */
typedef struct cmp_prop_scrollbar_gutter {
  cmp_scrollbar_gutter_t gutter;
} cmp_prop_scrollbar_gutter_t;

/**
 * @brief Represents scrollbar styling properties.
 */
typedef struct cmp_prop_scrollbar_styling {
  char *scrollbar_width;    /**< auto, thin, none */
  char *scrollbar_color;    /**< color pairs, auto */
  int has_webkit_scrollbar; /**< boolean flag indicating custom pseudo elements
                             */
} cmp_prop_scrollbar_styling_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes overflow group.
 * @param group The group to initialize.
 * @param x overflow-x.
 * @param y overflow-y.
 * @param clip_margin overflow-clip-margin string.
 * @return 0 on success.
 */
int cmp_prop_overflow_group_init(cmp_prop_overflow_group_t *group,
                                 cmp_overflow_value_t x, cmp_overflow_value_t y,
                                 const char *clip_margin);

/**
 * @brief Frees overflow group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_overflow_group_free(cmp_prop_overflow_group_t *group);

/**
 * @brief Initializes overscroll property.
 * @param prop The property to initialize.
 * @param x overscroll-behavior-x.
 * @param y overscroll-behavior-y.
 * @return 0 on success.
 */
int cmp_prop_overscroll_init(cmp_prop_overscroll_t *prop,
                             cmp_overscroll_behavior_value_t x,
                             cmp_overscroll_behavior_value_t y);

/**
 * @brief Initializes scroll-behavior property.
 * @param prop The property to initialize.
 * @param behavior behavior value.
 * @return 0 on success.
 */
int cmp_prop_scroll_behavior_init(cmp_prop_scroll_behavior_t *prop,
                                  cmp_scroll_behavior_t behavior);

/**
 * @brief Initializes scroll-snap group.
 * @param group The group to initialize.
 * @param type type string.
 * @param align align string.
 * @param stop stop value.
 * @return 0 on success.
 */
int cmp_prop_scroll_snap_group_init(cmp_prop_scroll_snap_group_t *group,
                                    const char *type, const char *align,
                                    cmp_scroll_snap_stop_t stop);

/**
 * @brief Frees scroll-snap group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_scroll_snap_group_free(cmp_prop_scroll_snap_group_t *group);

/**
 * @brief Initializes scroll-padding-margin property.
 * @param prop The property to initialize.
 * @param padding padding string.
 * @param margin margin string.
 * @return 0 on success.
 */
int cmp_prop_scroll_padding_margin_init(cmp_prop_scroll_padding_margin_t *prop,
                                        const char *padding,
                                        const char *margin);

/**
 * @brief Frees scroll-padding-margin property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_scroll_padding_margin_free(cmp_prop_scroll_padding_margin_t *prop);

/**
 * @brief Initializes scrollbar-gutter property.
 * @param prop The property to initialize.
 * @param gutter gutter value.
 * @return 0 on success.
 */
int cmp_prop_scrollbar_gutter_init(cmp_prop_scrollbar_gutter_t *prop,
                                   cmp_scrollbar_gutter_t gutter);

/**
 * @brief Initializes scrollbar styling property.
 * @param prop The property to initialize.
 * @param width width string.
 * @param color color string.
 * @param has_webkit flag.
 * @return 0 on success.
 */
int cmp_prop_scrollbar_styling_init(cmp_prop_scrollbar_styling_t *prop,
                                    const char *width, const char *color,
                                    int has_webkit);

/**
 * @brief Frees scrollbar styling property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_scrollbar_styling_free(cmp_prop_scrollbar_styling_t *prop);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_OVERFLOW_H */
