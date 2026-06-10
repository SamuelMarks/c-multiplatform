/* clang-format off */
#include "cmp_css_anchor_positioning.h"
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

int cmp_prop_anchor_name_init(cmp_prop_anchor_name_t *anchor,
                              const char *name) {
  if (!anchor || !name)
    return -1;
  return cmp_strdup_safe(name, &anchor->name);
}

int cmp_prop_anchor_name_free(cmp_prop_anchor_name_t *anchor) {
  if (!anchor)
    return -1;
  if (anchor->name) {
    free(anchor->name);
    anchor->name = NULL;
  }
  return 0;
}

int cmp_prop_position_anchor_init(cmp_prop_position_anchor_t *anchor,
                                  const char *name) {
  if (!anchor || !name)
    return -1;
  return cmp_strdup_safe(name, &anchor->name);
}

int cmp_prop_position_anchor_free(cmp_prop_position_anchor_t *anchor) {
  if (!anchor)
    return -1;
  if (anchor->name) {
    free(anchor->name);
    anchor->name = NULL;
  }
  return 0;
}

int cmp_anchor_func_init(cmp_anchor_funcs_t *func, const char *name,
                         cmp_anchor_side_t side) {
  if (!func)
    return -1;
  func->is_size = 0;
  func->func.anchor.side = side;
  {
    int rc = cmp_prop_size_init_auto(&func->func.anchor.fallback);
    if (rc != 0) return rc;
  }
  return cmp_strdup_safe(name, &func->func.anchor.anchor_name);
}

int cmp_anchor_size_func_init(cmp_anchor_funcs_t *func, const char *name,
                              cmp_anchor_size_dim_t dim) {
  if (!func)
    return -1;
  func->is_size = 1;
  func->func.anchor_size.dimension = dim;
  {
    int rc = cmp_prop_size_init_auto(&func->func.anchor_size.fallback);
    if (rc != 0) return rc;
  }
  return cmp_strdup_safe(name, &func->func.anchor_size.anchor_name);
}

int cmp_anchor_funcs_free(cmp_anchor_funcs_t *func) {
  if (!func)
    return -1;
  if (func->is_size) {
    if (func->func.anchor_size.anchor_name) {
      free(func->func.anchor_size.anchor_name);
      func->func.anchor_size.anchor_name = NULL;
    }
  } else {
    if (func->func.anchor.anchor_name) {
      free(func->func.anchor.anchor_name);
      func->func.anchor.anchor_name = NULL;
    }
  }
  return 0;
}

int cmp_prop_position_try_init(cmp_prop_position_try_t *prop,
                               const char *options,
                               cmp_position_try_order_t order) {
  if (!prop || !options)
    return -1;
  prop->order = order;
  return cmp_strdup_safe(options, &prop->options);
}

int cmp_prop_position_try_free(cmp_prop_position_try_t *prop) {
  if (!prop)
    return -1;
  if (prop->options) {
    free(prop->options);
    prop->options = NULL;
  }
  return 0;
}