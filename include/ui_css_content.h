/**
 * @file ui_css_content.h
 * @brief CSS Content definitions and parsing.
 * @author UI Framework Team
 * @date 2026
 */

#ifndef UI_CSS_CONTENT_H
#define UI_CSS_CONTENT_H

/**
 * @defgroup ui_css_content CSS Content
 * @brief Types and functions for CSS content properties.
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_css_values.h"
/* clang-format on */

/**
 * \brief Types of CSS content items.
 */
enum ui_css_content_item_type {
  UI_CSS_CONTENT_ITEM_STRING,        /**< String content. */
  UI_CSS_CONTENT_ITEM_IMAGE,         /**< Image content. */
  UI_CSS_CONTENT_ITEM_ATTR,          /**< Attribute content. */
  UI_CSS_CONTENT_ITEM_COUNTER,       /**< Counter content. */
  UI_CSS_CONTENT_ITEM_COUNTERS,      /**< Counters content. */
  UI_CSS_CONTENT_ITEM_OPEN_QUOTE,    /**< Open quote content. */
  UI_CSS_CONTENT_ITEM_CLOSE_QUOTE,   /**< Close quote content. */
  UI_CSS_CONTENT_ITEM_NO_OPEN_QUOTE, /**< No open quote content. */
  UI_CSS_CONTENT_ITEM_NO_CLOSE_QUOTE /**< No close quote content. */
};

/**
 * \brief Represents a single item in a CSS content property list.
 */
struct ui_css_content_item {
  enum ui_css_content_item_type type; /**< Type of the content item. */
  /** \brief Union for content item values */
  union {
    char string_val[128];      /**< String value */
    struct ui_css_image image; /**< Image value */
    char attr_name[64];        /**< Attribute name */
    /** @brief Counter properties */
    struct {
      char name[64];  /**< Counter name */
      char style[32]; /**< Counter style (e.g. "upper-roman" or empty for
                         default) */
    } counter;        /**< Counter properties */

    /** @brief Counters properties */
    struct {
      char name[64];                /**< Counter name */
      char separator[16];           /**< Separator string */
      char style[32];               /**< Counter style */
    } counters;                     /**< Counters properties */
  } value;                          /**< The actual value. */
  struct ui_css_content_item *next; /**< Pointer to the next item. */
};

/**
 * \brief Represents the parsed value of the CSS content property.
 */
struct ui_css_content {
  int is_none;                       /**< Non-zero if content is 'none'. */
  int is_normal;                     /**< Non-zero if content is 'normal'. */
  struct ui_css_content_item *items; /**< Linked list of content items. */
};

/**
 * \brief Parses the CSS 'content' property.
 *
 * \param str The string to parse.
 * \param out_content Pointer to receive the parsed content object.
 * \return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_parse_content(const char *str,
                                struct ui_css_content *out_content);

/**
 * \brief Destroys a parsed CSS content object, freeing any allocated strings or
 * list items.
 *
 * \param content The content object to destroy.
 */
ui_error_t ui_css_content_destroy(struct ui_css_content *content);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_CONTENT_H */
