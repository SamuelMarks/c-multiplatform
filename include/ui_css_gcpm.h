#ifndef UI_CSS_GCPM_H
#define UI_CSS_GCPM_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
/* clang-format on */

struct ui_css_computed_style; /* Forward declare */

/**
 * @brief Represents CSS Generated Content for Paged Media (GCPM) properties.
 */
struct ui_css_gcpm_properties {
  char *string_set;
  char *bookmark_label;
  int bookmark_level; /* 0 means 'none' */
  int bookmark_state; /* 0 = closed, 1 = open */
};

/**
 * @brief Parses CSS Generated Content for Paged Media properties.
 *
 * @param style The computed style.
 * @param out_props Pointer to the properties struct.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_css_gcpm_parse(const struct ui_css_computed_style *style,
                                struct ui_css_gcpm_properties *out_props);

/**
 * @brief Destroys string allocations within GCPM properties.
 *
 * @param props The struct to clean up.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_css_gcpm_properties_cleanup(struct ui_css_gcpm_properties *props);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSS_GCPM_H */
