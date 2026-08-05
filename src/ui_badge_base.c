/* clang-format off */
#include "ui_badge_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_badge_mock_fail = 0;
static ui_error_t mock_dom_node_set_tag_name(struct ui_dom_node *node,
                                             const char *tag) {
  if (g_badge_mock_fail == 1)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_set_tag_name)(node, tag);
}
#undef ui_dom_node_set_tag_name
#define ui_dom_node_set_tag_name mock_dom_node_set_tag_name

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
#define ui_dom_node_create mock_dom_node_create

static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_badge_mock_fail == 3)
    return UI_ERROR_UNKNOWN;
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
#define ui_dom_node_append_child mock_dom_node_append_child

ui_error_t run_badge_coverage(void);
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

  return UI_ERROR_NONE;
}
#endif

#if defined(_MSC_VER)
/* MSVC Safe CRT */
#endif

struct ui_badge_base {
  struct ui_component *component;
  struct ui_signal *text_signal;
};

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
    (void)ui_component_destroy(badge->component);
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  rc = ui_dom_node_set_tag_name(root_node, "span");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(badge->component);
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }
  rc = ui_dom_node_set_attribute(root_node, "role", "status");
  if (rc != UI_ERROR_NONE) {
    (void)ui_dom_node_destroy(root_node);
    (void)ui_component_destroy(badge->component);
    C_MULTIPLATFORM_FREE(badge);
    return rc;
  }

  /* Text content is stored in a child text node */
  {
    struct ui_dom_node *text_node;
    rc = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(badge->component);
      C_MULTIPLATFORM_FREE(badge);
      return rc;
    }
    rc = ui_dom_node_append_child(root_node, text_node);
    if (rc != UI_ERROR_NONE) {
      (void)ui_dom_node_destroy(text_node);
      (void)ui_dom_node_destroy(root_node);
      (void)ui_component_destroy(badge->component);
      C_MULTIPLATFORM_FREE(badge);
      return rc;
    }
  }

  badge->component->shadow_root = root_node;

  *out_badge = badge;
  return UI_ERROR_NONE;
}

ui_error_t ui_badge_base_destroy(struct ui_badge_base *badge) {
  if (badge) {
    if (badge->component) {
      (void)ui_component_destroy(badge->component);
    }
    C_MULTIPLATFORM_FREE(badge);
  }
  return UI_ERROR_NONE;
}

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

ui_error_t ui_badge_base_get_component(struct ui_badge_base *badge,
                                       struct ui_component **out_component) {
  if (!badge || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = badge->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_badge_base_bind_text(struct ui_badge_base *widget,
                                   struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = signal;
  return UI_ERROR_NONE;
}
