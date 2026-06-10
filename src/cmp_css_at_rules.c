/* clang-format off */
#include "cmp_css_at_rules.h"
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

int cmp_at_rule_media_init(cmp_at_rule_media_t *rule, const char *condition) {
  if (!rule) {
    return -1;
  }
  return duplicate_string(&rule->condition_text, condition);
}

int cmp_at_rule_media_free(cmp_at_rule_media_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->condition_text) {
    free(rule->condition_text);
    rule->condition_text = NULL;
  }
  return 0;
}

int cmp_at_rule_container_init(cmp_at_rule_container_t *rule, const char *name,
                               const char *condition) {
  int rc;
  if (!rule) {
    return -1;
  }
  rule->name = NULL;
  rule->condition_text = NULL;

  rc = duplicate_string(&rule->name, name);
  if (rc != 0) {
    goto error;
  }

  rc = duplicate_string(&rule->condition_text, condition);
  if (rc != 0) {
    goto error;
  }

  return 0;
error:
  cmp_at_rule_container_free(rule);
  return -1;
}

int cmp_at_rule_container_free(cmp_at_rule_container_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->name) {
    free(rule->name);
    rule->name = NULL;
  }
  if (rule->condition_text) {
    free(rule->condition_text);
    rule->condition_text = NULL;
  }
  return 0;
}

int cmp_at_rule_supports_init(cmp_at_rule_supports_t *rule,
                              const char *condition) {
  if (!rule) {
    return -1;
  }
  return duplicate_string(&rule->condition_text, condition);
}

int cmp_at_rule_supports_free(cmp_at_rule_supports_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->condition_text) {
    free(rule->condition_text);
    rule->condition_text = NULL;
  }
  return 0;
}

int cmp_at_rule_import_init(cmp_at_rule_import_t *rule, const char *url,
                            const char *layer, const char *supports,
                            const char *media) {
  if (!rule) {
    return -1;
  }
  rule->url = NULL;
  rule->layer_name = NULL;
  rule->supports_cond = NULL;
  rule->media_query = NULL;

  if (duplicate_string(&rule->url, url) != 0 ||
      duplicate_string(&rule->layer_name, layer) != 0 ||
      duplicate_string(&rule->supports_cond, supports) != 0 ||
      duplicate_string(&rule->media_query, media) != 0) {
    cmp_at_rule_import_free(rule);
    return -1;
  }
  return 0;
}

int cmp_at_rule_import_free(cmp_at_rule_import_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->url)
    free(rule->url);
  if (rule->layer_name)
    free(rule->layer_name);
  if (rule->supports_cond)
    free(rule->supports_cond);
  if (rule->media_query)
    free(rule->media_query);

  rule->url = NULL;
  rule->layer_name = NULL;
  rule->supports_cond = NULL;
  rule->media_query = NULL;
  return 0;
}

int cmp_at_rule_viewport_init(cmp_at_rule_viewport_t *rule,
                              const char *declarations) {
  if (!rule) {
    return -1;
  }
  return duplicate_string(&rule->declarations, declarations);
}

int cmp_at_rule_viewport_free(cmp_at_rule_viewport_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->declarations) {
    free(rule->declarations);
    rule->declarations = NULL;
  }
  return 0;
}

int cmp_at_rule_charset_init(cmp_at_rule_charset_t *rule, const char *charset) {
  if (!rule) {
    return -1;
  }
  return duplicate_string(&rule->charset, charset);
}

int cmp_at_rule_charset_free(cmp_at_rule_charset_t *rule) {
  if (!rule) {
    return -1;
  }
  if (rule->charset) {
    free(rule->charset);
    rule->charset = NULL;
  }
  return 0;
}
