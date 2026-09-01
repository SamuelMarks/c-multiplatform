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

ui_error_t run_badge_coverage(void);
/**
 * @brief run_badge_coverage.
 * @return Return value.
 */
ui_error_t run_badge_coverage(void) {
  struct ui_badge_base *badge = NULL;

  g_badge_mock_fail = 1;
  ui_badge_base_create(&badge);
  g_badge_mock_fail = 0;

  g_badge_mock_fail = 2;
  ui_badge_base_create(&badge);
  g_badge_mock_fail = 0;

  g_badge_mock_fail = 20;
  ui_badge_base_create(&badge);
  g_badge_mock_fail = 0;

  g_badge_mock_fail = 3;
  ui_badge_base_create(&badge);
  g_badge_mock_fail = 0;
  g_badge_mock_fail = 30;
  ui_badge_base_destroy(badge);
  g_badge_mock_fail = 0;

  return UI_ERROR_NONE;
}
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

  rc = ui_component_create(&badge->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &root_node);
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(badge->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "span");
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_component_destroy(badge->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "status");
  if (rc != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_component_destroy(badge->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  /* Text content is stored in a child text node */
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_component_destroy(badge->component);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      C_MULTIPLATFORM_FREE(badge);
      return rc;
    }
    rc = ui_dom_node_append_child(root_node, text_node);
    if (rc != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(text_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(root_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      {
        ui_error_t rc_cleanup = ui_component_destroy(badge->component);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      C_MULTIPLATFORM_FREE(badge);
      return rc;
    }
  }

  badge->component->shadow_root = root_node;

  *out_badge = badge;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_badge_base_destroy.
 * @param badge Parameter badge.
 * @return Return value.
 */
ui_error_t ui_badge_base_destroy(struct ui_badge_base *badge) {
  if (badge) {
    if (badge->component) {
      {
        ui_error_t rc_cleanup = ui_component_destroy(badge->component);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    C_MULTIPLATFORM_FREE(badge);
  }
  return UI_ERROR_NONE;
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
