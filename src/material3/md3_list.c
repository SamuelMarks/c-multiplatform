/**
 * @file md3_list.c
 * @brief Material 3 List and List Item components implementation.
 */

/* clang-format off */
#include "material3/md3_list.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_list_create(struct ui_engine *engine,
                           struct md3_list **out_list) {
  struct md3_list *list;
  ui_error_t rc;

  if (!engine || !out_list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  list = (struct md3_list *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_list));
  if (!list) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(list, 0, sizeof(struct md3_list));

  rc = ui_list_base_create(&list->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(list);
    return rc;
  }

  *out_list = list;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_destroy(struct md3_list *list) {
  ui_error_t rc;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_list_base_destroy(list->base);
  C_MULTIPLATFORM_FREE(list);
  return rc;
}

ui_error_t md3_list_set_segmented(struct md3_list *list, int is_segmented) {
  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  list->is_segmented = is_segmented ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_append_item(struct md3_list *list,
                                struct md3_list_item *item) {
  ui_error_t rc;

  if (!list || !item || !item->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_list_base_append_item(list->base, item->base);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_create(struct ui_engine *engine,
                                enum md3_list_item_lines lines,
                                struct md3_list_item **out_item) {
  struct md3_list_item *item;
  ui_error_t rc;

  if (!engine || !out_item ||
      (unsigned)lines < (unsigned)MD3_LIST_ITEM_ONE_LINE ||
      (unsigned)lines > (unsigned)MD3_LIST_ITEM_THREE_LINE) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct md3_list_item *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_list_item));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(item, 0, sizeof(struct md3_list_item));
  item->lines = lines;

  rc = ui_list_item_base_create(&item->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(item);
    return rc;
  }

  *out_item = item;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_destroy(struct md3_list_item *item) {
  ui_error_t rc;

  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_list_item_base_destroy(item->base);
  C_MULTIPLATFORM_FREE(item);
  return rc;
}

ui_error_t md3_list_item_set_headline(struct md3_list_item *item,
                                      const char *headline) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (headline) {
#if defined(_MSC_VER)
    strncpy_s(item->headline, sizeof(item->headline), headline,
              sizeof(item->headline) - 1);
#else
    strncpy(item->headline, headline, sizeof(item->headline) - 1);
    item->headline[sizeof(item->headline) - 1] = '\0';
#endif
  } else {
    item->headline[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_set_supporting_text(struct md3_list_item *item,
                                             const char *supporting_text) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (supporting_text) {
#if defined(_MSC_VER)
    strncpy_s(item->supporting_text, sizeof(item->supporting_text),
              supporting_text, sizeof(item->supporting_text) - 1);
#else
    strncpy(item->supporting_text, supporting_text,
            sizeof(item->supporting_text) - 1);
    item->supporting_text[sizeof(item->supporting_text) - 1] = '\0';
#endif
  } else {
    item->supporting_text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t
md3_list_item_set_trailing_supporting_text(struct md3_list_item *item,
                                           const char *trailing_text) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (trailing_text) {
#if defined(_MSC_VER)
    strncpy_s(item->trailing_supporting_text,
              sizeof(item->trailing_supporting_text), trailing_text,
              sizeof(item->trailing_supporting_text) - 1);
#else
    strncpy(item->trailing_supporting_text, trailing_text,
            sizeof(item->trailing_supporting_text) - 1);
    item->trailing_supporting_text[sizeof(item->trailing_supporting_text) - 1] =
        '\0';
#endif
  } else {
    item->trailing_supporting_text[0] = '\0';
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_set_selected(struct md3_list_item *item,
                                      int selected) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  item->selected = selected ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_get_selected(const struct md3_list_item *item,
                                      int *out_selected) {
  if (!item || !out_selected) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_selected = item->selected;
  return UI_ERROR_NONE;
}

ui_error_t md3_list_item_get_component(struct md3_list_item *item,
                                       struct ui_component **out_component) {
  ui_error_t rc;

  if (!item || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_list_item_base_get_component(item->base, out_component);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
