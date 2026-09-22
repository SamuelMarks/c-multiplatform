/**
 * @file ui_dl_list.c
 * @brief Implementation of the <dl> description list component.
 */

/* clang-format off */
#include "ui_dl_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

ui_error_t ui_dl_list_create(struct ui_dl_list **out_dl) {
  struct ui_dl_list *dl;
  ui_error_t err;

  if (out_dl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dl = (struct ui_dl_list *)malloc(sizeof(struct ui_dl_list));
  if (dl == NULL) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(dl, 0, sizeof(struct ui_dl_list));

  err = ui_component_create(&dl->component);
  if (err != UI_ERROR_NONE) {
    free(dl);
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dl->dl_container);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(dl->component);
    free(dl);
    return err;
  }

  err = ui_dom_node_set_tag_name(dl->dl_container, "dl");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dl->dl_container);
    ui_component_destroy(dl->component);
    free(dl);
    return err;
  }

  err = ui_dom_node_set_attribute(dl->dl_container, "role", "list");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dl->dl_container);
    ui_component_destroy(dl->component);
    free(dl);
    return err;
  }

  err = ui_dom_node_set_attribute(dl->dl_container, "class", "ui-dl-container");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dl->dl_container);
    ui_component_destroy(dl->component);
    free(dl);
    return err;
  }

  dl->component->shadow_root = dl->dl_container;

  *out_dl = dl;
  return UI_ERROR_NONE;
}

ui_error_t ui_dl_list_destroy(struct ui_dl_list *dl) {
  ui_error_t err;
  ui_error_t ret = UI_ERROR_NONE;

  if (dl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dl->component != NULL) {
    err = ui_component_destroy(dl->component);
    if (err != UI_ERROR_NONE && ret == UI_ERROR_NONE) {
      ret = err;
    }
    dl->component = NULL;
  }

  free(dl);
  return ret;
}

ui_error_t ui_dl_list_get_component(struct ui_dl_list *dl,
                                    struct ui_component **out_comp) {
  if (dl == NULL || out_comp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_comp = dl->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_dl_list_clear(struct ui_dl_list *dl) {
  size_t i;
  ui_error_t err;

  if (dl == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < dl->count; ++i) {
    if (dl->items[i].row_node != NULL) {
      err = ui_dom_node_remove_child(dl->dl_container, dl->items[i].row_node);
      if (err != UI_ERROR_NONE) {
        return err;
      }
      err = ui_dom_node_destroy(dl->items[i].row_node);
      if (err != UI_ERROR_NONE) {
        return err;
      }
      dl->items[i].row_node = NULL;
    }
  }

  memset(dl->items, 0, sizeof(dl->items));
  dl->count = 0;
  dl->selected_user_id[0] = '\0';

  return UI_ERROR_NONE;
}

ui_error_t ui_dl_list_add_user(struct ui_dl_list *dl,
                               const struct oauth2_user *user,
                               ui_dl_list_click_fn on_click, void *user_data) {
  struct ui_dl_item *item;
  struct ui_dom_node *row_node;
  struct ui_dom_node *dt_node;
  struct ui_dom_node *dd_node;
  struct ui_dom_node *dt_text;
  struct ui_dom_node *dd_text;
  char title_buf[128];
  enum oauth2_app_error app_rc;
  ui_error_t err;

  if (dl == NULL || user == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (dl->count >= UI_DL_LIST_MAX_ITEMS) {
    return UI_ERROR_QUEUE_FULL;
  }

  item = &dl->items[dl->count];

  app_rc = oauth2_safe_strcpy(item->user_id, sizeof(item->user_id), user->id);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  app_rc = oauth2_safe_strcpy(item->username, sizeof(item->username),
                              user->username);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  app_rc = oauth2_safe_strcpy(item->about_me, sizeof(item->about_me),
                              user->about_me);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item->role = user->role;
  item->sync_status = user->sync_status;
  item->on_click = on_click;
  item->user_data = user_data;

  /* Create wrapper row */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &row_node);
  if (err != UI_ERROR_NONE) {
    return err;
  }
  err = ui_dom_node_set_tag_name(row_node, "div");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_attribute(row_node, "class", "ui-dl-row");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_attribute(row_node, "role", "listitem");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_attribute(row_node, "data-user-id", user->id);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }

  /* Create <dt> element */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dt_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_tag_name(dt_node, "dt");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_attribute(dt_node, "class", "ui-dl-term");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }

#if defined(_MSC_VER)
  sprintf_s(title_buf, sizeof(title_buf), "%s [%s] [%s]", user->username,
            (user->role == OAUTH2_ROLE_ADMIN) ? "ADMIN" : "USER",
            (user->sync_status == OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY)
                ? "DIRTY"
                : "SYNCED");
#else
  sprintf(title_buf, "%s [%s] [%s]", user->username,
          (user->role == OAUTH2_ROLE_ADMIN) ? "ADMIN" : "USER",
          (user->sync_status == OAUTH2_SYNC_STATUS_MODIFIED_LOCALLY)
              ? "DIRTY"
              : "SYNCED");
#endif

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &dt_text);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_text_content(dt_text, title_buf);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_text);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_append_child(dt_node, dt_text);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_text);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }

  /* Create <dd> element */
  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dd_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_tag_name(dd_node, "dd");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_attribute(dd_node, "class", "ui-dl-desc");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &dd_text);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_set_text_content(dd_text, user->about_me);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_text);
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_append_child(dd_node, dd_text);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_text);
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }

  /* Assemble row */
  err = ui_dom_node_append_child(row_node, dt_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(dt_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_append_child(row_node, dd_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(dd_node);
    ui_dom_node_destroy(row_node);
    return err;
  }
  err = ui_dom_node_append_child(dl->dl_container, row_node);
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(row_node);
    return err;
  }

  item->row_node = row_node;
  item->dt_node = dt_node;
  item->dd_node = dd_node;
  dl->count++;

  return UI_ERROR_NONE;
}

ui_error_t ui_dl_list_set_selected(struct ui_dl_list *dl, const char *user_id) {
  size_t i;
  enum oauth2_app_error app_rc;

  if (dl == NULL || user_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  app_rc = oauth2_safe_strcpy(dl->selected_user_id,
                              sizeof(dl->selected_user_id), user_id);
  if (app_rc != OAUTH2_APP_OK) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  for (i = 0; i < dl->count; ++i) {
    if (strcmp(dl->items[i].user_id, user_id) == 0) {
      return ui_dom_node_set_attribute(dl->items[i].row_node, "aria-selected",
                                       "true");
    } else {
      ui_dom_node_set_attribute(dl->items[i].row_node, "aria-selected",
                                "false");
    }
  }

  return UI_ERROR_NOT_FOUND;
}

ui_error_t ui_dl_list_trigger_click(struct ui_dl_list *dl,
                                    const char *user_id) {
  size_t i;
  ui_error_t err;

  if (dl == NULL || user_id == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_dl_list_set_selected(dl, user_id);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  for (i = 0; i < dl->count; ++i) {
    if (strcmp(dl->items[i].user_id, user_id) == 0) {
      if (dl->items[i].on_click != NULL) {
        return dl->items[i].on_click(user_id, dl->items[i].user_data);
      }
      return UI_ERROR_NONE;
    }
  }

  return UI_ERROR_NOT_FOUND;
}
