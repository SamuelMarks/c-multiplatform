/* clang-format off */
#include "ui_banner_base.h"
#include "ui_internal_mem.h"
#include <stdio.h>
/* clang-format on */

struct ui_banner_base {
  struct ui_component *base;
  int is_open;
  struct ui_signal *open_signal;
  struct ui_computed *animating_signal;
};

enum ui_error ui_banner_base_create(struct ui_banner_base **out_banner) {
  struct ui_banner_base *banner;
  struct ui_component *base_comp;
  enum ui_error err;

  if (!out_banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  err = ui_component_create(&base_comp);
  if (err != UI_ERROR_NONE) {
    return err;
  }

  banner = (struct ui_banner_base *)UI_MALLOC(sizeof(struct ui_banner_base));
  if (!banner) {
    ui_component_destroy(base_comp);
    return UI_ERROR_OUT_OF_MEMORY;
  }

  banner->base = base_comp;

  banner->is_open = 0;
  banner->open_signal = NULL;
  banner->animating_signal = NULL;

  err =
      ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &banner->base->shadow_root);
  if (err != UI_ERROR_NONE) {
    ui_component_destroy(banner->base);
    UI_FREE(banner);
    return err;
  }

  err = ui_dom_node_set_tag_name(banner->base->shadow_root, "ui-banner");
  if (err != UI_ERROR_NONE) {
    ui_dom_node_destroy(banner->base->shadow_root);
    banner->base->shadow_root = NULL;
    ui_component_destroy(banner->base);
    UI_FREE(banner);
    return err;
  }

  ui_dom_node_set_attribute(banner->base->shadow_root, "role", "banner");

  *out_banner = banner;
  return UI_ERROR_NONE;
}

enum ui_error ui_banner_base_set_text(struct ui_banner_base *banner,
                                      const char *text) {
  struct ui_dom_node *text_node;
  enum ui_error err;

  if (!banner || !text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Just clear and set the text of the first child for this stub */
  if (!banner->base->shadow_root->first_child) {
    err = ui_dom_node_create(UI_DOM_NODE_TYPE_TEXT, &text_node);
    if (err != UI_ERROR_NONE) {
      return err;
    }
    ui_dom_node_append_child(banner->base->shadow_root, text_node);
  } else {
    text_node = banner->base->shadow_root->first_child;
  }

  return ui_dom_node_set_text_content(text_node, text);
}

enum ui_error ui_banner_base_set_dismissible(struct ui_banner_base *banner,
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

enum ui_error ui_banner_base_set_open(struct ui_banner_base *banner,
                                      int is_open) {
  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (banner->is_open == is_open) {
    return UI_ERROR_NONE;
  }

  banner->is_open = is_open;

  if (is_open) {
    ui_dom_node_set_attribute(banner->base->shadow_root, "data-open", "true");
  } else {
    ui_dom_node_remove_attribute(banner->base->shadow_root, "data-open");
  }

  if (banner->open_signal) {
    union ui_signal_payload payload;
    payload.bool_val = is_open;
    ui_signal_set(banner->open_signal, payload);
  }

  return UI_ERROR_NONE;
}

enum ui_error ui_banner_base_is_open(const struct ui_banner_base *banner,
                                     int *out_is_open) {
  if (!banner || !out_is_open) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_is_open = banner->is_open;
  return UI_ERROR_NONE;
}

enum ui_error ui_banner_base_bind_open(struct ui_banner_base *banner,
                                       struct ui_signal *open_signal) {
  if (!banner) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  banner->open_signal = open_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_banner_base_get_animating_signal(struct ui_banner_base *banner,
                                    struct ui_computed **out_animating) {
  if (!banner || !out_animating) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_animating = banner->animating_signal;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_banner_base_get_component(struct ui_banner_base *banner,
                             struct ui_component **out_component) {
  if (!banner || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = banner->base;
  return UI_ERROR_NONE;
}

void ui_banner_base_destroy(struct ui_banner_base *banner) {
  if (!banner) {
    return;
  }
  ui_component_destroy(banner->base);
  /* The banner allocation itself was flattened into base_comp, but then
     we allocated banner itself with UI_MALLOC so we need to free it.
     Note that ui_component_destroy only destroys internal fields, not the
     container struct pointer itself if it's embedded. Wait, base was created
     using ui_component_create which mallocs... then copied by value... */
  UI_FREE(banner);
}
