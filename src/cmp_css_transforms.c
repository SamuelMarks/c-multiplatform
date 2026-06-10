/* clang-format off */
#include "cmp_css_transforms.h"
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

int cmp_prop_transform_group_init(cmp_prop_transform_group_t *group,
                                  const char *transform, const char *origin,
                                  cmp_transform_style_t style,
                                  const char *perspective,
                                  const char *perspective_origin,
                                  cmp_backface_visibility_t backface) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_transform_group_t));

  group->transform_style = style;
  group->backface = backface;

  if (transform) {
    rc = cmp_strdup_safe(transform, &group->transform);
    if (rc != 0)
      goto error;
  }
  if (origin) {
    rc = cmp_strdup_safe(origin, &group->transform_origin);
    if (rc != 0)
      goto error;
  }
  if (perspective) {
    rc = cmp_strdup_safe(perspective, &group->perspective);
    if (rc != 0)
      goto error;
  }
  if (perspective_origin) {
    rc = cmp_strdup_safe(perspective_origin, &group->perspective_origin);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_transform_group_free(group);
  return -1;
}

int cmp_prop_transform_group_free(cmp_prop_transform_group_t *group) {
  if (!group)
    return -1;
  if (group->transform)
    free(group->transform);
  if (group->transform_origin)
    free(group->transform_origin);
  if (group->perspective)
    free(group->perspective);
  if (group->perspective_origin)
    free(group->perspective_origin);

  memset(group, 0, sizeof(cmp_prop_transform_group_t));
  return 0;
}

int cmp_prop_independent_transform_init(cmp_prop_independent_transform_t *group,
                                        const char *translate,
                                        const char *rotate, const char *scale) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_independent_transform_t));

  if (translate) {
    rc = cmp_strdup_safe(translate, &group->translate);
    if (rc != 0)
      goto error;
  }
  if (rotate) {
    rc = cmp_strdup_safe(rotate, &group->rotate);
    if (rc != 0)
      goto error;
  }
  if (scale) {
    rc = cmp_strdup_safe(scale, &group->scale);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_independent_transform_free(group);
  return -1;
}

int cmp_prop_independent_transform_free(
    cmp_prop_independent_transform_t *group) {
  if (!group)
    return -1;
  if (group->translate)
    free(group->translate);
  if (group->rotate)
    free(group->rotate);
  if (group->scale)
    free(group->scale);

  memset(group, 0, sizeof(cmp_prop_independent_transform_t));
  return 0;
}

int cmp_transform_funcs_init(cmp_transform_funcs_t *func,
                             cmp_transform_func_type_t type, const char *args) {
  int rc;
  if (!func || !args)
    return -1;
  memset(func, 0, sizeof(cmp_transform_funcs_t));

  func->type = type;
  rc = cmp_strdup_safe(args, &func->args);
  if (rc != 0)
    return -1;

  return 0;
}

int cmp_transform_funcs_free(cmp_transform_funcs_t *func) {
  if (!func)
    return -1;
  if (func->args)
    free(func->args);
  memset(func, 0, sizeof(cmp_transform_funcs_t));
  return 0;
}

int cmp_prop_motion_path_group_init(cmp_prop_motion_path_group_t *group,
                                    const char *path, const char *distance,
                                    const char *position, const char *anchor,
                                    const char *rotate) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_motion_path_group_t));

  if (path) {
    rc = cmp_strdup_safe(path, &group->path);
    if (rc != 0)
      goto error;
  }
  if (distance) {
    rc = cmp_strdup_safe(distance, &group->distance);
    if (rc != 0)
      goto error;
  }
  if (position) {
    rc = cmp_strdup_safe(position, &group->position);
    if (rc != 0)
      goto error;
  }
  if (anchor) {
    rc = cmp_strdup_safe(anchor, &group->anchor);
    if (rc != 0)
      goto error;
  }
  if (rotate) {
    rc = cmp_strdup_safe(rotate, &group->rotate);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_motion_path_group_free(group);
  return -1;
}

int cmp_prop_motion_path_group_free(cmp_prop_motion_path_group_t *group) {
  if (!group)
    return -1;

  if (group->path)
    free(group->path);
  if (group->distance)
    free(group->distance);
  if (group->position)
    free(group->position);
  if (group->anchor)
    free(group->anchor);
  if (group->rotate)
    free(group->rotate);

  memset(group, 0, sizeof(cmp_prop_motion_path_group_t));
  return 0;
}