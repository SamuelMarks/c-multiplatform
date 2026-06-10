/* clang-format off */
#include "cmp_css_scroll_animations.h"
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

int cmp_prop_scroll_anim_group_init(cmp_prop_scroll_anim_group_t *group,
                                    const char *timeline,
                                    const char *range_start,
                                    const char *range_end) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_scroll_anim_group_t));

  if (timeline) {
    rc = cmp_strdup_safe(timeline, &group->timeline);
    if (rc != 0)
      goto error;
  }
  if (range_start) {
    rc = cmp_strdup_safe(range_start, &group->range_start);
    if (rc != 0)
      goto error;
  }
  if (range_end) {
    rc = cmp_strdup_safe(range_end, &group->range_end);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_scroll_anim_group_free(group);
  return -1;
}

int cmp_prop_scroll_anim_group_free(cmp_prop_scroll_anim_group_t *group) {
  if (!group)
    return -1;
  if (group->timeline)
    free(group->timeline);
  if (group->range_start)
    free(group->range_start);
  if (group->range_end)
    free(group->range_end);

  memset(group, 0, sizeof(cmp_prop_scroll_anim_group_t));
  return 0;
}

int cmp_prop_scroll_timeline_init(cmp_prop_scroll_timeline_t *group,
                                  const char *name,
                                  cmp_scroll_timeline_axis_t axis) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_scroll_timeline_t));

  group->axis = axis;

  if (name) {
    rc = cmp_strdup_safe(name, &group->name);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_scroll_timeline_free(group);
  return -1;
}

int cmp_prop_scroll_timeline_free(cmp_prop_scroll_timeline_t *group) {
  if (!group)
    return -1;
  if (group->name)
    free(group->name);

  memset(group, 0, sizeof(cmp_prop_scroll_timeline_t));
  return 0;
}

int cmp_prop_view_timeline_init(cmp_prop_view_timeline_t *group,
                                const char *name,
                                cmp_scroll_timeline_axis_t axis,
                                const char *inset) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_view_timeline_t));

  group->axis = axis;

  if (name) {
    rc = cmp_strdup_safe(name, &group->name);
    if (rc != 0)
      goto error;
  }
  if (inset) {
    rc = cmp_strdup_safe(inset, &group->inset);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_view_timeline_free(group);
  return -1;
}

int cmp_prop_view_timeline_free(cmp_prop_view_timeline_t *group) {
  if (!group)
    return -1;
  if (group->name)
    free(group->name);
  if (group->inset)
    free(group->inset);

  memset(group, 0, sizeof(cmp_prop_view_timeline_t));
  return 0;
}

int cmp_prop_timeline_scope_init(cmp_prop_timeline_scope_t *scope_prop,
                                 const char *scope) {
  int rc;
  if (!scope_prop)
    return -1;
  memset(scope_prop, 0, sizeof(cmp_prop_timeline_scope_t));

  if (scope) {
    rc = cmp_strdup_safe(scope, &scope_prop->scope);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_timeline_scope_free(scope_prop);
  return -1;
}

int cmp_prop_timeline_scope_free(cmp_prop_timeline_scope_t *scope_prop) {
  if (!scope_prop)
    return -1;
  if (scope_prop->scope)
    free(scope_prop->scope);

  memset(scope_prop, 0, sizeof(cmp_prop_timeline_scope_t));
  return 0;
}

int cmp_prop_view_transition_name_init(
    cmp_prop_view_transition_name_t *name_prop, const char *name) {
  int rc;
  if (!name_prop)
    return -1;
  memset(name_prop, 0, sizeof(cmp_prop_view_transition_name_t));

  if (name) {
    rc = cmp_strdup_safe(name, &name_prop->name);
    if (rc != 0)
      goto error;
  }

  return 0;

error:
  (void)cmp_prop_view_transition_name_free(name_prop);
  return -1;
}

int cmp_prop_view_transition_name_free(
    cmp_prop_view_transition_name_t *name_prop) {
  if (!name_prop)
    return -1;
  if (name_prop->name)
    free(name_prop->name);

  memset(name_prop, 0, sizeof(cmp_prop_view_transition_name_t));
  return 0;
}