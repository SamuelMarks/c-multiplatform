#include "ui_list_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_list_base {
  struct ui_component *component;
  enum ui_list_orientation orientation;
  struct ui_computed *data_signal;
};

/** \brief ui_list_item_base */
struct ui_list_item_base {
  struct ui_component *component;
};

enum ui_error ui_list_base_create(struct ui_list_base **out_list) {
  struct ui_list_base *list;
  enum ui_error rc;

  if (!out_list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  list = (struct ui_list_base *)UI_MALLOC(sizeof(struct ui_list_base));
  if (!list) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&list->component);
  if (rc != UI_ERROR_NONE) {
    list->component = NULL;
    ui_list_base_destroy(list);
    return rc;
  }

  list->orientation = UI_LIST_ORIENTATION_VERTICAL;

  /* In a full engine, we would set the role="list" on the shadow root */

  *out_list = list;
  return UI_ERROR_NONE;
}

void ui_list_base_destroy(struct ui_list_base *list) {
  if (!list) {
    return;
  }
  if (list->component) {
    ui_component_destroy(list->component);
  }
  UI_FREE(list);
}

enum ui_error ui_list_base_get_component(struct ui_list_base *list,
                                         struct ui_component **out_component) {
  if (!list || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = list->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_list_base_set_orientation(struct ui_list_base *list,
                             enum ui_list_orientation orientation) {
  if (!list) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  list->orientation = orientation;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_list_base_get_orientation(struct ui_list_base *list,
                             enum ui_list_orientation *out_orientation) {
  if (!list || !out_orientation) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_orientation = list->orientation;
  return UI_ERROR_NONE;
}

enum ui_error ui_list_base_append_item(struct ui_list_base *list,
                                       struct ui_list_item_base *item) {
  if (!list || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* In a real implementation we would append the item->component->host_node
   * to the list->component->shadow_root or a specific slot. */

  return UI_ERROR_NONE;
}

enum ui_error ui_list_item_base_create(struct ui_list_item_base **out_item) {
  struct ui_list_item_base *item;
  enum ui_error rc;

  if (!out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item =
      (struct ui_list_item_base *)UI_MALLOC(sizeof(struct ui_list_item_base));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&item->component);
  if (rc != UI_ERROR_NONE) {
    item->component = NULL;
    ui_list_item_base_destroy(item);
    return rc;
  }

  /* Role="listitem" setup would occur here */

  *out_item = item;
  return UI_ERROR_NONE;
}

void ui_list_item_base_destroy(struct ui_list_item_base *item) {
  if (!item) {
    return;
  }
  if (item->component) {
    ui_component_destroy(item->component);
  }
  UI_FREE(item);
}

/** \brief ui_error */
enum ui_error
ui_list_item_base_get_component(struct ui_list_item_base *item,
                                struct ui_component **out_component) {
  if (!item || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = item->component;
  return UI_ERROR_NONE;
}

enum ui_error ui_list_base_bind_data(struct ui_list_base *widget,
                                     struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
