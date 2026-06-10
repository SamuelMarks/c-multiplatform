/* clang-format off */
#include "cmp_css_box_alignment.h"
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

int cmp_prop_align_content_group_init(cmp_prop_align_content_group_t *group,
                                      cmp_align_value_t jc,
                                      cmp_align_value_t ac) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_align_content_group_t));

  group->justify_content = jc;
  group->align_content = ac;

  return 0;
}

int cmp_prop_align_items_group_init(cmp_prop_align_items_group_t *group,
                                    cmp_align_value_t ji,
                                    cmp_align_value_t ai) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_align_items_group_t));

  group->justify_items = ji;
  group->align_items = ai;

  return 0;
}

int cmp_prop_align_self_group_init(cmp_prop_align_self_group_t *group,
                                   cmp_align_value_t js, cmp_align_value_t as) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_align_self_group_t));

  group->justify_self = js;
  group->align_self = as;

  return 0;
}

int cmp_prop_gap_init(cmp_prop_gap_t *gap, const char *row_gap,
                      const char *col_gap) {
  int rc;
  if (!gap)
    return -1;
  memset(gap, 0, sizeof(cmp_prop_gap_t));

  if (row_gap) {
    rc = cmp_strdup_safe(row_gap, &gap->row_gap);
    if (rc != 0)
      goto error;
  }

  if (col_gap) {
    rc = cmp_strdup_safe(col_gap, &gap->column_gap);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_gap_free(gap);
  return -1;
}

int cmp_prop_gap_free(cmp_prop_gap_t *gap) {
  if (!gap)
    return -1;

  if (gap->row_gap)
    free(gap->row_gap);
  if (gap->column_gap)
    free(gap->column_gap);

  memset(gap, 0, sizeof(cmp_prop_gap_t));
  return 0;
}