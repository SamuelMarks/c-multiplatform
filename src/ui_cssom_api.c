/* clang-format off */
#include "../include/ui_cssom_api.h"
#include "../include/ui_css_parser.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

/*
 * @brief ui_cssom_insert_rule.
 * @param stylesheet Parameter stylesheet.
 * @param css_text Parameter css_text.
 * @param index Parameter index.
 * @return Return value.
 */
ui_error_t ui_cssom_insert_rule(struct ui_css_stylesheet *stylesheet,
                                const char *css_text, size_t index) {
  struct ui_css_stylesheet *temp_sheet = NULL;
  struct ui_css_rule *parsed_rule = NULL;
  struct ui_css_rule *prev = NULL;
  struct ui_css_rule *curr = NULL;
  size_t i = 0;
  ui_error_t rc;

  if (!stylesheet || !css_text)
    return UI_ERROR_INVALID_ARGUMENT;

  /* Use the existing parser to parse the single rule */
  rc = ui_css_parse_stylesheet(css_text, &temp_sheet);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Steal the first rule */
  parsed_rule = temp_sheet->rules;
  if (!parsed_rule) {
    ui_css_stylesheet_destroy(temp_sheet);
    return UI_ERROR_INVALID_ARGUMENT; /* No rule parsed */
  }
  temp_sheet->rules = parsed_rule->next; /* Detach */
  parsed_rule->next = NULL;

  /* Insert at index */
  curr = stylesheet->rules;
  while (curr && i < index) {
    prev = curr;
    curr = curr->next;
    i++;
  }

  if (i < index) {
    /* Index out of bounds */
    ui_css_rule_destroy(parsed_rule);
    ui_css_stylesheet_destroy(temp_sheet);
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  if (prev) {
    prev->next = parsed_rule;
  } else {
    stylesheet->rules = parsed_rule;
  }
  parsed_rule->next = curr;

  ui_css_stylesheet_destroy(temp_sheet);
  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_delete_rule.
 * @param stylesheet Parameter stylesheet.
 * @param index Parameter index.
 * @return Return value.
 */
ui_error_t ui_cssom_delete_rule(struct ui_css_stylesheet *stylesheet,
                                size_t index) {
  struct ui_css_rule *prev = NULL;
  struct ui_css_rule *curr = NULL;
  size_t i = 0;

  if (!stylesheet)
    return UI_ERROR_INVALID_ARGUMENT;

  curr = stylesheet->rules;
  while (curr && i < index) {
    prev = curr;
    curr = curr->next;
    i++;
  }

  if (!curr)
    return UI_ERROR_OUT_OF_BOUNDS;

  if (prev) {
    prev->next = curr->next;
  } else {
    stylesheet->rules = curr->next;
  }

  curr->next = NULL;
  ui_css_rule_destroy(curr);
  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_set_property.
 * @param rule Parameter rule.
 * @param property_name Parameter property_name.
 * @param property_value Parameter property_value.
 * @param is_important Parameter is_important.
 * @return Return value.
 */
ui_error_t ui_cssom_set_property(struct ui_css_rule *rule,
                                 const char *property_name,
                                 const char *property_value, int is_important) {
  struct ui_css_declaration *decl = NULL;
  size_t val_len;

  if (!rule || !property_name || !property_value)
    return UI_ERROR_INVALID_ARGUMENT;

  decl = rule->declarations;
  while (decl) {
    if (strcmp(decl->property_name, property_name) == 0) {
      C_MULTIPLATFORM_FREE(decl->property_value);
      val_len = strlen(property_value);
      decl->property_value = (char *)C_MULTIPLATFORM_MALLOC(val_len + 1);
      if (!decl->property_value)
        return UI_ERROR_OUT_OF_MEMORY;
#if defined(_MSC_VER)
      strcpy_s(decl->property_value, val_len + 1, property_value);
#else
      strcpy(decl->property_value, property_value);
#endif
      decl->is_important = is_important;
      return UI_ERROR_NONE;
    }
    decl = decl->next;
  }

  /* Not found, append it */
  return ui_css_rule_append_declaration(rule, property_name, property_value,
                                        is_important);
}

/*
 * @brief ui_cssom_remove_property.
 * @param rule Parameter rule.
 * @param property_name Parameter property_name.
 * @return Return value.
 */
ui_error_t ui_cssom_remove_property(struct ui_css_rule *rule,
                                    const char *property_name) {
  struct ui_css_declaration *prev = NULL;
  struct ui_css_declaration *curr = NULL;

  if (!rule || !property_name)
    return UI_ERROR_INVALID_ARGUMENT;

  curr = rule->declarations;
  while (curr) {
    if (strcmp(curr->property_name, property_name) == 0) {
      if (prev) {
        prev->next = curr->next;
      } else {
        rule->declarations = curr->next;
      }
      C_MULTIPLATFORM_FREE(curr->property_name);
      C_MULTIPLATFORM_FREE(curr->property_value);
      C_MULTIPLATFORM_FREE(curr);
      return UI_ERROR_NONE;
    }
    prev = curr;
    curr = curr->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/*
 * @brief ui_cssom_get_property_value.
 * @param rule Parameter rule.
 * @param property_name Parameter property_name.
 * @param out_value Parameter out_value.
 * @return Return value.
 */
ui_error_t ui_cssom_get_property_value(const struct ui_css_rule *rule,
                                       const char *property_name,
                                       const char **out_value) {
  struct ui_css_declaration *decl = NULL;

  if (!rule || !property_name || !out_value)
    return UI_ERROR_INVALID_ARGUMENT;

  decl = rule->declarations;
  while (decl) {
    if (strcmp(decl->property_name, property_name) == 0) {
      *out_value = decl->property_value;
      return UI_ERROR_NONE;
    }
    decl = decl->next;
  }

  return UI_ERROR_NOT_FOUND;
}

/*
 * @brief ui_cssom_get_property_priority.
 * @param rule Parameter rule.
 * @param property_name Parameter property_name.
 * @param out_is_important Parameter out_is_important.
 * @return Return value.
 */
ui_error_t ui_cssom_get_property_priority(const struct ui_css_rule *rule,
                                          const char *property_name,
                                          int *out_is_important) {
  struct ui_css_declaration *decl = NULL;

  if (!rule || !property_name || !out_is_important)
    return UI_ERROR_INVALID_ARGUMENT;

  decl = rule->declarations;
  while (decl) {
    if (strcmp(decl->property_name, property_name) == 0) {
      *out_is_important = decl->is_important;
      return UI_ERROR_NONE;
    }
    decl = decl->next;
  }

  return UI_ERROR_NOT_FOUND;
}
