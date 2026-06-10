/* clang-format off */
#include "cmp_css_overflow.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_overflow_group_init(cmp_prop_overflow_group_t *group,
                                 cmp_overflow_value_t x, cmp_overflow_value_t y,
                                 const char *clip_margin) {
  if (!group) {
    return -1;
  }
  group->overflow_x = x;
  group->overflow_y = y;
  if (clip_margin) {
    size_t len = strlen(clip_margin);
    group->overflow_clip_margin = (char *)malloc(len + 1);
    if (!group->overflow_clip_margin) {
      return -1;
    }
    memcpy(group->overflow_clip_margin, clip_margin, len + 1);
  } else {
    group->overflow_clip_margin = NULL;
  }
  return 0;
}

int cmp_prop_overflow_group_free(cmp_prop_overflow_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->overflow_clip_margin) {
    free(group->overflow_clip_margin);
    group->overflow_clip_margin = NULL;
  }
  return 0;
}

int cmp_prop_overscroll_init(cmp_prop_overscroll_t *prop,
                             cmp_overscroll_behavior_value_t x,
                             cmp_overscroll_behavior_value_t y) {
  if (!prop) {
    return -1;
  }
  prop->overscroll_x = x;
  prop->overscroll_y = y;
  return 0;
}

int cmp_prop_scroll_behavior_init(cmp_prop_scroll_behavior_t *prop,
                                  cmp_scroll_behavior_t behavior) {
  if (!prop) {
    return -1;
  }
  prop->behavior = behavior;
  return 0;
}

int cmp_prop_scroll_snap_group_init(cmp_prop_scroll_snap_group_t *group,
                                    const char *type, const char *align,
                                    cmp_scroll_snap_stop_t stop) {
  if (!group) {
    return -1;
  }
  memset(group, 0, sizeof(*group));
  if (type) {
    size_t len = strlen(type);
    group->type.snap_type = (char *)malloc(len + 1);
    if (!group->type.snap_type)
      goto error;
    memcpy(group->type.snap_type, type, len + 1);
  }
  if (align) {
    size_t len = strlen(align);
    group->align.snap_align = (char *)malloc(len + 1);
    if (!group->align.snap_align)
      goto error;
    memcpy(group->align.snap_align, align, len + 1);
  }
  group->stop = stop;
  return 0;

error:
  (void)cmp_prop_scroll_snap_group_free(group);
  return -1;
}

int cmp_prop_scroll_snap_group_free(cmp_prop_scroll_snap_group_t *group) {
  if (!group) {
    return -1;
  }
  if (group->type.snap_type) {
    free(group->type.snap_type);
    group->type.snap_type = NULL;
  }
  if (group->align.snap_align) {
    free(group->align.snap_align);
    group->align.snap_align = NULL;
  }
  return 0;
}

int cmp_prop_scroll_padding_margin_init(cmp_prop_scroll_padding_margin_t *prop,
                                        const char *padding,
                                        const char *margin) {
  if (!prop) {
    return -1;
  }
  memset(prop, 0, sizeof(*prop));
  if (padding) {
    size_t len = strlen(padding);
    prop->scroll_padding = (char *)malloc(len + 1);
    if (!prop->scroll_padding)
      goto error;
    memcpy(prop->scroll_padding, padding, len + 1);
  }
  if (margin) {
    size_t len = strlen(margin);
    prop->scroll_margin = (char *)malloc(len + 1);
    if (!prop->scroll_margin)
      goto error;
    memcpy(prop->scroll_margin, margin, len + 1);
  }
  return 0;

error:
  (void)cmp_prop_scroll_padding_margin_free(prop);
  return -1;
}

int cmp_prop_scroll_padding_margin_free(
    cmp_prop_scroll_padding_margin_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->scroll_padding) {
    free(prop->scroll_padding);
    prop->scroll_padding = NULL;
  }
  if (prop->scroll_margin) {
    free(prop->scroll_margin);
    prop->scroll_margin = NULL;
  }
  return 0;
}

int cmp_prop_scrollbar_gutter_init(cmp_prop_scrollbar_gutter_t *prop,
                                   cmp_scrollbar_gutter_t gutter) {
  if (!prop) {
    return -1;
  }
  prop->gutter = gutter;
  return 0;
}

int cmp_prop_scrollbar_styling_init(cmp_prop_scrollbar_styling_t *prop,
                                    const char *width, const char *color,
                                    int has_webkit) {
  if (!prop) {
    return -1;
  }
  memset(prop, 0, sizeof(*prop));
  if (width) {
    size_t len = strlen(width);
    prop->scrollbar_width = (char *)malloc(len + 1);
    if (!prop->scrollbar_width)
      goto error;
    memcpy(prop->scrollbar_width, width, len + 1);
  }
  if (color) {
    size_t len = strlen(color);
    prop->scrollbar_color = (char *)malloc(len + 1);
    if (!prop->scrollbar_color)
      goto error;
    memcpy(prop->scrollbar_color, color, len + 1);
  }
  prop->has_webkit_scrollbar = has_webkit;
  return 0;

error:
  (void)cmp_prop_scrollbar_styling_free(prop);
  return -1;
}

int cmp_prop_scrollbar_styling_free(cmp_prop_scrollbar_styling_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->scrollbar_width) {
    free(prop->scrollbar_width);
    prop->scrollbar_width = NULL;
  }
  if (prop->scrollbar_color) {
    free(prop->scrollbar_color);
    prop->scrollbar_color = NULL;
  }
  return 0;
}
