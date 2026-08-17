/**
 * @file ui_css_page.h
 * @brief CSS Paged Media properties and parsing.
 *
 * This header defines structures and functions for handling CSS Paged
 * Media properties such as `@page` rules, page sizes, and crop/cross marks.
 */

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
  UI_CSS_PAGE_SIZE_AUTO,      /**< Automatic page size. */
  UI_CSS_PAGE_SIZE_PORTRAIT,  /**< Portrait orientation. */
  UI_CSS_PAGE_SIZE_LANDSCAPE, /**< Landscape orientation. */
  UI_CSS_PAGE_SIZE_A4,        /**< A4 page size. */
  UI_CSS_PAGE_SIZE_A5,        /**< A5 page size. */
  UI_CSS_PAGE_SIZE_A3,        /**< A3 page size. */
  UI_CSS_PAGE_SIZE_B5,        /**< B5 page size. */
  UI_CSS_PAGE_SIZE_B4,        /**< B4 page size. */
  UI_CSS_PAGE_SIZE_JIS_B5,    /**< JIS B5 page size. */
  UI_CSS_PAGE_SIZE_JIS_B4,    /**< JIS B4 page size. */
  UI_CSS_PAGE_SIZE_LETTER,    /**< Letter page size. */
  UI_CSS_PAGE_SIZE_LEGAL,     /**< Legal page size. */
  UI_CSS_PAGE_SIZE_LEDGER     /**< Ledger page size. */
};

/**
 * @brief Represents crop/cross marks for page rendering.
 */
enum ui_css_page_marks {
  UI_CSS_PAGE_MARKS_NONE = 0,  /**< No marks. */
  UI_CSS_PAGE_MARKS_CROP = 1,  /**< Crop marks. */
  UI_CSS_PAGE_MARKS_CROSS = 2, /**< Cross marks. */
  UI_CSS_PAGE_MARKS_BOTH = 3   /**< Both crop and cross marks. */
};

/**
 * @brief Contains properties defined within an \@page rule.
 */
struct ui_css_page_properties {
  enum ui_css_page_size_type size; /**< The size of the page. */
  enum ui_css_page_marks marks;    /**< The marks for the page. */
  char *page_name;   /**< Custom identifier for `page` property. */
  float bleed_value; /**< The bleed value. Simplified to a float for now. */
  int bleed_is_auto; /**< Indicates if the bleed is automatically calculated (1
                        for auto, 0 otherwise). */
};

/**
 * @brief Forward declaration of the computed style structure.
 */
struct ui_css_computed_style;

/**
 * @brief Parses CSS Paged Media properties from a computed style.
 *
 * @param style Pointer to the computed style.
 * @param out_props Pointer to the structure to populate with parsed properties.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_page_parse(const struct ui_css_computed_style *style,
                             struct ui_css_page_properties *out_props);

/**
 * @brief Destroys string allocations within page properties.
 *
 * @param props Pointer to the properties structure to clean up.
 * @return `UI_ERROR_NONE` on success, or an appropriate error code on failure.
 */
ui_error_t ui_css_page_properties_cleanup(struct ui_css_page_properties *props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_PAGE_H */
