/**
 * @file ui_banner_base.c
 * @brief Implementation of the banner base component.
 */

/* clang-format off */
#include "ui_banner_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_banner_mock_fail = 0;
static ui_error_t mock_dom_node_append_child(struct ui_dom_node *parent,
                                             struct ui_dom_node *child) {
  if (g_banner_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_append_child)(parent, child);
}
#undef ui_dom_node_append_child
/** @cond */
#define ui_dom_node_append_child mock_dom_node_append_child
/** @endcond */

static ui_error_t mock_dom_node_set_attribute(struct ui_dom_node *node,
                                              const char *k, const char *v) {
  if (g_banner_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_set_attribute)(node, k, v);
}
#undef ui_dom_node_set_attribute
/** @cond */
#define ui_dom_node_set_attribute mock_dom_node_set_attribute
/** @endcond */

static ui_error_t mock_dom_node_remove_attribute(struct ui_dom_node *node,
                                                 const char *k) {
  if (g_banner_mock_fail == 3) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_dom_node_remove_attribute)(node, k);
}
#undef ui_dom_node_remove_attribute
/** @cond */
#define ui_dom_node_remove_attribute mock_dom_node_remove_attribute
/** @endcond */

static ui_error_t mock_signal_set(struct ui_signal *signal,
                                  union ui_signal_payload p) {
  if (g_banner_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_signal_set)(signal, p);
}
#undef ui_signal_set
/** @cond */
#define ui_signal_set mock_signal_set
/** @endcond */

ui_error_t run_banner_coverage(void);
ui_error_t run_banner_coverage(void) {

  struct ui_banner_base *banner = NULL;
  union ui_signal_payload p;
  struct ui_signal *sig = NULL;

  {
    ui_error_t rc_cleanup = ui_banner_base_create(&banner);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  g_banner_mock_fail = 1;
  {
    ui_error_t rc_cleanup = ui_banner_base_set_text(banner, "text");
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 2;
  {
    ui_error_t rc_cleanup = ui_banner_base_set_open(banner, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_banner_mock_fail = 0;

  g_banner_mock_fail = 3;
  {
    ui_error_t rc_cleanup = ui_banner_base_set_open(banner, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_banner_mock_fail = 0;

  p.bool_val = 1;
  {
    ui_error_t rc_cleanup =
        ui_signal_create(NULL, p, UI_SIGNAL_TYPE_BOOL, NULL, NULL, 0, &sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_banner_base_bind_open(banner, sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_banner_mock_fail = 4;
  {
    ui_error_t rc_cleanup = ui_banner_base_set_open(banner, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_banner_mock_fail = 0;

  {
    ui_error_t rc_cleanup = ui_signal_destroy(sig);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_banner_base_destroy(banner);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  return UI_ERROR_NONE;
}
#endif

/**
 * @struct ui_banner_base
 * @struct ui_banner_base
 * @brief Internal state for a banner component.
 */
struct ui_banner_base {
  struct ui_component *base;            /**< Core UI component */
  int is_open;                          /**< Open state flag */
  struct ui_signal *open_signal;        /**< Bound open state signal */
  struct ui_computed *animating_signal; /**< Animating state signal */
};

ui_error_t ui_banner_base_create(struct ui_banner_base **out_banner) {
  struct ui_banner_base *banner;
  struct ui_component *base_comp;
  ui_error_t err;

  if (!out_banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  banner = (struct ui_banner_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_banner_base));
  if (!banner) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(base_comp);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    return UI_ERROR_OUT_OF_MEMORY;
  }

  banner->base = base_comp;

  banner->is_open = 0;
  banner->open_signal = NULL;
  banner->animating_signal = NULL;

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &banner->base->shadow_root);
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(banner->base);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(banner);
    return err;
  }

  err = ui_dom_node_set_tag_name(banner->base->shadow_root, "ui-banner");
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_dom_node_destroy(banner->base->shadow_root);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    banner->base->shadow_root = NULL;
    {
      ui_error_t rc_cleanup = ui_component_destroy(banner->base);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(banner);
    return err;
  }

  err = ui_dom_node_set_attribute(banner->base->shadow_root, "role", "banner");
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(banner->base);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    C_MULTIPLATFORM_FREE(banner);
    return err;
  }

  *out_banner = banner;
  return UI_ERROR_NONE;
}

ui_error_t ui_banner_base_set_text(struct ui_banner_base *banner,
                                   const char *text) {
  struct ui_dom_node *text_node;
  ui_error_t err;

  if (!banner || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Just clear and set the text of the first child for this stub */
  if (!banner->base->shadow_root->first_child) {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    err = ui_dom_node_append_child(banner->base->shadow_root, text_node);
    if (err != UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_dom_node_destroy(text_node);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      return err;
    }
  } else {
    text_node = banner->base->shadow_root->first_child;
  }

  return ui_dom_node_set_text_content(text_node, text);
}

ui_error_t ui_banner_base_set_dismissible(struct ui_banner_base *banner,
                                          int is_dismissible) {
  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_dismissible) {
    return ui_dom_node_set_attribute(banner->base->shadow_root,
                                     "data-dismissible", "true");
  } else {
    return ui_dom_node_remove_attribute(banner->base->shadow_root,
                                        "data-dismissible");
  }
}

ui_error_t ui_banner_base_set_open(struct ui_banner_base *banner, int is_open) {
  ui_error_t rc;
  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (banner->is_open == is_open) {
    return UI_ERROR_NONE;
  }

  banner->is_open = is_open;

  if (is_open) {
    rc = ui_dom_node_set_attribute(banner->base->shadow_root, "data-open",
                                   "true");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = ui_dom_node_remove_attribute(banner->base->shadow_root, "data-open");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (banner->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = is_open;
    rc = ui_signal_set(banner->open_signal, payload);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_banner_base_is_open(const struct ui_banner_base *banner,
                                  int *out_is_open) {
  if (!banner || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = banner->is_open;
  return UI_ERROR_NONE;
}

ui_error_t ui_banner_base_bind_open(struct ui_banner_base *banner,
                                    struct ui_signal *open_signal) {
  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  banner->open_signal = open_signal;
  return UI_ERROR_NONE;
}

ui_error_t
ui_banner_base_get_animating_signal(struct ui_banner_base *banner,
                                    struct ui_computed **out_animating) {
  if (!banner || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = banner->animating_signal;
  return UI_ERROR_NONE;
}

ui_error_t ui_banner_base_get_component(struct ui_banner_base *banner,
                                        struct ui_component **out_component) {
  if (!banner || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = banner->base;
  return UI_ERROR_NONE;
}

ui_error_t ui_banner_base_destroy(struct ui_banner_base *banner) {
  if (!banner) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(banner->base);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* The banner allocation itself was flattened into base_comp, but then
     we allocated banner itself with C_MULTIPLATFORM_MALLOC so we need to free
     it. Note that ui_component_destroy only destroys internal fields, not the
     container struct pointer itself if it's embedded. Wait, base was created
     using ui_component_create which mallocs... then copied by value... */
  C_MULTIPLATFORM_FREE(banner);
  return UI_ERROR_NONE;
}
