/**
 * @file md2_bottom_navigation.c
 * @brief Implementation of Material Design 2 Bottom Navigation component.
 */

/* clang-format off */
#include "material2/md2_bottom_navigation.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include <string.h>
/* clang-format on */

struct md2_bottom_navigation {
  struct ui_bottom_nav_base *base;
  enum md2_bottom_navigation_variant variant;
};

struct md2_bottom_navigation_item {
  struct ui_bottom_nav_item_base *base;
  char *label;
  char *icon;
};

ui_error_t
md2_bottom_navigation_create(struct ui_engine *engine,
                             enum md2_bottom_navigation_variant variant,
                             struct md2_bottom_navigation **out_nav) {
  struct md2_bottom_navigation *nav;
  ui_error_t rc;

  if (!engine || !out_nav) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  nav = (struct md2_bottom_navigation *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_bottom_navigation));
  if (!nav) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(nav, 0, sizeof(*nav));

  nav->variant = variant;

  rc = ui_bottom_nav_base_create(&nav->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(nav);
    return rc;
  }

  *out_nav = nav;
  return UI_ERROR_NONE;
}

ui_error_t md2_bottom_navigation_destroy(struct md2_bottom_navigation *nav) {
  ui_error_t rc;

  if (!nav) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_nav_base_destroy(nav->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  C_MULTIPLATFORM_FREE(nav);
  return UI_ERROR_NONE;
}

ui_error_t
md2_bottom_navigation_get_base(struct md2_bottom_navigation *nav,
                               struct ui_bottom_nav_base **out_base) {
  if (!nav || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = nav->base;
  return UI_ERROR_NONE;
}

ui_error_t
md2_bottom_navigation_append_item(struct md2_bottom_navigation *nav,
                                  struct md2_bottom_navigation_item *item) {
  ui_error_t rc;

  if (!nav || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_nav_base_append_item(nav->base, item->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md2_bottom_navigation_item_create(
    struct ui_engine *engine, const char *label, const char *icon,
    struct md2_bottom_navigation_item **out_item) {
  struct md2_bottom_navigation_item *item;
  ui_error_t rc;
  size_t label_len;
  size_t icon_len;

  if (!engine || !out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct md2_bottom_navigation_item *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md2_bottom_navigation_item));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(item, 0, sizeof(*item));

  if (label) {
    label_len = strlen(label);
    item->label = (char *)C_MULTIPLATFORM_MALLOC(label_len + 1);
    if (!item->label) {
      C_MULTIPLATFORM_FREE(item);
      return UI_ERROR_OUT_OF_MEMORY;
    }

#if defined(_MSC_VER)
    strcpy_s(item->label, label_len + 1, label);
#else
    strncpy(item->label, label, label_len);
    item->label[label_len] = '\0';
#endif
  }

  if (icon) {
    icon_len = strlen(icon);
    item->icon = (char *)C_MULTIPLATFORM_MALLOC(icon_len + 1);
    if (!item->icon) {
      if (item->label) {
        C_MULTIPLATFORM_FREE(item->label);
      }
      C_MULTIPLATFORM_FREE(item);
      return UI_ERROR_OUT_OF_MEMORY;
    }

#if defined(_MSC_VER)
    strcpy_s(item->icon, icon_len + 1, icon);
#else
    strncpy(item->icon, icon, icon_len);
    item->icon[icon_len] = '\0';
#endif
  }

  rc = ui_bottom_nav_item_base_create(&item->base);
  if (rc != UI_ERROR_NONE) {
    if (item->label) {
      C_MULTIPLATFORM_FREE(item->label);
    }
    if (item->icon) {
      C_MULTIPLATFORM_FREE(item->icon);
    }
    C_MULTIPLATFORM_FREE(item);
    return rc;
  }

  *out_item = item;
  return UI_ERROR_NONE;
}

ui_error_t
md2_bottom_navigation_item_destroy(struct md2_bottom_navigation_item *item) {
  ui_error_t rc;

  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_bottom_nav_item_base_destroy(item->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (item->label) {
    C_MULTIPLATFORM_FREE(item->label);
  }
  if (item->icon) {
    C_MULTIPLATFORM_FREE(item->icon);
  }

  C_MULTIPLATFORM_FREE(item);
  return UI_ERROR_NONE;
}

ui_error_t
md2_bottom_navigation_item_get_base(struct md2_bottom_navigation_item *item,
                                    struct ui_bottom_nav_item_base **out_base) {
  if (!item || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = item->base;
  return UI_ERROR_NONE;
}
