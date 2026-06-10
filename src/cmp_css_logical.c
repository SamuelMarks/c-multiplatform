/* clang-format off */
#include "cmp_css_logical.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_logical_size_group_init(cmp_prop_logical_size_group_t *group,
                                     const char *b_size, const char *i_size,
                                     const char *min_b, const char *min_i,
                                     const char *max_b, const char *max_i) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (b_size) {
    size_t len = strlen(b_size);
    group->block_size = (char *)malloc(len + 1);
    if (!group->block_size)
      goto error;
    memcpy(group->block_size, b_size, len + 1);
  }

  if (i_size) {
    size_t len = strlen(i_size);
    group->inline_size = (char *)malloc(len + 1);
    if (!group->inline_size)
      goto error;
    memcpy(group->inline_size, i_size, len + 1);
  }

  if (min_b) {
    size_t len = strlen(min_b);
    group->min_block_size = (char *)malloc(len + 1);
    if (!group->min_block_size)
      goto error;
    memcpy(group->min_block_size, min_b, len + 1);
  }

  if (min_i) {
    size_t len = strlen(min_i);
    group->min_inline_size = (char *)malloc(len + 1);
    if (!group->min_inline_size)
      goto error;
    memcpy(group->min_inline_size, min_i, len + 1);
  }

  if (max_b) {
    size_t len = strlen(max_b);
    group->max_block_size = (char *)malloc(len + 1);
    if (!group->max_block_size)
      goto error;
    memcpy(group->max_block_size, max_b, len + 1);
  }

  if (max_i) {
    size_t len = strlen(max_i);
    group->max_inline_size = (char *)malloc(len + 1);
    if (!group->max_inline_size)
      goto error;
    memcpy(group->max_inline_size, max_i, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_logical_size_group_free(group);
  return -1;
}

int cmp_prop_logical_size_group_free(cmp_prop_logical_size_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->block_size) {
    free(group->block_size);
    group->block_size = NULL;
  }
  if (group->inline_size) {
    free(group->inline_size);
    group->inline_size = NULL;
  }
  if (group->min_block_size) {
    free(group->min_block_size);
    group->min_block_size = NULL;
  }
  if (group->min_inline_size) {
    free(group->min_inline_size);
    group->min_inline_size = NULL;
  }
  if (group->max_block_size) {
    free(group->max_block_size);
    group->max_block_size = NULL;
  }
  if (group->max_inline_size) {
    free(group->max_inline_size);
    group->max_inline_size = NULL;
  }
  return 0;
}

int cmp_prop_logical_margin_group_init(cmp_prop_logical_margin_group_t *group,
                                       const char *bs, const char *be,
                                       const char *is, const char *ie) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (bs) {
    size_t len = strlen(bs);
    group->margin_block_start = (char *)malloc(len + 1);
    if (!group->margin_block_start)
      goto error;
    memcpy(group->margin_block_start, bs, len + 1);
  }

  if (be) {
    size_t len = strlen(be);
    group->margin_block_end = (char *)malloc(len + 1);
    if (!group->margin_block_end)
      goto error;
    memcpy(group->margin_block_end, be, len + 1);
  }

  if (is) {
    size_t len = strlen(is);
    group->margin_inline_start = (char *)malloc(len + 1);
    if (!group->margin_inline_start)
      goto error;
    memcpy(group->margin_inline_start, is, len + 1);
  }

  if (ie) {
    size_t len = strlen(ie);
    group->margin_inline_end = (char *)malloc(len + 1);
    if (!group->margin_inline_end)
      goto error;
    memcpy(group->margin_inline_end, ie, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_logical_margin_group_free(group);
  return -1;
}

int cmp_prop_logical_margin_group_free(cmp_prop_logical_margin_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->margin_block_start) {
    free(group->margin_block_start);
    group->margin_block_start = NULL;
  }
  if (group->margin_block_end) {
    free(group->margin_block_end);
    group->margin_block_end = NULL;
  }
  if (group->margin_inline_start) {
    free(group->margin_inline_start);
    group->margin_inline_start = NULL;
  }
  if (group->margin_inline_end) {
    free(group->margin_inline_end);
    group->margin_inline_end = NULL;
  }
  return 0;
}

int cmp_prop_logical_padding_group_init(cmp_prop_logical_padding_group_t *group,
                                        const char *bs, const char *be,
                                        const char *is, const char *ie) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (bs) {
    size_t len = strlen(bs);
    group->padding_block_start = (char *)malloc(len + 1);
    if (!group->padding_block_start)
      goto error;
    memcpy(group->padding_block_start, bs, len + 1);
  }

  if (be) {
    size_t len = strlen(be);
    group->padding_block_end = (char *)malloc(len + 1);
    if (!group->padding_block_end)
      goto error;
    memcpy(group->padding_block_end, be, len + 1);
  }

  if (is) {
    size_t len = strlen(is);
    group->padding_inline_start = (char *)malloc(len + 1);
    if (!group->padding_inline_start)
      goto error;
    memcpy(group->padding_inline_start, is, len + 1);
  }

  if (ie) {
    size_t len = strlen(ie);
    group->padding_inline_end = (char *)malloc(len + 1);
    if (!group->padding_inline_end)
      goto error;
    memcpy(group->padding_inline_end, ie, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_logical_padding_group_free(group);
  return -1;
}

int cmp_prop_logical_padding_group_free(
    cmp_prop_logical_padding_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->padding_block_start) {
    free(group->padding_block_start);
    group->padding_block_start = NULL;
  }
  if (group->padding_block_end) {
    free(group->padding_block_end);
    group->padding_block_end = NULL;
  }
  if (group->padding_inline_start) {
    free(group->padding_inline_start);
    group->padding_inline_start = NULL;
  }
  if (group->padding_inline_end) {
    free(group->padding_inline_end);
    group->padding_inline_end = NULL;
  }
  return 0;
}

