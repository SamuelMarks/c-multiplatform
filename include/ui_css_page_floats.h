/**
 * \file ui_css_page_floats.h
 * \brief CSS Page Floats definitions and parsing.
 * \author UI Framework Team
 * \date 2026
 */

#ifndef UI_CSS_PAGE_FLOATS_H
#define UI_CSS_PAGE_FLOATS_H

/**
 * \defgroup ui_css_page_floats CSS Page Floats
 * \brief Types and functions for CSS page floats properties.
 * \{
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * \brief Reference box for float positioning.
 */
enum ui_css_float_reference {
  UI_CSS_FLOAT_REFERENCE_INLINE, /**< Inline reference box. */
  UI_CSS_FLOAT_REFERENCE_COLUMN, /**< Column reference box. */
  UI_CSS_FLOAT_REFERENCE_REGION, /**< Region reference box. */
  UI_CSS_FLOAT_REFERENCE_PAGE    /**< Page reference box. */
};

/**
 * \brief Types of float deferment.
 */
enum ui_css_float_defer {
  UI_CSS_FLOAT_DEFER_NONE,   /**< No float deferment. */
  UI_CSS_FLOAT_DEFER_INTEGER /**< Integer float deferment. */
};

/**
 * \brief Represents clear-after property.
 */
enum ui_css_clear_after {
  UI_CSS_CLEAR_AFTER_NONE,       /**< Clear after none. */
  UI_CSS_CLEAR_AFTER_LEFT,       /**< Clear after left. */
  UI_CSS_CLEAR_AFTER_RIGHT,      /**< Clear after right. */
  UI_CSS_CLEAR_AFTER_BOTH,       /**< Clear after both. */
  UI_CSS_CLEAR_AFTER_START,      /**< Clear after start. */
  UI_CSS_CLEAR_AFTER_END,        /**< Clear after end. */
  UI_CSS_CLEAR_AFTER_DESCENDANTS /**< Clear after descendants. */
};

/**
 * \brief Encapsulates page float properties for a layout node.
 */
struct ui_css_page_floats_properties {
  enum ui_css_float_reference float_reference; /**< Float reference box. */
  enum ui_css_float_defer float_defer_type;    /**< Float deferment type. */
  int float_defer_value;                       /**< Float deferment value. */
  enum ui_css_clear_after clear_after;         /**< Clear after behavior. */
};

struct ui_css_computed_style; /* Forward declare */

/**
 * \brief Parses CSS Page Floats properties from a computed style.
 *
 * \param style The computed style.
 * \param out_props Pointer to the struct to populate.
 * \return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_css_page_floats_parse(const struct ui_css_computed_style *style,
                         struct ui_css_page_floats_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** \} */

#endif /* UI_CSS_PAGE_FLOATS_H */
