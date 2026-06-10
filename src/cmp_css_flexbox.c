/* clang-format off */
#include "cmp_css_flexbox.h"
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

int cmp_prop_flex_container_group_init(cmp_prop_flex_container_group_t *group,
                                       cmp_flex_direction_t direction,
                                       cmp_flex_wrap_t wrap) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_flex_container_group_t));

  group->direction = direction;
  group->wrap = wrap;

  return 0;
}

int cmp_prop_flex_item_group_init(cmp_prop_flex_item_group_t *group, float grow,
                                  float shrink, const char *basis) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_flex_item_group_t));

  group->grow = grow;
  group->shrink = shrink;

  if (basis) {
    rc = cmp_strdup_safe(basis, &group->basis);
    if (rc != 0)
      return -1;
  }

  return 0;
}

int cmp_prop_flex_item_group_free(cmp_prop_flex_item_group_t *group) {
  if (!group)
    return -1;
  if (group->basis)
    free(group->basis);
  memset(group, 0, sizeof(cmp_prop_flex_item_group_t));
  return 0;
}

int cmp_prop_order_init(cmp_prop_order_t *order_prop, int order) {
  if (!order_prop)
    return -1;
  memset(order_prop, 0, sizeof(cmp_prop_order_t));
  order_prop->order = order;
  return 0;
}