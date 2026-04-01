/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef enum {
  ITEM_TYPE_STANDARD,
  ITEM_TYPE_SUBMENU,
  ITEM_TYPE_DEFERRED
} item_type_t;

struct cmp_menu_item {
  char *title;
  char *symbol;
  item_type_t type;
  cmp_menu_item_role_t role;
  cmp_menu_item_state_t state;
  struct cmp_menu *child_menu; /* for submenu */
};

struct cmp_menu {
  cmp_menu_presentation_t presentation;
  cmp_menu_item_t **items;
  size_t count;
  size_t capacity;
};

int cmp_menu_create(cmp_menu_t **out_menu,
                    cmp_menu_presentation_t presentation) {
  struct cmp_menu *ctx;
  if (!out_menu)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_menu), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  ctx->presentation = presentation;
  ctx->items = NULL;
  ctx->count = 0;
  ctx->capacity = 0;

  *out_menu = (cmp_menu_t *)ctx;
  return CMP_SUCCESS;
}

int cmp_menu_destroy(cmp_menu_t *menu_opaque) {
  struct cmp_menu *menu = (struct cmp_menu *)menu_opaque;
  size_t i;
  if (!menu)
    return CMP_SUCCESS;

  if (menu->items) {
    for (i = 0; i < menu->count; ++i) {
      if (menu->items[i]) {
        if (menu->items[i]->title)
          CMP_FREE(menu->items[i]->title);
        if (menu->items[i]->symbol)
          CMP_FREE(menu->items[i]->symbol);
        if (menu->items[i]->child_menu)
          cmp_menu_destroy(menu->items[i]->child_menu);
        CMP_FREE(menu->items[i]);
      }
    }
    CMP_FREE(menu->items);
  }
  CMP_FREE(menu);
  return CMP_SUCCESS;
}

int cmp_menu_item_create(cmp_menu_item_t **out_item, const char *title,
                         const char *symbol, cmp_menu_item_role_t role) {
  struct cmp_menu_item *item;
  size_t len;

  if (!out_item || !title)
    return CMP_ERROR_INVALID_ARG;
  if (CMP_MALLOC(sizeof(struct cmp_menu_item), (void **)&item) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  item->type = ITEM_TYPE_STANDARD;
  item->role = role;
  item->state = CMP_MENU_ITEM_STATE_OFF;
  item->child_menu = NULL;

  len = strlen(title);
  if (CMP_MALLOC(len + 1, (void **)&item->title) != CMP_SUCCESS)
    return CMP_ERROR_OOM;
#if defined(_MSC_VER)
  strcpy_s(item->title, len + 1, title);
#else
  strcpy(item->title, title);
#endif

  item->symbol = NULL;
  if (symbol) {
    len = strlen(symbol);
    if (CMP_MALLOC(len + 1, (void **)&item->symbol) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
#if defined(_MSC_VER)
    strcpy_s(item->symbol, len + 1, symbol);
#else
    strcpy(item->symbol, symbol);
#endif
  }

  *out_item = (cmp_menu_item_t *)item;
  return CMP_SUCCESS;
}

int cmp_menu_add_item(cmp_menu_t *menu_opaque, cmp_menu_item_t *item_opaque) {
  struct cmp_menu *menu = (struct cmp_menu *)menu_opaque;
  struct cmp_menu_item *item = (struct cmp_menu_item *)item_opaque;
  cmp_menu_item_t **new_items;
  size_t new_cap;

  if (!menu || !item)
    return CMP_ERROR_INVALID_ARG;

  if (menu->count == menu->capacity) {
    new_cap = menu->capacity == 0 ? 4 : menu->capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_menu_item_t *), (void **)&new_items) !=
        CMP_SUCCESS)
      return CMP_ERROR_OOM;
    if (menu->items) {
      memcpy(new_items, menu->items, menu->count * sizeof(cmp_menu_item_t *));
      CMP_FREE(menu->items);
    }
    menu->items = new_items;
    menu->capacity = new_cap;
  }

  menu->items[menu->count++] = item;
  return CMP_SUCCESS;
}

