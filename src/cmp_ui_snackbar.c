/* clang-format off */
#include "cmp_ui_snackbar.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct cmp_ui_snackbar {
  cmp_ui_node_t *node_root;
  cmp_ui_node_t *node_message;
  cmp_ui_node_t *node_action;
  char *message;
  char *action_label;
};

int cmp_ui_snackbar_create(cmp_ui_snackbar_t **out_snackbar,
                           const char *message, const char *action_label) {
  cmp_ui_snackbar_t *snackbar;
  int err;
  size_t len;

  if (!out_snackbar) {
    return CMP_ERROR_INVALID_ARG;
  }

  err = CMP_MALLOC(sizeof(cmp_ui_snackbar_t), (void **)&snackbar);
  if (err != CMP_SUCCESS) {
    return err;
  }
  memset(snackbar, 0, sizeof(cmp_ui_snackbar_t));

  if (message) {
    len = strlen(message);
    err = CMP_MALLOC(len + 1, (void **)&snackbar->message);
    if (err == CMP_SUCCESS) {
      memcpy(snackbar->message, message, len + 1);
    }
  }

  if (action_label) {
    len = strlen(action_label);
    err = CMP_MALLOC(len + 1, (void **)&snackbar->action_label);
    if (err == CMP_SUCCESS) {
      memcpy(snackbar->action_label, action_label, len + 1);
    }
  }

  err = cmp_ui_box_create(&snackbar->node_root);
  if (err != CMP_SUCCESS) {
    CMP_FREE(snackbar->message);
    CMP_FREE(snackbar->action_label);
    CMP_FREE(snackbar);
    return err;
  }

  /* Snackbars are typically a row layout */
  snackbar->node_root->layout->direction = CMP_FLEX_ROW;

  err = cmp_ui_text_create(&snackbar->node_message,
                           snackbar->message ? snackbar->message : "", -1);
  if (err != CMP_SUCCESS) {
    cmp_ui_node_destroy(snackbar->node_root);
    CMP_FREE(snackbar->message);
    CMP_FREE(snackbar->action_label);
    CMP_FREE(snackbar);
    return err;
  }

  cmp_ui_node_add_child(snackbar->node_root, snackbar->node_message);

  if (snackbar->action_label) {
    err = cmp_ui_button_create(&snackbar->node_action, "", 0);
    if (err == CMP_SUCCESS) {
      cmp_ui_node_t *btn_text;
      err = cmp_ui_text_create(&btn_text, snackbar->action_label, -1);
      if (err == CMP_SUCCESS) {
        cmp_ui_node_add_child(snackbar->node_action, btn_text);
      }
      cmp_ui_node_add_child(snackbar->node_root, snackbar->node_action);
    }
  }

  *out_snackbar = snackbar;
  return CMP_SUCCESS;
}

int cmp_ui_snackbar_destroy(cmp_ui_snackbar_t *snackbar) {
  if (!snackbar) {
    return CMP_ERROR_INVALID_ARG;
  }
  CMP_FREE(snackbar->message);
  CMP_FREE(snackbar->action_label);
  CMP_FREE(snackbar);
  return CMP_SUCCESS;
}

int cmp_ui_snackbar_get_node(cmp_ui_snackbar_t *snackbar,
                             cmp_ui_node_t **out_node) {
  if (!snackbar || !out_node) {
    return CMP_ERROR_INVALID_ARG;
  }
  *out_node = snackbar->node_root;
  return CMP_SUCCESS;
}

int cmp_ui_snackbar_set_message(cmp_ui_snackbar_t *snackbar,
                                const char *message) {
  size_t len;
  int err;

  if (!snackbar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (snackbar->message) {
    CMP_FREE(snackbar->message);
    snackbar->message = NULL;
  }

  if (message) {
    len = strlen(message);
    err = CMP_MALLOC(len + 1, (void **)&snackbar->message);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(snackbar->message, message, len + 1);

    if (snackbar->node_message->properties) {
      CMP_FREE(snackbar->node_message->properties);
    }
    err = CMP_MALLOC(len + 1, (void **)&snackbar->node_message->properties);
    if (err == CMP_SUCCESS) {
      memcpy(snackbar->node_message->properties, message, len + 1);
    }
  } else {
    if (snackbar->node_message->properties) {
      CMP_FREE(snackbar->node_message->properties);
    }
    err = CMP_MALLOC(1, (void **)&snackbar->node_message->properties);
    if (err == CMP_SUCCESS) {
      ((char *)snackbar->node_message->properties)[0] = '\0';
    }
  }

  return CMP_SUCCESS;
}

int cmp_ui_snackbar_set_action(cmp_ui_snackbar_t *snackbar,
                               const char *action_label) {
  size_t len;
  int err;

  if (!snackbar) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (snackbar->action_label) {
    CMP_FREE(snackbar->action_label);
    snackbar->action_label = NULL;
  }

  /* Since the layout tree modification requires replacing or updating children,
   * this is a simplified approach, only changing the internal state for the API
   * structure. Full implementation would need to add/remove the actual button
   * node from the snackbar root node.
   */
  if (action_label) {
    len = strlen(action_label);
    err = CMP_MALLOC(len + 1, (void **)&snackbar->action_label);
    if (err != CMP_SUCCESS) {
      return err;
    }
    memcpy(snackbar->action_label, action_label, len + 1);
  }

  return CMP_SUCCESS;
}
int cmp_ui_snackbar_bind_a11y(cmp_ui_snackbar_t *widget,
                              cmp_a11y_tree_t *tree) {
  if (!widget || !tree) {
    return CMP_ERROR_INVALID_ARG;
  }
  cmp_a11y_tree_add_node(tree, widget->node_root->layout->id, "alert",
                         "Snackbar");
  return CMP_SUCCESS;
}
