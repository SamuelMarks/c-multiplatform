#ifndef UI_CSS_PAGE_H
#define UI_CSS_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

/**
 * @brief Represents predefined page sizes (CSS Paged Media).
 */
enum ui_css_page_size_type {
  UI_CSS_PAGE_SIZE_AUTO,
  UI_CSS_PAGE_SIZE_PORTRAIT,
  UI_CSS_PAGE_SIZE_LANDSCAPE,
  UI_CSS_PAGE_SIZE_A4,
  UI_CSS_PAGE_SIZE_A5,
  UI_CSS_PAGE_SIZE_A3,
  UI_CSS_PAGE_SIZE_B5,
  UI_CSS_PAGE_SIZE_B4,
  UI_CSS_PAGE_SIZE_JIS_B5,
  UI_CSS_PAGE_SIZE_JIS_B4,
  UI_CSS_PAGE_SIZE_LETTER,
  UI_CSS_PAGE_SIZE_LEGAL,
  UI_CSS_PAGE_SIZE_LEDGER
};

/**
 * @brief Represents crop/cross marks for page rendering.
 */
enum ui_css_page_marks {
  UI_CSS_PAGE_MARKS_NONE = 0,
  UI_CSS_PAGE_MARKS_CROP = 1,
  UI_CSS_PAGE_MARKS_CROSS = 2,
  UI_CSS_PAGE_MARKS_BOTH = 3
};

/**
 * @brief Contains properties defined within an @page rule.
 */
struct ui_css_page_properties {
  enum ui_css_page_size_type size;
  enum ui_css_page_marks marks;
  char *page_name;   /* custom identifier for `page` property */
  float bleed_value; /* simplified to a float for now */
  int bleed_is_auto;
};

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Parses CSS Paged Media properties from a computed style.
 *
 * @param style The computed style.
 * @param out_props Pointer to the struct to populate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_page_parse(const struct ui_css_computed_style *style,
                             struct ui_css_page_properties *out_props);

/**
 * @brief Destroys string allocations within page properties.
 *
 * @param props The struct to clean up.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_css_page_properties_cleanup(struct ui_css_page_properties *props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_PAGE_H */