int cmp_logical_border_part_init(cmp_logical_border_part_t *part,
                                 const char *width, cmp_border_style_t style,
                                 const cmp_prop_color_t *color) {
  if (!part) {
    return -1;
  }
  memset(part, 0, sizeof(*part));

  if (width) {
    size_t len = strlen(width);
    part->width = (char *)malloc(len + 1);
    if (!part->width) {
      return -1;
    }
    memcpy(part->width, width, len + 1);
  }

  part->style = style;
  if (color) {
    part->color = *color;
  } else {
    part->color.type = CMP_PROP_COLOR_CURRENTCOLOR;
  }
  return 0;
}

int cmp_logical_border_part_free(cmp_logical_border_part_t *part) {
  if (!part) {
    return -1;
  }
  if (part->width) {
    free(part->width);
    part->width = NULL;
  }
  (void)cmp_prop_color_free(&part->color);
  return 0;
}

int cmp_prop_logical_border_group_init(cmp_prop_logical_border_group_t *group) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));
  /* Parts are expected to be initialized manually */
  return 0;
}

int cmp_prop_logical_border_group_free(cmp_prop_logical_border_group_t *group) {
  if (!group) {
    return -1;
  }
  cmp_logical_border_part_free(&group->block_start);
  cmp_logical_border_part_free(&group->block_end);
  cmp_logical_border_part_free(&group->inline_start);
  cmp_logical_border_part_free(&group->inline_end);
  return 0;
}

int cmp_prop_logical_inset_group_init(cmp_prop_logical_inset_group_t *group,
                                      const char *bs, const char *be,
                                      const char *is, const char *ie) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (bs) {
    size_t len = strlen(bs);
    group->inset_block_start = (char *)malloc(len + 1);
    if (!group->inset_block_start)
      goto error;
    memcpy(group->inset_block_start, bs, len + 1);
  }

  if (be) {
    size_t len = strlen(be);
    group->inset_block_end = (char *)malloc(len + 1);
    if (!group->inset_block_end)
      goto error;
    memcpy(group->inset_block_end, be, len + 1);
  }

  if (is) {
    size_t len = strlen(is);
    group->inset_inline_start = (char *)malloc(len + 1);
    if (!group->inset_inline_start)
      goto error;
    memcpy(group->inset_inline_start, is, len + 1);
  }

  if (ie) {
    size_t len = strlen(ie);
    group->inset_inline_end = (char *)malloc(len + 1);
    if (!group->inset_inline_end)
      goto error;
    memcpy(group->inset_inline_end, ie, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_logical_inset_group_free(group);
  return -1;
}

int cmp_prop_logical_inset_group_free(cmp_prop_logical_inset_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->inset_block_start) {
    free(group->inset_block_start);
    group->inset_block_start = NULL;
  }
  if (group->inset_block_end) {
    free(group->inset_block_end);
    group->inset_block_end = NULL;
  }
  if (group->inset_inline_start) {
    free(group->inset_inline_start);
    group->inset_inline_start = NULL;
  }
  if (group->inset_inline_end) {
    free(group->inset_inline_end);
    group->inset_inline_end = NULL;
  }
  return 0;
}

int cmp_prop_logical_radius_group_init(cmp_prop_logical_radius_group_t *group,
                                       const char *ss, const char *se,
                                       const char *es, const char *ee) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));

  if (ss) {
    size_t len = strlen(ss);
    group->start_start = (char *)malloc(len + 1);
    if (!group->start_start)
      goto error;
    memcpy(group->start_start, ss, len + 1);
  }

  if (se) {
    size_t len = strlen(se);
    group->start_end = (char *)malloc(len + 1);
    if (!group->start_end)
      goto error;
    memcpy(group->start_end, se, len + 1);
  }

  if (es) {
    size_t len = strlen(es);
    group->end_start = (char *)malloc(len + 1);
    if (!group->end_start)
      goto error;
    memcpy(group->end_start, es, len + 1);
  }

  if (ee) {
    size_t len = strlen(ee);
    group->end_end = (char *)malloc(len + 1);
    if (!group->end_end)
      goto error;
    memcpy(group->end_end, ee, len + 1);
  }

  return 0;

error:
  (void)cmp_prop_logical_radius_group_free(group);
  return -1;
}

int cmp_prop_logical_radius_group_free(cmp_prop_logical_radius_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->start_start) {
    free(group->start_start);
    group->start_start = NULL;
  }
  if (group->start_end) {
    free(group->start_end);
    group->start_end = NULL;
  }
  if (group->end_start) {
    free(group->end_start);
    group->end_start = NULL;
  }
  if (group->end_end) {
    free(group->end_end);
    group->end_end = NULL;
  }
  return 0;
}
