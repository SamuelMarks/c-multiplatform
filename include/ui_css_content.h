#ifndef UI_CSS_CONTENT_H
#define UI_CSS_CONTENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * @brief Types of CSS content items.
 */
enum ui_css_content_item_type {
  UI_CSS_CONTENT_ITEM_STRING,
  UI_CSS_CONTENT_ITEM_IMAGE,
  UI_CSS_CONTENT_ITEM_ATTR,
  UI_CSS_CONTENT_ITEM_COUNTER,
  UI_CSS_CONTENT_ITEM_COUNTERS,
  UI_CSS_CONTENT_ITEM_OPEN_QUOTE,
  UI_CSS_CONTENT_ITEM_CLOSE_QUOTE,
  UI_CSS_CONTENT_ITEM_NO_OPEN_QUOTE,
  UI_CSS_CONTENT_ITEM_NO_CLOSE_QUOTE
};

/**
 * @brief Represents a single item in a CSS content property list.
 */
struct ui_css_content_item {
  enum ui_css_content_item_type type;
  /** \brief union */
  union {
    char string_val[128];
    struct ui_css_image image;
    char attr_name[64];
    /** \brief struct */
    struct {
      char name[64];
      char style[32]; /* e.g. "upper-roman" or empty for default */
    } counter;
    /** \brief struct */
    struct {
      char name[64];
      char separator[16];
      char style[32];
    } counters;
  } value;
  struct ui_css_content_item *next;
};

/**
 * @brief Represents the parsed value of the CSS content property.
 */
struct ui_css_content {
  int is_none;
  int is_normal;
  struct ui_css_content_item *items;
};

/**
 * @brief Parses the CSS 'content' property.
 *
 * @param str The string to parse.
 * @param out_content Pointer to receive the parsed content object.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_parse_content(const char *str,
                                   struct ui_css_content *out_content);

/**
 * @brief Destroys a parsed CSS content object, freeing any allocated strings or
 * list items.
 *
 * @param content The content object to destroy.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_content_destroy(struct ui_css_content *content);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_CONTENT_H */
