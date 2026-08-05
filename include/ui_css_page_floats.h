#ifndef UI_CSS_PAGE_FLOATS_H
#define UI_CSS_PAGE_FLOATS_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Reference box for float positioning.
 */
enum ui_css_float_reference {
  UI_CSS_FLOAT_REFERENCE_INLINE,
  UI_CSS_FLOAT_REFERENCE_COLUMN,
  UI_CSS_FLOAT_REFERENCE_REGION,
  UI_CSS_FLOAT_REFERENCE_PAGE
};

/**
 * @brief Types of float deferment.
 */
enum ui_css_float_defer { UI_CSS_FLOAT_DEFER_NONE, UI_CSS_FLOAT_DEFER_INTEGER };

/**
 * @brief Represents clear-after property.
 */
enum ui_css_clear_after {
  UI_CSS_CLEAR_AFTER_NONE,
  UI_CSS_CLEAR_AFTER_LEFT,
  UI_CSS_CLEAR_AFTER_RIGHT,
  UI_CSS_CLEAR_AFTER_BOTH,
  UI_CSS_CLEAR_AFTER_START,
  UI_CSS_CLEAR_AFTER_END,
  UI_CSS_CLEAR_AFTER_DESCENDANTS
};

/**
 * @brief Encapsulates page float properties for a layout node.
 */
struct ui_css_page_floats_properties {
  enum ui_css_float_reference float_reference;
  enum ui_css_float_defer float_defer_type;
  int float_defer_value;
  enum ui_css_clear_after clear_after;
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses CSS Page Floats properties from a computed style.
 *
 * @param style The computed style.
 * @param out_props Pointer to the struct to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_css_page_floats_parse(const struct ui_css_computed_style *style,
                         struct ui_css_page_floats_properties *out_props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_PAGE_FLOATS_H */
