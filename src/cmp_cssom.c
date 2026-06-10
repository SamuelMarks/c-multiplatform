/* clang-format off */
#include "cmp_cssom.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int duplicate_string(char **dest, const char *src) {
  size_t len;
  if (!dest) {
    return -1;
  }
  *dest = NULL;
  if (src) {
    len = strlen(src);
    *dest = (char *)malloc(len + 1);
    if (!*dest) {
      return -1;
    }
    memcpy(*dest, src, len + 1);
  }
  return 0;
}

int cmp_cssom_rule_init(cmp_cssom_rule_t *rule, cmp_cssom_rule_type_t type,
                        const char *css_text) {
  if (!rule) {
    return -1;
  }
  rule->type = type;
  rule->parent_rule = NULL;
  return duplicate_string(&rule->css_text, css_text);
}

int cmp_cssom_rule_free(cmp_cssom_rule_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->css_text) {
    free(rule->css_text);
    rule->css_text = NULL;
  }
  return 0;
}

int cmp_cssom_decl_init(cmp_cssom_decl_t *decl, const char *css_text) {
  if (!decl) {
    return -1;
  }
  decl->length = 0; /* Parsing properties not implemented yet */
  return duplicate_string(&decl->css_text, css_text);
}

int cmp_cssom_decl_free(cmp_cssom_decl_t *decl) {
  if (!decl) {
    return -1;
  }
  if (decl->css_text) {
    free(decl->css_text);
    decl->css_text = NULL;
  }
  return 0;
}

int cmp_cssom_computed_style_init(cmp_cssom_computed_style_t *cs,
                                  void *node_ref) {
  if (!cs) {
    return -1;
  }
  cs->node_ref = node_ref;
  cs->decl = NULL;
  return 0;
}

int cmp_cssom_computed_style_free(cmp_cssom_computed_style_t *cs) {
  if (!cs) {
    return -1;
  }
  if (cs->decl) {
    cmp_cssom_decl_free(cs->decl);
    free(cs->decl);
    cs->decl = NULL;
  }
  return 0;
}

int cmp_cssom_mutator_init(cmp_cssom_mutator_t *mutator, void *stylesheet_ref) {
  if (!mutator) {
    return -1;
  }
  mutator->stylesheet_ref = stylesheet_ref;
  return 0;
}

int cmp_cssom_mutator_insert_rule(cmp_cssom_mutator_t *mutator,
                                  const char *rule, size_t index) {
  (void)index; /* Not implemented */
  if (!mutator || !rule) {
    return -1;
  }
  /* Stub implementation */
  return 0;
}

int cmp_cssom_mutator_delete_rule(cmp_cssom_mutator_t *mutator, size_t index) {
  (void)index; /* Not implemented */
  if (!mutator) {
    return -1;
  }
  /* Stub implementation */
  return 0;
}
