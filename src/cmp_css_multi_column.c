/* clang-format off */
#include "cmp_css_multi_column.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

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

int cmp_prop_column_def_group_init(cmp_prop_column_def_group_t *group,
                                   const char *width, const char *count) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_column_def_group_t));

  if (width) {
    rc = cmp_strdup_safe(width, &group->width);
    if (rc != 0)
      goto error;
  }

  if (count) {
    rc = cmp_strdup_safe(count, &group->count);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_column_def_group_free(group);
  return -1;
}

int cmp_prop_column_def_group_free(cmp_prop_column_def_group_t *group) {
  if (!group)
    return -1;
  if (group->width)
    free(group->width);
  if (group->count)
    free(group->count);
  memset(group, 0, sizeof(cmp_prop_column_def_group_t));
  return 0;
}

int cmp_prop_column_rule_group_init(cmp_prop_column_rule_group_t *group,
                                    const char *width, const char *style,
                                    const char *color) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_column_rule_group_t));

  if (width) {
    rc = cmp_strdup_safe(width, &group->width);
    if (rc != 0)
      goto error;
  }

  if (style) {
    rc = cmp_strdup_safe(style, &group->style);
    if (rc != 0)
      goto error;
  }

  if (color) {
    rc = cmp_strdup_safe(color, &group->color);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_column_rule_group_free(group);
  return -1;
}

int cmp_prop_column_rule_group_free(cmp_prop_column_rule_group_t *group) {
  if (!group)
    return -1;
  if (group->width)
    free(group->width);
  if (group->style)
    free(group->style);
  if (group->color)
    free(group->color);
  memset(group, 0, sizeof(cmp_prop_column_rule_group_t));
  return 0;
}

int cmp_prop_column_span_fill_init(cmp_prop_column_span_fill_t *group,
                                   cmp_column_span_t span,
                                   cmp_column_fill_t fill) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_column_span_fill_t));
  group->span = span;
  group->fill = fill;
  return 0;
}