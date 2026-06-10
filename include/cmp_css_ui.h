#ifndef CMP_CSS_UI_H
#define CMP_CSS_UI_H

/* clang-format off */
#include <stddef.h>
#include "cmp_css_color.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cmp_css_ui.h
 * @brief CSS User Interface (UI) & Interaction implementation.
 */

/**
 * @brief Represents cursor property.
 */
typedef struct cmp_prop_cursor {
  char *cursor; /**< Blob string representing cursor, e.g., "pointer",
                   "url(...), auto" */
} cmp_prop_cursor_t;

/**
 * @brief Represents pointer-events and touch-action properties.
 */
typedef struct cmp_prop_pointer_touch_group {
  char *pointer_events; /**< String representation */
  char *touch_action;   /**< String representation */
} cmp_prop_pointer_touch_group_t;

/**
 * @brief Represents user-select property.
 */
typedef enum cmp_user_select {
  CMP_USER_SELECT_AUTO = 0,
  CMP_USER_SELECT_NONE,
  CMP_USER_SELECT_TEXT,
  CMP_USER_SELECT_ALL,
  CMP_USER_SELECT_CONTAIN
} cmp_user_select_t;

/**
 * @brief Represents user-select.
 */
typedef struct cmp_prop_user_select {
  cmp_user_select_t select;
} cmp_prop_user_select_t;

/**
 * @brief Represents appearance property.
 */
typedef struct cmp_prop_appearance {
  char
      *appearance; /**< String representation, e.g., "none", "auto", "button" */
} cmp_prop_appearance_t;

/**
 * @brief Represents resize property.
 */
typedef enum cmp_resize {
  CMP_RESIZE_NONE = 0,
  CMP_RESIZE_BOTH,
  CMP_RESIZE_HORIZONTAL,
  CMP_RESIZE_VERTICAL,
  CMP_RESIZE_BLOCK,
  CMP_RESIZE_INLINE
} cmp_resize_t;

/**
 * @brief Represents resize.
 */
typedef struct cmp_prop_resize {
  cmp_resize_t resize;
} cmp_prop_resize_t;

/**
 * @brief Represents zoom property.
 */
typedef struct cmp_prop_zoom {
  char *zoom; /**< String representation, e.g., "1.5", "150%", "normal" */
} cmp_prop_zoom_t;

/**
 * @brief Represents caret-color property.
 */
typedef struct cmp_prop_caret_color {
  cmp_prop_color_t color;
} cmp_prop_caret_color_t;

/**
 * @brief Represents caret-shape property.
 */
typedef enum cmp_caret_shape {
  CMP_CARET_SHAPE_AUTO = 0,
  CMP_CARET_SHAPE_BAR,
  CMP_CARET_SHAPE_BLOCK,
  CMP_CARET_SHAPE_UNDERSCORE
} cmp_caret_shape_t;

/**
 * @brief Represents caret-shape.
 */
typedef struct cmp_prop_caret_shape {
  cmp_caret_shape_t shape;
} cmp_prop_caret_shape_t;

/**
 * @brief Represents accent-color property.
 */
typedef struct cmp_prop_accent_color {
  cmp_prop_color_t color;
} cmp_prop_accent_color_t;

/**
 * @brief Represents spatial navigation properties.
 */
typedef struct cmp_prop_spatial_nav {
  char *nav_up;
  char *nav_down;
  char *nav_left;
  char *nav_right;
} cmp_prop_spatial_nav_t;

/* --- Initialization APIs --- */

/**
 * @brief Initializes cursor property.
 * @param prop The property to initialize.
 * @param cursor cursor string.
 * @return 0 on success.
 */
int cmp_prop_cursor_init(cmp_prop_cursor_t *prop, const char *cursor);

/**
 * @brief Frees cursor property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_cursor_free(cmp_prop_cursor_t *prop);

/**
 * @brief Initializes pointer-touch group.
 * @param group The group to initialize.
 * @param ptr pointer-events string.
 * @param touch touch-action string.
 * @return 0 on success.
 */
int cmp_prop_pointer_touch_group_init(cmp_prop_pointer_touch_group_t *group,
                                      const char *ptr, const char *touch);

/**
 * @brief Frees pointer-touch group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_pointer_touch_group_free(cmp_prop_pointer_touch_group_t *group);

/**
 * @brief Initializes user-select property.
 * @param prop The property to initialize.
 * @param select select value.
 * @return 0 on success.
 */
int cmp_prop_user_select_init(cmp_prop_user_select_t *prop,
                              cmp_user_select_t select);

/**
 * @brief Initializes appearance property.
 * @param prop The property to initialize.
 * @param appearance appearance string.
 * @return 0 on success.
 */
int cmp_prop_appearance_init(cmp_prop_appearance_t *prop,
                             const char *appearance);

/**
 * @brief Frees appearance property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_appearance_free(cmp_prop_appearance_t *prop);

/**
 * @brief Initializes resize property.
 * @param prop The property to initialize.
 * @param resize resize value.
 * @return 0 on success.
 */
int cmp_prop_resize_init(cmp_prop_resize_t *prop, cmp_resize_t resize);

/**
 * @brief Initializes zoom property.
 * @param prop The property to initialize.
 * @param zoom zoom string.
 * @return 0 on success.
 */
int cmp_prop_zoom_init(cmp_prop_zoom_t *prop, const char *zoom);

/**
 * @brief Frees zoom property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_zoom_free(cmp_prop_zoom_t *prop);

/**
 * @brief Initializes caret-color property.
 * @param prop The property to initialize.
 * @param color caret color.
 * @return 0 on success.
 */
int cmp_prop_caret_color_init(cmp_prop_caret_color_t *prop,
                              const cmp_prop_color_t *color);

/**
 * @brief Frees caret-color property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_caret_color_free(cmp_prop_caret_color_t *prop);

/**
 * @brief Initializes caret-shape property.
 * @param prop The property to initialize.
 * @param shape caret shape value.
 * @return 0 on success.
 */
int cmp_prop_caret_shape_init(cmp_prop_caret_shape_t *prop,
                              cmp_caret_shape_t shape);

/**
 * @brief Initializes accent-color property.
 * @param prop The property to initialize.
 * @param color accent color.
 * @return 0 on success.
 */
int cmp_prop_accent_color_init(cmp_prop_accent_color_t *prop,
                               const cmp_prop_color_t *color);

/**
 * @brief Frees accent-color property.
 * @param prop The property to free.
 * @return 0 on success.
 */
int cmp_prop_accent_color_free(cmp_prop_accent_color_t *prop);

/**
 * @brief Initializes spatial nav group.
 * @param group The group to initialize.
 * @param up nav-up.
 * @param down nav-down.
 * @param left nav-left.
 * @param right nav-right.
 * @return 0 on success.
 */
int cmp_prop_spatial_nav_init(cmp_prop_spatial_nav_t *group, const char *up,
                              const char *down, const char *left,
                              const char *right);

/**
 * @brief Frees spatial nav group.
 * @param group The group to free.
 * @return 0 on success.
 */
int cmp_prop_spatial_nav_free(cmp_prop_spatial_nav_t *group);

#ifdef __cplusplus
}
#endif

#endif /* CMP_CSS_UI_H */
