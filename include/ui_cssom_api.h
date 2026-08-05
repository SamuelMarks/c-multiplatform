#ifndef UI_CSSOM_API_H
#define UI_CSSOM_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_cssom.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief CSSOM Level 1: Inserts a new CSS rule into the stylesheet at the
 * specified index.
 *
 * @param stylesheet The stylesheet.
 * @param css_text The CSS text of the rule to insert.
 * @param index The 0-based index at which to insert the rule.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_cssom_insert_rule(struct ui_css_stylesheet *stylesheet,
                                const char *css_text, size_t index);

/**
 * @brief CSSOM Level 1: Deletes a CSS rule from the stylesheet at the specified
 * index.
 *
 * @param stylesheet The stylesheet.
 * @param index The 0-based index of the rule to delete.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_cssom_delete_rule(struct ui_css_stylesheet *stylesheet,
                                size_t index);

/**
 * @brief CSSOM Level 1: Sets a property value in a CSS style rule.
 *
 * @param rule The CSS rule.
 * @param property_name The property to set.
 * @param property_value The value.
 * @param is_important Priority flag (1 for important, 0 for normal).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_cssom_set_property(struct ui_css_rule *rule,
                                 const char *property_name,
                                 const char *property_value, int is_important);

/**
 * @brief CSSOM Level 1: Removes a property from a CSS style rule.
 *
 * @param rule The CSS rule.
 * @param property_name The property to remove.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_cssom_remove_property(struct ui_css_rule *rule,
                                    const char *property_name);

/**
 * @brief CSSOM Level 1: Retrieves the value of a property from a CSS style
 * rule.
 *
 * @param rule The CSS rule.
 * @param property_name The property name.
 * @param out_value Pointer to receive the property value string (borrowed
 * reference).
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if absent.
 */
ui_error_t ui_cssom_get_property_value(const struct ui_css_rule *rule,
                                       const char *property_name,
                                       const char **out_value);

/**
 * @brief CSSOM Level 1: Retrieves the priority of a property from a CSS style
 * rule.
 *
 * @param rule The CSS rule.
 * @param property_name The property name.
 * @param out_is_important Pointer to receive the important flag.
 * @return UI_ERROR_NONE on success, UI_ERROR_NOT_FOUND if absent.
 */
ui_error_t ui_cssom_get_property_priority(const struct ui_css_rule *rule,
                                          const char *property_name,
                                          int *out_is_important);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSSOM_API_H */
