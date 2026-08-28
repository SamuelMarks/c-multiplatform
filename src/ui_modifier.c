/**
 * @file ui_modifier.c
 * @brief Implementation of style and class modifiers for UI components.
 * @details Provides the ability to compose and apply CSS classes and styles
 * dynamically.
 */
/* clang-format off */
#include "ui_modifier.h"
#include "ui_internal_mem.h"
#include <stddef.h>
#include <string.h>
/* clang-format on */

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * @struct ui_modifier_class
 * @brief Linked list node representing an added CSS class.
 */
struct ui_modifier_class {
  char *name;                     /**< Name of the class */
  struct ui_modifier_class *next; /**< Next class in list */
};

/**
 * @struct ui_modifier_style
 * @brief Linked list node representing an inline CSS style.
 */
struct ui_modifier_style {
  char *property;                 /**< Style property name */
  char *value;                    /**< Style property value */
  struct ui_modifier_style *next; /**< Next style in list */
};

/**
 * @struct ui_modifier
 * @brief Modifer context storing classes and inline styles.
 */
struct ui_modifier {
  struct ui_modifier_class *first_class; /**< Head of class list */
  struct ui_modifier_class *last_class;  /**< Tail of class list */
  struct ui_modifier_style *first_style; /**< Head of style list */
  struct ui_modifier_style *last_style;  /**< Tail of style list */
};

