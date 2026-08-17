/**
 * @file ui_css_view_transition.h
 * @brief CSS View Transitions properties and parsing.
 *
 * This header defines structures and functions for handling CSS View
 * Transitions properties, such as view-transition-name and
 * view-transition-class.
 */

#ifndef UI_CSS_VIEW_TRANSITION_H
#define UI_CSS_VIEW_TRANSITION_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief CSS view-transition-name property.
 */
struct ui_css_view_transition_name {
  int is_none;    /**< 1 if view-transition-name is 'none', 0 otherwise. */
  char name[256]; /**< The custom identifier. */
};

/**
 * @brief A single class name in view-transition-class.
 */
struct ui_css_view_transition_class_name {
  char name[256]; /**< The class name. */
  struct ui_css_view_transition_class_name
      *next; /**< Pointer to the next class name in the list. */
};

/**
 * @brief CSS view-transition-class property.
 */
struct ui_css_view_transition_class {
  int is_none; /**< 1 if view-transition-class is 'none', 0 otherwise. */
  struct ui_css_view_transition_class_name
      *names; /**< Linked list of transition class names. */
};

/**
 * @brief Parses the view-transition-name CSS property.
 *
 * @param str The string to parse.
 * @param out_name Pointer to receive the parsed name.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t
ui_css_parse_view_transition_name(const char *str,
                                  struct ui_css_view_transition_name *out_name);

/**
 * @brief Parses the view-transition-class CSS property.
 *
 * @param str The string to parse.
 * @param out_class Pointer to receive the parsed classes.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_parse_view_transition_class(
    const char *str, struct ui_css_view_transition_class *out_class);

/**
 * @brief Destroys a parsed view-transition-class list.
 *
 * @param vt_class Pointer to the class struct to free.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code.
 */
ui_error_t ui_css_view_transition_class_destroy(
    struct ui_css_view_transition_class *vt_class);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_VIEW_TRANSITION_H */
