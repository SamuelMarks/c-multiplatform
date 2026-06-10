/* clang-format off */
#include "cmp_css_images_shapes.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_object_fit_group_init(cmp_prop_object_fit_group_t *group,
                                   cmp_object_fit_t fit, const char *pos) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));
  group->object_fit = fit;
  if (pos) {
    size_t len = strlen(pos);
    group->object_position = (char *)malloc(len + 1);
    if (!group->object_position) {
      return -1;
    }
    memcpy(group->object_position, pos, len + 1);
  }
  return 0;
}

int cmp_prop_object_fit_group_free(cmp_prop_object_fit_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->object_position) {
    free(group->object_position);
    group->object_position = NULL;
  }
  return 0;
}

int cmp_prop_image_rendering_init(cmp_prop_image_rendering_t *prop,
                                  cmp_image_rendering_t rendering) {
  if (!prop) {
    return -1;
  }
  prop->rendering = rendering;
  return 0;
}

int cmp_prop_image_resolution_init(cmp_prop_image_resolution_t *prop,
                                   const char *resolution) {
  if (!prop) {
    return -1;
  }
  if (resolution) {
    size_t len = strlen(resolution);
    prop->resolution = (char *)malloc(len + 1);
    if (!prop->resolution) {
      return -1;
    }
    memcpy(prop->resolution, resolution, len + 1);
  } else {
    prop->resolution = NULL;
  }
  return 0;
}

int cmp_prop_image_resolution_free(cmp_prop_image_resolution_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->resolution) {
    free(prop->resolution);
    prop->resolution = NULL;
  }
  return 0;
}

int cmp_prop_shape_group_init(cmp_prop_shape_group_t *group,
                              const char *outside, const char *margin,
                              const char *threshold) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (outside) {
    size_t len = strlen(outside);
    group->shape_outside = (char *)malloc(len + 1);
    if (!group->shape_outside)
      goto error;
    memcpy(group->shape_outside, outside, len + 1);
  }

  if (margin) {
    size_t len = strlen(margin);
    group->shape_margin = (char *)malloc(len + 1);
    if (!group->shape_margin)
      goto error;
    memcpy(group->shape_margin, margin, len + 1);
  }

  if (threshold) {
    size_t len = strlen(threshold);
    group->shape_image_threshold = (char *)malloc(len + 1);
    if (!group->shape_image_threshold)
      goto error;
    memcpy(group->shape_image_threshold, threshold, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_shape_group_free(group);
  return -1;
}

int cmp_prop_shape_group_free(cmp_prop_shape_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->shape_outside) {
    free(group->shape_outside);
    group->shape_outside = NULL;
  }
  if (group->shape_margin) {
    free(group->shape_margin);
    group->shape_margin = NULL;
  }
  if (group->shape_image_threshold) {
    free(group->shape_image_threshold);
    group->shape_image_threshold = NULL;
  }
  return 0;
}
