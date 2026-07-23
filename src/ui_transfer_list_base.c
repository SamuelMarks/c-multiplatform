/* clang-format off */
#include "ui_transfer_list_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

static enum ui_error trigger_cva_change(struct ui_transfer_list_base *list) {
  if (list && list->cva_on_change) {
    union ui_signal_payload payload;
    struct ui_transfer_list_payload *pl =
        (struct ui_transfer_list_payload *)UI_MALLOC(
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

static enum ui_error trigger_cva_touched(struct ui_transfer_list_base *list) {
  if (list && list->cva_on_touched) {
    (void)list->cva_on_touched(list->cva_on_touched_user_data);
  }
  return UI_ERROR_NONE;
}

static enum ui_error
transfer_list_cva_write_value(void *component, union ui_signal_payload value) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  struct ui_transfer_list_payload *pl;

  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;

  pl = (struct ui_transfer_list_payload *)value.ptr_val;
  if (pl) {
    list->left_list = pl->left_list;
    list->right_list = pl->right_list;
    UI_FREE(pl);
  }

  return UI_ERROR_NONE;
}

/** \brief transfer_list_cva_register_on_change */
static enum ui_error transfer_list_cva_register_on_change(
    void *component,
    enum ui_error (*callback)(union ui_signal_payload new_value,
                              void *user_data),
    void *user_data) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->cva_on_change = callback;
  list->cva_on_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/** \brief transfer_list_cva_register_on_touched */
static enum ui_error transfer_list_cva_register_on_touched(
    void *component, enum ui_error (*callback)(void *user_data),
    void *user_data) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->cva_on_touched = callback;
  list->cva_on_touched_user_data = user_data;
  return UI_ERROR_NONE;
}

static enum ui_error transfer_list_cva_set_disabled_state(void *component,
                                                          int is_disabled) {
  struct ui_transfer_list_base *list =
      (struct ui_transfer_list_base *)component;
  if (!list)
    return UI_ERROR_INVALID_ARGUMENT;
  list->is_disabled = is_disabled;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

enum ui_error ui_transfer_list_base_add_item(struct ui_transfer_list_base *list,
                                             int to_right, int id, void *data) {
  struct ui_transfer_list_item *item;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct ui_transfer_list_item *)UI_MALLOC(sizeof(*item));
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

  trigger_cva_change(list);

  return UI_ERROR_NONE;
}

static struct ui_transfer_list_item *
find_item(struct ui_transfer_list_item *head, int id) {
  struct ui_transfer_list_item *curr = head;
  while (curr) {
    if (curr->id == id) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

/** \brief ui_error */
enum ui_error
ui_transfer_list_base_set_selected(struct ui_transfer_list_base *list, int id,
                                   int selected) {
  struct ui_transfer_list_item *item;

  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  item = find_item(list->left_list, id);
  if (!item) {
    item = find_item(list->right_list, id);
  }

  if (!item) {
    return UI_ERROR_NOT_FOUND;
  }

  if (item->selected != selected) {
    item->selected = selected;
    trigger_cva_touched(list);
    trigger_cva_change(list);
  }
  return UI_ERROR_NONE;
}

static enum ui_error move_items(struct ui_transfer_list_item **src_head,
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

/** \brief ui_error */
enum ui_error
ui_transfer_list_base_move_selected(struct ui_transfer_list_base *list,
                                    int to_right) {
  int moved;
  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  trigger_cva_touched(list);

  if (to_right) {
    (void)move_items(&list->left_list, &list->right_list, 1, &moved);
  } else {
    (void)move_items(&list->right_list, &list->left_list, 1, &moved);
  }

  if (moved > 0) {
    trigger_cva_change(list);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_transfer_list_base_move_all(struct ui_transfer_list_base *list,
                                             int to_right) {
  int moved;
  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (list->is_disabled) {
    return UI_ERROR_NONE;
  }

  trigger_cva_touched(list);

  if (to_right) {
    (void)move_items(&list->left_list, &list->right_list, 0, &moved);
  } else {
    (void)move_items(&list->right_list, &list->left_list, 0, &moved);
  }

  if (moved > 0) {
    trigger_cva_change(list);
  }

  return UI_ERROR_NONE;
}

static enum ui_error free_list(struct ui_transfer_list_item *head) {
  struct ui_transfer_list_item *curr = head;
  struct ui_transfer_list_item *next_item;
  while (curr) {
    next_item = curr->next;
    UI_FREE(curr);
    curr = next_item;
  }
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_transfer_list_base_cleanup(struct ui_transfer_list_base *list) {
  if (!list) {
    return UI_ERROR_NONE;
  }

  free_list(list->left_list);
  list->left_list = NULL;

  free_list(list->right_list);
  list->right_list = NULL;
  return UI_ERROR_NONE;
}