/**
 * @brief Creates a new modifier configuration.
 * @param[out] out_modifier Pointer to store the created modifier.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_modifier_create(struct ui_modifier **out_modifier) {
  struct ui_modifier *modifier;

  if (!out_modifier) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  modifier =
      (struct ui_modifier *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_modifier));
  if (!modifier) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  modifier->first_class = NULL;
  modifier->last_class = NULL;
  modifier->first_style = NULL;
  modifier->last_style = NULL;

  *out_modifier = modifier;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a modifier configuration.
 * @param[in,out] modifier The modifier to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_modifier_destroy(struct ui_modifier *modifier) {
  struct ui_modifier_class *c_curr;
  struct ui_modifier_class *c_next;
  struct ui_modifier_style *s_curr;
  struct ui_modifier_style *s_next;

  if (!modifier) {
    return UI_ERROR_NONE;
  }

  c_curr = modifier->first_class;
  while (c_curr) {
    c_next = c_curr->next;
    if (c_curr->name) {
      C_MULTIPLATFORM_FREE(c_curr->name);
    }
    C_MULTIPLATFORM_FREE(c_curr);
    c_curr = c_next;
  }

  s_curr = modifier->first_style;
  while (s_curr) {
    s_next = s_curr->next;
    if (s_curr->property) {
      C_MULTIPLATFORM_FREE(s_curr->property);
    }
    if (s_curr->value) {
      C_MULTIPLATFORM_FREE(s_curr->value);
    }
    C_MULTIPLATFORM_FREE(s_curr);
    s_curr = s_next;
  }

  C_MULTIPLATFORM_FREE(modifier);
  return UI_ERROR_NONE;
}

/**
 * @brief Duplicates a string using standard allocation.
 * @param[in] src The string to copy.
 * @param[out] out_dest Pointer to store the copied string.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t str_dup(const char *src, char **out_dest) {
  size_t len;
  char *dest;

  len = strlen(src);
  dest = (char *)C_MULTIPLATFORM_MALLOC(len + 1);
  if (!dest) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

#if defined(_MSC_VER)
  strcpy_s(dest, len + 1, src);
#else
  strcpy(dest, src);
#endif

  *out_dest = dest;
  return UI_ERROR_NONE;
}

/**
 * @brief Adds a CSS class to the modifier.
 * @param[in,out] modifier The modifier configuration.
 * @param[in] class_name The CSS class name to add.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_modifier_add_class(struct ui_modifier *modifier,
                                 const char *class_name) {
  struct ui_modifier_class *new_class;
  ui_error_t rc;

  if (!modifier || !class_name) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_class = (struct ui_modifier_class *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_modifier_class));
  if (!new_class) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  new_class->name = NULL;
  new_class->next = NULL;

  rc = str_dup(class_name, &new_class->name);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(new_class);
    return rc;
  }

  if (!modifier->first_class) {
    modifier->first_class = new_class;
  } else {
    modifier->last_class->next = new_class;
  }
  modifier->last_class = new_class;

  return UI_ERROR_NONE;
}

/**
 * @brief Adds an inline CSS style to the modifier.
 * @param[in,out] modifier The modifier configuration.
 * @param[in] property_name The CSS property name.
 * @param[in] property_value The CSS property value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_modifier_add_style(struct ui_modifier *modifier,
                                 const char *property_name,
                                 const char *property_value) {
  struct ui_modifier_style *new_style;
  ui_error_t rc;

  if (!modifier || !property_name || !property_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_style = (struct ui_modifier_style *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_modifier_style));
  if (!new_style) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  new_style->property = NULL;
  new_style->value = NULL;
  new_style->next = NULL;

  rc = str_dup(property_name, &new_style->property);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(new_style);
    return rc;
  }

  rc = str_dup(property_value, &new_style->value);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(new_style->property);
    C_MULTIPLATFORM_FREE(new_style);
    return rc;
  }

  if (!modifier->first_style) {
    modifier->first_style = new_style;
  } else {
    modifier->last_style->next = new_style;
  }
  modifier->last_style = new_style;

  return UI_ERROR_NONE;
}

/**
 * @brief Applies the modifier's classes and styles to a target component's
 * shadow root.
 * @param[in] modifier The modifier configuration.
 * @param[in,out] component The target UI component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_modifier_apply(const struct ui_modifier *modifier,
                             struct ui_component *component) {
  const char *existing_class = NULL;
  const char *existing_style = NULL;
  struct ui_modifier_class *c_curr;
  struct ui_modifier_style *s_curr;
  char *new_class_str = NULL;
  char *new_style_str = NULL;
  size_t class_len = 0;
  size_t style_len = 0;
  ui_error_t rc = UI_ERROR_NONE;

  if (!modifier || !component || !component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Calculate length for new class string */
  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(component->shadow_root,
                                                   "class", &existing_class);
    if (attr_rc != UI_ERROR_NONE) {
      (void)attr_rc;
    }
    if (attr_rc == UI_ERROR_NONE) {
      if (existing_class[0] != '\0') {
        class_len = strlen(existing_class) + 1; /* +1 for space */
      }
    }
  }

  c_curr = modifier->first_class;
  while (c_curr) {
    class_len += strlen(c_curr->name) + 1; /* +1 for space or null terminator */
    c_curr = c_curr->next;
  }

  if (class_len > 0) {
    new_class_str = (char *)C_MULTIPLATFORM_MALLOC(class_len + 1);
    if (!new_class_str) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    new_class_str[0] = '\0';

    if (existing_class && existing_class[0] != '\0') {
#if defined(_MSC_VER)
      strcat_s(new_class_str, class_len + 1, existing_class);
#else
      strcat(new_class_str, existing_class);
#endif
    }

    c_curr = modifier->first_class;
    while (c_curr) {
      if (new_class_str[0] != '\0') {
#if defined(_MSC_VER)
        strcat_s(new_class_str, class_len + 1, " ");
#else
        strcat(new_class_str, " ");
#endif
      }
#if defined(_MSC_VER)
      strcat_s(new_class_str, class_len + 1, c_curr->name);
#else
      strcat(new_class_str, c_curr->name);
#endif
      c_curr = c_curr->next;
    }

    rc = ui_dom_node_set_attribute(component->shadow_root, "class",
                                   new_class_str);
    if (rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(new_class_str);
      (void)new_style_str;
      return rc;
    }
  }

  /* Calculate length for new style string */
  {
    ui_error_t attr_rc = ui_dom_node_get_attribute(component->shadow_root,
                                                   "style", &existing_style);
    if (attr_rc != UI_ERROR_NONE) {
      (void)attr_rc;
    }
    if (attr_rc == UI_ERROR_NONE) {
      if (existing_style[0] != '\0') {
        style_len = strlen(existing_style) + 1; /* +1 for space */
      }
    }
  }

  s_curr = modifier->first_style;
  while (s_curr) {
    /* property: value; */
    style_len += strlen(s_curr->property) + 2 + strlen(s_curr->value) + 2;
    s_curr = s_curr->next;
  }

  if (style_len > 0) {
    new_style_str = (char *)C_MULTIPLATFORM_MALLOC(style_len + 1);
    if (!new_style_str) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
    new_style_str[0] = '\0';

    if (existing_style && existing_style[0] != '\0') {
#if defined(_MSC_VER)
      strcat_s(new_style_str, style_len + 1, existing_style);
#else
      strcat(new_style_str, existing_style);
#endif
    }

    s_curr = modifier->first_style;
    while (s_curr) {
      if (new_style_str[0] != '\0' &&
          new_style_str[strlen(new_style_str) - 1] != ';') {
#if defined(_MSC_VER)
        strcat_s(new_style_str, style_len + 1, "; ");
#else
        strcat(new_style_str, "; ");
#endif
      } else if (new_style_str[0] != '\0') {
#if defined(_MSC_VER)
        strcat_s(new_style_str, style_len + 1, " ");
#else
        strcat(new_style_str, " ");
#endif
      }
#if defined(_MSC_VER)
      strcat_s(new_style_str, style_len + 1, s_curr->property);
      strcat_s(new_style_str, style_len + 1, ": ");
      strcat_s(new_style_str, style_len + 1, s_curr->value);
      strcat_s(new_style_str, style_len + 1, ";");
#else
      strcat(new_style_str, s_curr->property);
      strcat(new_style_str, ": ");
      strcat(new_style_str, s_curr->value);
      strcat(new_style_str, ";");
#endif
      s_curr = s_curr->next;
    }

    rc = ui_dom_node_set_attribute(component->shadow_root, "style",
                                   new_style_str);
    if (rc != UI_ERROR_NONE) {
      C_MULTIPLATFORM_FREE(new_class_str);
      C_MULTIPLATFORM_FREE(new_style_str);
      return rc;
    }
  }

cleanup:
  if (new_class_str) {
    C_MULTIPLATFORM_FREE(new_class_str);
  }
  if (new_style_str) {
    C_MULTIPLATFORM_FREE(new_style_str);
  }
  return rc;
}
