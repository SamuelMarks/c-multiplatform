/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

typedef struct cmp_system_menu_item {
  char *label;
  int id;
} cmp_system_menu_item_t;

struct cmp_system_menu {
  cmp_system_menu_item_t *items;
  size_t count;
  size_t capacity;
  int is_showing;
};

/**
 * @brief cmp_system_menu_create
 *
 * @param out_menu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_menu_create(cmp_system_menu_t **out_menu) {
  int rc = CMP_SUCCESS;
  cmp_system_menu_t *menu;
  if (!out_menu) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (CMP_MALLOC(sizeof(cmp_system_menu_t), (void **)&menu) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
  memset(menu, 0, sizeof(cmp_system_menu_t));
  *out_menu = menu;

  return rc;
}

/**
 * @brief cmp_system_menu_destroy
 *
 * @param menu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_menu_destroy(cmp_system_menu_t *menu) {
  int rc = CMP_SUCCESS;
  size_t i;
  if (!menu) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (menu->items) {
    for (i = 0; i < menu->count; i++) {
      if (menu->items[i].label) {
        CMP_FREE(menu->items[i].label);
      }
    }
    CMP_FREE(menu->items);
  }
  CMP_FREE(menu);

  return rc;
}

/**
 * @brief cmp_system_menu_add_item
 *
 * @param menu Parameter description.
 * @param label Parameter description.
 * @param id Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_menu_add_item(cmp_system_menu_t *menu, const char *label,
                             int id) {
  int rc = CMP_SUCCESS;
  size_t len;
  if (!menu || !label) {
    return CMP_ERROR_INVALID_ARG;
  }
  if (menu->count >= menu->capacity) {
    size_t new_cap = menu->capacity == 0 ? 4 : menu->capacity * 2;
    cmp_system_menu_item_t *new_items;
    if (CMP_MALLOC(new_cap * sizeof(cmp_system_menu_item_t),
                   (void **)&new_items) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    if (menu->items) {
      memcpy(new_items, menu->items,
             menu->count * sizeof(cmp_system_menu_item_t));
      CMP_FREE(menu->items);
    }
    menu->items = new_items;
    menu->capacity = new_cap;
  }

  len = strlen(label);
  if (CMP_MALLOC(len + 1, (void **)&menu->items[menu->count].label) !=
      CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }
#if defined(_MSC_VER)
  strcpy_s(menu->items[menu->count].label, len + 1, label);
#else
  strcpy(menu->items[menu->count].label, label);
#endif
  menu->items[menu->count].id = id;
  menu->count++;

  return rc;
}

/**
 * @brief cmp_system_menu_show
 *
 * @param menu Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_system_menu_show(cmp_system_menu_t *menu) {
  int rc = CMP_SUCCESS;
  if (!menu) {
    return CMP_ERROR_INVALID_ARG;
  }
  menu->is_showing = 1;

  return rc;
}
