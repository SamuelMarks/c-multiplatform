/* clang-format off */
#include "cmp_css_containment.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_contain_init(cmp_prop_contain_t *prop, int flags) {
  if (!prop)
    return -1;
  prop->flags = flags;
  return 0;
}

int cmp_prop_content_visibility_init(cmp_prop_content_visibility_t *prop,
                                     cmp_content_visibility_t visibility) {
  if (!prop)
    return -1;
  prop->visibility = visibility;
  return 0;
}

int cmp_prop_contain_intrinsic_init(cmp_prop_contain_intrinsic_t *group,
                                    const char *size, const char *block,
                                    const char *inln) {
  if (!group)
    return -1;

  group->size = NULL;
  group->block_size = NULL;
  group->inline_size = NULL;

  if (size) {
    size_t len = strlen(size);
    group->size = (char *)malloc(len + 1);
    if (!group->size)
      goto error;
    memcpy(group->size, size, len + 1);
  }
  if (block) {
    size_t len = strlen(block);
    group->block_size = (char *)malloc(len + 1);
    if (!group->block_size)
      goto error;
    memcpy(group->block_size, block, len + 1);
  }
  if (inln) {
    size_t len = strlen(inln);
    group->inline_size = (char *)malloc(len + 1);
    if (!group->inline_size)
      goto error;
    memcpy(group->inline_size, inln, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_contain_intrinsic_free(group);
  return -1;
}

int cmp_prop_contain_intrinsic_free(cmp_prop_contain_intrinsic_t *group) {
  if (!group)
    return -1;
  if (group->size) {
    free(group->size);
    group->size = NULL;
  }
  if (group->block_size) {
    free(group->block_size);
    group->block_size = NULL;
  }
  if (group->inline_size) {
    free(group->inline_size);
    group->inline_size = NULL;
  }
  return 0;
}

int cmp_prop_will_change_init(cmp_prop_will_change_t *prop,
                              const char *will_change) {
  size_t len;
  if (!prop)
    return -1;
  prop->will_change = NULL;
  if (will_change) {
    len = strlen(will_change);
    prop->will_change = (char *)malloc(len + 1);
    if (!prop->will_change)
      return -1;
    memcpy(prop->will_change, will_change, len + 1);
  }
  return 0;
}

int cmp_prop_will_change_free(cmp_prop_will_change_t *prop) {
  if (!prop)
    return -1;
  if (prop->will_change) {
    free(prop->will_change);
    prop->will_change = NULL;
  }
  return 0;
}

int cmp_prop_container_group_init(cmp_prop_container_group_t *group,
                                  cmp_prop_container_type_t type,
                                  const char *name) {
  size_t len;
  if (!group)
    return -1;

  group->type = type;
  group->name = NULL;

  if (name) {
    len = strlen(name);
    group->name = (char *)malloc(len + 1);
    if (!group->name) {
      return -1;
    }
    memcpy(group->name, name, len + 1);
  }

  return 0;
}

int cmp_prop_container_group_free(cmp_prop_container_group_t *group) {
  if (!group)
    return -1;
  if (group->name) {
    free(group->name);
    group->name = NULL;
  }
  return 0;
}
