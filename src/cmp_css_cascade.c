/* clang-format off */
#include "cmp_css_cascade.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Helper to duplicate a string using safe CRT if available.
 */
static int cmp_strdup_safe(const char *src, char **dst) {
  size_t len;
  if (!dst)
    return -1;
  if (!src) {
    *dst = NULL;
    return 0;
  }
  len = strlen(src);
  *dst = (char *)malloc(len + 1);
  if (!*dst)
    return -1;
#if defined(_MSC_VER)
  strcpy_s(*dst, len + 1, src);
#else
  strcpy(*dst, src);
#endif
  return 0;
}

int cmp_specificity_compute(const char *selector_str,
                            cmp_specificity_t *out_specificity) {
  if (!selector_str || !out_specificity)
    return -1;
  /* Placeholder logic: all specificities are zero unless properly parsed */
  out_specificity->a = 0;
  out_specificity->b = 0;
  out_specificity->c = 0;
  return 0;
}

int cmp_specificity_compare(const cmp_specificity_t *s1,
                            const cmp_specificity_t *s2, int *out_cmp) {
  if (!s1 || !s2 || !out_cmp)
    return -1;

  if (s1->a != s2->a) {
    *out_cmp = (s1->a > s2->a) ? 1 : -1;
    return 0;
  }
  if (s1->b != s2->b) {
    *out_cmp = (s1->b > s2->b) ? 1 : -1;
    return 0;
  }
  if (s1->c != s2->c) {
    *out_cmp = (s1->c > s2->c) ? 1 : -1;
    return 0;
  }

  *out_cmp = 0;
  return 0;
}

int cmp_at_rule_layer_init(cmp_at_rule_layer_t *layer, const char *name) {
  int rc;
  if (!layer)
    return -1;
  rc = cmp_strdup_safe(name, &layer->name);
  if (rc != 0)
    return rc;
  return 0;
}

int cmp_at_rule_layer_free(cmp_at_rule_layer_t *layer) {
  if (!layer)
    return -1;
  if (layer->name) {
    free(layer->name);
    layer->name = NULL;
  }
  return 0;
}

int cmp_css_var_init(cmp_css_var_t *var, const char *name, const char *value) {
  int rc;
  if (!var || !name || !value)
    return -1;
  rc = cmp_strdup_safe(name, &var->name);
  if (rc != 0)
    return rc;
  rc = cmp_strdup_safe(value, &var->value);
  if (rc != 0) {
    free(var->name);
    var->name = NULL;
    return rc;
  }
  return 0;
}

int cmp_css_var_free(cmp_css_var_t *var) {
  if (!var)
    return -1;
  if (var->name) {
    free(var->name);
    var->name = NULL;
  }
  if (var->value) {
    free(var->value);
    var->value = NULL;
  }
  return 0;
}

int cmp_var_func_init(cmp_var_func_t *func, const char *name,
                      const char *fallback) {
  int rc;
  if (!func || !name)
    return -1;
  rc = cmp_strdup_safe(name, &func->custom_property_name);
  if (rc != 0)
    return rc;
  rc = cmp_strdup_safe(fallback, &func->fallback);
  if (rc != 0) {
    free(func->custom_property_name);
    func->custom_property_name = NULL;
    return rc;
  }
  return 0;
}

int cmp_var_func_free(cmp_var_func_t *func) {
  if (!func)
    return -1;
  if (func->custom_property_name) {
    free(func->custom_property_name);
    func->custom_property_name = NULL;
  }
  if (func->fallback) {
    free(func->fallback);
    func->fallback = NULL;
  }
  return 0;
}

int cmp_at_rule_property_init(cmp_at_rule_property_t *prop, const char *name,
                              const char *syntax, int inherits,
                              const char *initial_value) {
  int rc;
  if (!prop || !name || !syntax || !initial_value)
    return -1;
  rc = cmp_strdup_safe(name, &prop->name);
  if (rc != 0)
    return rc;
  rc = cmp_strdup_safe(syntax, &prop->syntax);
  if (rc != 0) {
    free(prop->name);
    prop->name = NULL;
    return rc;
  }
  prop->inherits = inherits;
  rc = cmp_strdup_safe(initial_value, &prop->initial_value);
  if (rc != 0) {
    free(prop->name);
    prop->name = NULL;
    free(prop->syntax);
    prop->syntax = NULL;
    return rc;
  }
  return 0;
}

int cmp_at_rule_property_free(cmp_at_rule_property_t *prop) {
  if (!prop)
    return -1;
  if (prop->name) {
    free(prop->name);
    prop->name = NULL;
  }
  if (prop->syntax) {
    free(prop->syntax);
    prop->syntax = NULL;
  }
  if (prop->initial_value) {
    free(prop->initial_value);
    prop->initial_value = NULL;
  }
  return 0;
}