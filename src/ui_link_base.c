/**
 * @file ui_link_base.c
 * @brief ui_link_base.c implementation.
 */
/* clang-format off */
#include "ui_link_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_link_base_mock_fail = 0;

/**
 * @brief mock_link_component_destroy.
 * @param comp Parameter comp.
 * @return Return value.
 */
static ui_error_t mock_link_component_destroy(struct ui_component *comp) {
  if (g_link_base_mock_fail == 1) {
    (ui_component_destroy)(comp);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_link_component_destroy
/** @endcond */

/**
 * @brief mock_link_dom_node_destroy.
 * @param node Parameter node.
 * @return Return value.
 */
static ui_error_t mock_link_dom_node_destroy(struct ui_dom_node *node) {
  if (g_link_base_mock_fail == 2) {
    (ui_dom_node_destroy)(node);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_destroy)(node);
}
#undef ui_dom_node_destroy
/** @cond */
#define ui_dom_node_destroy mock_link_dom_node_destroy
/** @endcond */

/**
 * @brief mock_link_dom_node_append_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_link_dom_node_append_child(struct ui_dom_node *parent,
                                                  struct ui_dom_node *child) {
  if (g_link_base_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_link_dom_node_append_child
/** @endcond */
#endif

/**
 * @brief ui_link_base_create.
 * @param out_link Parameter out_link.
 * @return Return value.
 */
ui_error_t ui_link_base_create(struct ui_link_base **out_link) {
  struct ui_link_base *link;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_link) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  link = (struct ui_link_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_link_base));
  if (!link) {
    ui_error_t rc_cleanup = ui_component_destroy(base_comp);
    if (rc_cleanup != UI_ERROR_NONE) {
      return rc_cleanup;
    }
    return UI_ERROR_OUT_OF_MEMORY;
  }

  link->base = *base_comp;
  C_MULTIPLATFORM_FREE(base_comp);

  err = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &link->base.shadow_root);
  if (err != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(link);
    return err;
  }

  err = ui_dom_node_set_tag_name(link->base.shadow_root, "ui-link");
  if (err != UI_ERROR_NONE) {
    ui_error_t rc_cleanup = ui_dom_node_destroy(link->base.shadow_root);
    if (rc_cleanup != UI_ERROR_NONE) {
      err = rc_cleanup;
    }
    C_MULTIPLATFORM_FREE(link);
    return err;
  }

  /* Links are focusable by default */
  err = ui_dom_node_set_attribute(link->base.shadow_root, "tabindex", "0");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(link->base.shadow_root);
    C_MULTIPLATFORM_FREE(link);
    return err;
  }
  err = ui_dom_node_set_attribute(link->base.shadow_root, "role", "link");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(link->base.shadow_root);
    C_MULTIPLATFORM_FREE(link);
    return err;
  }

  *out_link = link;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_link_base_set_href.
 * @param link Parameter link.
 * @param url Parameter url.
 * @return Return value.
 */
ui_error_t ui_link_base_set_href(struct ui_link_base *link, const char *url) {
  if (!link || !url) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_dom_node_set_attribute(link->base.shadow_root, "href", url);
}

/**
 * @brief ui_link_base_set_text.
 * @param link Parameter link.
 * @param text Parameter text.
 * @return Return value.
 */
ui_error_t ui_link_base_set_text(struct ui_link_base *link, const char *text) {
  struct ui_dom_node *text_node;
  ui_error_t err;

  if (!link || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Create or find the text node */
  if (!link->base.shadow_root->first_child) {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_dom_node_append_child(link->base.shadow_root, text_node);
    if (err != UI_ERROR_NONE) {
      ui_dom_node_destroy(text_node);
      return err;
    }

  } else {
    text_node = link->base.shadow_root->first_child;
  }

  return ui_dom_node_set_text_content(text_node, text);
}

/**
 * @brief ui_link_base_bind_disabled.
 * @param widget Parameter widget.
 * @param disabled_signal Parameter disabled_signal.
 * @return Return value.
 */
ui_error_t ui_link_base_bind_disabled(struct ui_link_base *widget,
                                      struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_link_base_bind_text.
 * @param widget Parameter widget.
 * @param text_signal Parameter text_signal.
 * @return Return value.
 */
ui_error_t ui_link_base_bind_text(struct ui_link_base *widget,
                                  struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
