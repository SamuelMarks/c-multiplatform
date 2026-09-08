/**
 * @file ui_transfer_list_base.c
 * @brief ui_transfer_list_base.c implementation.
 */
/* clang-format off */
#include "ui_transfer_list_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief trigger_cva_change.
 * @param list Parameter list.
 * @return Return value.
 */
static ui_error_t trigger_cva_change(struct ui_transfer_list_base *list) {
  if (list->cva_on_change) {
    union ui_signal_payload payload;
    struct ui_transfer_list_payload *pl =
        (struct ui_transfer_list_payload *)C_MULTIPLATFORM_MALLOC(
            sizeof(struct ui_transfer_list_payload));
    if (pl) {
      pl->left_list = list->left_list;
      pl->right_list = list->right_list;
      payload.ptr_val = pl;
      (void)list->cva_on_change(payload, list->cva_on_change_user_data);
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief trigger_cva_touched.
 * @param list Parameter list.
 * @return Return value.
 */
static ui_error_t trigger_cva_touched(struct ui_transfer_list_base *list) {
  if (list->cva_on_touched) {
    (void)list->cva_on_touched(list->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief transfer_list_cva_write_value.
 * @param component Parameter component.
 * @param value Parameter value.
 * @return Return value.
 */
static ui_error_t transfer_list_cva_write_value(void *component,
                                                union ui_signal_payload value) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  struct ui_transfer_list_payload *pl;

  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;

  pl = (struct ui_transfer_list_payload *)value.ptr_val;
  if (pl) {
    list->left_list = pl->left_list;
    list->right_list = pl->right_list;
    C_MULTIPLATFORM_FREE(pl);
  }

  return UI_ERROR_NONE;
}

/* \brief transfer_list_cva_register_on_change
 */
/**
 * @brief transfer_list_cva_register_on_change.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t transfer_list_cva_register_on_change(
    void *component,
    ui_error_t (*callback)(union ui_signal_payload new_value, void *user_data),
    void *user_data) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->cva_on_change = callback;
  list->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/* \brief transfer_list_cva_register_on_touched
 */
/**
 * @brief transfer_list_cva_register_on_touched.
 * @param component Parameter component.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t transfer_list_cva_register_on_touched(
    void *component, ui_error_t (*callback)(void *user_data), void *user_data) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->cva_on_touched = callback;
  list->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief transfer_list_cva_set_disabled_state.
 * @param component Parameter component.
 * @param is_disabled Parameter is_disabled.
 * @return Return value.
 */
static ui_error_t transfer_list_cva_set_disabled_state(void *component,
                                                       int is_disabled) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_transfer_list_base_init(struct ui_transfer_list_base *list,
                           struct ui_component *component,
                           struct ui_control_value_accessor *out_cva) {
  if (!list || !component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  list->component = component;
  list->left_list = NULL;
  list->right_list = NULL;
  list->cva_on_change = NULL;
  list->cva_on_change_user_data = NULL;
  list->cva_on_touched = NULL;
  list->cva_on_touched_user_data = NULL;
  list->is_disabled = 0;

  if (out_cva) {
    out_cva->write_value = transfer_list_cva_write_value;
    out_cva->register_on_change = transfer_list_cva_register_on_change;
    out_cva->register_on_touched = transfer_list_cva_register_on_touched;
    out_cva->set_disabled_state = transfer_list_cva_set_disabled_state;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_transfer_list_base_add_item.
 * @param list Parameter list.
 * @param to_right Parameter to_right.
 * @param id Parameter id.
 * @param data Parameter data.
 * @return Return value.
 */
ui_error_t ui_transfer_list_base_add_item(struct ui_transfer_list_base *list,
                                          int to_right, int id, void *data) {
  struct ui_transfer_list_item *item;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct ui_transfer_list_item *)C_MULTIPLATFORM_MALLOC(sizeof(*item));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  item->id = id;
  item->data = data;
  item->selected = 0;

  if (to_right) {
    item->next = list->right_list;
    list->right_list = item;
  } else {
    item->next = list->left_list;
    list->left_list = item;
  }

  return trigger_cva_change(list);
}

/**
 * @brief find_item.
 * @param head Parameter head.
 * @param id Parameter id.
 * @param out_item Parameter out_item.
 * @return Return value.
 */
static ui_error_t find_item(struct ui_transfer_list_item *head, int id,
                            struct ui_transfer_list_item **out_item) {
  struct ui_transfer_list_item *curr = head;
  if (!out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_item = NULL;
  while (curr) {
    if (curr->id == id) {
      *out_item = curr;
      return UI_ERROR_NONE;
    }
    curr = curr->next;
  }
  return UI_ERROR_NOT_FOUND;
}

/* \brief ui_error
 */
ui_error_t
ui_transfer_list_base_set_selected(struct ui_transfer_list_base *list, int id,
                                   int selected) {
  struct ui_transfer_list_item *item = NULL;
  ui_error_t rc;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = find_item(list->left_list, id, &item);
  if (rc != UI_ERROR_NONE || !item) {
    rc = find_item(list->right_list, id, &item);
  }

  if (rc != UI_ERROR_NONE || !item) {
    return UI_ERROR_NOT_FOUND;
  }

  if (item->selected != selected) {
    item->selected = selected;
    rc = trigger_cva_touched(list);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = trigger_cva_change(list);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }
  return UI_ERROR_NONE;
}

/**
 * @brief move_items.
 * @param src_head Parameter src_head.
 * @param dst_head Parameter dst_head.
 * @param only_selected Parameter only_selected.
 * @param out_moved Parameter out_moved.
 * @return Return value.
 */
static ui_error_t move_items(struct ui_transfer_list_item **src_head,
                             struct ui_transfer_list_item **dst_head,
                             int only_selected, int *out_moved) {
  struct ui_transfer_list_item *curr = *src_head;
  struct ui_transfer_list_item *prev = NULL;
  struct ui_transfer_list_item *next_item;
  int moved_count = 0;
  *out_moved = 0;

  while (curr) {
    next_item = curr->next;
    if (!only_selected || curr->selected) {
      /* Remove from src */
      if (prev) {
        prev->next = next_item;
      } else {
        *src_head = next_item;
      }

      /* Add to dst */
      curr->next = *dst_head;
      *dst_head = curr;

      /* Deselect upon move */
      curr->selected = 0;
      moved_count++;
    } else {
      prev = curr;
    }
    curr = next_item;
  }
  *out_moved = moved_count;
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t
ui_transfer_list_base_move_selected(struct ui_transfer_list_base *list,
                                    int to_right) {
  int moved;
  ui_error_t rc;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = trigger_cva_touched(list);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (to_right) {
    rc = move_items(&list->left_list, &list->right_list, 1, &moved);
  } else {
    rc = move_items(&list->right_list, &list->left_list, 1, &moved);
  }
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (moved > 0) {
    rc = trigger_cva_change(list);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief ui_transfer_list_base_move_all.
 * @param list Parameter list.
 * @param to_right Parameter to_right.
 * @return Return value.
 */
ui_error_t ui_transfer_list_base_move_all(struct ui_transfer_list_base *list,
                                          int to_right) {
  int moved;
  ui_error_t rc;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  rc = trigger_cva_touched(list);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (to_right) {
    rc = move_items(&list->left_list, &list->right_list, 0, &moved);
  } else {
    rc = move_items(&list->right_list, &list->left_list, 0, &moved);
  }
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (moved > 0) {
    rc = trigger_cva_change(list);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief free_list.
 * @param head Parameter head.
 * @return Return value.
 */
static ui_error_t free_list(struct ui_transfer_list_item *head) {
  struct ui_transfer_list_item *curr = head;
  struct ui_transfer_list_item *next_item;
  while (curr) {
    next_item = curr->next;
    C_MULTIPLATFORM_FREE(curr);
    curr = next_item;
  }
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t ui_transfer_list_base_cleanup(struct ui_transfer_list_base *list) {
  ui_error_t rc;

  if (!list) {
    return UI_ERROR_NONE;
  }

  rc = free_list(list->left_list);
  list->left_list = NULL;
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = free_list(list->right_list);
  list->right_list = NULL;
  return rc;
}
