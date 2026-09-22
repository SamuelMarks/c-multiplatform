/**
 * @file ui_badge_base.c
 * @brief Implementation of the badge base component.
 */

/* clang-format off */
#include "ui_badge_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_badge_mock_fail = 0;
/**
 * @brief mock_dom_node_set_tag_name.
 * @param node Parameter node.
 * @param tag Parameter tag.
 * @return Return value.
 */
static ui_error_t mock_dom_node_set_tag_name(struct ui_dom_node *node,
                                             const char *tag) {
  if (g_badge_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_set_tag_name)(node, tag);
}
#undef ui_dom_node_set_tag_name
/** @cond */
#define ui_dom_node_set_tag_name mock_dom_node_set_tag_name
/** @endcond */

/**
 * @brief mock_dom_node_create.
 * @param type Parameter type.
 * @param out Parameter out.
 * @return Return value.
 */
static ui_error_t mock_dom_node_create(enum ui_dom_node_type type,
                                       struct ui_dom_node **out) {
  if (g_badge_mock_fail == 2)
    return UI_ERROR_UNKNOWN;
  /* To fail ONLY on the text node creation */
  if (g_badge_mock_fail == 20) {
    if (type == UI_DOM_NODE_TYPE_TEXT)
      return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_create)(type, out);
}
#undef ui_dom_node_create
/** @cond */
#define ui_dom_node_create mock_dom_node_create
/** @endcond */

/**
 * @brief mock_dom_node_append_child.
 * @param parent Parameter parent.
 * @param child Parameter child.
 * @return Return value.
 */
static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_badge_mock_fail == 3)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

static ui_error_t mock_badge_component_destroy(struct ui_component *comp) {
  if (g_badge_mock_fail == 4) {
    (ui_component_destroy)(comp);
    return UI_ERROR_UNKNOWN;
  }
  return (ui_component_destroy)(comp);
}
#undef ui_component_destroy
/** @cond */
#define ui_component_destroy mock_badge_component_destroy
/** @endcond */
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

/**
 * @struct ui_badge_base
 * @struct ui_badge_base
 * @brief Internal representation of a badge component.
 */
struct ui_badge_base {
  struct ui_component *component; /**< The core UI component */
  struct ui_signal *text_signal;  /**< Optional signal to bind the text to */
};

/**
 * @brief ui_badge_base_create.
 * @param out_badge Parameter out_badge.
 * @return Return value.
 */
ui_error_t ui_badge_base_create(struct ui_badge_base **out_badge) {
  struct ui_badge_base *badge;
  struct ui_dom_node *root_node = NULL;
  ui_error_t rc;

  if (!out_badge) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  badge = (struct ui_badge_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_badge_base));
  if (!badge) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  badge->component = NULL;
  badge->text_signal = NULL;

  rc = ui_component_create(&badge->component);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dom_node_set_tag_name(root_node, "span");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "status");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Text content is stored in a child text node */
  {
    struct ui_dom_node *text_node = NULL;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE) {
      goto cleanup;
    }
    rc = ui_dom_node_append_child(root_node, text_node);
    if (rc != UI_ERROR_NONE) {
      ui_dom_node_destroy(text_node);
      goto cleanup;
    }
  }

  badge->component->shadow_root = root_node;
  root_node = NULL;

  *out_badge = badge;
  return UI_ERROR_NONE;

cleanup:
  if (root_node) {
    ui_dom_node_destroy(root_node);
  }
  if (badge->component) {
    ui_component_destroy(badge->component);
  }
  C_MULTIPLATFORM_FREE(badge);
  return rc;
}

/**
 * @brief ui_badge_base_destroy.
 * @param badge Parameter badge.
 * @return Return value.
 */
ui_error_t ui_badge_base_destroy(struct ui_badge_base *badge) {
  ui_error_t rc = UI_ERROR_NONE;
  if (badge) {
    if (badge->component) {
      ui_error_t rc_cleanup = ui_component_destroy(badge->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        rc = rc_cleanup;
      }
    }
    C_MULTIPLATFORM_FREE(badge);
  }
  return rc;
}

/**
 * @brief ui_badge_base_set_value.
 * @param badge Parameter badge.
 * @param value Parameter value.
 * @param max_value Parameter max_value.
 * @return Return value.
 */
ui_error_t ui_badge_base_set_value(struct ui_badge_base *badge, int value,
                                   int max_value) {
  char buf[32];

  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (value > max_value) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d+", max_value);
#else
    sprintf(buf, "%d+", max_value);
#endif
  } else {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "%d", value);
#else
    sprintf(buf, "%d", value);
#endif
  }

  if (badge->component->shadow_root->first_child) {
    return ui_dom_node_set_text_content(
        badge->component->shadow_root->first_child, buf);
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief ui_badge_base_set_text.
 * @param badge Parameter badge.
 * @param text Parameter text.
 * @return Return value.
 */
ui_error_t ui_badge_base_set_text(struct ui_badge_base *badge,
                                  const char *text) {
  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (badge->component->shadow_root->first_child) {
    return ui_dom_node_set_text_content(
        badge->component->shadow_root->first_child, text ? text : "");
  }
  return UI_ERROR_INVALID_ARGUMENT;
}

/**
 * @brief ui_badge_base_set_hidden.
 * @param badge Parameter badge.
 * @param is_hidden Parameter is_hidden.
 * @return Return value.
 */
ui_error_t ui_badge_base_set_hidden(struct ui_badge_base *badge,
                                    int is_hidden) {
  if (!badge || !badge->component || !badge->component->shadow_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_hidden) {
    return ui_dom_node_set_attribute(badge->component->shadow_root,
                                     "aria-hidden", "true");
  } else {
    return ui_dom_node_remove_attribute(badge->component->shadow_root,
                                        "aria-hidden");
  }
}

/**
 * @brief ui_badge_base_get_component.
 * @param badge Parameter badge.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t ui_badge_base_get_component(struct ui_badge_base *badge,
                                       struct ui_component **out_component) {
  if (!badge || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = badge->component;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_badge_base_bind_text.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_badge_base_bind_text(struct ui_badge_base *widget,
                                   struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
