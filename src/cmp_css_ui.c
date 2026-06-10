/* clang-format off */
#include "cmp_css_ui.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_prop_cursor_init(cmp_prop_cursor_t *prop, const char *cursor) {
  if (!prop) {
    return -1;
  }
  if (cursor) {
    size_t len = strlen(cursor);
    prop->cursor = (char *)malloc(len + 1);
    if (!prop->cursor)
      return -1;
    memcpy(prop->cursor, cursor, len + 1);
  } else {
    prop->cursor = NULL;
  }
  return 0;
}

int cmp_prop_cursor_free(cmp_prop_cursor_t *prop) {
  if (!prop) {
    return -1;
  }
  if (prop->cursor) {
    free(prop->cursor);
    prop->cursor = NULL;
  }
  return 0;
}

int cmp_prop_pointer_touch_group_init(cmp_prop_pointer_touch_group_t *group,
                                      const char *ptr, const char *touch) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(*group));
  if (ptr) {
    size_t len = strlen(ptr);
    group->pointer_events = (char *)malloc(len + 1);
    if (!group->pointer_events)
      goto error;
    memcpy(group->pointer_events, ptr, len + 1);
  }
  if (touch) {
    size_t len = strlen(touch);
    group->touch_action = (char *)malloc(len + 1);
    if (!group->touch_action)
      goto error;
    memcpy(group->touch_action, touch, len + 1);
  }
  return 0;

error:
  (void)cmp_prop_pointer_touch_group_free(group);
  return -1;
}

int cmp_prop_pointer_touch_group_free(cmp_prop_pointer_touch_group_t *group) {
  if (!group)
    return -1;
  if (group->pointer_events) {
    free(group->pointer_events);
    group->pointer_events = NULL;
  }
  if (group->touch_action) {
    free(group->touch_action);
    group->touch_action = NULL;
  }
  return 0;
}

int cmp_prop_user_select_init(cmp_prop_user_select_t *prop,
                              cmp_user_select_t select) {
  if (!prop)
    return -1;
  prop->select = select;
  return 0;
}

int cmp_prop_appearance_init(cmp_prop_appearance_t *prop,
                             const char *appearance) {
  if (!prop)
    return -1;
  if (appearance) {
    size_t len = strlen(appearance);
    prop->appearance = (char *)malloc(len + 1);
    if (!prop->appearance)
      return -1;
    memcpy(prop->appearance, appearance, len + 1);
  } else {
    prop->appearance = NULL;
  }
  return 0;
}

int cmp_prop_appearance_free(cmp_prop_appearance_t *prop) {
  if (!prop)
    return -1;
  if (prop->appearance) {
    free(prop->appearance);
    prop->appearance = NULL;
  }
  return 0;
}

int cmp_prop_resize_init(cmp_prop_resize_t *prop, cmp_resize_t resize) {
  if (!prop)
    return -1;
  prop->resize = resize;
  return 0;
}

int cmp_prop_zoom_init(cmp_prop_zoom_t *prop, const char *zoom) {
  if (!prop)
    return -1;
  if (zoom) {
    size_t len = strlen(zoom);
    prop->zoom = (char *)malloc(len + 1);
    if (!prop->zoom)
      return -1;
    memcpy(prop->zoom, zoom, len + 1);
  } else {
    prop->zoom = NULL;
  }
  return 0;
}

int cmp_prop_zoom_free(cmp_prop_zoom_t *prop) {
  if (!prop)
    return -1;
  if (prop->zoom) {
    free(prop->zoom);
    prop->zoom = NULL;
  }
  return 0;
}

int cmp_prop_caret_color_init(cmp_prop_caret_color_t *prop,
                              const cmp_prop_color_t *color) {
  if (!prop)
    return -1;
  if (color) {
    prop->color = *color;
  } else {
    prop->color.type = CMP_PROP_COLOR_CURRENTCOLOR;
  }
  return 0;
}

int cmp_prop_caret_color_free(cmp_prop_caret_color_t *prop) {
  if (!prop)
    return -1;
  (void)cmp_prop_color_free(&prop->color);
  return 0;
}

int cmp_prop_caret_shape_init(cmp_prop_caret_shape_t *prop,
                              cmp_caret_shape_t shape) {
  if (!prop)
    return -1;
  prop->shape = shape;
  return 0;
}

int cmp_prop_accent_color_init(cmp_prop_accent_color_t *prop,
                               const cmp_prop_color_t *color) {
  if (!prop)
    return -1;
  if (color) {
    prop->color = *color;
  } else {
    prop->color.type = CMP_PROP_COLOR_CURRENTCOLOR;
  }
  return 0;
}

int cmp_prop_accent_color_free(cmp_prop_accent_color_t *prop) {
  if (!prop)
    return -1;
  (void)cmp_prop_color_free(&prop->color);
  return 0;
}

int cmp_prop_spatial_nav_init(cmp_prop_spatial_nav_t *group, const char *up,
                              const char *down, const char *left,
                              const char *right) {
  if (!group)
    return -1;
  memset(group, 0, sizeof(*group));
  if (up) {
    size_t len = strlen(up);
    group->nav_up = (char *)malloc(len + 1);
    if (!group->nav_up)
      goto error;
    memcpy(group->nav_up, up, len + 1);
  }
  if (down) {
    size_t len = strlen(down);
    group->nav_down = (char *)malloc(len + 1);
    if (!group->nav_down)
      goto error;
    memcpy(group->nav_down, down, len + 1);
  }
  if (left) {
    size_t len = strlen(left);
    group->nav_left = (char *)malloc(len + 1);
    if (!group->nav_left)
      goto error;
    memcpy(group->nav_left, left, len + 1);
  }
  if (right) {
    size_t len = strlen(right);
    group->nav_right = (char *)malloc(len + 1);
    if (!group->nav_right)
      goto error;
    memcpy(group->nav_right, right, len + 1);
  }
  return 0;

error:
  (void)cmp_prop_spatial_nav_free(group);
  return -1;
}

int cmp_prop_spatial_nav_free(cmp_prop_spatial_nav_t *group) {
  if (!group)
    return -1;
  if (group->nav_up) {
    free(group->nav_up);
    group->nav_up = NULL;
  }
  if (group->nav_down) {
    free(group->nav_down);
    group->nav_down = NULL;
  }
  if (group->nav_left) {
    free(group->nav_left);
    group->nav_left = NULL;
  }
  if (group->nav_right) {
    free(group->nav_right);
    group->nav_right = NULL;
  }
  return 0;
}
