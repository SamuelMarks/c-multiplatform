/* clang-format off */
#include "cmp_css_grid.h"
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

int cmp_prop_grid_template_group_init(cmp_prop_grid_template_group_t *group,
                                      const char *columns, const char *rows,
                                      const char *areas) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_grid_template_group_t));

  if (columns) {
    rc = cmp_strdup_safe(columns, &group->columns);
    if (rc != 0)
      goto error;
  }
  if (rows) {
    rc = cmp_strdup_safe(rows, &group->rows);
    if (rc != 0)
      goto error;
  }
  if (areas) {
    rc = cmp_strdup_safe(areas, &group->areas);
    if (rc != 0)
      goto error;
  }
  return 0;

error:
  (void)cmp_prop_grid_template_group_free(group);
  return -1;
}

int cmp_prop_grid_template_group_free(cmp_prop_grid_template_group_t *group) {
  if (!group)
    return -1;
  if (group->columns)
    free(group->columns);
  if (group->rows)
    free(group->rows);
  if (group->areas)
    free(group->areas);
  memset(group, 0, sizeof(cmp_prop_grid_template_group_t));
  return 0;
}

int cmp_prop_grid_auto_group_init(cmp_prop_grid_auto_group_t *group,
                                  const char *columns, const char *rows,
                                  cmp_grid_auto_flow_t flow) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_grid_auto_group_t));

  group->flow = flow;

  if (columns) {
    rc = cmp_strdup_safe(columns, &group->columns);
    if (rc != 0)
      goto error;
  }
  if (rows) {
    rc = cmp_strdup_safe(rows, &group->rows);
    if (rc != 0)
      goto error;
  }
  return 0;

error:
  (void)cmp_prop_grid_auto_group_free(group);
  return -1;
}

int cmp_prop_grid_auto_group_free(cmp_prop_grid_auto_group_t *group) {
  if (!group)
    return -1;
  if (group->columns)
    free(group->columns);
  if (group->rows)
    free(group->rows);
  memset(group, 0, sizeof(cmp_prop_grid_auto_group_t));
  return 0;
}

int cmp_prop_grid_placement_group_init(cmp_prop_grid_placement_group_t *group,
                                       const char *rs, const char *re,
                                       const char *cs, const char *ce) {
  int rc;
  if (!group)
    return -1;
  memset(group, 0, sizeof(cmp_prop_grid_placement_group_t));

  if (rs) {
    rc = cmp_strdup_safe(rs, &group->row_start);
    if (rc != 0)
      goto error;
  }
  if (re) {
    rc = cmp_strdup_safe(re, &group->row_end);
    if (rc != 0)
      goto error;
  }
  if (cs) {
    rc = cmp_strdup_safe(cs, &group->column_start);
    if (rc != 0)
      goto error;
  }
  if (ce) {
    rc = cmp_strdup_safe(ce, &group->column_end);
    if (rc != 0)
      goto error;
  }
  return 0;

error:
  (void)cmp_prop_grid_placement_group_free(group);
  return -1;
}

int cmp_prop_grid_placement_group_free(cmp_prop_grid_placement_group_t *group) {
  if (!group)
    return -1;
  if (group->row_start)
    free(group->row_start);
  if (group->row_end)
    free(group->row_end);
  if (group->column_start)
    free(group->column_start);
  if (group->column_end)
    free(group->column_end);
  memset(group, 0, sizeof(cmp_prop_grid_placement_group_t));
  return 0;
}

int cmp_grid_track_funcs_init(cmp_grid_track_funcs_t *func,
                              cmp_grid_track_func_type_t type,
                              const char *args) {
  int rc;
  if (!func || !args)
    return -1;
  memset(func, 0, sizeof(cmp_grid_track_funcs_t));

  func->type = type;
  rc = cmp_strdup_safe(args, &func->args);
  if (rc != 0)
    return -1;

  return 0;
}

int cmp_grid_track_funcs_free(cmp_grid_track_funcs_t *func) {
  if (!func)
    return -1;
  if (func->args)
    free(func->args);
  memset(func, 0, sizeof(cmp_grid_track_funcs_t));
  return 0;
}