int cmp_menu_item_create_submenu(cmp_menu_item_t **out_item, const char *title,
                                 const char *symbol, cmp_menu_t *child_menu) {
  int err;
  struct cmp_menu_item *item;

  if (!child_menu)
    return CMP_ERROR_INVALID_ARG;

  err = cmp_menu_item_create(out_item, title, symbol,
                             CMP_MENU_ITEM_ROLE_STANDARD);
  if (err != CMP_SUCCESS)
    return err;

  item = (struct cmp_menu_item *)*out_item;
  item->type = ITEM_TYPE_SUBMENU;
  item->child_menu = child_menu; /* Assumes ownership */

  return CMP_SUCCESS;
}

int cmp_menu_item_create_deferred(cmp_menu_item_t **out_item,
                                  const char *title) {
  int err;
  struct cmp_menu_item *item;

  err =
      cmp_menu_item_create(out_item, title, NULL, CMP_MENU_ITEM_ROLE_STANDARD);
  if (err != CMP_SUCCESS)
    return err;

  item = (struct cmp_menu_item *)*out_item;
  item->type = ITEM_TYPE_DEFERRED;

  return CMP_SUCCESS;
}

int cmp_menu_item_set_state(cmp_menu_item_t *item_opaque,
                            cmp_menu_item_state_t state) {
  struct cmp_menu_item *item = (struct cmp_menu_item *)item_opaque;
  if (!item)
    return CMP_ERROR_INVALID_ARG;
  item->state = state;
  return CMP_SUCCESS;
}

int cmp_menu_resolve_presentation(cmp_menu_t *menu_opaque, int *out_bg_blur,
                                  int *out_lift_target, int *out_has_arrow) {
  struct cmp_menu *menu = (struct cmp_menu *)menu_opaque;
  if (!menu || !out_bg_blur || !out_lift_target || !out_has_arrow)
    return CMP_ERROR_INVALID_ARG;

  *out_bg_blur = 0;
  *out_lift_target = 0;
  *out_has_arrow = 0;

  switch (menu->presentation) {
  case CMP_MENU_PRESENTATION_CONTEXT:
    *out_bg_blur = 1;
    *out_lift_target = 1;
    break;
  case CMP_MENU_PRESENTATION_ACTION_SHEET:
    *out_bg_blur = 1; /* Slight dim, not heavy blur usually, but we represent it
                         this way for now */
    break;
  case CMP_MENU_PRESENTATION_POPOVER:
    *out_has_arrow = 1;
    break;
  }

  return CMP_SUCCESS;
}

int cmp_menu_item_resolve_visuals(cmp_menu_item_t *item_opaque, int *out_is_red,
                                  const char **out_state_icon) {
  struct cmp_menu_item *item = (struct cmp_menu_item *)item_opaque;
  if (!item || !out_is_red || !out_state_icon)
    return CMP_ERROR_INVALID_ARG;

  *out_is_red = (item->role == CMP_MENU_ITEM_ROLE_DESTRUCTIVE) ? 1 : 0;

  *out_state_icon = NULL;
  if (item->state == CMP_MENU_ITEM_STATE_ON) {
    *out_state_icon = "checkmark"; /* Standard SF Symbol for selected */
  } else if (item->state == CMP_MENU_ITEM_STATE_MIXED) {
    *out_state_icon = "minus"; /* Standard SF Symbol for mixed/indeterminate */
  } else if (item->type == ITEM_TYPE_DEFERRED) {
    *out_state_icon = "spinner"; /* Indicator to show loading */
  } else if (item->type == ITEM_TYPE_SUBMENU) {
    *out_state_icon = "chevron.right"; /* Nested indicator */
  }

  return CMP_SUCCESS;
}
